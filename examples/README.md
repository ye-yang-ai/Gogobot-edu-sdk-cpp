# Examples

The BLE and WebSocket examples are aligned with the Python SDK example layout.

Every `.cpp` example starts with a short usage block that shows purpose, risk,
commands, expected result, and exit behavior.

## Risk Levels

| Level | Meaning |
|---|---|
| Low | Reads state, scans devices, connects, or changes non-motion settings |
| Medium | Runs actions or movement through high-level robot APIs |
| High | Changes body, foot, or joint targets; requires careful supervision |

## Recommended BLE Order

1. `01_connection/bluetooth/ble_scan_and_connect.cpp`
2. `01_connection/bluetooth/ble_connect_by_address.cpp`
3. `04_sensors/bluetooth/ble_imu_read.cpp`
4. `04_sensors/bluetooth/ble_tof_read.cpp`
5. `02_actions/bluetooth/ble_basic_actions.cpp`
6. `02_actions/bluetooth/ble_ears_expressions_audio.cpp`
7. `03_movement/bluetooth/ble_directional_move.cpp`
8. `03_movement/bluetooth/ble_timed_move.cpp`
9. `02_actions/bluetooth/ble_choreography.cpp`
10. `06_robot_adjust/safe_pose_adjust.cpp`
11. `06_robot_adjust/custom_action.cpp`

## Directory Structure

```text
examples/
  01_connection/
    bluetooth/
      ble_scan_and_connect.cpp
      ble_connect_by_address.cpp
    websocket/
      ws_connection_test.cpp
  02_actions/
    bluetooth/
      ble_basic_actions.cpp
      ble_choreography.cpp
      ble_ears_expressions_audio.cpp
    websocket/
      ws_basic_actions.cpp
      ws_choreography.cpp
      ws_ears_expressions_audio.cpp
  03_movement/
    bluetooth/
      ble_directional_move.cpp
      ble_timed_move.cpp
    websocket/
      ws_directional_move.cpp
      ws_timed_move.cpp
  04_sensors/
    bluetooth/
      ble_imu_read.cpp
      ble_tof_read.cpp
    websocket/
      ws_imu_lan_read.cpp
      ws_tof_lan_read.cpp
  06_robot_adjust/
    safe_pose_adjust.cpp
    custom_action.cpp
```

## Index

| Path | Target | Purpose | Risk | Typical command |
|---|---|---|---|---|
| `01_connection/bluetooth/ble_scan_and_connect.cpp` | `aidog_ble_scan_and_connect` | Scan and connect to a Gogobot BLE device | Low | `.\build\Release\aidog_ble_scan_and_connect.exe` |
| `01_connection/bluetooth/ble_connect_by_address.cpp` | `aidog_ble_connect_by_address` | Connect to a known BLE device | Low | `.\build\Release\aidog_ble_connect_by_address.exe --address AA:BB:CC:DD:EE:FF` |
| `02_actions/bluetooth/ble_basic_actions.cpp` | `aidog_ble_basic_actions` | Run one high-level BLE action | Medium | `.\build\Release\aidog_ble_basic_actions.exe --address AA:BB:CC:DD:EE:FF --action sit_down --yes` |
| `02_actions/bluetooth/ble_choreography.cpp` | `aidog_ble_choreography` | Run a combined BLE choreography | Medium | `.\build\Release\aidog_ble_choreography.exe --address AA:BB:CC:DD:EE:FF --yes` |
| `02_actions/bluetooth/ble_ears_expressions_audio.cpp` | `aidog_ble_ears_expressions_audio` | Control ears, expression, audio, volume, and special detection | Low/Medium | `.\build\Release\aidog_ble_ears_expressions_audio.exe --address AA:BB:CC:DD:EE:FF --yes` |
| `03_movement/bluetooth/ble_directional_move.cpp` | `aidog_ble_directional_move` | Move in one selected direction over BLE | Medium | `.\build\Release\aidog_ble_directional_move.exe --address AA:BB:CC:DD:EE:FF --direction forward --duration 1 --yes` |
| `03_movement/bluetooth/ble_timed_move.cpp` | `aidog_ble_timed_move` | Run a timed BLE movement sequence | Medium | `.\build\Release\aidog_ble_timed_move.exe --address AA:BB:CC:DD:EE:FF --duration 1 --pause 1 --yes` |
| `04_sensors/bluetooth/ble_imu_read.cpp` | `aidog_ble_imu_read` | Read BLE IMU stream | Low | `.\build\Release\aidog_ble_imu_read.exe --address AA:BB:CC:DD:EE:FF --hz 20 --seconds 20` |
| `04_sensors/bluetooth/ble_tof_read.cpp` | `aidog_ble_tof_read` | Read BLE TOF stream | Low | `.\build\Release\aidog_ble_tof_read.exe --address AA:BB:CC:DD:EE:FF --hz 20 --seconds 20` |
| `01_connection/websocket/ws_connection_test.cpp` | `aidog_ws_connection_test` | Wait for a robot WebSocket connection | Low | `.\build\Release\aidog_ws_connection_test.exe` |
| `02_actions/websocket/ws_basic_actions.cpp` | `aidog_ws_basic_actions` | Run one action through the WebSocket host | Medium | `.\build\Release\aidog_ws_basic_actions.exe --action sit_down --yes` |
| `02_actions/websocket/ws_choreography.cpp` | `aidog_ws_choreography` | Run a combined WebSocket choreography | High | `.\build\Release\aidog_ws_choreography.exe --yes` |
| `02_actions/websocket/ws_ears_expressions_audio.cpp` | `aidog_ws_ears_expressions_audio` | Control ears, expression, audio, and volume over WebSocket | Low/Medium | `.\build\Release\aidog_ws_ears_expressions_audio.exe --volume 2` |
| `03_movement/websocket/ws_directional_move.cpp` | `aidog_ws_directional_move` | Move in one selected direction over WebSocket | High | `.\build\Release\aidog_ws_directional_move.exe --direction forward --duration 1 --yes` |
| `03_movement/websocket/ws_timed_move.cpp` | `aidog_ws_timed_move` | Run a timed WebSocket movement sequence | High | `.\build\Release\aidog_ws_timed_move.exe --duration 1 --pause 1 --yes` |
| `04_sensors/websocket/ws_imu_lan_read.cpp` | `aidog_ws_imu_lan_read` | Read WebSocket IMU JSON stream | Low | `.\build\Release\aidog_ws_imu_lan_read.exe --hz 20 --seconds 20` |
| `04_sensors/websocket/ws_tof_lan_read.cpp` | `aidog_ws_tof_lan_read` | Read WebSocket TOF JSON stream | Low | `.\build\Release\aidog_ws_tof_lan_read.exe --hz 20 --seconds 20` |
| `06_robot_adjust/safe_pose_adjust.cpp` | `aidog_safe_pose_adjust` | Run low-amplitude body and foot adjustment | High | `.\build\Release\aidog_safe_pose_adjust.exe --address AA:BB:CC:DD:EE:FF --yes` |
| `06_robot_adjust/custom_action.cpp` | `aidog_custom_action` | Run a sniff-like custom robot-adjustment action | High | `.\build\Release\aidog_custom_action.exe --address AA:BB:CC:DD:EE:FF --yes` |

## Common Arguments

- `--prefix`, `--name-prefix`: BLE advertisement prefix, default `Gogobot`.
- `--address`: BLE MAC address or Windows BLE address.
- `--timeout`: operation timeout for supported action examples.
- `--hz`: requested sensor stream rate.
- `--seconds`: sensor read duration.
- `--action`: high-level action name.
- `--direction`: movement direction.
- `--duration`: movement or action duration in seconds.
- `--pause`: pause between timed movement commands.
- `--hold`: custom robot-adjustment hold duration in seconds.
- `--yes`: skip confirmation for examples that can move the robot.

## Build

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

## Control Panel

The Windows BLE upper-computer control panel lives in `../tools/` and is built
as:

```powershell
.\build\Release\aidog_user_control_ble.exe
```

It provides scan/connect, movement, common actions, ears, expressions, audio,
0-4 volume levels, and IMU/TOF text monitoring through the public C++ `AiDog`
APIs.
