# 15. settings/model/engine glue for bar

| | |
|---|---|
| Launched | 2026-10-09 07:29 UTC |
| Role | implementer |
| Model | Claude Haiku 5.5 |
| Agent id | `a55a57df33a4de093` |

## Instructions from the planner (verbatim)

````text
You are modifying three fragments of HawkTile, a clean-room tiling window manager written as a Windhawk mod (C++23, clang, mingw target). Project root: <repo>

## Ground rules
- Read `src/10_contract.h` fully. New in it: Settings.osdDurationMs default 2000 read from setting "indicatorDuration"; Settings.taskbarIndicator / taskbarIndicatorOffset; timers kTimerBar / kTimerBarPoll; the "Taskbar workspace indicator (56_bar.cpp)" API (RequestBarUpdate, UpdateTaskbarIndicators, DestroyTaskbarIndicators — implemented by another agent in parallel; just call them). Also read the settings YAML in `src/00_header.wh.cpp`. Then read `src/40_config.cpp`, `src/50_model.cpp`, `src/60_engine.cpp`.
- Edit ONLY those three files. Keep changes minimal; match existing style.
- Log with `HT_LOG(L"...", ...)` only (string-literal format).
- Clean room: do NOT look at or reproduce code from komorebi, GlazeWM, Hyprland, i3, bspwm or any other window manager.
- Verify each: `powershell -NoProfile -File <repo>\build.ps1 -Check 40_config.cpp`, `-Check 50_model.cpp`, `-Check 60_engine.cpp` → errors=0 warnings=0 each. Do NOT run the full build (other fragments change in parallel). NEVER run the mod or anything touching real windows.
- Final report under 200 words.

## 40_config.cpp
- Read the duration from setting `indicatorDuration` (replacing `osdDuration`): value <= 0 → 2000; clamp 100..10000; store in s.osdDurationMs.
- Read `taskbarIndicator` (bool) and `taskbarIndicatorOffset` (int, clamp 0..2000).
- Include them in the summary log line.

## 50_model.cpp — call RequestBarUpdate() (so the taskbar indicator follows state):
- at the end of SwitchWorkspace (both the early "already active" path and the normal path),
- at the end of MoveWindowToWorkspace,
- in ManageWindow after a window was added,
- in UnmanageWindow,
- at the end of RefreshMonitors and ApplySettingsChange,
- in OnUserDragEnd when a window changes monitor (both the floating membership branch and the MoveWindowToMonitor branch).

## 60_engine.cpp
- WM_TIMER: kTimerBar → KillTimer(hwnd, kTimerBar), UpdateTaskbarIndicators(). kTimerBarPoll → UpdateTaskbarIndicators() (periodic; do NOT kill it).
- kTimerDisplay handler: also call UpdateTaskbarIndicators() after RefreshMonitors/RetileAll.
- Init: after the existing end-of-init steps (RetileAll, OnFocusChanged, kTimerSettle), call UpdateTaskbarIndicators() and SetTimer(hwnd, kTimerBarPoll, 1000, nullptr).
- EVENT_SYSTEM_FOREGROUND: also call RequestBarUpdate() (re-raises the bars above the taskbar when the taskbar is clicked, hides them for fullscreen apps).
- WM_HT_RELOAD: after ApplySettingsChange(), call UpdateTaskbarIndicators().
- Teardown: KillTimer(kTimerBar) and KillTimer(kTimerBarPoll) with the other timers, and call DestroyTaskbarIndicators() right before DestroyOsd().

Run the three -Check commands until all report errors=0 warnings=0.
````

## Report from the agent (2026-10-09 07:31 UTC)

````text
All three fragments are edited and each -Check passes with errors=0 warnings=0. Only the three fragments were touched; no full build and the mod was not run.

40_config.cpp: reads `indicatorDuration` (<=0 gives 2000, clamped 100..10000) into osdDurationMs, replacing `osdDuration`. Reads `taskbarIndicator` (bool) and `taskbarIndicatorOffset` (clamped 0..2000). The summary log line includes both. I also renamed its "osdDuration" label to "indicatorDuration".

50_model.cpp: RequestBarUpdate() is called at the 9 sites listed: SwitchWorkspace (early and normal paths), MoveWindowToWorkspace, ManageWindow (before return true), UnmanageWindow, RefreshMonitors, ApplySettingsChange, and both OnUserDragEnd monitor-change branches. The MoveWindowToMonitor branch call is redundant because MoveWindowToWorkspace already requests an update; it is harmless.

60_engine.cpp: kTimerBar (kill then UpdateTaskbarIndicators) and kTimerBarPoll (not killed) are handled in WM_TIMER. kTimerDisplay calls UpdateTaskbarIndicators at the end. Init calls UpdateTaskbarIndicators and starts the 1000 ms poll after kTimerSettle. EVENT_SYSTEM_FOREGROUND calls RequestBarUpdate. WM_HT_RELOAD calls UpdateTaskbarIndicators after ApplySettingsChange. Teardown kills both bar timers and calls DestroyTaskbarIndicators right before DestroyOsd.

Notes:
- The -Check only confirms the declarations in 10_contract.h. Linking depends on 56_bar.cpp defining RequestBarUpdate, UpdateTaskbarIndicators and DestroyTaskbarIndicators, which I did not verify.
- The old `osdDuration` setting is no longer read, so users with a saved value fall back to 2000 ms.
````
