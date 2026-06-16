#include <algorithm>
#include <cctype>
#include <iostream>
#include <stdexcept>
#include <string>

#include <aidog.hpp>

namespace {

struct Options {
    aidog::ConnectOptions connect;
    bool enable = true;
    bool yes = false;
};

std::string _normalize(std::string text)
{
    std::transform(text.begin(), text.end(), text.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return text;
}

bool _parse_state(const std::string& value)
{
    auto key = _normalize(value);
    if (key == "enable" || key == "on" || key == "true" || key == "1") {
        return true;
    }
    if (key == "disable" || key == "off" || key == "false" || key == "0") {
        return false;
    }
    throw std::invalid_argument("state must be enable or disable");
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
        } else if (arg == "--state" && i + 1 < argc) {
            options.enable = _parse_state(argv[++i]);
        } else if (arg == "--enable") {
            options.enable = true;
        } else if (arg == "--disable") {
            options.enable = false;
        } else if (arg == "--yes") {
            options.yes = true;
        }
    }
    return options;
}

} // namespace

int main(int argc, char** argv)
{
    try {
        auto options = _parse_options(argc, argv);
        if (!options.yes) {
            std::cerr << "This command changes special detection state. Pass --yes to run.\n";
            return 2;
        }

        aidog::AiDog dog(false);
        dog.connect(options.connect);
        dog.set_special_detection(options.enable);
        dog.disconnect();
        std::cout << "special detection " << (options.enable ? "enabled" : "disabled") << "\n";
        return 0;
    } catch (const std::exception& exc) {
        std::cerr << exc.what() << "\n";
        return 1;
    }
}
