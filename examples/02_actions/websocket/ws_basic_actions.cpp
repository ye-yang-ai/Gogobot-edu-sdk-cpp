/*
Purpose:
    Run one basic action through the Dev PC WebSocket host.
Risk level:
    Medium. The robot may move after it connects to the PC host.
Run:
    .\build\Release\aidog_ws_basic_actions.exe
Expected result:
    The host waits on ws://0.0.0.0:8766, then runs sit_down after the robot connects.
Exit:
    Wait for the action to finish, or press Ctrl+C.
*/

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
