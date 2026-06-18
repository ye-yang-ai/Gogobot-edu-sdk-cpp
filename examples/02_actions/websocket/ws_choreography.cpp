/*
Purpose:
    Run a short WebSocket choreography with actions, ears, expressions, and audio.
Risk level:
    High. The robot performs several movements.
Run:
    .\build\Release\aidog_ws_choreography.exe --yes
    .\build\Release\aidog_ws_choreography.exe --bind 0.0.0.0 --port 8766 --yes
Expected result:
    The robot completes the short choreography and prints choreography_done=true.
Exit:
    Keep the robot on a flat open floor until the sequence finishes.
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
        } else if (arg == "--connect-timeout" && i + 1 < argc) {
            options.connectTimeoutS = std::stod(argv[++i]);
        } else if (arg == "--yes") {
            options.yes = true;
        }
    }
    return options;
}

void _sleep_ms(int ms)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

} // namespace

int main(int argc, char** argv)
{
    try {
        auto options = _parse_options(argc, argv);
        if (!options.yes) {
            std::cerr << "This choreography moves the robot. Pass --yes to run.\n";
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

        aidog::ActionOptions actionOptions;
        actionOptions.transport = "ws";
        actionOptions.timeoutS = 25.0;

        dog.disable_special_detection("ws");
        dog.send_expression(aidog::ExpressionAction::Happy01, "ws");
        dog.send_audio(aidog::Tone::WakeUp, "ws");
        dog.send_ear(aidog::EarAction::EarStandLeftAndRight, "ws");
        _sleep_ms(800);
        dog.perform_action(aidog::Action::StandUp, actionOptions);
        dog.perform_action(aidog::Action::ShakeHand, actionOptions);
        dog.send_expression(aidog::ExpressionAction::Love01, "ws");
        dog.send_audio(aidog::Tone::Agree, "ws");
        dog.perform_action(aidog::Action::Dance, actionOptions);
        dog.send_ear(aidog::EarAction::EarBreathe, "ws");
        _sleep_ms(800);
        dog.send_audio(aidog::Tone::Stop, "ws");
        dog.enable_special_detection("ws");
        dog.reset("ws");
        _sleep_ms(500);
        host.stop();

        std::cout << "choreography_done=true\n";
        return 0;
    } catch (const std::exception& exc) {
        std::cerr << exc.what() << "\n";
        return 1;
    }
}
