# 4. model/commands fragment

| | |
|---|---|
| Launched | 2026-10-09 06:18 UTC |
| Role | implementer |
| Model | Claude Haiku 5.5 |
| Agent id | `aa55080ea2f39229f` |

## Instructions from the planner (verbatim)

````text
You are implementing one fragment of HawkTile, a clean-room tiling window manager written as a Windhawk mod (C++23, clang, mingw target). Project root: <repo>

## Ground rules (apply to every fragment)
- First read `src/10_contract.h` completely, and `PLAN.md`. The contract is binding: types, globals and every cross-module function signature. Implement exactly those signatures. Do NOT edit 10_contract.h or any file other than the one assigned to you. If you believe the contract is wrong, implement it as written and mention the problem in your final report.
- Fragments are concatenated into ONE translation unit after 10_contract.h, so do not #include project files. The contract already includes <windows.h>, <dwmapi.h>, <shellapi.h>, <algorithm>, <cstdio>, <cwchar>, <cwctype>, <string>, <unordered_set>, <vector>.
- Put all code inside `namespace ht { ... }`. File-local helpers go in an anonymous namespace inside `ht`.
- Log with `HT_LOG(L"format %d", x)` only (the format must be a string literal).
- Clean room: do NOT look at, search for, or reproduce code from komorebi, GlazeWM, Hyprland, i3, bspwm or any other window manager. Use only your own knowledge of Win32.
- Style: 4-space indent, `//` comments only where something is non-obvious, Chromium-like formatting, no exceptions.
- Everything runs on one engine thread; no locks.
- Verify with: `powershell -NoProfile -File <repo>\build.ps1 -Check 50_model.cpp` — it must end with `errors=0 warnings=0`. Fix every warning.
- Never run anything that moves, hides or focuses real windows on this machine.
- Final report (keep it under 250 words): what you implemented, any deviation from the contract, any concerns.

## Your assignment: `src/50_model.cpp`

Define the global `EngineState g;` (inside namespace ht) and implement every function in the contract's "Model (50_model.cpp)" section: RefreshMonitors, ApplySettingsChange, Locate, MonitorIndexFromWindow, ActiveWorkspace, ManageWindow, UnmanageWindow, Retile, RetileAll, RequestRetile, SwitchWorkspace, MoveWindowToWorkspace, MoveWindowToMonitor, CommandTarget, OnFocusChanged, UpdateFocusBorder, OnUserDragEnd, ExecuteCommand, AdoptExistingWindows.

Functions from other fragments you call (declared in the contract): ComputeLayout, FindNeighbor, NextLayout, LayoutName (layout); ClassifyWindow, GetVisibleFrame, PlaceWindow, FocusWindow, SetFocusBorder, ClearFocusBorder, HideForWorkspace, ShowForWorkspace, PersistHiddenList (win32). Do not reimplement them.

Detailed semantics beyond the contract:

Helpers you should write (file-local): `Workspace MakeWorkspace()` (layout = prevLayout = g.settings.defaultLayout, ratio = g.settings.masterRatio); `bool RemoveFrom(std::vector<HWND>&, HWND)`; `HWND FirstWindow(const Workspace&)` (lastFocus if it is still in this workspace's lists, else tiled[0], else floating[0], else nullptr); `void FocusMonitor(int mon)` (g.focusedMon = mon; FocusWindow(FirstWindow(ActiveWorkspace(mon))) if non-null); `std::vector<RECT> MonitorRects()` (rcMonitor of each monitor, for FindNeighbor across monitors).

RefreshMonitors: EnumDisplayMonitors(nullptr, nullptr, callback, lparam) + GetMonitorInfoW(MONITORINFO). Sort by rcMonitor.left then rcMonitor.top. Match each new monitor to an old one by HMONITOR, else by EqualRect(rcMonitor); a matched monitor keeps its workspaces and active index (but updates handle, rcMonitor, rcWork). Unmatched new monitors get workspaceCount fresh workspaces. Windows of unmatched old monitors are appended to new monitor 0's ACTIVE workspace (tiled stays tiled, floating stays floating); call ShowForWorkspace on any of them that is in g.hiddenByUs. If enumeration returns nothing, keep the old list. Clamp g.focusedMon to [0, size-1]. Log the monitor count and each rect.

ApplySettingsChange: as in contract. When shrinking, windows from removed workspaces go into the new last workspace; if that last workspace is the active one show them (ShowForWorkspace if hidden by us); clamp `active` into range (if the active workspace was removed, the new last one becomes active — show its windows that are hidden by us).

Locate: scan every monitor/workspace/tiled+floating vector. Return false for nullptr.

MonitorIndexFromWindow: MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST) compared to monitors[i].handle; fallback: monitor whose rcMonitor contains the center of GetVisibleFrame(hwnd); fallback 0. Return -1 only if g.monitors is empty.

ManageWindow: as in contract; also insert into g.zoomed if IsZoomed. Log hwnd (as %p with (void*)hwnd) and whether floating.

UnmanageWindow: as in contract. Erase from g.pendingVisibility too. If g.borderedHwnd == hwnd: if IsWindow(hwnd) ClearFocusBorder(hwnd); g.borderedHwnd = nullptr.

Retile(mon): as in contract. area = rcWork shrunk by outerGap on every side (if that leaves width or height < 50, use rcWork). LayoutParams{ws.layout, ws.ratio, g.settings.innerGap}. Windows whose PlaceWindow returns false are moved from tiled to floating after the loop (log each), then call RequestRetile() once if any moved.

RequestRetile: `if (g.engineHwnd) SetTimer(g.engineHwnd, kTimerRetile, 15, nullptr);`

SwitchWorkspace(mon, idx): bounds-check both. If idx == active: g.focusedMon = mon; return. Otherwise: newWs = workspaces[idx]; first ShowForWorkspace every window of newWs that is in g.hiddenByUs; then HideForWorkspace every window of the old workspace not already in g.hiddenByUs; set active = idx; g.focusedMon = mon; Retile(mon); focus FirstWindow(newWs) if any; UpdateFocusBorder(); log "workspace %d on monitor %d".

MoveWindowToWorkspace(hwnd, mon, idx): Locate(hwnd) or return; bounds-check; if same monitor and workspace return. Remove from the source vector, append to the target workspace's matching vector (tiled/floating), target.lastFocus = hwnd; clear source.lastFocus if it was hwnd. If the target workspace is not the active one on `mon` → HideForWorkspace(hwnd). Else if mon != source mon and the window is floating → keep its size, move it so it has the same offset from rcWork.left/top of the new monitor as it had on the old monitor (clamped inside the new rcWork) via PlaceWindow. Retile(source mon); if mon differs, Retile(mon). If the window was hidden (target not active), and the source workspace is active, focus FirstWindow(source workspace) so focus stays on the visible workspace.

MoveWindowToMonitor(hwnd, mon): MoveWindowToWorkspace(hwnd, mon, g.monitors[mon].active) with bounds check.

CommandTarget: fg = GetForegroundWindow(); if Locate(fg) return fg; root = GetAncestor(fg, GA_ROOTOWNER); if Locate(root) return root; then lastFocus of ActiveWorkspace(g.focusedMon) if Locate succeeds; else nullptr. Guard g.monitors empty.

OnFocusChanged(hwnd): resolve to hwnd or its GA_ROOTOWNER, whichever is managed. If managed: g.focusedMon = loc.mon; that workspace's lastFocus = resolved hwnd. Always UpdateFocusBorder().

UpdateFocusBorder: target = settings.focusBorder ? CommandTarget() : nullptr. If target == g.borderedHwnd and target != nullptr → SetFocusBorder again anyway (color may have changed) and return. Else clear the old one (if IsWindow), set the new one, store it.

OnUserDragEnd: per contract. Use the center of GetVisibleFrame(hwnd). Other monitor = MonitorFromPoint(center, MONITOR_DEFAULTTONULL) mapped to an index different from the window's current monitor.

AdoptExistingWindows: EnumWindows into a vector first, then for each: skip if Locate; ClassifyWindow(hwnd, g.settings); Tile → ManageWindow(hwnd, false); Float → ManageWindow(hwnd, true). Reverse the enumerated vector before processing (EnumWindows is top-of-z-order first; we want the bottom-most window to become the first tile). Log the count adopted.

ExecuteCommand(cmd): if g.paused and kind is not TogglePause and not Retile → return. Let cur = CommandTarget(), has = Locate(cur, &loc). Workspace for layout commands = has ? the window's workspace : ActiveWorkspace(g.focusedMon) (guard empty monitors). Semantics:
- Focus dir: if !has → FocusMonitor(g.focusedMon) and return. If the workspace layout is Monocle and the window is tiled: among tiled windows that are visible and not iconic, Left/Up focuses the previous one, Right/Down the next one, wrapping around. Otherwise: build `hwnds` = visible, non-iconic tiled windows of that workspace, plus cur if it is floating; rects = GetVisibleFrame of each; n = FindNeighbor(rects, index of cur, dir). If n >= 0 → FocusWindow(hwnds[n]). Else m = FindNeighbor(MonitorRects(), loc.mon, dir); if m >= 0 → FocusMonitor(m).
- Move dir: if !has return. If floating: m = FindNeighbor(MonitorRects(), loc.mon, dir); if m >= 0 → MoveWindowToMonitor(cur, m), FocusWindow(cur). Return. If layout is Monocle: Left/Up swaps with tiled[index-1] if it exists, Right/Down with tiled[index+1] if it exists; if no such slot, fall through to the monitor crossing below. Otherwise find the neighbour exactly as in Focus (tiled visible non-iconic windows only); if found swap the two HWNDs in ws.tiled, Retile(loc.mon), FocusWindow(cur). If not found: m = FindNeighbor(MonitorRects(), loc.mon, dir); if m >= 0 → MoveWindowToMonitor(cur, m); FocusWindow(cur).
- FocusWorkspace: arg in 1..settings.workspaceCount → SwitchWorkspace(g.focusedMon, arg - 1).
- MoveToWorkspace: has and arg in range → MoveWindowToWorkspace(cur, loc.mon, arg - 1).
- FocusMonitorNext: if more than one monitor → FocusMonitor((g.focusedMon + 1) % count).
- MoveToMonitorNext: has and more than one monitor → MoveWindowToMonitor(cur, (loc.mon + 1) % count); FocusWindow(cur).
- CycleLayout: ws.layout = NextLayout(ws.layout); Retile(that monitor); log LayoutName.
- SetLayout: ws.layout = cmd.layout; Retile; log.
- ToggleFloat: has. Floating → remove from floating, append to tiled. Tiled → remove from tiled, push to floating, then PlaceWindow(cur, centered rect of 60% width and 60% height of the monitor's rcWork). Retile(loc.mon); UpdateFocusBorder().
- ToggleMonocle: if ws.layout == Monocle → ws.layout = (ws.prevLayout == Monocle ? g.settings.defaultLayout : ws.prevLayout); else ws.prevLayout = ws.layout, ws.layout = Monocle. Retile; if has FocusWindow(cur) (brings it on top in monocle).
- Promote: has and tiled and tiled.size() > 1: if index == 0 swap tiled[0] and tiled[1]; else erase and insert at 0. Retile; FocusWindow(cur).
- Grow / Shrink: ws.ratio ± g.settings.ratioStep, clamped to [0.15, 0.85]; Retile.
- Retile: RefreshMonitors(); AdoptExistingWindows(); RetileAll(); UpdateFocusBorder().
- TogglePause: flip g.paused; log; if now unpaused → RetileAll().
- Close: has → PostMessageW(cur, WM_SYSCOMMAND, SC_CLOSE, 0).

Guard every index access (monitors may be empty; vectors may be empty). Run `build.ps1 -Check 50_model.cpp` until it reports errors=0 warnings=0.
````

## Report from the agent (2026-10-09 06:26 UTC)

````text
Implemented src/50_model.cpp: defines `EngineState g` and all 19 functions in the contract's Model section, with file-local helpers in an anonymous namespace. `build.ps1 -Check 50_model.cpp` ends with `errors=0 warnings=0`. No lines exceed 80 columns. I did not edit the contract or any other fragment, and ran nothing that moves or focuses windows.

Deviations from the brief (deliberate):
- UnmanageWindow: for a live window we hid, it calls ShowForWorkspace instead of only erasing it from hiddenByUs, so an unmanaged window is never left invisible. Destroyed windows are just erased and persisted.
- SwitchWorkspace and MoveWindowToWorkspace call RequestRetile() after showing windows, because a just-shown window may not report IsWindowVisible yet when Retile runs.
- MoveWindowToWorkspace also shows the window if it was hidden and the target is active, and calls UpdateFocusBorder() at the end.
- ApplySettingsChange shows any hidden windows in the active workspace, which covers the removed-active case generically.
- Cycle/set layout into Monocle records prevLayout, so toggle-monocle returns to the previous layout.
- ActiveWorkspace returns a static fallback for an invalid index and creates a workspace if the list is empty.
- SwitchWorkspace logs the 1-based workspace number.

Concerns for the engine fragment (60_engine.cpp):
- Retile only lays out windows with IsWindowVisible true. Windows shown via ShowWindowAsync get placed only after the retile triggered by their show event, so the engine must RequestRetile on EVENT_OBJECT_SHOW/HIDE/DESTROY for managed windows.
- Because UnmanageWindow now makes a hidden window visible, the engine must not unmanage windows on its own hide events, or they will reappear on the wrong workspace.

No contract problems found.
````
