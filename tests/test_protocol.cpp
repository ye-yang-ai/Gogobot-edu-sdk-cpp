#include <doctest/doctest.h>

#include "aidog/protocol.hpp"

using namespace aidog;

TEST_CASE("raw packet and websocket control json")
{
    std::vector<std::uint8_t> data{7, 0};
    auto packet = make_raw_packet(ModeInteraction, data);
    CHECK(packet == std::vector<std::uint8_t>{0xAA, 0x55, 0x00, 0x02, 7, 0});

    auto json = make_control_raw_json(ModeInteraction, data, "cmd-1");
    CHECK(json["cmd"] == "control_raw");
    CHECK(json["id"] == "cmd-1");
    CHECK(json["packet"] == "AA5500020700");
}

TEST_CASE("interaction data encodes angle little endian")
{
    auto data = make_interaction_data(Action::RightAngleInteraction, 360);
    CHECK(data == std::vector<std::uint8_t>{65, 0x68, 0x01});
}

TEST_CASE("interaction data omits optional parameter when absent")
{
    auto data = make_interaction_data(Action::ShakeHand, std::nullopt);
    CHECK(data == std::vector<std::uint8_t>{10, 0});
}

TEST_CASE("movement packet keeps Python-compatible fields")
{
    auto data = make_movement_data(Movement::Right);
    REQUIRE(data.size() == 54);
    CHECK(data[0] == 1);
    CHECK(data[1] == 3);
    CHECK(data[2] == 100);
    CHECK(data[9] == 500 >> 8);
    CHECK(data[10] == (500 & 0xFF));
}

TEST_CASE("pose adjust sorts by firmware parameter order")
{
    std::vector<PoseAdjustItem> poses{
        {"pitch", 1.0F, 2.0F},
        {"cog_x", 3.0F, 4.0F},
    };
    auto data = make_pose_adjust_data(poses, 300);
    CHECK(data[0] == 0x01);
    CHECK(data[1] == 0b1001);
    CHECK(data[2] == 0x2C);
    CHECK(data[3] == 0x01);
}
