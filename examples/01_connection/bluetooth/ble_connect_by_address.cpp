/*
Purpose:
    Connect directly to a known BLE address.
Risk level:
    Low. This example does not command physical movement.
Run:
    .\build\Release\aidog_ble_connect_by_address.exe --address AA:BB:CC:DD:EE:FF
Expected result:
    The terminal reports a successful BLE connection.
Exit:
    Press Enter after connection, or Ctrl+C.
*/

#include <iostream>
#include <optional>
#include <stdexcept>
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
        }
    }
    if (!options.address.has_value()) {
        throw std::invalid_argument("--address is required");
    }
    return options;
}

} // namespace

int main(int argc, char** argv)
{
    aidog::AiDog dog;
    try {
        auto options = _parse_options(argc, argv);
        std::cout << "[connect] address: " << *options.address << "\n";
        dog.connect(options);
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
