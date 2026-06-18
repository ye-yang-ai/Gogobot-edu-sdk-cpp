/*
Purpose:
    Run a timed movement sequence through WebSocket.
Risk level:
    High. The robot walks in multiple directions.
Run:
    .\build\Release\aidog_ws_timed_move.exe --duration 0.8 --pause 0.4 --yes
Expected result:
    The robot moves forward, right, back, left, then stops.
Exit:
    Keep the robot on a flat open floor while the sequence runs.
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
    double durationS = 0.8;
    double pauseS = 0.4;
    double connectTimeoutS = 60.0;
    bool yes = false;
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
        } else if (arg == "--duration" && i + 1 < argc) {
            options.durationS = std::stod(argv[++i]);
        } else if (arg == "--pause" && i + 1 < argc) {
            options.pauseS = std::stod(argv[++i]);
        } else if (arg == "--connect-timeout" && i + 1 < argc) {
            options.connectTimeoutS = std::stod(argv[++i]);
        } else if (arg == "--yes") {
            options.yes = true;
        }
    }
    return options;
}

void _pause(double seconds)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(seconds * 1000.0)));
}

} // namespace

int main(int argc, char** argv)
{
    try {
        auto options = _parse_options(argc, argv);
        if (!options.yes) {
            std::cerr << "This command moves the robot. Pass --yes to run.\n";
            return 2;
        }

        aidog::AiDog dog;
        aidog::WebSocketHost host(options.bind, options.port, &dog);
        host.start();
        std::cout << "waiting for robot on ws://" << options.bind << ":" << options.port << "\n";
        if (!host.wait_robot_connected(options.connectTimeoutS)) {
            std::cerr << "timeout waiting for robot\n";
            return 1;
        }

        const aidog::Movement sequence[] = {
            aidog::Movement::Forward,
            aidog::Movement::Right,
            aidog::Movement::Back,
            aidog::Movement::Left,
        };
        for (const auto movement : sequence) {
            dog.send_movement(movement, options.durationS, "ws");
            _pause(options.pauseS);
        }
        dog.reset("ws");
        _pause(0.5);
        host.stop();

        std::cout << "timed_move_done=true\n";
        return 0;
    } catch (const std::exception& exc) {
        std::cerr << exc.what() << "\n";
        return 1;
    }
}
