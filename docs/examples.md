# Examples

The full C++ example index is maintained in [`../examples/README.md`](../examples/README.md).

Recommended first commands:

```powershell
.\build\Release\aidog_ble_scan_and_connect.exe --prefix Gogobot
.\build\Release\aidog_ble_connect_by_address.exe --address AA:BB:CC:DD:EE:FF
.\build\Release\aidog_ble_imu_read.exe --address AA:BB:CC:DD:EE:FF --hz 20 --seconds 10
.\build\Release\aidog_ble_basic_actions.exe --address AA:BB:CC:DD:EE:FF --action sit_down --yes
```

For WebSocket examples, configure the robot Dev-PC IP first:

```powershell
.\build\Release\aidog_set_dev_pc_ws_ip_ble.exe --address AA:BB:CC:DD:EE:FF 192.168.11.101
.\build\Release\aidog_ws_connection_test.exe --timeout 60 --no-keep-alive
```
