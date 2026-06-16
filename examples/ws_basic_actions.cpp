#include <iostream>

#include <aidog.hpp>

int main()
{
    try {
        aidog::AiDog dog;
        aidog::WebSocketHost host("0.0.0.0", 8766, &dog);
        host.start();
        std::cout << "waiting for robot on ws://0.0.0.0:8766\n";
        if (!host.wait_robot_connected(30.0)) {
            std::cerr << "timeout waiting for robot\n";
            return 1;
        }
        aidog::ActionOptions options;
        options.transport = "ws";
        const bool ok = dog.perform_action(aidog::Action::SitDown, options);
        std::cout << "action_done=" << std::boolalpha << ok << "\n";
        host.stop();
        return ok ? 0 : 2;
    } catch (const std::exception& exc) {
        std::cerr << exc.what() << "\n";
        return 1;
    }
}
