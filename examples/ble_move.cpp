#include <chrono>
#include <iostream>
#include <optional>
#include <string>
#include <thread>

#include <aidog.hpp>

namespace {

struct Options {
    aidog::ConnectOptions connect;
    std::string direction = "forward";
    double durationS = 1.0;
    bool yes = false;
};

aidog::Movement _parse_direction(const std::string& direction)
{
    if (direction == "forward") {
        return aidog::Movement::Forward;
    }
    if (direction == "back" || direction == "backward") {
        return aidog::Movement::Back;
    }
    if (direction == "left") {
        return aidog::Movement::Left;
    }
    if (direction == "right") {
        return aidog::Movement::Right;
    }
    if (direction == "step") {
        return aidog::Movement::Step;
    }
    throw std::invalid_argument("unsupported direction");
}

Options _parse_options(int argc, char** argv)
{
    Options options;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--address" && i + 1 < argc) {
            options.connect.address = argv[++i];
        } else if (arg == "--prefix" && i + 1 < argc) {
            options.connect.namePrefix = argv[++i];
        } else if (arg == "--direction" && i + 1 < argc) {
            options.direction = argv[++i];
        } else if (arg == "--duration" && i + 1 < argc) {
            options.durationS = std::stod(argv[++i]);
        } else if (arg == "--yes") {
            options.yes = true;
        }
    }
    return options;
}

} // namespace

int main(int argc, char** argv)
{
    aidog::AiDog dog;
    bool connected = false;
    try {
        auto options = _parse_options(argc, argv);
        if (!options.yes) {
            std::cerr << "This command moves the robot. Pass --yes to run.\n";
            return 2;
        }
        if (options.durationS <= 0.0 || options.durationS > 10.0) {
            throw std::invalid_argument("duration must be > 0 and <= 10 seconds");
        }

        auto direction = _parse_direction(options.direction);
        dog.connect(options.connect);
        connected = true;
        std::cout << "move direction=" << options.direction << " duration=" << options.durationS << "s\n";
        dog.send_movement(direction, options.durationS);
        dog.disconnect();
        connected = false;
        return 0;
    } catch (const std::exception& exc) {
        std::cerr << exc.what() << "\n";
        if (connected) {
            try {
                dog.stop_movement();
                dog.disconnect();
            } catch (...) {
            }
        }
        return 1;
    }
}
