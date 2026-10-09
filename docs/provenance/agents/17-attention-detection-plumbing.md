# 17. attention detection plumbing

| | |
|---|---|
| Launched | 2026-10-09 07:56 UTC |
| Role | implementer |
| Model | Claude Haiku 5.5 |
| Agent id | `a31e8d6925534c537` |

## Instructions from the planner (verbatim)

````text
You are modifying three fragments of HawkTile, a clean-room tiling window manager written as a Windhawk mod (C++23, clang, mingw target). Project root: <repo>

## Ground rules
- Read `src/10_contract.h` fully. New: EngineState.attention, Settings.attentionDots, MarkAttention, WorkspaceHasAttention, and the extended OnFocusChanged contract. Read the settings YAML in `src/00_header.wh.cpp`. Then read `src/40_config.cpp`, `src/50_model.cpp`, `src/60_engine.cpp`.
- Edit ONLY those three files; minimal, focused changes in the existing style.
- Log with `HT_LOG(L"...", ...)` only (string-literal format).
- Clean room: do NOT look at or reproduce code from komorebi, GlazeWM, Hyprland, i3, bspwm or any other window manager or taskbar replacement.
- Verify: `powershell -NoProfile -File <repo>\build.ps1 -Check 40_config.cpp`, `-Check 50_model.cpp`, `-Check 60_engine.cpp` → errors=0 warnings=0 each. Do NOT run the full build (another agent edits 55_osd.cpp / 56_bar.cpp in parallel). NEVER run the mod or anything touching real windows.
- Final report under 150 words.

## 40_config.cpp
Read `attentionDots` (bool) into s.attentionDots; add it to the summary log line.

## 50_model.cpp
- Implement `MarkAttention(HWND hwnd)` and `WorkspaceHasAttention(int mon, int wsIndex)` exactly per the contract. For MarkAttention's foreground test: fg = GetForegroundWindow(); skip if fg == resolved window or GetAncestor(fg, GA_ROOTOWNER) == resolved window. Log once when a window is newly marked ("attention %p").
- OnFocusChanged: after resolving the managed window, erase it from g.attention; if something was erased call RequestBarUpdate().
- UnmanageWindow: erase the window from g.attention.

## 60_engine.cpp
- File-local `UINT s_shellHookMsg = 0;`.
- Init, right after the engine window is created: `s_shellHookMsg = RegisterWindowMessageW(L"SHELLHOOK");` and `RegisterShellHookWindow(hwnd)` (log if it fails).
- EngineWndProc: before the switch (the registered message id is not a compile-time constant), `if (s_shellHookMsg != 0 && msg == s_shellHookMsg) { if (wParam == HSHELL_FLASH) MarkAttention(reinterpret_cast<HWND>(lParam)); return 0; }`. HSHELL_FLASH is (HSHELL_REDRAW | HSHELL_HIGHBIT) = 0x8006; if the headers don't define HSHELL_FLASH, define it with #ifndef at the top of the fragment. Note: a plain HSHELL_REDRAW (0x0006) also arrives while a window is flashing — it must NOT clear attention; only activation clears it (already handled by OnFocusChanged).
- Teardown: `DeregisterShellHookWindow(hwnd)` before DestroyWindow; reset s_shellHookMsg.

Run the three -Check commands until clean.
````

## Report from the agent (2026-10-09 07:57 UTC)

````text
All three fragments edited; -Check is clean for each (errors=0 warnings=0). I did not run the full build or touch real windows. Only 40_config.cpp, 50_model.cpp and 60_engine.cpp changed.

- 40_config.cpp: reads attentionDots into s.attentionDots (same Wh_GetIntSetting != 0 pattern) and adds attentionDots=%d to the summary log.
- 50_model.cpp: MarkAttention and WorkspaceHasAttention added after OnFocusChanged, per contract. OnFocusChanged erases the resolved window from g.attention and calls RequestBarUpdate if it was there. UnmanageWindow erases from g.attention. MarkAttention logs "attention %p" and requests a bar update only on a new insertion.
- 60_engine.cpp: HSHELL_FLASH fallback (0x8006, #ifndef). File-local s_shellHookMsg. After g.engineHwnd is set: RegisterWindowMessageW("SHELLHOOK") and RegisterShellHookWindow (logs on failure). EngineWndProc dispatches shell-hook messages before the switch; only HSHELL_FLASH acts, so HSHELL_REDRAW does not clear attention. Teardown calls DeregisterShellHookWindow and resets s_shellHookMsg before DestroyWindow.

00_header.wh.cpp already had the attentionDots YAML entry, so it was not touched.
````
