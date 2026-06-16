#include <algorithm>
#include <cctype>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>
#include <unordered_map>

#include <aidog.hpp>

namespace {

struct Options {
    aidog::ConnectOptions connect;
    aidog::ExpressionAction expression = aidog::ExpressionAction::Happy01;
    bool yes = false;
};

std::string _normalize(std::string text)
{
    std::transform(text.begin(), text.end(), text.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return text;
}

aidog::ExpressionAction _parse_expression(const std::string& value)
{
    static const std::unordered_map<std::string, aidog::ExpressionAction> aliases{
        {"idle", aidog::ExpressionAction::Idle},
        {"happy", aidog::ExpressionAction::Happy01},
        {"happy01", aidog::ExpressionAction::Happy01},
        {"smile", aidog::ExpressionAction::Smile01},
        {"love", aidog::ExpressionAction::Love01},
        {"angry", aidog::ExpressionAction::Anger01},
        {"sad", aidog::ExpressionAction::Sad01},
        {"scared", aidog::ExpressionAction::Scared01},
        {"comfortable", aidog::ExpressionAction::Comfortable01},
        {"doubt", aidog::ExpressionAction::Doubt01},
        {"nervous", aidog::ExpressionAction::Nervous},
        {"tired", aidog::ExpressionAction::Tired},
        {"sleepy", aidog::ExpressionAction::Sleepy},
        {"wink", aidog::ExpressionAction::WinkNormal},
        {"music", aidog::ExpressionAction::Music},
        {"eat_snack", aidog::ExpressionAction::EatSnack},
        {"charging", aidog::ExpressionAction::Charging},
        {"alert", aidog::ExpressionAction::Alert},
        {"sniff", aidog::ExpressionAction::SniffExpression},
        {"dead", aidog::ExpressionAction::Dead},
        {"yawn", aidog::ExpressionAction::Yawn},
    };
    auto key = _normalize(value);
    if (auto it = aliases.find(key); it != aliases.end()) {
        return it->second;
    }
    auto id = std::stoi(value);
    if (id < 0 || id > 255) {
        throw std::invalid_argument("expression id must be 0-255");
    }
    return static_cast<aidog::ExpressionAction>(static_cast<std::uint8_t>(id));
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
        } else if ((arg == "--expression" || arg == "--name") && i + 1 < argc) {
            options.expression = _parse_expression(argv[++i]);
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
            std::cerr << "This command changes expression. Pass --yes to run.\n";
            return 2;
        }

        aidog::AiDog dog(false);
        dog.connect(options.connect);
        dog.send_expression(options.expression);
        dog.disconnect();
        std::cout << "expression sent\n";
        return 0;
    } catch (const std::exception& exc) {
        std::cerr << exc.what() << "\n";
        return 1;
    }
}
