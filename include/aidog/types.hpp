#pragma once

#include <cstdint>
#include <functional>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

namespace aidog {

struct DeviceInfo {
    std::string name;
    std::string address;
};

struct ConnectOptions {
    std::string namePrefix = "Gogobot";
    std::optional<std::string> address;
    int retries = 3;
    double retryDelayS = 1.0;
};

struct ActionOptions {
    std::optional<int> duration;
    std::optional<int> count;
    std::optional<int> angle;
    double timeoutS = 20.0;
    bool requireRunningState = true;
    std::string transport = "ble";
};

struct ImuData {
    nlohmann::json raw;
    std::optional<double> yawDeg;
    std::optional<double> pitchDeg;
    std::optional<double> rollDeg;
};

struct TofData {
    nlohmann::json raw;
};

using ImuCallback = std::function<void(const ImuData&)>;
using TofCallback = std::function<void(const TofData&)>;
using PcmCallback = std::function<void(const std::vector<std::uint8_t>&)>;

struct ParsedNotify {
    std::optional<int> interactionTaskStatus;
    std::optional<ImuData> imu;
    std::optional<TofData> tof;
};

struct PoseAdjustItem {
    std::string name;
    float now = 0.0F;
    float end = 0.0F;
};

struct DeltaAdjustItem {
    std::string name;
    float delta = 0.0F;
};

} // namespace aidog
