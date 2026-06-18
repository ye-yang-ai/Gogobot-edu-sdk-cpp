#include "aidog/websocket_host.hpp"

#if !AIDOG_ENABLE_WEBSOCKET

#include "aidog/error.hpp"

namespace aidog {

struct WebSocketHost::Impl {};

WebSocketHost::WebSocketHost(std::string, int, AiDog*)
    : impl_(std::make_unique<Impl>())
{
}

WebSocketHost::~WebSocketHost() = default;

void WebSocketHost::set_imu_callback(ImuCallback) {}

void WebSocketHost::set_tof_callback(TofCallback) {}

void WebSocketHost::set_pcm_callback(PcmCallback) {}

void WebSocketHost::set_connection_callback(ConnectionCallback) {}

void WebSocketHost::start(double)
{
    throw UnsupportedError("WebSocket host is disabled");
}

void WebSocketHost::stop() {}

bool WebSocketHost::wait_robot_connected(double, double) const
{
    return false;
}

bool WebSocketHost::is_robot_connected() const
{
    return false;
}

void WebSocketHost::send_text(const std::string&)
{
    throw UnsupportedError("WebSocket host is disabled");
}

void WebSocketHost::send_control_raw(std::uint8_t, std::span<const std::uint8_t>, const std::string&)
{
    throw UnsupportedError("WebSocket host is disabled");
}

void WebSocketHost::send_control(std::uint8_t, std::span<const std::uint8_t>)
{
    throw UnsupportedError("WebSocket host is disabled");
}

void WebSocketHost::send_control_json(const nlohmann::json&)
{
    throw UnsupportedError("WebSocket host is disabled");
}

void WebSocketHost::send_config(const nlohmann::json&)
{
    throw UnsupportedError("WebSocket host is disabled");
}

void WebSocketHost::send_edu_session(const std::string&, int, const std::string&)
{
    throw UnsupportedError("WebSocket host is disabled");
}

std::optional<nlohmann::json> WebSocketHost::wait_ack(const std::string&, double)
{
    return std::nullopt;
}

bool WebSocketHost::is_connected() const
{
    return false;
}

} // namespace aidog

#endif
