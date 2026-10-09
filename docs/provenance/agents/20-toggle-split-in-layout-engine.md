# 20. toggle-split in layout engine

| | |
|---|---|
| Launched | 2026-10-09 08:24 UTC |
| Role | implementer |
| Model | Claude Haiku 5.5 |
| Agent id | `ad05488633b380543` |

## Instructions from the planner (verbatim)

````text
You are modifying the layout fragment of HawkTile, a clean-room tiling window manager written as a Windhawk mod (C++23, clang, mingw target). Project root: <repo>

## Ground rules
- Read the "Layout (20_layout.cpp)" section of `src/10_contract.h` — new: `LayoutParams::flips` (Dwindle-only orientation inversion per split) and `bool ToggleSplitAt(LayoutParams& p, int count, int index)`. Then read `src/20_layout.cpp` and `tests/layout_test.cpp` fully.
- Edit ONLY `src/20_layout.cpp` and `tests/layout_test.cpp`. Pure functions, no window APIs.
- Clean room: do NOT look at or reproduce code from komorebi, GlazeWM, Hyprland, i3, bspwm or any other window manager.
- Verify: `powershell -NoProfile -File <repo>\build.ps1 -Test` (0 warnings, 0 failed) and `... build.ps1 -Check 20_layout.cpp` (errors=0 warnings=0). Do NOT run the full build (another agent edits 40_config.cpp / 50_model.cpp in parallel).
- Final report under 120 words.

## Implementation
1. In the shared split walker used by ComputeLayout and ApplyResize: for Dwindle, after choosing the automatic orientation of split i, invert it when `i < p.flips.size() && p.flips[i] != 0`. Other layouts ignore flips. ApplyResize must keep working with flips (it uses the same walker).
2. `ToggleSplitAt`: per contract — Dwindle only; target split = index < count-1 ? index : count-2; pad p.flips with 0 to count-1; toggle that entry (0 ↔ 1); return true. Return false (p unchanged) for other layouts, count < 2, or index outside [0, count).
3. Tests (keep all existing ones passing): Dwindle n=2 on {0,0,1920,1080} gap 0: default vertical split ({0,0,960,1080},{960,0,1920,1080}); after ToggleSplitAt(p,2,0) horizontal ({0,0,1920,540},{0,540,1920,1080}); toggling again restores vertical. Dwindle n=3: toggling window 2 flips split 1 (window 1/2 become side-by-side instead of stacked), split 0 untouched; toggling window 1 flips the same split 1. Non-dwindle (Columns, MasterStack, Monocle) → false and p.flips untouched. count 1 → false. Invariant tests (count, inside area, bounding box, no overlap, gaps) also run with a flips vector such as {1,0,1}. ApplyResize on a flipped Dwindle split moves the right edge (check one case).

Run both commands until clean.
````

## Report from the agent (2026-10-09 08:26 UTC)

````text
Dwindle split flips and ToggleSplitAt are implemented in src/20_layout.cpp, with tests in tests/layout_test.cpp. No other files were touched.

- IsVertical now takes LayoutParams. Dwindle inverts its orientation when flips[i] != 0. ComputeLayout and ApplyResize share WalkSplits, so both use it.
- ToggleSplitAt: Dwindle only, count >= 2, index in range. It targets split index (or count-2 for the last window), pads flips to count-1, and toggles 0/1.
- Tests: all cases from the brief. The invariant and gap-spacing loops also run with flips {1,0,1}. Two flipped ApplyResize cases: a vertical split's right edge, and a horizontal split's bottom edge.

build.ps1 -Test: 1035 passed, 0 failed, 0 warnings (baseline was 588 passed). build.ps1 -Check 20_layout.cpp: errors=0 warnings=0. The full build was not run.
````
