# 30. Alt+Shift chord + setting

| | |
|---|---|
| Launched | 2026-10-14 05:46 UTC |
| Role | implementer |
| Model | Claude Haiku 5.5 |
| Agent id | `a1adb5ef1343e3476` |

## Instructions from the planner (verbatim)

````text
You are modifying two fragments of HawkTile, a tiling window manager written as a Windhawk mod (C++23, clang, mingw target). Project root: <repo>

## Ground rules
- Read `src/10_contract.h` fully. New: setting `mouseResize` (path `windows.mouseResize`), timer `kTimerMouseResize`, the model functions BeginMouseResize / UpdateMouseResize / EndMouseResize / MouseResizeActive (implemented by another agent in 50_model.cpp at the same time), and at the very end the "Alt+Shift mouse resize" rules you implement in the engine, right after the hold-left-Alt rules. Then read `src/60_engine.cpp` (especially OnRawKey and the hold-Alt code, which you must keep working exactly as now) and `src/40_config.cpp`.
- Edit ONLY `src/60_engine.cpp` and `src/40_config.cpp`.
- Match each file's style. Never log key codes or characters.
- Clean room: do NOT look at, search for or reproduce code from any other window manager or window utility.
- Never run the mod or hawktile-dev.exe. Running `build.ps1 -Test` IS allowed and expected.

## 40_config.cpp
`s.mouseResize = Wh_GetIntSetting(L"windows.mouseResize") != 0;` next to the other windows.* reads, and add it to the LoadSettings summary log.

## 60_engine.cpp
Implement the contract's "Alt+Shift mouse resize" rules:
- State: `s_resizePolling`, `s_resizeAnchor` (POINT), `s_resizeGaveUp` (this chord found no tiled window; stop polling until the chord turns on again).
- Helpers: `ResizeChordDown()` (GetAsyncKeyState: VK_LMENU and VK_SHIFT down; VK_CONTROL, VK_LWIN, VK_RWIN up) and `StopMouseResize()` (KillTimer kTimerMouseResize, s_resizePolling = false, EndMouseResize()).
- In OnRawKey: after the existing hold-Alt handling of each key event (keep that logic unchanged), on a key-DOWN of left Alt or of Shift (VK_SHIFT/VK_LSHIFT/VK_RSHIFT): if not polling and ResizeChordDown() and settings.mouseResize && !g.paused && !WorkspaceOverviewOpen() → anchor = GetCursorPos, start the 16 ms timer, s_resizePolling = true. Auto-repeat key-downs must not restart it (only start when not already polling). Any other key-down (not Alt, not Shift, not kMenuMaskVk/0xFF — those are already filtered) while polling → StopMouseResize(). Key-UP of left Alt or Shift while polling → StopMouseResize().
- Hold-Alt interaction: kTimerHold's OpenHoldOverview must not open the overview while s_resizePolling (add that condition).
- WM_TIMER kTimerMouseResize: if !ResizeChordDown() → StopMouseResize(). Else if !MouseResizeActive(): if the cursor is more than 2*SM_CXDRAG horizontally or 2*SM_CYDRAG vertically from the anchor → if BeginMouseResize(anchor) then SendMenuMaskKey() else StopMouseResize() (give up for this chord). Else (session active) UpdateMouseResize(cursor).
- Teardown, WM_HT_RELOAD when !mouseResize, and WM_ENDSESSION: StopMouseResize(). KillTimer with the others in teardown.

## Verify
- `powershell -NoProfile -File <repo>\build.ps1 -Check 60_engine.cpp` and `-Check 40_config.cpp` → errors=0 warnings=0 (authoritative).
- Full `... build.ps1` (both clang lines) and `... build.ps1 -Test` (0 failed). If the full build fails only because the four model functions are not defined yet (other agent mid-edit), say so and do not touch 50_model.cpp.

## Report
Under 150 words: changes, verification results line by line, anything ambiguous.
````

## Report from the agent (2026-10-14 05:48 UTC)

````text
Done. Edited only src/40_config.cpp and src/60_engine.cpp.

Changes:
- 40_config.cpp: `s.mouseResize = Wh_GetIntSetting(L"windows.mouseResize") != 0;` after followActivation; `mouseResize=%d` added to the LoadSettings summary log.
- 60_engine.cpp: state s_resizePolling, s_resizeAnchor, s_resizeGaveUp. Helpers ResizeChordDown, StopMouseResize, PollMouseResize (timer logic per brief), ResizeKeyEvent. OnRawKey now filters then calls HoldKeyEvent (the old body, logic unchanged, only its signature changed) and then ResizeKeyEvent. OpenHoldOverview returns early while s_resizePolling. WM_TIMER kTimerMouseResize, WM_HT_RELOAD (when !mouseResize), WM_ENDSESSION, and TeardownEngine (KillTimer + StopMouseResize) wired.

Verification:
- -Check 60_engine.cpp: errors=0 warnings=0
- -Check 40_config.cpp: errors=0 warnings=0
- full build.ps1 (both clang lines): exit 0, errors=0 warnings=0
- build.ps1 -Test: 1035 passed, 0 failed

Ambiguities, decided as follows:
- Right Alt (E0) key-down during a poll stops it, matching the hold rules' "counts as any other key".
- Key-up of any Shift (not only left) stops the poll, per the brief, even if the other Shift is still held.
- s_resizeGaveUp clears on any key event where the chord is off.

Not done: 00_header.wh.cpp already has windows.mouseResize in its settings spec (line 191), so nothing was needed there.
````
