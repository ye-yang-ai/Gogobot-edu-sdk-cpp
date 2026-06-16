#pragma once

#include <functional>
#include <memory>
#include <span>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "aidog/transport.hpp"
#include "aidog/types.hpp"

namespace aidog {

class BleTransportWin : public ITransport {
public:
    using NotifyCallback = std::function<void(const std::vector<std::uint8_t>&)>;

    explicit BleTransportWin(NotifyCallback onNotify = {});
    ~BleTransportWin() override;

    BleTransportWin(const BleTransportWin&) = delete;
    BleTransportWin& operator=(const BleTransportWin&) = delete;

    std::vector<DeviceInfo> scan(const std::string& namePrefix);
    bool connect(const std::string& address, int retries = 3, double retryDelayS = 1.0);
    void disconnect();
    void shutdown();
    nlohmann::json read_actions();

    void send_control(std::uint8_t mode, std::span<const std::uint8_t> data) override;
    void send_control_json(const nlohmann::json& payload) override;
    void send_config(const nlohmann::json& config) override;
    bool is_connected() const override;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace aidog
