/*
Purpose:
    Start the Dev PC WebSocket host and stream realtime PCM audio both ways.
Risk level:
    Medium. This example captures microphone audio and plays robot audio.
Run:
    .\build\Release\aidog_ws_bidirectional_pcm_host.exe --list-devices
    .\build\Release\aidog_ws_bidirectional_pcm_host.exe --connect-timeout 120
    .\build\Release\aidog_ws_bidirectional_pcm_host.exe --connect-timeout 120 --input-device 1 --output-device 1
    .\build\Release\aidog_ws_bidirectional_pcm_host.exe --connect-timeout 120 --no-input
    .\build\Release\aidog_ws_bidirectional_pcm_host.exe --connect-timeout 120 --no-output
Audio format:
    16000 Hz, mono, signed 16-bit PCM, 20 ms per packet.
Exit:
    Press Ctrl+C.
*/

#include <atomic>
#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <cstring>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

#ifndef _WIN32
#error "bidirectional_pcm_ws_host is Windows-only"
#endif

#include <windows.h>
#include <mmsystem.h>

#include <aidog.hpp>

namespace {

constexpr int kSampleRate = 16000;
constexpr int kChannels = 1;
constexpr int kBitsPerSample = 16;
constexpr int kChunkSamples = 320;
constexpr int kChunkBytes = kChunkSamples * kChannels * (kBitsPerSample / 8);
constexpr int kCaptureBufferCount = 6;
constexpr int kPlaybackBufferCount = 8;

struct Options {
    std::string bind = "0.0.0.0";
    int port = 8766;
    double connectTimeoutS = 120.0;
    int seconds = 0;
    int inputDevice = WAVE_MAPPER;
    int outputDevice = WAVE_MAPPER;
    bool listDevices = false;
    bool inputEnabled = true;
    bool outputEnabled = true;
};

std::string _wide_to_utf8(const wchar_t* text)
{
    if (text == nullptr || text[0] == L'\0') {
        return {};
    }
    const int size = WideCharToMultiByte(CP_UTF8, 0, text, -1, nullptr, 0, nullptr, nullptr);
    std::string result(static_cast<std::size_t>((std::max)(0, size - 1)), '\0');
    if (size > 1) {
        WideCharToMultiByte(CP_UTF8, 0, text, -1, result.data(), size, nullptr, nullptr);
    }
    return result;
}

std::string _mm_error(MMRESULT result)
{
    wchar_t buffer[MAXERRORLENGTH] = {};
    if (waveInGetErrorTextW(result, buffer, MAXERRORLENGTH) == MMSYSERR_NOERROR) {
        return _wide_to_utf8(buffer);
    }
    if (waveOutGetErrorTextW(result, buffer, MAXERRORLENGTH) == MMSYSERR_NOERROR) {
        return _wide_to_utf8(buffer);
    }
    return "MMRESULT=" + std::to_string(result);
}

void _list_devices()
{
    std::cout << "Input devices:\n";
    const UINT inputCount = waveInGetNumDevs();
    if (inputCount == 0) {
        std::cout << "  (none)\n";
    }
    for (UINT i = 0; i < inputCount; ++i) {
        WAVEINCAPSW caps = {};
        if (waveInGetDevCapsW(i, &caps, sizeof(caps)) == MMSYSERR_NOERROR) {
            std::cout << "  [" << i << "] " << _wide_to_utf8(caps.szPname) << "\n";
        }
    }

    std::cout << "Output devices:\n";
    const UINT outputCount = waveOutGetNumDevs();
    if (outputCount == 0) {
        std::cout << "  (none)\n";
    }
    for (UINT i = 0; i < outputCount; ++i) {
        WAVEOUTCAPSW caps = {};
        if (waveOutGetDevCapsW(i, &caps, sizeof(caps)) == MMSYSERR_NOERROR) {
            std::cout << "  [" << i << "] " << _wide_to_utf8(caps.szPname) << "\n";
        }
    }
}

Options _parse_options(int argc, char** argv)
{
    Options options;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--bind" && i + 1 < argc) {
            options.bind = argv[++i];
        } else if (arg == "--port" && i + 1 < argc) {
            options.port = std::stoi(argv[++i]);
        } else if (arg == "--connect-timeout" && i + 1 < argc) {
            options.connectTimeoutS = std::stod(argv[++i]);
        } else if (arg == "--seconds" && i + 1 < argc) {
            options.seconds = std::stoi(argv[++i]);
        } else if (arg == "--input-device" && i + 1 < argc) {
            options.inputDevice = std::stoi(argv[++i]);
        } else if (arg == "--output-device" && i + 1 < argc) {
            options.outputDevice = std::stoi(argv[++i]);
        } else if (arg == "--list-devices") {
            options.listDevices = true;
        } else if (arg == "--no-input") {
            options.inputEnabled = false;
        } else if (arg == "--no-output") {
            options.outputEnabled = false;
        }
    }
    return options;
}

WAVEFORMATEX _pcm_format()
{
    WAVEFORMATEX format = {};
    format.wFormatTag = WAVE_FORMAT_PCM;
    format.nChannels = kChannels;
    format.nSamplesPerSec = kSampleRate;
    format.wBitsPerSample = kBitsPerSample;
    format.nBlockAlign = static_cast<WORD>((format.nChannels * format.wBitsPerSample) / 8);
    format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;
    return format;
}

class WaveInput {
public:
    using Callback = std::function<void(const std::uint8_t* data, std::size_t size)>;

    WaveInput(int deviceId, Callback callback)
        : callback_(std::move(callback))
    {
        auto format = _pcm_format();
        auto result = waveInOpen(&handle_, static_cast<UINT>(deviceId), &format, reinterpret_cast<DWORD_PTR>(&WaveInput::_wave_in_proc),
            reinterpret_cast<DWORD_PTR>(this), CALLBACK_FUNCTION);
        if (result != MMSYSERR_NOERROR) {
            throw std::runtime_error("waveInOpen failed: " + _mm_error(result));
        }

        buffers_.resize(kCaptureBufferCount);
        headers_.resize(kCaptureBufferCount);
        for (int i = 0; i < kCaptureBufferCount; ++i) {
            buffers_[i].resize(kChunkBytes);
            headers_[i].lpData = reinterpret_cast<LPSTR>(buffers_[i].data());
            headers_[i].dwBufferLength = static_cast<DWORD>(buffers_[i].size());
            result = waveInPrepareHeader(handle_, &headers_[i], sizeof(WAVEHDR));
            if (result != MMSYSERR_NOERROR) {
                throw std::runtime_error("waveInPrepareHeader failed: " + _mm_error(result));
            }
            result = waveInAddBuffer(handle_, &headers_[i], sizeof(WAVEHDR));
            if (result != MMSYSERR_NOERROR) {
                throw std::runtime_error("waveInAddBuffer failed: " + _mm_error(result));
            }
        }
    }

    ~WaveInput()
    {
        stop();
        if (handle_ != nullptr) {
            for (auto& header : headers_) {
                waveInUnprepareHeader(handle_, &header, sizeof(WAVEHDR));
            }
            waveInClose(handle_);
        }
    }

    void start()
    {
        auto result = waveInStart(handle_);
        if (result != MMSYSERR_NOERROR) {
            throw std::runtime_error("waveInStart failed: " + _mm_error(result));
        }
        running_ = true;
    }

    void stop()
    {
        if (!running_.exchange(false) || handle_ == nullptr) {
            return;
        }
        waveInStop(handle_);
        waveInReset(handle_);
    }

private:
    static void CALLBACK _wave_in_proc(HWAVEIN, UINT msg, DWORD_PTR user, DWORD_PTR param1, DWORD_PTR)
    {
        if (msg != WIM_DATA || user == 0) {
            return;
        }
        auto* self = reinterpret_cast<WaveInput*>(user);
        auto* header = reinterpret_cast<WAVEHDR*>(param1);
        if (self->running_ && header->dwBytesRecorded > 0 && self->callback_) {
            self->callback_(reinterpret_cast<const std::uint8_t*>(header->lpData), header->dwBytesRecorded);
        }
        if (self->running_) {
            header->dwFlags &= ~WHDR_DONE;
            waveInAddBuffer(self->handle_, header, sizeof(WAVEHDR));
        }
    }

    HWAVEIN handle_ = nullptr;
    std::atomic_bool running_ = false;
    Callback callback_;
    std::vector<std::vector<std::uint8_t>> buffers_;
    std::vector<WAVEHDR> headers_;
};

class WaveOutput {
public:
    explicit WaveOutput(int deviceId)
    {
        auto format = _pcm_format();
        auto result = waveOutOpen(&handle_, static_cast<UINT>(deviceId), &format, reinterpret_cast<DWORD_PTR>(&WaveOutput::_wave_out_proc),
            reinterpret_cast<DWORD_PTR>(this), CALLBACK_FUNCTION);
        if (result != MMSYSERR_NOERROR) {
            throw std::runtime_error("waveOutOpen failed: " + _mm_error(result));
        }
        worker_ = std::thread([this]() {
            _run();
        });
    }

    ~WaveOutput()
    {
        stop();
    }

    void write(std::vector<std::uint8_t> data)
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (stopping_) {
                return;
            }
            if (queue_.size() > 50) {
                queue_.pop();
            }
            queue_.push(std::move(data));
        }
        cv_.notify_one();
    }

    void stop()
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (stopping_) {
                return;
            }
            stopping_ = true;
        }
        cv_.notify_all();
        doneCv_.notify_all();
        if (worker_.joinable()) {
            worker_.join();
        }
        if (handle_ != nullptr) {
            waveOutReset(handle_);
            for (auto& header : headers_) {
                if ((header.dwFlags & WHDR_PREPARED) != 0) {
                    waveOutUnprepareHeader(handle_, &header, sizeof(WAVEHDR));
                }
            }
            waveOutClose(handle_);
            handle_ = nullptr;
        }
    }

private:
    static void CALLBACK _wave_out_proc(HWAVEOUT, UINT msg, DWORD_PTR user, DWORD_PTR, DWORD_PTR)
    {
        if (msg != WOM_DONE || user == 0) {
            return;
        }
        auto* self = reinterpret_cast<WaveOutput*>(user);
        self->doneCv_.notify_one();
    }

    bool _has_free_header() const
    {
        for (const auto& header : headers_) {
            if ((header.dwFlags & WHDR_INQUEUE) == 0) {
                return true;
            }
        }
        return false;
    }

    WAVEHDR* _free_header()
    {
        for (auto& header : headers_) {
            if ((header.dwFlags & WHDR_INQUEUE) == 0) {
                return &header;
            }
        }
        return nullptr;
    }

    void _run()
    {
        headers_.resize(kPlaybackBufferCount);
        buffers_.resize(kPlaybackBufferCount);
        while (true) {
            std::vector<std::uint8_t> data;
            {
                std::unique_lock<std::mutex> lock(mutex_);
                cv_.wait(lock, [this]() {
                    return stopping_ || !queue_.empty();
                });
                if (stopping_ && queue_.empty()) {
                    break;
                }
                doneCv_.wait(lock, [this]() {
                    return stopping_ || _has_free_header();
                });
                if (stopping_) {
                    break;
                }
                data = std::move(queue_.front());
                queue_.pop();
            }

            auto* header = _free_header();
            if (header == nullptr) {
                continue;
            }
            const auto index = static_cast<std::size_t>(header - headers_.data());
            buffers_[index] = std::move(data);
            if ((header->dwFlags & WHDR_PREPARED) != 0) {
                waveOutUnprepareHeader(handle_, header, sizeof(WAVEHDR));
            }
            std::memset(header, 0, sizeof(WAVEHDR));
            header->lpData = reinterpret_cast<LPSTR>(buffers_[index].data());
            header->dwBufferLength = static_cast<DWORD>(buffers_[index].size());
            auto result = waveOutPrepareHeader(handle_, header, sizeof(WAVEHDR));
            if (result == MMSYSERR_NOERROR) {
                result = waveOutWrite(handle_, header, sizeof(WAVEHDR));
            }
            if (result != MMSYSERR_NOERROR) {
                std::cerr << "waveOutWrite failed: " << _mm_error(result) << "\n";
            }
        }
    }

    HWAVEOUT handle_ = nullptr;
    std::mutex mutex_;
    std::condition_variable cv_;
    std::condition_variable doneCv_;
    std::queue<std::vector<std::uint8_t>> queue_;
    std::vector<std::vector<std::uint8_t>> buffers_;
    std::vector<WAVEHDR> headers_;
    std::thread worker_;
    bool stopping_ = false;
};

} // namespace

int main(int argc, char** argv)
{
    try {
        auto options = _parse_options(argc, argv);
        if (options.listDevices) {
            _list_devices();
            return 0;
        }

        std::atomic<std::uint64_t> uplinkBytes = 0;
        std::atomic<std::uint64_t> downlinkBytes = 0;
        std::unique_ptr<WaveOutput> output;
        if (options.outputEnabled) {
            output = std::make_unique<WaveOutput>(options.outputDevice);
        }

        aidog::AiDog dog;
        aidog::WebSocketHost host(options.bind, options.port, &dog);
        host.set_pcm_callback([&](const std::vector<std::uint8_t>& pcm) {
            downlinkBytes += pcm.size();
            if (output) {
                output->write(pcm);
            }
        });
        host.start();
        std::cout << "waiting for robot on ws://" << options.bind << ":" << options.port << "\n";
        if (!host.wait_robot_connected(options.connectTimeoutS)) {
            std::cerr << "timeout waiting for robot\n";
            return 1;
        }
        std::cout << "robot_connected=true\n";

        std::unique_ptr<WaveInput> input;
        if (options.inputEnabled) {
            input = std::make_unique<WaveInput>(options.inputDevice, [&](const std::uint8_t* data, std::size_t size) {
                std::vector<std::uint8_t> pcm(data, data + size);
                try {
                    host.send_pcm(pcm);
                    uplinkBytes += pcm.size();
                } catch (const std::exception& exc) {
                    static std::atomic_bool printed = false;
                    if (!printed.exchange(true)) {
                        std::cerr << "send pcm failed: " << exc.what() << "\n";
                    }
                }
            });
            input->start();
        }

        auto start = std::chrono::steady_clock::now();
        auto last = start;
        std::uint64_t lastUp = 0;
        std::uint64_t lastDown = 0;
        while (host.is_robot_connected()) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            const auto now = std::chrono::steady_clock::now();
            const auto up = uplinkBytes.load();
            const auto down = downlinkBytes.load();
            std::cout << "pcm up=" << (up - lastUp) << " B/s, down=" << (down - lastDown) << " B/s\n";
            lastUp = up;
            lastDown = down;
            last = now;
            if (options.seconds > 0 && std::chrono::duration_cast<std::chrono::seconds>(now - start).count() >= options.seconds) {
                break;
            }
        }

        if (input) {
            input->stop();
        }
        if (output) {
            output->stop();
        }
        host.stop();
        return 0;
    } catch (const std::exception& exc) {
        std::cerr << exc.what() << "\n";
        return 1;
    }
}
