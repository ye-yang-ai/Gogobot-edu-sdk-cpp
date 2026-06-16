#include <iostream>
#include <optional>
#include <string>

#include <aidog.hpp>

namespace {

aidog::ConnectOptions _parse_options(int argc, char** argv)
{
    aidog::ConnectOptions options;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--address" && i + 1 < argc) {
            options.address = argv[++i];
        } else if (arg == "--prefix" && i + 1 < argc) {
            options.namePrefix = argv[++i];
        }
    }
    return options;
}

} // namespace

int main(int argc, char** argv)
{
    try {
        aidog::AiDog dog(false);
        auto options = _parse_options(argc, argv);
        dog.connect(options);
        std::cout << "connected=" << std::boolalpha << dog.is_connected() << "\n";
        dog.disconnect();
        std::cout << "disconnected\n";
        return 0;
    } catch (const std::exception& exc) {
        std::cerr << exc.what() << "\n";
        return 1;
    }
}
