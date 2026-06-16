#include "aidog/actions.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <stdexcept>
#include <unordered_map>

namespace aidog {
namespace {

std::string _lower_ascii(std::string_view text)
{
    std::string out;
    out.reserve(text.size());
    for (unsigned char ch : text) {
        out.push_back(static_cast<char>(std::tolower(ch)));
    }
    return out;
}

const std::vector<ActionSpec> kActionSpecs = {
    {Action::Idle, ParameterType::Normal, std::nullopt, "Idle"},
    {Action::SlowUp, ParameterType::Normal, std::nullopt, "Slowly stand up"},
    {Action::SlowDown, ParameterType::Normal, std::nullopt, "Slowly crouch down"},
    {Action::SlowDownForCharge, ParameterType::Normal, std::nullopt, "Crouch down for charging posture"},
    {Action::SlowDownForProgram, ParameterType::Time, 5, "Slow crouch for programming flows"},
    {Action::UpAndDown, ParameterType::Count, 3, "Up-and-down motion"},
    {Action::SitDown, ParameterType::Normal, std::nullopt, "Sit down"},
    {Action::SitDownForProgram, ParameterType::Time, 5, "Sit down for programming flows"},
    {Action::StandUp, ParameterType::Normal, std::nullopt, "Stand up from sitting posture"},
    {Action::ShakeHand, ParameterType::Count, 5, "Shake hand"},
    {Action::ShakeHandWithSitDown, ParameterType::Count, 3, "Sit down and shake hand"},
    {Action::Nod, ParameterType::Count, 2, "Nod"},
    {Action::ShakeHead, ParameterType::Count, 4, "Shake head"},
    {Action::Stretch, ParameterType::Normal, std::nullopt, "Stretch"},
    {Action::Pee, ParameterType::Count, 2, "Simulated urination motion"},
    {Action::Twist, ParameterType::Count, 3, "Twist"},
    {Action::PushUp, ParameterType::Count, 3, "Push-up"},
    {Action::NewYear, ParameterType::Count, 3, "New Year greeting"},
    {Action::WagTail, ParameterType::Count, 5, "Wag tail"},
    {Action::Stomp, ParameterType::Count, 6, "Stomp"},
    {Action::Sniff, ParameterType::Normal, std::nullopt, "Sniff"},
    {Action::Celebrate, ParameterType::Count, 3, "Celebrate"},
    {Action::Jump, ParameterType::Normal, std::nullopt, "Jump"},
    {Action::Dance, ParameterType::Time, 3, "Dance"},
    {Action::KickBall, ParameterType::Normal, std::nullopt, "Kick ball"},
    {Action::TouchGroundRight, ParameterType::Normal, std::nullopt, "Touch ground with right paw"},
    {Action::TouchGroundLeft, ParameterType::Normal, std::nullopt, "Touch ground with left paw"},
    {Action::PlayDead, ParameterType::Normal, std::nullopt, "Play dead"},
    {Action::StepInteraction, ParameterType::Time, 3, "Step in place"},
    {Action::ForwardInteraction, ParameterType::Time, 3, "Move forward"},
    {Action::BackInteraction, ParameterType::Time, 3, "Move backward"},
    {Action::LeftInteraction, ParameterType::Time, 3, "Turn left"},
    {Action::RightInteraction, ParameterType::Time, 3, "Turn right"},
    {Action::LowForwardAndBackwardInteraction, ParameterType::Time, 3, "Low posture forward/backward movement"},
    {Action::LowForwardInteraction, ParameterType::Time, 3, "Low posture forward movement"},
    {Action::LowBackwardInteraction, ParameterType::Time, 3, "Low posture backward movement"},
    {Action::LowLeftInteraction, ParameterType::Time, 3, "Low posture left turn"},
    {Action::LowRightInteraction, ParameterType::Time, 3, "Low posture right turn"},
    {Action::StopInteraction, ParameterType::Normal, std::nullopt, "Stop interaction motion"},
    {Action::UpAndDownForTest, ParameterType::Normal, std::nullopt, "Up-and-down test motion"},
    {Action::RolloverRecoveryRight, ParameterType::Normal, std::nullopt, "Recover from right-side rollover"},
    {Action::RolloverRecoveryLeft, ParameterType::Normal, std::nullopt, "Recover from left-side rollover"},
    {Action::Flailing, ParameterType::Count, 4, "Flailing motion"},
    {Action::StopFlailing, ParameterType::Normal, std::nullopt, "Stop flailing"},
    {Action::LightOnInteraction, ParameterType::Normal, std::nullopt, "Turn flashlight on"},
    {Action::LightOffInteraction, ParameterType::Normal, std::nullopt, "Turn flashlight off"},
    {Action::SwingLeftAndRight, ParameterType::Normal, std::nullopt, "Swing left and right"},
    {Action::SwingLeft, ParameterType::Normal, std::nullopt, "Swing left"},
    {Action::SwingRight, ParameterType::Normal, std::nullopt, "Swing right"},
    {Action::ExcitedInspace, ParameterType::Normal, std::nullopt, "Excited in-place motion"},
    {Action::LazyPatPat, ParameterType::Normal, std::nullopt, "Lazy pat-pat motion"},
    {Action::CheekyPaw, ParameterType::Normal, std::nullopt, "Cheeky paw motion"},
    {Action::Whining, ParameterType::Normal, std::nullopt, "Whining motion"},
    {Action::SniffForwardInteraction, ParameterType::Time, 3, "Sniff forward"},
    {Action::SpaceBackwardInteraction, ParameterType::Time, 3, "Space-walk backward"},
    {Action::SniffLeftInteraction, ParameterType::Time, 3, "Sniff left turn"},
    {Action::SniffRightInteraction, ParameterType::Time, 3, "Sniff right turn"},
    {Action::SniffStepInteraction, ParameterType::Time, 3, "Sniff step in place"},
    {Action::LeftAngleInteraction, ParameterType::Angle, 90, "Turn left by a specified angle"},
    {Action::RightAngleInteraction, ParameterType::Angle, 90, "Turn right by a specified angle"},
};

const std::unordered_map<std::string, Action> kActionAliases = {
    {"idle", Action::Idle},
    {"slow_up", Action::SlowUp},
    {"slow_down", Action::SlowDown},
    {"sit_down", Action::SitDown},
    {"stand_up", Action::StandUp},
    {"shake_hand", Action::ShakeHand},
    {"nod", Action::Nod},
    {"shake_head", Action::ShakeHead},
    {"stretch", Action::Stretch},
    {"pee", Action::Pee},
    {"twist", Action::Twist},
    {"push_up", Action::PushUp},
    {"new_year", Action::NewYear},
    {"wag_tail", Action::WagTail},
    {"stomp", Action::Stomp},
    {"sniff", Action::Sniff},
    {"celebrate", Action::Celebrate},
    {"jump", Action::Jump},
    {"dance", Action::Dance},
    {"kick_ball", Action::KickBall},
    {"touch_ground_right", Action::TouchGroundRight},
    {"touch_ground_left", Action::TouchGroundLeft},
    {"play_dead", Action::PlayDead},
    {"step", Action::StepInteraction},
    {"walk_forward", Action::ForwardInteraction},
    {"walk_back", Action::BackInteraction},
    {"turn_left", Action::LeftInteraction},
    {"turn_right", Action::RightInteraction},
    {"turn_left_angle", Action::LeftAngleInteraction},
    {"turn_right_angle", Action::RightAngleInteraction},
    {"left_angle", Action::LeftAngleInteraction},
    {"right_angle", Action::RightAngleInteraction},
    {"low_forward_and_backward", Action::LowForwardAndBackwardInteraction},
    {"low_forward", Action::LowForwardInteraction},
    {"low_backward", Action::LowBackwardInteraction},
    {"low_left", Action::LowLeftInteraction},
    {"low_right", Action::LowRightInteraction},
    {"stop", Action::StopInteraction},
    {"light_on", Action::LightOnInteraction},
    {"light_off", Action::LightOffInteraction},
    {"rollover_recovery_right", Action::RolloverRecoveryRight},
    {"rollover_recovery_left", Action::RolloverRecoveryLeft},
    {"light_on_interaction", Action::LightOnInteraction},
    {"light_off_interaction", Action::LightOffInteraction},
    {"swing_left_and_right", Action::SwingLeftAndRight},
    {"swing_left", Action::SwingLeft},
    {"swing_right", Action::SwingRight},
    {"excited_inspace", Action::ExcitedInspace},
    {"lazy_pat_pat", Action::LazyPatPat},
    {"cheeky_paw", Action::CheekyPaw},
    {"whining", Action::Whining},
    {"sniff_forward", Action::SniffForwardInteraction},
    {"space_backward", Action::SpaceBackwardInteraction},
    {"sniff_left", Action::SniffLeftInteraction},
    {"sniff_right", Action::SniffRightInteraction},
    {"sniff_step", Action::SniffStepInteraction},
    {"sniff_forward_interaction", Action::SniffForwardInteraction},
    {"space_backward_interaction", Action::SpaceBackwardInteraction},
    {"sniff_left_interaction", Action::SniffLeftInteraction},
    {"sniff_right_interaction", Action::SniffRightInteraction},
    {"sniff_step_interaction", Action::SniffStepInteraction},
};

} // namespace

Action resolve_action(int id)
{
    switch (id) {
    case 6:
    case 47:
    case 48:
    case 49:
    case 50:
    case 51:
        return Action::Idle;
    default:
        break;
    }

    auto value = static_cast<Action>(id);
    auto it = std::find_if(kActionSpecs.begin(), kActionSpecs.end(), [value](const ActionSpec& spec) {
        return spec.action == value;
    });
    if (it == kActionSpecs.end()) {
        throw std::invalid_argument("unknown action id");
    }
    return value;
}

Action resolve_action(std::string_view name)
{
    auto key = _lower_ascii(name);
    auto it = kActionAliases.find(key);
    if (it == kActionAliases.end()) {
        throw std::invalid_argument("unknown action name");
    }
    return it->second;
}

const ActionSpec& action_spec(Action action)
{
    auto it = std::find_if(kActionSpecs.begin(), kActionSpecs.end(), [action](const ActionSpec& spec) {
        return spec.action == action;
    });
    if (it == kActionSpecs.end()) {
        throw std::invalid_argument("unknown action");
    }
    return *it;
}

bool is_timer_based(Action action)
{
    return action_spec(action).parameterType == ParameterType::Time;
}

bool is_count_based(Action action)
{
    return action_spec(action).parameterType == ParameterType::Count;
}

bool is_angle_based(Action action)
{
    return action_spec(action).parameterType == ParameterType::Angle;
}

std::optional<int> action_default(Action action)
{
    return action_spec(action).defaultValue;
}

const std::unordered_map<std::string, Action>& action_aliases()
{
    return kActionAliases;
}

const std::vector<ActionSpec>& action_specs()
{
    return kActionSpecs;
}

} // namespace aidog
