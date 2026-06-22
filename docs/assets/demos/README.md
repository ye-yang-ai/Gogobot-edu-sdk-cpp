# Demo Video Assets

This directory stores demo video references used by the C++ SDK documentation.

## Current Videos

The current public videos are hosted through GitHub attachment URLs and documented in [Demo Videos](../../demo_videos.md). The behavior references come from the Python SDK demos; the C++ SDK currently validates the same primitives through examples such as `aidog_ble_choreography`, `aidog_ws_choreography`, `aidog_custom_action`, and `aidog_safe_pose_adjust`.

| Video | C++ related examples | Description |
| --- | --- | --- |
| Demo 1 | `aidog_ble_choreography`, `aidog_ws_choreography` | Full performance routine with ears, expressions, movement, angle turning, slow crouch, stretch, and tail wag. |
| Demo 2 | `aidog_custom_action`, `aidog_safe_pose_adjust` | Custom sniff-like action built from smooth body/foot adjustment, expression, and audio. |

## Asset Guidelines

- Prefer real robot footage.
- Keep file names lowercase and descriptive.
- Use MP4 for long demos and GIF/WebP only for short README previews.
- Keep large video assets in this directory or link to an official hosted copy from the docs.
- When adding a new video, update both this file and `docs/demo_videos.md`.
