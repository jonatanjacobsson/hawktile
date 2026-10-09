# 10. OSD redesign with window lists

| | |
|---|---|
| Launched | 2026-10-09 06:52 UTC |
| Role | implementer |
| Model | Claude Haiku 5.5 |
| Agent id | `a24aa4c9d3e740d97` |

## Instructions from the planner (verbatim)

````text
You are modifying one fragment of HawkTile, a clean-room tiling window manager written as a Windhawk mod (C++23, clang, mingw target). Project root: <repo>

## Ground rules
- Read `src/10_contract.h` (the "Workspace indicator / OSD" section was just rewritten — it is the spec) and `src/55_osd.cpp` fully. Edit ONLY `src/55_osd.cpp`. Keep what still fits (window class, layered/topmost/click-through window, timer, DPI handling, OsdResolveTheme, DestroyOsd); rework layout and painting.
- Fragments are concatenated into one translation unit; the anonymous namespace inside `ht` is shared by all fragments, so helpers need distinctive names (prefix `Osd`). You may #include system headers at the top of the fragment (e.g. <unordered_map>, <winver.h>). version.lib, gdi32, shcore, advapi32, dwmapi are linked.
- Log with `HT_LOG(L"...", ...)` only (string-literal format). Do not log per show.
- Clean room: do NOT look at or reproduce code from komorebi, GlazeWM, Hyprland, i3, bspwm or any other window manager.
- Verify: `powershell -NoProfile -File <repo>\build.ps1 -Check 55_osd.cpp` must end with `errors=0 warnings=0`; then the full `powershell -NoProfile -File <repo>\build.ps1` must end with errors=0 warnings=0 on both passes.
- NEVER run the mod, the standalone exe, or anything that creates or touches real windows. Compile checks only.
- Final report under 200 words.

## What to build

### 1. Theme
Extend OsdTheme with `listText` (secondary small text) and `activeListText` (small text under the active workspace). Change colors so occupied vs empty is clearly different:
- Dark: background RGB(32,32,32); activeFill = accent Light2 (as now); activeText RGB(0,0,0); occupiedFill RGB(72,72,72); occupiedText RGB(255,255,255); emptyFill RGB(22,22,22); emptyText RGB(130,130,130); listText RGB(170,170,170); activeListText RGB(235,235,235).
- Light: background RGB(243,243,243); activeFill = accent Dark1 (as now); activeText RGB(255,255,255); occupiedFill RGB(205,205,205); occupiedText RGB(20,20,20); emptyFill RGB(252,252,252); emptyText RGB(150,150,150); listText RGB(90,90,90); activeListText RGB(25,25,25).
- High contrast: keep current mapping; listText = activeListText = GetSysColor(COLOR_WINDOWTEXT).

### 2. Window labels (computed in ShowWorkspaceOsd)
For each workspace of g.monitors[mon]: iterate `tiled` then `floating` HWNDs; label each with `std::wstring OsdAppLabel(HWND)`:
- pid via GetWindowThreadProcessId → OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION) → QueryFullProcessImageNameW (buffer 1024) → full exe path.
- If the exe file name is `ApplicationFrameHost.exe` (case-insensitive) → use the window title (GetWindowTextW) instead; if empty, "App".
- Otherwise use the exe's FileDescription: GetFileVersionInfoSizeW / GetFileVersionInfoW / VerQueryValueW(L"\\VarFileInfo\\Translation") → first (lang, codepage) pair → VerQueryValueW(L"\\StringFileInfo\\%04x%04x\\FileDescription"). Trim it. If missing/empty → exe file name without extension.
- Cache path → description in a file-local `std::unordered_map<std::wstring, std::wstring>` (version-info reads are slow); cap the cache at 256 entries (clear when exceeded).
- Group duplicates per workspace preserving first-seen order: a label that occurs k > 1 times becomes `Label ×k` (use L"\u00D7").
- At most 6 lines per column: if there are more than 6 groups, show the first 5 and then `+N more` where N = remaining groups.

### 3. Layout (96-dpi units, scale with the monitor DPI as now)
- Outer padding 14. Gap between columns 6.
- Column width: 108 if the workspace has windows, else 36 (just the badge).
- Badge: circle of diameter 30, horizontally centered in its column, top at padding.
- List: starts 8 below the badge; line height 15; small font. Rows = max lines across columns (0 if no window anywhere → no list area at all).
- Width = 2*pad + sum(column widths) + gaps; height = 2*pad + 30 + (rows > 0 ? 8 + rows*15 : 0).
- If width exceeds the monitor work-area width minus 2*24, shrink the wide (108) columns proportionally down to a minimum of 64.
- Window centered horizontally in rcWork, vertically centered in rcWork.

### 4. Paint (double-buffered as now; delete every GDI object)
- Background fill theme.background (the window corners are rounded by DWM; leave that as is).
- Badge (Ellipse with NULL_PEN and a solid brush):
  - active workspace: brush activeFill, number in activeText, BOLD font.
  - occupied (not active): brush occupiedFill, number in occupiedText, BOLD font.
  - empty (not active): brush emptyFill, number in emptyText, NORMAL weight font.
  Number = index + 1, DT_CENTER | DT_VCENTER | DT_SINGLELINE inside the circle rect.
- Fonts: create on show (DPI can differ per monitor) and delete the previous set: number bold = CreateFontW(-scale(15), ..., FW_BOLD, ..., CLEARTYPE_QUALITY, ..., L"Segoe UI"); number normal = same with FW_NORMAL; list = CreateFontW(-scale(11), ..., FW_NORMAL, ..., L"Segoe UI").
- List lines: small font, color activeListText for the active workspace's column else listText; each line in its own rect (column x-range inset by 3, height 15), DT_CENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX | DT_VCENTER.
- SetBkMode TRANSPARENT.

Everything else (show/hide timer, click-through, never activating, DestroyOsd cleanup — now including the extra fonts and clearing the label cache) stays as it is. Run both checks until they pass with zero warnings.
````

## Report from the agent (2026-10-09 06:56 UTC)

````text
Rewrote src/55_osd.cpp only. The public API (ShowWorkspaceOsd, DestroyOsd) and 10_contract.h are unchanged.

Checks: `build.ps1 -Check 55_osd.cpp` and the full build (both passes) each end errors=0 warnings=0. I ran no mod, exe or window code.

Implemented per the brief: the new theme colors with listText and activeListText; app labels with an exe-path cache (capped at 256, cleared in DestroyOsd); grouping as "Name ×k" with "+N more" past 6 groups; the 96-dpi layout with proportional shrink to 64; badges and list painted as specified; three fonts rebuilt on every show; all GDI objects deleted.

Interpretations to check:
- If a window's exe path cannot be read (for example an elevated process), the label is the window title, or "App" if that is empty. The brief did not cover this case.
- The shrink works in physical pixels. Each occupied column gives up a share of the cut proportional to its slack, so totals can be off by about a pixel.

Contract mismatch, not edited because it is outside this fragment: 10_contract.h lines 468-471 say occupied circles are lighter and empty ones darker. With the light-theme colors from the brief this is inverted (occupied 205 is darker than empty 252). The dark theme still matches. Reword that comment, or revisit the light colors.
````
