# Dev PC WebSocket

Some firmware builds can connect to a PC-side WebSocket server for LAN control, sensor data, and bidirectional PCM audio.

## Firmware Configuration

The current C++ workflow configures the robot's Dev-PC WebSocket IP through BLE:

```powershell
.\build\Release\aidog_set_dev_pc_ws_ip_ble.exe --address 12:0A:AB:16:3A:04 192.168.11.101
```

The PC and robot must be on the same LAN, and the port must match the host program. The default port is `8766`.

## Build Dependencies

WebSocket support is enabled by default when CMake can fetch the third-party dependencies:

```powershell
"C:\Program Files\CMake\bin\cmake.exe" -S . -B build -G "Visual Studio 17 2022" -A x64 -DAIDOG_ENABLE_WEBSOCKET=ON
"C:\Program Files\CMake\bin\cmake.exe" --build build --config Release
```

## Sensor JSON Host

Examples:

```powershell
.\build\Release\aidog_ws_imu_lan_read.exe --bind 0.0.0.0 --port 8766 --hz 20 --seconds 10 --connect-timeout 120
.\build\Release\aidog_ws_tof_lan_read.exe --bind 0.0.0.0 --port 8766 --hz 20 --seconds 10 --connect-timeout 120
```

C++ usage:

```cpp
aidog::AiDog dog;
aidog::WebSocketHost host("0.0.0.0", 8766, &dog);
host.start();
host.wait_robot_connected(120.0);
dog.request_imu_stream(true, 20, "ws");
```

## Control over WebSocket

WebSocket control uses text JSON frames carrying the same raw packet that BLE writes to `ae03`. The firmware dispatches it through the existing remote-control parser, so action, ear, expression, audio, special detection, movement, sensor stream, and volume commands keep BLE-compatible behavior.

High-level SDK usage:

```cpp
aidog::AiDog dog;
aidog::WebSocketHost host("0.0.0.0", 8766, &dog);
host.start();
host.wait_robot_connected(120.0);

dog.send_audio(aidog::Tone::Jeez, "ws");
dog.send_interaction(aidog::Action::ShakeHand, std::nullopt, "ws");
dog.start_movement(aidog::Movement::Forward, "ws");
dog.stop_movement("ws");
dog.set_volume(3, std::nullopt, 0.2, "ws");
```

Control examples:

```powershell
.\build\Release\aidog_ws_connection_test.exe --timeout 60 --no-keep-alive
.\build\Release\aidog_ws_basic_actions.exe --action sit_down --yes --connect-timeout 120
.\build\Release\aidog_ws_ears_expressions_audio.exe --volume 2 --connect-timeout 120
.\build\Release\aidog_ws_directional_move.exe --direction forward --duration 2 --yes --connect-timeout 120
```

## WebSocket User Control Panel

`tools/user_control_ws_win.cpp` builds as `aidog_user_control_ws.exe`. It mirrors the BLE control panel layout while using the Dev-PC WebSocket link.

Run:

```powershell
.\build\Release\aidog_user_control_ws.exe
```

Workflow:

1. Configure the robot Dev-PC WebSocket IP through BLE.
2. Start the panel and click the WS host start button.
3. The panel listens on `ws://0.0.0.0:8766` by default.
4. After the robot connects, use the same pages as the BLE panel for movement, actions, ears, expressions, audio, special detection, and sensor plots.

## Bidirectional PCM Audio

Binary WebSocket frames are raw PCM:

- 16 kHz
- 16-bit signed little-endian
- mono

List Windows audio devices:

```powershell
.\build\Release\aidog_ws_bidirectional_pcm_host.exe --list-devices
```

Run bidirectional audio:

```powershell
.\build\Release\aidog_ws_bidirectional_pcm_host.exe --connect-timeout 120 --input-device 0 --output-device 0
```
