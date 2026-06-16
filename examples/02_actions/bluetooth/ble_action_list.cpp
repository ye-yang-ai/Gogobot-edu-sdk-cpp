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
        dog.connect(_parse_options(argc, argv));
        auto actions = dog.get_action_list();
        std::cout << actions.dump(2) << "\n";
        dog.disconnect();
        return 0;
    } catch (const std::exception& exc) {
        std::cerr << exc.what() << "\n";
        return 1;
    }
}
