# 5. engine thread/events fragment

| | |
|---|---|
| Launched | 2026-10-09 06:19 UTC |
| Role | implementer |
| Model | Claude Haiku 5.5 |
| Agent id | `a39ea4b3ac814ed23` |

## Instructions from the planner (verbatim)

````text
You are implementing one fragment of HawkTile, a clean-room tiling window manager written as a Windhawk mod (C++23, clang, mingw target). Project root: <repo>

## Ground rules (apply to every fragment)
- First read `src/10_contract.h` completely, and `PLAN.md`. The contract is binding: types, globals and every cross-module function signature. Implement exactly those signatures. Do NOT edit 10_contract.h or any file other than the one assigned to you. If you believe the contract is wrong, implement it as written and mention the problem in your final report.
- Fragments are concatenated into ONE translation unit after 10_contract.h, so do not #include project files. The contract already includes <windows.h>, <dwmapi.h>, <shellapi.h>, <algorithm>, <cstdio>, <cwchar>, <cwctype>, <string>, <unordered_set>, <vector>.
- Put all code inside `namespace ht { ... }`. File-local state and helpers go in an anonymous namespace inside `ht`.
- Log with `HT_LOG(L"format %d", x)` only (the format must be a string literal).
- Clean room: do NOT look at, search for, or reproduce code from komorebi, GlazeWM, Hyprland, i3, bspwm or any other window manager. Use only your own knowledge of Win32.
- Style: 4-space indent, `//` comments only where something is non-obvious, Chromium-like formatting, no exceptions.
- Verify with: `powershell -NoProfile -File <repo>\build.ps1 -Check 60_engine.cpp` — it must end with `errors=0 warnings=0`. Fix every warning.
- Never run anything that moves, hides or focuses real windows on this machine.
- Final report (keep it under 250 words): what you implemented, any deviation from the contract, any concerns.

## Your assignment: `src/60_engine.cpp`

Implement StartEngine, NotifySettingsChanged, StopEngine (contract "Engine" section), plus the file-local engine thread, window procedure, WinEvent callback and pending-work processing described below. You call functions from other fragments (all declared in the contract): LoadSettings, RegisterBindings, UnregisterBindings, ClassifyWindow, IsWindowCloaked, SendMenuMaskKey, ShowForWorkspace, ClearFocusBorder, PersistHiddenList, RestoreLeftoverHiddenWindows, RefreshMonitors, ApplySettingsChange, Locate, ManageWindow, UnmanageWindow, RetileAll, RequestRetile, MoveWindowToWorkspace, OnFocusChanged, UpdateFocusBorder, OnUserDragEnd, ExecuteCommand, AdoptExistingWindows, and the global `g`. Do not reimplement them.

If EVENT_OBJECT_CLOAKED / EVENT_OBJECT_UNCLOAKED are not defined by the headers, define them (0x8017 / 0x8018) with #ifndef at the top of your fragment (outside the namespace). Same for DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 if missing ((DPI_AWARENESS_CONTEXT)-4).

File-local state: HANDLE s_thread, DWORD s_threadId, HANDLE s_initEvent, bool s_initOk, std::vector<HWINEVENTHOOK> s_hooks, HINSTANCE s_hinst.

StartEngine (called on Windhawk's thread): create a manual-reset event; CreateThread(EngineThreadProc); WaitForMultipleObjects on {initEvent, thread} with 10000 ms. If the init event fired → return s_initOk (if false, wait for the thread to exit and clean up handles). If the thread exited or timeout → log, (on timeout PostThreadMessageW(WM_QUIT) and wait 5 s), clean up, return false.

NotifySettingsChanged: if g.engineHwnd PostMessageW(g.engineHwnd, WM_HT_RELOAD, 0, 0).

StopEngine: if s_thread: PostThreadMessageW(s_threadId, WM_QUIT, 0, 0); WaitForSingleObject(s_thread, 10000); close handles; reset state.

EngineThreadProc:
1. SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2) — all coordinates must be physical pixels.
2. Get the module handle containing our code: GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCWSTR)&EngineWndProc, &s_hinst).
3. RegisterClassExW with kEngineWindowClass and EngineWndProc. CreateWindowExW(WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE, kEngineWindowClass, L"HawkTile", WS_POPUP, 0, 0, 0, 0, nullptr, nullptr, s_hinst, nullptr). It must NOT be a message-only window (HWND_MESSAGE windows don't receive WM_DISPLAYCHANGE / WM_SETTINGCHANGE broadcasts). Never show it. g.engineHwnd = hwnd. On failure: log, s_initOk = false, SetEvent, return 1.
4. LoadSettings(g.settings); RestoreLeftoverHiddenWindows(); RefreshMonitors(); AdoptExistingWindows();
5. Log the result of RegisterBindings(hwnd, g.settings.bindings) vs bindings.size().
6. Install WinEvent hooks, each with WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS, process 0, thread 0, callback WinEventProc, for these ranges: [EVENT_SYSTEM_FOREGROUND], [EVENT_SYSTEM_MOVESIZESTART..EVENT_SYSTEM_MOVESIZEEND], [EVENT_SYSTEM_MINIMIZESTART..EVENT_SYSTEM_MINIMIZEEND], [EVENT_OBJECT_DESTROY..EVENT_OBJECT_HIDE], [EVENT_OBJECT_LOCATIONCHANGE], [EVENT_OBJECT_CLOAKED..EVENT_OBJECT_UNCLOAKED]. Keep narrow ranges — never hook the whole 0x8000 range (too noisy). Log any failed hook.
7. RetileAll(); OnFocusChanged(GetForegroundWindow());
8. s_initOk = true; SetEvent(s_initEvent).
9. Standard GetMessageW loop until it returns 0 or -1 (TranslateMessage/DispatchMessageW).
10. Cleanup (always, in this order): unhook all hooks; UnregisterBindings(hwnd, g.settings.bindings); KillTimer for kTimerRetile/kTimerPending/kTimerDisplay; copy g.hiddenByUs to a vector and ShowForWorkspace each one that IsWindow (so no window stays hidden); PersistHiddenList(); if g.borderedHwnd && IsWindow → ClearFocusBorder; DestroyWindow(hwnd); UnregisterClassW(kEngineWindowClass, s_hinst); `g = EngineState{};` Log "engine stopped".

EngineWndProc:
- WM_HOTKEY: idx = (int)wParam - kHotkeyIdBase; if 0 <= idx < bindings.size(): if the binding's modifiers include MOD_ALT or MOD_WIN → SendMenuMaskKey(); then ExecuteCommand(binding.cmd). Return 0.
- WM_TIMER: kTimerRetile → KillTimer, RetileAll(). kTimerPending → KillTimer, ProcessPending(). kTimerDisplay → KillTimer, RefreshMonitors(), RetileAll(), UpdateFocusBorder(). Return 0.
- WM_DISPLAYCHANGE, or WM_SETTINGCHANGE with wParam == SPI_SETWORKAREA → SetTimer(hwnd, kTimerDisplay, 400, nullptr); return 0 (for WM_SETTINGCHANGE call DefWindowProc for other wParams).
- WM_HT_RELOAD: UnregisterBindings(hwnd, g.settings.bindings); LoadSettings(g.settings); RegisterBindings(...) and log count; ApplySettingsChange(). Return 0.
- Everything else → DefWindowProcW.

Helper SchedulePending(): SetTimer(g.engineHwnd, kTimerPending, 80, nullptr).

WinEventProc(HWINEVENTHOOK, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD, DWORD):
- Ignore unless hwnd != nullptr, idObject == OBJID_WINDOW, idChild == CHILDID_SELF, hwnd != g.engineHwnd.
- WinLoc loc; bool managed = Locate(hwnd, &loc); bool hiddenByUs = g.hiddenByUs.count(hwnd) > 0.
- EVENT_OBJECT_SHOW: managed && hiddenByUs → g.pendingVisibility.insert, SchedulePending. !managed && GetAncestor(hwnd, GA_ROOT) == hwnd → g.pendingEval.insert, SchedulePending.
- EVENT_OBJECT_UNCLOAKED: !managed && top-level → pendingEval + SchedulePending.
- EVENT_OBJECT_HIDE, EVENT_OBJECT_CLOAKED: managed && !hiddenByUs → pendingVisibility + SchedulePending. (Our own hides are recognised via g.hiddenByUs; the deferred check avoids races with async show/hide.)
- EVENT_OBJECT_DESTROY: g.pendingEval.erase(hwnd); if managed → UnmanageWindow(hwnd), RequestRetile(); else if hiddenByUs → g.hiddenByUs.erase(hwnd), PersistHiddenList().
- EVENT_SYSTEM_MINIMIZESTART: managed → RequestRetile().
- EVENT_SYSTEM_MINIMIZEEND: managed → RequestRetile(); else pendingEval + SchedulePending.
- EVENT_SYSTEM_FOREGROUND: if !managed → pendingEval.insert + SchedulePending. Then always OnFocusChanged(hwnd).
- EVENT_SYSTEM_MOVESIZEEND: managed && !loc.floating → OnUserDragEnd(hwnd).
- EVENT_OBJECT_LOCATIONCHANGE: managed only: bool z = IsZoomed(hwnd) != FALSE; bool was = g.zoomed.count(hwnd) > 0; if z != was → update g.zoomed and RequestRetile().

ProcessPending():
- Swap g.pendingEval into a local vector (clear the set). For each hwnd: skip if !IsWindow or already managed; e = ClassifyWindow(hwnd, g.settings); Tile/Float → ManageWindow(hwnd, e == Eligibility::Float), changed = true; if it is GetForegroundWindow() → OnFocusChanged(hwnd).
- Swap g.pendingVisibility into a local vector. For each hwnd: WinLoc loc; if !Locate → continue. If !IsWindow → UnmanageWindow, changed. Else if g.hiddenByUs contains it: if IsWindowVisible(hwnd) (the app re-showed a window we hid on an inactive workspace) → ShowForWorkspace(hwnd) and MoveWindowToWorkspace(hwnd, loc.mon, g.monitors[loc.mon].active), log, changed. Else (not hidden by us): if !IsWindowVisible(hwnd) || IsWindowCloaked(hwnd) → UnmanageWindow, changed.
- If changed → RequestRetile(); UpdateFocusBorder().

Run `build.ps1 -Check 60_engine.cpp` until it reports errors=0 warnings=0.
````

## Report from the agent (2026-10-09 06:22 UTC)

````text
Implemented src/60_engine.cpp. `build.ps1 -Check 60_engine.cpp` reports errors=0 warnings=0. No other file was edited, and nothing was run that touches real windows.

Implemented: StartEngine, NotifySettingsChanged, StopEngine, plus file-local EngineThreadProc (split into CreateEngineWindow, InstallHooks, TeardownEngine), EngineWndProc, WinEventProc, ProcessPending and SchedulePending. The #ifndef defines for EVENT_OBJECT_CLOAKED/UNCLOAKED and DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 are at the top, outside the namespace.

Deviations and additions (all small):
- EVENT_OBJECT_DESTROY also erases pendingVisibility, since HWNDs can be reused.
- Teardown erases destroyed HWNDs from hiddenByUs before PersistHiddenList, so the persisted list is clean.
- GetModuleHandleExW failure and CreateWindowExW failure abort init (the latter also unregisters the class).
- StartEngine returns s_initOk if called while already running. A timeout or WAIT_FAILED posts WM_QUIT and waits 5 s; a thread exit during init is logged separately.
- ProcessPending calls OnFocusChanged only for windows that were actually managed, and sets changed only when ManageWindow returns true.

Concerns:
- NotifySettingsChanged reads g.engineHwnd from Windhawk's thread without synchronisation. That is a benign pointer read, and PostMessageW to a stale HWND just fails. It follows the no-locks rule, but it is still a race.
- The helpers sit in anonymous namespaces inside ht, so a same-named helper in another fragment (e.g. ProcessPending, ResetHandles, IsTopLevel, SchedulePending) would be a redefinition error at concatenation. Worth a grep when the fragments are joined.
- EVENT_OBJECT_LOCATIONCHANGE calls Locate (a linear scan) on every event. This may matter with many windows.
- Contract: no problems found; implemented as written.
````
