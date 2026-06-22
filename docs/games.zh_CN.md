# 小游戏

Python SDK 中包含基于 pygame 的 PC 侧小游戏，可以使用键盘、BLE IMU 或 WebSocket IMU 作为输入。C++ SDK 当前还没有独立的小游戏可执行程序，但已经提供了构建同类 C++ 游戏所需的 BLE / WebSocket IMU 数据链路。

## Python 参考游戏

| 游戏 | Python 入口脚本 | 控制模式 | 说明 |
| --- | --- | --- | --- |
| 平衡球 | `../../aidog_sdk/game/balance_ball/aidog_balance_ball_game.py` | 键盘、BLE IMU、WebSocket IMU | 通过倾斜机器狗保持小球平衡。 |
| 打砖块 | `../../aidog_sdk/game/brick_breaker/aidog_brick_breaker_game.py` | 键盘、BLE IMU、WebSocket IMU | 用机器狗横滚控制挡板。 |
| 星际飞机 | `../../aidog_sdk/game/space_fighter/aidog_space_fighter_game.py` | 键盘、WebSocket IMU | 控制飞机移动、击败敌人和 Boss。 |

## 界面预览

### 平衡球

<p align="center">
  <img src="assets/images/balance_ball_game.png" alt="Balance Ball game UI" width="75%">
</p>

### 打砖块

<p align="center">
  <img src="assets/images/brick_breaker_game.png" alt="Brick Breaker game UI" width="75%">
</p>

### 星际飞机

<p align="center">
  <img src="assets/images/space_fighter_game.png" alt="Space Fighter game UI" width="75%">
</p>

## C++ 游戏可用的传感器输入

BLE IMU：

```powershell
.\build\Release\aidog_ble_imu_read.exe --address AA:BB:CC:DD:EE:FF --hz 20 --seconds 10
```

WebSocket IMU：

```powershell
.\build\Release\aidog_ws_imu_lan_read.exe --bind 0.0.0.0 --port 8766 --hz 20 --seconds 10 --connect-timeout 120
```

C++ 回调方式：

```cpp
dog.add_imu_listener([](const aidog::ImuData& imu) {
    // 可以使用 imu.rollDeg 或 imu.pitchDeg 作为游戏输入。
});
```

## 后续 C++ 游戏建议

后续如果开发 C++ 小游戏，建议把游戏循环和机器狗通信分离：

- BLE 或 WS 输入适配器负责读取 `ImuData`。
- 游戏逻辑只消费归一化后的 roll / pitch。
- 音频、耳朵、表情等机器狗反馈作为可选输出，并限制发送频率。

## 安全说明

- 键盘模式是最安全的第一次试玩方式。
- 使用机器狗 IMU 控制时，请把机器狗放在稳定位置，避免运动时触碰腿部。
- 在输入和停止逻辑验证前，不要从游戏循环里直接下发运动命令。
