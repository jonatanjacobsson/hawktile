# Changelog

## 0.12.0

- New layout: **mainstage**, made for wide screens. The window you work in
  sits on a stage in the middle of the screen; the workspace's other windows
  wait at the sides as small live pictures. Each side stacks up to three; with
  more, the first two stay large and the rest share the last place in a small
  grid. Drag a side tile onto the stage, or move focus to it, and
  it swaps onto the stage. Drag tiles onto each other or into free space to
  arrange the two sides, and drag the stage window by its title bar to a side
  to put it away. Clicks, right-clicks and the mouse wheel on a tile go into
  the window it shows (most apps, not all). Hold the left Alt key over a tile
  for half a second to read it: the tile shows that spot of the window at
  full size, the cursor stays put and moving the mouse drags the picture,
  until you let go of Alt or drag the tile away. Pick the layout with
  **Alt + L**, `layout mainstage`, or as the default layout. The existing
  layouts are unchanged.
- Hold Alt while dragging a window by its title bar (any layout) or a
  mainstage side tile, and drop it on a workspace in the overview to move it
  there.
- **Alt + Period** joins Alt + Minus as a resize pair on neighbouring keys
  (`resizeactive left`). In mainstage, Alt + Minus widens the stage and
  Alt + Period narrows it. Existing installs keep their saved key bindings,
  so add `alt+period` → `resizeactive left` under *Key bindings* to use it.
- New settings under Layout and gaps: *Layout per workspace* (give single
  workspaces their own layout, for example mainstage on workspace 1 only) and
  *Mainstage width* (50 %).

## 0.11.0

- **Alt + Shift + T** (`togglealwaysfloat`): never tile this app again. Every
  window of the focused app floats, now and in future sessions; press it again
  on that app to put it back into the layout. Existing installs keep their
  saved key bindings, so add `alt+shift+t` → `togglealwaysfloat` under
  *Key bindings* to use it.

## 0.10.0

- Workspace overview: point at a workspace while holding Alt and let go to
  jump there. The column under the mouse is highlighted as a whole, and a
  click anywhere in a column switches to it.
- Hold left Alt + Shift and move the mouse to resize the active window,
  tiled or floating, without clicking. The edges on the side where the cursor
  started follow it; the other tiled windows make room when you let go. New
  setting under Window handling: *Resize with Alt+Shift and the mouse* (on).
- Fix: some apps (Revit, for example) kept drawing at their old size after
  being shown on a workspace switch or restored from maximized, leaving the
  rest of the window black. HawkTile now re-sends their size a moment later.

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
