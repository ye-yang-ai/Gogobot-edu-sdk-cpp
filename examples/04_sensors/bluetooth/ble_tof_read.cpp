#include <atomic>
#include <chrono>
#include <csignal>
#include <iostream>
#include <optional>
#include <string>
#include <thread>

#include <aidog.hpp>

namespace {

std::atomic_bool g_stop{false};

void _on_signal(int)
{
    g_stop.store(true);
}

aidog::ConnectOptions _parse_options(int argc, char** argv, int& hz, int& seconds)
{
    aidog::ConnectOptions options;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--address" && i + 1 < argc) {
            options.address = argv[++i];
        } else if (arg == "--prefix" && i + 1 < argc) {
            options.namePrefix = argv[++i];
        } else if (arg == "--hz" && i + 1 < argc) {
            hz = std::stoi(argv[++i]);
        } else if (arg == "--seconds" && i + 1 < argc) {
            seconds = std::stoi(argv[++i]);
        }
    }
    return options;
}

} // namespace

int main(int argc, char** argv)
{
    std::signal(SIGINT, _on_signal);
    int hz = 20;
    int seconds = 20;
    try {
        aidog::AiDog dog(false);
        auto options = _parse_options(argc, argv, hz, seconds);
        dog.add_tof_listener([](const aidog::TofData& tof) {
            std::cout << "tof " << tof.raw.dump() << "\n";
        });
        dog.connect(options);
        dog.request_tof_stream(true, hz);
        auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(seconds);
        while (!g_stop.load() && std::chrono::steady_clock::now() < deadline) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        dog.request_tof_stream(false);
        dog.disconnect();
        return 0;
    } catch (const std::exception& exc) {
        std::cerr << exc.what() << "\n";
        return 1;
    }
}
