/*
Purpose:
    Read TOF data through the Dev PC WebSocket host.
Risk level:
    Low. This example only requests and prints sensor data.
Run:
    .\build\Release\aidog_ws_tof_lan_read.exe --hz 20 --seconds 20
    .\build\Release\aidog_ws_tof_lan_read.exe --bind 0.0.0.0 --port 8766 --hz 50
Expected result:
    The host prints front and oblique TOF distance after the robot connects.
Exit:
    Press Ctrl+C, or set --seconds to stop automatically.
*/

#include <atomic>
#include <chrono>
#include <csignal>
#include <iostream>
#include <string>
#include <thread>

#include <aidog.hpp>

namespace {

std::atomic_bool g_stop{false};

struct Options {
    std::string bind = "0.0.0.0";
    int port = 8766;
    int hz = 20;
    double seconds = 0.0;
    double connectTimeoutS = 60.0;
    bool stopStreamOnExit = false;
};

void _handle_signal(int)
{
    g_stop.store(true);
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
        } else if (arg == "--hz" && i + 1 < argc) {
            options.hz = std::stoi(argv[++i]);
        } else if (arg == "--seconds" && i + 1 < argc) {
            options.seconds = std::stod(argv[++i]);
        } else if (arg == "--connect-timeout" && i + 1 < argc) {
            options.connectTimeoutS = std::stod(argv[++i]);
        } else if (arg == "--stop-stream-on-exit") {
            options.stopStreamOnExit = true;
        }
    }
    return options;
}

} // namespace

int main(int argc, char** argv)
{
    std::signal(SIGINT, _handle_signal);
    try {
        auto options = _parse_options(argc, argv);
        aidog::AiDog dog(false);
        dog.add_tof_listener([](const aidog::TofData& tof) {
            std::cout << "tof " << tof.raw.dump() << "\n";
        });

        aidog::WebSocketHost host(options.bind, options.port, &dog);
        host.set_connection_callback([](bool connected) {
            std::cout << (connected ? "robot connected\n" : "robot disconnected\n");
        });
        host.start();
        std::cout << "waiting for robot on ws://" << options.bind << ":" << options.port << "\n";
        if (!host.wait_robot_connected(options.connectTimeoutS)) {
            std::cerr << "timeout waiting for robot\n";
            return 1;
        }
        nlohmann::json config{{"cmd", "sensor_stream"}, {"tof", {{"enable", true}, {"hz", options.hz}}}};
        host.send_control_json(config);
        std::cout << "requested TOF stream hz=" << options.hz << "\n";

        const auto start = std::chrono::steady_clock::now();
        while (!g_stop.load()) {
            if (options.seconds > 0.0) {
                const auto elapsed = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
                if (elapsed >= options.seconds) {
                    break;
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        if (options.stopStreamOnExit) {
            try {
                nlohmann::json stopConfig{{"cmd", "sensor_stream"}, {"tof", {{"enable", false}}}};
                host.send_control_json(stopConfig);
            } catch (...) {
            }
        }
        host.stop();
        return 0;
    } catch (const std::exception& exc) {
        std::cerr << exc.what() << "\n";
        return 1;
    }
}
