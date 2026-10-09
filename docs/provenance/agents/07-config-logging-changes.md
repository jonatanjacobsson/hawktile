# 7. config + logging changes

| | |
|---|---|
| Launched | 2026-10-09 06:40 UTC |
| Role | implementer |
| Model | Claude Haiku 5.5 |
| Agent id | `a118dd5f46104f07a` |

## Instructions from the planner (verbatim)

````text
You are modifying two fragments of HawkTile, a clean-room tiling window manager written as a Windhawk mod (C++23, clang, mingw target). Project root: <repo>

## Ground rules
- First read `src/10_contract.h` completely — it was just updated (new settings, new commands, new LogLine function, HT_LOG now calls ::ht::LogLine outside standalone builds). Then read the two files you modify. Implement exactly the contract. Do NOT edit any other file. If the contract seems wrong, implement it as written and mention it in your report.
- Fragments are concatenated into ONE translation unit after 10_contract.h. File-local helpers live in an anonymous namespace inside `ht` that is SHARED by all fragments in the joined file, so give new helpers distinctive names (e.g. prefix `Log`).
- Log with `HT_LOG(L"...", ...)` (string-literal format) — except inside LogLine itself, which must not call HT_LOG (that would recurse).
- Clean room: do NOT look at or reproduce code from komorebi/whkd, GlazeWM, Hyprland, i3, bspwm or any other window manager.
- Style: match the existing code in each file. 4-space indent, no exceptions.
- Verify both: `powershell -NoProfile -File <repo>\build.ps1 -Check 40_config.cpp` and `... -Check 30_win32.cpp` must each end with `errors=0 warnings=0`. These checks define HT_STANDALONE; the real build does not, so code under `#ifndef HT_STANDALONE` must also be valid — read `C:\Program Files\Windhawk\Compiler\include\windhawk_api.h` for exact Wh_* signatures (e.g. `size_t Wh_GetModStoragePath(PWSTR pathBuffer, size_t bufferChars)`, and `Wh_Log` is a MACRO that takes a format string literal: call it as `Wh_Log(L"%s", buffer)`).
- NEVER run the mod or anything that touches real windows.
- Final report under 200 words.

## Changes in `src/40_config.cpp`
1. LoadSettings: read the new settings (names from the YAML in `src/00_header.wh.cpp`): `tileMaximized` (bool), `workspaceOsd` (bool), `osdDuration` (int → s.osdDurationMs; value <= 0 means 700; then clamp 100..5000), `debugLog` (bool). Include them in the one-line summary log.
2. ParseCommand: add `send-to-workspace N` (→ CommandKind::SendToWorkspace, arg N in 1..9) and `toggle-maximize` (→ CommandKind::ToggleMaximize). Keep everything else unchanged.
3. kDefaultBindings (in the contract) gained `alt+shift+f` → `toggle-maximize`; make sure it parses.

## Changes in `src/30_win32.cpp`
Implement `void LogLine(const wchar_t* format, ...)` exactly as documented in the contract:
- `va_list` + `_vsnwprintf(buf, 1023, format, args)`; ensure NUL termination (buf[1023] = 0); on a negative return the buffer is still usable (truncated).
- `#ifndef HT_STANDALONE`: `Wh_Log(L"%s", buf);` then, if `g.settings.debugLog`: build the path once per call: `WCHAR dir[MAX_PATH]; Wh_GetModStoragePath(dir, MAX_PATH)`, `CreateDirectoryW(dir, nullptr)` (ignore ERROR_ALREADY_EXISTS), path = dir + L"\\hawktile.log". If the file exists and is larger than 2 MB (GetFileAttributesExW), DeleteFileW it. Open with CreateFileW(path, FILE_APPEND_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr). Line = local time from GetLocalTime formatted "HH:MM:SS.mmm " + text + "\r\n", converted with WideCharToMultiByte(CP_UTF8). WriteFile, CloseHandle. Ignore all failures silently.
- `#else` (standalone): `fwprintf(stderr, L"%s\n", buf);`
- It may run on any thread; keep it free of shared mutable state.

Run both -Check commands until both report errors=0 warnings=0.
````

## Follow-up from the planner (2026-10-09 06:40 UTC)

````text
FYI from the planner: the contract's kDefaultBindings entry for toggle-maximize is now `alt+f` (was `alt+shift+f`). No action needed beyond making sure every kDefaultBindings entry still parses.
````

## Report from the agent (2026-10-09 06:42 UTC)

````text
Both fragments are done and verified. No other file was edited.

Verification: `build.ps1 -Check 30_win32.cpp` and `-Check 40_config.cpp` both report `errors=0 warnings=0`. I also ran a real-mode `-fsyntax-only` (no HT_STANDALONE, no WH_EDITING) on fragments 00, 10, 20, 30, 40 from a scratch copy in the scratchpad, with 0 errors and 0 warnings. The full build's real-mode step was not run, because it writes into dist/, so fragments 50, 55, 60 and 90 are not covered by my checks. Nothing was run against the mod or real windows.

40_config.cpp:
- LoadSettings/ReadWindhawkSettings read tileMaximized, workspaceOsd, osdDuration (<=0 gives 700, then clamp 100..5000) and debugLog. debugLog is read first, because LoadSettings is called with g.settings itself (60_engine.cpp:271, 384) and `s = Settings{}` resets it before LogLine checks it. The summary line includes all four values.
- ParseCommand: `send-to-workspace N` maps to SendToWorkspace; `toggle-maximize` was added to the no-arg table.
- The contract's default is now `alt+f` (I confirmed this in 10_contract.h), and it parses.

30_win32.cpp:
- LogLine is implemented as specified. The file-writing helper LogAppendToFile sits in the anonymous namespace under `#ifndef HT_STANDALONE`. The standalone branch is fwprintf.

Contract notes (no contract change needed):
- Early HT_LOG lines in LoadSettings that fire before ReadWindhawkSettings (invalid default entries) see debugLog as false and go only to Wh_Log.
- The standalone branch of LogLine is unreachable, because HT_LOG maps straight to fwprintf in standalone builds.
````
