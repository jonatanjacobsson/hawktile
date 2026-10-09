// ==WindhawkMod==
// @id              hawktile
// @name            HawkTile - Tiling Window Manager
// @description     Window Tiling Manager. Stop dragging windows around. This tiles them for you, with per-monitor workspaces and Alt+ shortcuts.
// @version         0.8.0
// @author          Jonatan Jacobsson
// @github          https://github.com/jonatanjacobsson
// @homepage        https://github.com/jonatanjacobsson/hawktile
// @include         windhawk.exe
// @compilerOptions -ldwmapi -lshell32 -lshcore -lgdi32 -ladvapi32 -lgdiplus
// @license         MIT
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# HawkTile

A keyboard-driven tiling window manager for Windows. It runs as a Windhawk
tool mod in its own `windhawk.exe` process, so it never injects into Explorer
or your apps.

## Features
- Layouts: **dwindle** (binary splits), **master** (master-stack),
  **columns**, **rows**, **monocle**. Chosen per workspace.
- Up to 9 workspaces per monitor, switched independently on each monitor.
- Inner and outer gaps, adjustable split sizes.
- Directional focus and swap, crossing to the neighbouring monitor at the edge.
- Drag a tiled window onto another to swap them, or onto another monitor to
  move it there. Resize with the normal window borders and the neighbours
  follow.
- Workspace indicator on every taskbar, with notification dots.
- Float and ignore rules by process, window class or title.
- Focus border in a color of your choice (Windows 11).

## Default keys
Modelled on [Omarchy](https://learn.omacom.io/2/the-omarchy-manual/53/hotkeys)'s
Hyprland bindings, with Alt in place of Super.

| Keys | Action |
|---|---|
| Alt + Arrows | Focus the window in that direction |
| Alt + Shift + Arrows | Swap with the window in that direction |
| Alt + 1..9 | Switch workspace |
| Alt + Shift + 1..9 | Move window to workspace and follow it |
| Alt + W | Close window |
| Alt + F | Fullscreen (maximize) / restore |
| Alt + T | Toggle floating / tiling |
| Alt + J | Toggle split direction (dwindle) |
| Alt + L | Next layout |
| Alt + Equal / Minus | Grow the window to the left / right |
| Alt + Shift + Equal / Minus | Grow the window down / up |
| Alt + M | Toggle monocle |
| Alt + Shift + Enter | Swap with master |
| Alt + Shift + R | Re-scan windows and monitors |
| Alt + Shift + P | Pause / resume tiling |

Not bound by default because Windows already uses the keys: `workspace next` /
`workspace prev` (Alt + Tab is the Windows switcher) and
`movetoworkspacesilent <n>` (Ctrl + Alt is AltGr on many keyboards).

## Commands
Command names follow Hyprland's dispatchers. `<dir>` is left, right, up, down
(or l, r, u, d).

`movefocus <dir>`, `swapwindow <dir>`, `resizeactive <dir>`,
`workspace <1-9|next|prev>`, `movetoworkspace <n>` (you follow),
`movetoworkspacesilent <n>` (you stay), `focusmonitor next`,
`movetomonitor next`, `nextlayout`,
`layout <dwindle|master|columns|rows|monocle>`, `togglefloating`,
`togglemonocle`, `fullscreen`, `togglesplit`, `swapwithmaster`, `growmaster`,
`shrinkmaster`, `rescan`, `toggletiling`, `closewindow`.

Keys are written like `alt+shift+h`, `ctrl+alt+f1`, `win+alt+left`. Most
`win+<key>` combinations are reserved by Explorer and will fail to register;
failures are written to the mod log.

## Workspaces
Each monitor has its own workspaces. An indicator appears in the middle of the
monitor for a moment when you switch or move a window: the active workspace is
a filled circle in your accent color, workspaces with windows are bold, empty
ones are dim, and under each workspace you see the icons of its apps.

A compact version sits on the left side of every taskbar; click a number to
switch. When an app on another workspace flashes in the taskbar (a new
message, a call), a small dot appears on that workspace until you open the
app. Clicking a Windows notification of an app on another workspace switches
there and opens the app. Everything follows your Windows theme: light or dark
mode, your accent color, and high contrast.

## Maximized windows
Windows that are maximized when HawkTile picks them up are restored and tiled.
Maximizing a tiled window afterwards (Alt + F or the maximize button) lifts it
out of the layout until you restore it.

## Rules
`process:notepad.exe`, `class:#32770`, `title:Picture-in-Picture`. Without a
prefix the rule matches the process name. Dialogs, owned windows,
non-resizable windows and always-on-top windows float automatically.

## Safety
Windows on hidden workspaces are hidden, not closed. Disabling the mod shows
them all again. If the process is killed, they are shown again the next time
the mod starts.
*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
- layout:
  - default: dwindle
    $name: Default layout
    $options:
    - dwindle: Dwindle (binary splits)
    - master: Master-stack
    - columns: Columns
    - rows: Rows
    - monocle: Monocle
  - workspaces: 9
    $name: Workspaces per monitor
    #! $min: 1
    #! $max: 9
  - outerGap: 8
    $name: Outer gap (px)
    $description: Space between the windows and the screen edges
    #! $min: 0
    #! $max: 200
  - innerGap: 8
    $name: Inner gap (px)
    $description: Space between neighbouring windows
    #! $min: 0
    #! $max: 200
  - masterRatio: 55
    $name: Master size (%)
    $description: Share of the screen the first window gets
    #! $min: 15
    #! $max: 85
    #! $format: slider
  - resizeStep: 5
    $name: Resize step (%)
    $description: How far the resize keys move a window's edge
    #! $min: 1
    #! $max: 25
  $name: Layout and gaps
- indicators:
  - taskbar: true
    $name: Workspace badges on the taskbar
    $description: Small clickable workspace numbers on the left side of each taskbar
  - taskbarOffset: 12
    $name: Taskbar badge offset (px)
    $description: Distance from the left edge of the taskbar; increase it if your taskbar icons are left-aligned
    #! $min: 0
    #! $max: 2000
  - popup: true
    $name: Workspace popup
    $description: Briefly show a monitor's workspaces and their apps when you switch or move a window
  - popupDuration: 2000
    $name: Popup duration (ms)
    #! $min: 100
    #! $max: 10000
  - notificationDots: true
    $name: Notification dots
    $description: Show a dot on a workspace when one of its apps flashes in the taskbar
  $name: Workspace indicators
- windows:
  - tileMaximized: true
    $name: Tile maximized windows
    $description: Restore and tile windows that are maximized when HawkTile picks them up
  - floatNonResizable: true
    $name: Float non-resizable windows
  - floatTopmost: true
    $name: Float always-on-top windows
  - followActivation: true
    $name: Follow activated windows
    $description: When a window on another workspace brings itself to the front (for example after you click its notification), switch to that workspace
  $name: Window handling
- border:
  - enabled: true
    $name: Focus border
    $description: Color the border of the focused window (Windows 11 only)
  - color: "4C9AFF"
    $name: Border color
    $description: Hex color RRGGBB
    #! $format: colorRgb
  $name: Focus border
- rules:
  - float:
    - class:#32770
    - class:OperationStatusWindow
    - title:Picture-in-Picture
    $name: Float rules
    $description: Windows that float instead of tiling. process:<exe>, class:<class>, or title:<text in the title>
  - ignore: [""]
    $name: Ignore rules
    $description: Windows HawkTile never touches. Same syntax as the float rules
  $name: Rules
- binds:
  - - keys: alt+left
    - command: movefocus left
  - - keys: alt+right
    - command: movefocus right
  - - keys: alt+up
    - command: movefocus up
  - - keys: alt+down
    - command: movefocus down
  - - keys: alt+shift+left
    - command: swapwindow left
  - - keys: alt+shift+right
    - command: swapwindow right
  - - keys: alt+shift+up
    - command: swapwindow up
  - - keys: alt+shift+down
    - command: swapwindow down
  - - keys: alt+1
    - command: workspace 1
  - - keys: alt+2
    - command: workspace 2
  - - keys: alt+3
    - command: workspace 3
  - - keys: alt+4
    - command: workspace 4
  - - keys: alt+5
    - command: workspace 5
  - - keys: alt+6
    - command: workspace 6
  - - keys: alt+7
    - command: workspace 7
  - - keys: alt+8
    - command: workspace 8
  - - keys: alt+9
    - command: workspace 9
  - - keys: alt+shift+1
    - command: movetoworkspace 1
  - - keys: alt+shift+2
    - command: movetoworkspace 2
  - - keys: alt+shift+3
    - command: movetoworkspace 3
  - - keys: alt+shift+4
    - command: movetoworkspace 4
  - - keys: alt+shift+5
    - command: movetoworkspace 5
  - - keys: alt+shift+6
    - command: movetoworkspace 6
  - - keys: alt+shift+7
    - command: movetoworkspace 7
  - - keys: alt+shift+8
    - command: movetoworkspace 8
  - - keys: alt+shift+9
    - command: movetoworkspace 9
  - - keys: alt+w
    - command: closewindow
  - - keys: alt+f
    - command: fullscreen
  - - keys: alt+t
    - command: togglefloating
  - - keys: alt+j
    - command: togglesplit
  - - keys: alt+l
    - command: nextlayout
  - - keys: alt+equals
    - command: resizeactive left
  - - keys: alt+minus
    - command: resizeactive right
  - - keys: alt+shift+equals
    - command: resizeactive down
  - - keys: alt+shift+minus
    - command: resizeactive up
  - - keys: alt+m
    - command: togglemonocle
  - - keys: alt+shift+enter
    - command: swapwithmaster
  - - keys: alt+shift+r
    - command: rescan
  - - keys: alt+shift+p
    - command: toggletiling
  $name: Key bindings
  $description: Each entry is a key combination and a command (see the mod details for the list)
- advanced:
  - debugLog: false
    $name: Debug log file
    $description: Write hawktile.log to the mod's storage folder (for troubleshooting)
  $name: Advanced
*/
// ==/WindhawkModSettings==

