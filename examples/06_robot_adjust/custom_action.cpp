/*
Purpose:
    Run a custom BLE sniff-like action built from robot-adjustment APIs.
Risk level:
    High. This example changes foot targets and plays expression/audio.
Run:
    .\build\Release\aidog_custom_action.exe --address AA:BB:CC:DD:EE:FF --hold 4 --yes
    .\build\Release\aidog_custom_action.exe --prefix Gogobot --hold 4
Expected result:
    The robot performs the custom sniff motion, then returns to basic mode.
Exit:
    Keep hands near the robot. Without --yes, type RUN to confirm.
*/

#include <chrono>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include <aidog.hpp>

namespace {

struct Options {
    aidog::ConnectOptions connect;
    bool yes = false;
    double holdS = 4.0;
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
        } else if (arg == "--hold" && i + 1 < argc) {
            options.holdS = std::stod(argv[++i]);
        } else if (arg == "--yes") {
            options.yes = true;
        }
    }
    if (options.holdS < 0.0 || options.holdS > 10.0) {
        throw std::invalid_argument("hold must be 0-10 seconds");
    }
    return options;
}

void _require_confirmation(bool yes)
{
    if (yes) {
        return;
    }
    std::cerr << "HIGH RISK: custom robot adjustment will move feet. Type RUN to continue: ";
    std::string answer;
    std::getline(std::cin, answer);
    if (answer != "RUN") {
        throw std::runtime_error("cancelled");
    }
}

void _sleep_ms(int ms)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

void _sleep_s(double seconds)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(seconds * 1000.0)));
}

void _run_sniff_motion(aidog::AiDog& dog, double holdS)
{
    dog.request_basic_mode();
    _sleep_ms(400);
    dog.default_pose_output(0.0F, 0.0F, 5.0F, 110.0F);
    _sleep_ms(400);

    dog.syn_foot_adjust(
        std::vector<aidog::DeltaAdjustItem>{
            {"foot_0_x", 25.0F},
            {"foot_2_x", 25.0F},
            {"foot_1_x", 25.0F},
            {"foot_3_x", 25.0F},
        },
        600);
    _sleep_ms(800);

    dog.syn_foot_adjust(
        std::vector<aidog::DeltaAdjustItem>{
            {"foot_0_z", 40.0F},
            {"foot_2_z", 40.0F},
            {"foot_0_x", -20.0F},
            {"foot_2_x", -20.0F},
        },
        600);
    _sleep_ms(700);

    dog.send_expression(aidog::ExpressionAction::EatSnack);
    dog.send_audio(aidog::Tone::Eating);
    _sleep_s(holdS);
    dog.send_audio(aidog::Tone::Stop);

    dog.syn_foot_adjust(
        std::vector<aidog::DeltaAdjustItem>{
            {"foot_0_z", -40.0F},
            {"foot_2_z", -40.0F},
            {"foot_0_x", 20.0F},
            {"foot_2_x", 20.0F},
        },
        600);
    _sleep_ms(800);

    dog.syn_foot_adjust(
        std::vector<aidog::DeltaAdjustItem>{
            {"foot_0_x", -25.0F},
            {"foot_2_x", -25.0F},
            {"foot_1_x", -25.0F},
            {"foot_3_x", -25.0F},
        },
        600);
    _sleep_ms(800);

    dog.request_basic_mode();
}

} // namespace

int main(int argc, char** argv)
{
    try {
        auto options = _parse_options(argc, argv);
        _require_confirmation(options.yes);

        aidog::AiDog dog(false);
        dog.connect(options.connect);

        try {
            dog.disable_special_detection();
            _run_sniff_motion(dog, options.holdS);
            std::cout << "[custom] sniff motion completed\n";
        } catch (...) {
            try {
                dog.send_audio(aidog::Tone::Stop);
                dog.request_basic_mode();
                dog.enable_special_detection();
                dog.reset();
            } catch (...) {
            }
            dog.disconnect();
            throw;
        }

        dog.send_audio(aidog::Tone::Stop);
        dog.request_basic_mode();
        dog.enable_special_detection();
        dog.reset();
        dog.disconnect();
        return 0;
    } catch (const std::exception& exc) {
        std::cerr << exc.what() << "\n";
        return 1;
    }
}
