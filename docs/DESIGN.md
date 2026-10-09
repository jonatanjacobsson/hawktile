# HawkTile design notes

How HawkTile is put together, for people who want to read or change the code.

## Process model

```
windhawk.exe (tray / UI)                windhawk.exe -tool-mod "<mod id>"
  └─ mod loaded → launcher only ──spawn──►  └─ engine thread (per-monitor DPI aware v2)
                                              ├─ hidden top-level window: WM_HOTKEY, timers,
                                              │  WM_DISPLAYCHANGE, settings reload, shell hook
                                              ├─ WinEvent hooks (out of context, skip own process)
                                              ├─ model: monitors → workspaces → tiled / floating windows
                                              ├─ layout engine (pure functions, unit tested)
                                              ├─ workspace indicator (center OSD) + taskbar badges
                                              └─ Win32 actuators: SetWindowPos (async), ShowWindowAsync,
                                                 SetForegroundWindow, DWM border color
```

- **Tool mod.** Windhawk's "mods as tools" pattern: the engine runs in its own
  `windhawk.exe` process, so a bug can never take Explorer or your apps down.
  When the mod is recompiled, the new process waits for the old one to exit.
- **One thread owns all state.** Windhawk's callbacks only post messages to the
  engine window. There are no locks.
- **Workspaces hide windows** (`ShowWindowAsync(SW_HIDE)`). The hidden list is
  kept in mod storage as `hwnd:pid` pairs, so a killed process is cleaned up on
  the next start. Disabling the mod and signing out (`WM_ENDSESSION`, where
  Windhawk doesn't call the mod's uninit) both show everything again.
- **Shutdown-safe globals.** All global state is plain containers and handles;
  the engine thread is joined and its windows, hooks, class and GDI+ are
  released on that thread before the mod unloads, as Windhawk's
  "Global objects and process shutdown" guide asks.
- **Logging** is plain `Wh_Log` (free while logging is off); only the optional
  debug log file formats text itself.
- **Invisible borders** are compensated with `DWMWA_EXTENDED_FRAME_BOUNDS`, so
  gaps are even.
- **Hung apps can't block the engine:** cross-process calls are asynchronous or
  use timeouts.

## Layout model

Every layout except monocle is a sequence of binary splits of the work area.
Split *i* cuts the remaining region; window *i* takes the first part (left of a
vertical cut, top of a horizontal cut) and the rest continues in the second
part.

| Layout | Orientation of split *i* | Default ratio |
|---|---|---|
| dwindle | longer side of the remaining region (can be flipped per split) | first split: master ratio, then 0.5 |
| master | first vertical, the rest horizontal | first split: master ratio, then equal rows |
| columns / rows | all vertical / all horizontal | equal |
| monocle | no splits | — |

Each workspace stores optional per-split ratio overrides and dwindle flips.
Unset splits keep their defaults, so they stay even when windows come and go.
Resizing a window with its borders (or `resizeactive`) rewrites only the splits
next to the moved edges and pins the others, so nothing else moves on screen.

Gaps: the area is grown by half a gap, partitioned, and every slot shrunk by
half a gap. Outer edges line up exactly with the work area and neighbours are
exactly one gap apart, odd gap sizes included.

## Source layout

`build.ps1` concatenates the fragments into the single file Windhawk needs.
Fragments never include each other; everything shared is declared in
`10_contract.h`.

| Fragment | Contents |
|---|---|
| `src/00_header.wh.cpp` | Windhawk metadata, readme and settings schema |
| `src/10_contract.h` | shared types, state and every cross-fragment declaration |
| `src/20_layout.cpp` | pure layout math, neighbour search, split resizing |
| `src/30_win32.cpp` | window classification, placement, focus, border, hide/show, logging |
| `src/40_config.cpp` | settings, key and command parsing, hotkey registration |
| `src/50_model.cpp` | monitors, workspaces, retiling and every command |
| `src/55_osd.cpp` | theme resolution and the center workspace indicator |
| `src/56_bar.cpp` | workspace badges on each taskbar |
| `src/60_engine.cpp` | engine thread, window events, shell hook, lifecycle |
| `src/90_toolmod.cpp` | Windhawk entry points and the tool-mod launcher |
| `tests/layout_test.cpp` | unit tests for the layout engine |
| `tests/standalone_main.cpp` | runs the engine as a plain exe for development |

## Known limitations

- Apps with a minimum window size (some browsers and Electron apps) can be
  larger than their tile.
- Windows running as administrator can't be moved unless Windhawk is elevated;
  they float instead.
- Alt-based hotkeys would open menu bars when Alt is released, so HawkTile sends
  a harmless "menu mask" key (`vk 0xE8`) after each one.
- Hidden windows are not on the taskbar while their workspace is inactive.
- Windows' own virtual desktops are not integrated yet: switching them makes
  HawkTile re-adopt the windows.

## Roadmap

- Live-test matrix: multiple monitors with mixed DPI, explorer restarts,
  sign-out.
- Reliability items from [the research notes](research-windhawk-mods.md):
  react to taskbar re-creation immediately, mark hidden windows with a window
  property, queue window events, Windows virtual desktop awareness, gentler
  restore of minimized Electron/Chromium windows.
- Optional companion mod for explorer.exe that frees Win+key combinations for
  binding.
