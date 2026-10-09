# 8. model + engine changes

| | |
|---|---|
| Launched | 2026-10-09 06:40 UTC |
| Role | implementer |
| Model | Claude Haiku 5.5 |
| Agent id | `a6b3619db44833f87` |

## Instructions from the planner (verbatim)

````text
You are modifying two fragments of HawkTile, a clean-room tiling window manager written as a Windhawk mod (C++23, clang, mingw target). Project root: <repo>

## Ground rules
- First read `src/10_contract.h` completely — it was just updated (Settings gained tileMaximized / workspaceOsd / osdDurationMs / debugLog; CommandKind gained SendToWorkspace and ToggleMaximize; MoveToWorkspace now means "move and follow"; new MoveWindowToWorkspaceAndFollow; new ShowWorkspaceOsd / DestroyOsd implemented by another fragment; new timer id kTimerSettle; ManageWindow and SwitchWorkspace comments changed). Then read `src/50_model.cpp` and `src/60_engine.cpp` fully. Implement exactly the contract. Do NOT edit any file other than those two. If the contract seems wrong, implement it as written and mention it in your report.
- Fragments are concatenated into ONE translation unit; the file-local anonymous namespace inside `ht` is SHARED across fragments, so new helper names must be distinctive.
- Log with `HT_LOG(L"...", ...)` only (string-literal format).
- Clean room: do NOT look at or reproduce code from komorebi, GlazeWM, Hyprland, i3, bspwm or any other window manager.
- Style: match the existing code. Keep changes minimal and focused; don't reformat untouched code.
- Verify: `powershell -NoProfile -File <repo>\build.ps1 -Check 50_model.cpp` and `... -Check 60_engine.cpp` must each end with `errors=0 warnings=0`.
- NEVER run the mod or anything that touches real windows.
- Final report under 250 words.

## Changes in `src/50_model.cpp`
1. ManageWindow: after adding a TILED window, if `g.settings.tileMaximized && IsZoomed(hwnd)` → `ShowWindowAsync(hwnd, SW_RESTORE)` and log it. Keep inserting zoomed windows into g.zoomed (the engine's LOCATIONCHANGE handler sees the zoomed→restored transition and retiles).
2. ToggleFloating: when a floating window becomes tiled and it IsZoomed and tileMaximized → ShowWindowAsync(SW_RESTORE).
3. Refactor: extract the membership move in MoveWindowToWorkspace (remove from the source tiled/floating vector, append to the target workspace's matching vector, set target.lastFocus = hwnd, clear source.lastFocus if it was hwnd) into a file-local helper with a distinctive name (e.g. `MoveWindowMembership(HWND, const WinLoc&, int mon, int wsIndex)`), and use it from MoveWindowToWorkspace (behaviour of MoveWindowToWorkspace otherwise unchanged).
4. New `void MoveWindowToWorkspaceAndFollow(HWND hwnd, int wsIndex)` per contract: Locate or return; mon = loc.mon; bounds-check wsIndex against that monitor's workspaces. If loc.ws == wsIndex → if workspaceOsd ShowWorkspaceOsd(mon); return. If loc.ws is not the monitor's active workspace (unusual) → MoveWindowToWorkspace(hwnd, mon, wsIndex); SwitchWorkspace(mon, wsIndex); return. Otherwise: MoveWindowMembership(...) WITHOUT hiding the window, then SwitchWorkspace(mon, wsIndex). (SwitchWorkspace shows the target's hidden windows, hides what is left in the old workspace — no longer including hwnd — retiles, and focuses FirstWindow(target), which is hwnd because target.lastFocus = hwnd.) Log it.
5. SwitchWorkspace: if wsIndex == active: set g.focusedMon = mon, and if g.settings.workspaceOsd call ShowWorkspaceOsd(mon), then return. In the normal path call ShowWorkspaceOsd(mon) at the end when g.settings.workspaceOsd.
6. ExecuteCommand:
   - MoveToWorkspace: `has && arg in 1..workspaceCount` → MoveWindowToWorkspaceAndFollow(cur, arg - 1).
   - SendToWorkspace (new): `has && arg in range` → MoveWindowToWorkspace(cur, loc.mon, arg - 1); then if workspaceOsd ShowWorkspaceOsd(loc.mon).
   - ToggleMaximize (new): `has` → if IsZoomed(cur) ShowWindowAsync(cur, SW_RESTORE) else ShowWindowAsync(cur, SW_MAXIMIZE). (The engine's LOCATIONCHANGE handling takes it out of / back into the layout.)
   - FocusMonitorNext: after focusing the next monitor, if workspaceOsd ShowWorkspaceOsd(that monitor).

## Changes in `src/60_engine.cpp`
1. At the end of init (right after the existing RetileAll() / OnFocusChanged), `SetTimer(hwnd, kTimerSettle, 300, nullptr)` — windows restored from maximized at adoption finish asynchronously, possibly before the hooks were installed, so one late retile catches them.
2. WM_TIMER: handle kTimerSettle → KillTimer, RetileAll(), UpdateFocusBorder().
3. Teardown: KillTimer(kTimerSettle) with the others, and call DestroyOsd() before DestroyWindow(hwnd).
4. Change the startup log line (or add one) to say "engine started" with the number of monitors and managed windows if convenient (optional).

Run both -Check commands until both report errors=0 warnings=0.
````

## Report from the agent (2026-10-09 06:42 UTC)

````text
Both fragments are implemented per the contract. `build.ps1 -Check 50_model.cpp` and `-Check 60_engine.cpp` each end with errors=0 warnings=0. Only src/50_model.cpp and src/60_engine.cpp were edited. The mod was not run.

50_model.cpp:
- ManageWindow: a tiled window that is maximized, with tileMaximized set, gets ShowWindowAsync(SW_RESTORE) and is still inserted into g.zoomed.
- ToggleFloating: a floating window that becomes tiled and is maximized gets SW_RESTORE.
- New anonymous-namespace helper MoveWindowMembership(hwnd, loc, mon, wsIndex). MoveWindowToWorkspace now uses it, with unchanged behaviour.
- New MoveWindowToWorkspaceAndFollow as specified: no-op plus OSD when already there; unusual inactive-source branch uses send + switch; normal path moves membership without hiding, then SwitchWorkspace.
- SwitchWorkspace shows the OSD on both paths. ExecuteCommand: MoveToWorkspace follows, SendToWorkspace added, ToggleMaximize added, FocusMonitorNext shows the OSD.

60_engine.cpp:
- SetTimer(hwnd, kTimerSettle, 300) after the initial RetileAll/OnFocusChanged.
- WM_TIMER handles kTimerSettle (KillTimer, RetileAll, UpdateFocusBorder).
- Teardown adds KillTimer(kTimerSettle) and DestroyOsd() before DestroyWindow.
- Startup log: "engine started: N monitor(s), M managed window(s)".

Findings:
- `-Check` compiles the contract plus one fragment only, so 60_engine.cpp cannot call MonitorCount() (a 50_model helper). I used g.monitors.size() there. Engine code that calls model internals will fail the check the same way.
- Contract edge case: if the window is on an inactive workspace and wsIndex is the active one, SwitchWorkspace returns early. The window is moved and shown but not focused. Implemented as written. A FocusWindow(hwnd) after the call would fix it if you want that.
- src/55_osd.cpp does not exist yet, so ShowWorkspaceOsd and DestroyOsd are only checked as declarations. The full build needs that fragment.
````
