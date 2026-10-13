# 27. hold-Alt detection + settings

| | |
|---|---|
| Launched | 2026-10-13 07:58 UTC |
| Role | implementer |
| Model | Claude Haiku 5.5 |
| Agent id | `a94a07085d91365ec` |

## Instructions from the planner (verbatim)

````text
You are modifying two fragments of HawkTile, a tiling window manager written as a Windhawk mod (C++23, clang, mingw target). Project root: <repo>

## Ground rules
- Read `src/10_contract.h` fully first. It is the shared contract; do not change it. New in it: timers kTimerHold / kTimerHoldPoll, settings fields holdOverview / holdDelayMs, the "Workspace overview (hold left Alt)" functions (ShowWorkspaceOverview, CloseWorkspaceOverview, WorkspaceOverviewOpen, RefreshWorkspaceOverview — implemented by another agent in 55_osd.cpp at the same time), and at the very end the "Hold-left-Alt detection" rules you implement. Then read `src/60_engine.cpp` and `src/40_config.cpp` fully.
- Edit ONLY `src/60_engine.cpp` and `src/40_config.cpp`. Do not touch 55_osd.cpp (another agent is editing it) or 00_header.wh.cpp (the planner adds the YAML).
- Match each file's style: comment density, naming, logging with HT_LOG.
- Clean room: do NOT look at, search for or reproduce code from any other window manager or task switcher. Use only the Win32 API documentation you know and this repository.
- NEVER run the mod or any built executable.

## 40_config.cpp
Read two new settings next to the other `indicators.*` ones, in the same style as `indicators.popupDuration`:
- `s.holdOverview = Wh_GetIntSetting(L"indicators.holdAlt") != 0;`
- `indicators.holdDelay`: <= 0 → 400, otherwise clamped to 100..2000 → `s.holdDelayMs`.
If the file has a list of setting paths in a comment or log, add these two.

## 60_engine.cpp
1. Raw Input registration: in EngineThreadProc, right after the hotkeys are registered, register the keyboard (usUsagePage 0x01, usUsage 0x06, dwFlags RIDEV_INPUTSINK, hwndTarget = the engine window) with RegisterRawInputDevices; log a failure with GetLastError and carry on (the overview then simply never opens). Do NOT use RIDEV_NOLEGACY. In TeardownEngine: KillTimer kTimerHold and kTimerHoldPoll with the other timers, and unregister (same usage page/usage, dwFlags RIDEV_REMOVE, hwndTarget nullptr) before the window is destroyed.
2. WM_INPUT in EngineWndProc: read with GetRawInputData((HRAWINPUT)lParam, RID_INPUT, &raw, &size, sizeof(RAWINPUTHEADER)) into a local RAWINPUT (size = sizeof(RAWINPUT); treat (UINT)-1 or a non-keyboard dwType as nothing to do); for RIM_TYPEKEYBOARD call a new anonymous-namespace function `OnRawKey(const RAWKEYBOARD&)`. Then fall through to DefWindowProcW (required for WM_INPUT cleanup) — do not `return 0`.
3. OnRawKey implements the contract's rules exactly. State in the anonymous namespace: `s_leftAltDown`, `s_holdArmed`, `s_holdShown` (this Alt press opened the overview). Details:
   - Ignore VKey == kMenuMaskVk (our own injected key) and VKey == 0xFF.
   - Key up = (Flags & RI_KEY_BREAK). Left Alt = (VKey == VK_MENU || VKey == VK_LMENU) && !(Flags & RI_KEY_E0). Right Alt (E0) counts as "any other key".
   - Left Alt down: if already s_leftAltDown it is auto-repeat → ignore. Else set s_leftAltDown; arm (SetTimer kTimerHold, g.settings.holdDelayMs; s_holdArmed = true) only if g.settings.holdOverview && !g.paused && Shift, Ctrl and both Win keys are up (GetAsyncKeyState(VK_SHIFT/VK_CONTROL/VK_LWIN/VK_RWIN) & 0x8000).
   - Left Alt up: s_leftAltDown = false; disarm (KillTimer, s_holdArmed = false); if s_holdShown → s_holdShown = false and CloseWorkspaceOverview(false).
   - Any other key, key-down only (ignore key-ups):
     - "modifier" = VK_SHIFT, VK_LSHIFT, VK_RSHIFT, VK_CONTROL, VK_LCONTROL, VK_RCONTROL, VK_LWIN, VK_RWIN.
     - "bound" = some binding in g.settings.bindings has (modifiers & MOD_ALT) and vk == VKey. Write a small helper for it.
     - If the overview is open for this press (s_holdShown && WorkspaceOverviewOpen()): modifier or bound → nothing; anything else (Escape included) → s_holdShown = false, CloseWorkspaceOverview(true).
     - Else, if s_leftAltDown: bound → re-arm the hold timer (only when the arming conditions holdOverview && !paused hold; modifiers are NOT checked here); modifier or anything else → disarm.
4. WM_TIMER:
   - kTimerHold: KillTimer; s_holdArmed = false; if s_leftAltDown && (GetAsyncKeyState(VK_LMENU) & 0x8000) && g.settings.holdOverview && !g.paused && g.focusedMon is a valid index of g.monitors → SendMenuMaskKey(); ShowWorkspaceOverview(g.focusedMon); s_holdShown = WorkspaceOverviewOpen(); if s_holdShown → SetTimer(kTimerHoldPoll, 100).
   - kTimerHoldPoll: if !WorkspaceOverviewOpen() → KillTimer(kTimerHoldPoll), s_holdShown = false. Else if s_holdShown and left Alt is not down (GetAsyncKeyState(VK_LMENU) & 0x8000 == 0) → s_leftAltDown = false, s_holdShown = false, CloseWorkspaceOverview(false). (Keep polling until the overview reports closed: a drag can keep it open briefly after Alt is up.)
   - kTimerBar: after UpdateTaskbarIndicators(), also call RefreshWorkspaceOverview().
5. WM_HT_RELOAD: after the settings are applied, if !g.settings.holdOverview → disarm, s_holdShown = false, CloseWorkspaceOverview(true).
6. WM_ENDSESSION (the session-ending branch): disarm and CloseWorkspaceOverview(true) along with the rest.
7. Log once when the overview opens is done by 55_osd.cpp; do not add per-key logging (keys must never be logged).

## Verify
- `powershell -NoProfile -File <repo>\build.ps1 -Check 60_engine.cpp` and `... -Check 40_config.cpp` must each show errors=0 warnings=0. These are the authoritative checks for your work.
- Then the full build `powershell -NoProfile -File <repo>\build.ps1` (both clang lines should be errors=0 warnings=0) and `... build.ps1 -Test` (0 failed). If the full build fails ONLY because the four overview functions are not yet defined or 55_osd.cpp has errors (the other agent's file, possibly mid-edit), say so and do not touch it.
- Confirm by reading your code once more that no key code or character is ever logged.

## Report
Under 200 words: what you changed in each file, the verification results line by line, and anything in the spec you could not follow or found ambiguous.
````

## Report from the agent (2026-10-13 08:03 UTC)

````text
**40_config.cpp**: reads `indicators.holdAlt` into `holdOverview` and `indicators.holdDelay` into `holdDelayMs` (<=0 gives 400, clamped 100..2000). Both were also added to the LoadSettings summary log.

**60_engine.cpp**: keyboard Raw Input (RIDEV_INPUTSINK) is registered after the hotkeys and removed in teardown. WM_INPUT calls OnRawKey, then falls through to DefWindowProcW. Hold state, the kTimerHold and kTimerHoldPoll handlers, RefreshWorkspaceOverview on kTimerBar, and DropHold on reload (when the feature is off) and on session end are all in place. OnRawKey logs nothing; the only new log line is the RegisterRawInputDevices failure.

**Judgment calls**: teardown also closes the overview and resets the hold flags, because they are statics that survive a stop/start. A bound key disarms the hold when the feature is off or paused. Adding the two settings to the summary log is my reading of "list of setting paths in a log".

**Verification**:
- `-Check 60_engine.cpp`: errors=0 warnings=0
- `-Check 40_config.cpp`: errors=0 warnings=0
- Full build: all fragments compile with no warnings. Link fails with 4 undefined symbols (ShowWorkspaceOverview, CloseWorkspaceOverview, WorkspaceOverviewOpen, RefreshWorkspaceOverview), all from 55_osd.cpp, which is not implemented yet. I did not touch it.
- Real-mode syntax-only pass on dist/hawktile.wh.cpp: no diagnostics.
- `-Test`: not run. It executes layout_test.exe, which the ground rules forbid. Its inputs, 10_contract.h and 20_layout.cpp, are unchanged.

**Spec notes**: per the re-arm rule, Alt, then Shift, then 1 re-arms the hold, so the overview can open with Shift held. If a left Alt key-up is lost while the overview is closed, the next Alt press is ignored as a repeat until Alt is released.
````
