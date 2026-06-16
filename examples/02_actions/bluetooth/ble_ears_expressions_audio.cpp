/*
Purpose:
    Run a BLE demo for volume, audio, expression, ears, and special detection.
Risk level:
    Low/Medium. Ears and expression change; special detection is toggled briefly.
Run:
    .\build\Release\aidog_ble_ears_expressions_audio.exe --address AA:BB:CC:DD:EE:FF --yes
    .\build\Release\aidog_ble_ears_expressions_audio.exe --prefix Gogobot --yes
Expected result:
    The robot plays one tone, changes expression, moves ears, and restores special detection.
Exit:
    Wait for the demo to finish.
*/

#include <chrono>
#include <iostream>
#include <optional>
#include <string>
#include <thread>

#include <aidog.hpp>

namespace {

aidog::ConnectOptions _parse_options(int argc, char** argv, bool& runDemo)
{
    aidog::ConnectOptions options;
    runDemo = false;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--address" && i + 1 < argc) {
            options.address = argv[++i];
        } else if ((arg == "--prefix" || arg == "--name-prefix") && i + 1 < argc) {
            options.namePrefix = argv[++i];
        } else if (arg == "--yes") {
            runDemo = true;
        }
    }
    return options;
}

} // namespace

int main(int argc, char** argv)
{
    bool runDemo = false;
    try {
        auto options = _parse_options(argc, argv, runDemo);
        if (!runDemo) {
            std::cerr << "This demo changes ears/expression/audio. Pass --yes to run.\n";
            return 2;
        }

        aidog::AiDog dog;
        dog.connect(options);

        std::cout << "set volume\n";
        dog.set_volume(3);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        std::cout << "play tone\n";
        dog.send_audio(aidog::Tone::Jeez);
        std::this_thread::sleep_for(std::chrono::seconds(1));

        std::cout << "happy expression\n";
        dog.send_expression(aidog::ExpressionAction::Happy01);
        std::this_thread::sleep_for(std::chrono::seconds(1));

        std::cout << "ear stand\n";
        dog.send_ear(aidog::EarAction::EarStand);
        std::this_thread::sleep_for(std::chrono::seconds(1));

        std::cout << "ear percentage 50\n";
        dog.send_ear_percentage(50);
        std::this_thread::sleep_for(std::chrono::seconds(1));

        std::cout << "disable special detection\n";
        dog.disable_special_detection();
        std::this_thread::sleep_for(std::chrono::milliseconds(300));

        std::cout << "enable special detection\n";
        dog.enable_special_detection();

        dog.disconnect();
        return 0;
    } catch (const std::exception& exc) {
        std::cerr << exc.what() << "\n";
        return 1;
    }
}
