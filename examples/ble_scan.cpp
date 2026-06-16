#include <iostream>
#include <string>

#include <aidog.hpp>

int main(int argc, char** argv)
{
    std::string prefix = argc > 1 ? argv[1] : "Gogobot";
    try {
        aidog::AiDog dog(false);
        auto devices = dog.scan(prefix);
        std::cout << "found " << devices.size() << " device(s)\n";
        for (const auto& device : devices) {
            std::cout << device.name << " " << device.address << "\n";
        }
        return devices.empty() ? 2 : 0;
    } catch (const std::exception& exc) {
        std::cerr << exc.what() << "\n";
        return 1;
    }
}
