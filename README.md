# Gogobot EDU SDK for C++

Windows-first C++ SDK for Changba AI-Dog / Gogobot EDU robot education and developer use.

## Status

This SDK is being built to match the Python `aidog_sdk` feature set:

- Windows BLE control through `ae01/ae02/ae03/ae04/ae10`.
- Dev PC WebSocket host control.
- Action, movement, ear, expression, tone, volume, IMU/TOF, EDU mode, and robot adjustment APIs.
- Unit-testable protocol and sensor parsing logic.

## Build

Use a Windows Developer PowerShell or Developer Command Prompt with MSVC and CMake available.

```powershell
cd C:\C_project_3.1\arbitrarion10\aidog_sdk_cpp
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

WebSocket dependencies are fetched by CMake. To only run protocol tests, disable WebSocket:

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DAIDOG_ENABLE_WEBSOCKET=OFF
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

If `cmake` is not found after installation, open a new terminal or use the full path:

```powershell
& "C:\Program Files\CMake\bin\cmake.exe" --version
```

Without Visual Studio/MSVC, pure protocol tests can be built with MinGW by disabling BLE and WebSocket:

```powershell
& "C:\Program Files\CMake\bin\cmake.exe" -S . -B build-mingw -G "MinGW Makefiles" -DAIDOG_ENABLE_WEBSOCKET=OFF -DAIDOG_ENABLE_WINDOWS_BLE=OFF -DAIDOG_BUILD_EXAMPLES=OFF
& "C:\Program Files\CMake\bin\cmake.exe" --build build-mingw
& "C:\Program Files\CMake\bin\ctest.exe" --test-dir build-mingw --output-on-failure
```

## Examples

```powershell
.\build\Release\aidog_ble_scan.exe
.\build\Release\aidog_ble_connect_test.exe
.\build\Release\aidog_ble_action_list.exe
.\build\Release\aidog_ble_imu_read.exe --hz 20 --seconds 20
.\build\Release\aidog_ble_tof_read.exe --hz 20 --seconds 20
.\build\Release\aidog_ble_set_volume.exe --volume 3
.\build\Release\aidog_ble_ears_expressions_audio.exe --yes
.\build\Release\aidog_ble_basic_actions.exe
.\build\Release\aidog_ws_basic_actions.exe
.\build\Release\aidog_ws_imu_read.exe
```

BLE examples accept `--address <BluetoothAddress>` to skip scanning and
`--prefix <name>` to change the default `Gogobot` scan prefix.

WebSocket examples require firmware with `DEV_PC_AUDIO_WS_ENABLE=1` and `DEV_PC_AUDIO_WS_URL` pointing to the PC.
