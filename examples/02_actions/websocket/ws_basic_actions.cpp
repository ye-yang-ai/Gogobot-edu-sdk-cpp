/*
Purpose:
    Run one high-level action through the Dev PC WebSocket host.
Risk level:
    Medium. Some actions move the robot.
Run:
    .\build\Release\aidog_ws_basic_actions.exe --action sit_down --yes
    .\build\Release\aidog_ws_basic_actions.exe --bind 0.0.0.0 --port 8766 --action shake_hand --count 2 --yes
    .\build\Release\aidog_ws_basic_actions.exe --action turn_right_angle --angle 90 --settle 0.6 --yes
Expected result:
    The selected action runs and prints action_done=true when completed.
Exit:
    Wait for the action to finish. Keep the robot on a flat open floor.
*/

#include <chrono>
#include <iostream>
#include <optional>
#include <string>
#include <thread>

#include <aidog.hpp>

namespace {

struct Options {
    std::string bind = "0.0.0.0";
    int port = 8766;
    std::string action = "sit_down";
    std::optional<int> count;
    std::optional<int> duration;
    std::optional<int> angle;
    double timeoutS = 20.0;
    double connectTimeoutS = 60.0;
    double readyTimeoutS = 5.0;
    double settleS = 0.6;
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
        } else if ((arg == "--action" || arg == "--name") && i + 1 < argc) {
            options.action = argv[++i];
        } else if (arg == "--count" && i + 1 < argc) {
            options.count = std::stoi(argv[++i]);
        } else if (arg == "--duration" && i + 1 < argc) {
            options.duration = std::stoi(argv[++i]);
        } else if (arg == "--angle" && i + 1 < argc) {
            options.angle = std::stoi(argv[++i]);
        } else if (arg == "--timeout" && i + 1 < argc) {
            options.timeoutS = std::stod(argv[++i]);
        } else if (arg == "--connect-timeout" && i + 1 < argc) {
            options.connectTimeoutS = std::stod(argv[++i]);
        } else if (arg == "--ready-timeout" && i + 1 < argc) {
            options.readyTimeoutS = std::stod(argv[++i]);
        } else if (arg == "--settle" && i + 1 < argc) {
            options.settleS = std::stod(argv[++i]);
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
            std::cerr << "This action may move the robot. Pass --yes to run.\n";
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
        std::cout << "robot connected\n";
        if (!dog.wait_interaction_ready(options.readyTimeoutS)) {
            std::cerr << "timeout waiting for interaction status ready\n";
            return 1;
        }
        std::cout << "interaction status ready\n";
        if (options.settleS > 0.0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(options.settleS * 1000.0)));
        }

        aidog::ActionOptions actionOptions;
        actionOptions.count = options.count;
        actionOptions.duration = options.duration;
        actionOptions.angle = options.angle;
        actionOptions.timeoutS = options.timeoutS;
        actionOptions.transport = "ws";
        actionOptions.requireRunningState = false;

        const bool ok = dog.perform_action(options.action, actionOptions);
        std::cout << "action_done=" << std::boolalpha << ok << "\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        host.stop();
        return ok ? 0 : 2;
    } catch (const std::exception& exc) {
        std::cerr << exc.what() << "\n";
        return 1;
    }
}
