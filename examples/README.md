# Examples

The examples are organized by feature area and transport. Bluetooth examples
control the robot directly over BLE. WebSocket examples are for the Dev PC host
mode where the robot connects back to the PC.

## Risk Levels

| Level | Meaning |
|---|---|
| Low | Reads state, scans devices, connects, or changes non-motion settings |
| Medium | Runs actions or movement through high-level robot APIs |
| High | Changes body, foot, or joint targets; requires careful supervision |

## Recommended Order

1. `01_connection/bluetooth/ble_scan.cpp`
2. `01_connection/bluetooth/ble_connect_test.cpp`
3. `04_sensors/bluetooth/ble_imu_read.cpp`
4. `04_sensors/bluetooth/ble_tof_read.cpp`
5. `02_actions/bluetooth/ble_action_list.cpp`
6. `02_actions/bluetooth/ble_action.cpp`
7. `03_movement/bluetooth/ble_move.cpp`
8. `02_actions/bluetooth/ble_ears_expressions_audio.cpp`
9. `05_audio/bluetooth/ble_set_volume.cpp`
10. `06_robot_adjust/bluetooth/ble_safe_pose_adjust.cpp`
11. `06_robot_adjust/bluetooth/ble_custom_action.cpp`
12. `02_actions/websocket/ws_basic_actions.cpp`
13. `04_sensors/websocket/ws_imu_read.cpp`

## Directory Structure

```text
examples/
  01_connection/
    bluetooth/
      ble_scan.cpp
      ble_connect_test.cpp
    websocket/
  02_actions/
    bluetooth/
      ble_action.cpp
      ble_action_list.cpp
      ble_basic_actions.cpp
      ble_ears_expressions_audio.cpp
    websocket/
      ws_basic_actions.cpp
  03_movement/
    bluetooth/
      ble_move.cpp
    websocket/
  04_sensors/
    bluetooth/
      ble_imu_read.cpp
      ble_tof_read.cpp
    websocket/
      ws_imu_read.cpp
  05_audio/
    bluetooth/
      ble_set_volume.cpp
    websocket/
  06_robot_adjust/
    bluetooth/
      ble_safe_pose_adjust.cpp
      ble_custom_action.cpp
    websocket/
```

## Index

| Path | Target | Purpose | Risk | Typical command |
|---|---|---|---|---|
| `01_connection/bluetooth/ble_scan.cpp` | `aidog_ble_scan` | Scan Gogobot BLE devices | Low | `.\build\Release\aidog_ble_scan.exe` |
| `01_connection/bluetooth/ble_connect_test.cpp` | `aidog_ble_connect_test` | Connect to a known BLE device | Low | `.\build\Release\aidog_ble_connect_test.exe --address AA:BB:CC:DD:EE:FF` |
| `02_actions/bluetooth/ble_action_list.cpp` | `aidog_ble_action_list` | List known high-level actions | Low | `.\build\Release\aidog_ble_action_list.exe` |
| `02_actions/bluetooth/ble_action.cpp` | `aidog_ble_action` | Run one high-level BLE action | Medium | `.\build\Release\aidog_ble_action.exe --address AA:BB:CC:DD:EE:FF --action sit_down --yes` |
| `02_actions/bluetooth/ble_basic_actions.cpp` | `aidog_ble_basic_actions` | Compatibility wrapper for one BLE action | Medium | `.\build\Release\aidog_ble_basic_actions.exe --address AA:BB:CC:DD:EE:FF --action sit_down --yes` |
| `02_actions/bluetooth/ble_ears_expressions_audio.cpp` | `aidog_ble_ears_expressions_audio` | Control ears, expression, and tone over BLE | Low/Medium | `.\build\Release\aidog_ble_ears_expressions_audio.exe --address AA:BB:CC:DD:EE:FF --yes` |
| `02_actions/websocket/ws_basic_actions.cpp` | `aidog_ws_basic_actions` | Run one action through the WebSocket host | Medium | `.\build\Release\aidog_ws_basic_actions.exe` |
| `03_movement/bluetooth/ble_move.cpp` | `aidog_ble_move` | Move in one selected direction over BLE | Medium | `.\build\Release\aidog_ble_move.exe --address AA:BB:CC:DD:EE:FF --direction forward --duration 1 --yes` |
| `04_sensors/bluetooth/ble_imu_read.cpp` | `aidog_ble_imu_read` | Read BLE IMU stream | Low | `.\build\Release\aidog_ble_imu_read.exe --address AA:BB:CC:DD:EE:FF --hz 20 --seconds 10` |
| `04_sensors/bluetooth/ble_tof_read.cpp` | `aidog_ble_tof_read` | Read BLE TOF stream | Low | `.\build\Release\aidog_ble_tof_read.exe --address AA:BB:CC:DD:EE:FF --hz 20 --seconds 10` |
| `04_sensors/websocket/ws_imu_read.cpp` | `aidog_ws_imu_read` | Read WebSocket IMU JSON stream | Low | `.\build\Release\aidog_ws_imu_read.exe` |
| `05_audio/bluetooth/ble_set_volume.cpp` | `aidog_ble_set_volume` | Set BLE volume level 0-4 | Low | `.\build\Release\aidog_ble_set_volume.exe --address AA:BB:CC:DD:EE:FF --volume 3` |
| `06_robot_adjust/bluetooth/ble_safe_pose_adjust.cpp` | `aidog_ble_safe_pose_adjust` | Run low-amplitude body and foot adjustment | High | `.\build\Release\aidog_ble_safe_pose_adjust.exe --address AA:BB:CC:DD:EE:FF --yes` |
| `06_robot_adjust/bluetooth/ble_custom_action.cpp` | `aidog_ble_custom_action` | Run a sniff-like custom robot-adjustment action | High | `.\build\Release\aidog_ble_custom_action.exe --address AA:BB:CC:DD:EE:FF --yes` |

## Common Arguments

- `--prefix`: BLE advertisement prefix, default `Gogobot`.
- `--address`: BLE MAC address or Windows BLE address.
- `--timeout`: scan, connect, or operation timeout.
- `--hz`: requested sensor stream rate.
- `--seconds`: sensor read duration.
- `--action`: high-level action name.
- `--direction`: movement direction.
- `--duration`: movement or action duration in seconds.
- `--hold`: custom robot-adjustment hold duration in seconds.
- `--volume`: volume level, `0` is mute and `4` is max.
- `--no-verify`: set volume without playing a verification tone.
- `--yes`: skip confirmation for examples that can move the robot.

## Build

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```
