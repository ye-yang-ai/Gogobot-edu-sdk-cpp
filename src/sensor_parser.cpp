#include "aidog/sensor_parser.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <string>

namespace aidog {
namespace {

std::string _trim(std::string_view text)
{
    auto begin = text.begin();
    auto end = text.end();
    while (begin != end && std::isspace(static_cast<unsigned char>(*begin)) != 0) {
        ++begin;
    }
    while (begin != end && std::isspace(static_cast<unsigned char>(*(end - 1))) != 0) {
        --end;
    }
    return std::string(begin, end);
}

std::optional<double> _number_value(const nlohmann::json& obj, const char* key)
{
    auto it = obj.find(key);
    if (it == obj.end() || !it->is_number()) {
        return std::nullopt;
    }
    return it->get<double>();
}

} // namespace

ImuData normalize_imu_payload(const nlohmann::json& imu)
{
    ImuData out;
    out.raw = imu;

    std::vector<double> rawAngles;
    for (const auto* axis : {"yaw", "pitch", "roll"}) {
        auto value = _number_value(imu, axis);
        if (value.has_value()) {
            rawAngles.push_back(*value);
        }
    }

    bool useMilliDeg = false;
    if (!rawAngles.empty()) {
        double maxAbs = 0.0;
        bool allIntLike = true;
        for (auto value : rawAngles) {
            maxAbs = std::max(maxAbs, std::abs(value));
            allIntLike = allIntLike && std::abs(value - std::round(value)) < 1e-9;
        }
        if (maxAbs > 1000.0 || (allIntLike && maxAbs > 180.0)) {
            useMilliDeg = true;
        }
    }

    nlohmann::json angleDeg = nlohmann::json::object();
    auto normalizeAxis = [&](const char* axis) -> std::optional<double> {
        auto value = _number_value(imu, axis);
        if (!value.has_value()) {
            return std::nullopt;
        }
        auto deg = useMilliDeg ? *value / 1000.0 : *value;
        std::string key = std::string(axis) + "_deg";
        out.raw[key] = deg;
        angleDeg[axis] = deg;
        return deg;
    };

    out.yawDeg = normalizeAxis("yaw");
    out.pitchDeg = normalizeAxis("pitch");
    out.rollDeg = normalizeAxis("roll");
    if (!angleDeg.empty()) {
        out.raw["angle_deg"] = angleDeg;
    }
    return out;
}

ParsedNotify parse_notify_json_text(std::string_view text)
{
    ParsedNotify out;
    try {
        auto trimmed = _trim(text);
        if (trimmed.size() < 2 || trimmed.front() != '{' || trimmed.back() != '}') {
            return out;
        }

        auto obj = nlohmann::json::parse(trimmed);
        if (!obj.is_object()) {
            return out;
        }

        auto status = obj.find("interaction_task_status");
        if (status != obj.end() && status->is_number_integer()) {
            out.interactionTaskStatus = status->get<int>();
        }

        auto imu = obj.find("imu");
        if (imu != obj.end() && imu->is_object()) {
            out.imu = normalize_imu_payload(*imu);
        }

        auto tof = obj.find("tof");
        if (tof != obj.end() && tof->is_object()) {
            out.tof = TofData{*tof};
        }
    } catch (...) {
        return ParsedNotify{};
    }
    return out;
}

} // namespace aidog
