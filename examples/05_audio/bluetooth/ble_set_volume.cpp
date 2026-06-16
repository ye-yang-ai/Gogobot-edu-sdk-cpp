#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>

#include <aidog.hpp>

namespace {

aidog::ConnectOptions _parse_options(int argc, char** argv, int& volume, bool& verify)
{
    aidog::ConnectOptions options;
    volume = 3;
    verify = true;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--address" && i + 1 < argc) {
            options.address = argv[++i];
        } else if (arg == "--prefix" && i + 1 < argc) {
            options.namePrefix = argv[++i];
        } else if (arg == "--volume" && i + 1 < argc) {
            volume = std::stoi(argv[++i]);
        } else if (arg == "--no-verify") {
            verify = false;
        }
    }
    if (volume < 0 || volume > 4) {
        throw std::invalid_argument("volume must be 0-4");
    }
    return options;
}

} // namespace

int main(int argc, char** argv)
{
    int volume = 3;
    bool verify = true;
    try {
        aidog::AiDog dog(false);
        auto options = _parse_options(argc, argv, volume, verify);
        dog.connect(options);
        dog.set_volume(volume, verify ? std::optional<aidog::Tone>(aidog::Tone::Beat1) : std::nullopt);
        std::cout << "volume set to " << volume << "\n";
        dog.disconnect();
        return 0;
    } catch (const std::exception& exc) {
        std::cerr << exc.what() << "\n";
        return 1;
    }
}
