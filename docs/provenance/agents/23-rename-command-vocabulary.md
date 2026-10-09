# 23. rename command vocabulary

| | |
|---|---|
| Launched | 2026-10-09 12:05 UTC |
| Role | implementer |
| Model | Claude Haiku 5.5 |
| Agent id | `a50046250c3127a48` |

## Instructions from the planner (verbatim)

````text
You are modifying two fragments of HawkTile, a tiling window manager written as a Windhawk mod (C++23, clang, mingw target). Project root: <repo>

## Ground rules
- Read `src/10_contract.h` fully. The command vocabulary was just renamed: `enum class CommandKind` has new member names, the ParseCommand grammar comment lists the new command strings, and kDefaultBindings uses them. The settings list holding key bindings is now named `binds` (see the YAML in `src/00_header.wh.cpp`). Then read `src/40_config.cpp` and `src/50_model.cpp` fully.
- Edit ONLY `src/40_config.cpp` and `src/50_model.cpp`. This is a rename — keep behaviour identical.
- Clean room: do NOT look at or reproduce code from any other window manager.
- Verify: `powershell -NoProfile -File <repo>\build.ps1 -Check 40_config.cpp`, `-Check 50_model.cpp`, then the full `... build.ps1` (both clang lines errors=0 warnings=0) and `... build.ps1 -Test` (0 failed). NEVER run the mod.
- Final report under 150 words.

## Mapping (old → new)
CommandKind: Focus→MoveFocus, Move→SwapWindow, FocusWorkspace→Workspace, MoveToWorkspace (follow) stays MoveToWorkspace, SendToWorkspace→MoveToWorkspaceSilent, CycleLayout→NextLayout, ToggleFloat→ToggleFloating, Promote→SwapWithMaster, Grow→GrowMaster, Shrink→ShrinkMaster, Retile→Rescan, TogglePause→ToggleTiling, Close→CloseWindow, ToggleMaximize→Fullscreen, Resize→ResizeActive; unchanged: None, FocusMonitorNext, MoveToMonitorNext, SetLayout, ToggleMonocle, ToggleSplit, WorkspaceNext, WorkspacePrev.

Command strings (ParseCommand), old → new, old strings must NOT be accepted anymore:
`focus <dir>`→`movefocus <dir>`, `move <dir>`→`swapwindow <dir>`, `resize <dir>`→`resizeactive <dir>`, `workspace N` stays, `workspace-next`/`workspace-prev`→`workspace next`/`workspace prev` (the `workspace` command now accepts N = 1..9 OR the words next/prev), `move-to-workspace N`→`movetoworkspace N`, `send-to-workspace N`→`movetoworkspacesilent N`, `monitor-next`→`focusmonitor next`, `move-to-monitor-next`→`movetomonitor next`, `cycle-layout`→`nextlayout`, `layout <name>` stays, `toggle-float`→`togglefloating`, `toggle-monocle`→`togglemonocle`, `toggle-maximize`→`fullscreen`, `toggle-split`→`togglesplit`, `promote`→`swapwithmaster`, `grow`→`growmaster`, `shrink`→`shrinkmaster`, `retile`→`rescan`, `toggle-pause`→`toggletiling`, `close`→`closewindow`.
Directions: accept `left|right|up|down` and also the short forms `l|r|u|d`.

## Also
- 40_config.cpp: read the bindings from `binds[%d].keys` / `binds[%d].command` (was `keybindings[...]`). Every kDefaultBindings entry must parse.
- Update comments and log strings in both files that mention old command names (e.g. "send", "toggle-monocle", "retile command", "toggle-pause") to the new names; internal function names such as Retile/RequestRetile/MoveWindowToWorkspace stay as they are.
- Grep both files afterwards for the old strings listed above to make sure none remain (except as substrings of new names, e.g. "workspace").

Run all commands until clean.
````

## Report from the agent (2026-10-09 12:08 UTC)

````text
Renamed the command vocabulary in src/40_config.cpp and src/50_model.cpp only. Behaviour is unchanged.

- 40_config.cpp: the no-arg table and ParseCommand use the new verbs. Old strings are rejected. Directions accept l/r/u/d. Bindings are read from binds[%d].keys / binds[%d].command.
- 50_model.cpp: ExecuteCommand cases, guards and comments use the new CommandKind names. The prose comment "Send it" became "Move it silently".

Verification: -Check 40_config.cpp and -Check 50_model.cpp both report errors=0 warnings=0. The full build (both clang passes) reports errors=0 warnings=0. -Test reports 1035 passed, 0 failed. The layout test does not cover ParseCommand, so I compiled a scratch harness in the scratchpad (outside the repo): 109 checks, 0 failures. It covers all 39 default bindings parsing and the old-string rejects. The mod was not run.

Not touched: docs/PLAN.md and docs/research-windhawk-mods.md still mention `keybindings`. The binds YAML in src/00_header.wh.cpp already uses the new command strings.
````
