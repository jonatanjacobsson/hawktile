# Changelog

## 0.9.0

- Hold the left Alt key to open the workspace overview. It stays open while
  you hold Alt: drag an app's icon onto another workspace to move that app's
  windows there, click a number to switch, or click an icon to go to the app.
  The right Alt key (AltGr) never opens it. New settings under Workspace
  indicators: *Hold Alt for the workspace overview* (on) and *Hold delay*
  (400 ms).

## 0.8.0 — first public release

- Settings are grouped into sections: layout and gaps, workspace indicators,
  window handling, focus border, rules, key bindings and advanced.
- Command names follow Hyprland's dispatchers (`movefocus`, `swapwindow`,
  `movetoworkspace`, `togglefloating`, `fullscreen`, `togglesplit`, …).
- Default keys follow Omarchy's bindings, with Alt in place of Super.
- Follows Windhawk's mod guide: `@github` / `@homepage` metadata, Windhawk 2.0
  settings hints (ranges, slider, color picker) that Windhawk 1.7 ignores, and
  shutdown-safe global state.
- Logging goes straight to Windhawk's log and costs nothing while logging is
  off; an optional debug log file is available.
- Hidden workspace windows are shown again when you sign out or shut down.

Upgrading from a development build (0.1–0.7): the settings were reorganised,
so HawkTile starts from the new defaults once.

## Development versions (0.1 – 0.7)

- Dwindle, master, columns, rows and monocle layouts with gaps; up to 9
  workspaces per monitor; directional focus and swap across monitors.
- Maximized windows are restored and tiled; `fullscreen` lifts a window out of
  the layout.
- Moving a window to a workspace follows it; a silent variant stays.
- Center workspace popup with app icons, following the Windows theme.
- Resizing tiled windows with their borders or from the keyboard; flipping
  dwindle splits.
- Clickable workspace badges on every taskbar, hidden for fullscreen apps and
  auto-hidden taskbars.
- Notification dots on workspaces whose apps flash in the taskbar; clicking a
  notification switches to the app's workspace.
- Recompiling the mod no longer leaves it stopped (the new process waits for
  the old one).
