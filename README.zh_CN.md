# Gogobot EDU C++ SDK

[English](README.md) | [中文](README.zh_CN.md)

<img src="docs/assets/images/logo-text.png" alt="Gogobot EDU" width="260">

这是面向 **畅吧 AI-Dog / Gogobot EDU** 四足机器人的 Windows 优先 C++ SDK。它提供了课堂编程和上位机开发常用的 C++ 接口，覆盖 BLE 连接、动作控制、耳朵控制、表情控制、音频控制、传感器读取、Dev PC WebSocket 控制，以及双向 PCM 音频链路。

![Gogobot EDU robot](docs/assets/images/gogo-readme.jpg)

## 功能特性

- Windows BLE 扫描、按地址连接和 GATT 通信。
- 动作、摇杆运动、耳朵、表情、音频、音量和调姿 API。
- IMU / TOF 传感器读取示例。
- Dev PC WebSocket 主机模式，用于局域网动作控制和传感器读取。
- WebSocket 双向 PCM 音频示例，支持上位机麦克风到机器狗、机器狗 PCM 到上位机播放。
- Windows 图形上位机：BLE 版和 WS 版共用一致的控制界面。
- CMake 工程，示例程序按 Python SDK 的目录风格归类。

## 环境要求

- Windows 10 / Windows 11。
- Visual Studio 2022，安装 C++ 桌面开发组件。
- CMake 3.20 或更高版本。
- Python 3.10+，仅在需要运行 OTA / 调试辅助脚本时使用。
- 一台 Gogobot EDU / AI-Dog 机器人。

> 当前 C++ SDK 优先支持 Windows。Linux 和 macOS 暂不作为第一阶段支持目标。

## 构建

```powershell
cd C:\C_project_3.1\arbitrarion10\aidog_sdk_cpp
"C:\Program Files\CMake\bin\cmake.exe" -S . -B build -G "Visual Studio 17 2022" -A x64
"C:\Program Files\CMake\bin\cmake.exe" --build build --config Release
```

如果只想构建某个目标：

```powershell
"C:\Program Files\CMake\bin\cmake.exe" --build build --config Release --target aidog_user_control_ble
"C:\Program Files\CMake\bin\cmake.exe" --build build --config Release --target aidog_user_control_ws
```

## 快速开始

```cpp
#include <aidog.hpp>

int main() {
    aidog::AiDog dog;

    aidog::ConnectOptions options;
    options.address = "AA:BB:CC:DD:EE:FF";
    dog.connect(options);

    dog.send_interaction(aidog::Action::SitDown);
    dog.start_movement(aidog::Movement::Forward);
    dog.stop_movement();

    dog.send_ear(aidog::EarAction::EarStand);
    dog.send_expression(aidog::ExpressionAction::Happy01);
    dog.send_audio(aidog::Tone::Jeez);
    dog.set_volume(3);

    dog.disconnect();
    return 0;
}
```

## BLE 控制

常用 BLE 示例命令：

```powershell
cd C:\C_project_3.1\arbitrarion10\aidog_sdk_cpp

.\build\Release\aidog_ble_scan_and_connect.exe --prefix Gogobot
.\build\Release\aidog_ble_connect_by_address.exe --address AA:BB:CC:DD:EE:FF
.\build\Release\aidog_ble_basic_actions.exe --address AA:BB:CC:DD:EE:FF --action sit_down --yes
.\build\Release\aidog_ble_imu_read.exe --address AA:BB:CC:DD:EE:FF --hz 20 --seconds 10
```

## WebSocket 控制

WebSocket 模式下，机器狗主动连接开发电脑。第一次使用时，先通过 BLE 把开发电脑 IP 写入机器狗：

```powershell
cd C:\C_project_3.1\arbitrarion10\aidog_sdk_cpp
.\build\Release\aidog_set_dev_pc_ws_ip_ble.exe --address AA:BB:CC:DD:EE:FF 192.168.11.101
```

然后运行 WS 示例：

```powershell
.\build\Release\aidog_ws_connection_test.exe --timeout 60 --no-keep-alive
.\build\Release\aidog_ws_basic_actions.exe --action sit_down --yes --connect-timeout 120
.\build\Release\aidog_ws_imu_lan_read.exe --hz 20 --seconds 10 --connect-timeout 120
```

双向 PCM 音频：

```powershell
.\build\Release\aidog_ws_bidirectional_pcm_host.exe --list-devices
.\build\Release\aidog_ws_bidirectional_pcm_host.exe --connect-timeout 120 --input-device 0 --output-device 0
```

## 示例目录

| 目录 | 内容 |
| --- | --- |
| `examples/01_connection` | BLE / WS 连接测试 |
| `examples/02_actions` | 动作、耳朵、表情、音频和编舞控制 |
| `examples/03_movement` | 摇杆方向运动和定时运动 |
| `examples/04_sensors` | IMU / TOF 传感器读取 |
| `examples/05_audio` | WebSocket 双向 PCM 音频 |
| `examples/06_robot_adjust` | 调姿和自定义动作示例 |

完整示例说明见 [examples/README.md](examples/README.md)。

## 上位机工具

BLE 上位机：

```powershell
.\build\Release\aidog_user_control_ble.exe
```

WS 上位机：

```powershell
.\build\Release\aidog_user_control_ws.exe
```

两个上位机的控制界面保持一致，WS 版只把连接方式替换为 Dev PC WebSocket 主机模式。

## 文档

- [快速开始](docs/quick_start.zh_CN.md)
- [API 参考](docs/api_reference.md)
- [BLE 连接说明](docs/connection_ble.md)
- [Dev PC WebSocket](docs/dev_pc_websocket.md)
- [固件兼容性](docs/firmware_compatibility.md)
- [故障排查](docs/troubleshooting.md)
- [示例索引](examples/README.md)

## 项目结构

```text
aidog_sdk_cpp/
  include/aidog/       Public C++ headers
  src/                 SDK implementation
  examples/            Example programs
  tools/               Windows GUI tools
  tests/               SDK tests
  docs/                Design notes and assets
```

## 许可协议

Apache-2.0。详见 [LICENSE](LICENSE)。
