# BLE Connection Guide

The C++ SDK uses Windows BLE to scan, connect, write robot commands, and subscribe to robot notifications.

## Device Discovery

Default name prefix:

```powershell
.\build\Release\aidog_ble_scan_and_connect.exe --prefix Gogobot
```

C++ usage:

```cpp
aidog::AiDog dog;
auto devices = dog.scan("Gogobot");
```

Returned records are `aidog::DeviceInfo` values with name and address fields.

## Connection

Direct address connection:

```powershell
.\build\Release\aidog_ble_connect_by_address.exe --address AA:BB:CC:DD:EE:FF
```

C++ usage:

```cpp
aidog::ConnectOptions options;
options.address = "AA:BB:CC:DD:EE:FF";
dog.connect(options);
```

Prefix-based connection:

```cpp
aidog::ConnectOptions options;
options.namePrefix = "Gogobot";
dog.connect(options);
```

## Notifications

The SDK subscribes to:

- `ae02`: device-to-host status notifications.
- `ae04`: sensor stream notifications, typically IMU / TOF JSON.

## Common Issues

- If scanning returns no devices, check robot power, Bluetooth range, Windows Bluetooth state, and the device name prefix.
- On Windows, direct address connection can still require the device to be visible to the BLE cache.
- If writes fail, reconnect the robot and keep only one host connected at a time.
- If another script or GUI is connected over BLE, close it before starting a new BLE example.

See [Troubleshooting](troubleshooting.md) for more details.
