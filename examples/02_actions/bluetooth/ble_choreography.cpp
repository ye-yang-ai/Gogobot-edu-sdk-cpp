/*
Purpose:
    Run a small BLE choreography with ears, expression, audio, actions, and movement.
Risk level:
    Medium. The robot will move.
Run:
    .\build\Release\aidog_ble_choreography.exe --address AA:BB:CC:DD:EE:FF --yes
    .\build\Release\aidog_ble_choreography.exe --prefix Gogobot --yes
Expected result:
    The robot performs a short combined demo, then returns to a safe state.
Exit:
    Wait for the demo to finish, or press Ctrl+C. Keep the robot on a flat open floor.
*/

#include <chrono>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>

#include <aidog.hpp>

namespace {

struct Options {
    aidog::ConnectOptions connect;
    bool yes = false;
    double timeoutS = 20.0;
};

Options _parse_options(int argc, char** argv)
{
    Options options;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--address" && i + 1 < argc) {
            options.connect.address = argv[++i];
        } else if ((arg == "--prefix" || arg == "--name-prefix") && i + 1 < argc) {
            options.connect.namePrefix = argv[++i];
        } else if (arg == "--timeout" && i + 1 < argc) {
            options.timeoutS = std::stod(argv[++i]);
        } else if (arg == "--yes") {
            options.yes = true;
        }
    }
    return options;
}

void _require_confirmation(bool yes)
{
    if (yes) {
        return;
    }
    std::cerr << "This choreography moves the robot. Pass --yes to run.\n";
    throw std::runtime_error("cancelled");
}

void _sleep_ms(int ms)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

void _run_action(aidog::AiDog& dog, aidog::Action action, const aidog::ActionOptions& options = {})
{
    const bool ok = dog.perform_action(action, options);
    std::cout << "action_done=" << std::boolalpha << ok << "\n";
}

} // namespace

int main(int argc, char** argv)
{
    try {
        auto options = _parse_options(argc, argv);
        _require_confirmation(options.yes);

        aidog::AiDog dog;
        dog.connect(options.connect);

        try {
            dog.disable_special_detection();

            std::cout << "[demo] ears + expression + audio\n";
            dog.send_ear(aidog::EarAction::EarStand);
            dog.send_expression(aidog::ExpressionAction::Yawn);
            dog.send_audio(aidog::Tone::Beat1);
            _sleep_ms(800);

            aidog::ActionOptions countOptions;
            countOptions.count = 2;
            countOptions.timeoutS = options.timeoutS;
            _run_action(dog, aidog::Action::ShakeHand, countOptions);
            _run_action(dog, aidog::Action::Nod, countOptions);

            std::cout << "[demo] forward movement\n";
            dog.send_movement(aidog::Movement::Forward, 2.0);
            _sleep_ms(300);

            aidog::ActionOptions angleOptions;
            angleOptions.angle = 90;
            angleOptions.timeoutS = options.timeoutS;
            _run_action(dog, aidog::Action::RightAngleInteraction, angleOptions);

            countOptions.count = 2;
            _run_action(dog, aidog::Action::WagTail, countOptions);
        } catch (...) {
            try {
                dog.send_audio(aidog::Tone::Stop);
                dog.stop_movement();
                dog.enable_special_detection();
                dog.reset();
            } catch (...) {
            }
            dog.disconnect();
            throw;
        }

        dog.send_audio(aidog::Tone::Stop);
        dog.stop_movement();
        dog.enable_special_detection();
        dog.reset();
        dog.disconnect();
        return 0;
    } catch (const std::exception& exc) {
        std::cerr << exc.what() << "\n";
        return 1;
    }
}
