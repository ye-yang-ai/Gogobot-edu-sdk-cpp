/*
Purpose:
    Run a timed BLE movement sequence.
Risk level:
    Medium. The robot will walk and step.
Run:
    .\build\Release\aidog_ble_timed_move.exe --address AA:BB:CC:DD:EE:FF --duration 1 --pause 1 --yes
    .\build\Release\aidog_ble_timed_move.exe --prefix Gogobot --duration 0.8 --pause 0.5 --yes
Expected result:
    The robot runs forward, right, back, left, and step movements with pauses.
Exit:
    Wait for the sequence to finish, or press Ctrl+C. Keep the robot on a flat open floor.
*/

#include <chrono>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include <aidog.hpp>

namespace {

struct Options {
    aidog::ConnectOptions connect;
    double durationS = 1.0;
    double pauseS = 1.0;
    bool yes = false;
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
        } else if (arg == "--duration" && i + 1 < argc) {
            options.durationS = std::stod(argv[++i]);
        } else if (arg == "--pause" && i + 1 < argc) {
            options.pauseS = std::stod(argv[++i]);
        } else if (arg == "--yes") {
            options.yes = true;
        }
    }
    if (options.durationS <= 0.0 || options.durationS > 10.0) {
        throw std::invalid_argument("duration must be 0-10 seconds");
    }
    if (options.pauseS < 0.0 || options.pauseS > 10.0) {
        throw std::invalid_argument("pause must be 0-10 seconds");
    }
    return options;
}

void _sleep_s(double seconds)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(seconds * 1000.0)));
}

} // namespace

int main(int argc, char** argv)
{
    try {
        auto options = _parse_options(argc, argv);
        if (!options.yes) {
            std::cerr << "This movement sequence may move the robot. Pass --yes to run.\n";
            return 2;
        }

        aidog::AiDog dog;
        dog.connect(options.connect);

        try {
            const std::vector<aidog::Movement> sequence{
                aidog::Movement::Forward,
                aidog::Movement::Right,
                aidog::Movement::Back,
                aidog::Movement::Left,
                aidog::Movement::Step,
            };
            for (auto movement : sequence) {
                dog.send_movement(movement, options.durationS);
                dog.stop_movement();
                _sleep_s(options.pauseS);
            }
        } catch (...) {
            try {
                dog.stop_movement();
                dog.reset();
            } catch (...) {
            }
            dog.disconnect();
            throw;
        }

        dog.stop_movement();
        dog.reset();
        dog.disconnect();
        return 0;
    } catch (const std::exception& exc) {
        std::cerr << exc.what() << "\n";
        return 1;
    }
}
