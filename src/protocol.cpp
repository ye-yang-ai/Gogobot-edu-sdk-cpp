#include "aidog/protocol.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <iomanip>
#include <numbers>
#include <sstream>
#include <stdexcept>
#include <unordered_map>

namespace aidog {
namespace {

constexpr std::uint8_t kHeaderHi = 0xAA;
constexpr std::uint8_t kHeaderLo = 0x55;
constexpr std::uint8_t kRobotAdjustPose = 0x01;
constexpr std::uint8_t kRobotAdjustFoot = 0x02;
constexpr std::uint8_t kRobotAdjustJoint = 0x03;
constexpr std::uint8_t kRobotAdjustDefaultPose = 0x05;

template <typename T>
T _clamp(T value, T minValue, T maxValue)
{
    return std::max(minValue, std::min(maxValue, value));
}

void _append_u16_le(std::vector<std::uint8_t>& out, int value)
{
    auto v = static_cast<std::uint16_t>(_clamp(value, 0, 65535));
    out.push_back(static_cast<std::uint8_t>(v & 0xFF));
    out.push_back(static_cast<std::uint8_t>((v >> 8) & 0xFF));
}

void _append_float_le(std::vector<std::uint8_t>& out, float value)
{
    static_assert(sizeof(float) == 4);
    std::array<std::uint8_t, 4> bytes{};
    std::memcpy(bytes.data(), &value, sizeof(float));
#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    std::reverse(bytes.begin(), bytes.end());
#endif
    out.insert(out.end(), bytes.begin(), bytes.end());
}

std::uint8_t _movement_state(Movement direction)
{
    switch (direction) {
    case Movement::Forward:
        return 0;
    case Movement::Back:
        return 1;
    case Movement::Step:
        return 2;
    case Movement::Right:
        return 3;
    case Movement::Left:
        return 4;
    }
    throw std::invalid_argument("unsupported movement direction");
}

std::vector<std::uint8_t> _make_delta_adjust_data(
    std::span<const DeltaAdjustItem> items,
    int durationMs,
    std::uint8_t subMode,
    const std::vector<std::string>& order)
{
    if (items.empty()) {
        throw std::invalid_argument("adjust items must not be empty");
    }

    int mask = 0;
    int used = 0;
    std::vector<std::pair<int, float>> indexed;
    for (const auto& item : items) {
        auto it = std::find(order.begin(), order.end(), item.name);
        if (it == order.end()) {
            throw std::invalid_argument("unknown adjust parameter");
        }
        auto idx = static_cast<int>(std::distance(order.begin(), it));
        auto bit = 1 << idx;
        if ((used & bit) != 0) {
            throw std::invalid_argument("duplicate adjust parameter");
        }
        used |= bit;
        mask |= bit;
        indexed.emplace_back(idx, item.delta);
    }
    std::sort(indexed.begin(), indexed.end(), [](const auto& lhs, const auto& rhs) {
        return lhs.first < rhs.first;
    });

    std::vector<std::uint8_t> out{subMode, static_cast<std::uint8_t>(mask & 0xFF)};
    _append_u16_le(out, durationMs);
    for (const auto& [_, delta] : indexed) {
        _append_float_le(out, delta);
    }
    return out;
}

const std::vector<std::string> kPoseOrder = {"cog_x", "cog_z", "roll", "pitch"};
const std::vector<std::string> kFootOrder = {
    "foot_0_x", "foot_0_z", "foot_1_x", "foot_1_z",
    "foot_2_x", "foot_2_z", "foot_3_x", "foot_3_z",
};
const std::vector<std::string> kJointOrder = {
    "joint_0_1", "joint_0_2", "joint_1_1", "joint_1_2",
    "joint_2_1", "joint_2_2", "joint_3_1", "joint_3_2",
};

} // namespace

std::vector<std::uint8_t> make_raw_packet(std::uint8_t mode, std::span<const std::uint8_t> data)
{
    std::vector<std::uint8_t> packet{kHeaderHi, kHeaderLo, 0x00, mode};
    packet.insert(packet.end(), data.begin(), data.end());
    return packet;
}

std::string bytes_to_hex(std::span<const std::uint8_t> data)
{
    std::ostringstream oss;
    oss << std::uppercase << std::hex << std::setfill('0');
    for (auto byte : data) {
        oss << std::setw(2) << static_cast<int>(byte);
    }
    return oss.str();
}

std::vector<std::uint8_t> make_interaction_data(Action action, std::optional<int> param)
{
    std::vector<std::uint8_t> data{static_cast<std::uint8_t>(action), 0};
    if (!param.has_value()) {
        return data;
    }
    if (is_angle_based(action)) {
        auto value = _clamp(*param, 0, 65535);
        data[1] = static_cast<std::uint8_t>(value & 0xFF);
        data.push_back(static_cast<std::uint8_t>((value >> 8) & 0xFF));
        return data;
    }
    data[1] = static_cast<std::uint8_t>(_clamp(*param, 0, 255));
    return data;
}

std::vector<std::uint8_t> make_movement_data(Movement direction)
{
    std::vector<std::uint8_t> data;
    data.reserve(50);
    data.push_back(1);
    data.push_back(_movement_state(direction));
    data.push_back(100);
    data.insert(data.end(), {0, 0});
    data.insert(data.end(), {0, 0});
    data.insert(data.end(), {0, 0});
    data.insert(data.end(), {500 >> 8, 500 & 0xFF});
    data.push_back(75);
    data.push_back(35);
    _append_float_le(data, 0.5F);
    _append_float_le(data, 0.5F);
    _append_float_le(data, 0.01F);
    _append_float_le(data, 8.0F);
    _append_float_le(data, 5.0F);
    data.push_back(0);
    _append_float_le(data, 0.5F);
    _append_float_le(data, 0.0F);
    _append_float_le(data, static_cast<float>(std::numbers::pi));
    _append_float_le(data, static_cast<float>(std::numbers::pi));
    _append_float_le(data, 0.0F);
    return data;
}

std::vector<std::uint8_t> make_pose_adjust_data(std::span<const PoseAdjustItem> poses, int durationMs)
{
    if (poses.empty()) {
        throw std::invalid_argument("poses must not be empty");
    }

    int mask = 0;
    int used = 0;
    std::vector<std::tuple<int, float, float>> indexed;
    for (const auto& pose : poses) {
        auto it = std::find(kPoseOrder.begin(), kPoseOrder.end(), pose.name);
        if (it == kPoseOrder.end()) {
            throw std::invalid_argument("unknown pose parameter");
        }
        auto idx = static_cast<int>(std::distance(kPoseOrder.begin(), it));
        auto bit = 1 << idx;
        if ((used & bit) != 0) {
            throw std::invalid_argument("duplicate pose parameter");
        }
        used |= bit;
        mask |= bit;
        indexed.emplace_back(idx, pose.now, pose.end);
    }
    std::sort(indexed.begin(), indexed.end(), [](const auto& lhs, const auto& rhs) {
        return std::get<0>(lhs) < std::get<0>(rhs);
    });

    std::vector<std::uint8_t> out{kRobotAdjustPose, static_cast<std::uint8_t>(mask & 0xFF)};
    _append_u16_le(out, durationMs);
    for (const auto& [_, now, end] : indexed) {
        _append_float_le(out, now);
        _append_float_le(out, end);
    }
    return out;
}

std::vector<std::uint8_t> make_foot_adjust_data(std::span<const DeltaAdjustItem> feet, int durationMs)
{
    return _make_delta_adjust_data(feet, durationMs, kRobotAdjustFoot, kFootOrder);
}

std::vector<std::uint8_t> make_joint_adjust_data(std::span<const DeltaAdjustItem> joints, int durationMs)
{
    return _make_delta_adjust_data(joints, durationMs, kRobotAdjustJoint, kJointOrder);
}

std::vector<std::uint8_t> make_default_pose_data(float roll, float pitch, float x, float z)
{
    std::vector<std::uint8_t> out{kRobotAdjustDefaultPose};
    _append_float_le(out, roll);
    _append_float_le(out, pitch);
    _append_float_le(out, x);
    _append_float_le(out, z);
    return out;
}

nlohmann::json make_control_raw_json(std::uint8_t mode, std::span<const std::uint8_t> data, const std::string& commandId)
{
    auto packet = make_raw_packet(mode, data);
    nlohmann::json payload{{"cmd", "control_raw"}, {"packet", bytes_to_hex(packet)}};
    if (!commandId.empty()) {
        payload["id"] = commandId;
    }
    return payload;
}

nlohmann::json make_config_json(const nlohmann::json& config, const std::string& commandId)
{
    nlohmann::json payload{{"cmd", "config_json"}, {"config", config}};
    if (!commandId.empty()) {
        payload["id"] = commandId;
    }
    return payload;
}

nlohmann::json make_edu_session_json(const std::string& action, int leaseMs, const std::string& commandId)
{
    nlohmann::json payload{
        {"cmd", "edu_session"},
        {"action", action},
        {"lease_ms", std::max(1000, leaseMs)},
    };
    if (!commandId.empty()) {
        payload["id"] = commandId;
    }
    return payload;
}

} // namespace aidog
