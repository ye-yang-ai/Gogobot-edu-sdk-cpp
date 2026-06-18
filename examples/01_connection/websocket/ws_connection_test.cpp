/*
Purpose:
    Start the Dev PC WebSocket host and wait for the robot to connect.
Risk level:
    Low. This example only accepts a WebSocket connection.
Run:
    .\build\Release\aidog_ws_connection_test.exe --bind 0.0.0.0 --port 8766
Expected result:
    The robot connects and the tool prints robot_connected=true.
Exit:
    Press Ctrl+C after the robot connects.
*/

#include <chrono>
#include <iostream>
#include <string>
#include <thread>

#include <aidog.hpp>

namespace {

struct Options {
    std::string bind = "0.0.0.0";
    int port = 8766;
    double timeoutS = 60.0;
    bool keepAlive = true;
};

Options _parse_options(int argc, char** argv)
{
    Options options;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--bind" && i + 1 < argc) {
            options.bind = argv[++i];
        } else if (arg == "--port" && i + 1 < argc) {
            options.port = std::stoi(argv[++i]);
        } else if (arg == "--timeout" && i + 1 < argc) {
            options.timeoutS = std::stod(argv[++i]);
        } else if (arg == "--no-keep-alive") {
            options.keepAlive = false;
        }
    }
    return options;
}

} // namespace

int main(int argc, char** argv)
{
    try {
        auto options = _parse_options(argc, argv);
        aidog::AiDog dog;
        aidog::WebSocketHost host(options.bind, options.port, &dog);
        host.start();
        std::cout << "waiting for robot on ws://" << options.bind << ":" << options.port << "\n";
        const bool ok = host.wait_robot_connected(options.timeoutS);
        std::cout << "robot_connected=" << std::boolalpha << ok << "\n";
        while (ok && options.keepAlive) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        host.stop();
        return ok ? 0 : 1;
    } catch (const std::exception& exc) {
        std::cerr << exc.what() << "\n";
        return 1;
    }
}
