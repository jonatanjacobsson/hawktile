# 31. resize nudge for stale apps

| | |
|---|---|
| Launched | 2026-10-14 11:32 UTC |
| Role | implementer |
| Model | Claude Haiku 5.5 |
| Agent id | `a651384f70ab8c31f` |

## Instructions from the planner (verbatim)

````text
You are modifying HawkTile, a tiling window manager written as a Windhawk mod (C++23, clang, mingw target). Project root: <repo>

## Background
Some apps (seen with Revit) miss the resize HawkTile sends right after the window is shown again (workspace switch) or restored from maximized: the frame gets the new size but the app keeps painting at its old size, so the rest of the window is black. Fix: shortly after such an event, re-send the window's size with a one-pixel "nudge" so the app gets a fresh WM_SIZE.

## Ground rules
- Read `src/10_contract.h` fully. New in it: timer `kTimerNudge`, `EngineState::needsNudge`, the extended comment on ShowForWorkspace, and `NudgeWindow` / `RequestNudge` (Win32 helpers section). Do not change the contract.
- Edit ONLY `src/30_win32.cpp` and `src/60_engine.cpp`. Match each file's style.
- Clean room: do NOT look at, search for or reproduce code from any other window manager.
- Never run the mod or hawktile-dev.exe. Running `build.ps1 -Test` IS allowed and expected.

## 30_win32.cpp
- ShowForWorkspace: after ShowWindowAsync, `g.needsNudge.insert(hwnd); RequestNudge();`.
- RequestNudge(): if g.engineHwnd → SetTimer(g.engineHwnd, kTimerNudge, 300, nullptr).
- NudgeWindow(hwnd): return if !IsWindow, !IsWindowVisible, IsIconic or IsZoomed. GetWindowRect → r. Flags = SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOACTIVATE | SWP_ASYNCWINDOWPOS. SetWindowPos(hwnd, nullptr, r.left, r.top, width - 1, height, flags) then SetWindowPos(hwnd, nullptr, r.left, r.top, width, height, flags). Only when width > 1.

## 60_engine.cpp
- WinEventProc, EVENT_OBJECT_LOCATIONCHANGE: where a managed window goes from zoomed to not zoomed (the existing `zoomed != wasZoomed` branch, the !zoomed case), also `g.needsNudge.insert(hwnd); RequestNudge();`.
- EVENT_OBJECT_DESTROY: `g.needsNudge.erase(hwnd);` with the other erases.
- WM_TIMER kTimerNudge: KillTimer; move g.needsNudge into a local vector and clear the set; for each hwnd that Locate() finds, is not in g.hiddenByUs, and whose workspace is its monitor's active one → NudgeWindow(hwnd). HT_LOG one line with the count when > 0 ("nudged %d window(s)").
- TeardownEngine: KillTimer(kTimerNudge) with the other timers. (g is reset at the end anyway.)

## Verify
- `powershell -NoProfile -File <repo>\build.ps1 -Check 30_win32.cpp` and `-Check 60_engine.cpp` → errors=0 warnings=0.
- Full `... build.ps1` (both clang lines errors=0 warnings=0) and `... build.ps1 -Test` (0 failed).

## Report
Under 120 words: changes, verification results line by line, anything ambiguous.
````

## Report from the agent (2026-10-14 11:33 UTC)

````text
Nudge feature implemented in src/30_win32.cpp and src/60_engine.cpp only. Contract untouched.

Changes:
- 30_win32.cpp: ShowForWorkspace now inserts into g.needsNudge and calls RequestNudge(). Added RequestNudge() (SetTimer kTimerNudge 300 ms) and NudgeWindow() (skips invalid/hidden/minimized/maximized, width <= 1; two SetWindowPos calls, width-1 then width).
- 60_engine.cpp: LOCATIONCHANGE zoomed→not-zoomed inserts into needsNudge and calls RequestNudge(). DESTROY erases from needsNudge. WM_TIMER kTimerNudge: KillTimer, snapshot and clear the set, nudge windows that Locate() finds, are not in hiddenByUs, and are on their monitor's active workspace; HT_LOG "nudged %d window(s)" when > 0. TeardownEngine kills kTimerNudge.

Verification:
- build.ps1 -Check 30_win32.cpp: errors=0 warnings=0
- build.ps1 -Check 60_engine.cpp: errors=0 warnings=0
- build.ps1 full: both clang lines errors=0 warnings=0
- build.ps1 -Test: 1035 passed, 0 failed

Ambiguities: none blocking. I wrapped one long if-condition in the timer case to fit the file's line width. Not run against Revit, since running the mod was out of scope.
````
