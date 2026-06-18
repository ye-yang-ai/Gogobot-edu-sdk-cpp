#include "aidog/websocket_host.hpp"

#if AIDOG_ENABLE_WEBSOCKET

#include <chrono>
#include <exception>
#include <set>
#include <thread>

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include "aidog/aidog.hpp"
#include "aidog/error.hpp"
#include "aidog/protocol.hpp"
#include "aidog/sensor_parser.hpp"

namespace aidog {
namespace {

using Server = websocketpp::server<websocketpp::config::asio>;
using ConnectionHdl = websocketpp::connection_hdl;

std::vector<std::uint8_t> _payload_bytes(const std::string& payload)
{
    return std::vector<std::uint8_t>(payload.begin(), payload.end());
}

} // namespace

struct WebSocketHost::Impl {
    Impl(std::string bindHost, int bindPort, AiDog* dogPtr)
        : host(std::move(bindHost)), port(bindPort), dog(dogPtr)
    {
        server.clear_access_channels(websocketpp::log::alevel::all);
        server.clear_error_channels(websocketpp::log::elevel::all);
        server.init_asio();
        server.set_reuse_addr(true);

        server.set_open_handler([this](ConnectionHdl hdl) {
            {
                std::lock_guard<std::mutex> lock(mutex);
                active = hdl;
                connected = true;
            }
            cv.notify_all();
            if (onConnection) {
                onConnection(true);
            }
            if (dog != nullptr) {
                std::thread([dogPtr = dog]() {
                    dogPtr->on_ws_robot_connected();
                }).detach();
            }
        });

        server.set_close_handler([this](ConnectionHdl) {
            if (dog != nullptr && !stopping) {
                std::thread([dogPtr = dog]() {
                    dogPtr->on_ws_robot_disconnected();
                }).detach();
            }
            {
                std::lock_guard<std::mutex> lock(mutex);
                active.reset();
                connected = false;
            }
            cv.notify_all();
            if (onConnection) {
                onConnection(false);
            }
        });

        server.set_fail_handler([this](ConnectionHdl) {
            {
                std::lock_guard<std::mutex> lock(mutex);
                active.reset();
                connected = false;
            }
            cv.notify_all();
        });

        server.set_message_handler([this](ConnectionHdl, Server::message_ptr message) {
            if (message->get_opcode() == websocketpp::frame::opcode::text) {
                handle_text(message->get_payload());
            } else if (onPcm) {
                onPcm(_payload_bytes(message->get_payload()));
            }
        });
    }

    std::string host;
    int port = 8766;
    AiDog* dog = nullptr;
    mutable std::mutex mutex;
    mutable std::condition_variable cv;
    std::condition_variable ackCv;
    Server server;
    std::thread thread;
    std::optional<ConnectionHdl> active;
    bool ready = false;
    bool connected = false;
    bool stopping = false;
    std::exception_ptr startError;
    std::map<std::string, nlohmann::json> acks;
    ImuCallback onImu;
    TofCallback onTof;
    PcmCallback onPcm;
    ConnectionCallback onConnection;

    void run()
    {
        try {
            websocketpp::lib::error_code ec;
            auto address = asio::ip::make_address(host, ec);
            if (ec) {
                throw ConnectionError("invalid WebSocket bind address");
            }
            server.listen(asio::ip::tcp::endpoint(address, static_cast<unsigned short>(port)), ec);
            if (ec) {
                throw ConnectionError("WebSocket listen failed: " + ec.message());
            }
            server.start_accept(ec);
            if (ec) {
                throw ConnectionError("WebSocket accept failed: " + ec.message());
            }
            {
                std::lock_guard<std::mutex> lock(mutex);
                ready = true;
            }
            cv.notify_all();
            server.run();
        } catch (...) {
            std::lock_guard<std::mutex> lock(mutex);
            startError = std::current_exception();
            ready = true;
            connected = false;
            cv.notify_all();
        }
    }

    void handle_text(const std::string& text)
    {
        try {
            auto obj = nlohmann::json::parse(text);
            if (obj.is_object() && obj.value("type", "") == "ack") {
                auto id = obj.value("id", "");
                if (!id.empty()) {
                    {
                        std::lock_guard<std::mutex> lock(mutex);
                        acks[id] = obj;
                    }
                    ackCv.notify_all();
                }
                return;
            }
        } catch (...) {
        }

        if (dog != nullptr) {
            dog->feed_sensor_stream_json(text);
        }
        auto parsed = parse_notify_json_text(text);
        if (parsed.imu.has_value() && onImu) {
            onImu(*parsed.imu);
        }
        if (parsed.tof.has_value() && onTof) {
            onTof(*parsed.tof);
        }
    }
};

WebSocketHost::WebSocketHost(std::string host, int port, AiDog* dog)
    : impl_(std::make_unique<Impl>(std::move(host), port, dog))
{
    if (dog != nullptr) {
        dog->attach_ws_control(this);
    }
}

WebSocketHost::~WebSocketHost()
{
    stop();
}

void WebSocketHost::set_imu_callback(ImuCallback callback)
{
    std::lock_guard<std::mutex> lock(impl_->mutex);
    impl_->onImu = std::move(callback);
}

void WebSocketHost::set_tof_callback(TofCallback callback)
{
    std::lock_guard<std::mutex> lock(impl_->mutex);
    impl_->onTof = std::move(callback);
}

void WebSocketHost::set_pcm_callback(PcmCallback callback)
{
    std::lock_guard<std::mutex> lock(impl_->mutex);
    impl_->onPcm = std::move(callback);
}

void WebSocketHost::set_connection_callback(ConnectionCallback callback)
{
    std::lock_guard<std::mutex> lock(impl_->mutex);
    impl_->onConnection = std::move(callback);
}

void WebSocketHost::start(double waitReadyS)
{
    if (impl_->thread.joinable()) {
        return;
    }
    {
        std::lock_guard<std::mutex> lock(impl_->mutex);
        impl_->ready = false;
        impl_->stopping = false;
        impl_->startError = nullptr;
    }
    impl_->thread = std::thread([this]() {
        impl_->run();
    });
    std::unique_lock<std::mutex> lock(impl_->mutex);
    if (!impl_->cv.wait_for(lock, std::chrono::milliseconds(static_cast<int>(waitReadyS * 1000.0)), [this]() {
            return impl_->ready;
        })) {
        throw TimeoutError("WebSocket host did not become ready");
    }
    if (impl_->startError) {
        std::rethrow_exception(impl_->startError);
    }
}

void WebSocketHost::stop()
{
    try {
        {
            std::lock_guard<std::mutex> lock(impl_->mutex);
            impl_->stopping = true;
        }
        impl_->server.stop_listening();
        {
            std::lock_guard<std::mutex> lock(impl_->mutex);
            if (impl_->active.has_value()) {
                websocketpp::lib::error_code ec;
                impl_->server.close(*impl_->active, websocketpp::close::status::normal, "", ec);
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        impl_->server.stop();
    } catch (...) {
    }
    if (impl_->thread.joinable()) {
        impl_->thread.join();
    }
}

bool WebSocketHost::wait_robot_connected(double timeoutS, double pollS) const
{
    auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(static_cast<int>(timeoutS * 1000.0));
    auto delay = std::chrono::milliseconds(std::max(10, static_cast<int>(pollS * 1000.0)));
    while (std::chrono::steady_clock::now() < deadline) {
        if (is_robot_connected()) {
            return true;
        }
        std::this_thread::sleep_for(delay);
    }
    return is_robot_connected();
}

bool WebSocketHost::is_robot_connected() const
{
    std::lock_guard<std::mutex> lock(impl_->mutex);
    return impl_->connected && impl_->active.has_value();
}

void WebSocketHost::send_text(const std::string& text)
{
    std::lock_guard<std::mutex> lock(impl_->mutex);
    if (!impl_->active.has_value()) {
        throw ConnectionError("no active robot WebSocket connection");
    }
    websocketpp::lib::error_code ec;
    impl_->server.send(*impl_->active, text, websocketpp::frame::opcode::text, ec);
    if (ec) {
        throw ConnectionError("WebSocket send failed: " + ec.message());
    }
}

void WebSocketHost::send_control_raw(std::uint8_t mode, std::span<const std::uint8_t> data, const std::string& commandId)
{
    send_control_json(make_control_raw_json(mode, data, commandId));
}

void WebSocketHost::send_control(std::uint8_t mode, std::span<const std::uint8_t> data)
{
    send_control_raw(mode, data);
}

void WebSocketHost::send_control_json(const nlohmann::json& payload)
{
    send_text(payload.dump());
}

void WebSocketHost::send_config(const nlohmann::json& config)
{
    send_text(make_config_json(config).dump());
}

void WebSocketHost::send_edu_session(const std::string& action, int leaseMs, const std::string& commandId)
{
    send_text(make_edu_session_json(action, leaseMs, commandId).dump());
}

std::optional<nlohmann::json> WebSocketHost::wait_ack(const std::string& commandId, double timeoutS)
{
    auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(static_cast<int>(timeoutS * 1000.0));
    std::unique_lock<std::mutex> lock(impl_->mutex);
    while (std::chrono::steady_clock::now() < deadline) {
        auto it = impl_->acks.find(commandId);
        if (it != impl_->acks.end()) {
            auto ack = it->second;
            impl_->acks.erase(it);
            return ack;
        }
        impl_->ackCv.wait_until(lock, deadline);
    }
    return std::nullopt;
}

bool WebSocketHost::is_connected() const
{
    return is_robot_connected();
}

} // namespace aidog

#endif
