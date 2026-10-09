# 6. workspace OSD fragment

| | |
|---|---|
| Launched | 2026-10-09 06:39 UTC |
| Role | implementer |
| Model | Claude Haiku 5.5 |
| Agent id | `a126de84b4b84f1f9` |

## Instructions from the planner (verbatim)

````text
You are implementing one fragment of HawkTile, a clean-room tiling window manager written as a Windhawk mod (C++23, clang, mingw target). Project root: <repo>

## Ground rules
- First read `src/10_contract.h` completely (the binding contract) and skim `src/50_model.cpp` to see how `g.monitors`, `Workspace` and settings are used. Implement exactly the contract signatures. Do NOT edit any file other than your own. If the contract seems wrong, implement it as written and mention it in your report.
- Fragments are concatenated into ONE translation unit after 10_contract.h (order: ... 50_model.cpp, 55_osd.cpp, 60_engine.cpp ...). Do not #include project files. You may #include system headers you need at the top of your fragment (e.g. <shellscalingapi.h> for GetDpiForMonitor; shcore and gdi32 are linked).
- All code inside `namespace ht { ... }`; file-local helpers/state in an anonymous namespace inside ht. Name helpers distinctively (prefix `Osd`) — other fragments share the same anonymous namespace in the joined file, so generic names like `Paint`, `Scale`, `s_hwnd` may collide.
- Log with `HT_LOG(L"format %d", x)` only (string-literal format).
- Clean room: do NOT look at or reproduce code from komorebi, GlazeWM, Hyprland, i3, bspwm or any other window manager.
- Style: 4-space indent, Chromium-like, sparse `//` comments for non-obvious Win32 behavior, no exceptions.
- Verify: `powershell -NoProfile -File <repo>\build.ps1 -Check 55_osd.cpp` must end with `errors=0 warnings=0`.
- NEVER run anything that creates windows, moves/hides/focuses real windows, or runs the mod on this machine. Compile checks only.
- Final report under 200 words: what you built, deviations, concerns.

## Your assignment: new file `src/55_osd.cpp`

Implement `ShowWorkspaceOsd(int mon)` and `DestroyOsd()` as documented in the contract's "Workspace indicator / OSD" section. Everything runs on the engine thread (which is per-monitor-DPI-aware v2, so all coordinates are physical pixels).

Design:
- Window class kOsdWindowClass, registered lazily on first use with hInstance obtained via GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCWSTR)&<your wndproc>, &hinst). hCursor nullptr, hbrBackground nullptr (you paint everything).
- CreateWindowExW(WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE | WS_EX_TRANSPARENT, kOsdWindowClass, L"HawkTile workspaces", WS_POPUP, ...). SetLayeredWindowAttributes(hwnd, 0, 235, LWA_ALPHA). Ask for rounded corners on Windows 11: DwmSetWindowAttribute(hwnd, 33 /*DWMWA_WINDOW_CORNER_PREFERENCE*/, &pref, sizeof(pref)) with DWORD pref = 2 (round); ignore failure.
- Store what to draw in file-local state: workspace count N, active index, and a std::vector<bool> occupied (tiled or floating non-empty), the accent color (g.settings.focusBorderColor) and the DPI scale.
- DPI: GetDpiForMonitor(g.monitors[mon].handle, MDT_EFFECTIVE_DPI, &dx, &dy); fall back to 96. scale(v) = MulDiv(v, dpi, 96).
- Geometry (in 96-dpi units, then scaled): cell 40x40, gap between cells 8, padding 12 around. Width = 2*pad + N*cell + (N-1)*gap, height = 2*pad + cell. Center it horizontally in rcWork and place its vertical center at rcWork.top + 75% of the work-area height... no: place it vertically centered in rcWork (simple and visible).
- Show: SetWindowPos(hwnd, HWND_TOPMOST, x, y, w, h, SWP_NOACTIVATE | SWP_SHOWWINDOW); InvalidateRect(hwnd, nullptr, FALSE); UpdateWindow(hwnd). Then SetTimer(hwnd, 1, g.settings.osdDurationMs (fallback 700 if <= 0), nullptr). On WM_TIMER id 1: KillTimer and ShowWindow(hwnd, SW_HIDE).
- Paint (WM_PAINT, BeginPaint/EndPaint) with a memory DC + compatible bitmap (double buffering) then BitBlt:
  - background RGB(28, 28, 30).
  - each cell i: rounded rectangle (RoundRect, corner radius scale(10)):
    - active: brush = accent color, number text white.
    - occupied: brush RGB(58, 58, 64), text RGB(235, 235, 240).
    - empty: brush RGB(40, 40, 44), text RGB(120, 120, 128).
    Use a null pen (GetStockObject(NULL_PEN)) so cells have no outline. Delete every GDI object you create.
  - number = i + 1, centered (DrawTextW with DT_CENTER | DT_VCENTER | DT_SINGLELINE), font "Segoe UI Semibold", height -scale(17) (CreateFontW with CLEARTYPE_QUALITY). Create the font when showing (DPI may differ per monitor) and delete the previous one; SetBkMode TRANSPARENT.
- WM_NCHITTEST → HTTRANSPARENT. WM_MOUSEACTIVATE → MA_NOACTIVATE. WM_ERASEBKGND → return 1.
- No-op (and no window creation) if mon is out of range or g.monitors is empty. If window creation fails, log once and return.
- DestroyOsd: KillTimer, DestroyWindow, delete the font, UnregisterClassW(kOsdWindowClass, hinst); reset all state so a later ShowWorkspaceOsd works again.

Run `build.ps1 -Check 55_osd.cpp` until errors=0 warnings=0.
````

## Report from the agent (2026-10-09 06:42 UTC)

````text
Built src/55_osd.cpp (only file touched). Implements ShowWorkspaceOsd(int mon) and DestroyOsd() from the contract. Compile check `build.ps1 -Check 55_osd.cpp` gives errors=0 warnings=0. I did not run the full build or any window-creating code.

What it does: lazy class registration and hidden layered popup (WS_EX_LAYERED|TOPMOST|TOOLWINDOW|NOACTIVATE|TRANSPARENT, alpha 235, DWM corner pref 2 ignored on failure). Per show it reads workspace count and active index, occupied flags, focusBorderColor, and the per-monitor effective DPI (fallback 96). It rebuilds the Segoe UI Semibold font, centers the strip in rcWork, and restarts a hide timer (osdDurationMs, fallback 700). WM_PAINT double-buffers, draws RoundRect cells with a null pen, and handles NCHITTEST, MOUSEACTIVATE and ERASEBKGND as specified. DestroyOsd kills the timer, destroys the window, deletes the font, unregisters the class and resets state.

Deviations:
- RoundRect takes an ellipse diameter, so I pass 2*scale(10) to get a 10 px corner radius.
- SetLayeredWindowAttributes failing is treated as a creation failure (window destroyed, logged). An unset layered window is never visible.
- Sizes are scaled per component, not as a total, so sizing and paint agree exactly.
- Count comes from monitor workspaces.size() clamped to 1..9, not settings.workspaceCount. They match after ApplySettingsChange.

Concerns:
- DWM corner rounding may not apply to WS_EX_LAYERED popups, so corners could stay square. A SetWindowRgn round region would guarantee it. I did not add one because the spec asked only for DWM. Not verified at runtime.
- If the engine dies without DestroyOsd, the class stays registered with the old module's wndproc. A reloaded mod then gets ERROR_CLASS_ALREADY_EXISTS and the OSD stays disabled for that run.
````
