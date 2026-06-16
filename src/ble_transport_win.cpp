#include "aidog/ble_transport_win.hpp"

#if AIDOG_ENABLE_WINDOWS_BLE

#include <chrono>
#include <cctype>
#include <mutex>
#include <sstream>
#include <thread>

#include <winrt/Windows.Devices.Bluetooth.h>
#include <winrt/Windows.Devices.Bluetooth.Advertisement.h>
#include <winrt/Windows.Devices.Bluetooth.GenericAttributeProfile.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Storage.Streams.h>

#include "aidog/error.hpp"
#include "aidog/protocol.hpp"

namespace aidog {
namespace {

using namespace winrt;
using namespace Windows::Devices::Bluetooth;
using namespace Windows::Devices::Bluetooth::Advertisement;
using namespace Windows::Devices::Bluetooth::GenericAttributeProfile;
using namespace Windows::Storage::Streams;

constexpr wchar_t kUuidAe01[] = L"0000ae01-0000-1000-8000-00805f9b34fb";
constexpr wchar_t kUuidAe02[] = L"0000ae02-0000-1000-8000-00805f9b34fb";
constexpr wchar_t kUuidAe03[] = L"0000ae03-0000-1000-8000-00805f9b34fb";
constexpr wchar_t kUuidAe04[] = L"0000ae04-0000-1000-8000-00805f9b34fb";
constexpr wchar_t kUuidAe10[] = L"0000ae10-0000-1000-8000-00805f9b34fb";

guid _guid(const wchar_t* text)
{
    return guid{text};
}

std::string _narrow(const winrt::hstring& text)
{
    return winrt::to_string(text);
}

IBuffer _make_buffer(std::span<const std::uint8_t> data)
{
    DataWriter writer;
    writer.WriteBytes(data);
    return writer.DetachBuffer();
}

std::vector<std::uint8_t> _read_buffer(const IBuffer& buffer)
{
    DataReader reader = DataReader::FromBuffer(buffer);
    std::vector<std::uint8_t> out(reader.UnconsumedBufferLength());
    reader.ReadBytes(out);
    return out;
}

bool _is_success(GattCommunicationStatus status)
{
    return status == GattCommunicationStatus::Success;
}

std::uint64_t _parse_bluetooth_address(const std::string& address)
{
    std::string compact;
    compact.reserve(address.size());
    bool hasHexSeparator = false;
    bool hasHexLetter = false;
    for (unsigned char ch : address) {
        if (ch == ':' || ch == '-') {
            hasHexSeparator = true;
            continue;
        }
        if (std::isxdigit(ch) == 0) {
            throw ConnectionError("invalid BLE address format");
        }
        if (std::isalpha(ch) != 0) {
            hasHexLetter = true;
        }
        compact.push_back(static_cast<char>(ch));
    }
    if (compact.empty()) {
        throw ConnectionError("BLE address is empty");
    }
    if (hasHexSeparator || hasHexLetter) {
        if (compact.size() > 12) {
            throw ConnectionError("BLE MAC address is too long");
        }
        return std::stoull(compact, nullptr, 16);
    }

    std::size_t consumed = 0;
    auto value = std::stoull(compact, &consumed, 10);
    if (consumed != compact.size()) {
        throw ConnectionError("invalid numeric BLE address");
    }
    return value;
}

} // namespace

struct BleTransportWin::Impl {
    explicit Impl(NotifyCallback cb) : onNotify(std::move(cb))
    {
        winrt::init_apartment();
    }

    ~Impl()
    {
        try {
            disconnect();
        } catch (...) {
        }
    }

    NotifyCallback onNotify;
    mutable std::mutex mutex;
    BluetoothLEDevice device{nullptr};
    GattCharacteristic configChar{nullptr};
    GattCharacteristic notifyChar{nullptr};
    GattCharacteristic sensorNotifyChar{nullptr};
    GattCharacteristic writeChar{nullptr};
    GattCharacteristic actionsChar{nullptr};
    winrt::event_token notifyToken{};
    winrt::event_token sensorNotifyToken{};
    bool connected = false;

    std::vector<DeviceInfo> scan(const std::string& namePrefix)
    {
        std::mutex scanMutex;
        std::vector<DeviceInfo> devices;
        BluetoothLEAdvertisementWatcher watcher;
        watcher.ScanningMode(BluetoothLEScanningMode::Active);
        auto token = watcher.Received([&](auto&&, const BluetoothLEAdvertisementReceivedEventArgs& args) {
            auto name = _narrow(args.Advertisement().LocalName());
            if (name.empty() || name.rfind(namePrefix, 0) != 0) {
                return;
            }
            auto address = std::to_string(args.BluetoothAddress());
            std::lock_guard<std::mutex> lock(scanMutex);
            auto found = std::find_if(devices.begin(), devices.end(), [&](const DeviceInfo& item) {
                return item.address == address;
            });
            if (found == devices.end()) {
                devices.push_back(DeviceInfo{name, address});
            }
        });
        watcher.Start();
        std::this_thread::sleep_for(std::chrono::seconds(5));
        watcher.Stop();
        watcher.Received(token);
        return devices;
    }

    bool connect(const std::string& address, int retries, double retryDelayS)
    {
        const auto attempts = std::max(1, retries);
        std::exception_ptr lastError;
        for (int i = 0; i < attempts; ++i) {
            try {
                connect_once(address);
                return true;
            } catch (...) {
                lastError = std::current_exception();
                if (i + 1 < attempts && retryDelayS > 0.0) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(retryDelayS * 1000.0)));
                }
            }
        }
        if (lastError) {
            std::rethrow_exception(lastError);
        }
        return false;
    }

    void connect_once(const std::string& address)
    {
        disconnect();
        try {
            auto bluetoothAddress = _parse_bluetooth_address(address);
            auto dev = BluetoothLEDevice::FromBluetoothAddressAsync(bluetoothAddress).get();
            if (!dev) {
                throw ConnectionError("failed to open BLE device");
            }
            device = dev;
        } catch (const AidogError&) {
            throw;
        } catch (const std::exception& exc) {
            throw ConnectionError(std::string("invalid BLE address: ") + exc.what());
        }

        auto servicesResult = device.GetGattServicesAsync(BluetoothCacheMode::Uncached).get();
        if (!_is_success(servicesResult.Status())) {
            throw ConnectionError("failed to discover GATT services");
        }

        for (const auto& service : servicesResult.Services()) {
            auto charsResult = service.GetCharacteristicsAsync(BluetoothCacheMode::Uncached).get();
            if (!_is_success(charsResult.Status())) {
                continue;
            }
            for (const auto& ch : charsResult.Characteristics()) {
                auto uuid = ch.Uuid();
                if (uuid == _guid(kUuidAe01)) {
                    configChar = ch;
                } else if (uuid == _guid(kUuidAe02)) {
                    notifyChar = ch;
                } else if (uuid == _guid(kUuidAe03)) {
                    writeChar = ch;
                } else if (uuid == _guid(kUuidAe04)) {
                    sensorNotifyChar = ch;
                } else if (uuid == _guid(kUuidAe10)) {
                    actionsChar = ch;
                }
            }
        }

        if (!writeChar) {
            throw ConnectionError("write characteristic ae03 not found");
        }

        subscribe_notify(notifyChar, notifyToken);
        subscribe_notify(sensorNotifyChar, sensorNotifyToken);
        connected = true;
    }

    void subscribe_notify(GattCharacteristic ch, winrt::event_token& token)
    {
        if (!ch) {
            return;
        }
        token = ch.ValueChanged([this](const GattCharacteristic&, const GattValueChangedEventArgs& args) {
            if (onNotify) {
                onNotify(_read_buffer(args.CharacteristicValue()));
            }
        });
        auto status = ch.WriteClientCharacteristicConfigurationDescriptorAsync(
                            GattClientCharacteristicConfigurationDescriptorValue::Notify)
                          .get();
        if (!_is_success(status)) {
            ch.WriteClientCharacteristicConfigurationDescriptorAsync(
                  GattClientCharacteristicConfigurationDescriptorValue::Indicate)
                .get();
        }
    }

    void disconnect()
    {
        std::lock_guard<std::mutex> lock(mutex);
        try {
            if (notifyChar) {
                notifyChar.ValueChanged(notifyToken);
            }
            if (sensorNotifyChar) {
                sensorNotifyChar.ValueChanged(sensorNotifyToken);
            }
        } catch (...) {
        }
        configChar = nullptr;
        notifyChar = nullptr;
        sensorNotifyChar = nullptr;
        writeChar = nullptr;
        actionsChar = nullptr;
        device = nullptr;
        connected = false;
    }

    void write_raw(GattCharacteristic ch, std::span<const std::uint8_t> payload)
    {
        if (!connected || !ch) {
            throw ConnectionError("BLE characteristic is not connected");
        }
        auto status = ch.WriteValueAsync(_make_buffer(payload), GattWriteOption::WriteWithoutResponse).get();
        if (!_is_success(status)) {
            throw ConnectionError("BLE write failed");
        }
    }

    void send_control(std::uint8_t mode, std::span<const std::uint8_t> data)
    {
        auto packet = make_raw_packet(mode, data);
        write_raw(writeChar, packet);
    }

    void send_control_json(const nlohmann::json& payload)
    {
        auto text = payload.dump();
        std::vector<std::uint8_t> bytes(text.begin(), text.end());
        bytes.push_back(0);
        write_raw(writeChar, bytes);
    }

    void send_config(const nlohmann::json& config)
    {
        if (!configChar) {
            throw ConnectionError("config characteristic ae01 not found");
        }
        auto text = config.dump();
        std::vector<std::uint8_t> bytes(text.begin(), text.end());
        bytes.push_back(0);
        write_raw(configChar, bytes);
    }

    nlohmann::json read_actions()
    {
        if (!actionsChar) {
            throw ConnectionError("action characteristic ae10 not found");
        }
        std::string buf;
        for (int i = 0; i < 10; ++i) {
            auto result = actionsChar.ReadValueAsync(BluetoothCacheMode::Uncached).get();
            if (!_is_success(result.Status())) {
                throw ConnectionError("read ae10 failed");
            }
            auto bytes = _read_buffer(result.Value());
            buf.append(reinterpret_cast<const char*>(bytes.data()), bytes.size());
            try {
                return nlohmann::json::parse(buf);
            } catch (...) {
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
            }
        }
        throw ProtocolError("could not read complete action JSON from ae10");
    }
};

BleTransportWin::BleTransportWin(NotifyCallback onNotify)
    : impl_(std::make_unique<Impl>(std::move(onNotify)))
{
}

BleTransportWin::~BleTransportWin() = default;

std::vector<DeviceInfo> BleTransportWin::scan(const std::string& namePrefix)
{
    return impl_->scan(namePrefix);
}

bool BleTransportWin::connect(const std::string& address, int retries, double retryDelayS)
{
    return impl_->connect(address, retries, retryDelayS);
}

void BleTransportWin::disconnect()
{
    impl_->disconnect();
}

void BleTransportWin::shutdown()
{
    impl_->disconnect();
}

nlohmann::json BleTransportWin::read_actions()
{
    return impl_->read_actions();
}

void BleTransportWin::send_control(std::uint8_t mode, std::span<const std::uint8_t> data)
{
    impl_->send_control(mode, data);
}

void BleTransportWin::send_control_json(const nlohmann::json& payload)
{
    impl_->send_control_json(payload);
}

void BleTransportWin::send_config(const nlohmann::json& config)
{
    impl_->send_config(config);
}

bool BleTransportWin::is_connected() const
{
    return impl_->connected;
}

} // namespace aidog

#endif
