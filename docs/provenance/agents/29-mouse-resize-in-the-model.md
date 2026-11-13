# 29. mouse resize in the model

| | |
|---|---|
| Launched | 2026-10-14 05:46 UTC |
| Role | implementer |
| Model | Claude Haiku 5.5 |
| Agent id | `a28836eab5ac09497` |

## Instructions from the planner (verbatim)

````text
You are modifying one fragment of HawkTile, a tiling window manager written as a Windhawk mod (C++23, clang, mingw target). Project root: <repo>

## Ground rules
- Read `src/10_contract.h` fully, especially the new section "Alt+Shift mouse resize" just above ExecuteCommand (BeginMouseResize, UpdateMouseResize, EndMouseResize, MouseResizeActive) and ApplyResize / ComputeLayout in the layout section. Then read `src/50_model.cpp` fully; note LayoutWindowsOf, LayoutAreaOf, LayoutParamsOf, Retile and OnUserDragEnd (which already does a one-shot ApplyResize — reuse its approach).
- Edit ONLY `src/50_model.cpp`. Another agent edits 60_engine.cpp and 40_config.cpp at the same time; it calls your four functions.
- Match the file's style (anonymous-namespace helpers, HT_LOG, comment density).
- Clean room: do NOT look at, search for or reproduce code from any other window manager or window utility.
- Never run the mod or hawktile-dev.exe. Running `build.ps1 -Test` IS allowed and expected.

## Implement exactly as the contract says
- Session state in the anonymous namespace (hwnd, mon, ws index, layout index, count, area, start ratios copy, start slot, start cursor, which horizontal edge (left/right) and vertical edge (top/bottom) moves, last applied delta, `changed` flag).
- BeginMouseResize(cursor): EndMouseResize() first. WindowFromPoint(cursor) → GetAncestor(GA_ROOT). Must Locate() as tiled (not floating), loc.ws == the monitor's active workspace, ws.layout != Monocle, LayoutWindowsOf(ws) contains it and has >= 2 windows. Slot = ComputeLayout(LayoutAreaOf(mon), count, LayoutParamsOf(ws))[index]. Edges by quadrant: cursor.x < slot center x → left edge, else right; cursor.y < center y → top, else bottom.
- UpdateMouseResize(cursor): validate the window is still tiled at the same mon/ws/index with the same count and the workspace is still active and not monocle — otherwise EndMouseResize() and return. dx/dy = cursor - start cursor; skip if equal to the last applied delta. newRect = start slot with the chosen left-or-right edge += dx and top-or-bottom edge += dy, then keep each moved edge >= 50 px from its opposite edge. params = LayoutParamsOf(ws) but with params.ratios = start ratios copy. If ApplyResize(area, count, params, index, newRect) → ws.ratios = params.ratios, changed = true, Retile(mon). (Recomputing from the start ratios every tick keeps it stable.)
- EndMouseResize(): if changed, HT_LOG(L"mouse-resized window %p", (void*)hwnd); clear the session. MouseResizeActive(): session present.
- UnmanageWindow: if the session's window is unmanaged, clear the session (no log needed).

## Verify
- `powershell -NoProfile -File <repo>\build.ps1 -Check 50_model.cpp` → errors=0 warnings=0 (authoritative).
- Full `... build.ps1` (both clang lines) and `... build.ps1 -Test` (0 failed). If the full build fails only because of 60_engine.cpp / 40_config.cpp (the other agent, possibly mid-edit), say so and do not touch them.

## Report
Under 150 words: changes, verification results line by line, anything ambiguous.
````

## Report from the agent (2026-10-14 05:48 UTC)

````text
Implemented the Alt+Shift mouse resize model in src/50_model.cpp (only file edited).

Changes:
- Anonymous namespace: MouseResizeSession struct (hwnd, mon, ws, index, count, area, startRatios, startSlot, startCursor, leftEdge, topEdge, lastDx/lastDy, changed) and the instance gResize; MouseResizeStillValid() checks the window is still tiled at the same mon/ws/index with the same count, workspace active, not monocle.
- UnmanageWindow: clears gResize when its window is unmanaged (silent).
- BeginMouseResize, UpdateMouseResize, EndMouseResize, MouseResizeActive implemented per the contract, placed before ExecuteCommand. Update recomputes from startRatios each tick, skips an unchanged delta, and keeps moved edges >= 50 px from the opposite edge. EndMouseResize logs "mouse-resized window %p" only when changed.

Verification:
- build.ps1 -Check 50_model.cpp: clang exit=0 errors=0 warnings=0
- Full build.ps1 (both clang lines): exit=0 errors=0 warnings=0 each
- build.ps1 -Test: 1035 passed, 0 failed

Ambiguities: BeginMouseResize does not check g.paused (the contract does not list it, and the engine gates it). If the window is quietly moved to floating by a Retile failure, the session ends on the next update via the validity check.
````

## Follow-up from the planner (2026-10-14 10:49 UTC)

````text
Follow-up: the owner changed the Alt+Shift mouse-resize behaviour; please rework your 50_model.cpp implementation.

Re-read the "Alt+Shift mouse resize" section of src/10_contract.h (just above ExecuteCommand) — it was rewritten. Summary of what changed:
- The target is the ACTIVE window, CommandTarget(), not the window under the cursor. Edges are picked by the cursor position relative to that window's center (left of center → left edge, else right; above → top, else bottom); the cursor may be outside the window.
- Floating windows qualify too (e.g. toggled with togglefloating): start rect = GetVisibleFrame. Tiled: start rect = its slot as before. Minimized or maximized windows never qualify; the window must be on its monitor's active workspace.
- UpdateMouseResize only moves the active window live: PlaceWindow(hwnd, newRect). No ratios, no Retile during the drag. Floating: also clamp newRect inside the monitor's rcWork (keep size >= 50 px).
- EndMouseResize does the layout once: for a tiled window whose rect changed, ApplyResize from the RECORDED start ratios with the last newRect; if changed → ws.ratios = params.ratios; then Retile(mon) either way. Floating: nothing more. Log "mouse-resized window %p" once when something changed.
- Keep: validity check (end without applying when the window moved/was unmanaged), skip unchanged deltas, UnmanageWindow clears the session, MouseResizeActive.

Edit ONLY src/50_model.cpp; the engine side is unchanged. Clean-room rule as before. Never run the mod or hawktile-dev.exe; `build.ps1 -Test` IS allowed. Verify: `build.ps1 -Check 50_model.cpp`, full `build.ps1` (both clang lines), `build.ps1 -Test`. Report under 150 words.

Address this before completing your current task.
````

## Report from the agent (2026-10-14 10:55 UTC)

````text
Reworked the Alt+Shift mouse resize in src/50_model.cpp to the new contract (only file touched).

Changes:
- Target is CommandTarget(), tiled or floating; minimized/maximized and non-active-workspace windows are rejected. Tiled rules (non-monocle, >= 2 windows, index in LayoutWindowsOf) kept. Floating start rect = GetVisibleFrame.
- Edges chosen by cursor vs. the start rect's center.
- UpdateMouseResize: validity check (ends without applying), skip unchanged deltas, 50 px minimum per moved edge, floating edges clamped to rcWork, then PlaceWindow only. No ratios, no Retile during the drag.
- EndMouseResize: validity check; tiled with a changed rect runs ApplyResize from the recorded start ratios with the last rect, writes ws.ratios on change; Retile(mon) for tiled sessions; floating does nothing more. Logs "mouse-resized window %p" once when something changed.
- UnmanageWindow clear and MouseResizeActive unchanged.

Verification:
- build.ps1 -Check 50_model.cpp: errors=0 warnings=0
- full build.ps1: clang exit=0 errors=0 warnings=0 (both lines)
- build.ps1 -Test: 1035 passed, 0 failed

Ambiguity: the contract says "moved" for the validity check; I read it as list membership or state change, not the rect, since the active window moves live during the drag.
````
