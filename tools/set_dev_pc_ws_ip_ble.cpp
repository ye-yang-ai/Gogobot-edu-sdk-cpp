/*
Purpose:
    Configure the robot Dev PC WebSocket IP through BLE.
Risk level:
    Low. This writes only the PC IP used by the robot WebSocket client.
Run:
    .\build\Release\aidog_set_dev_pc_ws_ip_ble.exe --address AA:BB:CC:DD:EE:FF 192.168.11.23
    .\build\Release\aidog_set_dev_pc_ws_ip_ble.exe --prefix Gogobot 192.168.11.23
Expected result:
    The tool prints configured dev_pc_ip=<ip>.
Exit:
    The tool disconnects automatically after writing the configuration.
*/

#include <iostream>
#include <optional>
#include <sstream>
#include <string>

#include <aidog.hpp>

namespace {

struct Options {
    aidog::ConnectOptions connect;
    std::string ip;
};

bool _is_ipv4(const std::string& ip)
{
    std::istringstream stream(ip);
    std::string part;
    int count = 0;
    while (std::getline(stream, part, '.')) {
        if (part.empty() || part.size() > 3) {
            return false;
        }
        for (const auto ch : part) {
            if (ch < '0' || ch > '9') {
                return false;
            }
        }
        const int value = std::stoi(part);
        if (value < 0 || value > 255) {
            return false;
        }
        ++count;
    }
    return count == 4;
}

Options _parse_options(int argc, char** argv)
{
    Options options;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--address" && i + 1 < argc) {
            options.connect.address = argv[++i];
        } else if ((arg == "--prefix" || arg == "--name-prefix") && i + 1 < argc) {
            options.connect.namePrefix = argv[++i];
        } else if (!arg.starts_with("--") && options.ip.empty()) {
            options.ip = arg;
        }
    }
    return options;
}

} // namespace

int main(int argc, char** argv)
{
    try {
        auto options = _parse_options(argc, argv);
        if (!_is_ipv4(options.ip)) {
            std::cerr << "Usage: aidog_set_dev_pc_ws_ip_ble.exe [--address AA:BB:CC:DD:EE:FF | --prefix Gogobot] 192.168.11.23\n";
            return 2;
        }

        aidog::AiDog dog(false);
        dog.connect(options.connect);
        dog.set_dev_pc_ws_ip(options.ip);
        dog.disconnect();

        std::cout << "configured dev_pc_ip=" << options.ip << "\n";
        return 0;
    } catch (const std::exception& exc) {
        std::cerr << exc.what() << "\n";
        return 1;
    }
}
