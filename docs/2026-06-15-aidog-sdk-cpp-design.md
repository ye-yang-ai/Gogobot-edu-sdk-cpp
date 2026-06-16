# aidog_sdk_cpp 设计方案

## 背景

Python 版 `aidog_sdk` 已经形成一套完整能力：BLE 控制、Dev PC WebSocket 控制、动作参数封装、EDU 模式保活、IMU/TOF 状态缓存、PCM 音频通道和高级姿态调节。C++ 版第一版目标是完整替代 Python SDK，并优先支持 Windows，不支持 Linux 和 macOS。

## 目标

- 提供原生 C++ SDK，不依赖 Python 运行时。
- 第一版同时支持 Windows BLE 和 Dev PC WebSocket。
- 高层 API 尽量对齐 Python 版 `AiDog`，降低迁移成本。
- BLE 和 WebSocket 共用同一套协议编码，避免行为分叉。
- 协议、JSON 解析、状态等待、ACK 路由必须可单元测试。

## 非目标

- 第一版不支持 Linux 和 macOS。
- 第一版不实现 GUI 上位机。
- 第一版不重写固件协议，只复刻并封装现有 Python SDK 暴露的协议。
- 第一版不把小游戏迁移到 C++。

## 技术路线

采用原生 C++ SDK + Windows BLE + WebSocket 完整实现。

- 构建系统：CMake + MSVC。
- BLE：Windows C++/WinRT。
- JSON：`nlohmann/json`。
- WebSocket：Boost.Asio + Boost.Beast。
- 测试：优先 `doctest`，轻量覆盖协议和状态逻辑。

选择 Windows C++/WinRT 是因为第一版只面向 Windows，可以直接使用系统 BLE 能力，减少跨平台抽象成本。选择 Boost.Beast 是因为它能覆盖 WebSocket server、异步 IO 和二进制帧，适合复刻 Python 版 `DevPcWebSocketHost`。

## 目录结构

```text
aidog_sdk_cpp/
  CMakeLists.txt
  include/aidog/
    aidog.hpp
    actions.hpp
    ble_transport_win.hpp
    error.hpp
    protocol.hpp
    sensor_parser.hpp
    transport.hpp
    types.hpp
    websocket_host.hpp
  src/
    aidog.cpp
    actions.cpp
    ble_transport_win.cpp
    protocol.cpp
    sensor_parser.cpp
    websocket_host.cpp
  examples/
    ble_basic_actions.cpp
    ble_imu_read.cpp
    ws_basic_actions.cpp
    ws_imu_read.cpp
  tests/
    test_actions.cpp
    test_protocol.cpp
    test_sensor_parser.cpp
    test_websocket_ack.cpp
  docs/
    2026-06-15-aidog-sdk-cpp-design.md
```

## 模块设计

### `AiDog`

`AiDog` 是用户入口，负责连接生命周期、高层控制、状态缓存和 EDU 会话。

主要接口：

- `scan(namePrefix)`
- `connect(options)`
- `disconnect()`
- `shutdown()`
- `perform_action(action, options)`
- `send_interaction(action, param)`
- `send_movement(direction, duration)`
- `start_movement(direction)`
- `stop_movement()`
- `reset()`
- `send_ear(action)`
- `send_ear_percentage(percent)`
- `send_expression(expression)`
- `send_audio(tone)`
- `set_volume(volume, verifyTone)`
- `request_imu_stream(enable, hz)`
- `request_tof_stream(enable, hz)`
- `get_latest_imu()`
- `get_latest_tof()`
- `add_imu_listener(callback)`
- `add_tof_listener(callback)`
- `enter_edu_mode(transport, leaseMs)`
- `exit_edu_mode()`
- `send_raw(mode, data)`
- `get_action_list()`
- `syn_pose_adjust(...)`
- `syn_foot_adjust(...)`
- `syn_joint_adjust(...)`
- `default_pose_output(...)`
- `request_basic_mode()`

### `Protocol`

`Protocol` 只负责字节和 JSON 编码，不持有连接状态。

职责：

- 生成 BLE raw packet：`AA 55 00 mode data...`
- 生成 WebSocket `control_raw` JSON。
- 生成 WebSocket `config_json` JSON。
- 生成 WebSocket `edu_session` JSON。
- 编码 interaction 参数，包含时间、次数、角度。
- 编码 movement 长包。
- 编码 robot adjust 小端浮点 payload。

### `ITransport`

高层 SDK 通过传输抽象发送控制命令。

建议接口：

```cpp
class ITransport {
public:
    virtual ~ITransport() = default;
    virtual void send_control(uint8_t mode, std::span<const uint8_t> data) = 0;
    virtual void send_config(const nlohmann::json& config) = 0;
    virtual bool is_connected() const = 0;
};
```

BLE 和 WebSocket 可以分别实现这个接口。`AiDog` 保留默认 BLE 传输，同时允许挂载 WebSocket host。

### `BleTransportWin`

Windows BLE 传输层，功能对齐 Python `_ble.py`。

职责：

- 扫描名称前缀为 `Gogobot` 的设备。
- 连接指定 BLE 地址。
- 发现 `ae01`、`ae02`、`ae03`、`ae04`、`ae10`。
- 对 `ae02` 和 `ae04` 订阅 notify/indicate。
- 向 `ae03` 写控制包和控制 JSON。
- 向 `ae01` 写 config JSON。
- 从 `ae10` 读取 action list JSON。
- 提供写重试和连接重试。

### `WebSocketHost`

PC 端 WebSocket server，功能对齐 Python `DevPcWebSocketHost`。

职责：

- 监听 `0.0.0.0:8766` 等配置地址。
- 等待机器狗作为 client 连接。
- 接收文本帧：ACK、IMU/TOF JSON。
- 接收二进制帧：PCM。
- 发送 `control_raw`、`config_json`、`edu_session`。
- 根据 command id 等待 ACK。
- 连接建立后触发 `AiDog` 自动进入 EDU 模式。
- 断开后触发 EDU 退出逻辑。

### `SensorParser`

传感器解析层独立于传输层。

职责：

- 解析 notify JSON。
- 提取 `interaction_task_status`。
- 提取并规范化 IMU。
- 提取 TOF。
- 将毫度格式转换为度，并生成 `yawDeg`、`pitchDeg`、`rollDeg`。

## 枚举和动作元数据

`actions.hpp` 对齐 Python `actions.py`：

- `Action`
- `Movement`
- `EarAction`
- `ExpressionAction`
- `Tone`
- `ParameterType`
- `ActionSpec`
- `resolve_action`
- `timer_based`
- `count_based`
- `angle_based`

隐藏固件 ID `6`、`47`、`48`、`49`、`50`、`51` 继续映射为 `IDLE`。

## 状态等待策略

`perform_action` 继续依赖 `interaction_task_status`：

- `0`：未运行或已完成。
- `1`：运行中。
- `2`：被杀死或中断。

发送新动作前先等待旧动作空闲。发送动作后记录 notify 序号，只有收到新的状态变更后才把 idle 当作完成。时间类、次数类、角度类动作根据参数扩展等待超时。

## EDU 模式

默认 `AiDog` 自动进入 EDU 模式，并用后台线程保活。

- BLE：向 `ae03` 写控制 JSON。
- WebSocket：发送 `edu_session` JSON 并等待 ACK。
- `lease_ms` 最小值为 1000。
- `shutdown()` 和 `disconnect()` 必须停止 heartbeat 并尝试发送 exit。

## 错误处理

第一版使用异常表达失败：

- `AidogError`
- `ConnectionError`
- `TimeoutError`
- `ProtocolError`
- `UnsupportedError`

底层 Windows BLE 错误需要包成 SDK 异常，并保留原始错误信息。

## 测试策略

无需机器狗即可完成的测试必须先覆盖：

- raw packet 编码。
- WebSocket JSON 编码。
- action 参数 clamp 和字节序。
- movement 长包长度和关键字段。
- robot adjust 小端浮点编码。
- IMU 毫度转角度。
- notify JSON 解析。
- `interaction_task_status` 状态更新。
- WebSocket ACK 路由。

需要机器狗的测试放入 `examples/` 和文档，不纳入默认单元测试。

## 分阶段实施

### 阶段 1：工程骨架和纯逻辑

- 建立 CMake 工程。
- 建立 public include 和 src 结构。
- 实现 `actions`、`protocol`、`sensor_parser`。
- 加入 doctest 单元测试。

### 阶段 2：WebSocket

- 实现 `WebSocketHost`。
- 支持连接、文本帧、二进制帧、ACK、控制命令。
- 实现 WS 示例：动作、IMU。

### 阶段 3：Windows BLE

- 实现扫描、连接、特征发现、notify、write、read。
- 对齐 `ae01/ae02/ae03/ae04/ae10`。
- 实现 BLE 示例：连接、动作、IMU。

### 阶段 4：高层整合

- 实现 `AiDog`。
- 整合 BLE/WS 传输选择。
- 实现 EDU 自动进入和 heartbeat。
- 对齐 Python 版高层 API。

### 阶段 5：验收

- 默认单元测试通过。
- WS 示例能等待机器狗连接并执行动作。
- BLE 示例能扫描、连接并执行安全动作。
- IMU/TOF listener 能收到数据。
- `perform_action` 能根据状态返回完成结果。

## 风险

- Windows BLE API 较繁琐，扫描缓存、设备配对状态和 GATT 服务发现可能需要多轮调试。
- WebSocket 控制依赖固件启用 `DEV_PC_AUDIO_WS_ENABLE` 并配置正确 PC IP。
- 部分旧固件可能不持续上报 `interaction_task_status`，动作完成判断会退化。
- BLE notify 中 `ae02/ae04` 的 notify/indicate 属性可能因固件和 Windows 后端表现不同而变化。

## 验收标准

- C++ 示例代码能完成 Python quick start 的等价流程。
- BLE 和 WebSocket 发送同一个高层命令时，底层协议包一致。
- 传感器 JSON 在 BLE 和 WebSocket 两条路径解析结果一致。
- SDK 默认构建、测试、示例文档都能在 Windows/MSVC 下运行。
