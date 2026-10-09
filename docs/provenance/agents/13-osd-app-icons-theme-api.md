# 13. OSD app icons + Theme API

| | |
|---|---|
| Launched | 2026-10-09 07:29 UTC |
| Role | implementer |
| Model | Claude Haiku 5.5 |
| Agent id | `a83ad1470dc3cb6b8` |

## Instructions from the planner (verbatim)

````text
You are modifying one fragment of HawkTile, a clean-room tiling window manager written as a Windhawk mod (C++23, clang, mingw target). Project root: <repo>

## Ground rules
- Read `src/10_contract.h` — the "Theme + workspace indicator / OSD (55_osd.cpp)" section changed: there is now a public `struct Theme` and `Theme ResolveTheme()` you must provide (another agent's new fragment 56_bar.cpp calls it), and the window list under each workspace becomes APP ICONS. Then read `src/55_osd.cpp` fully. Edit ONLY `src/55_osd.cpp`.
- Fragments are concatenated into one translation unit; the anonymous namespace inside `ht` is shared by all fragments, so file-local helpers need distinctive names (prefix `Osd`). System headers may be #included at the top of the fragment (shell32, gdi32, shcore, advapi32, dwmapi, version, gdiplus are linked).
- Log with `HT_LOG(L"...", ...)` only (string-literal format); no per-show logging.
- Clean room: do NOT look at or reproduce code from komorebi, GlazeWM, Hyprland, i3, bspwm or any other window manager.
- Verify: `powershell -NoProfile -File <repo>\build.ps1 -Check 55_osd.cpp` must end with errors=0 warnings=0. Do NOT run the full build (other fragments are being changed in parallel). NEVER run the mod or anything that creates or touches real windows.
- Final report under 200 words.

## Changes
1. Theme: delete the file-local OsdTheme struct and turn OsdResolveTheme into the public `Theme ResolveTheme()` declared in the contract (same resolution logic and colors as today; set `highContrast` true only in the high-contrast branch). Use `Theme` everywhere in the OSD.
2. Replace the text labels under each workspace with app icons:
   - Per workspace: iterate tiled then floating HWNDs; key = lowercase full exe path (QueryFullProcessImageNameW via OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION)); if the path can't be read, key = a unique per-window string so it is its own group. Group by key in first-seen order; keep the first HWND as representative and a count.
   - `HICON OsdIconFor(HWND hwnd, const std::wstring& exePath)`: try SendMessageTimeoutW(hwnd, WM_GETICON, ICON_BIG, 0, SMTO_ABORTIFHUNG | SMTO_BLOCK, 50, &result), then ICON_SMALL2, then ICON_SMALL; then GetClassLongPtrW(hwnd, GCLP_HICON), then GCLP_HICONSM; then the exe's own icon via ExtractIconExW(path, 0, &large, nullptr, 1) — cache these in a file-local map path → HICON because they are OWNED and must be DestroyIcon'ed in DestroyOsd (cap the cache at 128 entries; destroy what you evict); finally LoadIconW(nullptr, IDI_APPLICATION). Never destroy icons obtained from WM_GETICON / GetClassLongPtr / LoadIcon. Resolve icons on every show (apps change icons).
   - Remove the old FileDescription/label code and its cache (anything left unused would warn).
3. Layout (96-dpi units, scaled with the monitor DPI as now): badge row unchanged (diameter 30). Icon size 24, gap 6 between icons, max 3 icons per row, max 3 rows (9 slots). If a workspace has more than 9 groups, show 8 icons and put "+N" (N = groups − 8) as small text in the 9th slot. Occupied column width = max(36, k*24 + (k−1)*6) with k = min(3, slots used); empty columns 36. The icon area starts 10 below the badge; each row is centered within its column. Panel height = pad + 30 + (maxRows > 0 ? 10 + maxRows*24 + (maxRows−1)*6 : 0) + pad. Keep the existing "shrink if wider than the work area" logic, but with a minimum occupied width of 36.
4. Paint: draw icons with DrawIconEx(hdc, x, y, icon, s24, s24, 0, nullptr, DI_NORMAL) on the double-buffer DC. For a group with count > 1 draw a small count badge: filled circle of diameter scale(13) whose center sits on the icon's bottom-right corner (inset by scale(2)), brush theme.activeFill, NULL_PEN, number in theme.activeText with a small bold font (-scale(9), "Segoe UI"), "9+" when count > 9. "+N" text uses theme.listText and the small font. Keep the rest (background, badges, fonts, timer, click-through, DestroyOsd cleanup) as it is; DestroyOsd must also destroy the cached exe icons and the extra font.

Run the -Check command until it reports errors=0 warnings=0.
````

## Report from the agent (2026-10-09 07:34 UTC)

````text
55_osd.cpp is updated (the only file I edited). `build.ps1 -Check 55_osd.cpp` reports errors=0 warnings=0 (clang -Wall -Wextra). I did not run the full build or the mod.

What changed:
- Theme: OsdTheme is gone. `Theme ResolveTheme()` is defined in `ht` outside the anonymous namespace. highContrast is set only in the high-contrast branch. The OSD uses `Theme` throughout.
- Icons: windows are grouped by lowercase exe path in first-seen order, with unreadable paths as per-window groups. OsdIconFor follows the specified order. Only exe icons are cached and owned: capped at 128, evicted entries are destroyed, and DestroyOsd destroys the rest. Window and class icons are never destroyed.
- Eviction safety: each show gets a stamp, and entries used by that show are never evicted, so no icon being painted gets destroyed.
- Layout and paint follow the spec: 3 per row, 3 rows, "+N" in slot 9 when there are more than 9 groups, count badges, and "9+".
- Removed: the label code, label cache, and `winver.h`. The 11dip list font is replaced by a 9dip bold `smallFont`, which is used for counts and "+N" and destroyed in DestroyOsd.

Caveats:
1. Per the exe-path key, all UWP windows under ApplicationFrameHost.exe collapse into one icon with a count. I dropped the old title-based special case.
2. The shrink logic can narrow occupied columns below the icon-row width (minimum 36, row up to 84). On a very narrow work area, icon rows can overlap neighbouring columns.
````
