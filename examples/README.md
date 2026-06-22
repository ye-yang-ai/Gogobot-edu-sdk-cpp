# Examples

The C++ examples follow the same learning path as the Python SDK: connect first, read sensors next, then try actions, movement, audio, and robot adjustment.

Every `.cpp` example starts with a usage block that explains purpose, risk, command line, expected result, and exit behavior.

## Risk Levels

| Level | Meaning |
| --- | --- |
| Low | Scans devices, connects, reads state, or changes non-motion settings |
| Medium | Runs actions, ears, expressions, audio, volume, or short high-level movement |
| High | Changes pose, foot, joint, or timed movement targets; supervise the robot closely |

## Recommended Order

1. `01_connection/bluetooth/ble_scan_and_connect.cpp`
2. `01_connection/bluetooth/ble_connect_by_address.cpp`
3. `01_connection/websocket/ws_connection_test.cpp`
4. `04_sensors/bluetooth/ble_imu_read.cpp`
5. `04_sensors/bluetooth/ble_tof_read.cpp`
6. `04_sensors/websocket/ws_imu_lan_read.cpp`
7. `04_sensors/websocket/ws_tof_lan_read.cpp`
8. `02_actions/bluetooth/ble_basic_actions.cpp`
9. `02_actions/bluetooth/ble_ears_expressions_audio.cpp`
10. `02_actions/websocket/ws_basic_actions.cpp`
11. `02_actions/websocket/ws_ears_expressions_audio.cpp`
12. `03_movement/bluetooth/ble_directional_move.cpp`
13. `03_movement/websocket/ws_directional_move.cpp`
14. `05_audio/bidirectional_pcm_ws_host.cpp`
15. `06_robot_adjust/safe_pose_adjust.cpp`
16. `06_robot_adjust/custom_action.cpp`

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
  05_audio/
    bidirectional_pcm_ws_host.cpp
  06_robot_adjust/
    safe_pose_adjust.cpp
    custom_action.cpp
```

## Index

| Path | Target | Purpose | Risk | Typical command |
| --- | --- | --- | --- | --- |
| `01_connection/bluetooth/ble_scan_and_connect.cpp` | `aidog_ble_scan_and_connect` | Scan and connect to a Gogobot BLE device | Low | `.\build\Release\aidog_ble_scan_and_connect.exe --prefix Gogobot` |
| `01_connection/bluetooth/ble_connect_by_address.cpp` | `aidog_ble_connect_by_address` | Connect to a known BLE device | Low | `.\build\Release\aidog_ble_connect_by_address.exe --address 12:0A:AB:16:3A:04` |
| `01_connection/websocket/ws_connection_test.cpp` | `aidog_ws_connection_test` | Wait for a robot WebSocket connection | Low | `.\build\Release\aidog_ws_connection_test.exe --timeout 60 --no-keep-alive` |
| `02_actions/bluetooth/ble_basic_actions.cpp` | `aidog_ble_basic_actions` | Run one high-level BLE action | Medium | `.\build\Release\aidog_ble_basic_actions.exe --address 12:0A:AB:16:3A:04 --action sit_down --yes` |
| `02_actions/bluetooth/ble_ears_expressions_audio.cpp` | `aidog_ble_ears_expressions_audio` | Control ears, expressions, audio, volume, and special detection over BLE | Low/Medium | `.\build\Release\aidog_ble_ears_expressions_audio.exe --address 12:0A:AB:16:3A:04 --yes` |
| `02_actions/bluetooth/ble_choreography.cpp` | `aidog_ble_choreography` | Run a combined BLE choreography | Medium | `.\build\Release\aidog_ble_choreography.exe --address 12:0A:AB:16:3A:04 --yes` |
| `02_actions/websocket/ws_basic_actions.cpp` | `aidog_ws_basic_actions` | Run one action through the WebSocket host | Medium | `.\build\Release\aidog_ws_basic_actions.exe --action sit_down --settle 0.6 --connect-timeout 120 --yes` |
| `02_actions/websocket/ws_ears_expressions_audio.cpp` | `aidog_ws_ears_expressions_audio` | Control ears, expressions, audio, and volume over WebSocket | Low/Medium | `.\build\Release\aidog_ws_ears_expressions_audio.exe --volume 2 --connect-timeout 120` |
| `02_actions/websocket/ws_choreography.cpp` | `aidog_ws_choreography` | Run a combined WebSocket choreography | High | `.\build\Release\aidog_ws_choreography.exe --connect-timeout 120 --yes` |
| `03_movement/bluetooth/ble_directional_move.cpp` | `aidog_ble_directional_move` | Move in one selected direction over BLE | Medium | `.\build\Release\aidog_ble_directional_move.exe --address 12:0A:AB:16:3A:04 --direction forward --duration 1 --yes` |
| `03_movement/bluetooth/ble_timed_move.cpp` | `aidog_ble_timed_move` | Run a timed BLE movement sequence | Medium | `.\build\Release\aidog_ble_timed_move.exe --address 12:0A:AB:16:3A:04 --duration 1 --pause 1 --yes` |
| `03_movement/websocket/ws_directional_move.cpp` | `aidog_ws_directional_move` | Move in one selected direction over WebSocket | High | `.\build\Release\aidog_ws_directional_move.exe --direction forward --duration 1 --connect-timeout 120 --yes` |
| `03_movement/websocket/ws_timed_move.cpp` | `aidog_ws_timed_move` | Run a timed WebSocket movement sequence | High | `.\build\Release\aidog_ws_timed_move.exe --duration 1 --pause 1 --connect-timeout 120 --yes` |
| `04_sensors/bluetooth/ble_imu_read.cpp` | `aidog_ble_imu_read` | Read BLE IMU stream | Low | `.\build\Release\aidog_ble_imu_read.exe --address 12:0A:AB:16:3A:04 --hz 20 --seconds 10` |
| `04_sensors/bluetooth/ble_tof_read.cpp` | `aidog_ble_tof_read` | Read BLE TOF stream | Low | `.\build\Release\aidog_ble_tof_read.exe --address 12:0A:AB:16:3A:04 --hz 20 --seconds 10` |
| `04_sensors/websocket/ws_imu_lan_read.cpp` | `aidog_ws_imu_lan_read` | Read WebSocket IMU JSON stream | Low | `.\build\Release\aidog_ws_imu_lan_read.exe --hz 20 --seconds 10 --connect-timeout 120` |
| `04_sensors/websocket/ws_tof_lan_read.cpp` | `aidog_ws_tof_lan_read` | Read WebSocket TOF JSON stream | Low | `.\build\Release\aidog_ws_tof_lan_read.exe --hz 20 --seconds 10 --connect-timeout 120` |
| `05_audio/bidirectional_pcm_ws_host.cpp` | `aidog_ws_bidirectional_pcm_host` | Send PC microphone PCM to the robot and play robot PCM on the PC | Medium | `.\build\Release\aidog_ws_bidirectional_pcm_host.exe --connect-timeout 120 --input-device 0 --output-device 0` |
| `06_robot_adjust/safe_pose_adjust.cpp` | `aidog_safe_pose_adjust` | Run low-amplitude body and foot adjustment | High | `.\build\Release\aidog_safe_pose_adjust.exe --address 12:0A:AB:16:3A:04 --yes` |
| `06_robot_adjust/custom_action.cpp` | `aidog_custom_action` | Run a sniff-like custom robot-adjustment action | High | `.\build\Release\aidog_custom_action.exe --address 12:0A:AB:16:3A:04 --yes` |

## Common Arguments

- `--prefix`, `--name-prefix`: BLE advertisement prefix, default `Gogobot`.
- `--address`: BLE MAC address or Windows BLE address.
- `--timeout`: operation timeout for supported examples.
- `--connect-timeout`: WebSocket robot connection wait timeout.
- `--settle`: WebSocket action delay after interaction status becomes ready.
- `--hz`: requested sensor stream rate.
- `--seconds`: sensor read duration.
- `--action`: high-level action name.
- `--direction`: movement direction.
- `--duration`: movement or action duration in seconds.
- `--pause`: pause between timed movement commands.
- `--hold`: custom robot-adjustment hold duration in seconds.
- `--bind`: WebSocket bind address, default `0.0.0.0`.
- `--port`: WebSocket listen port, default `8766`.
- `--input-device`: Windows recording device index for PCM upload.
- `--output-device`: Windows playback device index for PCM downlink.
- `--list-devices`: list available Windows audio input/output devices.
- `--yes`: skip confirmation for examples that can move the robot.

## Build

```powershell
cd C:\C_project_3.1\arbitrarion10\aidog_sdk_cpp
"C:\Program Files\CMake\bin\cmake.exe" -S . -B build -G "Visual Studio 17 2022" -A x64
"C:\Program Files\CMake\bin\cmake.exe" --build build --config Release
"C:\Program Files\CMake\bin\ctest.exe" --test-dir build -C Release --output-on-failure
```

## WebSocket Setup

The robot connects back to the development PC in WebSocket mode. Configure the PC IP over BLE first:

```powershell
.\build\Release\aidog_set_dev_pc_ws_ip_ble.exe --address 12:0A:AB:16:3A:04 192.168.11.101
```

Then start one WS example or the WS GUI and wait for the robot to connect:

```powershell
.\build\Release\aidog_ws_connection_test.exe --timeout 60 --no-keep-alive
.\build\Release\aidog_user_control_ws.exe
```

## Control Panels

BLE control panel:

```powershell
.\build\Release\aidog_user_control_ble.exe
```

WebSocket control panel:

```powershell
.\build\Release\aidog_user_control_ws.exe
```

The BLE and WS control panels share the same user-facing control layout. The WS version only changes the connection transport.
