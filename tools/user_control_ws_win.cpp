/*
Purpose:
    AI-Dog Windows WebSocket user control GUI.
Build:
    cd C:\C_project_3.1\arbitrarion10\aidog_sdk_cpp
    "C:\Program Files\CMake\bin\cmake.exe" --build build --config Release --target aidog_user_control_ws
Configure robot Dev-PC WebSocket IP over BLE:
    cd C:\C_project_3.1\arbitrarion10\aidog_sdk_cpp
    .\build\Release\aidog_set_dev_pc_ws_ip_ble.exe --address 12:0A:AB:16:3A:04 192.168.11.108
Run:
    cd C:\C_project_3.1\arbitrarion10\aidog_sdk_cpp
    .\build\Release\aidog_user_control_ws.exe
Flow:
    Click Start WS Host, wait for WS Status: connected, then use the same controls as the BLE GUI.
*/

#define AIDOG_USER_CONTROL_WS 1
#include "user_control_ble_win.cpp"
