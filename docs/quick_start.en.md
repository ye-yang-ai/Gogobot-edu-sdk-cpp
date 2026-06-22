# Quick Start

This guide helps you build the C++ SDK, connect to a Gogobot EDU / Changba AI-Dog robot, and run the first safe commands.

## 1. Prepare the Robot

- Put the robot on a flat open floor.
- Keep enough space around it before running movement examples.
- Make sure the robot is powered on and within Bluetooth range.
- For movement and adjustment examples, stay close enough to stop the robot quickly.

## 2. Build the SDK

Run commands from the C++ SDK root:

```powershell
cd C:\C_project_3.1\arbitrarion10\aidog_sdk_cpp
"C:\Program Files\CMake\bin\cmake.exe" -S . -B build -G "Visual Studio 17 2022" -A x64
"C:\Program Files\CMake\bin\cmake.exe" --build build --config Release
```

Run tests:

```powershell
"C:\Program Files\CMake\bin\ctest.exe" --test-dir build -C Release --output-on-failure
```

## 3. Scan and Connect over BLE

Scan by name prefix:

```powershell
.\build\Release\aidog_ble_scan_and_connect.exe --prefix Gogobot
```

Connect by known address:

```powershell
.\build\Release\aidog_ble_connect_by_address.exe --address AA:BB:CC:DD:EE:FF
```

C++ usage:

```cpp
#include <aidog.hpp>

aidog::AiDog dog;
aidog::ConnectOptions options;
options.address = "AA:BB:CC:DD:EE:FF";
dog.connect(options);
```

## 4. Run a Safe Action

```powershell
.\build\Release\aidog_ble_basic_actions.exe --address AA:BB:CC:DD:EE:FF --action sit_down --yes
```

C++ usage:

```cpp
aidog::ActionOptions options;
options.timeoutS = 20.0;
const bool ok = dog.perform_action(aidog::Action::SitDown, options);
```

## 5. Use WebSocket Control

The robot connects back to the development PC. Configure the PC IP through BLE when the PC IP changes:

```powershell
.\build\Release\aidog_set_dev_pc_ws_ip_ble.exe --address AA:BB:CC:DD:EE:FF 192.168.11.101
```

Then start a PC WebSocket host example:

```powershell
.\build\Release\aidog_ws_connection_test.exe --timeout 60 --no-keep-alive
.\build\Release\aidog_ws_basic_actions.exe --action sit_down --yes --connect-timeout 120
```

High-level C++ usage:

```cpp
aidog::AiDog dog;
aidog::WebSocketHost host("0.0.0.0", 8766, &dog);
host.start();
host.wait_robot_connected(120.0);

dog.send_audio(aidog::Tone::Jeez, "ws");
dog.send_interaction(aidog::Action::ShakeHand, std::nullopt, "ws");
dog.start_movement(aidog::Movement::Forward, "ws");
dog.stop_movement("ws");
```

## 6. Try the Control Panels

BLE control panel:

```powershell
.\build\Release\aidog_user_control_ble.exe
```

WebSocket control panel:

```powershell
.\build\Release\aidog_user_control_ws.exe
```

## 7. Next Examples

- `examples/04_sensors`: IMU and TOF data.
- `examples/02_actions`: actions, ears, expressions, audio, and volume.
- `examples/03_movement`: directional and timed movement.
- `examples/05_audio`: bidirectional PCM audio over WebSocket.
- `examples/06_robot_adjust`: advanced pose, foot, and joint adjustment.

Read [Safety](safety.md) before running movement or robot-adjustment examples.
