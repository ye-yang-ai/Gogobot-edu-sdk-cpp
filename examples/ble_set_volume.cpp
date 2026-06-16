#include <iostream>
#include <optional>
#include <string>

#include <aidog.hpp>

namespace {

aidog::ConnectOptions _parse_options(int argc, char** argv, int& volume)
{
    aidog::ConnectOptions options;
    volume = 3;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--address" && i + 1 < argc) {
            options.address = argv[++i];
        } else if (arg == "--prefix" && i + 1 < argc) {
            options.namePrefix = argv[++i];
        } else if (arg == "--volume" && i + 1 < argc) {
            volume = std::stoi(argv[++i]);
        }
    }
    return options;
}

} // namespace

int main(int argc, char** argv)
{
    int volume = 3;
    try {
        aidog::AiDog dog(false);
        auto options = _parse_options(argc, argv, volume);
        dog.connect(options);
        dog.set_volume(volume, aidog::Tone::Beat1);
        dog.disconnect();
        return 0;
    } catch (const std::exception& exc) {
        std::cerr << exc.what() << "\n";
        return 1;
    }
}
