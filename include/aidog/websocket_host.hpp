#pragma once

#include <condition_variable>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <vector>

#include <nlohmann/json.hpp>

#include "aidog/transport.hpp"
#include "aidog/types.hpp"

namespace aidog {

class AiDog;

class WebSocketHost : public ITransport {
public:
    WebSocketHost(std::string host = "0.0.0.0", int port = 8766, AiDog* dog = nullptr);
    ~WebSocketHost() override;

    WebSocketHost(const WebSocketHost&) = delete;
    WebSocketHost& operator=(const WebSocketHost&) = delete;

    void set_imu_callback(ImuCallback callback);
    void set_tof_callback(TofCallback callback);
    void set_pcm_callback(PcmCallback callback);
    void start(double waitReadyS = 5.0);
    void stop();
    bool wait_robot_connected(double timeoutS = 30.0, double pollS = 0.1) const;
    bool is_robot_connected() const;

    void send_text(const std::string& text);
    void send_control_raw(std::uint8_t mode, std::span<const std::uint8_t> data, const std::string& commandId = {});
    void send_control(std::uint8_t mode, std::span<const std::uint8_t> data) override;
    void send_control_json(const nlohmann::json& payload) override;
    void send_config(const nlohmann::json& config) override;
    void send_edu_session(const std::string& action, int leaseMs, const std::string& commandId = {});
    std::optional<nlohmann::json> wait_ack(const std::string& commandId, double timeoutS = 1.0);
    bool is_connected() const override;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace aidog
