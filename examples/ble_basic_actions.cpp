#include <iostream>

#include <aidog.hpp>

int main()
{
    try {
        aidog::AiDog dog;
        dog.connect();
        const bool ok = dog.perform_action(aidog::Action::UpAndDown);
        std::cout << "action_done=" << std::boolalpha << ok << "\n";
        dog.shutdown();
        return ok ? 0 : 2;
    } catch (const std::exception& exc) {
        std::cerr << exc.what() << "\n";
        return 1;
    }
}
