/*
Purpose:
    Read IMU data through the Dev PC WebSocket host.
Risk level:
    Low. This example only requests and prints sensor data.
Run:
    .\build\Release\aidog_ws_imu_read.exe
Expected result:
    The host waits on ws://0.0.0.0:8766 and prints IMU values after the robot connects.
Exit:
    Press Ctrl+C.
*/

#include <chrono>
#include <iostream>
#include <thread>

#include <aidog.hpp>

int main()
{
    try {
        aidog::AiDog dog;
        aidog::WebSocketHost host("0.0.0.0", 8766, &dog);
        dog.add_imu_listener([](const aidog::ImuData& imu) {
            std::cout << "imu yaw=" << imu.yawDeg.value_or(0.0)
                      << " pitch=" << imu.pitchDeg.value_or(0.0)
                      << " roll=" << imu.rollDeg.value_or(0.0) << "\n";
        });
        host.start();
        std::cout << "waiting for robot on ws://0.0.0.0:8766\n";
        if (!host.wait_robot_connected(30.0)) {
            std::cerr << "timeout waiting for robot\n";
            return 1;
        }
        dog.request_imu_stream(true, 50, "ws");
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    } catch (const std::exception& exc) {
        std::cerr << exc.what() << "\n";
        return 1;
    }
}
