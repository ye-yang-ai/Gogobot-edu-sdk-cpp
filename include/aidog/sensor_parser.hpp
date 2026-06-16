#pragma once

#include <string_view>

#include "aidog/types.hpp"

namespace aidog {

ImuData normalize_imu_payload(const nlohmann::json& imu);
ParsedNotify parse_notify_json_text(std::string_view text);

} // namespace aidog
