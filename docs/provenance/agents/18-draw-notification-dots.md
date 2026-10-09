# 18. draw notification dots

| | |
|---|---|
| Launched | 2026-10-09 07:56 UTC |
| Role | implementer |
| Model | Claude Haiku 5.5 |
| Agent id | `a6e13cd2b967757fd` |

## Instructions from the planner (verbatim)

````text
You are modifying two fragments of HawkTile, a clean-room tiling window manager written as a Windhawk mod (C++23, clang, mingw target). Project root: <repo>

## Ground rules
- Read `src/10_contract.h` — especially the "Notification dot" paragraph in the taskbar indicator section and `WorkspaceHasAttention(int mon, int wsIndex)` (implemented in parallel by another agent in 50_model.cpp; just call it). Then read `src/55_osd.cpp` and `src/56_bar.cpp` fully.
- Edit ONLY `src/55_osd.cpp` and `src/56_bar.cpp`; minimal changes in the existing style. New helpers keep the file's prefix (`Osd` / `Bar`) because the anonymous namespace is shared across fragments.
- Clean room: do NOT look at or reproduce code from komorebi, GlazeWM, Hyprland, i3, bspwm or any other window manager or bar.
- Verify: `powershell -NoProfile -File <repo>\build.ps1 -Check 55_osd.cpp` and `-Check 56_bar.cpp` → errors=0 warnings=0 each. Do NOT run the full build (another agent edits other fragments in parallel). NEVER run the mod or anything that creates or touches real windows.
- Final report under 150 words.

## The dot (iOS-style notification badge)
For every workspace i that is NOT the monitor's active workspace and `WorkspaceHasAttention(mon, i)` is true, draw a small dot on its badge:
- Center on the badge circle at 45° toward the top-right: cx + r·0.7071, cy − r·0.7071 (r = badge radius).
- Diameter scale(8) on the taskbar bar (badge 22) and scale(10) on the OSD (badge 30); first a ring: a circle 2·scale(1.5) px larger (round to whole pixels, at least 2 px larger) filled with theme.background, then the dot filled with theme.activeFill (same color as the active badge). Never draw it on the active workspace.

## 56_bar.cpp
- Add an `attention` vector<bool> to BarLayout, filled in BarComputeLayout via WorkspaceHasAttention(mon index, i) (false for the active index).
- Include the attention bits and theme.background in BarSignature so the bar re-renders when they change.
- Draw with GDI+ (antialiased FillEllipse, alpha 255) after the badge and its number. The dot may stick out of the badge by a pixel or two; the existing padding (4 dip) covers it — if your math shows it would be clipped, increase the top/right padding accordingly and keep the badge row's vertical centering.
- The click hit-test stays on the badge circles.

## 55_osd.cpp
- Draw the dot with GDI (Ellipse with NULL_PEN and solid brushes; delete the brushes) after each badge's number, using the monitor index the OSD is showing.

Run both -Check commands until clean.
````

## Report from the agent (2026-10-09 07:58 UTC)

````text
Notification dot added to the taskbar bar and the OSD. Both -Check runs are clean (55_osd.cpp: errors=0 warnings=0; 56_bar.cpp: errors=0 warnings=0). Only src/55_osd.cpp and src/56_bar.cpp were edited; no full build, no mod run.

56_bar.cpp:
- BarLayout gets `attention`, filled in BarComputeLayout via WorkspaceHasAttention(tb.mon, i), false for the active index.
- BarSignature now includes the attention bits and theme.background.
- BarDrawDot (GDI+, alpha 255) draws the background ring then the activeFill dot, after each badge's number. Dot is scale(8), ring is 2*scale(1.5) wider, min 2 px.
- Padding: at 96 dpi the ring reaches about 2.3 px past the badge top and right edges, leaving about 1.7 px inside the 4 dip padding. Not clipped at 100/125/150/200%, so padding is unchanged. Hit-test is untouched.

55_osd.cpp:
- OsdColumn gets `attention`, set in ShowWorkspaceOsd via WorkspaceHasAttention(mon, i) with i != osd.active.
- OsdPaintDot (GDI, NULL_PEN, brushes deleted) draws after each number. Dot is scale(10), ring is 2*scale(1.5) wider, min 2 px.
- Ring stays inside the 3 dip column margin, so nothing is clipped.
````
