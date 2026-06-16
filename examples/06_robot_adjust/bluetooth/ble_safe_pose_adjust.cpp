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
};

Options _parse_options(int argc, char** argv)
{
    Options options;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--address" && i + 1 < argc) {
            options.connect.address = argv[++i];
        } else if (arg == "--prefix" && i + 1 < argc) {
            options.connect.namePrefix = argv[++i];
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
    std::cerr << "HIGH RISK: robot adjustment will move body and feet. Type RUN to continue: ";
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

} // namespace

int main(int argc, char** argv)
{
    auto options = _parse_options(argc, argv);
    try {
        _require_confirmation(options.yes);

        aidog::AiDog dog(false);
        dog.connect(options.connect);

        try {
            std::cout << "[adjust] request basic mode\n";
            dog.request_basic_mode();
            dog.disable_special_detection();
            _sleep_ms(400);

            std::cout << "[adjust] baseline pose\n";
            dog.default_pose_output(0.0F, 0.0F, 5.0F, 110.0F);
            _sleep_ms(500);

            std::cout << "[adjust] lower cog\n";
            std::vector<aidog::PoseAdjustItem> lowerCog{{"cog_z", 110.0F, 55.0F}};
            dog.syn_pose_adjust(lowerCog, 1000);
            _sleep_ms(1000);

            std::cout << "[adjust] restore cog\n";
            std::vector<aidog::PoseAdjustItem> restoreCog{{"cog_z", 55.0F, 110.0F}};
            dog.syn_pose_adjust(restoreCog, 1000);
            _sleep_ms(1000);

            std::cout << "[adjust] small foot x shift\n";
            std::vector<aidog::DeltaAdjustItem> forward{
                {"foot_0_x", 30.0F},
                {"foot_1_x", 30.0F},
                {"foot_2_x", 30.0F},
                {"foot_3_x", 30.0F},
            };
            dog.syn_foot_adjust(forward, 500);
            _sleep_ms(1000);

            std::vector<aidog::DeltaAdjustItem> backward{
                {"foot_0_x", -30.0F},
                {"foot_1_x", -30.0F},
                {"foot_2_x", -30.0F},
                {"foot_3_x", -30.0F},
            };
            dog.syn_foot_adjust(backward, 500);
            _sleep_ms(1000);

            std::cout << "[adjust] complete\n";
        } catch (...) {
            try {
                dog.request_basic_mode();
                dog.enable_special_detection();
                dog.stop_movement();
            } catch (...) {
            }
            dog.disconnect();
            throw;
        }

        dog.request_basic_mode();
        dog.enable_special_detection();
        dog.stop_movement();
        dog.disconnect();
        return 0;
    } catch (const std::exception& exc) {
        std::cerr << exc.what() << "\n";
        return 1;
    }
}
