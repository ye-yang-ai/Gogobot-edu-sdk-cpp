/*
Purpose:
    Move the robot in one direction through WebSocket.
Risk level:
    High. The robot walks for the requested duration.
Run:
    .\build\Release\aidog_ws_directional_move.exe --direction forward --duration 1.0 --yes
    .\build\Release\aidog_ws_directional_move.exe --bind 0.0.0.0 --port 8766 --direction left --duration 0.8 --yes
Expected result:
    The robot moves briefly, stops, and prints move_done=true.
Exit:
    Keep the robot on a flat open floor while the command runs.
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
    std::string direction = "forward";
    double durationS = 1.0;
    double connectTimeoutS = 60.0;
    bool yes = false;
};

aidog::Movement _movement_from_name(const std::string& name)
{
    if (name == "forward") {
        return aidog::Movement::Forward;
    }
    if (name == "back" || name == "backward") {
        return aidog::Movement::Back;
    }
    if (name == "left") {
        return aidog::Movement::Left;
    }
    if (name == "right") {
        return aidog::Movement::Right;
    }
    if (name == "step") {
        return aidog::Movement::Step;
    }
    throw std::invalid_argument("unknown direction");
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
        } else if (arg == "--direction" && i + 1 < argc) {
            options.direction = argv[++i];
        } else if (arg == "--duration" && i + 1 < argc) {
            options.durationS = std::stod(argv[++i]);
        } else if (arg == "--connect-timeout" && i + 1 < argc) {
            options.connectTimeoutS = std::stod(argv[++i]);
        } else if (arg == "--yes") {
            options.yes = true;
        }
    }
    return options;
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
        dog.send_movement(_movement_from_name(options.direction), options.durationS, "ws");
        dog.reset("ws");
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        host.stop();
        std::cout << "move_done=true\n";
        return 0;
    } catch (const std::exception& exc) {
        std::cerr << exc.what() << "\n";
        return 1;
    }
}
