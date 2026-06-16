/*
Purpose:
    Run one BLE directional movement command.
Risk level:
    Medium. The robot will walk or step.
Run:
    .\build\Release\aidog_ble_directional_move.exe --address AA:BB:CC:DD:EE:FF --direction forward --duration 1 --yes
    .\build\Release\aidog_ble_directional_move.exe --prefix Gogobot --direction left --duration 1 --yes
Expected result:
    The robot moves in the selected direction, then stops.
Exit:
    Wait for the command to finish, or press Ctrl+C. Keep the robot on a flat open floor.
*/

#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>

#include <aidog.hpp>

namespace {

struct Options {
    aidog::ConnectOptions connect;
    aidog::Movement direction = aidog::Movement::Forward;
    double durationS = 1.0;
    bool yes = false;
};

aidog::Movement _parse_direction(const std::string& value)
{
    static const std::unordered_map<std::string, aidog::Movement> kDirections{
        {"forward", aidog::Movement::Forward},
        {"back", aidog::Movement::Back},
        {"backward", aidog::Movement::Back},
        {"left", aidog::Movement::Left},
        {"right", aidog::Movement::Right},
        {"step", aidog::Movement::Step},
    };
    auto found = kDirections.find(value);
    if (found == kDirections.end()) {
        throw std::invalid_argument("direction must be forward, back, left, right, or step");
    }
    return found->second;
}

Options _parse_options(int argc, char** argv)
{
    Options options;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--address" && i + 1 < argc) {
            options.connect.address = argv[++i];
        } else if ((arg == "--prefix" || arg == "--name-prefix") && i + 1 < argc) {
            options.connect.namePrefix = argv[++i];
        } else if (arg == "--direction" && i + 1 < argc) {
            options.direction = _parse_direction(argv[++i]);
        } else if (arg == "--duration" && i + 1 < argc) {
            options.durationS = std::stod(argv[++i]);
        } else if (arg == "--yes") {
            options.yes = true;
        }
    }
    if (options.durationS <= 0.0 || options.durationS > 10.0) {
        throw std::invalid_argument("duration must be 0-10 seconds");
    }
    return options;
}

} // namespace

int main(int argc, char** argv)
{
    try {
        auto options = _parse_options(argc, argv);
        if (!options.yes) {
            std::cerr << "This movement may move the robot. Pass --yes to run.\n";
            return 2;
        }

        aidog::AiDog dog;
        dog.connect(options.connect);

        try {
            dog.send_movement(options.direction, options.durationS);
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
