#include <doctest/doctest.h>

#include "aidog/actions.hpp"

using namespace aidog;

TEST_CASE("resolve action aliases and hidden ids")
{
    CHECK(resolve_action("sit_down") == Action::SitDown);
    CHECK(resolve_action("turn_right_angle") == Action::RightAngleInteraction);
    CHECK(resolve_action(7) == Action::SitDown);
    CHECK(resolve_action(47) == Action::Idle);
}

TEST_CASE("action parameter metadata")
{
    CHECK(is_count_based(Action::ShakeHand));
    CHECK(is_timer_based(Action::ForwardInteraction));
    CHECK(is_angle_based(Action::LeftAngleInteraction));
    CHECK(action_default(Action::ShakeHand) == 5);
    CHECK(action_default(Action::LeftAngleInteraction) == 90);
    CHECK_FALSE(action_default(Action::SitDown).has_value());
}
