#pragma once

#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "aidog/actions.hpp"
#include "aidog/types.hpp"

namespace aidog {

inline constexpr std::uint8_t ModeEar = 0x00;
inline constexpr std::uint8_t ModeSport = 0x01;
inline constexpr std::uint8_t ModeInteraction = 0x02;
inline constexpr std::uint8_t ModeExpression = 0x05;
inline constexpr std::uint8_t ModeAudio = 0x06;
inline constexpr std::uint8_t ModeSensor = 0x08;
inline constexpr std::uint8_t ModeStream = 0x09;
inline constexpr std::uint8_t ModeRobotAdjust = 0x0A;
inline constexpr int ConfigSetVolume = 1;

std::vector<std::uint8_t> make_raw_packet(std::uint8_t mode, std::span<const std::uint8_t> data);
std::string bytes_to_hex(std::span<const std::uint8_t> data);
std::vector<std::uint8_t> make_interaction_data(Action action, std::optional<int> param);
std::vector<std::uint8_t> make_movement_data(Movement direction);
std::vector<std::uint8_t> make_pose_adjust_data(std::span<const PoseAdjustItem> poses, int durationMs);
std::vector<std::uint8_t> make_foot_adjust_data(std::span<const DeltaAdjustItem> feet, int durationMs);
std::vector<std::uint8_t> make_joint_adjust_data(std::span<const DeltaAdjustItem> joints, int durationMs);
std::vector<std::uint8_t> make_default_pose_data(float roll, float pitch, float x, float z);
nlohmann::json make_control_raw_json(std::uint8_t mode, std::span<const std::uint8_t> data, const std::string& commandId = {});
nlohmann::json make_config_json(const nlohmann::json& config, const std::string& commandId = {});
nlohmann::json make_edu_session_json(const std::string& action, int leaseMs, const std::string& commandId = {});

} // namespace aidog
