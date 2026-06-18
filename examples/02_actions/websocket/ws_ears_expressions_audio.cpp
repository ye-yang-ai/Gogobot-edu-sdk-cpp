/*
Purpose:
    Test ears, expressions, audio, and volume through WebSocket.
Risk level:
    Low. This example does not command walking movement.
Run:
    .\build\Release\aidog_ws_ears_expressions_audio.exe
    .\build\Release\aidog_ws_ears_expressions_audio.exe --bind 0.0.0.0 --port 8766 --volume 3
Expected result:
    The robot changes ears, expression, and plays short audio cues.
Exit:
    The example resets audio and ear state before exit.
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
    int volume = 2;
    double connectTimeoutS = 60.0;
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
        } else if (arg == "--volume" && i + 1 < argc) {
            options.volume = std::stoi(argv[++i]);
        } else if (arg == "--connect-timeout" && i + 1 < argc) {
            options.connectTimeoutS = std::stod(argv[++i]);
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
        aidog::AiDog dog;
        aidog::WebSocketHost host(options.bind, options.port, &dog);
        host.start();
        std::cout << "waiting for robot on ws://" << options.bind << ":" << options.port << "\n";
        if (!host.wait_robot_connected(options.connectTimeoutS)) {
            std::cerr << "timeout waiting for robot\n";
            return 1;
        }

        dog.set_volume(options.volume, aidog::Tone::Agree, 0.2, "ws");
        _sleep_ms(800);
        dog.send_ear(aidog::EarAction::EarStandLeft, "ws");
        dog.send_expression(aidog::ExpressionAction::Happy01, "ws");
        dog.send_audio(aidog::Tone::Curious, "ws");
        _sleep_ms(1000);
        dog.send_ear_percentage(80, "ws");
        dog.send_expression(aidog::ExpressionAction::Love01, "ws");
        dog.send_audio(aidog::Tone::Jeez, "ws");
        _sleep_ms(1000);
        dog.send_ear(aidog::EarAction::EarDown, "ws");
        dog.send_expression(aidog::ExpressionAction::Idle, "ws");
        dog.send_audio(aidog::Tone::Stop, "ws");
        _sleep_ms(500);
        host.stop();

        std::cout << "ears_expressions_audio_done=true\n";
        return 0;
    } catch (const std::exception& exc) {
        std::cerr << exc.what() << "\n";
        return 1;
    }
}
