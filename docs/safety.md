# Safety Guide

## Before Running Examples

- Place the robot on a flat open floor.
- Keep hands, cables, and fragile objects away from the legs.
- Start with connection and sensor examples before running movement.
- Use short durations first, such as `--duration 1`.
- Keep the terminal or GUI visible so you can stop the current program quickly.

## Risk Levels

| Level | Examples | Notes |
| --- | --- | --- |
| Low | Scan, connect, IMU, TOF, audio volume | No body movement expected |
| Medium | Basic actions, ears, expressions, short movement | Robot may move or shift weight |
| High | Timed movement, choreography, pose / foot / joint adjustment | Requires close supervision |

## Recommended Safe Sequence for Choreography

1. Verify BLE connection.
2. Read IMU or TOF for at least 5 seconds.
3. Run `sit_down`.
4. Run `stand_up`.
5. Run one short movement command with `--duration 1`.
6. Only then try choreography or robot adjustment examples.

## Emergency Stop Behavior

For command-line examples:

- Press `Ctrl+C` to stop the host program.
- Run a stop or reset command if the robot remains active.

For GUI tools:

- Use the stop movement button.
- Disconnect the current transport if the command path appears stuck.

If the robot is physically unsafe, power it off first and debug the software path afterward.
