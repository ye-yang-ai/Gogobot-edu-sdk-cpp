#include <doctest/doctest.h>

#include "aidog/sensor_parser.hpp"

using namespace aidog;

TEST_CASE("notify parser extracts status imu and tof")
{
    auto parsed = parse_notify_json_text(R"({"interaction_task_status":1,"imu":{"yaw":-400710,"pitch":630,"roll":-206},"tof":{"front_mm":300}})");
    REQUIRE(parsed.interactionTaskStatus.has_value());
    CHECK(*parsed.interactionTaskStatus == 1);
    REQUIRE(parsed.imu.has_value());
    CHECK(parsed.imu->yawDeg == doctest::Approx(-400.710));
    CHECK(parsed.imu->pitchDeg == doctest::Approx(0.630));
    CHECK(parsed.imu->rollDeg == doctest::Approx(-0.206));
    REQUIRE(parsed.tof.has_value());
    CHECK(parsed.tof->raw["front_mm"] == 300);
}

TEST_CASE("notify parser ignores non-json text")
{
    auto parsed = parse_notify_json_text("hello");
    CHECK_FALSE(parsed.interactionTaskStatus.has_value());
    CHECK_FALSE(parsed.imu.has_value());
    CHECK_FALSE(parsed.tof.has_value());
}

TEST_CASE("notify parser accepts integer-like status values")
{
    auto fromFloat = parse_notify_json_text(R"({"interaction_task_status":0.0})");
    REQUIRE(fromFloat.interactionTaskStatus.has_value());
    CHECK(*fromFloat.interactionTaskStatus == 0);

    auto fromString = parse_notify_json_text(R"({"interaction_task_status":"1"})");
    REQUIRE(fromString.interactionTaskStatus.has_value());
    CHECK(*fromString.interactionTaskStatus == 1);
}
