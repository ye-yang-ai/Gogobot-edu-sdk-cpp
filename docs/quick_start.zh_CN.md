# 快速开始

本指南用于让 Gogobot EDU / Changba AI-Dog 机器狗完成 C++ SDK 构建、第一次 BLE 连接，以及安全动作测试。

## 1. 准备机器狗

- 将机器狗放在平整、空旷的地面。
- 运行运动示例前，确认周围有足够空间。
- 确认机器狗已开机，并且电脑在蓝牙范围内。
- 运行动作、运动和调姿示例时，确保你可以随时停止机器狗。

## 2. 构建 SDK

在 C++ SDK 根目录执行：

```powershell
cd C:\C_project_3.1\arbitrarion10\aidog_sdk_cpp
"C:\Program Files\CMake\bin\cmake.exe" -S . -B build -G "Visual Studio 17 2022" -A x64
"C:\Program Files\CMake\bin\cmake.exe" --build build --config Release
```

运行测试：

```powershell
"C:\Program Files\CMake\bin\ctest.exe" --test-dir build -C Release --output-on-failure
```

## 3. 扫描和连接 BLE

按名称前缀扫描：

```powershell
.\build\Release\aidog_ble_scan_and_connect.exe --prefix Gogobot
```

按已知蓝牙地址连接：

```powershell
.\build\Release\aidog_ble_connect_by_address.exe --address AA:BB:CC:DD:EE:FF
```

C++ 调用方式：

```cpp
#include <aidog.hpp>

aidog::AiDog dog;
aidog::ConnectOptions options;
options.address = "AA:BB:CC:DD:EE:FF";
dog.connect(options);
```

## 4. 运行一个安全动作

```powershell
.\build\Release\aidog_ble_basic_actions.exe --address AA:BB:CC:DD:EE:FF --action sit_down --yes
```

C++ 调用方式：

```cpp
aidog::ActionOptions options;
options.timeoutS = 20.0;
const bool ok = dog.perform_action(aidog::Action::SitDown, options);
```

## 5. 使用 WebSocket 控制

WebSocket 模式下机器狗会主动连接开发电脑。电脑 IP 变化时，先通过 BLE 写入 Dev PC WebSocket IP：

```powershell
.\build\Release\aidog_set_dev_pc_ws_ip_ble.exe --address AA:BB:CC:DD:EE:FF 192.168.11.101
```

然后启动 PC 侧 WebSocket host 示例：

```powershell
.\build\Release\aidog_ws_connection_test.exe --timeout 60 --no-keep-alive
.\build\Release\aidog_ws_basic_actions.exe --action sit_down --yes --connect-timeout 120
```

C++ 高层调用方式：

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

## 6. 使用上位机

BLE 上位机：

```powershell
.\build\Release\aidog_user_control_ble.exe
```

WebSocket 上位机：

```powershell
.\build\Release\aidog_user_control_ws.exe
```

## 7. 下一步示例

- `examples/04_sensors`：IMU 和 TOF 数据读取。
- `examples/02_actions`：动作、耳朵、表情、音频和音量控制。
- `examples/03_movement`：方向运动和定时运动。
- `examples/05_audio`：WebSocket 双向 PCM 音频。
- `examples/06_robot_adjust`：高级姿态、足端和关节调姿。

运行运动和调姿示例前，请先阅读 [安全说明](safety.md)。
