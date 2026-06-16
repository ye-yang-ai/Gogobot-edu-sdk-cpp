cmake_minimum_required(VERSION 3.21)

project(aidog_sdk_cpp VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

option(AIDOG_BUILD_EXAMPLES "Build aidog examples" ON)
option(AIDOG_BUILD_TESTS "Build aidog tests" ON)
option(AIDOG_ENABLE_WEBSOCKET "Build Dev PC WebSocket host" ON)
option(AIDOG_ENABLE_WINDOWS_BLE "Build Windows BLE transport" ${WIN32})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

FetchContent_Declare(
    nlohmann_json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(nlohmann_json)

if(AIDOG_ENABLE_WEBSOCKET)
    FetchContent_Declare(
        asio
        URL https://github.com/chriskohlhoff/asio/archive/refs/tags/asio-1-30-2.tar.gz
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_Declare(
        websocketpp
        URL https://github.com/zaphoyd/websocketpp/archive/refs/tags/0.8.2.tar.gz
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(asio websocketpp)
endif()

add_library(aidog_sdk
    src/actions.cpp
    src/aidog.cpp
    src/protocol.cpp
    src/sensor_parser.cpp
)

add_library(aidog::sdk ALIAS aidog_sdk)

target_include_directories(aidog_sdk
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(aidog_sdk PUBLIC nlohmann_json::nlohmann_json)

target_compile_definitions(aidog_sdk
    PUBLIC
        AIDOG_ENABLE_WEBSOCKET=$<BOOL:${AIDOG_ENABLE_WEBSOCKET}>
        AIDOG_ENABLE_WINDOWS_BLE=$<BOOL:${AIDOG_ENABLE_WINDOWS_BLE}>
)

if(AIDOG_ENABLE_WEBSOCKET)
    target_sources(aidog_sdk PRIVATE src/websocket_host.cpp)
    target_include_directories(aidog_sdk PUBLIC
        ${asio_SOURCE_DIR}/asio/include
        ${websocketpp_SOURCE_DIR}
    )
    target_compile_definitions(aidog_sdk PUBLIC ASIO_STANDALONE _WEBSOCKETPP_CPP11_STL_)
else()
    target_sources(aidog_sdk PRIVATE src/websocket_host_stub.cpp)
endif()

if(AIDOG_ENABLE_WINDOWS_BLE)
    if(NOT WIN32)
        message(FATAL_ERROR "AIDOG_ENABLE_WINDOWS_BLE requires Windows")
    endif()
    target_sources(aidog_sdk PRIVATE src/ble_transport_win.cpp)
    target_link_libraries(aidog_sdk PUBLIC windowsapp)
else()
    target_sources(aidog_sdk PRIVATE src/ble_transport_stub.cpp)
endif()

if(MSVC)
    target_compile_options(aidog_sdk PRIVATE /W4 /permissive-)
else()
    target_compile_options(aidog_sdk PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(AIDOG_BUILD_EXAMPLES)
    if(AIDOG_ENABLE_WINDOWS_BLE)
        add_executable(aidog_ble_scan examples/ble_scan.cpp)
        target_link_libraries(aidog_ble_scan PRIVATE aidog::sdk)

        add_executable(aidog_ble_connect_test examples/ble_connect_test.cpp)
        target_link_libraries(aidog_ble_connect_test PRIVATE aidog::sdk)

        add_executable(aidog_ble_action_list examples/ble_action_list.cpp)
        target_link_libraries(aidog_ble_action_list PRIVATE aidog::sdk)

        add_executable(aidog_ble_imu_read examples/ble_imu_read.cpp)
        target_link_libraries(aidog_ble_imu_read PRIVATE aidog::sdk)

        add_executable(aidog_ble_tof_read examples/ble_tof_read.cpp)
        target_link_libraries(aidog_ble_tof_read PRIVATE aidog::sdk)

        add_executable(aidog_ble_ears_expressions_audio examples/ble_ears_expressions_audio.cpp)
        target_link_libraries(aidog_ble_ears_expressions_audio PRIVATE aidog::sdk)

        add_executable(aidog_ble_set_volume examples/ble_set_volume.cpp)
        target_link_libraries(aidog_ble_set_volume PRIVATE aidog::sdk)

        add_executable(aidog_ble_basic_actions examples/ble_basic_actions.cpp)
        target_link_libraries(aidog_ble_basic_actions PRIVATE aidog::sdk)
    endif()

    if(AIDOG_ENABLE_WEBSOCKET)
        add_executable(aidog_ws_basic_actions examples/ws_basic_actions.cpp)
        target_link_libraries(aidog_ws_basic_actions PRIVATE aidog::sdk)

        add_executable(aidog_ws_imu_read examples/ws_imu_read.cpp)
        target_link_libraries(aidog_ws_imu_read PRIVATE aidog::sdk)
    endif()
endif()

if(AIDOG_BUILD_TESTS)
    enable_testing()
    FetchContent_Declare(
        doctest
        URL https://github.com/doctest/doctest/archive/refs/tags/v2.4.11.tar.gz
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(doctest)

    add_executable(aidog_tests
        tests/test_main.cpp
        tests/test_actions.cpp
        tests/test_protocol.cpp
        tests/test_sensor_parser.cpp
    )
    target_link_libraries(aidog_tests PRIVATE aidog::sdk doctest::doctest)
    add_test(NAME aidog_tests COMMAND aidog_tests)
endif()
