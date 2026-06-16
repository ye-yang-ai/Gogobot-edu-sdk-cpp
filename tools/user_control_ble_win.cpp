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
constexpr UINT WM_APP_TASK_DONE = WM_APP + 1;
constexpr UINT WM_APP_SENSOR = WM_APP + 2;
constexpr UINT_PTR kRefreshTimer = 1;
constexpr int kMaxLogLines = 300;

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

        hwnd_ = CreateWindowExW(0, kWindowClass, L"AI-Dog BLE User Control", WS_OVERLAPPEDWINDOW,
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
            refresh_sensor_text();
            return 0;
        case WM_APP_TASK_DONE:
            on_task_done(reinterpret_cast<TaskResult*>(lparam));
            return 0;
        case WM_APP_SENSOR:
            refresh_sensor_text();
            return 0;
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

    void on_create()
    {
        worker_ = std::make_unique<Worker>(hwnd_);
        dog_.add_imu_listener([this](const aidog::ImuData& imu) {
            {
                std::lock_guard<std::mutex> lock(sensorMutex_);
                sensors_.imu = imu;
            }
            PostMessageW(hwnd_, WM_APP_SENSOR, 0, 0);
        });
        dog_.add_tof_listener([this](const aidog::TofData& tof) {
            {
                std::lock_guard<std::mutex> lock(sensorMutex_);
                sensors_.tof = tof;
            }
            PostMessageW(hwnd_, WM_APP_SENSOR, 0, 0);
        });

        create_top_bar();
        create_tabs();
        create_log();
        SetTimer(hwnd_, kRefreshTimer, 500, nullptr);
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
        prefixEdit_ = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"Gogobot", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
            0, 0, 120, 24, hwnd_, reinterpret_cast<HMENU>(IdPrefix), instance_, nullptr);
        scanButton_ = make_button(hwnd_, L"Scan", IdScan);
        deviceCombo_ = CreateWindowExW(0, WC_COMBOBOXW, L"", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
            0, 0, 320, 200, hwnd_, reinterpret_cast<HMENU>(IdDeviceCombo), instance_, nullptr);
        connectButton_ = make_button(hwnd_, L"Connect", IdConnect);
        disconnectButton_ = make_button(hwnd_, L"Disconnect", IdDisconnect);
        statusLabel_ = make_label(hwnd_, L"Status: disconnected");
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
        const ButtonDef moves[] = {
            {L"Forward", 3001},
            {L"Back", 3002},
            {L"Left", 3003},
            {L"Right", 3004},
            {L"Step", 3005},
            {L"Stop", 3006},
        };
        create_buttons(pages_[0], moveButtons_, moves, 6);

        const std::vector<std::pair<const wchar_t*, aidog::Action>> actions{
            {L"Sit", aidog::Action::SitDown},
            {L"Stand", aidog::Action::StandUp},
            {L"Shake Hand", aidog::Action::ShakeHand},
            {L"Nod", aidog::Action::Nod},
            {L"Shake Head", aidog::Action::ShakeHead},
            {L"Stretch", aidog::Action::Stretch},
            {L"Dance", aidog::Action::Dance},
            {L"Celebrate", aidog::Action::Celebrate},
            {L"Sniff", aidog::Action::Sniff},
            {L"Stop Action", aidog::Action::StopInteraction},
        };
        for (std::size_t i = 0; i < actions.size(); ++i) {
            const int id = 4000 + static_cast<int>(i);
            actionMap_[id] = actions[i].second;
            actionButtons_.push_back(make_button(pages_[0], actions[i].first, id));
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

        const std::vector<std::pair<const wchar_t*, aidog::EarAction>> ears{
            {L"Stand", aidog::EarAction::EarStand},
            {L"Left", aidog::EarAction::EarStandLeft},
            {L"Right", aidog::EarAction::EarStandRight},
            {L"Both", aidog::EarAction::EarStandLeftAndRight},
            {L"Shake", aidog::EarAction::EarShakeSynForBle},
            {L"Breathe", aidog::EarAction::EarBreathe},
            {L"Down", aidog::EarAction::EarDown},
            {L"Random", aidog::EarAction::EarFlickRandom},
        };
        for (std::size_t i = 0; i < ears.size(); ++i) {
            const int id = 5200 + static_cast<int>(i);
            earMap_[id] = ears[i].second;
            earButtons_.push_back(make_button(pages_[1], ears[i].first, id));
        }
    }

    void create_expression_page()
    {
        expressionTitle_ = make_label(pages_[2], L"Expression test");
        const std::vector<std::pair<const wchar_t*, aidog::ExpressionAction>> expressions{
            {L"Happy", aidog::ExpressionAction::Happy01},
            {L"Smile", aidog::ExpressionAction::Smile01},
            {L"Love", aidog::ExpressionAction::Love01},
            {L"Angry", aidog::ExpressionAction::Anger01},
            {L"Sad", aidog::ExpressionAction::Sad01},
            {L"Doubt", aidog::ExpressionAction::Doubt01},
            {L"Sleepy", aidog::ExpressionAction::Sleepy},
            {L"Wink", aidog::ExpressionAction::WinkNormal},
            {L"Music", aidog::ExpressionAction::Music},
            {L"Eat Snack", aidog::ExpressionAction::EatSnack},
            {L"Alert", aidog::ExpressionAction::Alert},
            {L"Yawn", aidog::ExpressionAction::Yawn},
        };
        for (std::size_t i = 0; i < expressions.size(); ++i) {
            const int id = 6000 + static_cast<int>(i);
            expressionMap_[id] = expressions[i].second;
            expressionButtons_.push_back(make_button(pages_[2], expressions[i].first, id));
        }
    }

    void create_audio_page()
    {
        audioTitle_ = make_label(pages_[3], L"Audio test");
        volumeTitle_ = make_label(pages_[3], L"Volume level");
        const std::vector<std::pair<const wchar_t*, aidog::Tone>> tones{
            {L"Stop", aidog::Tone::Stop},
            {L"Jeez", aidog::Tone::Jeez},
            {L"Eating", aidog::Tone::Eating},
            {L"Curious", aidog::Tone::Curious},
            {L"Sleepy", aidog::Tone::Sleepy},
            {L"Sad", aidog::Tone::Sad},
            {L"Angry", aidog::Tone::Angry},
            {L"Agree", aidog::Tone::Agree},
            {L"Alert", aidog::Tone::Alert},
            {L"Beat1", aidog::Tone::Beat1},
            {L"Beat2", aidog::Tone::Beat2},
            {L"Beat3", aidog::Tone::Beat3},
        };
        for (std::size_t i = 0; i < tones.size(); ++i) {
            const int id = 7000 + static_cast<int>(i);
            toneMap_[id] = tones[i].second;
            toneButtons_.push_back(make_button(pages_[3], tones[i].first, id));
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

        MoveWindow(prefixEdit_, pad, pad, 140, 24, TRUE);
        MoveWindow(scanButton_, 156, pad, 70, 26, TRUE);
        MoveWindow(deviceCombo_, 234, pad, 360, 300, TRUE);
        MoveWindow(connectButton_, 602, pad, 76, 26, TRUE);
        MoveWindow(disconnectButton_, 686, pad, 90, 26, TRUE);
        MoveWindow(statusLabel_, 786, pad + 4, std::max(100, width - 794), 24, TRUE);

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

    void layout_pages(int width, int)
    {
        MoveWindow(motionTitle_, 18, 18, 180, 24, TRUE);
        MoveWindow(actionTitle_, 390, 18, 180, 24, TRUE);
        for (std::size_t i = 0; i < moveButtons_.size(); ++i) {
            MoveWindow(moveButtons_[i], 18 + static_cast<int>(i % 2) * 118, 56 + static_cast<int>(i / 2) * 42, 110, 34, TRUE);
        }
        for (std::size_t i = 0; i < actionButtons_.size(); ++i) {
            MoveWindow(actionButtons_[i], 390 + static_cast<int>(i % 4) * 126, 56 + static_cast<int>(i / 4) * 42, 118, 34, TRUE);
        }

        MoveWindow(earTitle_, 18, 18, 180, 24, TRUE);
        MoveWindow(earSlider_, 18, 52, 360, 36, TRUE);
        MoveWindow(earValueLabel_, 390, 56, 60, 24, TRUE);
        MoveWindow(earSendButton_, 460, 50, 150, 30, TRUE);
        MoveWindow(earActionTitle_, 18, 104, 180, 24, TRUE);
        for (std::size_t i = 0; i < earButtons_.size(); ++i) {
            MoveWindow(earButtons_[i], 18 + static_cast<int>(i % 4) * 120, 138 + static_cast<int>(i / 4) * 42, 110, 34, TRUE);
        }

        MoveWindow(expressionTitle_, 18, 18, 180, 24, TRUE);
        for (std::size_t i = 0; i < expressionButtons_.size(); ++i) {
            MoveWindow(expressionButtons_[i], 18 + static_cast<int>(i % 5) * 120, 56 + static_cast<int>(i / 5) * 42, 112, 34, TRUE);
        }

        MoveWindow(audioTitle_, 18, 18, 180, 24, TRUE);
        for (std::size_t i = 0; i < toneButtons_.size(); ++i) {
            MoveWindow(toneButtons_[i], 18 + static_cast<int>(i % 5) * 120, 56 + static_cast<int>(i / 5) * 42, 112, 34, TRUE);
        }
        MoveWindow(volumeTitle_, 18, 168, 180, 24, TRUE);
        for (std::size_t i = 0; i < volumeButtons_.size(); ++i) {
            MoveWindow(volumeButtons_[i], 18 + static_cast<int>(i) * 92, 202, 82, 38, TRUE);
        }

        MoveWindow(sensorTitle_, 18, 18, 180, 24, TRUE);
        MoveWindow(imuCheck_, 18, 52, 120, 24, TRUE);
        MoveWindow(tofCheck_, 148, 52, 120, 24, TRUE);
        MoveWindow(hzEdit_, 280, 52, 60, 24, TRUE);
        MoveWindow(sensorText_, 18, 90, std::max(300, width - 36), 260, TRUE);
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
        } else if (id >= 3001 && id <= 3006) {
            handle_movement(id);
        } else if (auto it = actionMap_.find(id); it != actionMap_.end()) {
            post_command(L"Action", [this, action = it->second]() { dog_.send_interaction(action); });
        } else if (auto it = earMap_.find(id); it != earMap_.end()) {
            post_command(L"Ear action", [this, action = it->second]() { dog_.send_ear(action); });
        } else if (auto it = expressionMap_.find(id); it != expressionMap_.end()) {
            post_command(L"Expression", [this, expression = it->second]() { dog_.send_expression(expression); });
        } else if (auto it = toneMap_.find(id); it != toneMap_.end()) {
            post_command(L"Audio", [this, tone = it->second]() { dog_.send_audio(tone); });
        } else if (id >= 7100 && id <= 7104) {
            const int volume = id - 7100;
            post_command(L"Volume", [this, volume]() { dog_.set_volume(volume); });
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
    }

    void scan_devices()
    {
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
    }

    void connect_device()
    {
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
    }

    void disconnect_device()
    {
        worker_->post(L"Disconnect", [this]() {
            cleanup_robot();
            connected_.store(false);
        });
    }

    void handle_movement(int id)
    {
        if (id == 3006) {
            post_command(L"Stop movement", [this]() { dog_.stop_movement(); });
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
        post_command(L"Movement", [this, movement]() { dog_.start_movement(movement); });
    }

    void send_ear_percentage()
    {
        const int value = static_cast<int>(SendMessageW(earSlider_, TBM_GETPOS, 0, 0));
        SetWindowTextW(earValueLabel_, (std::to_wstring(value) + L"%").c_str());
        post_command(L"Ear percent", [this, value]() { dog_.send_ear_percentage(value); });
    }

    void toggle_imu()
    {
        const bool enabled = Button_GetCheck(imuCheck_) == BST_CHECKED;
        const int hz = get_hz();
        post_command(enabled ? L"Enable IMU" : L"Disable IMU", [this, enabled, hz]() { dog_.request_imu_stream(enabled, hz); });
    }

    void toggle_tof()
    {
        const bool enabled = Button_GetCheck(tofCheck_) == BST_CHECKED;
        const int hz = get_hz();
        post_command(enabled ? L"Enable TOF" : L"Disable TOF", [this, enabled, hz]() { dog_.request_tof_stream(enabled, hz); });
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

    void on_task_done(TaskResult* result)
    {
        if (!result) {
            return;
        }
        std::unique_ptr<TaskResult> holder(result);
        append_log(result->label + L": " + result->message);
        refresh_devices_combo();
        SetWindowTextW(statusLabel_, connected_.load() ? L"Status: connected" : L"Status: disconnected");
    }

    void refresh_devices_combo()
    {
        ComboBox_ResetContent(deviceCombo_);
        std::lock_guard<std::mutex> lock(deviceMutex_);
        for (const auto& device : devices_) {
            auto text = to_wide(device.name + " [" + device.address + "]");
            ComboBox_AddString(deviceCombo_, text.c_str());
        }
        if (!devices_.empty()) {
            ComboBox_SetCurSel(deviceCombo_, 0);
        }
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
        try {
            dog_.disconnect();
        } catch (...) {
        }
    }

    void on_close()
    {
        KillTimer(hwnd_, kRefreshTimer);
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
    HWND earSlider_ = nullptr;
    HWND earValueLabel_ = nullptr;
    HWND earSendButton_ = nullptr;
    HWND imuCheck_ = nullptr;
    HWND tofCheck_ = nullptr;
    HWND hzEdit_ = nullptr;
    HWND sensorText_ = nullptr;

    std::vector<HWND> moveButtons_;
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
    std::unique_ptr<Worker> worker_;
    std::atomic_bool connected_{false};
    std::mutex deviceMutex_;
    std::vector<DeviceEntry> devices_;
    std::mutex sensorMutex_;
    SensorState sensors_;
    std::deque<std::wstring> logLines_;
};

} // namespace

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int)
{
    UserControlApp app(instance);
    return app.run();
}
