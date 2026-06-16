#include "aidog/aidog.hpp"

#include <algorithm>
#include <chrono>
#include <cstring>
#include <stdexcept>
#include <thread>

#include "aidog/ble_transport_win.hpp"
#include "aidog/error.hpp"
#include "aidog/protocol.hpp"
#include "aidog/sensor_parser.hpp"
#include "aidog/websocket_host.hpp"

namespace aidog {
namespace {

constexpr std::uint8_t kSpecialDetectionEnable = 100;
constexpr std::uint8_t kSpecialDetectionDisable = 101;
constexpr int kControlJsonEduMode = 5;

std::vector<std::uint8_t> _one_byte(std::uint8_t value)
{
    return {value};
}

int _clamp_int(int value, int minValue, int maxValue)
{
    return std::max(minValue, std::min(maxValue, value));
}

std::string _bytes_to_text(std::span<const std::uint8_t> data)
{
    return std::string(reinterpret_cast<const char*>(data.data()), data.size());
}

} // namespace

AiDog::AiDog(bool autoEdu, int eduLeaseMs)
    : autoEdu_(autoEdu), eduLeaseMs_(std::max(1000, eduLeaseMs))
{
    ble_ = std::make_unique<BleTransportWin>([this](const std::vector<std::uint8_t>& data) {
        feed_notify_bytes(data);
    });
}

AiDog::~AiDog()
{
    shutdown();
}

std::vector<DeviceInfo> AiDog::scan(const std::string& namePrefix)
{
    return ble_->scan(namePrefix);
}

void AiDog::connect(const ConnectOptions& options)
{
    auto address = options.address;
    if (!address.has_value()) {
        auto devices = ble_->scan(options.namePrefix);
        if (devices.empty()) {
            throw ConnectionError("no device found with requested name prefix");
        }
        address = devices.front().address;
    }

    if (!ble_->connect(*address, options.retries, options.retryDelayS)) {
        throw ConnectionError("failed to connect BLE device");
    }
    if (autoEdu_) {
        enter_edu_mode("ble", eduLeaseMs_);
    }
}

void AiDog::disconnect()
{
    exit_edu_mode();
    ble_->disconnect();
}

void AiDog::shutdown()
{
    if (shutdownDone_.exchange(true)) {
        return;
    }
    exit_edu_mode();
    if (ble_) {
        ble_->shutdown();
    }
}

bool AiDog::is_connected() const
{
    return ble_ && ble_->is_connected();
}

void AiDog::attach_ws_control(WebSocketHost* host)
{
    wsControl_ = host;
}

bool AiDog::perform_action(Action action, const ActionOptions& options)
{
    wait_until_interaction_idle(6.0);

    std::optional<int> actionParam;
    if (options.count.has_value() && is_count_based(action)) {
        actionParam = _clamp_int(*options.count, 1, 255);
    } else if (options.duration.has_value() && is_timer_based(action)) {
        actionParam = _clamp_int(*options.duration, 1, 255);
    } else if (options.angle.has_value() && is_angle_based(action)) {
        actionParam = _clamp_int(*options.angle, 1, 360);
    }

    auto waitTimeout = options.timeoutS;
    if (actionParam.has_value() && is_timer_based(action)) {
        waitTimeout = std::max(waitTimeout, static_cast<double>(*actionParam) + 3.0);
    } else if (actionParam.has_value() && is_count_based(action)) {
        waitTimeout = std::max(waitTimeout, static_cast<double>(*actionParam) * 4.0 + 3.0);
    } else if (actionParam.has_value() && is_angle_based(action)) {
        waitTimeout = std::max(waitTimeout, 15.0);
    }

    std::uint64_t startStatusSeq = 0;
    {
        std::lock_guard<std::mutex> lock(stateMutex_);
        startStatusSeq = interactionStatusNotifySeq_;
    }

    send_interaction(action, actionParam, options.transport);

    const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(static_cast<int>(waitTimeout * 1000.0));
    bool seenRunning = false;
    bool started = false;
    while (std::chrono::steady_clock::now() < deadline) {
        std::unique_lock<std::mutex> lock(stateMutex_);
        stateCv_.wait_for(lock, std::chrono::milliseconds(200));
        const auto status = lastInteractionTaskStatus_;
        const bool statusTickChanged = interactionStatusNotifySeq_ > startStatusSeq;

        if (status == 1) {
            seenRunning = true;
            started = true;
        } else if (status.has_value() && *status != 0) {
            started = true;
        }
        if (status == 0 && statusTickChanged) {
            if (!options.requireRunningState || seenRunning || started) {
                return true;
            }
        }
        if (status == 2) {
            return false;
        }
    }
    return false;
}

bool AiDog::perform_action(const std::string& actionName, const ActionOptions& options)
{
    return perform_action(resolve_action(actionName), options);
}

void AiDog::send_interaction(Action action, std::optional<int> param, const std::string& transport)
{
    auto data = make_interaction_data(action, param);
    send_control(ModeInteraction, data, transport);
}

void AiDog::send_movement(Movement direction, std::optional<double> durationS, const std::string& transport)
{
    auto data = make_movement_data(direction);
    send_control(ModeSport, data, transport);
    if (durationS.has_value()) {
        if (*durationS <= 0.0) {
            throw std::invalid_argument("durationS must be positive");
        }
        try {
            std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(*durationS * 1000.0)));
        } catch (...) {
            stop_movement(transport);
            throw;
        }
        stop_movement(transport);
    }
}

void AiDog::start_movement(Movement direction, const std::string& transport)
{
    send_movement(direction, std::nullopt, transport);
}

void AiDog::stop_movement(const std::string& transport)
{
    auto data = make_movement_data(Movement::Forward);
    data[1] = 5;
    send_control(ModeSport, data, transport);
}

void AiDog::reset(const std::string& transport)
{
    stop_movement(transport);
    send_interaction(Action::StopInteraction, std::nullopt, transport);
}

void AiDog::send_ear(EarAction action, const std::string& transport)
{
    auto data = _one_byte(static_cast<std::uint8_t>(action));
    send_control(ModeEar, data, transport);
}

void AiDog::send_ear_percentage(int percentage, const std::string& transport)
{
    std::vector<std::uint8_t> data{14, static_cast<std::uint8_t>(_clamp_int(percentage, 0, 100))};
    send_control(ModeEar, data, transport);
}

void AiDog::set_special_detection(bool enable, const std::string& transport)
{
    auto data = _one_byte(enable ? kSpecialDetectionEnable : kSpecialDetectionDisable);
    send_control(ModeEar, data, transport);
}

void AiDog::enable_special_detection(const std::string& transport)
{
    set_special_detection(true, transport);
}

void AiDog::disable_special_detection(const std::string& transport)
{
    set_special_detection(false, transport);
}

void AiDog::set_tof_enable(bool enable)
{
    nlohmann::json payload{{"mode", kControlJsonEduMode}, {"tof_enable", enable ? 1 : 0}};
    ble_->send_control_json(payload);
}

void AiDog::send_expression(ExpressionAction expression, const std::string& transport)
{
    auto data = _one_byte(static_cast<std::uint8_t>(expression));
    send_control(ModeExpression, data, transport);
}

void AiDog::send_audio(Tone tone, const std::string& transport)
{
    auto data = _one_byte(static_cast<std::uint8_t>(tone));
    send_control(ModeAudio, data, transport);
}

void AiDog::set_volume(int volume, std::optional<Tone> verifyTone, double verifyDelayS, const std::string& transport)
{
    nlohmann::json config{{"cmd", ConfigSetVolume}, {"volume", _clamp_int(volume, 0, 4)}};
    transport_for(transport).send_config(config);
    if (verifyTone.has_value()) {
        if (verifyDelayS > 0.0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(verifyDelayS * 1000.0)));
        }
        send_audio(*verifyTone, transport);
    }
}

void AiDog::request_imu_stream(bool enable, int hz, const std::string& transport)
{
    std::vector<std::uint8_t> data;
    if (enable) {
        data = {0x01, static_cast<std::uint8_t>(_clamp_int(hz, 1, 200))};
    } else {
        data = {0x00};
    }
    send_control(ModeSensor, data, transport);
}

void AiDog::request_tof_stream(bool enable, int hz, const std::string& transport)
{
    std::vector<std::uint8_t> data;
    if (enable) {
        data = {0x02, static_cast<std::uint8_t>(_clamp_int(hz, 1, 200))};
    } else {
        data = {0x03};
    }
    send_control(ModeSensor, data, transport);
}

void AiDog::send_raw(std::uint8_t mode, std::span<const std::uint8_t> data, const std::string& transport)
{
    send_control(mode, data, transport);
}

nlohmann::json AiDog::get_action_list()
{
    return ble_->read_actions();
}

void AiDog::syn_pose_adjust(std::span<const PoseAdjustItem> poses, int durationMs)
{
    auto data = make_pose_adjust_data(poses, durationMs);
    send_control(ModeRobotAdjust, data, "ble");
}

void AiDog::syn_foot_adjust(std::span<const DeltaAdjustItem> feet, int durationMs)
{
    auto data = make_foot_adjust_data(feet, durationMs);
    send_control(ModeRobotAdjust, data, "ble");
}

void AiDog::syn_joint_adjust(std::span<const DeltaAdjustItem> joints, int durationMs)
{
    auto data = make_joint_adjust_data(joints, durationMs);
    send_control(ModeRobotAdjust, data, "ble");
}

void AiDog::default_pose_output(float roll, float pitch, float x, float z)
{
    auto data = make_default_pose_data(roll, pitch, x, z);
    send_control(ModeRobotAdjust, data, "ble");
}

void AiDog::request_basic_mode()
{
    std::vector<std::uint8_t> data{0x04, 0, 0, 0};
    send_control(ModeRobotAdjust, data, "ble");
}

void AiDog::enter_edu_mode(const std::string& transport, int leaseMs)
{
    exit_edu_mode();
    std::lock_guard<std::mutex> lock(eduMutex_);
    ++eduSessionSeq_;
    eduTransport_ = transport;
    eduLeaseMs_ = std::max(1000, leaseMs);
    eduMaybeActive_ = true;
    send_edu_session("enter", eduTransport_, eduLeaseMs_);
    eduStop_.store(false);
    auto sessionSeq = eduSessionSeq_;
    eduThread_ = std::thread([this, sessionSeq]() {
        edu_heartbeat_loop(sessionSeq);
    });
}

void AiDog::exit_edu_mode()
{
    std::unique_lock<std::mutex> lock(eduMutex_);
    const auto transport = eduTransport_;
    const auto leaseMs = eduLeaseMs_;
    const auto shouldSendExit = eduMaybeActive_;
    ++eduSessionSeq_;
    eduStop_.store(true);
    auto thread = std::move(eduThread_);
    eduMaybeActive_ = false;
    lock.unlock();

    if (thread.joinable()) {
        thread.join();
    }
    if (!shouldSendExit) {
        return;
    }
    try {
        send_edu_session("exit", transport, leaseMs);
    } catch (...) {
    }
}

std::optional<ImuData> AiDog::get_latest_imu() const
{
    std::lock_guard<std::mutex> lock(stateMutex_);
    return lastImu_;
}

std::optional<TofData> AiDog::get_latest_tof() const
{
    std::lock_guard<std::mutex> lock(stateMutex_);
    return lastTof_;
}

int AiDog::add_imu_listener(ImuCallback callback)
{
    std::lock_guard<std::mutex> lock(stateMutex_);
    const auto id = nextCallbackId_++;
    imuCallbacks_.emplace_back(id, std::move(callback));
    return id;
}

int AiDog::add_tof_listener(TofCallback callback)
{
    std::lock_guard<std::mutex> lock(stateMutex_);
    const auto id = nextCallbackId_++;
    tofCallbacks_.emplace_back(id, std::move(callback));
    return id;
}

void AiDog::remove_imu_listener(int id)
{
    std::lock_guard<std::mutex> lock(stateMutex_);
    std::erase_if(imuCallbacks_, [id](const auto& item) {
        return item.first == id;
    });
}

void AiDog::remove_tof_listener(int id)
{
    std::lock_guard<std::mutex> lock(stateMutex_);
    std::erase_if(tofCallbacks_, [id](const auto& item) {
        return item.first == id;
    });
}

void AiDog::feed_sensor_stream_json(std::string_view text)
{
    dispatch_parsed_notify(parse_notify_json_text(text));
}

void AiDog::feed_notify_bytes(std::span<const std::uint8_t> data)
{
    feed_sensor_stream_json(_bytes_to_text(data));
}

void AiDog::on_ws_robot_connected()
{
    if (autoEdu_) {
        enter_edu_mode("ws", eduLeaseMs_);
    }
}

void AiDog::on_ws_robot_disconnected()
{
    if (eduTransport_ == "ws") {
        exit_edu_mode();
    }
}

ITransport& AiDog::transport_for(const std::string& transport)
{
    if (transport == "ble") {
        return *ble_;
    }
    if (transport == "ws") {
        if (wsControl_ == nullptr) {
            throw ConnectionError("WebSocket control is not attached");
        }
        return *wsControl_;
    }
    throw UnsupportedError("unsupported transport");
}

void AiDog::send_control(std::uint8_t mode, std::span<const std::uint8_t> data, const std::string& transport)
{
    if (transport == "ws") {
        if (wsControl_ == nullptr) {
            throw ConnectionError("WebSocket control is not attached");
        }
        ++wsControlSeq_;
        auto commandId = "dog-" + std::to_string(static_cast<int>(mode)) + "-" + std::to_string(wsControlSeq_);
        wsControl_->send_control_raw(mode, data, commandId);
        const bool isEarPercentage =
            mode == ModeEar && data.size() >= 2 && data[0] == 14;
        if (isEarPercentage) {
            return;
        }
        auto ack = wsControl_->wait_ack(commandId, 1.2);
        if (!ack.has_value()) {
            throw TimeoutError("WebSocket control ack timeout");
        }
        if (ack->value("result", "") != "accepted") {
            throw ProtocolError("WebSocket control rejected");
        }
        return;
    }
    transport_for(transport).send_control(mode, data);
}

void AiDog::send_edu_session(const std::string& action, const std::string& transport, int leaseMs)
{
    if (transport == "ble") {
        nlohmann::json payload{{"mode", kControlJsonEduMode}, {"lease_ms", std::max(1000, leaseMs)}};
        if (action == "heartbeat") {
            payload["edu_heartbeat"] = 1;
        } else {
            payload["edu_enable"] = action == "enter" ? 1 : 0;
        }
        ble_->send_control_json(payload);
        return;
    }

    if (transport == "ws") {
        if (wsControl_ == nullptr) {
            throw ConnectionError("WebSocket control is not attached");
        }
        ++wsControlSeq_;
        auto commandId = "dog-edu-" + std::to_string(wsControlSeq_);
        wsControl_->send_edu_session(action, leaseMs, commandId);
        auto ack = wsControl_->wait_ack(commandId, 1.2);
        if (!ack.has_value()) {
            throw TimeoutError("WebSocket EDU session ack timeout");
        }
        if (ack->value("result", "") != "accepted") {
            throw ProtocolError("WebSocket EDU session rejected");
        }
        return;
    }

    throw UnsupportedError("unsupported EDU transport");
}

void AiDog::edu_heartbeat_loop(std::uint64_t sessionSeq)
{
    const auto interval = std::chrono::milliseconds(std::max(500, eduLeaseMs_ / 3));
    while (!eduStop_.load()) {
        std::this_thread::sleep_for(interval);
        if (eduStop_.load()) {
            return;
        }
        {
            std::lock_guard<std::mutex> lock(eduMutex_);
            if (sessionSeq != eduSessionSeq_) {
                return;
            }
        }
        try {
            send_edu_session("heartbeat", eduTransport_, eduLeaseMs_);
        } catch (...) {
        }
    }
}

bool AiDog::wait_until_interaction_idle(double timeoutS)
{
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(static_cast<int>(timeoutS * 1000.0));
    std::unique_lock<std::mutex> lock(stateMutex_);
    while (std::chrono::steady_clock::now() < deadline) {
        if (!lastInteractionTaskStatus_.has_value() || lastInteractionTaskStatus_ == 0) {
            return true;
        }
        stateCv_.wait_for(lock, std::chrono::milliseconds(100));
    }
    return false;
}

void AiDog::dispatch_parsed_notify(const ParsedNotify& parsed)
{
    std::vector<ImuCallback> imuCallbacks;
    std::vector<TofCallback> tofCallbacks;
    {
        std::lock_guard<std::mutex> lock(stateMutex_);
        if (parsed.interactionTaskStatus.has_value()) {
            ++interactionStatusNotifySeq_;
            lastInteractionTaskStatus_ = parsed.interactionTaskStatus;
        }
        if (parsed.imu.has_value()) {
            lastImu_ = parsed.imu;
            for (const auto& [_, callback] : imuCallbacks_) {
                imuCallbacks.push_back(callback);
            }
        }
        if (parsed.tof.has_value()) {
            lastTof_ = parsed.tof;
            for (const auto& [_, callback] : tofCallbacks_) {
                tofCallbacks.push_back(callback);
            }
        }
    }
    stateCv_.notify_all();

    if (parsed.imu.has_value()) {
        for (const auto& callback : imuCallbacks) {
            if (callback) {
                callback(*parsed.imu);
            }
        }
    }
    if (parsed.tof.has_value()) {
        for (const auto& callback : tofCallbacks) {
            if (callback) {
                callback(*parsed.tof);
            }
        }
    }
}

} // namespace aidog
