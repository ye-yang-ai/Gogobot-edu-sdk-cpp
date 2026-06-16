#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace aidog {

enum class Action : std::uint8_t {
    Idle = 0,
    SlowUp = 1,
    SlowDown = 2,
    SlowDownForCharge = 3,
    SlowDownForProgram = 4,
    UpAndDown = 5,
    SitDown = 7,
    SitDownForProgram = 8,
    StandUp = 9,
    ShakeHand = 10,
    ShakeHandWithSitDown = 11,
    Nod = 12,
    ShakeHead = 13,
    Stretch = 14,
    Pee = 15,
    Twist = 16,
    PushUp = 17,
    NewYear = 18,
    WagTail = 19,
    Stomp = 20,
    Sniff = 21,
    Celebrate = 22,
    Jump = 23,
    Dance = 24,
    KickBall = 25,
    TouchGroundRight = 26,
    TouchGroundLeft = 27,
    PlayDead = 28,
    StepInteraction = 29,
    ForwardInteraction = 30,
    BackInteraction = 31,
    LeftInteraction = 32,
    RightInteraction = 33,
    LowForwardAndBackwardInteraction = 34,
    LowForwardInteraction = 35,
    LowBackwardInteraction = 36,
    LowLeftInteraction = 37,
    LowRightInteraction = 38,
    StopInteraction = 39,
    UpAndDownForTest = 40,
    RolloverRecoveryRight = 41,
    RolloverRecoveryLeft = 42,
    Flailing = 43,
    StopFlailing = 44,
    LightOnInteraction = 45,
    LightOffInteraction = 46,
    SwingLeftAndRight = 52,
    SwingLeft = 53,
    SwingRight = 54,
    ExcitedInspace = 55,
    LazyPatPat = 56,
    CheekyPaw = 57,
    Whining = 58,
    SniffForwardInteraction = 59,
    SpaceBackwardInteraction = 60,
    SniffLeftInteraction = 61,
    SniffRightInteraction = 62,
    SniffStepInteraction = 63,
    LeftAngleInteraction = 64,
    RightAngleInteraction = 65,
};

enum class Movement : std::uint8_t {
    Forward = 0x01,
    Back = 0x02,
    Step = 0x10,
    Right = 0x08,
    Left = 0x04,
};

enum class EarAction : std::uint8_t {
    Idle = 0,
    EarShakeAsyn13 = 1,
    EarShakeAsyn12 = 2,
    EarShakeSyn = 3,
    EarShakeSynForBle = 4,
    EarPear1 = 5,
    EarPear2 = 6,
    EarPear3 = 7,
    EarStand = 8,
    EarStandLeft = 9,
    EarStandRight = 10,
    EarStandLeftAndRight = 11,
    EarForWink = 12,
    EarForVideo = 13,
    EarPercentageBasic = 14,
    SpecialDetectionToggleBasic = 15,
    EarFlickExcited = 16,
    EarFlickLeftQuick = 17,
    EarFlickRightQuick = 18,
    EarFlickAlternate = 19,
    EarFlickLeftAndRightUp = 20,
    EarFlickRandom = 21,
    EarWiggleSubtleSelfStable = 22,
    EarFlickRandomNegative = 23,
    EarFlickRandomPositive = 24,
    EarBreathe = 25,
    EarDown = 26,
};

enum class ExpressionAction : std::uint8_t {
    Idle = 0,
    Happy01 = 1,
    Happy02 = 2,
    Happy03 = 3,
    Happy04 = 4,
    Smile01 = 5,
    Smile02 = 6,
    Smile03 = 7,
    Love01 = 8,
    Love02 = 9,
    Anger01 = 10,
    Anger02 = 11,
    Anger03 = 12,
    Anger04 = 13,
    Sad01 = 14,
    Sad02 = 15,
    Sad03 = 16,
    DoubleSad03 = 17,
    Sad04 = 18,
    Scared01 = 19,
    Scared02 = 20,
    Comfortable01 = 21,
    Comfortable02 = 22,
    Doubt01 = 23,
    Doubt02 = 24,
    Doubt03 = 25,
    Nervous = 26,
    NervousComplete = 27,
    Tired = 28,
    TiredComplete = 29,
    Sleepy = 30,
    SleepyComplete = 31,
    WinkFast = 32,
    WinkNormal = 33,
    LookRight = 34,
    LookLeft = 35,
    LookLeftAndRight = 36,
    NoteParticleCircle = 37,
    Music = 38,
    Basketball = 39,
    Pingpong = 40,
    Football = 41,
    Sound0 = 42,
    Sound25 = 43,
    Sound50 = 44,
    Sound75 = 45,
    Sound100 = 46,
    SoundCircle = 47,
    GetUp = 48,
    EatSnack = 49,
    Charging = 50,
    Sunglasses = 51,
    EyesFighting = 52,
    TurnOff = 53,
    Caring = 54,
    Shy = 55,
    Drink = 56,
    Alert = 57,
    Boring = 58,
    NoWifi = 59,
    Shame = 60,
    Shame02 = 61,
    SniffExpression = 62,
    Dead = 63,
    Pride = 64,
    Yawn = 65,
    LightOn = 66,
    LightOff = 67,
};

enum class Tone : std::uint8_t {
    Stop = 0,
    Jeez = 1,
    Uh = 2,
    Eating = 3,
    Charging = 4,
    Curious = 5,
    Sleepy = 6,
    Heng = 7,
    Sad = 8,
    Angry = 9,
    Doubt = 10,
    Agree = 11,
    Enheng = 12,
    Alert = 13,
    WakeUp = 14,
    Comfort = 15,
    Sigh = 16,
    Snore = 17,
    Sniff = 18,
    Beat1 = 19,
    Beat2 = 20,
    Beat3 = 21,
    Beat4 = 22,
    Beat5 = 23,
    Beat6 = 24,
    Beat7 = 25,
};

enum class ParameterType {
    Normal,
    Time,
    Count,
    Angle,
};

struct ActionSpec {
    Action action;
    ParameterType parameterType;
    std::optional<int> defaultValue;
    const char* description;
};

Action resolve_action(int id);
Action resolve_action(std::string_view name);
const ActionSpec& action_spec(Action action);
bool is_timer_based(Action action);
bool is_count_based(Action action);
bool is_angle_based(Action action);
std::optional<int> action_default(Action action);

const std::unordered_map<std::string, Action>& action_aliases();
const std::vector<ActionSpec>& action_specs();

} // namespace aidog
