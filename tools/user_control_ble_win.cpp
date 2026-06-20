/*
Purpose:
    AI-Dog Windows BLE user control GUI.
Build:
    cd C:\C_project_3.1\arbitrarion10\aidog_sdk_cpp
    "C:\Program Files\CMake\bin\cmake.exe" --build build --config Release --target aidog_user_control_ble
Run:
    cd C:\C_project_3.1\arbitrarion10\aidog_sdk_cpp
    .\build\Release\aidog_user_control_ble.exe
Device:
    Current BLE address: 12:0A:AB:16:3A:04
*/

#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <deque>
#include <functional>
#include <cmath>
#include <iomanip>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

#include <aidog.hpp>

#pragma comment(lib, "comctl32.lib")

namespace {

constexpr wchar_t kWindowClass[] = L"AiDogBleControlWindow";
#ifdef AIDOG_USER_CONTROL_WS
constexpr wchar_t kWindowTitle[] = L"AI-Dog WS User Control";
constexpr const char* kControlTransport = "ws";
#else
constexpr wchar_t kWindowTitle[] = L"AI-Dog BLE User Control";
constexpr const char* kControlTransport = "ble";
#endif
constexpr UINT WM_APP_TASK_DONE = WM_APP + 1;
constexpr UINT WM_APP_SENSOR = WM_APP + 2;
#ifdef AIDOG_USER_CONTROL_WS
constexpr UINT WM_APP_WS_STATE = WM_APP + 3;
#endif
constexpr UINT_PTR kRefreshTimer = 1;
constexpr UINT_PTR kPlotTimer = 2;
constexpr int kMaxLogLines = 300;
constexpr int kJoystickSize = 260;
constexpr int kJoystickDeadzone = 18;
constexpr int kPlotHistoryMs = 12000;
constexpr DWORD kEarSendIntervalMs = 60;

enum ControlId {
    IdPrefix = 100,
    IdScan,
    IdDeviceCombo,
    IdConnect,
    IdDisconnect,
    IdTabs,
    IdLog,
    IdImuCheck,
    IdTofCheck,
    IdHzEdit,
    IdSensorText,
    IdJoystick,
    IdApplyHz,
    IdClearPlots,
    IdPlotBase = 9000,
};

template <typename Enum>
struct EnumButtonDef {
    const wchar_t* text;
    Enum value;
};

std::wstring to_wide(const std::string& text)
{
    if (text.empty()) {
        return {};
    }
    int size = MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), nullptr, 0);
    std::wstring out(static_cast<std::size_t>(size), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), out.data(), size);
    return out;
}

std::string to_utf8(const std::wstring& text)
{
    if (text.empty()) {
        return {};
    }
    int size = WideCharToMultiByte(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), nullptr, 0, nullptr, nullptr);
    std::string out(static_cast<std::size_t>(size), '\0');
    WideCharToMultiByte(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), out.data(), size, nullptr, nullptr);
    return out;
}

std::wstring now_text()
{
    SYSTEMTIME time{};
    GetLocalTime(&time);
    wchar_t buffer[16]{};
    swprintf_s(buffer, L"%02u:%02u:%02u", time.wHour, time.wMinute, time.wSecond);
    return buffer;
}

std::wstring number_text(double value)
{
    std::wostringstream oss;
    oss << std::fixed << std::setprecision(2) << value;
    return oss.str();
}

const wchar_t* action_text(aidog::Action action)
{
    switch (action) {
    case aidog::Action::Idle: return L"IDLE";
    case aidog::Action::SlowUp: return L"SLOW_UP";
    case aidog::Action::SlowDown: return L"SLOW_DOWN";
    case aidog::Action::SlowDownForCharge: return L"SLOW_DOWN_FOR_CHARGE";
    case aidog::Action::SlowDownForProgram: return L"SLOW_DOWN_FOR_PROGRAM";
    case aidog::Action::UpAndDown: return L"UP_AND_DOWN";
    case aidog::Action::SitDown: return L"SIT_DOWN";
    case aidog::Action::SitDownForProgram: return L"SIT_DOWN_FOR_PROGRAM";
    case aidog::Action::StandUp: return L"STAND_UP";
    case aidog::Action::ShakeHand: return L"SHAKE_HAND";
    case aidog::Action::ShakeHandWithSitDown: return L"SHAKE_HAND_WITH_SIT_DOWN";
    case aidog::Action::Nod: return L"NOD";
    case aidog::Action::ShakeHead: return L"SHAKE_HEAD";
    case aidog::Action::Stretch: return L"STRETCH";
    case aidog::Action::Pee: return L"PEE";
    case aidog::Action::Twist: return L"TWIST";
    case aidog::Action::PushUp: return L"PUSH_UP";
    case aidog::Action::NewYear: return L"NEW_YEAR";
    case aidog::Action::WagTail: return L"WAG_TAIL";
    case aidog::Action::Stomp: return L"STOMP";
    case aidog::Action::Sniff: return L"SNIFF";
    case aidog::Action::Celebrate: return L"CELEBRATE";
    case aidog::Action::Jump: return L"JUMP";
    case aidog::Action::Dance: return L"DANCE";
    case aidog::Action::KickBall: return L"KICK_BALL";
    case aidog::Action::TouchGroundRight: return L"TOUCH_GROUND_RIGHT";
    case aidog::Action::TouchGroundLeft: return L"TOUCH_GROUND_LEFT";
    case aidog::Action::PlayDead: return L"PLAY_DEAD";
    case aidog::Action::StepInteraction: return L"STEP_INTERACTION";
    case aidog::Action::ForwardInteraction: return L"FORWARD_INTERACTION";
    case aidog::Action::BackInteraction: return L"BACK_INTERACTION";
    case aidog::Action::LeftInteraction: return L"LEFT_INTERACTION";
    case aidog::Action::RightInteraction: return L"RIGHT_INTERACTION";
    case aidog::Action::LowForwardAndBackwardInteraction: return L"LOW_FORWARD_AND_BACKWARD_INTERACTION";
    case aidog::Action::LowForwardInteraction: return L"LOW_FORWARD_INTERACTION";
    case aidog::Action::LowBackwardInteraction: return L"LOW_BACKWARD_INTERACTION";
    case aidog::Action::LowLeftInteraction: return L"LOW_LEFT_INTERACTION";
    case aidog::Action::LowRightInteraction: return L"LOW_RIGHT_INTERACTION";
    case aidog::Action::StopInteraction: return L"STOP_INTERACTION";
    case aidog::Action::UpAndDownForTest: return L"UP_AND_DOWN_FOR_TEST";
    case aidog::Action::RolloverRecoveryRight: return L"ROLLOVER_RECOVERY_RIGHT";
    case aidog::Action::RolloverRecoveryLeft: return L"ROLLOVER_RECOVERY_LEFT";
    case aidog::Action::Flailing: return L"FLAILING";
    case aidog::Action::StopFlailing: return L"STOP_FLAILING";
    case aidog::Action::LightOnInteraction: return L"LIGHT_ON_INTERACTION";
    case aidog::Action::LightOffInteraction: return L"LIGHT_OFF_INTERACTION";
    case aidog::Action::SwingLeftAndRight: return L"SWING_LEFT_AND_RIGHT";
    case aidog::Action::SwingLeft: return L"SWING_LEFT";
    case aidog::Action::SwingRight: return L"SWING_RIGHT";
    case aidog::Action::ExcitedInspace: return L"EXCITED_INSPACE";
    case aidog::Action::LazyPatPat: return L"LAZY_PAT_PAT";
    case aidog::Action::CheekyPaw: return L"CHEEKY_PAW";
    case aidog::Action::Whining: return L"WHINING";
    case aidog::Action::SniffForwardInteraction: return L"SNIFF_FORWARD_INTERACTION";
    case aidog::Action::SpaceBackwardInteraction: return L"SPACE_BACKWARD_INTERACTION";
    case aidog::Action::SniffLeftInteraction: return L"SNIFF_LEFT_INTERACTION";
    case aidog::Action::SniffRightInteraction: return L"SNIFF_RIGHT_INTERACTION";
    case aidog::Action::SniffStepInteraction: return L"SNIFF_STEP_INTERACTION";
    case aidog::Action::LeftAngleInteraction: return L"LEFT_ANGLE_INTERACTION";
    case aidog::Action::RightAngleInteraction: return L"RIGHT_ANGLE_INTERACTION";
    default: return L"UNKNOWN_ACTION";
    }
}

const std::vector<EnumButtonDef<aidog::EarAction>>& ear_defs()
{
    static const std::vector<EnumButtonDef<aidog::EarAction>> defs{
        {L"IDLE", aidog::EarAction::Idle},
        {L"EAR_SHAKE_ASYN_1_3", aidog::EarAction::EarShakeAsyn13},
        {L"EAR_SHAKE_ASYN_1_2", aidog::EarAction::EarShakeAsyn12},
        {L"EAR_SHAKE_SYN", aidog::EarAction::EarShakeSyn},
        {L"EAR_SHAKE_SYN_FOR_BLE", aidog::EarAction::EarShakeSynForBle},
        {L"EAR_PEAR1", aidog::EarAction::EarPear1},
        {L"EAR_PEAR2", aidog::EarAction::EarPear2},
        {L"EAR_PEAR3", aidog::EarAction::EarPear3},
        {L"EAR_STAND", aidog::EarAction::EarStand},
        {L"EAR_STAND_LEFT", aidog::EarAction::EarStandLeft},
        {L"EAR_STAND_RIGHT", aidog::EarAction::EarStandRight},
        {L"EAR_STAND_LEFT_AND_RIGHT", aidog::EarAction::EarStandLeftAndRight},
        {L"EAR_FOR_WINK", aidog::EarAction::EarForWink},
        {L"EAR_FOR_VIDEO", aidog::EarAction::EarForVideo},
        {L"EAR_PERCENTAGE_BASIC", aidog::EarAction::EarPercentageBasic},
        {L"EAR_FLICK_EXCITED", aidog::EarAction::EarFlickExcited},
        {L"EAR_FLICK_LEFT_QUICK", aidog::EarAction::EarFlickLeftQuick},
        {L"EAR_FLICK_RIGHT_QUICK", aidog::EarAction::EarFlickRightQuick},
        {L"EAR_FLICK_ALTERNATE", aidog::EarAction::EarFlickAlternate},
        {L"EAR_FLICK_LEFT_AND_RIGHT_UP", aidog::EarAction::EarFlickLeftAndRightUp},
        {L"EAR_FLICK_RANDOM", aidog::EarAction::EarFlickRandom},
        {L"EAR_WIGGLE_SUBTLE_SELF_STABLE", aidog::EarAction::EarWiggleSubtleSelfStable},
        {L"EAR_FLICK_RANDOM_NEGATIVE", aidog::EarAction::EarFlickRandomNegative},
        {L"EAR_FLICK_RANDOM_POSITIVE", aidog::EarAction::EarFlickRandomPositive},
        {L"EAR_BREATHE", aidog::EarAction::EarBreathe},
        {L"EAR_DOWN", aidog::EarAction::EarDown},
    };
    return defs;
}

const std::vector<EnumButtonDef<aidog::ExpressionAction>>& expression_defs()
{
    static const std::vector<EnumButtonDef<aidog::ExpressionAction>> defs{
        {L"IDLE", aidog::ExpressionAction::Idle},
        {L"HAPPY_01", aidog::ExpressionAction::Happy01},
        {L"HAPPY_02", aidog::ExpressionAction::Happy02},
        {L"HAPPY_03", aidog::ExpressionAction::Happy03},
        {L"HAPPY_04", aidog::ExpressionAction::Happy04},
        {L"SMILE_01", aidog::ExpressionAction::Smile01},
        {L"SMILE_02", aidog::ExpressionAction::Smile02},
        {L"SMILE_03", aidog::ExpressionAction::Smile03},
        {L"LOVE_01", aidog::ExpressionAction::Love01},
        {L"LOVE_02", aidog::ExpressionAction::Love02},
        {L"ANGER_01", aidog::ExpressionAction::Anger01},
        {L"ANGER_02", aidog::ExpressionAction::Anger02},
        {L"ANGER_03", aidog::ExpressionAction::Anger03},
        {L"ANGER_04", aidog::ExpressionAction::Anger04},
        {L"SAD_01", aidog::ExpressionAction::Sad01},
        {L"SAD_02", aidog::ExpressionAction::Sad02},
        {L"SAD_03", aidog::ExpressionAction::Sad03},
        {L"DOUBLE_SAD_03", aidog::ExpressionAction::DoubleSad03},
        {L"SAD_04", aidog::ExpressionAction::Sad04},
        {L"SCARED_01", aidog::ExpressionAction::Scared01},
        {L"SCARED_02", aidog::ExpressionAction::Scared02},
        {L"COMFORTABLE_01", aidog::ExpressionAction::Comfortable01},
        {L"COMFORTABLE_02", aidog::ExpressionAction::Comfortable02},
        {L"DOUBT_01", aidog::ExpressionAction::Doubt01},
        {L"DOUBT_02", aidog::ExpressionAction::Doubt02},
        {L"DOUBT_03", aidog::ExpressionAction::Doubt03},
        {L"NERVOUS", aidog::ExpressionAction::Nervous},
        {L"NERVOUS_COMPLETE", aidog::ExpressionAction::NervousComplete},
        {L"TIRED", aidog::ExpressionAction::Tired},
        {L"TIRED_COMPLETE", aidog::ExpressionAction::TiredComplete},
        {L"SLEEPY", aidog::ExpressionAction::Sleepy},
        {L"SLEEPY_COMPLETE", aidog::ExpressionAction::SleepyComplete},
        {L"WINK_FAST", aidog::ExpressionAction::WinkFast},
        {L"WINK_NORMAL", aidog::ExpressionAction::WinkNormal},
        {L"LOOK_RIGHT", aidog::ExpressionAction::LookRight},
        {L"LOOK_LEFT", aidog::ExpressionAction::LookLeft},
        {L"LOOK_LEFT_AND_RIGHT", aidog::ExpressionAction::LookLeftAndRight},
        {L"NOTE_PARTICLE_CIRCLE", aidog::ExpressionAction::NoteParticleCircle},
        {L"MUSIC", aidog::ExpressionAction::Music},
        {L"BASKETBALL", aidog::ExpressionAction::Basketball},
        {L"PINGPONG", aidog::ExpressionAction::Pingpong},
        {L"FOOTBALL", aidog::ExpressionAction::Football},
        {L"SOUND_0", aidog::ExpressionAction::Sound0},
        {L"SOUND_25", aidog::ExpressionAction::Sound25},
        {L"SOUND_50", aidog::ExpressionAction::Sound50},
        {L"SOUND_75", aidog::ExpressionAction::Sound75},
        {L"SOUND_100", aidog::ExpressionAction::Sound100},
        {L"SOUND_CIRCLE", aidog::ExpressionAction::SoundCircle},
        {L"GET_UP", aidog::ExpressionAction::GetUp},
        {L"EAT_SNACK", aidog::ExpressionAction::EatSnack},
        {L"CHARGING", aidog::ExpressionAction::Charging},
        {L"SUNGLASSES", aidog::ExpressionAction::Sunglasses},
        {L"EYES_FIGHTING", aidog::ExpressionAction::EyesFighting},
        {L"TURN_OFF", aidog::ExpressionAction::TurnOff},
        {L"CARING", aidog::ExpressionAction::Caring},
        {L"SHY", aidog::ExpressionAction::Shy},
        {L"DRINK", aidog::ExpressionAction::Drink},
        {L"ALERT", aidog::ExpressionAction::Alert},
        {L"BORING", aidog::ExpressionAction::Boring},
        {L"NO_WIFI", aidog::ExpressionAction::NoWifi},
        {L"SHAME", aidog::ExpressionAction::Shame},
        {L"SHAME_02", aidog::ExpressionAction::Shame02},
        {L"SNIFF_EXPRESSION", aidog::ExpressionAction::SniffExpression},
        {L"DEAD", aidog::ExpressionAction::Dead},
        {L"PRIDE", aidog::ExpressionAction::Pride},
        {L"YAWN", aidog::ExpressionAction::Yawn},
        {L"LIGHT_ON", aidog::ExpressionAction::LightOn},
        {L"LIGHT_OFF", aidog::ExpressionAction::LightOff},
    };
    return defs;
}

const std::vector<EnumButtonDef<aidog::Tone>>& tone_defs()
{
    static const std::vector<EnumButtonDef<aidog::Tone>> defs{
        {L"STOP", aidog::Tone::Stop},
        {L"JEEZ", aidog::Tone::Jeez},
        {L"UH", aidog::Tone::Uh},
        {L"EATING", aidog::Tone::Eating},
        {L"CHARGING", aidog::Tone::Charging},
        {L"CURIOUS", aidog::Tone::Curious},
        {L"SLEEPY", aidog::Tone::Sleepy},
        {L"HENG", aidog::Tone::Heng},
        {L"SAD", aidog::Tone::Sad},
        {L"ANGRY", aidog::Tone::Angry},
        {L"DOUBT", aidog::Tone::Doubt},
        {L"AGREE", aidog::Tone::Agree},
        {L"ENHENG", aidog::Tone::Enheng},
        {L"ALERT", aidog::Tone::Alert},
        {L"WAKE_UP", aidog::Tone::WakeUp},
        {L"COMFORT", aidog::Tone::Comfort},
        {L"SIGH", aidog::Tone::Sigh},
        {L"SNORE", aidog::Tone::Snore},
        {L"SNIFF", aidog::Tone::Sniff},
        {L"BEAT_1", aidog::Tone::Beat1},
        {L"BEAT_2", aidog::Tone::Beat2},
        {L"BEAT_3", aidog::Tone::Beat3},
        {L"BEAT_4", aidog::Tone::Beat4},
        {L"BEAT_5", aidog::Tone::Beat5},
        {L"BEAT_6", aidog::Tone::Beat6},
        {L"BEAT_7", aidog::Tone::Beat7},
    };
    return defs;
}

std::wstring clamp_edit_text(HWND edit)
{
    int len = GetWindowTextLengthW(edit);
    std::wstring text(static_cast<std::size_t>(len), L'\0');
    GetWindowTextW(edit, text.data(), len + 1);
    return text;
}

struct DeviceEntry {
    std::string name;
    std::string address;
};

struct SensorState {
    std::optional<aidog::ImuData> imu;
    std::optional<aidog::TofData> tof;
};

struct PlotPoint {
    DWORD tick = 0;
    double value = 0.0;
};

struct PlotSeries {
    std::wstring title;
    std::wstring unit;
    std::deque<PlotPoint> points;
    std::optional<double> latest;
};

struct TaskResult {
    std::wstring label;
    std::wstring message;
    bool ok = false;
};

struct ButtonDef {
    const wchar_t* text;
    int id;
};

LRESULT CALLBACK page_window_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
    if (message == WM_COMMAND || message == WM_NOTIFY || message == WM_HSCROLL) {
        return SendMessageW(GetParent(hwnd), message, wparam, lparam);
    }
    return DefWindowProcW(hwnd, message, wparam, lparam);
}

class Worker {
public:
    explicit Worker(HWND hwnd) : hwnd_(hwnd), thread_([this]() { run(); }) {}

    ~Worker()
    {
        stop();
    }

    void post(std::wstring label, std::function<void()> task)
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            tasks_.push({std::move(label), std::move(task)});
        }
        cv_.notify_one();
    }

    void stop()
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            stop_ = true;
        }
        cv_.notify_one();
        if (thread_.joinable()) {
            thread_.join();
        }
    }

private:
    struct Task {
        std::wstring label;
        std::function<void()> fn;
    };

    void run()
    {
        while (true) {
            Task task;
            {
                std::unique_lock<std::mutex> lock(mutex_);
                cv_.wait(lock, [this]() { return stop_ || !tasks_.empty(); });
                if (stop_ && tasks_.empty()) {
                    return;
                }
                task = std::move(tasks_.front());
                tasks_.pop();
            }

            auto* result = new TaskResult;
            result->label = task.label;
            try {
                task.fn();
                result->ok = true;
                result->message = L"OK";
            } catch (const std::exception& exc) {
                result->ok = false;
                result->message = to_wide(exc.what());
            } catch (...) {
                result->ok = false;
                result->message = L"Unknown error";
            }
            PostMessageW(hwnd_, WM_APP_TASK_DONE, 0, reinterpret_cast<LPARAM>(result));
        }
    }

    HWND hwnd_ = nullptr;
    std::mutex mutex_;
    std::condition_variable cv_;
    std::queue<Task> tasks_;
    bool stop_ = false;
    std::thread thread_;
};

class UserControlApp {
public:
    explicit UserControlApp(HINSTANCE instance) : instance_(instance) {}

    int run()
    {
        INITCOMMONCONTROLSEX icc{sizeof(INITCOMMONCONTROLSEX), ICC_TAB_CLASSES | ICC_STANDARD_CLASSES | ICC_BAR_CLASSES};
        InitCommonControlsEx(&icc);

        WNDCLASSW wc{};
        wc.lpfnWndProc = &UserControlApp::window_proc;
        wc.hInstance = instance_;
        wc.lpszClassName = kWindowClass;
        wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
        wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1);
        RegisterClassW(&wc);

        hwnd_ = CreateWindowExW(0, kWindowClass, kWindowTitle, WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT, 1180, 780, nullptr, nullptr, instance_, this);
        if (!hwnd_) {
            return 1;
        }

        ShowWindow(hwnd_, SW_SHOW);
        UpdateWindow(hwnd_);

        MSG msg{};
        while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
        return static_cast<int>(msg.wParam);
    }

private:
    static LRESULT CALLBACK window_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
    {
        auto* app = reinterpret_cast<UserControlApp*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
        if (message == WM_NCCREATE) {
            auto* create = reinterpret_cast<CREATESTRUCTW*>(lparam);
            app = reinterpret_cast<UserControlApp*>(create->lpCreateParams);
            SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(app));
            app->hwnd_ = hwnd;
        }
        if (!app) {
            return DefWindowProcW(hwnd, message, wparam, lparam);
        }
        return app->handle_message(message, wparam, lparam);
    }

    static LRESULT CALLBACK joystick_window_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
    {
        auto* app = reinterpret_cast<UserControlApp*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
        if (!app) {
            return DefWindowProcW(hwnd, message, wparam, lparam);
        }
        return app->handle_joystick_message(hwnd, message, wparam, lparam);
    }

    static LRESULT CALLBACK plot_window_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
    {
        auto* app = reinterpret_cast<UserControlApp*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
        if (!app) {
            return DefWindowProcW(hwnd, message, wparam, lparam);
        }
        return app->handle_plot_message(hwnd, message, wparam, lparam);
    }

    LRESULT handle_message(UINT message, WPARAM wparam, LPARAM lparam)
    {
        switch (message) {
        case WM_CREATE:
            on_create();
            return 0;
        case WM_SIZE:
            layout();
            return 0;
        case WM_COMMAND:
            on_command(LOWORD(wparam));
            return 0;
        case WM_HSCROLL:
            on_hscroll(reinterpret_cast<HWND>(lparam));
            return 0;
        case WM_NOTIFY:
            on_notify(reinterpret_cast<NMHDR*>(lparam));
            return 0;
        case WM_TIMER:
            if (wparam == kRefreshTimer) {
                refresh_sensor_text();
            } else if (wparam == kPlotTimer) {
                flush_pending_ear_percentage();
                refresh_plots();
            }
            return 0;
        case WM_APP_TASK_DONE:
            on_task_done(reinterpret_cast<TaskResult*>(lparam));
            return 0;
        case WM_APP_SENSOR:
            refresh_sensor_text();
            return 0;
#ifdef AIDOG_USER_CONTROL_WS
        case WM_APP_WS_STATE:
            connected_.store(wparam != 0);
            SetWindowTextW(statusLabel_, connected_.load() ? L"WS Status: connected" : L"WS Status: waiting");
            append_log(connected_.load() ? L"Robot connected" : L"Robot disconnected");
            return 0;
#endif
        case WM_CLOSE:
            on_close();
            DestroyWindow(hwnd_);
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        default:
            return DefWindowProcW(hwnd_, message, wparam, lparam);
        }
    }

    LRESULT handle_plot_message(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
    {
        if (message == WM_PAINT) {
            paint_plot(hwnd);
            return 0;
        }
        return DefWindowProcW(hwnd, message, wparam, lparam);
    }

    LRESULT handle_joystick_message(HWND hwnd, UINT message, WPARAM, LPARAM lparam)
    {
        switch (message) {
        case WM_PAINT:
            paint_joystick(hwnd);
            return 0;
        case WM_LBUTTONDOWN:
            SetCapture(hwnd);
            update_joystick(GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam), false);
            return 0;
        case WM_MOUSEMOVE:
            if (GetCapture() == hwnd) {
                update_joystick(GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam), false);
            }
            return 0;
        case WM_LBUTTONUP:
            if (GetCapture() == hwnd) {
                ReleaseCapture();
            }
            update_joystick(GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam), true);
            return 0;
        case WM_CAPTURECHANGED:
            reset_joystick(true);
            return 0;
        default:
            return DefWindowProcW(hwnd, message, 0, lparam);
        }
    }

    void on_create()
    {
        worker_ = std::make_unique<Worker>(hwnd_);
        dog_.add_imu_listener([this](const aidog::ImuData& imu) {
            {
                std::lock_guard<std::mutex> lock(sensorMutex_);
                sensors_.imu = imu;
                const DWORD tick = GetTickCount();
                if (imu.yawDeg.has_value()) {
                    add_plot_point(0, *imu.yawDeg, tick);
                }
                if (imu.pitchDeg.has_value()) {
                    add_plot_point(1, *imu.pitchDeg, tick);
                }
                if (imu.rollDeg.has_value()) {
                    add_plot_point(2, *imu.rollDeg, tick);
                }
            }
            PostMessageW(hwnd_, WM_APP_SENSOR, 0, 0);
        });
        dog_.add_tof_listener([this](const aidog::TofData& tof) {
            {
                std::lock_guard<std::mutex> lock(sensorMutex_);
                sensors_.tof = tof;
                const DWORD tick = GetTickCount();
                auto front = json_number(tof.raw, {"front_mm", "front", "tof_front", "distance_mm"});
                auto oblique = json_number(tof.raw, {"oblique_mm", "oblique", "tof_oblique", "side_mm"});
                if (front.has_value()) {
                    add_plot_point(3, *front, tick);
                }
                if (oblique.has_value()) {
                    add_plot_point(4, *oblique, tick);
                }
            }
            PostMessageW(hwnd_, WM_APP_SENSOR, 0, 0);
        });

        create_top_bar();
        create_tabs();
        create_log();
        SetTimer(hwnd_, kRefreshTimer, 500, nullptr);
        SetTimer(hwnd_, kPlotTimer, 100, nullptr);
        init_plots();
        append_log(L"Ready");
    }

    HWND make_button(HWND parent, const wchar_t* text, int id)
    {
        return CreateWindowExW(0, L"BUTTON", text, WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            0, 0, 100, 28, parent, reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)), instance_, nullptr);
    }

    HWND make_label(HWND parent, const wchar_t* text)
    {
        return CreateWindowExW(0, L"STATIC", text, WS_CHILD | WS_VISIBLE,
            0, 0, 100, 24, parent, nullptr, instance_, nullptr);
    }

    void create_top_bar()
    {
#ifdef AIDOG_USER_CONTROL_WS
        prefixEdit_ = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"0.0.0.0", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
            0, 0, 120, 24, hwnd_, reinterpret_cast<HMENU>(IdPrefix), instance_, nullptr);
        deviceCombo_ = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"8766", WS_CHILD | WS_VISIBLE | ES_NUMBER,
            0, 0, 80, 24, hwnd_, reinterpret_cast<HMENU>(IdDeviceCombo), instance_, nullptr);
        scanButton_ = make_button(hwnd_, L"Start WS Host", IdScan);
        connectButton_ = make_button(hwnd_, L"Wait Robot", IdConnect);
        disconnectButton_ = make_button(hwnd_, L"Stop WS Host", IdDisconnect);
        statusLabel_ = make_label(hwnd_, L"WS Status: stopped");
#else
        prefixEdit_ = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"Gogobot", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
            0, 0, 120, 24, hwnd_, reinterpret_cast<HMENU>(IdPrefix), instance_, nullptr);
        scanButton_ = make_button(hwnd_, L"Scan", IdScan);
        deviceCombo_ = CreateWindowExW(0, WC_COMBOBOXW, L"", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
            0, 0, 320, 200, hwnd_, reinterpret_cast<HMENU>(IdDeviceCombo), instance_, nullptr);
        connectButton_ = make_button(hwnd_, L"Connect", IdConnect);
        disconnectButton_ = make_button(hwnd_, L"Disconnect", IdDisconnect);
        statusLabel_ = make_label(hwnd_, L"Status: disconnected");
#endif
    }

    void create_tabs()
    {
        tabs_ = CreateWindowExW(0, WC_TABCONTROLW, L"", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
            0, 0, 100, 100, hwnd_, reinterpret_cast<HMENU>(IdTabs), instance_, nullptr);
        const wchar_t* names[] = {L"Motion/Actions", L"Ears", L"Expression", L"Audio/Volume", L"Sensors"};
        for (int i = 0; i < 5; ++i) {
            TCITEMW item{};
            item.mask = TCIF_TEXT;
            item.pszText = const_cast<wchar_t*>(names[i]);
            TabCtrl_InsertItem(tabs_, i, &item);
            pages_[i] = CreateWindowExW(0, L"STATIC", L"", WS_CHILD | (i == 0 ? WS_VISIBLE : 0),
                0, 0, 100, 100, hwnd_, nullptr, instance_, nullptr);
            SetWindowLongPtrW(pages_[i], GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&page_window_proc));
        }
        create_motion_page();
        create_ear_page();
        create_expression_page();
        create_audio_page();
        create_sensor_page();
    }

    void create_buttons(HWND parent, std::vector<HWND>& out, const ButtonDef* defs, int count)
    {
        for (int i = 0; i < count; ++i) {
            out.push_back(make_button(parent, defs[i].text, defs[i].id));
        }
    }

    void create_motion_page()
    {
        motionTitle_ = make_label(pages_[0], L"Joystick movement");
        actionTitle_ = make_label(pages_[0], L"Interaction actions");
        joystick_ = CreateWindowExW(WS_EX_CLIENTEDGE, L"STATIC", L"", WS_CHILD | WS_VISIBLE | SS_NOTIFY,
            0, 0, kJoystickSize, kJoystickSize, pages_[0], reinterpret_cast<HMENU>(IdJoystick), instance_, nullptr);
        SetWindowLongPtrW(joystick_, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&joystick_window_proc));
        SetWindowLongPtrW(joystick_, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

        stepButton_ = make_button(pages_[0], L"STEP", 3005);
        stopButton_ = make_button(pages_[0], L"STOP MOVEMENT", 3006);

        for (const auto& spec : aidog::action_specs()) {
            const int id = 4000 + static_cast<int>(actionButtons_.size());
            actionMap_[id] = spec.action;
            actionButtons_.push_back(make_button(pages_[0], action_text(spec.action), id));
        }
    }

    void create_ear_page()
    {
        earTitle_ = make_label(pages_[1], L"Ear position");
        earActionTitle_ = make_label(pages_[1], L"Ear actions");
        earSlider_ = CreateWindowExW(0, TRACKBAR_CLASSW, L"", WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS,
            0, 0, 240, 32, pages_[1], nullptr, instance_, nullptr);
        SendMessageW(earSlider_, TBM_SETRANGE, TRUE, MAKELONG(0, 100));
        SendMessageW(earSlider_, TBM_SETPOS, TRUE, 50);
        earValueLabel_ = make_label(pages_[1], L"50%");
        earSendButton_ = make_button(pages_[1], L"Send Ear Percent", 5100);

        for (const auto& def : ear_defs()) {
            const int id = 5200 + static_cast<int>(earButtons_.size());
            earMap_[id] = def.value;
            earButtons_.push_back(make_button(pages_[1], def.text, id));
        }
    }

    void create_expression_page()
    {
        expressionTitle_ = make_label(pages_[2], L"Expression test");
        for (const auto& def : expression_defs()) {
            const int id = 6000 + static_cast<int>(expressionButtons_.size());
            expressionMap_[id] = def.value;
            expressionButtons_.push_back(make_button(pages_[2], def.text, id));
        }
    }

    void create_audio_page()
    {
        audioTitle_ = make_label(pages_[3], L"Audio test");
        volumeTitle_ = make_label(pages_[3], L"Volume level");
        for (const auto& def : tone_defs()) {
            const int id = 7000 + static_cast<int>(toneButtons_.size());
            toneMap_[id] = def.value;
            toneButtons_.push_back(make_button(pages_[3], def.text, id));
        }
        for (int level = 0; level <= 4; ++level) {
            auto text = std::to_wstring(level) + L" Level";
            volumeButtons_.push_back(make_button(pages_[3], text.c_str(), 7100 + level));
        }
    }

    void create_sensor_page()
    {
        sensorTitle_ = make_label(pages_[4], L"Sensor streams");
        imuCheck_ = CreateWindowExW(0, L"BUTTON", L"IMU Stream", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
            0, 0, 120, 24, pages_[4], reinterpret_cast<HMENU>(IdImuCheck), instance_, nullptr);
        tofCheck_ = CreateWindowExW(0, L"BUTTON", L"TOF Stream", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
            0, 0, 120, 24, pages_[4], reinterpret_cast<HMENU>(IdTofCheck), instance_, nullptr);
        hzEdit_ = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"20", WS_CHILD | WS_VISIBLE | ES_NUMBER,
            0, 0, 50, 24, pages_[4], reinterpret_cast<HMENU>(IdHzEdit), instance_, nullptr);
        applyHzButton_ = make_button(pages_[4], L"Apply Hz", IdApplyHz);
        clearPlotsButton_ = make_button(pages_[4], L"Clear Plots", IdClearPlots);
        for (int i = 0; i < 5; ++i) {
            plotWindows_[i] = CreateWindowExW(WS_EX_CLIENTEDGE, L"STATIC", L"", WS_CHILD | WS_VISIBLE | SS_NOTIFY,
                0, 0, 100, 100, pages_[4], reinterpret_cast<HMENU>(static_cast<INT_PTR>(IdPlotBase + i)), instance_, nullptr);
            SetWindowLongPtrW(plotWindows_[i], GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&plot_window_proc));
            SetWindowLongPtrW(plotWindows_[i], GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
        }
        sensorText_ = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"No data", WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_READONLY | WS_VSCROLL,
            0, 0, 400, 260, pages_[4], reinterpret_cast<HMENU>(IdSensorText), instance_, nullptr);
    }

    void create_log()
    {
        logEdit_ = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_READONLY | WS_VSCROLL,
            0, 0, 100, 100, hwnd_, reinterpret_cast<HMENU>(IdLog), instance_, nullptr);
    }

    void layout()
    {
        RECT client{};
        GetClientRect(hwnd_, &client);
        const int width = client.right;
        const int height = client.bottom;
        const int pad = 8;
        const int topH = 38;
        const int logH = 140;

#ifdef AIDOG_USER_CONTROL_WS
        MoveWindow(prefixEdit_, pad, pad, 140, 24, TRUE);
        MoveWindow(deviceCombo_, 156, pad, 70, 24, TRUE);
        MoveWindow(scanButton_, 234, pad, 120, 26, TRUE);
        MoveWindow(connectButton_, 362, pad, 92, 26, TRUE);
        MoveWindow(disconnectButton_, 462, pad, 120, 26, TRUE);
        MoveWindow(statusLabel_, 592, pad + 4, std::max(100, width - 600), 24, TRUE);
#else
        MoveWindow(prefixEdit_, pad, pad, 140, 24, TRUE);
        MoveWindow(scanButton_, 156, pad, 70, 26, TRUE);
        MoveWindow(deviceCombo_, 234, pad, 360, 300, TRUE);
        MoveWindow(connectButton_, 602, pad, 76, 26, TRUE);
        MoveWindow(disconnectButton_, 686, pad, 90, 26, TRUE);
        MoveWindow(statusLabel_, 786, pad + 4, std::max(100, width - 794), 24, TRUE);
#endif

        MoveWindow(tabs_, pad, topH, width - pad * 2, height - topH - logH - pad * 2, TRUE);
        RECT tabRect{};
        GetClientRect(tabs_, &tabRect);
        TabCtrl_AdjustRect(tabs_, FALSE, &tabRect);
        MapWindowPoints(tabs_, hwnd_, reinterpret_cast<POINT*>(&tabRect), 2);
        for (auto page : pages_) {
            MoveWindow(page, tabRect.left, tabRect.top, tabRect.right - tabRect.left, tabRect.bottom - tabRect.top, TRUE);
        }
        layout_pages(tabRect.right - tabRect.left, tabRect.bottom - tabRect.top);
        MoveWindow(logEdit_, pad, height - logH - pad, width - pad * 2, logH, TRUE);
    }

    void layout_pages(int width, int height)
    {
        MoveWindow(motionTitle_, 18, 18, 180, 24, TRUE);
        MoveWindow(joystick_, 18, 52, kJoystickSize, kJoystickSize, TRUE);
        MoveWindow(stepButton_, 18, 328, kJoystickSize, 34, TRUE);
        MoveWindow(stopButton_, 18, 370, kJoystickSize, 34, TRUE);

        const int actionX = 304;
        const int actionButtonW = std::max(120, (width - actionX - 28) / 5 - 8);
        MoveWindow(actionTitle_, actionX, 18, 180, 24, TRUE);
        for (std::size_t i = 0; i < actionButtons_.size(); ++i) {
            MoveWindow(actionButtons_[i],
                actionX + static_cast<int>(i % 5) * (actionButtonW + 8),
                56 + static_cast<int>(i / 5) * 38,
                actionButtonW,
                30,
                TRUE);
        }

        MoveWindow(earTitle_, 18, 18, 180, 24, TRUE);
        MoveWindow(earSlider_, 18, 52, std::max(360, width - 120), 36, TRUE);
        MoveWindow(earValueLabel_, width - 84, 56, 60, 24, TRUE);
        ShowWindow(earSendButton_, SW_HIDE);
        MoveWindow(earActionTitle_, 18, 104, 180, 24, TRUE);
        const int earButtonW = std::max(160, (width - 44) / 4 - 8);
        for (std::size_t i = 0; i < earButtons_.size(); ++i) {
            MoveWindow(earButtons_[i],
                18 + static_cast<int>(i % 4) * (earButtonW + 8),
                138 + static_cast<int>(i / 4) * 42,
                earButtonW,
                34,
                TRUE);
        }

        MoveWindow(expressionTitle_, 18, 18, 180, 24, TRUE);
        const int expressionButtonW = std::max(120, (width - 48) / 6 - 8);
        for (std::size_t i = 0; i < expressionButtons_.size(); ++i) {
            MoveWindow(expressionButtons_[i],
                18 + static_cast<int>(i % 6) * (expressionButtonW + 8),
                56 + static_cast<int>(i / 6) * 38,
                expressionButtonW,
                30,
                TRUE);
        }

        MoveWindow(audioTitle_, 18, 18, 180, 24, TRUE);
        const int toneButtonW = std::max(120, (width - 48) / 6 - 8);
        for (std::size_t i = 0; i < toneButtons_.size(); ++i) {
            MoveWindow(toneButtons_[i],
                18 + static_cast<int>(i % 6) * (toneButtonW + 8),
                56 + static_cast<int>(i / 6) * 38,
                toneButtonW,
                30,
                TRUE);
        }
        MoveWindow(volumeTitle_, 18, 238, 180, 24, TRUE);
        for (std::size_t i = 0; i < volumeButtons_.size(); ++i) {
            MoveWindow(volumeButtons_[i], 18 + static_cast<int>(i) * 100, 272, 90, 38, TRUE);
        }

        MoveWindow(sensorTitle_, 18, 18, 180, 24, TRUE);
        MoveWindow(imuCheck_, 18, 52, 120, 24, TRUE);
        MoveWindow(tofCheck_, 148, 52, 120, 24, TRUE);
        MoveWindow(hzEdit_, 280, 52, 60, 24, TRUE);
        MoveWindow(applyHzButton_, 352, 50, 92, 28, TRUE);
        MoveWindow(clearPlotsButton_, 456, 50, 110, 28, TRUE);

        const int leftW = std::max(320, (width - 54) / 2);
        const int rightW = std::max(320, width - leftW - 54);
        const int topY = 92;
        const int gap = 12;
        const int availableH = std::max(330, height - topY - 18);
        const int plotH = std::max(100, (availableH - gap * 2) / 3);
        const int row1 = topY;
        const int row2 = topY + plotH + gap;
        const int row3 = topY + (plotH + gap) * 2;
        MoveWindow(plotWindows_[0], 18, row1, leftW, plotH, TRUE);
        MoveWindow(plotWindows_[1], 18, row2, leftW, plotH, TRUE);
        MoveWindow(plotWindows_[2], 18, row3, leftW, plotH, TRUE);
        MoveWindow(plotWindows_[3], 36 + leftW, row1, rightW, plotH, TRUE);
        MoveWindow(plotWindows_[4], 36 + leftW, row2, rightW, plotH, TRUE);
        MoveWindow(sensorText_, 36 + leftW, row3, rightW, plotH, TRUE);
    }

    void on_command(int id)
    {
        if (id == IdScan) {
            scan_devices();
        } else if (id == IdConnect) {
            connect_device();
        } else if (id == IdDisconnect) {
            disconnect_device();
        } else if (id == 5100) {
            send_ear_percentage();
        } else if (id == IdImuCheck) {
            toggle_imu();
        } else if (id == IdTofCheck) {
            toggle_tof();
        } else if (id == IdApplyHz) {
            apply_sensor_hz();
        } else if (id == IdClearPlots) {
            clear_plots();
        } else if (id == 3005 || id == 3006) {
            handle_movement(id);
        } else if (auto it = actionMap_.find(id); it != actionMap_.end()) {
            post_command(L"Action", [this, action = it->second]() { dog_.send_interaction(action, std::nullopt, kControlTransport); });
        } else if (auto it = earMap_.find(id); it != earMap_.end()) {
            post_command(L"Ear action", [this, action = it->second]() { dog_.send_ear(action, kControlTransport); });
        } else if (auto it = expressionMap_.find(id); it != expressionMap_.end()) {
            post_command(L"Expression", [this, expression = it->second]() { dog_.send_expression(expression, kControlTransport); });
        } else if (auto it = toneMap_.find(id); it != toneMap_.end()) {
            post_command(L"Audio", [this, tone = it->second]() { dog_.send_audio(tone, kControlTransport); });
        } else if (id >= 7100 && id <= 7104) {
            const int volume = id - 7100;
            post_command(L"Volume", [this, volume]() { dog_.set_volume(volume, std::nullopt, 0.2, kControlTransport); });
        }
    }

    void on_notify(NMHDR* hdr)
    {
        if (hdr->hwndFrom == tabs_ && hdr->code == TCN_SELCHANGE) {
            const int current = TabCtrl_GetCurSel(tabs_);
            for (int i = 0; i < 5; ++i) {
                ShowWindow(pages_[i], i == current ? SW_SHOW : SW_HIDE);
            }
        }
    }

    void on_hscroll(HWND control)
    {
        if (control != earSlider_) {
            return;
        }
        const int value = static_cast<int>(SendMessageW(earSlider_, TBM_GETPOS, 0, 0));
        SetWindowTextW(earValueLabel_, (std::to_wstring(value) + L"%").c_str());
        queue_ear_percentage(value);
    }

    void paint_joystick(HWND hwnd)
    {
        PAINTSTRUCT ps{};
        HDC hdc = BeginPaint(hwnd, &ps);
        RECT rc{};
        GetClientRect(hwnd, &rc);
        FillRect(hdc, &rc, reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1));

        const int cx = (rc.right - rc.left) / 2;
        const int cy = (rc.bottom - rc.top) / 2;
        const int radius = std::min(cx, cy) - 28;
        const int knobRadius = 25;

        HPEN gridPen = CreatePen(PS_SOLID, 1, RGB(210, 216, 225));
        HPEN borderPen = CreatePen(PS_SOLID, 2, RGB(150, 160, 176));
        HBRUSH diskBrush = CreateSolidBrush(RGB(248, 250, 252));
        HBRUSH knobBrush = CreateSolidBrush(RGB(31, 111, 235));
        HBRUSH oldBrush = static_cast<HBRUSH>(SelectObject(hdc, diskBrush));
        HPEN oldPen = static_cast<HPEN>(SelectObject(hdc, borderPen));

        Ellipse(hdc, cx - radius, cy - radius, cx + radius, cy + radius);
        SelectObject(hdc, gridPen);
        MoveToEx(hdc, cx, cy - radius + 12, nullptr);
        LineTo(hdc, cx, cy + radius - 12);
        MoveToEx(hdc, cx - radius + 12, cy, nullptr);
        LineTo(hdc, cx + radius - 12, cy);

        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, RGB(70, 78, 92));
        TextOutW(hdc, cx - 18, 8, L"Forward", 7);
        TextOutW(hdc, cx - 14, rc.bottom - 24, L"Back", 4);
        TextOutW(hdc, 10, cy - 8, L"Left", 4);
        TextOutW(hdc, rc.right - 42, cy - 8, L"Right", 5);

        SelectObject(hdc, knobBrush);
        SelectObject(hdc, borderPen);
        Ellipse(hdc,
            joystickKnobX_ - knobRadius,
            joystickKnobY_ - knobRadius,
            joystickKnobX_ + knobRadius,
            joystickKnobY_ + knobRadius);

        SelectObject(hdc, oldPen);
        SelectObject(hdc, oldBrush);
        DeleteObject(gridPen);
        DeleteObject(borderPen);
        DeleteObject(diskBrush);
        DeleteObject(knobBrush);
        EndPaint(hwnd, &ps);
    }

    void update_joystick(int x, int y, bool release)
    {
        const int cx = kJoystickSize / 2;
        const int cy = kJoystickSize / 2;
        if (release) {
            reset_joystick(true);
            return;
        }

        double dx = static_cast<double>(x - cx);
        double dy = static_cast<double>(y - cy);
        double dist = std::sqrt(dx * dx + dy * dy);
        constexpr double maxRadius = 88.0;
        if (dist > maxRadius) {
            const double scale = maxRadius / dist;
            dx *= scale;
            dy *= scale;
            dist = maxRadius;
        }

        joystickKnobX_ = cx + static_cast<int>(std::round(dx));
        joystickKnobY_ = cy + static_cast<int>(std::round(dy));
        InvalidateRect(joystick_, nullptr, TRUE);

        if (dist < kJoystickDeadzone) {
            stop_active_movement();
            return;
        }

        aidog::Movement movement = aidog::Movement::Forward;
        if (std::abs(dx) > std::abs(dy)) {
            movement = dx > 0 ? aidog::Movement::Right : aidog::Movement::Left;
        } else {
            movement = dy > 0 ? aidog::Movement::Back : aidog::Movement::Forward;
        }
        start_active_movement(movement);
    }

    void reset_joystick(bool stop)
    {
        joystickKnobX_ = kJoystickSize / 2;
        joystickKnobY_ = kJoystickSize / 2;
        if (joystick_) {
            InvalidateRect(joystick_, nullptr, TRUE);
        }
        if (stop) {
            stop_active_movement();
        }
    }

    void init_plots()
    {
        plotSeries_[0].title = L"IMU yaw";
        plotSeries_[0].unit = L"deg";
        plotSeries_[1].title = L"IMU pitch";
        plotSeries_[1].unit = L"deg";
        plotSeries_[2].title = L"IMU roll";
        plotSeries_[2].unit = L"deg";
        plotSeries_[3].title = L"TOF front";
        plotSeries_[3].unit = L"mm";
        plotSeries_[4].title = L"TOF oblique";
        plotSeries_[4].unit = L"mm";
    }

    void add_plot_point(int index, double value, DWORD tick)
    {
        if (index < 0 || index >= 5) {
            return;
        }
        auto& series = plotSeries_[index];
        series.latest = value;
        series.points.push_back({tick, value});
        while (!series.points.empty() && tick - series.points.front().tick > kPlotHistoryMs) {
            series.points.pop_front();
        }
    }

    void refresh_plots()
    {
        for (auto hwnd : plotWindows_) {
            if (hwnd) {
                InvalidateRect(hwnd, nullptr, TRUE);
            }
        }
    }

    void clear_plots()
    {
        for (auto& series : plotSeries_) {
            series.points.clear();
            series.latest.reset();
        }
        refresh_plots();
        append_log(L"Sensor plots cleared");
    }

    std::optional<double> json_number(const nlohmann::json& value, std::initializer_list<const char*> keys)
    {
        for (const auto* key : keys) {
            auto it = value.find(key);
            if (it != value.end() && it->is_number()) {
                return it->get<double>();
            }
        }
        return std::nullopt;
    }

    void paint_plot(HWND hwnd)
    {
        int index = static_cast<int>(GetWindowLongPtrW(hwnd, GWLP_ID)) - IdPlotBase;
        if (index < 0 || index >= 5) {
            return;
        }

        PAINTSTRUCT ps{};
        HDC hdc = BeginPaint(hwnd, &ps);
        RECT rc{};
        GetClientRect(hwnd, &rc);
        FillRect(hdc, &rc, reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1));

        auto& series = plotSeries_[index];
        const int width = rc.right - rc.left;
        const int height = rc.bottom - rc.top;
        const int left = 42;
        const int right = width - 8;
        const int top = 24;
        const int bottom = height - 22;

        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, RGB(20, 24, 32));
        TextOutW(hdc, 8, 5, series.title.c_str(), static_cast<int>(series.title.size()));
        if (series.latest.has_value()) {
            const auto latest = number_text(*series.latest) + L" " + series.unit;
            TextOutW(hdc, std::max(8, width - 110), 5, latest.c_str(), static_cast<int>(latest.size()));
        }

        HPEN gridPen = CreatePen(PS_SOLID, 1, RGB(235, 238, 244));
        HPEN axisPen = CreatePen(PS_SOLID, 1, RGB(198, 204, 214));
        HPEN linePen = CreatePen(PS_SOLID, 2, RGB(31, 111, 235));
        HPEN oldPen = static_cast<HPEN>(SelectObject(hdc, gridPen));
        for (int i = 0; i <= 4; ++i) {
            const int y = top + (bottom - top) * i / 4;
            MoveToEx(hdc, left, y, nullptr);
            LineTo(hdc, right, y);
            const int x = left + (right - left) * i / 4;
            MoveToEx(hdc, x, top, nullptr);
            LineTo(hdc, x, bottom);
        }
        SelectObject(hdc, axisPen);
        Rectangle(hdc, left, top, right, bottom);

        if (series.points.empty()) {
            const wchar_t text[] = L"No data";
            TextOutW(hdc, width / 2 - 24, height / 2 - 8, text, 7);
        } else {
            double minValue = series.points.front().value;
            double maxValue = series.points.front().value;
            for (const auto& point : series.points) {
                minValue = std::min(minValue, point.value);
                maxValue = std::max(maxValue, point.value);
            }
            if (std::abs(maxValue - minValue) < 1e-9) {
                const double margin = std::max(1.0, std::abs(maxValue) * 0.1);
                minValue -= margin;
                maxValue += margin;
            } else {
                const double margin = (maxValue - minValue) * 0.12;
                minValue -= margin;
                maxValue += margin;
            }

            const DWORD now = GetTickCount();
            SelectObject(hdc, linePen);
            bool first = true;
            for (const auto& point : series.points) {
                const double age = static_cast<double>(now - point.tick);
                const double xRatio = std::clamp(1.0 - age / static_cast<double>(kPlotHistoryMs), 0.0, 1.0);
                const double yRatio = (maxValue - point.value) / (maxValue - minValue);
                const int x = left + static_cast<int>(xRatio * (right - left));
                const int y = top + static_cast<int>(yRatio * (bottom - top));
                if (first) {
                    MoveToEx(hdc, x, y, nullptr);
                    first = false;
                } else {
                    LineTo(hdc, x, y);
                }
            }

            SetTextColor(hdc, RGB(92, 100, 112));
            auto maxText = number_text(maxValue);
            auto minText = number_text(minValue);
            TextOutW(hdc, 4, top, maxText.c_str(), static_cast<int>(maxText.size()));
            TextOutW(hdc, 4, bottom - 14, minText.c_str(), static_cast<int>(minText.size()));
        }

        SetTextColor(hdc, RGB(92, 100, 112));
        TextOutW(hdc, left, height - 18, L"-12s", 4);
        TextOutW(hdc, right - 28, height - 18, L"now", 3);

        SelectObject(hdc, oldPen);
        DeleteObject(gridPen);
        DeleteObject(axisPen);
        DeleteObject(linePen);
        EndPaint(hwnd, &ps);
    }

    void scan_devices()
    {
#ifdef AIDOG_USER_CONTROL_WS
        start_ws_host();
#else
        const auto prefix = clamp_edit_text(prefixEdit_);
        append_log(L"Start scan");
        worker_->post(L"Scan", [this, prefix]() {
            auto found = dog_.scan(to_utf8(prefix));
            std::lock_guard<std::mutex> lock(deviceMutex_);
            devices_.clear();
            for (const auto& item : found) {
                devices_.push_back({item.name, item.address});
            }
        });
#endif
    }

    void connect_device()
    {
#ifdef AIDOG_USER_CONTROL_WS
        wait_ws_robot();
#else
        const int index = ComboBox_GetCurSel(deviceCombo_);
        DeviceEntry device;
        {
            std::lock_guard<std::mutex> lock(deviceMutex_);
            if (index < 0 || index >= static_cast<int>(devices_.size())) {
                append_log(L"Scan and select a device first");
                return;
            }
            device = devices_[static_cast<std::size_t>(index)];
        }
        worker_->post(L"Connect", [this, device]() {
            aidog::ConnectOptions options;
            options.address = device.address;
            dog_.connect(options);
            connected_.store(true);
        });
#endif
    }

    void disconnect_device()
    {
#ifdef AIDOG_USER_CONTROL_WS
        worker_->post(L"Stop WS Host", [this]() {
            activeMovement_.reset();
            cleanup_robot();
            connected_.store(false);
        });
#else
        worker_->post(L"Disconnect", [this]() {
            activeMovement_.reset();
            cleanup_robot();
            connected_.store(false);
        });
#endif
        reset_joystick(false);
    }

    void handle_movement(int id)
    {
        if (id == 3006) {
            reset_joystick(true);
            return;
        }
        aidog::Movement movement = aidog::Movement::Forward;
        if (id == 3002) {
            movement = aidog::Movement::Back;
        } else if (id == 3003) {
            movement = aidog::Movement::Left;
        } else if (id == 3004) {
            movement = aidog::Movement::Right;
        } else if (id == 3005) {
            movement = aidog::Movement::Step;
        }
        start_active_movement(movement);
    }

    void start_active_movement(aidog::Movement movement)
    {
        if (activeMovement_.has_value() && activeMovement_.value() == movement) {
            return;
        }
        activeMovement_ = movement;
        post_command(L"Movement", [this, movement]() { dog_.start_movement(movement, kControlTransport); });
    }

    void stop_active_movement()
    {
        if (!activeMovement_.has_value()) {
            return;
        }
        activeMovement_.reset();
        post_command(L"Stop movement", [this]() { dog_.stop_movement(kControlTransport); });
    }

    void send_ear_percentage()
    {
        const int value = static_cast<int>(SendMessageW(earSlider_, TBM_GETPOS, 0, 0));
        SetWindowTextW(earValueLabel_, (std::to_wstring(value) + L"%").c_str());
        post_command(L"Ear percent", [this, value]() { dog_.send_ear_percentage(value, kControlTransport); });
    }

    void queue_ear_percentage(int value)
    {
        if (!connected_.load()) {
            return;
        }
        const DWORD now = GetTickCount();
        if (lastEarPercent_.has_value() && *lastEarPercent_ == value && now - lastEarPercentTick_ < 250) {
            return;
        }
        if (now - lastEarPercentTick_ < kEarSendIntervalMs) {
            pendingEarPercent_ = value;
            return;
        }
        send_ear_percentage_value(value);
    }

    void flush_pending_ear_percentage()
    {
        if (!pendingEarPercent_.has_value()) {
            return;
        }
        const DWORD now = GetTickCount();
        if (now - lastEarPercentTick_ < kEarSendIntervalMs) {
            return;
        }
        const int value = *pendingEarPercent_;
        pendingEarPercent_.reset();
        send_ear_percentage_value(value);
    }

    void send_ear_percentage_value(int value)
    {
        lastEarPercent_ = value;
        lastEarPercentTick_ = GetTickCount();
        post_command(L"Ear percent", [this, value]() { dog_.send_ear_percentage(value, kControlTransport); });
    }

    void toggle_imu()
    {
        const bool enabled = Button_GetCheck(imuCheck_) == BST_CHECKED;
        const int hz = get_hz();
        post_command(enabled ? L"Enable IMU" : L"Disable IMU", [this, enabled, hz]() { dog_.request_imu_stream(enabled, hz, kControlTransport); });
    }

    void toggle_tof()
    {
        const bool enabled = Button_GetCheck(tofCheck_) == BST_CHECKED;
        const int hz = get_hz();
        post_command(enabled ? L"Enable TOF" : L"Disable TOF", [this, enabled, hz]() { dog_.request_tof_stream(enabled, hz, kControlTransport); });
    }

    void apply_sensor_hz()
    {
        const int hz = get_hz();
        const bool imuOn = Button_GetCheck(imuCheck_) == BST_CHECKED;
        const bool tofOn = Button_GetCheck(tofCheck_) == BST_CHECKED;
        if (!imuOn && !tofOn) {
            append_log(L"Sensor Hz updated locally; streams are off");
            return;
        }
        post_command(L"Apply sensor Hz", [this, hz, imuOn, tofOn]() {
            if (imuOn) {
                dog_.request_imu_stream(true, hz, kControlTransport);
            }
            if (tofOn) {
                dog_.request_tof_stream(true, hz, kControlTransport);
            }
        });
    }

    void post_command(std::wstring label, std::function<void()> fn)
    {
        if (!connected_.load()) {
            append_log(L"Not connected");
            return;
        }
        append_log(label + L": queued");
        worker_->post(std::move(label), std::move(fn));
    }

#ifdef AIDOG_USER_CONTROL_WS
    void start_ws_host()
    {
        if (wsHost_) {
            append_log(L"WS host already started");
            return;
        }
        auto bind = to_utf8(clamp_edit_text(prefixEdit_));
        int port = 8766;
        try {
            port = std::stoi(clamp_edit_text(deviceCombo_));
        } catch (...) {
            port = 8766;
        }
        append_log(L"Start WS Host: queued");
        worker_->post(L"Start WS Host", [this, bind, port]() {
            wsHost_ = std::make_unique<aidog::WebSocketHost>(bind, port, &dog_);
            wsHost_->set_connection_callback([this](bool connected) {
                PostMessageW(hwnd_, WM_APP_WS_STATE, connected ? TRUE : FALSE, 0);
            });
            wsHost_->set_imu_callback([this](const aidog::ImuData& imu) {
                {
                    std::lock_guard<std::mutex> lock(sensorMutex_);
                    sensors_.imu = imu;
                    const DWORD tick = GetTickCount();
                    if (imu.yawDeg.has_value()) {
                        add_plot_point(0, *imu.yawDeg, tick);
                    }
                    if (imu.pitchDeg.has_value()) {
                        add_plot_point(1, *imu.pitchDeg, tick);
                    }
                    if (imu.rollDeg.has_value()) {
                        add_plot_point(2, *imu.rollDeg, tick);
                    }
                }
                PostMessageW(hwnd_, WM_APP_SENSOR, 0, 0);
            });
            wsHost_->set_tof_callback([this](const aidog::TofData& tof) {
                {
                    std::lock_guard<std::mutex> lock(sensorMutex_);
                    sensors_.tof = tof;
                    const DWORD tick = GetTickCount();
                    auto front = json_number(tof.raw, {"front_mm", "front", "tof_front", "distance_mm"});
                    auto oblique = json_number(tof.raw, {"oblique_mm", "oblique", "tof_oblique", "side_mm"});
                    if (front.has_value()) {
                        add_plot_point(3, *front, tick);
                    }
                    if (oblique.has_value()) {
                        add_plot_point(4, *oblique, tick);
                    }
                }
                PostMessageW(hwnd_, WM_APP_SENSOR, 0, 0);
            });
            wsHost_->start();
            connected_.store(false);
        });
        SetWindowTextW(statusLabel_, L"WS Status: waiting");
    }

    void wait_ws_robot()
    {
        if (!wsHost_) {
            append_log(L"Start WS Host first");
            return;
        }
        append_log(L"Wait Robot: queued");
        worker_->post(L"Wait Robot", [this]() {
            if (!wsHost_->wait_robot_connected(120.0)) {
                throw std::runtime_error("timeout waiting for robot");
            }
            connected_.store(true);
        });
    }
#endif

    void on_task_done(TaskResult* result)
    {
        if (!result) {
            return;
        }
        std::unique_ptr<TaskResult> holder(result);
        append_log(result->label + L": " + result->message);
        refresh_devices_combo();
#ifdef AIDOG_USER_CONTROL_WS
        SetWindowTextW(statusLabel_, connected_.load() ? L"WS Status: connected" : (wsHost_ ? L"WS Status: waiting" : L"WS Status: stopped"));
#else
        SetWindowTextW(statusLabel_, connected_.load() ? L"Status: connected" : L"Status: disconnected");
#endif
    }

    void refresh_devices_combo()
    {
#ifdef AIDOG_USER_CONTROL_WS
        return;
#else
        ComboBox_ResetContent(deviceCombo_);
        std::lock_guard<std::mutex> lock(deviceMutex_);
        for (const auto& device : devices_) {
            auto text = to_wide(device.name + " [" + device.address + "]");
            ComboBox_AddString(deviceCombo_, text.c_str());
        }
        if (!devices_.empty()) {
            ComboBox_SetCurSel(deviceCombo_, 0);
        }
#endif
    }

    void refresh_sensor_text()
    {
        SensorState snapshot;
        {
            std::lock_guard<std::mutex> lock(sensorMutex_);
            snapshot = sensors_;
        }

        std::wstring text;
        if (snapshot.imu.has_value()) {
            const auto& imu = *snapshot.imu;
            text += L"IMU\r\n";
            text += L"  yaw: " + number_text(imu.yawDeg.value_or(0.0)) + L"\r\n";
            text += L"  pitch: " + number_text(imu.pitchDeg.value_or(0.0)) + L"\r\n";
            text += L"  roll: " + number_text(imu.rollDeg.value_or(0.0)) + L"\r\n\r\n";
        }
        if (snapshot.tof.has_value()) {
            text += L"TOF\r\n";
            text += to_wide(snapshot.tof->raw.dump(2)) + L"\r\n";
        }
        if (text.empty()) {
            text = L"No data";
        }
        SetWindowTextW(sensorText_, text.c_str());
        const int earPos = static_cast<int>(SendMessageW(earSlider_, TBM_GETPOS, 0, 0));
        SetWindowTextW(earValueLabel_, (std::to_wstring(earPos) + L"%").c_str());
    }

    int get_hz() const
    {
        int hz = 20;
        try {
            hz = std::stoi(clamp_edit_text(hzEdit_));
        } catch (...) {
            hz = 20;
        }
        return std::clamp(hz, 1, 100);
    }

    void append_log(const std::wstring& message)
    {
        logLines_.push_back(L"[" + now_text() + L"] " + message);
        while (logLines_.size() > kMaxLogLines) {
            logLines_.pop_front();
        }
        std::wstring all;
        for (const auto& line : logLines_) {
            all += line + L"\r\n";
        }
        SetWindowTextW(logEdit_, all.c_str());
        SendMessageW(logEdit_, EM_SETSEL, static_cast<WPARAM>(all.size()), static_cast<LPARAM>(all.size()));
        SendMessageW(logEdit_, EM_SCROLLCARET, 0, 0);
    }

    void cleanup_robot()
    {
#ifdef AIDOG_USER_CONTROL_WS
        try {
            dog_.request_imu_stream(false, 20, kControlTransport);
        } catch (...) {
        }
        try {
            dog_.request_tof_stream(false, 20, kControlTransport);
        } catch (...) {
        }
        try {
            dog_.stop_movement(kControlTransport);
        } catch (...) {
        }
        activeMovement_.reset();
        if (wsHost_) {
            wsHost_->stop();
            wsHost_.reset();
        }
#else
        try {
            dog_.request_imu_stream(false);
        } catch (...) {
        }
        try {
            dog_.request_tof_stream(false);
        } catch (...) {
        }
        try {
            dog_.stop_movement();
        } catch (...) {
        }
        activeMovement_.reset();
        try {
            dog_.disconnect();
        } catch (...) {
        }
#endif
    }

    void on_close()
    {
        KillTimer(hwnd_, kRefreshTimer);
        KillTimer(hwnd_, kPlotTimer);
        if (worker_) {
            worker_->post(L"Shutdown", [this]() {
                cleanup_robot();
                dog_.shutdown();
            });
            worker_->stop();
        }
    }

    HINSTANCE instance_ = nullptr;
    HWND hwnd_ = nullptr;
    HWND prefixEdit_ = nullptr;
    HWND scanButton_ = nullptr;
    HWND deviceCombo_ = nullptr;
    HWND connectButton_ = nullptr;
    HWND disconnectButton_ = nullptr;
    HWND statusLabel_ = nullptr;
    HWND tabs_ = nullptr;
    HWND pages_[5]{};
    HWND logEdit_ = nullptr;
    HWND motionTitle_ = nullptr;
    HWND actionTitle_ = nullptr;
    HWND earTitle_ = nullptr;
    HWND earActionTitle_ = nullptr;
    HWND expressionTitle_ = nullptr;
    HWND audioTitle_ = nullptr;
    HWND volumeTitle_ = nullptr;
    HWND sensorTitle_ = nullptr;
    HWND joystick_ = nullptr;
    HWND stepButton_ = nullptr;
    HWND stopButton_ = nullptr;
    HWND earSlider_ = nullptr;
    HWND earValueLabel_ = nullptr;
    HWND earSendButton_ = nullptr;
    HWND imuCheck_ = nullptr;
    HWND tofCheck_ = nullptr;
    HWND hzEdit_ = nullptr;
    HWND applyHzButton_ = nullptr;
    HWND clearPlotsButton_ = nullptr;
    HWND plotWindows_[5]{};
    HWND sensorText_ = nullptr;

    std::vector<HWND> actionButtons_;
    std::vector<HWND> earButtons_;
    std::vector<HWND> expressionButtons_;
    std::vector<HWND> toneButtons_;
    std::vector<HWND> volumeButtons_;

    std::unordered_map<int, aidog::Action> actionMap_;
    std::unordered_map<int, aidog::EarAction> earMap_;
    std::unordered_map<int, aidog::ExpressionAction> expressionMap_;
    std::unordered_map<int, aidog::Tone> toneMap_;

    aidog::AiDog dog_;
#ifdef AIDOG_USER_CONTROL_WS
    std::unique_ptr<aidog::WebSocketHost> wsHost_;
#endif
    std::unique_ptr<Worker> worker_;
    std::atomic_bool connected_{false};
    std::mutex deviceMutex_;
    std::vector<DeviceEntry> devices_;
    std::mutex sensorMutex_;
    SensorState sensors_;
    PlotSeries plotSeries_[5];
    std::deque<std::wstring> logLines_;
    std::optional<aidog::Movement> activeMovement_;
    std::optional<int> lastEarPercent_;
    std::optional<int> pendingEarPercent_;
    DWORD lastEarPercentTick_ = 0;
    int joystickKnobX_ = kJoystickSize / 2;
    int joystickKnobY_ = kJoystickSize / 2;
};

} // namespace

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int)
{
    UserControlApp app(instance);
    return app.run();
}
