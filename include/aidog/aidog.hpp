#pragma once

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <optional>
#include <thread>
#include <vector>

#include "aidog/actions.hpp"
#include "aidog/transport.hpp"
#include "aidog/types.hpp"

namespace aidog {

class BleTransportWin;
class WebSocketHost;

class AiDog {
public:
    explicit AiDog(bool autoEdu = true, int eduLeaseMs = 8000);
    ~AiDog();

    AiDog(const AiDog&) = delete;
    AiDog& operator=(const AiDog&) = delete;

    std::vector<DeviceInfo> scan(const std::string& namePrefix = "Gogobot");
    void connect(const ConnectOptions& options = {});
    void disconnect();
    void shutdown();
    bool is_connected() const;

    void attach_ws_control(WebSocketHost* host);

    bool perform_action(Action action, const ActionOptions& options = {});
    bool perform_action(const std::string& actionName, const ActionOptions& options = {});
    void send_interaction(Action action, std::optional<int> param = std::nullopt, const std::string& transport = "ble");
    void send_movement(Movement direction, std::optional<double> durationS = std::nullopt, const std::string& transport = "ble");
    void start_movement(Movement direction, const std::string& transport = "ble");
    void stop_movement(const std::string& transport = "ble");
    void reset(const std::string& transport = "ble");
    void send_ear(EarAction action, const std::string& transport = "ble");
    void send_ear_percentage(int percentage, const std::string& transport = "ble");
    void set_special_detection(bool enable, const std::string& transport = "ble");
    void enable_special_detection(const std::string& transport = "ble");
    void disable_special_detection(const std::string& transport = "ble");
    void set_tof_enable(bool enable);
    void send_expression(ExpressionAction expression, const std::string& transport = "ble");
    void send_audio(Tone tone, const std::string& transport = "ble");
    void set_volume(int volume, std::optional<Tone> verifyTone = std::nullopt, double verifyDelayS = 0.2, const std::string& transport = "ble");
    void set_dev_pc_ws_ip(const std::string& ip);
    void request_imu_stream(bool enable = true, int hz = 20, const std::string& transport = "ble");
    void request_tof_stream(bool enable = true, int hz = 20, const std::string& transport = "ble");
    void send_raw(std::uint8_t mode, std::span<const std::uint8_t> data, const std::string& transport = "ble");
    nlohmann::json get_action_list();

    void syn_pose_adjust(std::span<const PoseAdjustItem> poses, int durationMs);
    void syn_foot_adjust(std::span<const DeltaAdjustItem> feet, int durationMs);
    void syn_joint_adjust(std::span<const DeltaAdjustItem> joints, int durationMs);
    void default_pose_output(float roll, float pitch, float x, float z);
    void request_basic_mode();

    void enter_edu_mode(const std::string& transport = "ble", int leaseMs = 8000);
    void exit_edu_mode();

    std::optional<ImuData> get_latest_imu() const;
    std::optional<TofData> get_latest_tof() const;
    int add_imu_listener(ImuCallback callback);
    int add_tof_listener(TofCallback callback);
    void remove_imu_listener(int id);
    void remove_tof_listener(int id);
    bool wait_interaction_ready(double timeoutS = 5.0);
    void feed_sensor_stream_json(std::string_view text);
    void feed_notify_bytes(std::span<const std::uint8_t> data);

    void on_ws_robot_connected();
    void on_ws_robot_disconnected();

private:
    ITransport& transport_for(const std::string& transport);
    void send_control(std::uint8_t mode, std::span<const std::uint8_t> data, const std::string& transport);
    void send_edu_session(const std::string& action, const std::string& transport, int leaseMs);
    void edu_heartbeat_loop(std::uint64_t sessionSeq);
    bool wait_until_interaction_idle(double timeoutS);
    void dispatch_parsed_notify(const ParsedNotify& parsed);

    bool autoEdu_;
    std::atomic_bool shutdownDone_{false};
    mutable std::mutex stateMutex_;
    std::condition_variable stateCv_;
    std::optional<int> lastInteractionTaskStatus_;
    std::uint64_t interactionStatusNotifySeq_ = 0;
    std::optional<ImuData> lastImu_;
    std::optional<TofData> lastTof_;
    std::vector<std::pair<int, ImuCallback>> imuCallbacks_;
    std::vector<std::pair<int, TofCallback>> tofCallbacks_;
    int nextCallbackId_ = 1;

    std::unique_ptr<BleTransportWin> ble_;
    WebSocketHost* wsControl_ = nullptr;
    std::uint64_t wsControlSeq_ = 0;

    std::mutex eduMutex_;
    std::thread eduThread_;
    std::atomic_bool eduStop_{false};
    std::uint64_t eduSessionSeq_ = 0;
    std::string eduTransport_ = "ble";
    int eduLeaseMs_ = 8000;
    bool eduMaybeActive_ = false;
};

} // namespace aidog
