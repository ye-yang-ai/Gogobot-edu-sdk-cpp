/*
Purpose:
    Scan and connect over BLE.
Risk level:
    Low. This example does not command physical movement.
Run:
    .\build\Release\aidog_ble_scan_and_connect.exe
    .\build\Release\aidog_ble_scan_and_connect.exe --prefix Gogobot
    .\build\Release\aidog_ble_scan_and_connect.exe --address AA:BB:CC:DD:EE:FF
Expected result:
    The terminal lists nearby devices and reports a successful BLE connection.
Exit:
    Press Enter after connection, or Ctrl+C.
*/

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
        } else if ((arg == "--prefix" || arg == "--name-prefix") && i + 1 < argc) {
            options.namePrefix = argv[++i];
        }
    }
    return options;
}

} // namespace

int main(int argc, char** argv)
{
    aidog::AiDog dog;
    try {
        auto options = _parse_options(argc, argv);
        if (options.address.has_value()) {
            std::cout << "[connect] direct address: " << *options.address << "\n";
            dog.connect(options);
        } else {
            std::cout << "[scan] name prefix: " << options.namePrefix << "\n";
            auto devices = dog.scan(options.namePrefix);
            if (devices.empty()) {
                std::cout << "[scan] no matching device found\n";
                return 1;
            }
            std::cout << "[scan] found " << devices.size() << " device(s):\n";
            for (std::size_t i = 0; i < devices.size(); ++i) {
                std::cout << "  " << (i + 1) << ". " << devices[i].name << " [" << devices[i].address << "]\n";
            }
            options.address = devices.front().address;
            std::cout << "[connect] using first device: " << devices.front().name << " [" << devices.front().address << "]\n";
            dog.connect(options);
        }

        std::cout << "[connect] connected. Press Enter to disconnect.\n";
        std::string line;
        std::getline(std::cin, line);
        dog.disconnect();
        dog.shutdown();
        return 0;
    } catch (const std::exception& exc) {
        std::cerr << exc.what() << "\n";
        try {
            if (dog.is_connected()) {
                dog.disconnect();
            }
            dog.shutdown();
        } catch (...) {
        }
        return 1;
    }
}
