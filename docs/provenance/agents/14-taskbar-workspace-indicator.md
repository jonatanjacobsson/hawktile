# 14. taskbar workspace indicator

| | |
|---|---|
| Launched | 2026-10-09 07:29 UTC |
| Role | implementer |
| Model | Claude Haiku 5.5 |
| Agent id | `a88f44d11215fddff` |

## Instructions from the planner (verbatim)

````text
You are implementing a NEW fragment of HawkTile, a clean-room tiling window manager written as a Windhawk mod (C++23, clang, mingw target). Project root: <repo>

## Ground rules
- Read `src/10_contract.h` completely — your API is the "Taskbar workspace indicator (56_bar.cpp)" section (RequestBarUpdate, UpdateTaskbarIndicators, DestroyTaskbarIndicators). You may use `ResolveTheme()` / `Theme` (implemented by another agent in 55_osd.cpp, declared in the contract), `SwitchWorkspace`, `g`, `kTimerBar`, `HT_LOG`. Skim `src/55_osd.cpp` and `src/50_model.cpp` for conventions. Create ONLY `src/56_bar.cpp`.
- Fragments are concatenated into one translation unit (order ... 55_osd.cpp, 56_bar.cpp, 60_engine.cpp ...). Don't #include project files; you may #include system headers at the top (e.g. <gdiplus.h>, <shellscalingapi.h>). gdiplus, gdi32, shcore, dwmapi are linked. All code inside `namespace ht { ... }`; file-local state/helpers in an anonymous namespace inside `ht` — that namespace is SHARED with other fragments, so prefix every helper and static with `Bar`.
- Log with `HT_LOG(L"...", ...)` only (string-literal format). No logging on every update (it runs every second).
- Clean room: do NOT look at or reproduce code from komorebi, GlazeWM, Hyprland, i3, bspwm, yasb or any other window manager / bar.
- Everything runs on the engine thread (per-monitor-DPI-aware v2 → physical pixels).
- Verify: `powershell -NoProfile -File <repo>\build.ps1 -Check 56_bar.cpp` must end with errors=0 warnings=0. Do NOT run the full build (other fragments change in parallel). NEVER run the mod or anything that creates or touches real windows.
- Final report under 200 words.

## Spec
State: `struct BarWindow { HWND hwnd; HMONITOR monitor; int mon; std::vector<RECT> badges /*client coords*/; std::wstring signature; bool shown; };` `std::vector<BarWindow> s_barWindows;` plus class-registered flag, HINSTANCE (GetModuleHandleExW FROM_ADDRESS|UNCHANGED_REFCOUNT on your wndproc), GDI+ token (GdiplusStartup lazily on first UpdateTaskbarIndicators; GdiplusShutdown in DestroyTaskbarIndicators).

RequestBarUpdate(): `if (g.engineHwnd) SetTimer(g.engineHwnd, kTimerBar, 30, nullptr);`

UpdateTaskbarIndicators():
1. If `!g.settings.taskbarIndicator` or g.monitors is empty → hide every bar (ShowWindow SW_HIDE, shown=false) and return.
2. Find taskbars: EnumWindows, keep visible top-level windows whose class is exactly `Shell_TrayWnd` or `Shell_SecondaryTrayWnd`. For each: GetWindowRect → taskbar rect; HMONITOR via MonitorFromRect(DEFAULTTONEAREST); mon index = the g.monitors entry with that handle (skip if none). At most one bar per monitor.
3. Create a bar window for new monitors, destroy bars whose monitor no longer has a taskbar. Window: CreateWindowExW(WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE, kBarWindowClass, L"HawkTile taskbar", WS_POPUP, 0,0,0,0, nullptr, nullptr, hinst, nullptr). Class: lazily registered, hCursor = LoadCursor(nullptr, IDC_HAND).
4. Per bar decide visibility: visible part = IntersectRect(taskbar rect, monitor rcMonitor); hide if that is smaller than 8 px high or 100 px wide (auto-hidden taskbar). Hide if the foreground window (GetForegroundWindow) is on this monitor (MonitorFromWindow == bar's HMONITOR), is not a shell window (class not Progman / WorkerW / Shell_TrayWnd / Shell_SecondaryTrayWnd), and its GetWindowRect covers the whole rcMonitor (fullscreen app).
5. Geometry (96-dpi units scaled by GetDpiForMonitor(MDT_EFFECTIVE_DPI), fallback 96): badge diameter 22, gap 4, padding 4. N = that monitor's workspace count (1..9). width = 2*pad + N*22 + (N−1)*4, height = 2*pad + 22. x = taskbar.left + scale(g.settings.taskbarIndicatorOffset); y = visible.top + (visibleHeight − height) / 2. Badge rects in client coords.
6. Content signature: a wstring with x, y, w, h, dpi, N, active index, occupied bits (tiled or floating non-empty) and the theme colors (from ResolveTheme()). Only re-render when it differs from the bar's stored signature.
7. Render (per-pixel alpha): create a top-down 32-bpp DIB section (BITMAPINFOHEADER, biHeight = −height, BI_RGB) and wrap its bits in `Gdiplus::Bitmap bmp(width, height, width * 4, PixelFormat32bppPARGB, bits)`; `Gdiplus::Graphics gr(&bmp)`; SmoothingModeAntiAlias, TextRenderingHintAntiAliasGridFit; gr.Clear(Color(0,0,0,0)). For each workspace i: active → fill alpha 255 theme.activeFill, number theme.activeText, bold; occupied → fill alpha 235 theme.occupiedFill, number theme.occupiedText, bold; empty → fill alpha 150 theme.emptyFill, number theme.emptyText, regular. FillEllipse with a SolidBrush; number = i+1 drawn with Gdiplus::Font(L"Segoe UI", (REAL)scale(11), bold ? FontStyleBold : FontStyleRegular, UnitPixel) and a StringFormat centered both ways inside the badge RectF. COLORREF → Gdiplus::Color(a, GetRValue(c), GetGValue(c), GetBValue(c)). Then select the DIB into a memory DC and call UpdateLayeredWindow(hwnd, screenDC, &POINT{x,y}, &SIZE{w,h}, memDC, &POINT{0,0}, 0, &BLENDFUNCTION{AC_SRC_OVER, 0, 255, AC_SRC_ALPHA}, ULW_ALPHA). Free DCs/bitmap. (Fully transparent pixels are click-through by design, so only the badges take clicks.)
8. Show/raise every visible bar each call: if not shown → ShowWindow(SW_SHOWNOACTIVATE); always SetWindowPos(hwnd, HWND_TOPMOST, 0,0,0,0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE) so it stays above the taskbar after the user clicks the taskbar. Hidden bars: ShowWindow(SW_HIDE) only if currently shown.

Window procedure: WM_MOUSEACTIVATE → MA_NOACTIVATE. WM_NCHITTEST → HTCLIENT. WM_LBUTTONUP → find the bar by hwnd; if the click point is inside a badge circle (distance to its center ≤ radius) → bounds-check the monitor index, then SwitchWorkspace(bar.mon, i); return 0. WM_ERASEBKGND → 1. Default DefWindowProcW.

DestroyTaskbarIndicators(): destroy all bar windows, clear the vector, UnregisterClassW(kBarWindowClass, hinst) if registered, GdiplusShutdown if started; reset state so a later UpdateTaskbarIndicators works again.

Delete every GDI object you create. Run the -Check command until errors=0 warnings=0.
````

## Report from the agent (2026-10-09 07:33 UTC)

````text
Created src/56_bar.cpp. It implements RequestBarUpdate, UpdateTaskbarIndicators and DestroyTaskbarIndicators per the spec, with Bar*-prefixed helpers in the shared anonymous namespace and s_barWindows as the bar state. No other file was touched.

Check: `build.ps1 -Check 56_bar.cpp` ends with errors=0 warnings=0. I did not run the full build or the mod.

Deviations and notes:
- `PixelFormat32bppPARGB` is a macro in the MinGW gdiplus headers, not an enum, so it is used unqualified.
- Auto-hide thresholds (8 px high, 100 px wide) are raw physical pixels, as the spec literally says, not DPI-scaled.
- A bar keeps its last surface across hide/show and is only re-rendered when its signature changes. This assumes Windows keeps the layered content while a window is hidden.
- Link dependency: `ResolveTheme()` is declared in the contract but not yet defined. 55_osd.cpp only has `OsdResolveTheme()`, so the full build will not link until the theme agent adds `ResolveTheme`.
- 60_engine.cpp already calls RequestBarUpdate, UpdateTaskbarIndicators (kTimerBar, kTimerBarPoll, startup) and DestroyTaskbarIndicators (teardown), so no engine changes are needed.
````
