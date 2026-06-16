#include "aidog/ble_transport_win.hpp"

#if !AIDOG_ENABLE_WINDOWS_BLE

#include "aidog/error.hpp"

namespace aidog {

struct BleTransportWin::Impl {
    explicit Impl(NotifyCallback) {}
};

BleTransportWin::BleTransportWin(NotifyCallback onNotify)
    : impl_(std::make_unique<Impl>(std::move(onNotify)))
{
}

BleTransportWin::~BleTransportWin() = default;

std::vector<DeviceInfo> BleTransportWin::scan(const std::string&)
{
    throw UnsupportedError("Windows BLE transport is disabled");
}

bool BleTransportWin::connect(const std::string&, int, double)
{
    throw UnsupportedError("Windows BLE transport is disabled");
}

void BleTransportWin::disconnect() {}

void BleTransportWin::shutdown() {}

nlohmann::json BleTransportWin::read_actions()
{
    throw UnsupportedError("Windows BLE transport is disabled");
}

void BleTransportWin::send_control(std::uint8_t, std::span<const std::uint8_t>)
{
    throw UnsupportedError("Windows BLE transport is disabled");
}

void BleTransportWin::send_control_json(const nlohmann::json&)
{
    throw UnsupportedError("Windows BLE transport is disabled");
}

void BleTransportWin::send_config(const nlohmann::json&)
{
    throw UnsupportedError("Windows BLE transport is disabled");
}

bool BleTransportWin::is_connected() const
{
    return false;
}

} // namespace aidog

#endif
