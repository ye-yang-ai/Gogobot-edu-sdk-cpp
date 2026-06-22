# Troubleshooting

## BLE Scan Finds No Device

- Confirm the robot is powered on.
- Keep the robot close to the Windows PC.
- Check that Windows Bluetooth is enabled.
- Try the default prefix first:

```powershell
.\build\Release\aidog_ble_scan_and_connect.exe --prefix Gogobot
```

## Direct Address Connect Fails

- Use the Windows BLE address shown by scan, not only the address embedded in the device name.
- Close other BLE scripts and GUI tools before reconnecting.
- Run a scan once before direct address connection so the WinRT BLE cache is warm.

```powershell
.\build\Release\aidog_ble_connect_by_address.exe --address 12:0A:AB:16:3A:04
```

## Action Does Not Finish

- Some firmware builds may not send `interaction_task_status` reliably.
- Increase timeout for long actions:

```powershell
.\build\Release\aidog_ble_basic_actions.exe --address 12:0A:AB:16:3A:04 --action shake_hand --timeout 30 --yes
```

- For WebSocket actions, allow enough connection time:

```powershell
.\build\Release\aidog_ws_basic_actions.exe --action sit_down --connect-timeout 120 --yes
```

## IMU / TOF Has No Data

- Confirm the example requested the stream with the expected `--hz`.
- Try BLE first, then WebSocket.
- Keep the process running for several seconds because some firmware builds start streams slowly.

```powershell
.\build\Release\aidog_ble_imu_read.exe --address 12:0A:AB:16:3A:04 --hz 20 --seconds 10
.\build\Release\aidog_ws_imu_lan_read.exe --hz 20 --seconds 10 --connect-timeout 120
```

## WebSocket Host Does Not Receive Data

- Confirm PC and robot are on the same LAN.
- Write the current PC IP to the robot through BLE:

```powershell
.\build\Release\aidog_set_dev_pc_ws_ip_ble.exe --address 12:0A:AB:16:3A:04 192.168.11.101
```

- Check that only one WebSocket host is running on port `8766`.
- If the robot does not reconnect after many runs, restart the robot and capture firmware debug logs.

## Audio Device Error

List Windows audio devices:

```powershell
.\build\Release\aidog_ws_bidirectional_pcm_host.exe --list-devices
```

Then pass explicit indices:

```powershell
.\build\Release\aidog_ws_bidirectional_pcm_host.exe --connect-timeout 120 --input-device 0 --output-device 0
```

If PC-to-robot audio has traffic but no sound, confirm the firmware build includes Dev-PC PCM playback support.
