# 1. layout engine + tests

| | |
|---|---|
| Launched | 2026-10-09 06:17 UTC |
| Role | implementer |
| Model | Claude Haiku 5.5 |
| Agent id | `ae25e5156536488dd` |

## Instructions from the planner (verbatim)

````text
You are implementing one fragment of HawkTile, a clean-room tiling window manager written as a Windhawk mod (C++23, clang, mingw target). Project root: <repo>

## Ground rules (apply to every fragment)
- First read `src/10_contract.h` completely. It is the binding contract: types, globals and every cross-module function signature. Implement exactly those signatures. Do NOT edit 10_contract.h or any file other than the ones assigned to you. If you believe the contract is wrong, implement it as written and mention the problem in your final report.
- Fragments are concatenated into ONE translation unit after 10_contract.h, so do not #include project files. The contract already includes <windows.h>, <dwmapi.h>, <shellapi.h>, <algorithm>, <cstdio>, <cwchar>, <cwctype>, <string>, <unordered_set>, <vector>. You may add another standard-library #include at the top of your fragment if truly needed.
- Put all code inside `namespace ht { ... }`. File-local helpers go in an anonymous namespace inside `ht`.
- Log with `HT_LOG(L"format %d", x)` only (the format must be a string literal).
- Clean room: do NOT look at, search for, or reproduce code from komorebi, GlazeWM, Hyprland, i3, bspwm or any other window manager. Use only your own knowledge of Win32 / geometry.
- Style: 4-space indent, `//` comments only where something is non-obvious, Chromium-like formatting, no exceptions, no RTTI tricks.
- Verify with: `powershell -NoProfile -File <repo>\build.ps1 -Check <your-fragment-file-name>` — it must end with `errors=0 warnings=0`. Fix every warning.
- Never run anything that moves, hides or focuses real windows on this machine.
- Final report (keep it under 200 words): what you implemented, any deviation from the contract, any concerns.

## Your assignment: `src/20_layout.cpp` and `tests/layout_test.cpp`

Implement in `src/20_layout.cpp` (pure functions, no Win32 calls besides using RECT):
`ComputeLayout`, `FindNeighbor`, `NextLayout`, `LayoutName`, `ParseLayoutName` — semantics are documented in the contract. Extra precision:

Gap algorithm (use exactly this so outer edges align with `area` and neighbours are exactly innerGap apart, also for odd gaps):
- g = max(0, innerGap); a = g / 2; b = g - a.
- Expanded = { area.left - a, area.top - a, area.right + b, area.bottom + b }.
- Partition Expanded into raw slots with integer math where adjacent slots share their edge exactly (the last slot in any split takes the remainder, so there are never 1px holes). A split at ratio r of an extent [lo, hi) puts the cut at lo + (int)lround((hi - lo) * r).
- Each final rect = { slot.left + a, slot.top + a, slot.right - b, slot.bottom - b }, then clamp so right >= left + 1 and bottom >= top + 1.
- Monocle ignores gaps: every rect == area.
- masterRatio is clamped to [0.05, 0.95] before use.
- Dwindle: remaining = Expanded; for i in 0..count-2: r = (i == 0 ? masterRatio : 0.5); if remaining width > remaining height split vertically (window i gets the left part) else horizontally (window i gets the top part); remaining = the other part. Window count-1 gets remaining.
- MasterStack: count == 1 → whole Expanded. Otherwise master = left part at masterRatio; right part divided into count-1 equal-height slots (remainder distributed so the bottom edge is exact).
- Columns / Rows: equal slots, last takes remainder.

FindNeighbor: implement the ranking described in the contract. "Overlaps perpendicular extent" means for Left/Right the candidate's [top,bottom) overlaps from's [top,bottom); for Up/Down the [left,right) ranges overlap. Return -1 when from is out of range or nothing qualifies.

LayoutName returns "dwindle", "master", "columns", "rows", "monocle" (and "unknown" for anything else). ParseLayoutName is case-insensitive and also accepts "master-stack" and "bsp" as aliases for master and dwindle.

Write `tests/layout_test.cpp`: it is concatenated AFTER 10_contract.h and 20_layout.cpp (so it can use ht:: functions directly, no #includes of project files) and must define `int main()`. Use a tiny CHECK macro that prints file:line and the failed expression with fwprintf(stderr, ...) and counts failures; print "N passed, M failed" at the end and return 1 if any failed. Cover at least:
- For every layout and count 0..7 on area {0,0,1920,1080} with innerGap 0, 8 and 7: result size == count; every rect inside area; bounding box of all rects == area (count >= 1); no two rects overlap (except Monocle, where all equal area).
- For Dwindle/MasterStack/Columns/Rows with gap g: for two rects that are horizontally adjacent (one's right < other's left, vertical ranges overlap) the closest pair distance is exactly g — check at least Columns n=3 and MasterStack n=3 explicitly with expected coordinates you compute by hand.
- Dwindle n=3, area {0,0,1920,1080}, gap 0, ratio 0.5: expect {0,0,960,1080}, {960,0,1920,540}, {960,540,1920,1080}.
- MasterStack n=3, ratio 0.6, gap 0: {0,0,1152,1080}, {1152,0,1920,540}, {1152,540,1920,1080}.
- FindNeighbor on a 2x2 grid of quadrants (indices 0=TL,1=TR,2=BL,3=BR): from 0 Right→1, Down→2, Left→-1, Up→-1; from 3 Left→2, Up→1. Plus a case where overlap preference matters (a tall left window and two stacked right windows: from the bottom-right window, Left must return the tall left window).
- NextLayout cycles through all five and back; ParseLayoutName(LayoutName(k)) round-trips; aliases work; garbage returns false.

Run `powershell -NoProfile -File <repo>\build.ps1 -Test` until it compiles with 0 warnings and all tests pass. Also run `build.ps1 -Check 20_layout.cpp` (0 errors, 0 warnings). If a test fails, fix the implementation, not the expectation, unless the expectation contradicts the spec above.
````

## Report from the agent (2026-10-09 06:26 UTC)

````text
Layout fragment and tests are done. No contract deviations; I added `#include <cmath>` at the top of the fragment, which the brief allows.

Files:
- <repo>\src\20_layout.cpp: ComputeLayout, FindNeighbor, NextLayout, LayoutName, ParseLayoutName, following the gap algorithm and ranking in the brief.
- <repo>\tests\layout_test.cpp: 273 checks, all passing. They cover invariants for every layout, counts 0..7 and gaps 0/7/8; neighbour spacing; hand-computed Dwindle, MasterStack, Columns and Rows cases (including odd gap 7); masterRatio clamp; FindNeighbor on a 2x2 grid plus the overlap-preference case; NextLayout cycling; name parsing and aliases.

Verification:
- `build.ps1 -Check 20_layout.cpp`: errors=0 warnings=0.
- `build.ps1 -Test`: compiles with errors=0 warnings=0, but the exe exits with 0xC0000135 (DLL not found) when run on this machine. The Windhawk x86_64 toolchain imports libc++ and libunwind as `libc++.whl` and `libunwind.whl` (see `x86_64-w64-mingw32\lib\libc++.dll.a`), which only exist inside Windhawk.
- I ran the same exe from the scratchpad with renamed copies of those DLLs placed beside it: 273 passed, 0 failed.

build.ps1 is not mine, so I did not change it. A fix belongs there, for example copying the runtime into dist under those names. I have not tested that or a static-link alternative.

Concern: failure lines report `dist\layout_test_all.cpp` line numbers, because that is the concatenated file.
````
