# 12. model support for resizing

| | |
|---|---|
| Launched | 2026-10-09 06:57 UTC |
| Role | implementer |
| Model | Claude Haiku 5.5 |
| Agent id | `a230225487e1e90f7` |

## Instructions from the planner (verbatim)

````text
You are modifying the model fragment of HawkTile, a clean-room tiling window manager written as a Windhawk mod (C++23, clang, mingw target). Project root: <repo>

## Ground rules
- Read `src/10_contract.h` fully. Recent changes: LayoutParams gained `std::vector<double> ratios` (last member; 0.0 or any value outside (0,1) means "default"); Workspace lost `double ratio` and gained `std::vector<double> ratios`; new layout functions SplitRatio and ApplyResize (implemented in parallel by another agent in 20_layout.cpp — just call them per their contract); OnUserDragEnd's contract now covers border resizing. Then read `src/50_model.cpp` fully.
- Edit ONLY `src/50_model.cpp`.
- The anonymous namespace inside `ht` is shared by all fragments in the joined file, so new helper names must be distinctive (e.g. prefix `Layout...Of`).
- Log with `HT_LOG(L"...", ...)` only (string-literal format).
- Clean room: do NOT look at or reproduce code from komorebi, GlazeWM, Hyprland, i3, bspwm or any other window manager.
- Verify with `powershell -NoProfile -File <repo>\build.ps1 -Check 50_model.cpp` → errors=0 warnings=0. Do NOT run the full build (20_layout.cpp is being rewritten in parallel and may not link yet). NEVER run the mod or anything touching real windows.
- Final report under 200 words.

## Changes
1. MakeWorkspace: drop the `ratio` initialisation (ratios start empty).
2. Add three file-local helpers and use them in Retile so geometry is computed in exactly one way:
   - `std::vector<HWND> LayoutWindowsOf(const Workspace& ws)` — the tiled windows Retile lays out today (not in g.hiddenByUs, IsWindow, !IsIconic, !IsZoomed), in order.
   - `RECT LayoutAreaOf(int mon)` — rcWork shrunk by outerGap on all sides, falling back to rcWork if width or height would drop below 50 (today's logic).
   - `LayoutParams LayoutParamsOf(const Workspace& ws)` — `{ws.layout, g.settings.masterRatio, g.settings.innerGap, ws.ratios}`.
3. Grow / Shrink: n = max(2, LayoutWindowsOf(ws).size()); cur = SplitRatio(LayoutParamsOf(ws), n, 0); next = clamp(cur ± g.settings.ratioStep, 0.1, 0.9); pad ws.ratios with 0.0 to at least size 1; ws.ratios[0] = next; Retile(mon).
4. Layout changes: in the helper used by cycle-layout / set-layout (SetWorkspaceLayout), when the kind actually changes and neither the old nor the new kind is Monocle, keep only ratios[0] (resize to min(size, 1)) — split meanings differ between layouts except the first. toggle-monocle must NOT touch ratios (so returning from monocle restores the sizes).
5. OnUserDragEnd: implement the updated contract. Keep the existing cross-monitor branch first (floating → membership only; tiled → MoveWindowToMonitor). Then, for a tiled window whose workspace layout is not Monocle:
   - list = LayoutWindowsOf(ws); idx = position of hwnd in list; if idx < 0 → RequestRetile(); return.
   - area = LayoutAreaOf(loc.mon); params = LayoutParamsOf(ws); slots = ComputeLayout(area, list.size(), params); frame = GetVisibleFrame(hwnd).
   - resized = |frame width − slot width| > 2 || |frame height − slot height| > 2.
   - If resized: if ApplyResize(area, list.size(), params, idx, frame) → ws.ratios = params.ratios and HT_LOG the split ratios that changed (or simply "resized window %p"); RequestRetile(); return.
   - Else (moved): the existing swap-on-drop logic (center inside another tiled window's visible frame → swap in ws.tiled).
   - Always RequestRetile() at the end, as today.
   Monocle or floating windows: unchanged behaviour (just RequestRetile()).
6. Remove every remaining use of `ws.ratio`.

Run the -Check command until it reports errors=0 warnings=0.
````

## Report from the agent (2026-10-09 06:59 UTC)

````text
Model fragment updated; only src/50_model.cpp was edited. `build.ps1 -Check 50_model.cpp` reports errors=0 warnings=0. No full build, no run.

Done:
1. MakeWorkspace no longer sets ratio.
2. Added LayoutWindowsOf, LayoutAreaOf, LayoutParamsOf in the anonymous namespace. Retile now uses them and has no inline geometry.
3. Grow/Shrink: n = max(2, count), cur = SplitRatio(params, n, 0), next = clamp(cur +/- ratioStep, 0.1, 0.9) into ws.ratios[0] (padded to size 1).
4. SetWorkspaceLayout trims ws.ratios to min(size, 1) when the kind changes and neither side is Monocle. toggle-monocle is untouched.
5. OnUserDragEnd: cross-monitor branch unchanged. Tiled, non-monocle windows that are in the layout list get the resize-or-swap split: resized calls ApplyResize, stores ratios and logs changed splits; moved runs the old swap.
6. No ws.ratio references remain in src/.

Points to check:
- Monocle tiled windows no longer swap on drop. The contract limits the swap to non-monocle layouts, and the old code did swap there. This is a behaviour change.
- The swap scans all ws.tiled, as the old code did, not only the filtered layout list.
- Helper names have no collisions in the other fragments as they stand.
- ApplyResize and SplitRatio are not verified here; they live in 20_layout.cpp.
````
