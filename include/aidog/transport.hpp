#pragma once

#include <cstdint>
#include <span>

#include <nlohmann/json.hpp>

namespace aidog {

class ITransport {
public:
    virtual ~ITransport() = default;
    virtual void send_control(std::uint8_t mode, std::span<const std::uint8_t> data) = 0;
    virtual void send_control_json(const nlohmann::json& payload) = 0;
    virtual void send_config(const nlohmann::json& config) = 0;
    virtual bool is_connected() const = 0;
};

} // namespace aidog
