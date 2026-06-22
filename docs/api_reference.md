# API Reference

This page summarizes the public APIs exported by the C++ SDK.

## Connection

| API | Description |
| --- | --- |
| `AiDog::scan(namePrefix)` | Scan nearby BLE devices and return `DeviceInfo` records |
| `AiDog::connect(options)` | Connect by BLE address or name prefix |
| `AiDog::disconnect()` | Disconnect the current BLE device |
| `AiDog::shutdown()` | Disconnect and stop SDK background work |
| `AiDog::is_connected()` | Return BLE connection state |
| `AiDog::attach_ws_control(host)` | Attach a `WebSocketHost` for `transport="ws"` control |

## Actions and Movement

| API | Description |
| --- | --- |
| `perform_action(action, options)` | Send an interaction action and wait for completion feedback |
| `perform_action(actionName, options)` | Run an action from a user-facing action string |
| `send_interaction(action, param, transport)` | Send raw interaction command |
| `send_movement(direction, durationS, transport)` | Start movement and optionally auto-stop |
| `start_movement(direction, transport)` | Start movement in a direction |
| `stop_movement(transport)` | Stop movement |
| `reset(transport)` | Stop movement and send interaction stop |

### Action Inputs

`perform_action()` accepts `aidog::Action` or a string resolved by the C++ action alias table.

```cpp
dog.perform_action(aidog::Action::SitDown);
dog.perform_action("sit_down");
```

One optional firmware parameter can be sent with supported actions:

| Action set | Option field | Meaning |
| --- | --- | --- |
| Time based | `ActionOptions::duration` | Seconds, clamped by the SDK |
| Count based | `ActionOptions::count` | Repetitions, clamped by the SDK |
| Angle based | `ActionOptions::angle` | Degrees, clamped by the SDK |

```cpp
aidog::ActionOptions options;
options.duration = 3;
dog.perform_action(aidog::Action::ForwardInteraction, options);

options = {};
options.count = 2;
dog.perform_action(aidog::Action::ShakeHand, options);

options = {};
options.angle = 90;
dog.perform_action(aidog::Action::RightAngleInteraction, options);
```

## Ears, Expression, Audio

| API | Description |
| --- | --- |
| `send_ear(action, transport)` | Ear action |
| `send_ear_percentage(percentage, transport)` | Ear position percentage |
| `set_special_detection(enable, transport)` | Enable or disable special-state detection |
| `enable_special_detection(transport)` | Enable special-state detection |
| `disable_special_detection(transport)` | Disable special-state detection |
| `send_expression(expression, transport)` | Face expression |
| `send_audio(tone, transport)` | Tone or audio control |
| `set_volume(volume, verifyTone, verifyDelayS, transport)` | Set speaker volume level `0-4` |

## Sensors

| API | Description |
| --- | --- |
| `request_imu_stream(enable, hz, transport)` | Request IMU stream |
| `get_latest_imu()` | Last parsed IMU payload |
| `add_imu_listener(callback)` | Register IMU callback |
| `remove_imu_listener(id)` | Remove IMU callback |
| `request_tof_stream(enable, hz, transport)` | Request TOF stream |
| `get_latest_tof()` | Last parsed TOF payload |
| `add_tof_listener(callback)` | Register TOF callback |
| `remove_tof_listener(id)` | Remove TOF callback |
| `feed_sensor_stream_json(text)` | Feed LAN/WebSocket sensor JSON into SDK state |
| `feed_notify_bytes(data)` | Feed BLE notification bytes into SDK state |

## Robot Adjustment

These APIs require matching firmware support and should be treated as advanced control.

| API | Description |
| --- | --- |
| `syn_pose_adjust(items, durationMs)` | Smooth COG / pose adjustment |
| `syn_foot_adjust(items, durationMs)` | Smooth foot X/Z adjustment |
| `syn_joint_adjust(items, durationMs)` | Smooth joint delta adjustment |
| `default_pose_output(roll, pitch, x, z)` | Move to a known baseline pose |
| `request_basic_mode()` | Request return to basic mode |

## WebSocket Host

| API | Description |
| --- | --- |
| `WebSocketHost(host, port, dog)` | Create a PC-side WebSocket host |
| `start()` | Start listening |
| `stop()` | Stop listening and close connections |
| `wait_robot_connected(timeoutS)` | Wait for robot connection |
| `robot_connected()` | Return current robot connection state |
| `send_binary(data)` | Send raw binary frame |
| `send_pcm(data)` | Send PCM frame to the robot |

## Low-Level Extension

| API | Description |
| --- | --- |
| `send_raw(mode, data, transport)` | Send raw protocol packet |
| `get_action_list()` | Read optional action definition JSON from `ae10` |
| `set_dev_pc_ws_ip(ip)` | Write Dev-PC WebSocket IP configuration through BLE |

## Exported Enums

- `Action`
- `Movement`
- `EarAction`
- `ExpressionAction`
- `Tone`
- `ParameterType`
