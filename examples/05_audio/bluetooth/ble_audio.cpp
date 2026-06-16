#include <algorithm>
#include <chrono>
#include <cctype>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>
#include <unordered_map>

#include <aidog.hpp>

namespace {

struct Options {
    aidog::ConnectOptions connect;
    aidog::Tone tone = aidog::Tone::Jeez;
    double durationS = 0.0;
    bool yes = false;
};

std::string _normalize(std::string text)
{
    std::transform(text.begin(), text.end(), text.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return text;
}

aidog::Tone _parse_tone(const std::string& value)
{
    static const std::unordered_map<std::string, aidog::Tone> aliases{
        {"stop", aidog::Tone::Stop},
        {"jeez", aidog::Tone::Jeez},
        {"uh", aidog::Tone::Uh},
        {"eating", aidog::Tone::Eating},
        {"charging", aidog::Tone::Charging},
        {"curious", aidog::Tone::Curious},
        {"sleepy", aidog::Tone::Sleepy},
        {"sad", aidog::Tone::Sad},
        {"angry", aidog::Tone::Angry},
        {"doubt", aidog::Tone::Doubt},
        {"agree", aidog::Tone::Agree},
        {"alert", aidog::Tone::Alert},
        {"wake_up", aidog::Tone::WakeUp},
        {"comfort", aidog::Tone::Comfort},
        {"snore", aidog::Tone::Snore},
        {"sniff", aidog::Tone::Sniff},
        {"beat1", aidog::Tone::Beat1},
        {"beat2", aidog::Tone::Beat2},
        {"beat3", aidog::Tone::Beat3},
        {"beat4", aidog::Tone::Beat4},
        {"beat5", aidog::Tone::Beat5},
        {"beat6", aidog::Tone::Beat6},
        {"beat7", aidog::Tone::Beat7},
    };
    auto key = _normalize(value);
    if (auto it = aliases.find(key); it != aliases.end()) {
        return it->second;
    }
    auto id = std::stoi(value);
    if (id < 0 || id > 255) {
        throw std::invalid_argument("tone id must be 0-255");
    }
    return static_cast<aidog::Tone>(static_cast<std::uint8_t>(id));
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
        } else if ((arg == "--tone" || arg == "--audio") && i + 1 < argc) {
            options.tone = _parse_tone(argv[++i]);
        } else if (arg == "--duration" && i + 1 < argc) {
            options.durationS = std::stod(argv[++i]);
        } else if (arg == "--yes") {
            options.yes = true;
        }
    }
    if (options.durationS < 0.0 || options.durationS > 30.0) {
        throw std::invalid_argument("duration must be 0-30 seconds");
    }
    return options;
}

} // namespace

int main(int argc, char** argv)
{
    try {
        auto options = _parse_options(argc, argv);
        if (!options.yes) {
            std::cerr << "This command plays audio. Pass --yes to run.\n";
            return 2;
        }

        aidog::AiDog dog(false);
        dog.connect(options.connect);
        dog.send_audio(options.tone);
        if (options.durationS > 0.0 && options.tone != aidog::Tone::Stop) {
            std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(options.durationS * 1000.0)));
            dog.send_audio(aidog::Tone::Stop);
        }
        dog.disconnect();
        std::cout << "audio command sent\n";
        return 0;
    } catch (const std::exception& exc) {
        std::cerr << exc.what() << "\n";
        return 1;
    }
}
