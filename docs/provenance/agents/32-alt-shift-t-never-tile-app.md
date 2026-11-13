# 32. Alt+Shift+T never-tile app

| | |
|---|---|
| Launched | 2026-10-26 14:08 UTC |
| Role | implementer |
| Model | Claude Haiku 5.5 |
| Agent id | `a653c37a07baaf0f0` |

## Instructions from the planner (verbatim)

````text
You are adding a feature to HawkTile, a tiling window manager written as a Windhawk mod (C++23, clang, mingw target). Project root: <repo>

## Ground rules
- Read `src/10_contract.h` fully. New in it: `CommandKind::ToggleAlwaysFloat` (command string `togglealwaysfloat`, default key alt+shift+t in kDefaultBindings), `EngineState::alwaysFloatApps` (std::unordered_set<std::wstring> of lowercase exe names), `LoadAlwaysFloatApps()` / `SaveAlwaysFloatApps()` (documented next to PersistHiddenList), and the updated ClassifyWindow comment (Float when the window's processName is in g.alwaysFloatApps). Then read `src/30_win32.cpp`, `src/40_config.cpp`, `src/50_model.cpp` and `src/60_engine.cpp` fully.
- Edit ONLY those four files; minimal, focused changes in the existing style. The anonymous namespace inside `ht` is shared across fragments, so new file-local helpers need distinctive names.
- Log with `HT_LOG(L"...", ...)` only (string-literal format; HT_LOG is a statement).
- Clean room: do NOT look at or reproduce code from any other window manager.
- Verify: `powershell -NoProfile -File <repo>\build.ps1 -Check <fragment>` for each of the four fragments, then the full `... build.ps1` (both clang lines must end errors=0 warnings=0) and `... build.ps1 -Test` (0 failed). Running `build.ps1 -Test` is allowed; never run the mod or hawktile-dev.exe. Do not use git.
- Final report under 180 words.

## 30_win32.cpp
- Implement LoadAlwaysFloatApps / SaveAlwaysFloatApps per the contract. Storage key L"alwaysFloatApps"; value = names joined with '|' (exe names cannot contain '|'). Use Wh_GetStringValue (8192-char buffer) / Wh_SetStringValue / Wh_DeleteValue inside `#ifndef HT_STANDALONE`, exactly like PersistHiddenList / RestoreLeftoverHiddenWindows do. Lowercase and trim each name when loading; skip empty ones. Load logs e.g. "always-float apps: 2 (code.exe|olk.exe)".
- ClassifyWindow: right after the ignore-rules check (before the float rules), return Eligibility::Float when `!f.processName.empty() && g.alwaysFloatApps.contains(f.processName)`.

## 40_config.cpp
Add `togglealwaysfloat` → CommandKind::ToggleAlwaysFloat to the no-argument command table. Make sure every kDefaultBindings entry still parses (alt+shift+t is new).

## 50_model.cpp — ExecuteCommand case ToggleAlwaysFloat
Only when `has` (a managed command target). Get the target's facts with GetWindowFacts; if processName is empty, log "can't tell which app this window belongs to" and return.
- If the app is NOT in g.alwaysFloatApps: insert it, SaveAlwaysFloatApps(), log "never tiling <app> again". Then for every managed window on every monitor/workspace whose GetWindowFacts processName equals the app and that is TILED: move it from that workspace's `tiled` vector to its `floating` vector. The command target itself gets the same centred 60 % placement that ToggleFloating uses (reuse that code; extract a small helper if needed); other windows keep their current position.
- If the app IS in the set: erase it, SaveAlwaysFloatApps(), log "tiling <app> again". Then for every managed FLOATING window of that app: if ClassifyWindow(hwnd, g.settings) now returns Eligibility::Tile, move it from `floating` to `tiled` (appended), restoring it first when g.settings.tileMaximized && IsZoomed (same as ToggleFloating does).
- Afterwards RetileAll(), UpdateFocusBorder(), and RequestBarUpdate() if that function exists in the contract.
Collect the windows first, then move them (don't mutate a vector you are iterating).

## 60_engine.cpp
Call LoadAlwaysFloatApps() during engine init right after LoadSettings(...) and before RestoreLeftoverHiddenWindows / RefreshMonitors / AdoptExistingWindows, so existing windows of remembered apps are adopted as floating. (Teardown already resets `g`, which clears the set.)

Run all commands until clean.
````

## Report from the agent (2026-10-26 14:13 UTC)

````text
Always-float feature (togglealwaysfloat) is implemented across the four allowed fragments. No other files were edited, and git was not used.

- <repo>\src\30_win32.cpp: kAlwaysFloatKey; JoinAlwaysFloatApps (sorted, '|'-joined); LoadAlwaysFloatApps (trims, lowercases, skips empty names, logs "always-float apps: N (...)"); SaveAlwaysFloatApps; ClassifyWindow returns Float right after the ignore-rules check.
- <repo>\src\40_config.cpp: togglealwaysfloat added to the no-argument command table.
- <repo>\src\50_model.cpp: centred placement extracted from ToggleFloating into PlaceCenteredFloating; new ManagedWindowsOfApp and ToggleAlwaysFloatApp; ExecuteCommand case added.
- <repo>\src\60_engine.cpp: LoadAlwaysFloatApps called right after LoadSettings.

Verification: -Check passes for all four fragments; full build passes both clang lines with errors=0 warnings=0; -Test reports 1035 passed, 0 failed. A throwaway harness in the scratchpad confirmed all 40 default bindings parse, togglealwaysfloat parses, and alt+shift+t maps to MOD_ALT|MOD_SHIFT with VK 'T'.

Judgment call: enabling always-float re-centres the command target even when it was already floating, following the spec literally. Tell me if it should move only tiled targets.

Not run: the mod itself and hawktile-dev.exe.
````
