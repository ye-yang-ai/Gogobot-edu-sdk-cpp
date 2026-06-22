# Robot Action Parameter Types

Some firmware actions accept one optional parameter. The C++ SDK exposes this through `aidog::ActionOptions`.

```cpp
aidog::ActionOptions options;
options.timeoutS = 20.0;
```

Only one of `duration`, `count`, or `angle` is meaningful for a given action. If the field does not match the selected action type, the SDK sends the action without that parameter.

## Time Based Actions

Use `ActionOptions::duration` for actions that need a duration in seconds.

```cpp
aidog::ActionOptions options;
options.duration = 3;
dog.perform_action(aidog::Action::ForwardInteraction, options);
```

Common time-based action examples:

| Action | Typical alias |
| --- | --- |
| `ForwardInteraction` | `forward_interaction` |
| `BackInteraction` | `back_interaction` |
| `LeftInteraction` | `left_interaction` |
| `RightInteraction` | `right_interaction` |
| `SlowUp` | `slow_up` |
| `SlowDown` | `slow_down` |

## Count Based Actions

Use `ActionOptions::count` for actions that repeat.

```cpp
aidog::ActionOptions options;
options.count = 2;
dog.perform_action(aidog::Action::ShakeHand, options);
```

Common count-based action examples:

| Action | Typical alias |
| --- | --- |
| `ShakeHand` | `shake_hand` |
| `Nod` | `nod` |
| `ShakeHead` | `shake_head` |
| `WagTail` | `wag_tail` |

## Angle Based Actions

Use `ActionOptions::angle` for actions that turn by an angle.

```cpp
aidog::ActionOptions options;
options.angle = 90;
dog.perform_action(aidog::Action::RightAngleInteraction, options);
```

Common angle-based action examples:

| Action | Typical alias |
| --- | --- |
| `LeftAngleInteraction` | `left_angle_interaction` |
| `RightAngleInteraction` | `right_angle_interaction` |

## Normal Actions

Most actions do not need a parameter:

```cpp
dog.perform_action(aidog::Action::SitDown);
dog.perform_action("stand_up");
```

For command-line testing:

```powershell
.\build\Release\aidog_ble_basic_actions.exe --address AA:BB:CC:DD:EE:FF --action shake_hand --count 2 --yes
.\build\Release\aidog_ble_basic_actions.exe --address AA:BB:CC:DD:EE:FF --action right_angle_interaction --angle 90 --yes
```
