#include <algorithm>
#include <cctype>
#include <cstdint>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>

#include <aidog.hpp>

namespace {

struct Options {
    aidog::ConnectOptions connect;
    std::optional<aidog::EarAction> action = aidog::EarAction::EarStand;
    std::optional<int> percentage;
    bool yes = false;
};

std::string _normalize(std::string text)
{
    std::transform(text.begin(), text.end(), text.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return text;
}

aidog::EarAction _parse_ear_action(const std::string& value)
{
    static const std::unordered_map<std::string, aidog::EarAction> aliases{
        {"idle", aidog::EarAction::Idle},
        {"stand", aidog::EarAction::EarStand},
        {"stand_left", aidog::EarAction::EarStandLeft},
        {"stand_right", aidog::EarAction::EarStandRight},
        {"stand_both", aidog::EarAction::EarStandLeftAndRight},
        {"shake", aidog::EarAction::EarShakeSyn},
        {"shake_ble", aidog::EarAction::EarShakeSynForBle},
        {"breathe", aidog::EarAction::EarBreathe},
        {"down", aidog::EarAction::EarDown},
        {"flick", aidog::EarAction::EarFlickExcited},
        {"flick_left", aidog::EarAction::EarFlickLeftQuick},
        {"flick_right", aidog::EarAction::EarFlickRightQuick},
        {"flick_random", aidog::EarAction::EarFlickRandom},
    };
    auto key = _normalize(value);
    if (auto it = aliases.find(key); it != aliases.end()) {
        return it->second;
    }
    auto id = std::stoi(value);
    if (id < 0 || id > 255) {
        throw std::invalid_argument("ear action id must be 0-255");
    }
    return static_cast<aidog::EarAction>(static_cast<std::uint8_t>(id));
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
        } else if ((arg == "--action" || arg == "--ear") && i + 1 < argc) {
            options.action = _parse_ear_action(argv[++i]);
            options.percentage.reset();
        } else if (arg == "--percentage" && i + 1 < argc) {
            options.percentage = std::stoi(argv[++i]);
            options.action.reset();
        } else if (arg == "--yes") {
            options.yes = true;
        }
    }
    if (options.percentage.has_value() && (*options.percentage < 0 || *options.percentage > 100)) {
        throw std::invalid_argument("percentage must be 0-100");
    }
    return options;
}

} // namespace

int main(int argc, char** argv)
{
    try {
        auto options = _parse_options(argc, argv);
        if (!options.yes) {
            std::cerr << "This command moves ears. Pass --yes to run.\n";
            return 2;
        }

        aidog::AiDog dog(false);
        dog.connect(options.connect);
        if (options.percentage.has_value()) {
            dog.send_ear_percentage(*options.percentage);
            std::cout << "ear percentage set to " << *options.percentage << "\n";
        } else {
            dog.send_ear(*options.action);
            std::cout << "ear action sent\n";
        }
        dog.disconnect();
        return 0;
    } catch (const std::exception& exc) {
        std::cerr << exc.what() << "\n";
        return 1;
    }
}
