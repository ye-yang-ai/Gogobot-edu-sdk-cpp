#include <iostream>
#include <optional>
#include <string>

#include <aidog.hpp>

namespace {

struct Options {
    aidog::ConnectOptions connect;
    std::string action = "sit_down";
    std::optional<int> count;
    std::optional<int> duration;
    std::optional<int> angle;
    double timeoutS = 20.0;
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
        } else if ((arg == "--action" || arg == "--name") && i + 1 < argc) {
            options.action = argv[++i];
        } else if (arg == "--count" && i + 1 < argc) {
            options.count = std::stoi(argv[++i]);
        } else if (arg == "--duration" && i + 1 < argc) {
            options.duration = std::stoi(argv[++i]);
        } else if (arg == "--angle" && i + 1 < argc) {
            options.angle = std::stoi(argv[++i]);
        } else if (arg == "--timeout" && i + 1 < argc) {
            options.timeoutS = std::stod(argv[++i]);
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
            std::cerr << "This action may move the robot. Pass --yes to run.\n";
            return 2;
        }

        aidog::AiDog dog;
        dog.connect(options.connect);

        aidog::ActionOptions actionOptions;
        actionOptions.count = options.count;
        actionOptions.duration = options.duration;
        actionOptions.angle = options.angle;
        actionOptions.timeoutS = options.timeoutS;

        const bool ok = dog.perform_action(options.action, actionOptions);
        std::cout << "action_done=" << std::boolalpha << ok << "\n";
        dog.shutdown();
        return ok ? 0 : 2;
    } catch (const std::exception& exc) {
        std::cerr << exc.what() << "\n";
        return 1;
    }
}
