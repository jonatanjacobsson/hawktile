# 22. follow window activation

| | |
|---|---|
| Launched | 2026-10-09 08:27 UTC |
| Role | implementer |
| Model | Claude Haiku 5.5 |
| Agent id | `a49a51b9c31e44806` |

## Instructions from the planner (verbatim)

````text
You are modifying three fragments of HawkTile, a clean-room tiling window manager written as a Windhawk mod (C++23, clang, mingw target). Project root: <repo>

## Ground rules
- Read `src/10_contract.h` fully. New: Settings.followActivation, EngineState.pendingActivation, and `ActivateWindowWorkspace(HWND)`. Read the YAML in `src/00_header.wh.cpp` (setting `followActivation`). Then read `src/40_config.cpp`, `src/50_model.cpp`, `src/60_engine.cpp` fully.
- Edit ONLY those three files; minimal, focused changes in the existing style. New file-local helpers need distinctive names (the anonymous namespace is shared across fragments).
- Log with `HT_LOG(L"...", ...)` only.
- Clean room: do NOT look at or reproduce code from komorebi, GlazeWM, Hyprland, i3, bspwm or any other window manager.
- Verify: `powershell -NoProfile -File <repo>\build.ps1 -Check 40_config.cpp`, `-Check 50_model.cpp`, `-Check 60_engine.cpp` (errors=0 warnings=0 each), then the full `powershell -NoProfile -File <repo>\build.ps1` (both clang lines errors=0 warnings=0) and `... build.ps1 -Test` (0 failed). NEVER run the mod or anything touching real windows.
- Final report under 180 words.

## Background
Workspaces hide windows with ShowWindowAsync(SW_HIDE) and remember them in g.hiddenByUs. When the user clicks a Windows notification, the app activates its (hidden) window: either it calls SetForegroundWindow on it (→ EVENT_SYSTEM_FOREGROUND for a window in g.hiddenByUs), or it shows/restores it first (→ EVENT_OBJECT_SHOW; the engine's ProcessPending then sees a window that is in g.hiddenByUs but IsWindowVisible, and today MOVES it to the active workspace). The goal: instead, switch to the workspace that holds the window and focus it.

## 40_config.cpp
Read `followActivation` (bool) into s.followActivation; add it to the summary log.

## 50_model.cpp
Implement `ActivateWindowWorkspace(HWND hwnd)` per the contract: Locate or return; Workspace& ws = that workspace; ws.lastFocus = hwnd; if loc.ws != monitors[loc.mon].active → log "following activation of %p to workspace %d" and SwitchWorkspace(loc.mon, loc.ws); then FocusWindow(hwnd).

## 60_engine.cpp
1. EVENT_SYSTEM_FOREGROUND: resolve the managed window (hwnd itself, else GetAncestor(hwnd, GA_ROOTOWNER)); if it is managed AND in g.hiddenByUs AND g.settings.followActivation → g.pendingActivation.insert(that managed hwnd) and SchedulePending(). Keep the existing behaviour of that case otherwise (pendingEval for unmanaged, OnFocusChanged, RequestBarUpdate).
2. ProcessPending: first handle g.pendingActivation (swap into a local vector, clear): for each hwnd that is still managed (Locate) and still in g.hiddenByUs: check it (or a window it owns) is still the foreground — fg = GetForegroundWindow(); ok if fg == hwnd or GetAncestor(fg, GA_ROOTOWNER) == hwnd — if ok → ActivateWindowWorkspace(hwnd). (The 80 ms delay filters out transient activations.)
3. ProcessPending, pendingVisibility branch "window in g.hiddenByUs but IsWindowVisible" (the app re-showed a window on an inactive workspace): if g.settings.followActivation → ActivateWindowWorkspace(hwnd) (it remains in hiddenByUs until SwitchWorkspace shows the workspace — that is fine, SwitchWorkspace handles it) instead of moving it; otherwise keep today's behaviour (move it to the active workspace).
4. Erase the window from g.pendingActivation wherever the engine already erases it from the other pending sets on EVENT_OBJECT_DESTROY.

Run all the commands until clean.
````

## Report from the agent (2026-10-09 08:28 UTC)

````text
Follow-activation is implemented in the three fragments. All verification is clean.

- 40_config.cpp: reads `followActivation` into s.followActivation and adds it to the summary log.
- 50_model.cpp: `ActivateWindowWorkspace(HWND)` follows the contract. It normalises the active workspace, sets lastFocus, switches only if the workspace is not active (logged with a 1-based number), then focuses.
- 60_engine.cpp:
  - EVENT_SYSTEM_FOREGROUND: a managed hidden window, or its root owner, is queued in pendingActivation when followActivation is on.
  - ProcessPending handles pendingActivation first. It follows the window only if it is still foreground (or its root owner is) after the 80 ms delay.
  - The pendingVisibility branch calls ActivateWindowWorkspace instead of moving the window. Deviation: this only happens when the window's workspace is not active. A hidden window on the active workspace is an invariant break, and the old show-and-move path repairs it. ActivateWindowWorkspace would leave it hidden-but-visible.
  - EVENT_OBJECT_DESTROY erases from pendingActivation.

Verification: -Check is errors=0 warnings=0 for all three fragments. Full build is clean on both clang lines. -Test reports 1035 passed, 0 failed. -Test only builds and runs the standalone layout test, and the mod was not run. The build also regenerated dist/hawktile.wh.cpp. Nothing was committed.
````
