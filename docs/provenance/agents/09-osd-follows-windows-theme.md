# 9. OSD follows Windows theme

| | |
|---|---|
| Launched | 2026-10-09 06:48 UTC |
| Role | implementer |
| Model | Claude Haiku 5.5 |
| Agent id | `a1fea1b6538021bc4` |

## Instructions from the planner (verbatim)

````text
You are modifying one fragment of HawkTile, a clean-room tiling window manager written as a Windhawk mod (C++23, clang, mingw target). Project root: <repo>

## Ground rules
- Read `src/10_contract.h` (the "Workspace indicator / OSD" section was just updated) and `src/55_osd.cpp` fully. Edit ONLY `src/55_osd.cpp`. Keep its structure; change only what is needed.
- Fragments are concatenated into one translation unit; the anonymous namespace inside `ht` is shared by all fragments, so new helpers need distinctive names (prefix `Osd`).
- Log with `HT_LOG(L"...", ...)` only (string-literal format). advapi32 is now linked (registry APIs are fine).
- Clean room: do NOT look at or reproduce code from komorebi, GlazeWM, Hyprland, i3, bspwm or any other window manager.
- Verify: `powershell -NoProfile -File <repo>\build.ps1 -Check 55_osd.cpp` must end with `errors=0 warnings=0`, and then the full `powershell -NoProfile -File <repo>\build.ps1` must end with errors=0 warnings=0 on both passes.
- NEVER run the mod, the standalone exe, or anything that creates or touches real windows.
- Final report under 150 words.

## Task: make the OSD follow the user's Windows colors

Today the OSD uses fixed dark colors and g.settings.focusBorderColor for the active cell. Replace that with a theme resolved on EVERY ShowWorkspaceOsd call (so theme/accent changes are picked up without restart). Add a small struct, e.g.

struct OsdTheme { COLORREF background, activeFill, activeText, occupiedFill, occupiedText, emptyFill, emptyText; };

and a function `OsdTheme OsdResolveTheme()` that works like this:

1. High contrast: `HIGHCONTRASTW hc{sizeof(hc)}; SystemParametersInfoW(SPI_GETHIGHCONTRAST, sizeof(hc), &hc, 0)`; if it succeeds and `hc.dwFlags & HCF_HIGHCONTRASTON`: background = GetSysColor(COLOR_WINDOW); activeFill = GetSysColor(COLOR_HIGHLIGHT); activeText = GetSysColor(COLOR_HIGHLIGHTTEXT); occupiedFill = background; occupiedText = GetSysColor(COLOR_WINDOWTEXT); emptyFill = background; emptyText = GetSysColor(COLOR_GRAYTEXT). Return.
2. Light or dark: read DWORD `SystemUsesLightTheme` from HKCU `Software\Microsoft\Windows\CurrentVersion\Themes\Personalize` with RegGetValueW(..., RRF_RT_REG_DWORD, ...). Missing/failed → dark (0). (The OSD is shell UI, so the *system* theme is used, not the apps theme.)
3. Accent palette: read REG_BINARY `AccentPalette` from HKCU `Software\Microsoft\Windows\CurrentVersion\Explorer\Accent` (RRF_RT_REG_BINARY) into a 32-byte buffer; it holds 8 colors of 4 bytes each in R,G,B,A byte order: [0] Light3, [1] Light2, [2] Light1, [3] Accent, [4] Dark1, [5] Dark2, [6] Dark3, [7] unused. Use the RGB bytes (ignore A) to build COLORREFs with RGB(r, g, b).
   If that read fails or returns fewer than 32 bytes: fallback base accent from DwmGetColorizationColor(&argb, &opaque) (0xAARRGGBB → RGB((argb>>16)&255, (argb>>8)&255, argb&255)); if that fails too use RGB(0, 120, 212). Derive Light2 = blend(base, white, 0.45) and Dark1 = blend(base, black, 0.20) with a small helper `COLORREF OsdBlend(COLORREF a, COLORREF b, double t)` (per-channel a + (b - a) * t, rounded).
4. Dark theme: background RGB(32, 32, 32); activeFill = Light2; activeText = RGB(0, 0, 0); occupiedFill = RGB(55, 55, 55); occupiedText = RGB(255, 255, 255); emptyFill = RGB(43, 43, 43); emptyText = RGB(125, 125, 125).
   Light theme: background RGB(243, 243, 243); activeFill = Dark1; activeText = RGB(255, 255, 255); occupiedFill = RGB(225, 225, 225); occupiedText = RGB(26, 26, 26); emptyFill = RGB(235, 235, 235); emptyText = RGB(150, 150, 150).
   (These mirror the Windows 11 convention: accent fills use the lighter accent shade with dark text in dark mode, the darker shade with white text in light mode.)
5. Store the resolved theme in the file-local OSD state when showing, and use it in WM_PAINT for the background, cell fills and text colors (replacing the hard-coded colors and the use of g.settings.focusBorderColor). Remove state that becomes unused so there are no warnings.

Also log once per show at most at debug level? No — do not add per-show logging (it would spam the log). You may log once if reading the palette falls back.

Run the checks until both pass with zero warnings.
````

## Report from the agent (2026-10-09 06:49 UTC)

````text
The OSD now takes its colors from the Windows theme, resolved on every ShowWorkspaceOsd call. Only <repo>\src\55_osd.cpp was edited.

Changes:
- Added OsdTheme, OsdBlend and OsdResolveTheme. Resolution order: high contrast, then SystemUsesLightTheme (missing means dark), then AccentPalette entries 1 (Light2) and 4 (Dark1), falling back to DwmGetColorizationColor (or RGB(0,120,212)) with blended shades. Colors match the spec.
- OsdState holds `theme` instead of `accent`. The OSD no longer reads g.settings.focusBorderColor.
- OsdPaint uses the theme for background, cell fills and text. The six hard-coded color constants are removed.
- Added #include <cmath> for std::lround.
- No palette-fallback log (optional; skipped).

Verification: build.ps1 -Check 55_osd.cpp gives errors=0 warnings=0. The full build gives errors=0 warnings=0 on both passes. I did not run the mod or the standalone exe, and the colors are not checked on a real desktop.
````
