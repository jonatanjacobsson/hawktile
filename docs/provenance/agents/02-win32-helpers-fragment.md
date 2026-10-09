# 2. Win32 helpers fragment

| | |
|---|---|
| Launched | 2026-10-09 06:17 UTC |
| Role | implementer |
| Model | Claude Haiku 5.5 |
| Agent id | `a6bdf9130388e37cb` |

## Instructions from the planner (verbatim)

````text
You are implementing one fragment of HawkTile, a clean-room tiling window manager written as a Windhawk mod (C++23, clang, mingw target). Project root: <repo>

## Ground rules (apply to every fragment)
- First read `src/10_contract.h` completely. It is the binding contract: types, globals and every cross-module function signature. Implement exactly those signatures. Do NOT edit 10_contract.h or any file other than the ones assigned to you. If you believe the contract is wrong, implement it as written and mention the problem in your final report.
- Fragments are concatenated into ONE translation unit after 10_contract.h, so do not #include project files. The contract already includes <windows.h>, <dwmapi.h>, <shellapi.h>, <algorithm>, <cstdio>, <cwchar>, <cwctype>, <string>, <unordered_set>, <vector>. You may add another standard-library #include at the top of your fragment if truly needed.
- Put all code inside `namespace ht { ... }`. File-local helpers go in an anonymous namespace inside `ht`.
- Log with `HT_LOG(L"format %d", x)` only (the format must be a string literal).
- Clean room: do NOT look at, search for, or reproduce code from komorebi, GlazeWM, Hyprland, i3, bspwm or any other window manager. Use only your own knowledge of Win32.
- Style: 4-space indent, `//` comments only where something is non-obvious, Chromium-like formatting, no exceptions.
- Verify with: `powershell -NoProfile -File <repo>\build.ps1 -Check <your-fragment-file-name>` — it must end with `errors=0 warnings=0`. Fix every warning. (The check compiles with -fsyntax-only and HT_STANDALONE defined; the real build compiles WITHOUT HT_STANDALONE, so make sure both branches of any #ifdef HT_STANDALONE are valid — you can temporarily verify the non-standalone branch by reasoning carefully, since Wh_* functions are declared by windhawk_api.h in both cases.)
- Never run anything that moves, hides or focuses real windows on this machine.
- Final report (keep it under 200 words): what you implemented, any deviation from the contract, any concerns.

## Your assignment: `src/30_win32.cpp`

Implement: GetWindowFacts, IsWindowCloaked, ClassifyWindow, GetVisibleFrame, PlaceWindow, FocusWindow, SendMenuMaskKey, SetFocusBorder, ClearFocusBorder, HideForWorkspace, ShowForWorkspace, PersistHiddenList, RestoreLeftoverHiddenWindows. Semantics are documented in the contract; extra details:

- `g` (EngineState) is declared extern in the contract and defined elsewhere; just use it.
- GetWindowFacts: process name via GetWindowThreadProcessId → OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION) → QueryFullProcessImageNameW → take the file name after the last '\\' and lowercase it with towlower. Handle failures (empty name). Class via GetClassNameW (256 buffer), title via GetWindowTextLengthW + GetWindowTextW. style/exStyle via GetWindowLongPtrW. owner via GetWindow(hwnd, GW_OWNER). cloaked via IsWindowCloaked. Returns false if !IsWindow(hwnd).
- IsWindowCloaked: DWORD value from DwmGetWindowAttribute(hwnd, kDwmCloaked, &v, sizeof(v)); cloaked if the call succeeded and v != 0.
- ClassifyWindow: follow the exact check order in the contract. "Belongs to the current process" = pid == GetCurrentProcessId(). "Full WS_CAPTION" = (style & WS_CAPTION) == WS_CAPTION. Use AnyRuleMatches (implemented in another fragment) for rules. Class-name comparisons for the built-in ignore list are exact and case-sensitive.
- GetVisibleFrame: DwmGetWindowAttribute(hwnd, kDwmExtendedFrameBounds, &rect, sizeof(RECT)); on failure GetWindowRect.
- PlaceWindow: skip (return true) if IsIconic(hwnd) or the target is empty. Compute borders: wr = GetWindowRect, fr = GetVisibleFrame; left = fr.left - wr.left, top = fr.top - wr.top, right = wr.right - fr.right, bottom = wr.bottom - fr.bottom (clamp each to >= 0). New window rect = { target.left - left, target.top - top, target.right + right, target.bottom + bottom }. If it equals wr, return true without calling SetWindowPos. Flags: SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOACTIVATE | SWP_ASYNCWINDOWPOS. Compare MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST) with MonitorFromRect(&target, MONITOR_DEFAULTTONEAREST) BEFORE the first call; if different, call SetWindowPos a second time with the same arguments. If SetWindowPos fails: if GetLastError() == ERROR_ACCESS_DENIED log and return false; otherwise log and return true.
- FocusWindow: no-op if !IsWindow. If IsIconic → ShowWindowAsync(hwnd, SW_RESTORE). SetForegroundWindow(hwnd). If GetForegroundWindow() != hwnd: SendMenuMaskKey(), then SetForegroundWindow(hwnd) again. Finally, if still not foreground, log it.
- SendMenuMaskKey: two INPUT_KEYBOARD inputs (wVk = kMenuMaskVk, second with KEYEVENTF_KEYUP) in one SendInput call.
- SetFocusBorder / ClearFocusBorder: DwmSetWindowAttribute(hwnd, kDwmBorderColor, &color, sizeof(COLORREF)); ignore result; no-op if !IsWindow.
- HideForWorkspace / ShowForWorkspace: exactly as the contract says (insert/erase in g.hiddenByUs FIRST, then ShowWindowAsync with SW_HIDE / SW_SHOWNA), then PersistHiddenList().
- PersistHiddenList / RestoreLeftoverHiddenWindows: wrap the bodies in `#ifndef HT_STANDALONE` (no-ops in standalone). Storage key L"hiddenWindows". Format each HWND as hex with swprintf (`%llx` of (unsigned long long)(ULONG_PTR)hwnd), comma separated. Write with Wh_SetStringValue (if the set is empty, call Wh_DeleteValue(L"hiddenWindows") instead). Restore: WCHAR buffer of 8192, Wh_GetStringValue(L"hiddenWindows", buf, 8192); parse tokens with wcstoull(..., 16); for each HWND that IsWindow && !IsWindowVisible → ShowWindowAsync(SW_SHOWNA); log how many; then Wh_DeleteValue(L"hiddenWindows"). Note: Wh_GetStringValue(PCWSTR name, PWSTR buffer, size_t bufferChars) returns size_t; Wh_SetStringValue(PCWSTR name, PCWSTR value) returns BOOL; Wh_DeleteValue(PCWSTR name) returns BOOL. Check the real declarations in `C:\Program Files\Windhawk\Compiler\include\windhawk_api.h` if unsure.

Run `build.ps1 -Check 30_win32.cpp` until it reports errors=0 warnings=0.
````

## Report from the agent (2026-10-09 06:22 UTC)

````text
Implemented src/30_win32.cpp with all 13 assigned functions, signatures exactly as in 10_contract.h. build.ps1 -Check 30_win32.cpp reports errors=0 warnings=0. I also compiled the non-standalone branch (no HT_STANDALONE, no WH_EDITING, real build flags) from a scratch copy: zero warnings and errors.

Notes and concerns:
- No contract deviations. Helpers live in an anonymous namespace inside ht (ProcessNameOf, IsIgnoredClass, HwndToNumber, MoveOnce, kHiddenKey, kIgnoredClasses).
- RestoreLeftoverHiddenWindows will show any stored HWND that IsWindow and is not visible. HWND values are recycled, so after a crash a stale handle can point to an unrelated window that was hidden on purpose. Storing the pid next to each HWND and checking it on restore would close that gap. I implemented the contract as written.
- The ERROR_ACCESS_DENIED return from PlaceWindow is unverified at runtime. SWP_ASYNCWINDOWPOS may change when that error is reported. Only compile-checked so far.
- ClassifyWindow calls GetWindowFacts first, so it opens the process and reads class and title even for windows it will ignore. Cost is small.
- Nothing was run against real windows.
````
