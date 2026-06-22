# Demo Videos

These videos and notes come from the Python SDK demos and can be used as reference material for C++ SDK feature validation.

## GitHub Playback

GitHub does not reliably render repository-relative MP4 files from an HTML `<video>` tag in Markdown. To show an inline player on GitHub:

1. Open an issue, pull request, or discussion comment on GitHub.
2. Drag the `.mp4` file into the comment box and wait for GitHub to upload it.
3. Copy the generated `https://github.com/user-attachments/assets/...` URL.
4. Paste that URL on its own line in the "GitHub player URL" slot below.

## Demo 1: Performance Routine

- Python script: `../../aidog_sdk/demo/demo_1_performance.py`
- C++ related examples: `aidog_ble_choreography`, `aidog_ws_choreography`
- Shows: ears, expressions, actions, forward movement, angle turning, slow crouch, sleepy expression, stretch, and tail wag.

Performance sequence:

1. Connect to the robot and disable special detection.
2. Raise the ears and show a yawn expression.
3. Shake hand three times.
4. Nod twice.
5. Move forward while playing audio.
6. Jump.
7. Turn right by angle command.
8. Enter a slow crouch and switch to a sleepy expression.
9. Stretch.
10. Wag tail, stop audio, reset, and re-enable special detection.

GitHub player URL: https://github.com/user-attachments/assets/dc8c8bea-d5ef-4b3a-bc0b-4af86d036a9c

## Demo 2: Custom Action

- Python script: `../../aidog_sdk/demo/demo_2_custom_action.py`
- C++ related examples: `aidog_custom_action`, `aidog_safe_pose_adjust`
- Shows: forward movement followed by a custom sniff-like action built from smooth body/foot adjustment, expression, and audio.

Performance sequence:

1. Connect to the robot.
2. Move forward briefly.
3. Show a happy expression and stop movement.
4. Disable special detection and TOF avoidance for custom adjustment.
5. Request basic mode and move to a baseline pose.
6. Shift feet and lift front legs.
7. Play audio and show an eating expression.
8. Restore pose and return to basic mode.

GitHub player URL: https://github.com/user-attachments/assets/7a0e693a-2233-4a02-b598-6bd877eeb91f

## Demo 3: Reactive Companion Patrol

- Python script: `../../aidog_sdk/demo/demo_3_reactive_companion.py`
- C++ related examples: `aidog_ble_tof_read`, `aidog_ws_tof_lan_read`, movement examples
- Shows: stretch wake-up, TOF-based patrol, obstacle detection, short retreat, and reaction feedback.

The C++ SDK has the sensor and movement primitives needed for this demo, but there is not yet a single C++ demo executable that fully reproduces the Python script.

## Notes

- Treat Python demo scripts as behavior references.
- Use the C++ examples to validate each primitive before composing a long routine.
- Keep custom adjustment and patrol demos supervised.
