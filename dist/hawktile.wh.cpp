// ==WindhawkMod==
// @id              hawktile
// @name            HawkTile - Tiling Window Manager
// @description     Window Tiling Manager. Stop dragging windows around. This tiles them for you, with per-monitor workspaces and Alt+ shortcuts.
// @version         0.12.0
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
  **columns**, **rows**, **monocle**, **mainstage** (one window on a centre
  stage, the others as small live tiles at the sides). Chosen per workspace.
- Up to 9 workspaces per monitor, switched independently on each monitor.
- Inner and outer gaps, adjustable split sizes.
- Directional focus and swap, crossing to the neighbouring monitor at the edge.
- Drag a tiled window onto another to swap them, or onto another monitor to
  move it there. Resize with the normal window borders and the neighbours
  follow, or hold Alt + Shift and just move the mouse.
- Workspace indicator on every taskbar, with notification dots.
- Hold Alt to see every workspace and drag apps between them.
- Float and ignore rules by process, window class or title.
- Focus border in a color of your choice (Windows 11).

## Default keys
Modelled on [Omarchy](https://learn.omacom.io/2/the-omarchy-manual/53/hotkeys)'s
Hyprland bindings, with Alt in place of Super.

| Keys | Action |
|---|---|
| Alt + Arrows | Focus the window in that direction |
| Alt + Shift + Arrows | Swap with the window in that direction |
| Hold left Alt | Workspace overview: point at a workspace and let go to jump there, or drag app icons between workspaces |
| Hold left Alt + Shift, move the mouse | Resize the active window from the side the cursor is on |
| Alt + 1..9 | Switch workspace |
| Alt + Shift + 1..9 | Move window to workspace and follow it |
| Alt + W | Close window |
| Alt + F | Fullscreen (maximize) / restore |
| Alt + T | Toggle floating / tiling |
| Alt + Shift + T | Never tile this app again (press again to undo) |
| Alt + J | Toggle split direction (dwindle) |
| Alt + L | Next layout |
| Alt + Equal / Minus | Grow the window to the left / right |
| Alt + Period / Minus | The same pair on neighbouring keys. In mainstage: narrow / widen the stage |
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
`layout <dwindle|master|columns|rows|monocle|mainstage>`, `togglefloating`,
`togglemonocle`, `fullscreen`, `togglesplit`, `swapwithmaster`, `growmaster`,
`shrinkmaster`, `rescan`, `toggletiling`, `closewindow`,
`togglealwaysfloat` (remember the focused window's app as never tiled; again to undo).

Keys are written like `alt+shift+h`, `ctrl+alt+f1`, `win+alt+left`. Most
`win+<key>` combinations are reserved by Explorer and will fail to register;
failures are written to the mod log.

## Workspaces
Each monitor has its own workspaces. An indicator appears in the middle of the
monitor for a moment when you switch or move a window: the active workspace is
a filled circle in your accent color, workspaces with windows are bold, empty
ones are dim, and under each workspace you see the icons of its apps.

Hold the left Alt key for a moment and the same panel opens and stays while
you hold it. Drag an app's icon onto another workspace to move that app's
windows there, click a number to switch, or click an icon to go to the app.
Let go of Alt to close it; if the mouse is over a workspace when you let go,
you jump there. The right Alt key (AltGr) never opens it.

A compact version sits on the left side of every taskbar; click a number to
switch. When an app on another workspace flashes in the taskbar (a new
message, a call), a small dot appears on that workspace until you open the
app. Clicking a Windows notification of an app on another workspace switches
there and opens the app. Everything follows your Windows theme: light or dark
mode, your accent color, and high contrast.

## Mainstage
Made for wide screens: the window you work in sits on a stage in the middle
of the screen, and the workspace's other windows wait at the sides as small
live pictures. Each side stacks up to three; with more, the first two stay
large and the rest share the last place in a small grid.
Drag a side tile onto the stage, or move focus to it with Alt + Arrows, and
it swaps onto the stage; the window that was there takes its place at the
side. A window you open or switch to (Alt + Tab, the taskbar) goes straight
to the stage.

Arrange the sides with the mouse: drag a tile onto another tile to swap them,
or into free space on either side to move it to the end of that side. Drag
the stage window by its title bar onto a tile to trade places with it, or
into free space at a side to put it away there. Hold Alt while dragging a
tile, or any window by its title bar, and drop it on a workspace of the
overview to send it there.

Clicks, right-clicks and the mouse wheel on a tile go into the window it
shows, at the matching spot, without bringing it to the stage. This works
with most apps, not all: the tile is a live picture of a window that stays
stacked on the stage at full size, so typing still needs the stage, and
dragging inside a tile moves the tile.

Pick the layout with Alt + L, with `layout mainstage`, as the default layout,
or for single workspaces under *Layout per workspace*. Alt + Minus widens the
stage and Alt + Period narrows it; so do the stage window's border and the
*Mainstage width* setting.

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
    - mainstage: Mainstage (centre stage, live tiles at the sides)
  - workspaceLayouts: [""]
    $name: Layout per workspace
    $description: Optional. The first entry is the layout of workspace 1, the second of workspace 2, and so on. Write dwindle, master, columns, rows, monocle or mainstage. Leave an entry empty (or write default) to use the default layout
  - stageWidth: 50
    $name: Mainstage width (%)
    $description: Share of the screen width the stage takes in the mainstage layout. The rest is split between the two sides
    #! $min: 20
    #! $max: 90
    #! $format: slider
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
  - holdAlt: true
    $name: Hold Alt for the workspace overview
    $description: Hold the left Alt key to show the workspaces and their apps. Drag an app's icon onto another workspace to move its windows there, or click a number to switch
  - holdDelay: 400
    $name: Hold delay (ms)
    $description: How long the left Alt key must be held before the overview opens
    #! $min: 100
    #! $max: 2000
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
  - mouseResize: true
    $name: Resize with Alt+Shift and the mouse
    $description: Hold the left Alt key and Shift and move the mouse (no click) to resize the active window, tiled or floating, from the side the cursor is on. Other tiled windows make room when you let go
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
    $description: Windows that float instead of tiling. process:<exe>, class:<class>, or title:<text in the title>. Apps you float with Alt+Shift+T are remembered separately; press it again on the app to undo
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
  - - keys: alt+shift+t
    - command: togglealwaysfloat
  - - keys: alt+j
    - command: togglesplit
  - - keys: alt+l
    - command: nextlayout
  - - keys: alt+equals
    - command: resizeactive left
  - - keys: alt+period
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


// =============================================================================
// 10_contract.h — shared contract for all HawkTile fragments.
//
// build.ps1 concatenates the fragments in order into ONE translation unit:
//   00_header.wh.cpp, 10_contract.h, 20_layout.cpp, 30_win32.cpp,
//   40_config.cpp, 50_model.cpp, 55_osd.cpp, 56_bar.cpp, 57_stage.cpp,
//   60_engine.cpp, 90_toolmod.cpp
// Fragments therefore never #include each other. Everything a fragment needs
// from another fragment is declared here. Do not change these signatures in a
// fragment; if a change is needed, it is made here first.
//
// Threading rule: every function below runs on the engine thread unless the
// comment says otherwise. No locks are used anywhere.
// =============================================================================

#include <windows.h>
#include <dwmapi.h>
#include <shellapi.h>

#include <algorithm>
#include <cstdio>
#include <cwchar>
#include <cwctype>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// HT_LOG is a statement. Normally it is plain Wh_Log, whose arguments are only
// evaluated when Windhawk logging is on and which records line and function.
// Only while the "debug log file" setting is on does it go through LogLine,
// which formats the text itself so it can also append it to hawktile.log.
#ifdef HT_STANDALONE
#define HT_LOG(fmt, ...) (fwprintf(stderr, fmt L"\n", ##__VA_ARGS__))
#else
#define HT_LOG(fmt, ...)                       \
    do {                                       \
        if (::ht::DebugLogFileEnabled()) {     \
            ::ht::LogLine(fmt, ##__VA_ARGS__); \
        } else {                               \
            Wh_Log(fmt, ##__VA_ARGS__);        \
        }                                      \
    } while (0)
#endif

namespace ht {

// 30_win32.cpp. printf-style (wide). Formats with _vsnwprintf into a
// 1024-char buffer (truncating), forwards to Wh_Log(L"%s", buf), and when
// g.settings.debugLog is set also appends "HH:MM:SS.mmm <text>\r\n" (UTF-8) to
// "<Wh_GetModStoragePath>\hawktile.log" (directory created if missing; file
// deleted first when it exceeds 2 MB). May be called from any thread.
void LogLine(const wchar_t* format, ...);

// 30_win32.cpp. Returns g.settings.debugLog (read without locking; a stale
// value only routes one log line the other way).
bool DebugLogFileEnabled();

// ---- DWM constants (defined here so older SDK headers are not a problem) ----
inline constexpr DWORD kDwmExtendedFrameBounds = 9;   // DWMWA_EXTENDED_FRAME_BOUNDS
inline constexpr DWORD kDwmCloaked = 14;              // DWMWA_CLOAKED
inline constexpr DWORD kDwmBorderColor = 34;          // DWMWA_BORDER_COLOR (Win11)
inline constexpr COLORREF kDwmColorDefault = 0xFFFFFFFF;  // DWMWA_COLOR_DEFAULT

// ---- Engine window plumbing --------------------------------------------------
inline constexpr wchar_t kEngineWindowClass[] = L"HawkTileEngine";
inline constexpr UINT WM_HT_RELOAD = WM_APP + 1;      // posted by NotifySettingsChanged
inline constexpr UINT_PTR kTimerRetile = 1;           // debounce, 15 ms
inline constexpr UINT_PTR kTimerPending = 2;          // deferred window checks, 80 ms
inline constexpr UINT_PTR kTimerDisplay = 3;          // monitor topology change, 400 ms
inline constexpr UINT_PTR kTimerSettle = 5;           // one-off retile after startup, 300 ms
inline constexpr UINT_PTR kTimerBar = 6;              // coalesced taskbar-indicator refresh, 30 ms
inline constexpr UINT_PTR kTimerBarPoll = 7;          // periodic taskbar-indicator check, 1000 ms
inline constexpr UINT_PTR kTimerHold = 8;             // left Alt held long enough → overview, settings.holdDelayMs
inline constexpr UINT_PTR kTimerHoldPoll = 9;         // overview open: is left Alt still down? 100 ms
inline constexpr UINT_PTR kTimerMouseResize = 10;     // Alt+Shift held: follow the mouse, 16 ms
inline constexpr UINT_PTR kTimerNudge = 11;           // re-send sizes to g.needsNudge, 300 ms (coalesced)
inline constexpr UINT_PTR kTimerStage = 12;           // re-fit the Mainstage side tiles, 250 ms (coalesced)
inline constexpr int kHotkeyIdBase = 0x100;          // hotkey id = base + binding index
inline constexpr BYTE kMenuMaskVk = 0xE8;             // unassigned VK used to mask Alt/Win

// =============================================================================
// Layout (20_layout.cpp) — pure functions, no Win32 calls except RECT type.
// =============================================================================

enum class LayoutKind { Dwindle, MasterStack, Columns, Rows, Monocle, Mainstage, Count_ };
enum class Direction { Left, Right, Up, Down };

struct LayoutParams {
    LayoutKind kind = LayoutKind::Dwindle;
    double masterRatio = 0.55;  // default ratio of split 0 for Dwindle/MasterStack
    int innerGap = 8;           // pixels between neighbouring windows
    // Per-split overrides: entry i applies to split i when 0 < r < 1. Any other
    // value (e.g. 0.0) means "use the default", so unset splits stay even when
    // windows are added or removed.
    std::vector<double> ratios;
    // Dwindle only: flips[i] != 0 inverts the automatic orientation of split i
    // (vertical ↔ horizontal). Ignored by the other layouts.
    std::vector<int> flips;
    // Mainstage only: default share of the area's width the stage takes.
    double stageRatio = 0.5;
    // Mainstage only: sides[j] != 0 puts side window j (layout index j + 1) on
    // the right of the stage, 0 on the left. Side windows beyond the vector
    // alternate: even j left, odd j right.
    std::vector<int> sides;
};

// Returns exactly `count` rects tiling `area` (count <= 0 → empty vector).
// Gap rule: outer edges of the result coincide with `area`; neighbouring
// rects are separated by exactly innerGap px. Every rect has width/height >= 1.
// (Mainstage follows its own rules, below.)
//
// Every layout except Monocle and Mainstage is a sequence of count-1 binary splits of the
// gap-expanded area. Split i cuts the remaining region; window i takes side A
// (the left part of a vertical cut, the top part of a horizontal cut), windows
// i+1.. continue in side B. The last window takes what remains. Orientation:
//  - Dwindle:     vertical if the remaining region is wider than tall, else
//                 horizontal; inverted when p.flips[i] != 0.
//  - MasterStack: split 0 vertical (master on the left), the rest horizontal.
//  - Columns:     all vertical.          - Rows: all horizontal.
//  - Monocle:     no splits; every rect == area.
// The ratio of split i is SplitRatio(p, count, i).
// Cut position = lo + lround((hi - lo) * ratio) on the remaining region's extent.
//
// Mainstage: rect 0 is the STAGE, the full height of `area`, horizontally
// centred, lround(width * SplitRatio(p, count, 0)) wide. Rects 1.. are SIDE
// CELLS: side window j (rect j + 1) goes left or right of the stage as
// p.sides says, and each side fills in order. A side is the strip between the
// area's edge and the stage, innerGap away from the stage. It is divided into
// three equal SLOTS stacked from the top, innerGap apart. With up to three
// windows on a side each gets a slot. With more, the first two keep their
// slots and the rest share the last slot in a grid of boxes, innerGap apart,
// filled row by row: columns = the smallest c with c * c >= rest, rows =
// ceil(rest / c) (2 side by side, then 2x2, 3x2, 3x3, ...). A cell is the
// largest rect with the stage's aspect ratio inside its slot or box, centred,
// so a window sized for the stage scales into its cell without bars. All
// rects lie inside `area`.
std::vector<RECT> ComputeLayout(const RECT& area, int count, const LayoutParams& p);

// Effective ratio of split i: p.ratios[i] if present and 0 < r < 1 (clamped to
// [0.05, 0.95]), otherwise the default:
//   Dwindle:      i == 0 ? masterRatio : 0.5
//   MasterStack:  i == 0 ? masterRatio : 1.0 / (count - i)   (equal stack rows)
//   Columns/Rows: 1.0 / (count - i)                          (equal slots)
// masterRatio is clamped to [0.05, 0.95]. Returns 0.5 for out-of-range i.
// Mainstage has one ratio, "split" 0 for any count: the stage's share of the
// width, p.ratios[0] if set, else p.stageRatio, clamped to [0.2, 0.9].
double SplitRatio(const LayoutParams& p, int count, int split);

// The user resized tiled window `index` (its position in the same window list
// that produced ComputeLayout(area, count, p)) so its visible frame is now
// `newRect`. Rewrites p.ratios so the cut lines adjacent to the window follow
// its new edges:
//  - split `index` (if index < count-1): the window is side A; its right edge
//    (vertical cut) or bottom edge (horizontal cut) lies on the cut.
//  - every split i < index whose cut lies exactly on the window's slot edge:
//    the window is in side B; its left edge (vertical) or top edge (horizontal).
// An edge counts as moved when it differs from the current slot edge by more
// than 2 px. Only the dragged edges may move on screen; every other cut keeps
// its absolute position:
//  1. Walk the current geometry; record every split's orientation and
//     absolute cut. Desired cut = the new edge (through the same gap offsets
//     ComputeLayout uses) for moved splits, the old cut for all others.
//  2. Walk again from split 0 upward, recomputing each split's remaining
//     region with the ratios written so far. Moved split → write the ratio
//     that puts the cut at its desired position, clamped to [0.1, 0.9].
//     Unmoved split whose orientation is unchanged but whose cut would now
//     land elsewhere (>= 1 px) → write a pinning ratio for its old position
//     (clamped to [0.05, 0.95]). Any other split keeps its current entry, so
//     untouched splits stay at 0.0 ("default") and stay even when windows
//     are added or removed.
// p.ratios is padded with 0.0 up to count-1 entries first. Returns true if
// any ratio changed. Monocle, count < 2 or a bad index → false, p unchanged.
// Mainstage: only index 0 (the stage) can be resized, with any count >= 1. A
// width change of more than 2 px against the current stage rect is applied
// twice (the stage stays centred, so the dragged edge ends where it was put):
// p.ratios[0] = (stageWidth + 2 * change) / areaWidth, clamped to [0.2, 0.9].
bool ApplyResize(const RECT& area, int count, LayoutParams& p, int index,
                 const RECT& newRect);

// Dwindle only ("toggle split"): flips the orientation of the split that
// window `index` owns — split `index` when index < count-1, else split
// count-2 (the last window shares the previous split). Pads p.flips with 0 to
// count-1 entries. Returns false (p unchanged) for other layouts, count < 2
// or a bad index.
bool ToggleSplitAt(LayoutParams& p, int count, int index);

// Index of the rect closest to rects[from] in direction `dir`, or -1.
// Candidate: its center lies strictly in `dir` from the center of rects[from]
// (e.g. Left: candidate.centerX < from.centerX). Ranking: prefer candidates
// whose perpendicular extent overlaps rects[from]; then smallest distance along
// `dir` between centers; then smallest perpendicular center distance; then
// lowest index. Rects identical to rects[from] are never candidates.
int FindNeighbor(const std::vector<RECT>& rects, int from, Direction dir);

LayoutKind NextLayout(LayoutKind k);           // cycles, skipping Count_
const wchar_t* LayoutName(LayoutKind k);       // "dwindle","master","columns","rows","monocle","mainstage"
bool ParseLayoutName(const wchar_t* s, LayoutKind* out);  // case-insensitive, inverse of LayoutName

// =============================================================================
// Configuration (40_config.cpp)
// =============================================================================

// Command vocabulary follows Hyprland's dispatcher names where one exists.
enum class CommandKind {
    None,
    MoveFocus,              // dir — focus the neighbour (crosses monitors)
    SwapWindow,             // dir — swap with the neighbour (crosses monitors)
    Workspace,              // arg = 1-based workspace number
    WorkspaceNext,          // next workspace on the focused monitor (wraps)
    WorkspacePrev,          // previous workspace on the focused monitor (wraps)
    MoveToWorkspace,        // arg; moves the window there AND follows it
    MoveToWorkspaceSilent,  // arg; moves the window there, you stay
    FocusMonitorNext,
    MoveToMonitorNext,
    NextLayout,
    SetLayout,              // layout
    ToggleFloating,
    ToggleMonocle,
    Fullscreen,             // maximize / restore (out of the layout while maximized)
    ToggleSplit,            // dwindle: flip the focused window's split orientation
    SwapWithMaster,
    GrowMaster,
    ShrinkMaster,
    ResizeActive,           // dir: push the focused window's boundary toward dir
                            // (Mainstage: right/up widen the stage, left/down
                            // narrow it, by settings.ratioStep, within 0.2..0.9)
    Rescan,                 // re-read monitors, adopt new windows, retile
    ToggleTiling,           // pause / resume tiling
    CloseWindow,
    ToggleAlwaysFloat,      // remember / forget the focused window's app as "never tile"
};

struct Command {
    CommandKind kind = CommandKind::None;
    Direction dir = Direction::Left;
    int arg = 0;
    LayoutKind layout = LayoutKind::Dwindle;
};

struct Binding {
    UINT modifiers = 0;  // MOD_ALT | MOD_CONTROL | MOD_SHIFT | MOD_WIN (MOD_NOREPEAT added at registration)
    UINT vk = 0;
    Command cmd;
    std::wstring keysText;     // original text, for logging
    std::wstring commandText;  // original text, for logging
};

enum class RuleField { Process, Class, Title };

struct MatchRule {
    RuleField field = RuleField::Process;
    std::wstring pattern;  // stored lowercase
};

// Each field's Windhawk setting path is given on the right. Settings are
// grouped in the YAML (00_header.wh.cpp), so most paths are "group.key".
struct Settings {
    int outerGap = 8;                // layout.outerGap, clamped 0..200
    int innerGap = 8;                // layout.innerGap, clamped 0..200
    int workspaceCount = 9;          // layout.workspaces, clamped 1..9
    LayoutKind defaultLayout = LayoutKind::Dwindle;  // layout.default
    double masterRatio = 0.55;       // layout.masterRatio (percent, 55), clamped 15..85
    double ratioStep = 0.05;         // layout.resizeStep (percent, 5), clamped 1..25
    double stageRatio = 0.5;         // layout.stageWidth (percent, 50), clamped 20..90
    // layout.workspaceLayouts[i]: layout of workspace i + 1 on every monitor.
    // Count_ = "use defaultLayout" (empty, "default" or unknown text). Up to 9.
    std::vector<LayoutKind> workspaceLayouts;
    bool focusBorder = true;         // border.enabled
    COLORREF focusBorderColor = RGB(0x4C, 0x9A, 0xFF);  // border.color ("RRGGBB" or "#RRGGBB")
    bool floatNonResizable = true;   // windows.floatNonResizable
    bool floatTopmost = true;        // windows.floatTopmost
    bool tileMaximized = true;       // windows.tileMaximized
    bool workspaceOsd = true;        // indicators.popup
    int osdDurationMs = 2000;        // indicators.popupDuration; <= 0 → 2000, clamped 100..10000
    bool holdOverview = true;        // indicators.holdAlt
    int holdDelayMs = 400;           // indicators.holdDelay; <= 0 → 400, clamped 100..2000
    bool mouseResize = true;         // windows.mouseResize (Alt+Shift + move the mouse)
    bool taskbarIndicator = true;    // indicators.taskbar
    bool attentionDots = true;       // indicators.notificationDots
    bool followActivation = true;    // windows.followActivation
    int taskbarIndicatorOffset = 12; // indicators.taskbarOffset (96-dpi px), clamped 0..2000
    bool debugLog = false;           // advanced.debugLog
    std::vector<MatchRule> floatRules;   // rules.float[i]
    std::vector<MatchRule> ignoreRules;  // rules.ignore[i]
    std::vector<Binding> bindings;       // binds[i].keys / binds[i].command
};

// Default tables. The YAML defaults in 00_header.wh.cpp mirror these exactly.
struct DefaultBinding {
    const wchar_t* keys;
    const wchar_t* command;
};

inline constexpr DefaultBinding kDefaultBindings[] = {
    {L"alt+left", L"movefocus left"},
    {L"alt+right", L"movefocus right"},
    {L"alt+up", L"movefocus up"},
    {L"alt+down", L"movefocus down"},
    {L"alt+shift+left", L"swapwindow left"},
    {L"alt+shift+right", L"swapwindow right"},
    {L"alt+shift+up", L"swapwindow up"},
    {L"alt+shift+down", L"swapwindow down"},
    {L"alt+1", L"workspace 1"},
    {L"alt+2", L"workspace 2"},
    {L"alt+3", L"workspace 3"},
    {L"alt+4", L"workspace 4"},
    {L"alt+5", L"workspace 5"},
    {L"alt+6", L"workspace 6"},
    {L"alt+7", L"workspace 7"},
    {L"alt+8", L"workspace 8"},
    {L"alt+9", L"workspace 9"},
    {L"alt+shift+1", L"movetoworkspace 1"},
    {L"alt+shift+2", L"movetoworkspace 2"},
    {L"alt+shift+3", L"movetoworkspace 3"},
    {L"alt+shift+4", L"movetoworkspace 4"},
    {L"alt+shift+5", L"movetoworkspace 5"},
    {L"alt+shift+6", L"movetoworkspace 6"},
    {L"alt+shift+7", L"movetoworkspace 7"},
    {L"alt+shift+8", L"movetoworkspace 8"},
    {L"alt+shift+9", L"movetoworkspace 9"},
    {L"alt+w", L"closewindow"},
    {L"alt+f", L"fullscreen"},
    {L"alt+t", L"togglefloating"},
    {L"alt+shift+t", L"togglealwaysfloat"},
    {L"alt+j", L"togglesplit"},
    {L"alt+l", L"nextlayout"},
    {L"alt+equals", L"resizeactive left"},
    {L"alt+period", L"resizeactive left"},
    {L"alt+minus", L"resizeactive right"},
    {L"alt+shift+equals", L"resizeactive down"},
    {L"alt+shift+minus", L"resizeactive up"},
    {L"alt+m", L"togglemonocle"},
    {L"alt+shift+enter", L"swapwithmaster"},
    {L"alt+shift+r", L"rescan"},
    {L"alt+shift+p", L"toggletiling"},
};

inline constexpr const wchar_t* kDefaultFloatRules[] = {
    L"class:#32770",
    L"class:OperationStatusWindow",
    L"title:Picture-in-Picture",
};

// Fills `s` with defaults, then (unless HT_STANDALONE is defined) overrides from
// Windhawk settings via Wh_GetIntSetting / Wh_GetStringSetting. Invalid entries
// are logged with HT_LOG and skipped. If the bindings list read from settings
// is empty, kDefaultBindings is used. Strings from Wh_GetStringSetting are
// always released with Wh_FreeStringSetting.
void LoadSettings(Settings& s);

// "alt+shift+h" → MOD_ALT|MOD_SHIFT, 'H'. Case-insensitive, tokens separated by
// '+', surrounding spaces ignored. Modifiers: alt, ctrl|control, shift,
// win|super. Exactly one non-modifier key is required. Keys: a-z, 0-9, f1-f24,
// left right up down, enter|return, space, tab, escape|esc, backspace,
// delete|del, insert|ins, home, end, pageup|pgup, pagedown|pgdn, minus
// (VK_OEM_MINUS), plus|equals (VK_OEM_PLUS), comma, period, semicolon (VK_OEM_1),
// slash (VK_OEM_2), backtick|grave (VK_OEM_3), bracketleft (VK_OEM_4),
// backslash (VK_OEM_5), bracketright (VK_OEM_6), quote (VK_OEM_7),
// and "vkXX" with XX a hex virtual-key code. Returns false on any error.
bool ParseHotkey(const wchar_t* text, UINT* modifiers, UINT* vk);

// Command grammar (case-insensitive, single spaces between words, trimmed).
// <dir> = left|right|up|down or l|r|u|d; N = 1..9.
//   movefocus <dir>          swapwindow <dir>         resizeactive <dir>
//   workspace N|next|prev    movetoworkspace N        movetoworkspacesilent N
//   focusmonitor next        movetomonitor next
//   nextlayout               layout dwindle|master|columns|rows|monocle|mainstage
//   togglefloating  togglemonocle  fullscreen  togglesplit  swapwithmaster
//   growmaster  shrinkmaster  rescan  toggletiling  closewindow
//   togglealwaysfloat
bool ParseCommand(const wchar_t* text, Command* out);

// "process:notepad.exe" | "class:#32770" | "title:Picture-in-Picture".
// No prefix means process. Empty text → false.
bool ParseRule(const wchar_t* text, MatchRule* out);

struct WindowFacts {
    HWND hwnd = nullptr;
    DWORD pid = 0;
    std::wstring processName;  // lowercase file name, e.g. "notepad.exe" ("" if unknown)
    std::wstring className;    // as returned by GetClassNameW
    std::wstring title;        // as returned by GetWindowTextW
    LONG_PTR style = 0;
    LONG_PTR exStyle = 0;
    HWND owner = nullptr;      // GetWindow(hwnd, GW_OWNER)
    bool cloaked = false;
};

// Process: exact match (case-insensitive). Class: exact match
// (case-insensitive). Title: substring match (case-insensitive).
bool RuleMatches(const MatchRule& rule, const WindowFacts& facts);
bool AnyRuleMatches(const std::vector<MatchRule>& rules, const WindowFacts& facts);

// Registers every binding with RegisterHotKey(hwnd, kHotkeyIdBase + i,
// modifiers | MOD_NOREPEAT, vk). Logs each failure (key text + GetLastError).
// Returns the number of successful registrations.
int RegisterBindings(HWND hwnd, const std::vector<Binding>& bindings);
void UnregisterBindings(HWND hwnd, const std::vector<Binding>& bindings);

// =============================================================================
// Win32 helpers (30_win32.cpp)
// =============================================================================

enum class Eligibility { Ignore, Float, Tile };

bool GetWindowFacts(HWND hwnd, WindowFacts* out);
bool IsWindowCloaked(HWND hwnd);

// Decides whether a top-level window should be managed. Order of checks:
//  Ignore: !IsWindow; hwnd != GetAncestor(hwnd, GA_ROOT); WS_CHILD;
//          belongs to the current process; not visible (IsWindowVisible) and
//          not in g.hiddenByUs; cloaked; WS_EX_TOOLWINDOW; WS_EX_NOACTIVATE;
//          shell/system classes (Shell_TrayWnd, Shell_SecondaryTrayWnd, Progman,
//          WorkerW, NotifyIconOverflowWindow, Windows.UI.Core.CoreWindow,
//          MultitaskingViewFrame, ForegroundStaging, TaskListThumbnailWnd,
//          XamlExplorerHostIslandWindow, TopLevelWindowForOverflowXamlIsland,
//          #32768, kEngineWindowClass);
//          neither WS_THICKFRAME nor full WS_CAPTION;
//          visible frame smaller than 40x40 px (only checked when visible);
//          matches settings.ignoreRules.
//  Float:  its processName is in g.alwaysFloatApps (apps remembered with
//          togglealwaysfloat); matches settings.floatRules; has an owner window; (floatTopmost &&
//          WS_EX_TOPMOST); (floatNonResizable && !WS_THICKFRAME).
//  Tile:   everything else.
Eligibility ClassifyWindow(HWND hwnd, const Settings& settings);

// DWMWA_EXTENDED_FRAME_BOUNDS, falling back to GetWindowRect.
RECT GetVisibleFrame(HWND hwnd);

// Positions the window so that its *visible* frame equals `visibleTarget`,
// compensating the invisible resize borders (difference between GetWindowRect
// and the visible frame). Uses SetWindowPos with SWP_NOZORDER |
// SWP_NOOWNERZORDER | SWP_NOACTIVATE | SWP_ASYNCWINDOWPOS. Skips the call if the
// window rect already matches. If the target is on a different monitor than
// the window currently is (MonitorFromWindow vs MonitorFromRect, NEAREST),
// calls SetWindowPos a second time so DPI rescaling cannot leave it the wrong
// size. Returns false only if SetWindowPos failed with ERROR_ACCESS_DENIED
// (elevated window we can't move); true otherwise.
bool PlaceWindow(HWND hwnd, const RECT& visibleTarget);

// Restores if minimized (ShowWindowAsync SW_RESTORE), then SetForegroundWindow.
// If the foreground did not change, SendMenuMaskKey() and try once more.
void FocusWindow(HWND hwnd);

// SendInput: key down + key up of kMenuMaskVk. Prevents a released Alt/Win
// from opening a menu bar / the Start menu after our hotkey ate the key.
void SendMenuMaskKey();

// DwmSetWindowAttribute(kDwmBorderColor). Clear sets kDwmColorDefault.
// Failures are ignored.
void SetFocusBorder(HWND hwnd, COLORREF color);
void ClearFocusBorder(HWND hwnd);

// Workspace visibility. Hide: insert into g.hiddenByUs FIRST, then
// ShowWindowAsync(SW_HIDE). Show: erase from g.hiddenByUs FIRST, then
// ShowWindowAsync(SW_SHOWNA). Both call PersistHiddenList().
// ShowForWorkspace also inserts hwnd into g.needsNudge and calls
// RequestNudge().
void HideForWorkspace(HWND hwnd);
void ShowForWorkspace(HWND hwnd);

// Forces a fresh WM_SIZE without changing the final size: reads GetWindowRect,
// then two SetWindowPos calls (SWP_NOZORDER | SWP_NOOWNERZORDER |
// SWP_NOACTIVATE | SWP_ASYNCWINDOWPOS, so a hung app cannot block us): first
// one pixel narrower, then the original rect. Skips hidden, minimized and
// maximized windows.
void NudgeWindow(HWND hwnd);
// SetTimer(g.engineHwnd, kTimerNudge, 300) — restarts the delay, so a burst of
// shows/restores is nudged once, after it settles.
void RequestNudge();

// Mod storage key "hiddenWindows": comma-separated "hwnd:pid" hex pairs of
// g.hiddenByUs, via Wh_SetStringValue. RestoreLeftoverHiddenWindows reads it
// (Wh_GetStringValue), shows every listed window that IsWindow(), still
// belongs to the saved pid and is !IsWindowVisible() with
// ShowWindowAsync(SW_SHOWNA), then clears the value.
// Under HT_STANDALONE both are no-ops.
void PersistHiddenList();
void RestoreLeftoverHiddenWindows();

// Apps the user told HawkTile never to tile (togglealwaysfloat). Mod storage
// key "alwaysFloatApps": lowercase exe file names (as in
// WindowFacts::processName) joined with '|'. Load replaces g.alwaysFloatApps
// with the stored set (empty when missing) and logs how many apps it holds;
// Save writes the set back (Wh_DeleteValue when empty). Under HT_STANDALONE
// both only touch memory.
void LoadAlwaysFloatApps();
void SaveAlwaysFloatApps();

// =============================================================================
// Model (50_model.cpp)
// =============================================================================

struct Workspace {
    std::vector<HWND> tiled;      // layout order; index 0 = master / first split
    std::vector<HWND> floating;
    LayoutKind layout = LayoutKind::Dwindle;
    LayoutKind prevLayout = LayoutKind::Dwindle;  // restored by togglemonocle
    // Per-split overrides (LayoutParams::ratios). Empty = all defaults; the
    // default of split 0 comes from settings.masterRatio at layout time.
    std::vector<double> ratios;
    std::vector<int> flips;       // LayoutParams::flips (dwindle split orientation)
    // Mainstage: which side each tiled window's tile is on (0 left, 1 right).
    // A window without an entry gets the side with fewer tiles at the next
    // Retile; entries of windows that left the workspace are dropped there.
    std::unordered_map<HWND, int> stageSide;
    HWND lastFocus = nullptr;
};

struct Monitor {
    HMONITOR handle = nullptr;
    RECT rcMonitor{};
    RECT rcWork{};
    std::vector<Workspace> workspaces;
    int active = 0;
};

struct WinLoc {
    int mon = -1;
    int ws = -1;
    bool floating = false;
    int index = -1;  // index in the tiled or floating vector
};

struct EngineState {
    Settings settings;
    std::vector<Monitor> monitors;     // sorted by rcMonitor.left, then .top
    std::unordered_set<HWND> hiddenByUs;
    std::unordered_set<HWND> zoomed;   // managed windows last seen maximized
    std::unordered_set<HWND> pendingEval;        // unmanaged windows to classify later
    std::unordered_set<HWND> pendingVisibility;  // managed windows whose visibility changed
    std::unordered_set<HWND> pendingActivation;  // hidden managed windows that took the foreground
    int focusedMon = 0;
    bool paused = false;
    HWND engineHwnd = nullptr;
    HWND borderedHwnd = nullptr;       // window currently carrying our focus border
    // Managed windows that asked for attention (FlashWindowEx → shell hook
    // HSHELL_FLASH) and have not been activated since.
    std::unordered_set<HWND> attention;
    // Windows that were just shown again (ShowForWorkspace) or just restored
    // from maximized (engine, EVENT_OBJECT_LOCATIONCHANGE zoomed → not zoomed).
    // Some apps (Revit) miss the resize that follows and keep painting at
    // their old size, leaving the rest of the window black; kTimerNudge
    // re-sends their size (NudgeWindow) once things have settled.
    std::unordered_set<HWND> needsNudge;
    // Lowercase exe names whose windows always float (togglealwaysfloat),
    // persisted in mod storage by Load/SaveAlwaysFloatApps.
    std::unordered_set<std::wstring> alwaysFloatApps;
    // settings.workspaceLayouts as last applied to the workspaces, so a
    // settings reload only touches the workspaces whose entry changed.
    std::vector<LayoutKind> appliedWorkspaceLayouts;
    HWND dragHwnd = nullptr;           // window of the last EVENT_SYSTEM_MOVESIZESTART
    RECT dragStartFrame{};             // its visible frame when the drag started
};

extern EngineState g;  // defined in 50_model.cpp

// Enumerates monitors (EnumDisplayMonitors + GetMonitorInfoW), sorts them,
// keeps workspaces of monitors matched by HMONITOR or identical rcMonitor,
// creates fresh workspaces (settings.workspaceCount, defaultLayout,
// masterRatio) for new monitors, and moves all windows of vanished monitors
// into monitor 0's active workspace (showing any that were hidden by us).
// Clamps focusedMon. Never leaves g.monitors empty while a monitor exists.
void RefreshMonitors();

// Applies g.settings after a reload: resizes every monitor's workspace vector
// to workspaceCount (windows from removed workspaces move into the last
// remaining one and are shown if that one is active); every workspace whose
// settings.workspaceLayouts entry differs from g.appliedWorkspaceLayouts and
// names a layout switches to it; then RetileAll() and UpdateFocusBorder().
void ApplySettingsChange();

bool Locate(HWND hwnd, WinLoc* loc);           // linear scan of all lists
int MonitorIndexFromWindow(HWND hwnd);         // MonitorFromWindow NEAREST → index, fallback 0
Workspace& ActiveWorkspace(int mon);

// Adds to the active workspace of the window's monitor (appended). No-op and
// false if already managed. Does not retile. If tiled, IsZoomed and
// settings.tileMaximized → ShowWindowAsync(SW_RESTORE) (it is still inserted
// into g.zoomed, so the restore's LOCATIONCHANGE triggers a retile).
bool ManageWindow(HWND hwnd, bool floating);
// Removes from all lists, g.hiddenByUs (then PersistHiddenList), g.zoomed,
// clears lastFocus references and the focus border if it was on it. Does not retile.
void UnmanageWindow(HWND hwnd);

// Lays out the active workspace of `mon`: tiled windows that are not in
// g.hiddenByUs && !IsIconic && !IsZoomed, inside rcWork deflated by
// outerGap, using ComputeLayout. A window whose PlaceWindow returns false is
// moved to the floating list (logged). Lays nothing out while g.paused.
//
// Mainstage: every layout window is placed on the STAGE rect (rect 0), stacked
// like monocle, so the one on top of the z-order is the one in use. Before
// laying out, that topmost layout window is swapped into layout index 0 (the
// stage follows focus; the window that leaves the stage takes the other's
// place in the list and its side). Then stale ws.stageSide entries are
// dropped and side windows without one get the side with fewer tiles (left
// on a tie). The windows at index 1.. are shown in their side cells as live
// scaled-down tiles by SyncStageProxies(), which Retile and RetileAll always
// call last, paused or not.
void Retile(int mon);
void RetileAll();

// One Mainstage side window: the real window (stacked under the stage window)
// and the side cell its tile is drawn in, in screen pixels.
struct StageTile {
    HWND hwnd = nullptr;
    RECT cell{};
};
// Every side window of every monitor whose active workspace uses Mainstage:
// layout windows 1.. with rects 1.. of ComputeLayout. Empty while g.paused.
std::vector<StageTile> StageSideTiles();

// Mainstage drag and drop: tiled window `hwnd` (a side tile that was dragged,
// or the stage window after the user moved it by its title bar) was dropped
// with the mouse at screen point `pt`. No-op unless hwnd is a layout window of
// an active Mainstage workspace. By what lies under pt:
//  - its own slot: nothing;
//  - the stage (hwnd is a side window): FocusWindow(hwnd), so it takes the stage;
//  - another window's tile, hwnd is the stage window: FocusWindow(that window)
//    (they trade places);
//  - another window's tile, hwnd is a side window: the two swap places in the
//    tiled list and swap sides;
//  - elsewhere left or right of the stage, inside the monitor's work area:
//    hwnd moves to the end of that side (end of the tiled list, stageSide set).
//    When hwnd was the stage window, the next layout window is focused so it
//    takes the stage; needs at least two layout windows.
// Always ends with Retile(its monitor).
void StageDropWindow(HWND hwnd, POINT pt);
// SetTimer(g.engineHwnd, kTimerRetile, 15) — coalesces bursts of events.
void RequestRetile();

// 0-based. Also calls ShowWorkspaceOsd(mon) when settings.workspaceOsd, even
// when wsIndex is already active (so the key always gives feedback).
void SwitchWorkspace(int mon, int wsIndex);
// "Silent" move: the window goes to the workspace, the user stays. Hidden if that
// workspace is not active.
void MoveWindowToWorkspace(HWND hwnd, int mon, int wsIndex);
// "Move and follow": the window's membership moves to workspace wsIndex of
// its own monitor WITHOUT hiding it, then SwitchWorkspace(mon, wsIndex) runs,
// which hides the rest of the old workspace and focuses this window. No-op if
// already there (but still shows the OSD).
void MoveWindowToWorkspaceAndFollow(HWND hwnd, int wsIndex);
void MoveWindowToMonitor(HWND hwnd, int mon);  // to that monitor's active workspace

// The managed window that commands act on: GetForegroundWindow() if managed,
// else lastFocus of the focused monitor's active workspace (if still managed),
// else nullptr.
HWND CommandTarget();

// Called on EVENT_SYSTEM_FOREGROUND. If managed: g.focusedMon = its monitor,
// its workspace's lastFocus = hwnd. Then UpdateFocusBorder(). Also clears the
// resolved managed window from g.attention (RequestBarUpdate() if it was there).
// A tiled window on an active Mainstage workspace: Retile(its monitor) first,
// which moves it onto the stage.
void OnFocusChanged(HWND hwnd);

// "Follow activation": brings the workspace that holds managed window hwnd on
// screen and focuses the window — used when a window on a hidden workspace
// activates itself (notification click, a link opened in a browser that lives
// elsewhere, an app bringing itself to front). Sets that workspace's
// lastFocus = hwnd, then SwitchWorkspace(its monitor, its workspace) (which
// shows it, focuses it, flashes the OSD and updates the taskbar indicator),
// then FocusWindow(hwnd). If the workspace is already active it only focuses.
// No-op for unmanaged windows. Logs the switch.
void ActivateWindowWorkspace(HWND hwnd);

// Called by the engine for every HSHELL_FLASH. Resolves hwnd to a managed
// window (itself, else GetAncestor(GA_ROOTOWNER)); ignores unmanaged windows
// and the window that is currently in the foreground (or whose root owner
// is); otherwise inserts it into g.attention and RequestBarUpdate().
void MarkAttention(HWND hwnd);

// True when settings.attentionDots and any tiled or floating window of that
// workspace is in g.attention. False for bad indices.
bool WorkspaceHasAttention(int mon, int wsIndex);
// Puts the border on CommandTarget() (if settings.focusBorder) and clears it
// from the previously bordered window.
void UpdateFocusBorder();

// After a user drag or border resize (EVENT_SYSTEM_MOVESIZEEND) of a managed
// window: if the mouse is over a workspace column of the open overview
// (OverviewDropTarget) → MoveWindowToWorkspace(hwnd, that monitor, that
// workspace), OverviewDropDone(), RequestRetile() and nothing else. Otherwise:
// if its visible-frame center is now on another monitor → a floating
// window just joins that monitor's active workspace (not moved), a tiled one
// goes through MoveWindowToMonitor. Otherwise, for a tiled window in a
// non-monocle layout: if its visible frame's width or height differs by more
// than 2 px from g.dragStartFrame (recorded at MOVESIZESTART; compared with
// the layout slot only when there is no record for this window — apps with a
// minimum size never match their slot) it was RESIZED → ApplyResize on the same
// window list / area / params Retile uses, storing the result in the
// workspace's ratios (neighbours follow the new edges). Else it was MOVED → if
// its center lies inside the visible frame of another tiled window of the same
// workspace, swap them in the tiled vector. Always RequestRetile().
void OnUserDragEnd(HWND hwnd);

// ---- Alt+Shift mouse resize ("hold the keys and move the mouse") -----------
// No click is involved: while left Alt + Shift are held, moving the mouse
// resizes the ACTIVE window (CommandTarget()), tiled or floating. The edges
// that move are picked by where the cursor was, relative to that window's
// center, when the chord started: left of center → its left edge, else its
// right edge; above center → its top edge, else its bottom edge. Moving the
// mouse outward grows the window toward that side, inward shrinks it.
// While the keys are held ONLY the active window changes (live, PlaceWindow);
// the other tiled windows make room once, when the session ends.
//
// Starts a session for CommandTarget(). Qualifies: a managed window on its
// monitor's active workspace that is not minimized or maximized, and if tiled,
// in a non-monocle layout with >= 2 layout windows and present in
// LayoutWindowsOf(ws). Otherwise returns false and nothing is recorded.
// Records: the window, tiled or floating, its monitor and workspace; for a
// tiled window its index in LayoutWindowsOf(ws), the count, the layout area,
// a copy of ws.ratios and the start rect = its slot from ComputeLayout with
// the start params (NOT the visible frame: apps with a minimum size never
// match their slot); for a floating window start rect = GetVisibleFrame. Plus
// the start cursor and the chosen edges. Changes nothing yet. Ends a previous
// session first.
bool BeginMouseResize(POINT cursor);
// newRect = start rect with the chosen edges shifted by the cursor delta,
// each moved edge kept at least 50 px from its opposite edge; a floating
// window is also kept inside its monitor's work area. PlaceWindow(hwnd,
// newRect) only — no ratios, no Retile. No-op without a session or when the
// delta did not change since the last call; ends the session (without
// applying) when the window is no longer where it was recorded.
void UpdateMouseResize(POINT cursor);
// Ends the session. Tiled window whose rect changed: params = LayoutParamsOf
// with the RECORDED start ratios, ApplyResize(area, count, params, index,
// last newRect); if it changed anything ws.ratios = params.ratios; then
// Retile(mon) either way (neighbours follow, the window snaps to its slot).
// Floating window: nothing more. Logs once ("mouse-resized window %p") when
// something changed. No-op without a session.
void EndMouseResize();
bool MouseResizeActive();

// Executes a command (see CommandKind). While g.paused only ToggleTiling and
// Rescan do anything.
void ExecuteCommand(const Command& cmd);

// EnumWindows → ClassifyWindow → ManageWindow for every eligible unmanaged
// window. Used at startup and by the `rescan` command.
void AdoptExistingWindows();

// =============================================================================
// Theme + workspace indicator / OSD (55_osd.cpp)
// =============================================================================

// Colors for HawkTile's own UI, following the user's Windows theme.
struct Theme {
    COLORREF background;
    COLORREF activeFill, activeText;      // active workspace badge
    COLORREF occupiedFill, occupiedText;  // workspace with windows
    COLORREF emptyFill, emptyText;        // empty workspace
    COLORREF listText, activeListText;    // small secondary text
    bool highContrast;
};

// Re-reads the theme on every call: high contrast (system colors), else
// SystemUsesLightTheme (missing → dark) + AccentPalette (Light2 on dark,
// Dark1 on light) with a DwmGetColorizationColor fallback.
Theme ResolveTheme();

inline constexpr wchar_t kOsdWindowClass[] = L"HawkTileOsd";

// Shows a small rounded, semi-transparent, click-through, topmost, never
// activated overlay centered in g.monitors[mon].rcWork with one column per
// workspace (1..N). Each column has a round number badge on top:
//   active workspace  → circle filled with the theme accent, bold number;
//   occupied (has tiled or floating windows) → prominent circle (lighter on
//                       dark theme, darker on light theme), bold number;
//   empty             → recessed circle, normal-weight dim number.
// Below the badge of every occupied workspace, the APP ICONS of its windows:
// one small (16 px) icon per app (windows grouped by exe path), stacked in a
// single vertical column centered under the badge, with a tiny count in the
// icon's corner when the app has several windows there; at most 8 rows, the
// last becoming "+N" when there are more. Every column is as wide as the badge.
// Colors follow the user's Windows theme, re-read on every show (light/dark
// shell theme, accent palette, high contrast). Hides itself after settings.osdDurationMs;
// calling again while visible redraws and restarts the timer. Created lazily
// on the engine thread; sized for the monitor's DPI. No-op for bad `mon`.
// While the overview (below) is open, it instead re-renders the overview for
// `mon` in place: still interactive, no hide timer; a drag in progress is
// cancelled first.
void ShowWorkspaceOsd(int mon);

// ---- Workspace overview (hold left Alt) --------------------------------------
// The same panel, same look and same window as the popup, but INTERACTIVE:
// no hide timer, it takes mouse input (never activated, never takes focus) and
// it stays until CloseWorkspaceOverview. Mouse actions, left button only:
//   - drag an app icon onto another workspace's column and release → every
//     window of that icon's app group that is still on the source workspace of
//     this monitor moves there with MoveWindowToWorkspace (the silent move);
//   - click a badge → SwitchWorkspace(mon, column);
//   - click an icon → ActivateWindowWorkspace(first still-managed window of
//     the group) (switches to its workspace and focuses it).
// After every action the panel is re-rendered for the same monitor.
// Opening while the timed popup is visible converts it (its timer is killed).
// No-op for bad `mon`.
void ShowWorkspaceOverview(int mon);
// Hover: while nothing is pressed, the column under the mouse (its whole zone,
// badge and icons together) gets a light accent tint. Hover is only set by real
// mouse movement over the panel, so a panel that opens under a resting cursor
// has no hover; a re-render keeps an existing hover, re-hit-tested at the
// cursor. A press anywhere in a column that is not on an icon counts as the
// badge (click → switch).
// Closes the overview. cancelDrag == false (left Alt released): if a column is
// hovered and no press is in progress, SwitchWorkspace(mon, hovered column)
// first (no-op when already active), then hides; while a mouse button press or
// drag on the panel is in progress it hides right after that press ends
// instead (the drop or click still happens, no hover switch). cancelDrag == true: any
// press or drag is abandoned without effect and the panel hides now.
// The window goes back to click-through. No-op when the overview is not open.
void CloseWorkspaceOverview(bool cancelDrag);
// True while the overview is open (including a press that outlives Alt).
bool WorkspaceOverviewOpen();
// Dragging something else (a window by its title bar, a Mainstage side tile)
// onto the open overview. Returns true when the overview is open and screen
// point `screen` is over one of its workspace columns, with the overview's
// monitor and that workspace's 0-based index (either pointer may be null).
// Also moves the column highlight (the hover) to that column, or removes it
// when there is none, so the drag shows where it would land.
bool OverviewDropTarget(POINT screen, int* mon, int* wsIndex);
// The drag ended: removes that highlight, so letting go of Alt right after a
// drop does not switch workspace.
void OverviewDropDone();
// Re-renders the open overview for its monitor so it shows current windows;
// no-op when it is not open or while a press or drag is in progress. The
// engine calls it from the coalesced kTimerBar refresh.
void RefreshWorkspaceOverview();
// Destroys the OSD window and unregisters its class (engine teardown).
void DestroyOsd();

// =============================================================================
// Taskbar workspace indicator (56_bar.cpp)
// =============================================================================

inline constexpr wchar_t kBarWindowClass[] = L"HawkTileBar";

// SetTimer(g.engineHwnd, kTimerBar, 30) — coalesces bursts of state changes.
// The model calls it whenever workspaces, membership or monitors change.
void RequestBarUpdate();

// Notification dot (both the taskbar indicator and the OSD): every workspace
// that is NOT the monitor's active one and has WorkspaceHasAttention() gets a
// small dot at the top-right of its badge (center on the badge circle at 45°),
// filled with theme.activeFill (the active badge's color) and ringed with
// theme.background so it separates from the badge.
//
// One small per-pixel-alpha window per monitor that has a taskbar
// (Shell_TrayWnd / Shell_SecondaryTrayWnd), sitting over the taskbar's left
// area (settings.taskbarIndicatorOffset from its left edge, vertically
// centered), showing that monitor's workspaces as small round badges in the
// same visual language as the OSD (ResolveTheme colors; active = accent fill,
// occupied = bold, empty = dim) on a transparent background. Clicking a badge
// switches that monitor to that workspace; the window never takes focus.
// Each call: (re)creates/destroys bars to match monitors, repositions them,
// redraws them, hides a bar when settings.taskbarIndicator is off, its
// taskbar is not visible (auto-hidden), or a fullscreen window is in the
// foreground on that monitor, and re-raises visible bars above the taskbar
// (HWND_TOPMOST, no activation).
void UpdateTaskbarIndicators();
void DestroyTaskbarIndicators();  // engine teardown; also shuts down GDI+

// =============================================================================
// Mainstage side tiles (57_stage.cpp)
// =============================================================================

inline constexpr wchar_t kStageWindowClass[] = L"HawkTileStage";
// Mainstage side windows are this much wider than the stage window, to the
// left, so they are never fully covered (RetileWindows).
inline constexpr LONG kStageSliver = 2;

// Makes the side tiles on screen match StageSideTiles(): one small borderless
// window per tile (tool window, never activated, rounded on Windows 11, put
// at the bottom of the z-order when created so floating windows stay above
// it), filling the tile's cell and showing a live DWM thumbnail of the real
// window, scaled to fit the cell and centred, with the invisible resize
// borders cropped away; the rest of the cell is the theme background. Tiles
// that are no longer wanted, or whose window is gone, are destroyed. Starts
// kTimerStage (250 ms), because the real windows reach their size
// asynchronously.
//
// Mouse on a tile:
//  - Press the left button and move further than twice the system drag
//    threshold → the tile itself follows the cursor (topmost while dragged),
//    and OverviewDropTarget(cursor) highlights the overview column under it.
//    On release it returns to its cell; over an overview column the real
//    window goes to that workspace (MoveWindowToWorkspace, OverviewDropDone),
//    otherwise StageDropWindow(real window, cursor) decides.
//  - A left click without dragging, a double click, a right or middle click
//    and the mouse wheel are FORWARDED into the real window: the tile point is
//    mapped through the thumbnail's scale to a screen point of the real
//    window, the deepest child window there is found with
//    ChildWindowFromPointEx, and the messages are posted to it (WM_MOUSEMOVE,
//    then button down and up with client coordinates; WM_MOUSEWHEEL with
//    screen coordinates). Nothing is posted outside the thumbnail or outside
//    that window's client area (title bar, borders). The real window is not
//    activated, so it stays in its tile. Apps that read the real cursor
//    position or use pointer input instead of mouse messages will not react.
//  - Left Alt held for 500 ms with the cursor over a tile zooms it: the tile
//    shows its window at full size around the point under the cursor. The
//    cursor is then held in place and mouse movement drags the picture under
//    it. Releasing Alt or dragging the tile (button held) ends the zoom.
//    Clicks land on the point shown under the cursor.
void SyncStageProxies();
// Fits every tile's thumbnail to its window's current size (kTimerStage).
void RefitStageProxies();
// Destroys every tile and unregisters the class (engine teardown).
void DestroyStageProxies();

// =============================================================================
// Engine (60_engine.cpp)
// =============================================================================

// Called from Windhawk's thread. Starts the engine thread and waits (max 10 s)
// until it has finished initialising. Returns false if init failed.
bool StartEngine();
// Called from Windhawk's thread. Posts WM_HT_RELOAD to g.engineHwnd.
void NotifySettingsChanged();
// Called from Windhawk's thread. Asks the engine thread to quit, waits (max
// 10 s). The engine thread's cleanup re-shows every window in g.hiddenByUs,
// clears the focus border, unregisters hotkeys, unhooks WinEvents and calls
// DestroyOsd().
void StopEngine();

// Hold-left-Alt detection (60_engine.cpp) uses Raw Input (keyboard, with
// RIDEV_INPUTSINK on the engine window), which observes keys without delaying
// them, unlike a low-level keyboard hook. Right Alt is AltGr on many layouts
// and never opens the overview. Rules, per left-Alt press:
//   - Left Alt goes down (not a repeat) with Shift, Ctrl and Win all up, while
//     settings.holdOverview && !g.paused → arm kTimerHold (holdDelayMs).
//   - Before the overview opens: a HawkTile-bound key (the vk of any binding
//     whose modifiers include MOD_ALT) re-arms the timer, so holding Alt after
//     Alt+2 still opens it; any other key (modifiers included) disarms it.
//   - kTimerHold fires with left Alt still down → SendMenuMaskKey() (no menu
//     bar on release), ShowWorkspaceOverview(g.focusedMon), start kTimerHoldPoll.
//   - While open: Shift/Ctrl/Win and bound keys keep it open (bound commands
//     re-render it through ShowWorkspaceOsd); Escape or any other key →
//     CloseWorkspaceOverview(true).
//   - Left Alt released → CloseWorkspaceOverview(false).
//   - kTimerHoldPoll: overview closed → stop polling; left Alt no longer down
//     (GetAsyncKeyState; covers a key-up that Raw Input never delivered, e.g.
//     the secure desktop) → CloseWorkspaceOverview(false).
// kMenuMaskVk (our own injected key) and the 0xFF filler vk are ignored.
//
// Alt+Shift mouse resize (60_engine.cpp, same Raw Input keyboard stream):
//   - The chord is "on" while left Alt and Shift (either) are down and Ctrl
//     and Win are up (GetAsyncKeyState for Shift/Ctrl/Win). When it turns on
//     (on the key-down that completes it, in either order) while
//     settings.mouseResize && !g.paused && !WorkspaceOverviewOpen(): remember
//     the cursor (GetCursorPos) as the anchor and start kTimerMouseResize.
//   - kTimerMouseResize: chord no longer on (re-checked with GetAsyncKeyState:
//     VK_LMENU and VK_SHIFT down, VK_CONTROL/VK_LWIN/VK_RWIN up) → stop: kill
//     the timer, EndMouseResize(). Otherwise, without a session: once the
//     cursor is more than 2 * SM_CXDRAG / SM_CYDRAG from the anchor,
//     BeginMouseResize(anchor); on success SendMenuMaskKey() (no menu bar and
//     no input-language switch when the keys are released), else stop
//     polling for this chord. With a session: UpdateMouseResize(cursor).
//   - Any other key-down (not Alt, Shift, kMenuMaskVk, 0xFF) while polling →
//     stop as above (e.g. Alt+Shift+1 ends the resize, keeping the ratios).
//   - Left Alt or Shift released → stop as above.
//   - The overview never opens while a mouse-resize poll is running.

}  // namespace ht

// =============================================================================
// 20_layout.cpp — tiling geometry. Pure functions; the only Win32 type used is
// RECT. Signatures and semantics: 10_contract.h.
// =============================================================================

#include <cmath>

namespace ht {
namespace {

LONG AbsLong(LONG v) {
    return v < 0 ? -v : v;
}

bool RectEquals(const RECT& a, const RECT& b) {
    return a.left == b.left && a.top == b.top && a.right == b.right &&
           a.bottom == b.bottom;
}

// Clamps r to [lo, hi]. Written so that NaN takes the lower bound.
double ClampRatio(double r, double lo, double hi) {
    if (!(r >= lo)) {
        return lo;
    }
    if (r > hi) {
        return hi;
    }
    return r;
}

// Final rect of a raw slot. Low sides move in by `a`, high sides by `b`, so two
// slots that share an edge end up exactly a + b = innerGap apart.
RECT FinalizeSlot(const RECT& slot, LONG a, LONG b) {
    RECT rc{slot.left + a, slot.top + a, slot.right - b, slot.bottom - b};
    rc.right = std::max(rc.right, rc.left + 1);
    rc.bottom = std::max(rc.bottom, rc.top + 1);
    return rc;
}

// Cut that splits [lo, hi) at ratio r.
LONG SplitCut(LONG lo, LONG hi, double r) {
    return lo + std::lround(static_cast<double>(hi - lo) * r);
}

// Ratio that puts the cut at `cut` inside the remaining region [lo, hi).
double RatioTo(LONG lo, LONG hi, LONG cut) {
    return static_cast<double>(cut - lo) / static_cast<double>(hi - lo);
}

// One binary split of a layout. [lo, hi) is the remaining region's extent along
// the cut axis, in Expanded coordinates, and `cut` lies inside it.
struct LayoutSplit {
    bool vertical = true;  // vertical: the cut is the line x = cut; else y = cut
    LONG lo = 0;
    LONG hi = 0;
    LONG cut = 0;
};

// Orientation of split `split` for a split-based layout (not Monocle), given the
// remaining region `rest` that it cuts. Dwindle follows the region's shape unless
// p.flips[split] is set, which inverts it.
bool IsVertical(const LayoutParams& p, int split, const RECT& rest) {
    switch (p.kind) {
    case LayoutKind::MasterStack:
        return split == 0;
    case LayoutKind::Columns:
        return true;
    case LayoutKind::Rows:
        return false;
    default: {  // Dwindle
        const bool wider = rest.right - rest.left > rest.bottom - rest.top;
        const size_t index = static_cast<size_t>(split);
        const bool flipped = index < p.flips.size() && p.flips[index] != 0;
        return wider != flipped;
    }
    }
}

// Walks the count-1 splits of `expanded` (count >= 1, not Monocle). `splits` gets
// one entry per split. `slots` gets the raw slot of every window: window i is
// side A of split i (left or top part), and the last window takes what remains.
// ComputeLayout and ApplyResize both use this, so their geometry is identical.
void WalkSplits(const RECT& expanded, int count, const LayoutParams& p,
                std::vector<LayoutSplit>* splits, std::vector<RECT>* slots) {
    splits->clear();
    slots->clear();
    RECT rest = expanded;
    for (int i = 0; i + 1 < count; ++i) {
        LayoutSplit s;
        s.vertical = IsVertical(p, i, rest);
        s.lo = s.vertical ? rest.left : rest.top;
        s.hi = s.vertical ? rest.right : rest.bottom;
        s.cut = SplitCut(s.lo, s.hi, SplitRatio(p, count, i));
        splits->push_back(s);

        RECT sideA = rest;
        if (s.vertical) {
            sideA.right = s.cut;
            rest.left = s.cut;
        } else {
            sideA.bottom = s.cut;
            rest.top = s.cut;
        }
        slots->push_back(sideA);
    }
    slots->push_back(rest);
}

// Mainstage: the stage's share of the area's width.
double StageShare(const LayoutParams& p) {
    if (!p.ratios.empty() && 0.0 < p.ratios[0] && p.ratios[0] < 1.0) {
        return ClampRatio(p.ratios[0], 0.2, 0.9);
    }
    return ClampRatio(p.stageRatio, 0.2, 0.9);
}

// Mainstage: the stage rect, full height and horizontally centred in `area`.
RECT StageRect(const RECT& area, const LayoutParams& p) {
    const LONG areaW = std::max<LONG>(1, area.right - area.left);
    const LONG width = std::clamp<LONG>(
        std::lround(static_cast<double>(areaW) * StageShare(p)), 1, areaW);
    const LONG left = area.left + (areaW - width) / 2;
    return RECT{left, area.top, left + width, std::max(area.bottom, area.top + 1)};
}

// Mainstage: the largest rect with aspect ratio aspectW : aspectH inside `box`,
// centred in it.
RECT FitAspect(const RECT& box, LONG aspectW, LONG aspectH) {
    const LONG boxW = std::max<LONG>(1, box.right - box.left);
    const LONG boxH = std::max<LONG>(1, box.bottom - box.top);
    LONG w = boxW;
    LONG h = std::lround(static_cast<double>(w) * aspectH / aspectW);
    if (h > boxH) {
        h = boxH;
        w = std::lround(static_cast<double>(h) * aspectW / aspectH);
    }
    w = std::clamp<LONG>(w, 1, boxW);
    h = std::clamp<LONG>(h, 1, boxH);
    const LONG left = box.left + (boxW - w) / 2;
    const LONG top = box.top + (boxH - h) / 2;
    return RECT{left, top, left + w, top + h};
}

// Mainstage: the `n` cells of one side strip. The strip is three slots stacked
// from the top. Up to three windows get a slot each. With more, the first two
// keep their slots and the rest share the last slot in a grid (2 side by side,
// then 2x2, 3x2, 3x3, ...). Every cell keeps the stage's aspect ratio
// (aspectW : aspectH), centred in its slot or grid box.
std::vector<RECT> SideCells(const RECT& strip, int n, LONG gap, LONG aspectW,
                            LONG aspectH) {
    constexpr int kSlots = 3;
    std::vector<RECT> cells;
    if (n <= 0) {
        return cells;
    }
    cells.reserve(static_cast<size_t>(n));
    const LONG stripW = std::max<LONG>(1, strip.right - strip.left);
    const LONG stripH = std::max<LONG>(1, strip.bottom - strip.top);
    const LONG slotH = std::max<LONG>(1, (stripH - (kSlots - 1) * gap) / kSlots);
    const auto slot = [&](int i) {
        const LONG top = strip.top + i * (slotH + gap);
        return RECT{strip.left, top, strip.left + stripW, top + slotH};
    };

    const int single = n <= kSlots ? n : kSlots - 1;
    for (int i = 0; i < single; ++i) {
        cells.push_back(FitAspect(slot(i), aspectW, aspectH));
    }
    const int rest = n - single;
    if (rest <= 0) {
        return cells;
    }
    int columns = 1;
    while (columns * columns < rest) {
        ++columns;
    }
    const int rows = (rest + columns - 1) / columns;
    const RECT last = slot(kSlots - 1);
    const LONG boxW = std::max<LONG>(1, (stripW - (columns - 1) * gap) / columns);
    const LONG boxH = std::max<LONG>(1, (slotH - (rows - 1) * gap) / rows);
    for (int k = 0; k < rest; ++k) {
        const LONG left = last.left + (k % columns) * (boxW + gap);
        const LONG top = last.top + (k / columns) * (boxH + gap);
        cells.push_back(
            FitAspect(RECT{left, top, left + boxW, top + boxH}, aspectW, aspectH));
    }
    return cells;
}

// Mainstage: the stage, then the side cells, alternating left and right.
std::vector<RECT> MainstageLayout(const RECT& area, int count,
                                  const LayoutParams& p) {
    std::vector<RECT> out;
    out.reserve(static_cast<size_t>(count));
    const RECT stage = StageRect(area, p);
    out.push_back(stage);

    const LONG gap = std::max(0, p.innerGap);
    const int sides = count - 1;
    const LONG stageW = stage.right - stage.left;
    const LONG stageH = stage.bottom - stage.top;
    const RECT leftStrip{area.left, area.top,
                         std::max(area.left + 1, stage.left - gap), area.bottom};
    const RECT rightStrip{std::min(area.right - 1, stage.right + gap), area.top,
                          area.right, area.bottom};
    // Which side each side window is on: p.sides, alternating beyond it.
    std::vector<bool> onRight(static_cast<size_t>(std::max(0, sides)), false);
    int rightCount = 0;
    for (int j = 0; j < sides; ++j) {
        const size_t k = static_cast<size_t>(j);
        onRight[k] = k < p.sides.size() ? p.sides[k] != 0 : j % 2 == 1;
        if (onRight[k]) {
            ++rightCount;
        }
    }
    const std::vector<RECT> left =
        SideCells(leftStrip, sides - rightCount, gap, stageW, stageH);
    const std::vector<RECT> right =
        SideCells(rightStrip, rightCount, gap, stageW, stageH);
    size_t nextLeft = 0;
    size_t nextRight = 0;
    for (int j = 0; j < sides; ++j) {
        out.push_back(onRight[static_cast<size_t>(j)] ? right[nextRight++]
                                                      : left[nextLeft++]);
    }
    return out;
}

// Ranking key of a FindNeighbor candidate; smaller is better.
struct NeighborKey {
    int overlapRank = 1;  // 0 when the perpendicular extents overlap
    LONG along = 0;       // doubled centre distance in the direction
    LONG perp = 0;        // doubled perpendicular centre distance
};

bool KeyBetter(const NeighborKey& x, const NeighborKey& y) {
    if (x.overlapRank != y.overlapRank) {
        return x.overlapRank < y.overlapRank;
    }
    if (x.along != y.along) {
        return x.along < y.along;
    }
    return x.perp < y.perp;
}

// Centres are doubled (left + right) so all arithmetic stays integer. Returns
// false when `c` is not strictly in direction `dir` from `f`.
bool MakeNeighborKey(const RECT& f, const RECT& c, Direction dir, NeighborKey* key) {
    const LONG fx = f.left + f.right;
    const LONG fy = f.top + f.bottom;
    const LONG cx = c.left + c.right;
    const LONG cy = c.top + c.bottom;
    const bool horizontal = dir == Direction::Left || dir == Direction::Right;

    LONG along = 0;
    switch (dir) {
    case Direction::Left:
        along = fx - cx;
        break;
    case Direction::Right:
        along = cx - fx;
        break;
    case Direction::Up:
        along = fy - cy;
        break;
    case Direction::Down:
        along = cy - fy;
        break;
    }
    if (along <= 0) {
        return false;
    }

    const bool overlaps = horizontal ? (c.top < f.bottom && f.top < c.bottom)
                                     : (c.left < f.right && f.left < c.right);
    key->overlapRank = overlaps ? 0 : 1;
    key->along = along;
    key->perp = AbsLong(horizontal ? cy - fy : cx - fx);
    return true;
}

struct LayoutNameEntry {
    const wchar_t* name;
    LayoutKind kind;
};

// Accepted spellings for ParseLayoutName. LayoutName uses the first entry per kind.
constexpr LayoutNameEntry kLayoutNames[] = {
    {L"dwindle", LayoutKind::Dwindle},
    {L"bsp", LayoutKind::Dwindle},
    {L"master", LayoutKind::MasterStack},
    {L"master-stack", LayoutKind::MasterStack},
    {L"columns", LayoutKind::Columns},
    {L"rows", LayoutKind::Rows},
    {L"monocle", LayoutKind::Monocle},
    {L"mainstage", LayoutKind::Mainstage},
    {L"stage", LayoutKind::Mainstage},
};

// Case-insensitive comparison of a whole string against a literal.
bool EqualsIgnoreCase(const wchar_t* text, const wchar_t* literal) {
    for (; *text != L'\0' && *literal != L'\0'; ++text, ++literal) {
        if (std::towlower(*text) != std::towlower(*literal)) {
            return false;
        }
    }
    return *text == *literal;
}

}  // namespace

std::vector<RECT> ComputeLayout(const RECT& area, int count, const LayoutParams& p) {
    std::vector<RECT> out;
    if (count <= 0) {
        return out;
    }
    out.reserve(static_cast<size_t>(count));
    if (p.kind == LayoutKind::Monocle) {
        out.assign(static_cast<size_t>(count), area);
        return out;
    }
    if (p.kind == LayoutKind::Mainstage) {
        return MainstageLayout(area, count, p);
    }

    const int gap = std::max(0, p.innerGap);
    const LONG a = gap / 2;
    const LONG b = gap - a;
    const RECT expanded{area.left - a, area.top - a, area.right + b, area.bottom + b};

    std::vector<LayoutSplit> splits;
    std::vector<RECT> slots;
    WalkSplits(expanded, count, p, &splits, &slots);
    for (const RECT& slot : slots) {
        out.push_back(FinalizeSlot(slot, a, b));
    }
    return out;
}

double SplitRatio(const LayoutParams& p, int count, int split) {
    if (p.kind == LayoutKind::Mainstage && split == 0) {
        return StageShare(p);
    }
    if (split < 0 || split >= count - 1) {
        return 0.5;
    }
    const size_t index = static_cast<size_t>(split);
    if (index < p.ratios.size()) {
        const double r = p.ratios[index];
        if (0.0 < r && r < 1.0) {
            return ClampRatio(r, 0.05, 0.95);
        }
    }

    const double master = ClampRatio(p.masterRatio, 0.05, 0.95);
    switch (p.kind) {
    case LayoutKind::MasterStack:
        return split == 0 ? master : 1.0 / (count - split);
    case LayoutKind::Columns:
    case LayoutKind::Rows:
        return 1.0 / (count - split);
    default:  // Dwindle (Monocle has no splits)
        return split == 0 ? master : 0.5;
    }
}

bool ApplyResize(const RECT& area, int count, LayoutParams& p, int index,
                 const RECT& newRect) {
    if (p.kind == LayoutKind::Mainstage) {
        if (count < 1 || index != 0) {
            return false;
        }
        const RECT stage = StageRect(area, p);
        const LONG stageW = stage.right - stage.left;
        const LONG change = (newRect.right - newRect.left) - stageW;
        if (AbsLong(change) <= 2) {
            return false;
        }
        // The stage stays centred, so both edges move: twice the change puts
        // the dragged edge where the user left it.
        const LONG areaW = std::max<LONG>(1, area.right - area.left);
        const double ratio = ClampRatio(
            static_cast<double>(stageW + 2 * change) / static_cast<double>(areaW),
            0.2, 0.9);
        if (p.ratios.empty()) {
            p.ratios.resize(1, 0.0);
        }
        if (p.ratios[0] == ratio) {
            return false;
        }
        p.ratios[0] = ratio;
        return true;
    }
    if (p.kind == LayoutKind::Monocle || count < 2 || index < 0 || index >= count) {
        return false;
    }
    const size_t splitCount = static_cast<size_t>(count - 1);
    if (p.ratios.size() < splitCount) {
        p.ratios.resize(splitCount, 0.0);
    }

    const int gap = std::max(0, p.innerGap);
    const LONG a = gap / 2;
    const LONG b = gap - a;
    const RECT expanded{area.left - a, area.top - a, area.right + b, area.bottom + b};

    // Step 1: the current geometry. Every split keeps its orientation and absolute
    // cut unless the drag moves it. "Moved" is judged against the slot edges the
    // user saw, before this call.
    std::vector<LayoutSplit> oldSplits;
    std::vector<RECT> oldSlots;
    WalkSplits(expanded, count, p, &oldSplits, &oldSlots);
    const RECT oldSlot = oldSlots[static_cast<size_t>(index)];
    const RECT oldShown = FinalizeSlot(oldSlot, a, b);

    // Desired cut: the dragged edge for a moved split, the old cut for all others.
    // Candidates are split `index` (the window is side A, so its right or bottom
    // edge is the cut) and every earlier split whose cut is the window's left or
    // top edge (side B). Later splits lie entirely in the other part.
    std::vector<bool> moved(splitCount, false);
    std::vector<LONG> desired(splitCount, 0);
    for (size_t k = 0; k < splitCount; ++k) {
        desired[k] = oldSplits[k].cut;
    }
    const int last = std::min(index, count - 2);
    for (int i = 0; i <= last; ++i) {
        const size_t k = static_cast<size_t>(i);
        const LayoutSplit& was = oldSplits[k];
        if (i == index) {
            // Side A: visible edge = cut - b, so cut = edge + b.
            const LONG target = was.vertical ? newRect.right : newRect.bottom;
            const LONG shown = was.vertical ? oldShown.right : oldShown.bottom;
            if (AbsLong(target - shown) > 2) {
                moved[k] = true;
                desired[k] = target + b;
            }
        } else {
            // Side B: the window's left or top edge must lie on this cut.
            // Visible edge = cut + a, so cut = edge - a.
            const LONG onCut = was.vertical ? oldSlot.left : oldSlot.top;
            if (onCut != was.cut) {
                continue;
            }
            const LONG target = was.vertical ? newRect.left : newRect.top;
            const LONG shown = was.vertical ? oldShown.left : oldShown.top;
            if (AbsLong(target - shown) > 2) {
                moved[k] = true;
                desired[k] = target - a;
            }
        }
    }

    // Step 2: walk from split 0 upward, recomputing each remaining region with the
    // ratios written so far. A split whose orientation has changed no longer cuts
    // the same edges, so it is left alone.
    bool changed = false;
    std::vector<LayoutSplit> splits;
    std::vector<RECT> slots;
    for (int i = 0; i + 1 < count; ++i) {
        const size_t k = static_cast<size_t>(i);
        WalkSplits(expanded, count, p, &splits, &slots);
        const LayoutSplit& now = splits[k];
        if (now.vertical != oldSplits[k].vertical || now.hi <= now.lo) {
            continue;
        }
        double ratio = 0.0;
        if (moved[k]) {
            ratio = ClampRatio(RatioTo(now.lo, now.hi, desired[k]), 0.1, 0.9);
        } else if (AbsLong(now.cut - desired[k]) >= 1) {
            // Not dragged, but earlier writes would move this cut: pin it in place.
            ratio = ClampRatio(RatioTo(now.lo, now.hi, desired[k]), 0.05, 0.95);
        } else {
            continue;  // stays at its current entry (0.0 = default)
        }
        double& stored = p.ratios[k];
        if (stored != ratio) {
            stored = ratio;
            changed = true;
        }
    }
    return changed;
}

bool ToggleSplitAt(LayoutParams& p, int count, int index) {
    if (p.kind != LayoutKind::Dwindle || count < 2 || index < 0 || index >= count) {
        return false;
    }
    // The last window has no split of its own; it shares the previous one.
    const int split = index < count - 1 ? index : count - 2;
    const size_t splitCount = static_cast<size_t>(count - 1);
    if (p.flips.size() < splitCount) {
        p.flips.resize(splitCount, 0);
    }
    int& flip = p.flips[static_cast<size_t>(split)];
    flip = flip != 0 ? 0 : 1;
    return true;
}

int FindNeighbor(const std::vector<RECT>& rects, int from, Direction dir) {
    const int n = static_cast<int>(rects.size());
    if (from < 0 || from >= n) {
        return -1;
    }
    int best = -1;
    NeighborKey bestKey;
    for (int i = 0; i < n; ++i) {
        // Also skips `from` itself.
        if (RectEquals(rects[i], rects[from])) {
            continue;
        }
        NeighborKey key;
        if (!MakeNeighborKey(rects[from], rects[i], dir, &key)) {
            continue;
        }
        // Strict comparison keeps the lowest index on full ties.
        if (best < 0 || KeyBetter(key, bestKey)) {
            best = i;
            bestKey = key;
        }
    }
    return best;
}

LayoutKind NextLayout(LayoutKind k) {
    constexpr int kCount = static_cast<int>(LayoutKind::Count_);
    const int v = static_cast<int>(k);
    if (v < 0 || v >= kCount) {
        return LayoutKind::Dwindle;
    }
    return static_cast<LayoutKind>((v + 1) % kCount);
}

const wchar_t* LayoutName(LayoutKind k) {
    switch (k) {
    case LayoutKind::Dwindle:
        return L"dwindle";
    case LayoutKind::MasterStack:
        return L"master";
    case LayoutKind::Columns:
        return L"columns";
    case LayoutKind::Rows:
        return L"rows";
    case LayoutKind::Monocle:
        return L"monocle";
    case LayoutKind::Mainstage:
        return L"mainstage";
    case LayoutKind::Count_:
        break;
    }
    return L"unknown";
}

bool ParseLayoutName(const wchar_t* s, LayoutKind* out) {
    if (s == nullptr || out == nullptr) {
        return false;
    }
    for (const LayoutNameEntry& entry : kLayoutNames) {
        if (EqualsIgnoreCase(s, entry.name)) {
            *out = entry.kind;
            return true;
        }
    }
    return false;
}

}  // namespace ht

// =============================================================================
// 30_win32.cpp — Win32 helpers: window facts, classification, placement, focus,
// workspace hide/show and persistence of the hidden-window list.
// =============================================================================

namespace ht {

namespace {

constexpr DWORD kMaxProcessPathChars = 1024;

#ifndef HT_STANDALONE
constexpr wchar_t kHiddenKey[] = L"hiddenWindows";
constexpr wchar_t kAlwaysFloatKey[] = L"alwaysFloatApps";
#endif

// Shell and system classes that are never managed. Exact, case-sensitive.
constexpr const wchar_t* kIgnoredClasses[] = {
    L"Shell_TrayWnd",
    L"Shell_SecondaryTrayWnd",
    L"Progman",
    L"WorkerW",
    L"NotifyIconOverflowWindow",
    L"Windows.UI.Core.CoreWindow",
    L"MultitaskingViewFrame",
    L"ForegroundStaging",
    L"TaskListThumbnailWnd",
    L"XamlExplorerHostIslandWindow",
    L"TopLevelWindowForOverflowXamlIsland",
    L"#32768",
};

bool IsIgnoredClass(const std::wstring& name) {
    for (const wchar_t* cls : kIgnoredClasses) {
        if (name == cls) {
            return true;
        }
    }
    return false;
}

// Lowercase file name of the process image, or "" if it cannot be queried.
std::wstring ProcessNameOf(DWORD pid) {
    if (pid == 0) {
        return {};
    }
    HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (!process) {
        return {};
    }
    std::wstring path(kMaxProcessPathChars, L'\0');
    DWORD size = kMaxProcessPathChars;
    std::wstring name;
    if (QueryFullProcessImageNameW(process, 0, path.data(), &size)) {
        path.resize(size);
        const size_t slash = path.find_last_of(L'\\');
        name = path.substr(slash == std::wstring::npos ? 0 : slash + 1);
        for (wchar_t& ch : name) {
            ch = static_cast<wchar_t>(std::towlower(ch));
        }
    }
    CloseHandle(process);
    return name;
}

unsigned long long HwndToNumber(HWND hwnd) {
    return static_cast<unsigned long long>(reinterpret_cast<ULONG_PTR>(hwnd));
}

// g.alwaysFloatApps as one '|'-joined string, sorted so the stored value and the
// log line are the same every time. Exe names cannot contain '|'.
std::wstring JoinAlwaysFloatApps() {
    std::vector<std::wstring> names(g.alwaysFloatApps.begin(),
                                    g.alwaysFloatApps.end());
    std::sort(names.begin(), names.end());
    std::wstring text;
    for (const std::wstring& name : names) {
        if (!text.empty()) {
            text += L'|';
        }
        text += name;
    }
    return text;
}

// One SetWindowPos call. Returns false only for ERROR_ACCESS_DENIED.
bool MoveOnce(HWND hwnd, const RECT& rc) {
    constexpr UINT kFlags = SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOACTIVATE |
                            SWP_ASYNCWINDOWPOS;
    if (SetWindowPos(hwnd, nullptr, rc.left, rc.top, rc.right - rc.left,
                     rc.bottom - rc.top, kFlags)) {
        return true;
    }
    const DWORD error = GetLastError();
    if (error == ERROR_ACCESS_DENIED) {
        HT_LOG(L"PlaceWindow: access denied for window %llx", HwndToNumber(hwnd));
        return false;
    }
    HT_LOG(L"PlaceWindow: SetWindowPos failed for window %llx, error %lu",
           HwndToNumber(hwnd), error);
    return true;
}

#ifndef HT_STANDALONE
constexpr DWORD kLogMaxFileBytes = 2 * 1024 * 1024;

// Appends "HH:MM:SS.mmm <text>\r\n" as UTF-8 to <mod storage>\hawktile.log.
// Every failure is ignored. Called only from LogLine.
void LogAppendToFile(const wchar_t* text) {
    WCHAR dir[MAX_PATH] = {};
    if (Wh_GetModStoragePath(dir, MAX_PATH) == 0) {
        return;
    }
    if (!CreateDirectoryW(dir, nullptr) &&
        GetLastError() != ERROR_ALREADY_EXISTS) {
        return;
    }
    const std::wstring path = std::wstring(dir) + L"\\hawktile.log";

    WIN32_FILE_ATTRIBUTE_DATA info{};
    if (GetFileAttributesExW(path.c_str(), GetFileExInfoStandard, &info) &&
        (info.nFileSizeHigh != 0 || info.nFileSizeLow > kLogMaxFileBytes)) {
        DeleteFileW(path.c_str());
    }

    const HANDLE file = CreateFileW(path.c_str(), FILE_APPEND_DATA,
                                    FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
                                    OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE) {
        return;
    }

    SYSTEMTIME now{};
    GetLocalTime(&now);
    WCHAR stamp[16] = {};
    swprintf(stamp, 16, L"%02u:%02u:%02u.%03u ", now.wHour, now.wMinute,
             now.wSecond, now.wMilliseconds);
    std::wstring line = stamp;
    line += text;
    line += L"\r\n";

    const int length = static_cast<int>(line.size());
    const int bytes = WideCharToMultiByte(CP_UTF8, 0, line.c_str(), length,
                                          nullptr, 0, nullptr, nullptr);
    if (bytes > 0) {
        std::string utf8(static_cast<size_t>(bytes), '\0');
        WideCharToMultiByte(CP_UTF8, 0, line.c_str(), length, utf8.data(),
                            bytes, nullptr, nullptr);
        DWORD written = 0;
        WriteFile(file, utf8.data(), static_cast<DWORD>(utf8.size()), &written,
                  nullptr);
    }
    CloseHandle(file);
}
#endif

}  // namespace

bool GetWindowFacts(HWND hwnd, WindowFacts* out) {
    if (!IsWindow(hwnd)) {
        return false;
    }
    WindowFacts facts;
    facts.hwnd = hwnd;
    GetWindowThreadProcessId(hwnd, &facts.pid);
    facts.processName = ProcessNameOf(facts.pid);

    WCHAR className[256] = {};
    GetClassNameW(hwnd, className, 256);
    facts.className = className;

    std::wstring title(static_cast<size_t>(GetWindowTextLengthW(hwnd)) + 1,
                       L'\0');
    const int copied =
        GetWindowTextW(hwnd, title.data(), static_cast<int>(title.size()));
    title.resize(copied > 0 ? static_cast<size_t>(copied) : 0);
    facts.title = title;

    facts.style = GetWindowLongPtrW(hwnd, GWL_STYLE);
    facts.exStyle = GetWindowLongPtrW(hwnd, GWL_EXSTYLE);
    facts.owner = GetWindow(hwnd, GW_OWNER);
    facts.cloaked = IsWindowCloaked(hwnd);
    *out = facts;
    return true;
}

bool IsWindowCloaked(HWND hwnd) {
    DWORD cloaked = 0;
    const HRESULT hr =
        DwmGetWindowAttribute(hwnd, kDwmCloaked, &cloaked, sizeof(cloaked));
    return SUCCEEDED(hr) && cloaked != 0;
}

Eligibility ClassifyWindow(HWND hwnd, const Settings& settings) {
    WindowFacts f;
    if (!GetWindowFacts(hwnd, &f)) {
        return Eligibility::Ignore;
    }
    if (hwnd != GetAncestor(hwnd, GA_ROOT) || (f.style & WS_CHILD)) {
        return Eligibility::Ignore;
    }
    if (f.pid == GetCurrentProcessId()) {
        return Eligibility::Ignore;
    }
    const bool visible = IsWindowVisible(hwnd) != FALSE;
    if (!visible && !g.hiddenByUs.contains(hwnd)) {
        return Eligibility::Ignore;
    }
    if (f.cloaked || (f.exStyle & WS_EX_TOOLWINDOW) ||
        (f.exStyle & WS_EX_NOACTIVATE)) {
        return Eligibility::Ignore;
    }
    if (f.className == kEngineWindowClass || IsIgnoredClass(f.className)) {
        return Eligibility::Ignore;
    }
    const bool thickFrame = (f.style & WS_THICKFRAME) != 0;
    if (!thickFrame && (f.style & WS_CAPTION) != WS_CAPTION) {
        return Eligibility::Ignore;
    }
    if (visible) {
        const RECT frame = GetVisibleFrame(hwnd);
        if (frame.right - frame.left < 40 || frame.bottom - frame.top < 40) {
            return Eligibility::Ignore;
        }
    }
    if (AnyRuleMatches(settings.ignoreRules, f)) {
        return Eligibility::Ignore;
    }
    if (!f.processName.empty() && g.alwaysFloatApps.contains(f.processName)) {
        return Eligibility::Float;
    }

    if (AnyRuleMatches(settings.floatRules, f)) {
        return Eligibility::Float;
    }
    if (f.owner != nullptr) {
        return Eligibility::Float;
    }
    if (settings.floatTopmost && (f.exStyle & WS_EX_TOPMOST)) {
        return Eligibility::Float;
    }
    if (settings.floatNonResizable && !thickFrame) {
        return Eligibility::Float;
    }
    return Eligibility::Tile;
}

RECT GetVisibleFrame(HWND hwnd) {
    RECT rc{};
    if (SUCCEEDED(DwmGetWindowAttribute(hwnd, kDwmExtendedFrameBounds, &rc,
                                        sizeof(rc)))) {
        return rc;
    }
    GetWindowRect(hwnd, &rc);
    return rc;
}

bool PlaceWindow(HWND hwnd, const RECT& visibleTarget) {
    if (IsIconic(hwnd) || IsRectEmpty(&visibleTarget)) {
        return true;
    }
    RECT wr{};
    GetWindowRect(hwnd, &wr);
    const RECT fr = GetVisibleFrame(hwnd);

    // Invisible resize borders around the visible frame.
    const LONG left = std::max<LONG>(fr.left - wr.left, 0);
    const LONG top = std::max<LONG>(fr.top - wr.top, 0);
    const LONG right = std::max<LONG>(wr.right - fr.right, 0);
    const LONG bottom = std::max<LONG>(wr.bottom - fr.bottom, 0);
    const RECT want{visibleTarget.left - left, visibleTarget.top - top,
                    visibleTarget.right + right, visibleTarget.bottom + bottom};
    if (EqualRect(&want, &wr)) {
        return true;
    }

    // Taken before the first move, which may change the window's monitor.
    const bool crossesMonitor =
        MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST) !=
        MonitorFromRect(&visibleTarget, MONITOR_DEFAULTTONEAREST);
    if (!MoveOnce(hwnd, want)) {
        return false;
    }
    if (crossesMonitor && !MoveOnce(hwnd, want)) {
        return false;
    }
    return true;
}

void FocusWindow(HWND hwnd) {
    if (!IsWindow(hwnd)) {
        return;
    }
    if (IsIconic(hwnd)) {
        ShowWindowAsync(hwnd, SW_RESTORE);
    }
    SetForegroundWindow(hwnd);
    if (GetForegroundWindow() != hwnd) {
        SendMenuMaskKey();
        SetForegroundWindow(hwnd);
    }
    if (GetForegroundWindow() != hwnd) {
        HT_LOG(L"FocusWindow: window %llx is not in the foreground",
               HwndToNumber(hwnd));
    }
}

void SendMenuMaskKey() {
    INPUT inputs[2] = {};
    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = kMenuMaskVk;
    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = kMenuMaskVk;
    inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(2, inputs, sizeof(INPUT));
}

void SetFocusBorder(HWND hwnd, COLORREF color) {
    if (!IsWindow(hwnd)) {
        return;
    }
    DwmSetWindowAttribute(hwnd, kDwmBorderColor, &color, sizeof(color));
}

void ClearFocusBorder(HWND hwnd) {
    if (!IsWindow(hwnd)) {
        return;
    }
    COLORREF color = kDwmColorDefault;
    DwmSetWindowAttribute(hwnd, kDwmBorderColor, &color, sizeof(color));
}

void HideForWorkspace(HWND hwnd) {
    g.hiddenByUs.insert(hwnd);
    ShowWindowAsync(hwnd, SW_HIDE);
    PersistHiddenList();
}

void ShowForWorkspace(HWND hwnd) {
    g.hiddenByUs.erase(hwnd);
    ShowWindowAsync(hwnd, SW_SHOWNA);
    PersistHiddenList();
    g.needsNudge.insert(hwnd);
    RequestNudge();
}

void RequestNudge() {
    if (g.engineHwnd) {
        SetTimer(g.engineHwnd, kTimerNudge, 300, nullptr);
    }
}

void NudgeWindow(HWND hwnd) {
    if (!IsWindow(hwnd) || !IsWindowVisible(hwnd) || IsIconic(hwnd) ||
        IsZoomed(hwnd)) {
        return;
    }
    RECT r{};
    GetWindowRect(hwnd, &r);
    const LONG width = r.right - r.left;
    const LONG height = r.bottom - r.top;
    if (width <= 1) {
        return;
    }
    constexpr UINT kFlags = SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOACTIVATE |
                            SWP_ASYNCWINDOWPOS;
    // One pixel narrower, then the original size: the app gets two real WM_SIZE
    // messages and repaints at the final size.
    SetWindowPos(hwnd, nullptr, r.left, r.top, width - 1, height, kFlags);
    SetWindowPos(hwnd, nullptr, r.left, r.top, width, height, kFlags);
}

void PersistHiddenList() {
#ifndef HT_STANDALONE
    if (g.hiddenByUs.empty()) {
        Wh_DeleteValue(kHiddenKey);
        return;
    }
    std::wstring text;
    for (HWND hwnd : g.hiddenByUs) {
        if (!text.empty()) {
            text += L',';
        }
        // The pid guards against HWND reuse after a crash: only a window still
        // owned by the same process is re-shown on the next start.
        DWORD pid = 0;
        GetWindowThreadProcessId(hwnd, &pid);
        WCHAR token[48] = {};
        swprintf(token, 48, L"%llx:%lx", HwndToNumber(hwnd), pid);
        text += token;
    }
    Wh_SetStringValue(kHiddenKey, text.c_str());
#endif
}

void RestoreLeftoverHiddenWindows() {
#ifndef HT_STANDALONE
    WCHAR buffer[8192] = {};
    Wh_GetStringValue(kHiddenKey, buffer, 8192);
    const std::wstring text(buffer);
    int shown = 0;
    size_t pos = 0;
    while (pos < text.size()) {
        size_t comma = text.find(L',', pos);
        if (comma == std::wstring::npos) {
            comma = text.size();
        }
        const std::wstring token = text.substr(pos, comma - pos);
        pos = comma + 1;

        wchar_t* end = nullptr;
        const unsigned long long value =
            std::wcstoull(token.c_str(), &end, 16);
        if (end == token.c_str() || *end != L':') {
            continue;
        }
        const DWORD savedPid =
            static_cast<DWORD>(std::wcstoul(end + 1, nullptr, 16));
        const HWND hwnd = reinterpret_cast<HWND>(static_cast<ULONG_PTR>(value));
        DWORD pid = 0;
        if (IsWindow(hwnd) && GetWindowThreadProcessId(hwnd, &pid) &&
            pid == savedPid && !IsWindowVisible(hwnd)) {
            ShowWindowAsync(hwnd, SW_SHOWNA);
            ++shown;
        }
    }
    HT_LOG(L"RestoreLeftoverHiddenWindows: showed %d window(s)", shown);
    Wh_DeleteValue(kHiddenKey);
#endif
}

void LoadAlwaysFloatApps() {
    g.alwaysFloatApps.clear();
#ifndef HT_STANDALONE
    WCHAR buffer[8192] = {};
    Wh_GetStringValue(kAlwaysFloatKey, buffer, 8192);
    const std::wstring text(buffer);
    size_t pos = 0;
    while (pos < text.size()) {
        size_t bar = text.find(L'|', pos);
        if (bar == std::wstring::npos) {
            bar = text.size();
        }
        const std::wstring token = text.substr(pos, bar - pos);
        pos = bar + 1;

        const size_t begin = token.find_first_not_of(L" \t\r\n");
        if (begin == std::wstring::npos) {
            continue;
        }
        const size_t end = token.find_last_not_of(L" \t\r\n");
        std::wstring name = token.substr(begin, end - begin + 1);
        for (wchar_t& ch : name) {
            ch = static_cast<wchar_t>(std::towlower(ch));
        }
        g.alwaysFloatApps.insert(name);
    }
#endif
    HT_LOG(L"always-float apps: %d (%s)",
           static_cast<int>(g.alwaysFloatApps.size()),
           JoinAlwaysFloatApps().c_str());
}

void SaveAlwaysFloatApps() {
#ifndef HT_STANDALONE
    if (g.alwaysFloatApps.empty()) {
        Wh_DeleteValue(kAlwaysFloatKey);
        return;
    }
    Wh_SetStringValue(kAlwaysFloatKey, JoinAlwaysFloatApps().c_str());
#endif
}

bool DebugLogFileEnabled() { return g.settings.debugLog; }

void LogLine(const wchar_t* format, ...) {
    wchar_t buf[1024] = {};
    va_list args;
    va_start(args, format);
    // Negative return = truncated. buf[1023] is forced to NUL below either way.
    _vsnwprintf(buf, 1023, format, args);
    va_end(args);
    buf[1023] = L'\0';

#ifndef HT_STANDALONE
    Wh_Log(L"%s", buf);
    if (g.settings.debugLog) {
        LogAppendToFile(buf);
    }
#else
    fwprintf(stderr, L"%s\n", buf);
#endif
}

}  // namespace ht

// =============================================================================
// 40_config.cpp — settings loading, hotkey / command / rule parsing and hotkey
// registration.
// =============================================================================

namespace ht {

namespace {

struct KeyName {
    const wchar_t* name;
    UINT vk;
};

constexpr KeyName kKeyNames[] = {
    {L"left", VK_LEFT},
    {L"right", VK_RIGHT},
    {L"up", VK_UP},
    {L"down", VK_DOWN},
    {L"enter", VK_RETURN},
    {L"return", VK_RETURN},
    {L"space", VK_SPACE},
    {L"tab", VK_TAB},
    {L"escape", VK_ESCAPE},
    {L"esc", VK_ESCAPE},
    {L"backspace", VK_BACK},
    {L"delete", VK_DELETE},
    {L"del", VK_DELETE},
    {L"insert", VK_INSERT},
    {L"ins", VK_INSERT},
    {L"home", VK_HOME},
    {L"end", VK_END},
    {L"pageup", VK_PRIOR},
    {L"pgup", VK_PRIOR},
    {L"pagedown", VK_NEXT},
    {L"pgdn", VK_NEXT},
    {L"minus", VK_OEM_MINUS},
    {L"plus", VK_OEM_PLUS},
    {L"equals", VK_OEM_PLUS},
    {L"comma", VK_OEM_COMMA},
    {L"period", VK_OEM_PERIOD},
    {L"semicolon", VK_OEM_1},
    {L"slash", VK_OEM_2},
    {L"backtick", VK_OEM_3},
    {L"grave", VK_OEM_3},
    {L"bracketleft", VK_OEM_4},
    {L"backslash", VK_OEM_5},
    {L"bracketright", VK_OEM_6},
    {L"quote", VK_OEM_7},
};

struct NoArgCommand {
    const wchar_t* name;
    CommandKind kind;
};

constexpr NoArgCommand kNoArgCommands[] = {
    {L"nextlayout", CommandKind::NextLayout},
    {L"togglefloating", CommandKind::ToggleFloating},
    {L"togglealwaysfloat", CommandKind::ToggleAlwaysFloat},
    {L"togglemonocle", CommandKind::ToggleMonocle},
    {L"fullscreen", CommandKind::Fullscreen},
    {L"swapwithmaster", CommandKind::SwapWithMaster},
    {L"growmaster", CommandKind::GrowMaster},
    {L"shrinkmaster", CommandKind::ShrinkMaster},
    {L"rescan", CommandKind::Rescan},
    {L"toggletiling", CommandKind::ToggleTiling},
    {L"closewindow", CommandKind::CloseWindow},
    {L"togglesplit", CommandKind::ToggleSplit},
};

struct RulePrefix {
    const wchar_t* text;
    RuleField field;
};

constexpr RulePrefix kRulePrefixes[] = {
    {L"process:", RuleField::Process},
    {L"class:", RuleField::Class},
    {L"title:", RuleField::Title},
};

bool IsSpace(wchar_t c) {
    return std::iswspace(c) != 0;
}

std::wstring ToLower(const std::wstring& s) {
    std::wstring out = s;
    for (wchar_t& c : out) {
        c = static_cast<wchar_t>(std::towlower(c));
    }
    return out;
}

std::wstring Trim(const std::wstring& s) {
    size_t begin = 0;
    size_t end = s.size();
    while (begin < end && IsSpace(s[begin])) {
        ++begin;
    }
    while (end > begin && IsSpace(s[end - 1])) {
        --end;
    }
    return s.substr(begin, end - begin);
}

// Keeps empty fields, so "" yields one empty part.
std::vector<std::wstring> SplitChar(const std::wstring& s, wchar_t sep) {
    std::vector<std::wstring> parts;
    size_t start = 0;
    for (;;) {
        const size_t pos = s.find(sep, start);
        if (pos == std::wstring::npos) {
            parts.push_back(s.substr(start));
            return parts;
        }
        parts.push_back(s.substr(start, pos - start));
        start = pos + 1;
    }
}

// Splits on runs of whitespace; leading, trailing and repeated spaces vanish.
std::vector<std::wstring> SplitWords(const std::wstring& s) {
    std::vector<std::wstring> words;
    size_t i = 0;
    while (i < s.size()) {
        while (i < s.size() && IsSpace(s[i])) {
            ++i;
        }
        const size_t start = i;
        while (i < s.size() && !IsSpace(s[i])) {
            ++i;
        }
        if (i > start) {
            words.push_back(s.substr(start, i - start));
        }
    }
    return words;
}

bool ParseDecimal(const std::wstring& s, int* out) {
    if (s.empty() || s.size() > 3) {
        return false;
    }
    int value = 0;
    for (wchar_t c : s) {
        if (c < L'0' || c > L'9') {
            return false;
        }
        value = value * 10 + (c - L'0');
    }
    *out = value;
    return true;
}

bool ParseHex(const std::wstring& s, unsigned* out) {
    if (s.empty() || s.size() > 8) {
        return false;
    }
    unsigned value = 0;
    for (wchar_t c : s) {
        unsigned digit = 0;
        if (c >= L'0' && c <= L'9') {
            digit = c - L'0';
        } else if (c >= L'a' && c <= L'f') {
            digit = c - L'a' + 10;
        } else if (c >= L'A' && c <= L'F') {
            digit = c - L'A' + 10;
        } else {
            return false;
        }
        value = value * 16 + digit;
    }
    *out = value;
    return true;
}

// `name` is already lowercase.
bool ResolveKey(const std::wstring& name, UINT* vk) {
    if (name.size() == 1) {
        const wchar_t c = name[0];
        if (c >= L'a' && c <= L'z') {
            *vk = static_cast<UINT>(L'A' + (c - L'a'));
            return true;
        }
        if (c >= L'0' && c <= L'9') {
            *vk = static_cast<UINT>(c);
            return true;
        }
        return false;
    }
    if (name.size() >= 2 && name[0] == L'f') {
        int n = 0;
        if (ParseDecimal(name.substr(1), &n) && n >= 1 && n <= 24) {
            *vk = static_cast<UINT>(VK_F1 + n - 1);
            return true;
        }
    }
    if (name.size() > 2 && name.compare(0, 2, L"vk") == 0) {
        unsigned code = 0;
        if (ParseHex(name.substr(2), &code) && code >= 1 && code <= 254) {
            *vk = code;
            return true;
        }
        return false;
    }
    for (const KeyName& key : kKeyNames) {
        if (name == key.name) {
            *vk = key.vk;
            return true;
        }
    }
    return false;
}

bool ParseDirection(const std::wstring& word, Direction* out) {
    if (word == L"left" || word == L"l") {
        *out = Direction::Left;
    } else if (word == L"right" || word == L"r") {
        *out = Direction::Right;
    } else if (word == L"up" || word == L"u") {
        *out = Direction::Up;
    } else if (word == L"down" || word == L"d") {
        *out = Direction::Down;
    } else {
        return false;
    }
    return true;
}

bool ParseWorkspaceNumber(const std::wstring& word, int* out) {
    int n = 0;
    if (!ParseDecimal(word, &n) || n < 1 || n > 9) {
        return false;
    }
    *out = n;
    return true;
}

bool MakeBinding(const std::wstring& keys, const std::wstring& command, Binding* out) {
    Binding binding;
    if (!ParseHotkey(keys.c_str(), &binding.modifiers, &binding.vk) ||
        !ParseCommand(command.c_str(), &binding.cmd)) {
        return false;
    }
    binding.keysText = keys;
    binding.commandText = command;
    *out = binding;
    return true;
}

#ifndef HT_STANDALONE

constexpr int kMaxListEntries = 256;

// Wh_GetStringSetting may hand back null for a missing value. A non-null result
// is copied and released.
template <typename... Args>
std::wstring SettingString(PCWSTR name, Args... args) {
    const PCWSTR value = Wh_GetStringSetting(name, args...);
    std::wstring out = value ? value : L"";
    if (value) {
        Wh_FreeStringSetting(value);
    }
    return out;
}

bool ParseColor(const std::wstring& text, COLORREF* out) {
    std::wstring hex = Trim(text);
    if (!hex.empty() && hex[0] == L'#') {
        hex.erase(0, 1);
    }
    unsigned value = 0;
    if (hex.size() != 6 || !ParseHex(hex, &value)) {
        return false;
    }
    *out = RGB((value >> 16) & 0xFF, (value >> 8) & 0xFF, value & 0xFF);
    return true;
}

// Reads name[0], name[1], ... until an empty entry or kMaxListEntries.
std::vector<MatchRule> ReadRules(const wchar_t* format, const wchar_t* label) {
    std::vector<MatchRule> rules;
    for (int i = 0; i < kMaxListEntries; ++i) {
        const std::wstring text = SettingString(format, i);
        if (text.empty()) {
            break;
        }
        MatchRule rule;
        if (ParseRule(text.c_str(), &rule)) {
            rules.push_back(rule);
        } else {
            HT_LOG(L"Skipping invalid %s rule '%s'", label, text.c_str());
        }
    }
    return rules;
}

void ReadBindings(Settings& s) {
    std::vector<Binding> bindings;
    for (int i = 0; i < kMaxListEntries; ++i) {
        const std::wstring keys = SettingString(L"binds[%d].keys", i);
        const std::wstring command = SettingString(L"binds[%d].command", i);
        if (keys.empty() && command.empty()) {
            break;
        }
        Binding binding;
        if (MakeBinding(keys, command, &binding)) {
            bindings.push_back(binding);
        } else {
            HT_LOG(L"Skipping invalid binding %d: keys '%s', command '%s'", i,
                   keys.c_str(), command.c_str());
        }
    }
    if (bindings.empty()) {
        HT_LOG(L"No valid key bindings in settings, keeping the defaults");
        return;
    }
    s.bindings = std::move(bindings);
}

void ReadWindhawkSettings(Settings& s) {
    // Read first: LoadSettings passes g.settings, and LogLine checks debugLog,
    // so the rest of this load is written to the file when it is on.
    s.debugLog = Wh_GetIntSetting(L"advanced.debugLog") != 0;
    s.outerGap = std::clamp(Wh_GetIntSetting(L"layout.outerGap"), 0, 200);
    s.innerGap = std::clamp(Wh_GetIntSetting(L"layout.innerGap"), 0, 200);
    s.workspaceCount = std::clamp(Wh_GetIntSetting(L"layout.workspaces"), 1, 9);

    const std::wstring layoutName = SettingString(L"layout.default");
    LayoutKind layout = LayoutKind::Dwindle;
    if (ParseLayoutName(layoutName.c_str(), &layout)) {
        s.defaultLayout = layout;
    } else {
        HT_LOG(L"Invalid defaultLayout '%s', keeping %s", layoutName.c_str(),
               LayoutName(s.defaultLayout));
    }

    s.masterRatio = std::clamp(Wh_GetIntSetting(L"layout.masterRatio"), 15, 85) / 100.0;
    s.ratioStep = std::clamp(Wh_GetIntSetting(L"layout.resizeStep"), 1, 25) / 100.0;
    const int stageWidth = Wh_GetIntSetting(L"layout.stageWidth");
    s.stageRatio = std::clamp(stageWidth <= 0 ? 50 : stageWidth, 20, 90) / 100.0;

    // One entry per workspace; blanks in between are allowed, so every index is
    // read. Trailing "use the default" entries are dropped.
    s.workspaceLayouts.clear();
    for (int i = 0; i < 9; ++i) {
        const std::wstring name =
            Trim(SettingString(L"layout.workspaceLayouts[%d]", i));
        LayoutKind kind = LayoutKind::Count_;
        if (!name.empty() && ToLower(name) != L"default" &&
            !ParseLayoutName(name.c_str(), &kind)) {
            HT_LOG(L"Invalid layout '%s' for workspace %d, using the default",
                   name.c_str(), i + 1);
            kind = LayoutKind::Count_;
        }
        s.workspaceLayouts.push_back(kind);
    }
    while (!s.workspaceLayouts.empty() &&
           s.workspaceLayouts.back() == LayoutKind::Count_) {
        s.workspaceLayouts.pop_back();
    }

    s.focusBorder = Wh_GetIntSetting(L"border.enabled") != 0;
    const std::wstring colorText = SettingString(L"border.color");
    COLORREF color = 0;
    if (ParseColor(colorText, &color)) {
        s.focusBorderColor = color;
    } else {
        HT_LOG(L"Invalid focusBorderColor '%s', keeping the default",
               colorText.c_str());
    }
    s.floatNonResizable = Wh_GetIntSetting(L"windows.floatNonResizable") != 0;
    s.floatTopmost = Wh_GetIntSetting(L"windows.floatTopmost") != 0;

    s.tileMaximized = Wh_GetIntSetting(L"windows.tileMaximized") != 0;
    s.workspaceOsd = Wh_GetIntSetting(L"indicators.popup") != 0;
    const int osdDuration = Wh_GetIntSetting(L"indicators.popupDuration");
    s.osdDurationMs =
        std::clamp(osdDuration <= 0 ? 2000 : osdDuration, 100, 10000);
    s.holdOverview = Wh_GetIntSetting(L"indicators.holdAlt") != 0;
    const int holdDelay = Wh_GetIntSetting(L"indicators.holdDelay");
    s.holdDelayMs = std::clamp(holdDelay <= 0 ? 400 : holdDelay, 100, 2000);
    s.taskbarIndicator = Wh_GetIntSetting(L"indicators.taskbar") != 0;
    s.attentionDots = Wh_GetIntSetting(L"indicators.notificationDots") != 0;
    s.followActivation = Wh_GetIntSetting(L"windows.followActivation") != 0;
    s.mouseResize = Wh_GetIntSetting(L"windows.mouseResize") != 0;
    s.taskbarIndicatorOffset =
        std::clamp(Wh_GetIntSetting(L"indicators.taskbarOffset"), 0, 2000);

    s.floatRules = ReadRules(L"rules.float[%d]", L"float");
    s.ignoreRules = ReadRules(L"rules.ignore[%d]", L"ignore");
    ReadBindings(s);
}

#endif  // HT_STANDALONE

}  // namespace

bool ParseHotkey(const wchar_t* text, UINT* modifiers, UINT* vk) {
    if (!text || !modifiers || !vk) {
        return false;
    }
    UINT mods = 0;
    UINT key = 0;
    int keyCount = 0;
    for (const std::wstring& raw : SplitChar(ToLower(text), L'+')) {
        const std::wstring token = Trim(raw);
        if (token.empty()) {
            return false;
        }
        if (token == L"alt") {
            mods |= MOD_ALT;
        } else if (token == L"ctrl" || token == L"control") {
            mods |= MOD_CONTROL;
        } else if (token == L"shift") {
            mods |= MOD_SHIFT;
        } else if (token == L"win" || token == L"super") {
            mods |= MOD_WIN;
        } else {
            if (!ResolveKey(token, &key)) {
                return false;
            }
            ++keyCount;
        }
    }
    if (keyCount != 1) {
        return false;
    }
    *modifiers = mods;
    *vk = key;
    return true;
}

bool ParseCommand(const wchar_t* text, Command* out) {
    if (!text || !out) {
        return false;
    }
    const std::vector<std::wstring> words = SplitWords(ToLower(text));
    if (words.empty()) {
        return false;
    }
    const std::wstring& verb = words[0];
    const size_t argCount = words.size() - 1;
    Command cmd;
    if (verb == L"movefocus" || verb == L"swapwindow" ||
        verb == L"resizeactive") {
        if (argCount != 1 || !ParseDirection(words[1], &cmd.dir)) {
            return false;
        }
        if (verb == L"movefocus") {
            cmd.kind = CommandKind::MoveFocus;
        } else if (verb == L"swapwindow") {
            cmd.kind = CommandKind::SwapWindow;
        } else {
            cmd.kind = CommandKind::ResizeActive;
        }
    } else if (verb == L"workspace") {
        if (argCount != 1) {
            return false;
        }
        if (words[1] == L"next") {
            cmd.kind = CommandKind::WorkspaceNext;
        } else if (words[1] == L"prev") {
            cmd.kind = CommandKind::WorkspacePrev;
        } else if (ParseWorkspaceNumber(words[1], &cmd.arg)) {
            cmd.kind = CommandKind::Workspace;
        } else {
            return false;
        }
    } else if (verb == L"movetoworkspace" || verb == L"movetoworkspacesilent") {
        if (argCount != 1 || !ParseWorkspaceNumber(words[1], &cmd.arg)) {
            return false;
        }
        if (verb == L"movetoworkspace") {
            cmd.kind = CommandKind::MoveToWorkspace;
        } else {
            cmd.kind = CommandKind::MoveToWorkspaceSilent;
        }
    } else if (verb == L"focusmonitor" || verb == L"movetomonitor") {
        if (argCount != 1 || words[1] != L"next") {
            return false;
        }
        if (verb == L"focusmonitor") {
            cmd.kind = CommandKind::FocusMonitorNext;
        } else {
            cmd.kind = CommandKind::MoveToMonitorNext;
        }
    } else if (verb == L"layout") {
        if (argCount != 1 || !ParseLayoutName(words[1].c_str(), &cmd.layout)) {
            return false;
        }
        cmd.kind = CommandKind::SetLayout;
    } else {
        bool found = false;
        for (const NoArgCommand& entry : kNoArgCommands) {
            if (verb == entry.name) {
                found = true;
                cmd.kind = entry.kind;
                break;
            }
        }
        if (!found || argCount != 0) {
            return false;
        }
    }
    *out = cmd;
    return true;
}

bool ParseRule(const wchar_t* text, MatchRule* out) {
    if (!text || !out) {
        return false;
    }
    const std::wstring full = Trim(text);
    const std::wstring lower = ToLower(full);
    RuleField field = RuleField::Process;
    size_t prefixLength = 0;
    for (const RulePrefix& prefix : kRulePrefixes) {
        const size_t length = std::wcslen(prefix.text);
        if (lower.compare(0, length, prefix.text) == 0) {
            field = prefix.field;
            prefixLength = length;
            break;
        }
    }
    const std::wstring pattern = ToLower(Trim(full.substr(prefixLength)));
    if (pattern.empty()) {
        return false;
    }
    out->field = field;
    out->pattern = pattern;
    return true;
}

bool RuleMatches(const MatchRule& rule, const WindowFacts& facts) {
    if (rule.pattern.empty()) {
        return false;
    }
    const std::wstring pattern = ToLower(rule.pattern);
    if (rule.field == RuleField::Process) {
        return ToLower(facts.processName) == pattern;
    }
    if (rule.field == RuleField::Class) {
        return ToLower(facts.className) == pattern;
    }
    return ToLower(facts.title).find(pattern) != std::wstring::npos;
}

bool AnyRuleMatches(const std::vector<MatchRule>& rules, const WindowFacts& facts) {
    for (const MatchRule& rule : rules) {
        if (RuleMatches(rule, facts)) {
            return true;
        }
    }
    return false;
}

int RegisterBindings(HWND hwnd, const std::vector<Binding>& bindings) {
    int registered = 0;
    for (size_t i = 0; i < bindings.size(); ++i) {
        const Binding& binding = bindings[i];
        const int id = kHotkeyIdBase + static_cast<int>(i);
        if (RegisterHotKey(hwnd, id, binding.modifiers | MOD_NOREPEAT, binding.vk)) {
            ++registered;
        } else {
            HT_LOG(L"RegisterHotKey failed for '%s' (error %lu)",
                   binding.keysText.c_str(), GetLastError());
        }
    }
    return registered;
}

void UnregisterBindings(HWND hwnd, const std::vector<Binding>& bindings) {
    for (size_t i = 0; i < bindings.size(); ++i) {
        UnregisterHotKey(hwnd, kHotkeyIdBase + static_cast<int>(i));
    }
}

void LoadSettings(Settings& s) {
    s = Settings{};

    for (const wchar_t* text : kDefaultFloatRules) {
        MatchRule rule;
        if (ParseRule(text, &rule)) {
            s.floatRules.push_back(rule);
        } else {
            HT_LOG(L"Invalid default float rule '%s'", text);
        }
    }
    for (const DefaultBinding& def : kDefaultBindings) {
        Binding binding;
        if (MakeBinding(def.keys, def.command, &binding)) {
            s.bindings.push_back(binding);
        } else {
            HT_LOG(L"Invalid default binding '%s' -> '%s'", def.keys, def.command);
        }
    }

#ifndef HT_STANDALONE
    ReadWindhawkSettings(s);
#endif

    HT_LOG(L"Settings: gaps outer=%d inner=%d, workspaces=%d, layout=%s, "
           L"%d float rules, %d ignore rules, %d bindings, "
           L"tileMaximized=%d, workspaceOsd=%d, indicatorDuration=%d ms, "
           L"taskbarIndicator=%d, taskbarIndicatorOffset=%d px, "
           L"attentionDots=%d, followActivation=%d, holdOverview=%d, "
           L"holdDelayMs=%d, mouseResize=%d, debugLog=%d, stageWidth=%d%%, "
           L"%d workspace layouts",
           s.outerGap, s.innerGap, s.workspaceCount, LayoutName(s.defaultLayout),
           static_cast<int>(s.floatRules.size()),
           static_cast<int>(s.ignoreRules.size()),
           static_cast<int>(s.bindings.size()),
           static_cast<int>(s.tileMaximized), static_cast<int>(s.workspaceOsd),
           s.osdDurationMs, static_cast<int>(s.taskbarIndicator),
           s.taskbarIndicatorOffset, static_cast<int>(s.attentionDots),
           static_cast<int>(s.followActivation), static_cast<int>(s.holdOverview),
           s.holdDelayMs, static_cast<int>(s.mouseResize),
           static_cast<int>(s.debugLog),
           static_cast<int>(s.stageRatio * 100.0 + 0.5),
           static_cast<int>(s.workspaceLayouts.size()));
}

}  // namespace ht

#include <cmath>

namespace ht {

EngineState g;

namespace {

int MonitorCount() {
    return static_cast<int>(g.monitors.size());
}

int WsCount() {
    return std::clamp(g.settings.workspaceCount, 1, 9);
}

// Layout that settings.workspaceLayouts names for workspace `index`, or Count_.
LayoutKind ConfiguredLayout(const std::vector<LayoutKind>& layouts, int index) {
    if (index < 0 || index >= static_cast<int>(layouts.size())) {
        return LayoutKind::Count_;
    }
    return layouts[static_cast<size_t>(index)];
}

// A fresh workspace at position `index` of its monitor.
Workspace MakeWorkspace(int index) {
    const LayoutKind own = ConfiguredLayout(g.settings.workspaceLayouts, index);
    Workspace ws;
    ws.layout = own == LayoutKind::Count_ ? g.settings.defaultLayout : own;
    ws.prevLayout = ws.layout;
    return ws;
}

int IndexOf(const std::vector<HWND>& v, HWND hwnd) {
    for (size_t i = 0; i < v.size(); ++i) {
        if (v[i] == hwnd) return static_cast<int>(i);
    }
    return -1;
}

bool RemoveFrom(std::vector<HWND>& v, HWND hwnd) {
    const int i = IndexOf(v, hwnd);
    if (i < 0) return false;
    v.erase(v.begin() + i);
    return true;
}

bool InWorkspace(const Workspace& ws, HWND hwnd) {
    return hwnd && (IndexOf(ws.tiled, hwnd) >= 0 ||
                    IndexOf(ws.floating, hwnd) >= 0);
}

HWND FirstWindow(const Workspace& ws) {
    if (InWorkspace(ws, ws.lastFocus)) return ws.lastFocus;
    if (!ws.tiled.empty()) return ws.tiled.front();
    if (!ws.floating.empty()) return ws.floating.front();
    return nullptr;
}

int MonitorIndexOf(HMONITOR handle) {
    for (int i = 0; i < MonitorCount(); ++i) {
        if (g.monitors[i].handle == handle) return i;
    }
    return -1;
}

std::vector<RECT> MonitorRects() {
    std::vector<RECT> rects;
    for (const Monitor& m : g.monitors) rects.push_back(m.rcMonitor);
    return rects;
}

std::vector<RECT> FramesOf(const std::vector<HWND>& hwnds) {
    std::vector<RECT> frames;
    for (HWND h : hwnds) frames.push_back(GetVisibleFrame(h));
    return frames;
}

// Visible, non-iconic tiled windows in layout order. With appendCur, `cur`
// goes last so that directional focus can reach it while it floats.
std::vector<HWND> Candidates(const Workspace& ws, HWND cur, bool appendCur) {
    std::vector<HWND> out;
    for (HWND h : ws.tiled) {
        if (IsWindowVisible(h) && !IsIconic(h)) out.push_back(h);
    }
    if (appendCur) out.push_back(cur);
    return out;
}

// Tiled windows that Retile lays out, in layout order. Our own bookkeeping
// decides visibility, not IsWindowVisible: a window we just showed with
// ShowWindowAsync may not report visible yet.
std::vector<HWND> LayoutWindowsOf(const Workspace& ws) {
    std::vector<HWND> out;
    for (HWND h : ws.tiled) {
        if (!g.hiddenByUs.count(h) && IsWindow(h) && !IsIconic(h) &&
            !IsZoomed(h)) {
            out.push_back(h);
        }
    }
    return out;
}

// Work area of monitor `mon` inset by outerGap on every side; the plain work
// area when that inset would leave less than 50 px in either direction.
RECT LayoutAreaOf(int mon) {
    const RECT work = g.monitors[mon].rcWork;
    const int gap = g.settings.outerGap;
    RECT area{work.left + gap, work.top + gap, work.right - gap,
              work.bottom - gap};
    if (area.right - area.left < 50 || area.bottom - area.top < 50) {
        area = work;
    }
    return area;
}

// The layout parameters every geometry computation for `ws` uses.
LayoutParams LayoutParamsOf(const Workspace& ws) {
    LayoutParams p{ws.layout, g.settings.masterRatio, g.settings.innerGap,
                   ws.ratios, ws.flips, g.settings.stageRatio, {}};
    if (ws.layout == LayoutKind::Mainstage) {
        // A window without a side yet (until the next Retile) alternates.
        const std::vector<HWND> list = LayoutWindowsOf(ws);
        for (size_t i = 1; i < list.size(); ++i) {
            const auto side = ws.stageSide.find(list[i]);
            p.sides.push_back(side != ws.stageSide.end()
                                  ? side->second
                                  : static_cast<int>((i - 1) % 2));
        }
    }
    return p;
}

// Mainstage: drops the sides of windows that left the workspace and gives
// every side window without one the side with fewer tiles (left on a tie).
void AssignStageSides(Workspace& ws) {
    std::erase_if(ws.stageSide, [&ws](const auto& entry) {
        return IndexOf(ws.tiled, entry.first) < 0;
    });
    const std::vector<HWND> list = LayoutWindowsOf(ws);
    int perSide[2] = {0, 0};
    for (size_t i = 1; i < list.size(); ++i) {
        const auto side = ws.stageSide.find(list[i]);
        if (side != ws.stageSide.end()) ++perSide[side->second != 0 ? 1 : 0];
    }
    for (size_t i = 1; i < list.size(); ++i) {
        if (ws.stageSide.count(list[i]) > 0) continue;
        const int side = perSide[1] < perSide[0] ? 1 : 0;
        ws.stageSide[list[i]] = side;
        ++perSide[side];
    }
}

// Frames for directional navigation, one per entry of `hwnds`. Mainstage
// windows are stacked on the stage, so each is represented by its layout slot
// (the stage or its side cell) instead of its real frame.
std::vector<RECT> NavFramesOf(int mon, const Workspace& ws,
                              const std::vector<HWND>& hwnds) {
    std::vector<RECT> frames = FramesOf(hwnds);
    if (ws.layout != LayoutKind::Mainstage) return frames;
    const std::vector<HWND> list = LayoutWindowsOf(ws);
    const std::vector<RECT> slots = ComputeLayout(
        LayoutAreaOf(mon), static_cast<int>(list.size()), LayoutParamsOf(ws));
    for (size_t i = 0; i < hwnds.size(); ++i) {
        const int idx = IndexOf(list, hwnds[i]);
        if (idx >= 0 && idx < static_cast<int>(slots.size())) {
            frames[i] = slots[idx];
        }
    }
    return frames;
}

// Mainstage: the layout window on top of the z-order is the one the user sees
// on the stage, so it takes layout index 0. The window it replaces takes its
// place in the list (and so its side cell).
void StageFollowsZOrder(Workspace& ws) {
    const std::vector<HWND> list = LayoutWindowsOf(ws);
    if (list.size() < 2) return;
    // Bounded: the z-order can change while it is walked.
    int steps = 0;
    for (HWND h = GetTopWindow(nullptr); h && steps < 4096;
         h = GetWindow(h, GW_HWNDNEXT), ++steps) {
        if (IndexOf(list, h) < 0) continue;
        if (h != list[0]) {
            std::swap(ws.tiled[IndexOf(ws.tiled, list[0])],
                      ws.tiled[IndexOf(ws.tiled, h)]);
            // The window leaving the stage also takes the other's side.
            const auto side = ws.stageSide.find(h);
            if (side != ws.stageSide.end()) ws.stageSide[list[0]] = side->second;
        }
        return;
    }
}

void SetWorkspaceLayout(Workspace& ws, LayoutKind kind) {
    // Remember the layout we leave so togglemonocle can return to it.
    if (kind == LayoutKind::Monocle && ws.layout != LayoutKind::Monocle) {
        ws.prevLayout = ws.layout;
    }
    // Mainstage's one ratio is the stage width, unrelated to any split.
    if (kind != ws.layout && (kind == LayoutKind::Mainstage ||
                              ws.layout == LayoutKind::Mainstage)) {
        ws.ratios.clear();
        ws.flips.clear();
    }
    // Split i means different things in different layouts, so only split 0
    // (the master ratio) carries over. Monocle has no splits; its ratios stay.
    if (kind != ws.layout && kind != LayoutKind::Monocle &&
        ws.layout != LayoutKind::Monocle) {
        ws.ratios.resize(std::min<size_t>(ws.ratios.size(), 1));
        ws.flips.clear();  // split orientations do not carry over either
    }
    ws.layout = kind;
}

template <typename Fn>
void ForEachWindow(const Workspace& ws, Fn&& fn) {
    for (HWND h : ws.tiled) fn(h);
    for (HWND h : ws.floating) fn(h);
}

// Shows the windows of `ws` that we hid. Returns how many were shown.
int ShowHiddenIn(const Workspace& ws) {
    int shown = 0;
    ForEachWindow(ws, [&shown](HWND h) {
        if (!g.hiddenByUs.count(h)) return;
        ShowForWorkspace(h);
        ++shown;
    });
    return shown;
}

void HideShownIn(const Workspace& ws) {
    ForEachWindow(ws, [](HWND h) {
        if (!g.hiddenByUs.count(h)) HideForWorkspace(h);
    });
}

void AppendWindows(Workspace& dst, const Workspace& src) {
    dst.tiled.insert(dst.tiled.end(), src.tiled.begin(), src.tiled.end());
    dst.floating.insert(dst.floating.end(), src.floating.begin(),
                        src.floating.end());
}

// Moves a floating window into the work area `to`, keeping its size and its
// offset from the work-area origin `from`, clamped so it stays inside `to`.
void CarryFloatingWindow(HWND hwnd, const RECT& from, const RECT& to) {
    const RECT vis = GetVisibleFrame(hwnd);
    const LONG w = vis.right - vis.left;
    const LONG h = vis.bottom - vis.top;
    const LONG x = std::clamp(to.left + vis.left - from.left, to.left,
                              std::max(to.left, to.right - w));
    const LONG y = std::clamp(to.top + vis.top - from.top, to.top,
                              std::max(to.top, to.bottom - h));
    PlaceWindow(hwnd, RECT{x, y, x + w, y + h});
}

// Moves the membership of `hwnd` (currently at `loc`) to workspace wsIndex of
// monitor mon: out of its tiled or floating list, into the same kind of list of
// the target, and the target's lastFocus. No visibility or layout changes here.
void MoveWindowMembership(HWND hwnd, const WinLoc& loc, int mon, int wsIndex) {
    Workspace& src = g.monitors[loc.mon].workspaces[loc.ws];
    Workspace& dst = g.monitors[mon].workspaces[wsIndex];
    std::vector<HWND>& fromList = loc.floating ? src.floating : src.tiled;
    std::vector<HWND>& toList = loc.floating ? dst.floating : dst.tiled;
    RemoveFrom(fromList, hwnd);
    toList.push_back(hwnd);
    dst.lastFocus = hwnd;
    if (src.lastFocus == hwnd) src.lastFocus = nullptr;
}

// Focuses the workspace's first window. An empty workspace hands focus to the
// desktop, so keystrokes never go to a window we just hid.
void FocusWorkspaceOrDesktop(const Workspace& ws) {
    if (HWND first = FirstWindow(ws)) {
        FocusWindow(first);
    } else if (HWND shell = GetShellWindow()) {
        SetForegroundWindow(shell);
    }
}

void FocusMonitor(int mon) {
    if (mon < 0 || mon >= MonitorCount()) return;
    g.focusedMon = mon;
    if (HWND first = FirstWindow(ActiveWorkspace(mon))) FocusWindow(first);
}

void FocusInDirection(HWND cur, const WinLoc& loc, Direction dir) {
    Workspace& ws = g.monitors[loc.mon].workspaces[loc.ws];
    if (ws.layout == LayoutKind::Monocle && !loc.floating) {
        const std::vector<HWND> shown = Candidates(ws, cur, false);
        const int i = IndexOf(shown, cur);
        const int n = static_cast<int>(shown.size());
        if (i >= 0 && n > 1) {
            const bool back = dir == Direction::Left || dir == Direction::Up;
            FocusWindow(shown[(i + (back ? n - 1 : 1)) % n]);
        }
        return;
    }
    const std::vector<HWND> cands = Candidates(ws, cur, loc.floating);
    const int from = IndexOf(cands, cur);
    if (from >= 0) {
        const int nb =
            FindNeighbor(NavFramesOf(loc.mon, ws, cands), from, dir);
        if (nb >= 0) {
            FocusWindow(cands[nb]);
            return;
        }
    }
    const int m = FindNeighbor(MonitorRects(), loc.mon, dir);
    if (m >= 0) FocusMonitor(m);
}

void MoveInDirection(HWND cur, const WinLoc& loc, Direction dir) {
    Workspace& ws = g.monitors[loc.mon].workspaces[loc.ws];
    if (loc.floating) {
        const int m = FindNeighbor(MonitorRects(), loc.mon, dir);
        if (m >= 0) {
            MoveWindowToMonitor(cur, m);
            FocusWindow(cur);
        }
        return;
    }
    if (ws.layout == LayoutKind::Monocle) {
        const bool back = dir == Direction::Left || dir == Direction::Up;
        const int i = IndexOf(ws.tiled, cur);
        const int j = back ? i - 1 : i + 1;
        if (i >= 0 && j >= 0 && j < static_cast<int>(ws.tiled.size())) {
            std::swap(ws.tiled[i], ws.tiled[j]);
            Retile(loc.mon);
            FocusWindow(cur);
            return;
        }
    } else {
        const std::vector<HWND> cands = Candidates(ws, cur, false);
        const int from = IndexOf(cands, cur);
        const int nb = from < 0 ? -1
                                : FindNeighbor(NavFramesOf(loc.mon, ws, cands),
                                               from, dir);
        if (nb >= 0 && ws.layout == LayoutKind::Mainstage) {
            // The stage follows focus (OnFocusChanged), which is the swap.
            FocusWindow(cands[nb]);
            return;
        }
        if (nb >= 0) {
            const int a = IndexOf(ws.tiled, cur);
            const int b = IndexOf(ws.tiled, cands[nb]);
            std::swap(ws.tiled[a], ws.tiled[b]);
            Retile(loc.mon);
            FocusWindow(cur);
            return;
        }
    }
    const int m = FindNeighbor(MonitorRects(), loc.mon, dir);
    if (m >= 0) {
        MoveWindowToMonitor(cur, m);
        FocusWindow(cur);
    }
}

// A window that just started floating: 60 % of the work area, centred on it.
void PlaceCenteredFloating(HWND hwnd, int mon) {
    const RECT work = g.monitors[mon].rcWork;
    const LONG areaW = work.right - work.left;
    const LONG areaH = work.bottom - work.top;
    const LONG w = areaW * 6 / 10;
    const LONG h = areaH * 6 / 10;
    const LONG x = work.left + (areaW - w) / 2;
    const LONG y = work.top + (areaH - h) / 2;
    PlaceWindow(hwnd, RECT{x, y, x + w, y + h});
}

void ToggleFloating(HWND cur, const WinLoc& loc) {
    Workspace& ws = g.monitors[loc.mon].workspaces[loc.ws];
    if (loc.floating) {
        RemoveFrom(ws.floating, cur);
        ws.tiled.push_back(cur);
        if (g.settings.tileMaximized && IsZoomed(cur)) {
            ShowWindowAsync(cur, SW_RESTORE);
            HT_LOG(L"window %p was maximized; restoring it for tiling", (void*)cur);
        }
    } else {
        RemoveFrom(ws.tiled, cur);
        ws.floating.push_back(cur);
        PlaceCenteredFloating(cur, loc.mon);
    }
    Retile(loc.mon);
    UpdateFocusBorder();
}

// Managed windows of `app` (lowercase exe name) in the tiled or floating lists of
// every workspace on every monitor.
std::vector<HWND> ManagedWindowsOfApp(const std::wstring& app, bool floating) {
    std::vector<HWND> found;
    for (const Monitor& m : g.monitors) {
        for (const Workspace& ws : m.workspaces) {
            for (HWND h : floating ? ws.floating : ws.tiled) {
                WindowFacts facts;
                if (GetWindowFacts(h, &facts) && facts.processName == app) {
                    found.push_back(h);
                }
            }
        }
    }
    return found;
}

// togglealwaysfloat: remember the command target's app as "never tile", or forget
// it. Remembering moves its tiled windows to floating; forgetting moves its
// floating windows back to tiled when ClassifyWindow now says Tile.
void ToggleAlwaysFloatApp(HWND cur, const WinLoc& loc) {
    WindowFacts facts;
    if (!GetWindowFacts(cur, &facts) || facts.processName.empty()) {
        HT_LOG(L"can't tell which app this window belongs to");
        return;
    }
    const std::wstring app = facts.processName;
    if (!g.alwaysFloatApps.contains(app)) {
        g.alwaysFloatApps.insert(app);
        SaveAlwaysFloatApps();
        HT_LOG(L"never tiling %s again", app.c_str());
        // Collected first: moving entries would change the lists being searched.
        for (HWND h : ManagedWindowsOfApp(app, false)) {
            WinLoc hl;
            if (!Locate(h, &hl)) continue;
            Workspace& ws = g.monitors[hl.mon].workspaces[hl.ws];
            RemoveFrom(ws.tiled, h);
            ws.floating.push_back(h);
        }
        // Only a window that just left the layout is centred; one the user had
        // already floated stays where they put it.
        if (!loc.floating) PlaceCenteredFloating(cur, loc.mon);
    } else {
        g.alwaysFloatApps.erase(app);
        SaveAlwaysFloatApps();
        HT_LOG(L"tiling %s again", app.c_str());
        for (HWND h : ManagedWindowsOfApp(app, true)) {
            WinLoc hl;
            if (!Locate(h, &hl) || ClassifyWindow(h, g.settings) != Eligibility::Tile) {
                continue;
            }
            Workspace& ws = g.monitors[hl.mon].workspaces[hl.ws];
            RemoveFrom(ws.floating, h);
            ws.tiled.push_back(h);
            if (g.settings.tileMaximized && IsZoomed(h)) {
                ShowWindowAsync(h, SW_RESTORE);
                HT_LOG(L"window %p was maximized; restoring it for tiling", (void*)h);
            }
        }
    }
    RetileAll();
    UpdateFocusBorder();
    RequestBarUpdate();
}

struct FoundMonitor {
    HMONITOR handle = nullptr;
    RECT rcMonitor{};
    RECT rcWork{};
};

BOOL CALLBACK CollectMonitor(HMONITOR handle, HDC, LPRECT, LPARAM lparam) {
    auto* out = reinterpret_cast<std::vector<FoundMonitor>*>(lparam);
    MONITORINFO info{};
    info.cbSize = sizeof(info);
    if (GetMonitorInfoW(handle, &info)) {
        out->push_back(FoundMonitor{handle, info.rcMonitor, info.rcWork});
    }
    return TRUE;
}

BOOL CALLBACK CollectWindow(HWND hwnd, LPARAM lparam) {
    reinterpret_cast<std::vector<HWND>*>(lparam)->push_back(hwnd);
    return TRUE;
}

// Alt+Shift mouse resize session (see BeginMouseResize in the contract).
struct MouseResizeSession {
    bool active = false;
    HWND hwnd = nullptr;
    int mon = -1;
    int ws = -1;
    bool floating = false;     // floating windows move live, tiled ones snap
    int index = -1;            // tiled only: index in LayoutWindowsOf(ws)
    int count = 0;             // tiled only: size of that list at start
    RECT area{};               // tiled only: layout area at start
    std::vector<double> startRatios;  // tiled only
    RECT startRect{};          // tiled: its slot; floating: visible frame
    RECT lastRect{};           // last rect handed to PlaceWindow
    POINT startCursor{};
    bool leftEdge = false;     // true: left edge moves, false: right edge
    bool topEdge = false;      // true: top edge moves, false: bottom edge
    int lastDx = 0;            // last cursor delta handled
    int lastDy = 0;
};

MouseResizeSession gResize;

// True while the session's window is still where it was recorded: same
// monitor, workspace and tiled/floating state, workspace active, not minimized
// or maximized. Tiled: also not monocle and same index and count in
// LayoutWindowsOf(ws).
bool MouseResizeStillValid() {
    const MouseResizeSession& s = gResize;
    if (!s.active || s.mon < 0 || s.mon >= MonitorCount()) return false;
    if (IsIconic(s.hwnd) || IsZoomed(s.hwnd)) return false;
    WinLoc loc;
    if (!Locate(s.hwnd, &loc) || loc.mon != s.mon || loc.ws != s.ws ||
        loc.floating != s.floating) {
        return false;
    }
    const Monitor& m = g.monitors[s.mon];
    if (m.active != s.ws) return false;
    if (s.floating) return true;
    const Workspace& ws = m.workspaces[s.ws];
    if (ws.layout == LayoutKind::Monocle) return false;
    const std::vector<HWND> list = LayoutWindowsOf(ws);
    return static_cast<int>(list.size()) == s.count &&
           IndexOf(list, s.hwnd) == s.index;
}

}  // namespace

Workspace& ActiveWorkspace(int mon) {
    static Workspace fallback;  // only for invalid indices; callers check first
    if (mon < 0 || mon >= MonitorCount()) {
        fallback = Workspace{};
        return fallback;
    }
    Monitor& m = g.monitors[mon];
    if (m.workspaces.empty()) m.workspaces.push_back(MakeWorkspace(0));
    const int last = static_cast<int>(m.workspaces.size()) - 1;
    m.active = std::clamp(m.active, 0, last);
    return m.workspaces[m.active];
}

bool Locate(HWND hwnd, WinLoc* loc) {
    if (hwnd) {
        for (int mi = 0; mi < MonitorCount(); ++mi) {
            const std::vector<Workspace>& wss = g.monitors[mi].workspaces;
            for (int wi = 0; wi < static_cast<int>(wss.size()); ++wi) {
                const int t = IndexOf(wss[wi].tiled, hwnd);
                if (t >= 0) {
                    if (loc) *loc = WinLoc{mi, wi, false, t};
                    return true;
                }
                const int f = IndexOf(wss[wi].floating, hwnd);
                if (f >= 0) {
                    if (loc) *loc = WinLoc{mi, wi, true, f};
                    return true;
                }
            }
        }
    }
    if (loc) *loc = WinLoc{};
    return false;
}

int MonitorIndexFromWindow(HWND hwnd) {
    if (MonitorCount() == 0) return -1;
    const HMONITOR hm = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
    const int byHandle = MonitorIndexOf(hm);
    if (byHandle >= 0) return byHandle;
    const RECT vis = GetVisibleFrame(hwnd);
    const POINT center{(vis.left + vis.right) / 2, (vis.top + vis.bottom) / 2};
    for (int i = 0; i < MonitorCount(); ++i) {
        if (PtInRect(&g.monitors[i].rcMonitor, center)) return i;
    }
    return 0;
}

bool ManageWindow(HWND hwnd, bool floating) {
    if (!hwnd || Locate(hwnd, nullptr)) return false;
    const int mon = MonitorIndexFromWindow(hwnd);
    if (mon < 0) return false;
    Workspace& ws = ActiveWorkspace(mon);
    std::vector<HWND>& list = floating ? ws.floating : ws.tiled;
    list.push_back(hwnd);
    if (IsZoomed(hwnd)) g.zoomed.insert(hwnd);
    if (!floating && g.settings.tileMaximized && IsZoomed(hwnd)) {
        // Stays in g.zoomed; the restore's LOCATIONCHANGE retiles it.
        ShowWindowAsync(hwnd, SW_RESTORE);
        HT_LOG(L"window %p was maximized; restoring it for tiling", (void*)hwnd);
    }
    HT_LOG(L"managed %p floating=%d", (void*)hwnd, floating ? 1 : 0);
    RequestBarUpdate();
    return true;
}

void UnmanageWindow(HWND hwnd) {
    if (!hwnd) return;
    for (Monitor& m : g.monitors) {
        for (Workspace& ws : m.workspaces) {
            RemoveFrom(ws.tiled, hwnd);
            RemoveFrom(ws.floating, hwnd);
            if (ws.lastFocus == hwnd) ws.lastFocus = nullptr;
        }
    }
    if (g.hiddenByUs.count(hwnd)) {
        // A live window we stop managing must not stay invisible.
        if (IsWindow(hwnd)) {
            ShowForWorkspace(hwnd);  // also removes it from hiddenByUs
        } else {
            g.hiddenByUs.erase(hwnd);
            PersistHiddenList();
        }
    }
    g.zoomed.erase(hwnd);
    if (gResize.active && gResize.hwnd == hwnd) gResize = MouseResizeSession{};
    g.pendingVisibility.erase(hwnd);
    g.attention.erase(hwnd);
    if (g.borderedHwnd == hwnd) {
        if (IsWindow(hwnd)) ClearFocusBorder(hwnd);
        g.borderedHwnd = nullptr;
    }
    RequestBarUpdate();
}

void RefreshMonitors() {
    std::vector<FoundMonitor> found;
    EnumDisplayMonitors(nullptr, nullptr, CollectMonitor,
                        reinterpret_cast<LPARAM>(&found));
    if (found.empty()) {
        HT_LOG(L"monitor enumeration returned nothing; keeping old list");
        return;
    }
    std::sort(found.begin(), found.end(),
              [](const FoundMonitor& a, const FoundMonitor& b) {
                  if (a.rcMonitor.left != b.rcMonitor.left) {
                      return a.rcMonitor.left < b.rcMonitor.left;
                  }
                  return a.rcMonitor.top < b.rcMonitor.top;
              });

    // Match by handle first, then by geometry (handles change on re-plug).
    std::vector<Monitor> old;
    old.swap(g.monitors);
    std::vector<bool> used(old.size(), false);
    std::vector<int> oldFor(found.size(), -1);
    for (size_t f = 0; f < found.size(); ++f) {
        for (size_t o = 0; o < old.size() && oldFor[f] < 0; ++o) {
            if (!used[o] && old[o].handle == found[f].handle) {
                oldFor[f] = static_cast<int>(o);
                used[o] = true;
            }
        }
    }
    for (size_t f = 0; f < found.size(); ++f) {
        for (size_t o = 0; o < old.size() && oldFor[f] < 0; ++o) {
            if (!used[o] &&
                EqualRect(&old[o].rcMonitor, &found[f].rcMonitor)) {
                oldFor[f] = static_cast<int>(o);
                used[o] = true;
            }
        }
    }

    for (size_t f = 0; f < found.size(); ++f) {
        Monitor m;
        if (oldFor[f] >= 0) {
            m = std::move(old[oldFor[f]]);
        } else {
            for (int wi = 0; wi < WsCount(); ++wi) {
                m.workspaces.push_back(MakeWorkspace(wi));
            }
        }
        m.handle = found[f].handle;
        m.rcMonitor = found[f].rcMonitor;
        m.rcWork = found[f].rcWork;
        if (m.workspaces.empty()) m.workspaces.push_back(MakeWorkspace(0));
        const int last = static_cast<int>(m.workspaces.size()) - 1;
        m.active = std::clamp(m.active, 0, last);
        g.monitors.push_back(std::move(m));
    }

    // Windows of vanished monitors join the active workspace of monitor 0.
    Workspace& dst = ActiveWorkspace(0);
    for (size_t o = 0; o < old.size(); ++o) {
        if (used[o]) continue;
        HT_LOG(L"monitor vanished; moving its windows to monitor 0");
        for (Workspace& ws : old[o].workspaces) {
            ShowHiddenIn(ws);
            AppendWindows(dst, ws);
        }
    }

    g.focusedMon = std::clamp(g.focusedMon, 0, MonitorCount() - 1);
    HT_LOG(L"%d monitor(s)", MonitorCount());
    for (int i = 0; i < MonitorCount(); ++i) {
        const RECT& rc = g.monitors[i].rcMonitor;
        HT_LOG(L"  monitor %d: %ld,%ld-%ld,%ld", i, rc.left, rc.top, rc.right,
               rc.bottom);
    }
    RequestBarUpdate();
}

void ApplySettingsChange() {
    const int count = WsCount();
    for (int mi = 0; mi < MonitorCount(); ++mi) {
        Monitor& m = g.monitors[mi];
        while (static_cast<int>(m.workspaces.size()) < count) {
            m.workspaces.push_back(
                MakeWorkspace(static_cast<int>(m.workspaces.size())));
        }
        if (static_cast<int>(m.workspaces.size()) > count) {
            Workspace& last = m.workspaces[count - 1];
            for (size_t wi = static_cast<size_t>(count);
                 wi < m.workspaces.size(); ++wi) {
                const Workspace& gone = m.workspaces[wi];
                AppendWindows(last, gone);
                if (gone.lastFocus && !InWorkspace(last, last.lastFocus)) {
                    last.lastFocus = gone.lastFocus;
                }
            }
            m.workspaces.resize(static_cast<size_t>(count));
        }
        m.active = std::clamp(m.active, 0, count - 1);
        // Only a workspace whose own entry changed switches layout, so a layout
        // picked with the keyboard survives unrelated settings changes.
        for (int wi = 0; wi < count; ++wi) {
            const LayoutKind now =
                ConfiguredLayout(g.settings.workspaceLayouts, wi);
            if (now != LayoutKind::Count_ &&
                now != ConfiguredLayout(g.appliedWorkspaceLayouts, wi)) {
                SetWorkspaceLayout(m.workspaces[wi], now);
            }
        }
        // Covers a removed active workspace and windows moved into it.
        ShowHiddenIn(m.workspaces[m.active]);
    }
    g.appliedWorkspaceLayouts = g.settings.workspaceLayouts;
    RetileAll();
    UpdateFocusBorder();
    RequestBarUpdate();
}

namespace {

// Retile without the side-tile sync, so RetileAll syncs once.
void RetileWindows(int mon) {
    if (g.paused || mon < 0 || mon >= MonitorCount()) return;
    Workspace& ws = ActiveWorkspace(mon);
    const bool stacked = ws.layout == LayoutKind::Mainstage;
    if (stacked) {
        StageFollowsZOrder(ws);
        AssignStageSides(ws);
    }
    const std::vector<HWND> visible = LayoutWindowsOf(ws);
    if (visible.empty()) return;

    const std::vector<RECT> rects =
        ComputeLayout(LayoutAreaOf(mon), static_cast<int>(visible.size()),
                      LayoutParamsOf(ws));
    std::vector<HWND> failed;
    for (size_t i = 0; i < visible.size() && i < rects.size(); ++i) {
        // Mainstage windows all sit on the stage; rects 1.. are their tiles.
        RECT target = rects[stacked ? 0 : i];
        // A side window reaches out from under the stage window by a sliver:
        // apps that stop drawing while fully covered (Chromium, Electron)
        // would otherwise freeze their tile's picture.
        if (stacked && i > 0) target.left -= kStageSliver;
        if (!PlaceWindow(visible[i], target)) failed.push_back(visible[i]);
    }
    for (HWND h : failed) {
        RemoveFrom(ws.tiled, h);
        ws.floating.push_back(h);
        HT_LOG(L"window %p cannot be tiled; now floating", (void*)h);
    }
    if (!failed.empty()) RequestRetile();
}

}  // namespace

void Retile(int mon) {
    RetileWindows(mon);
    SyncStageProxies();
}

void RetileAll() {
    for (int i = 0; i < MonitorCount(); ++i) RetileWindows(i);
    SyncStageProxies();
}

std::vector<StageTile> StageSideTiles() {
    std::vector<StageTile> tiles;
    if (g.paused) return tiles;
    for (int mi = 0; mi < MonitorCount(); ++mi) {
        const Workspace& ws = ActiveWorkspace(mi);
        if (ws.layout != LayoutKind::Mainstage) continue;
        const std::vector<HWND> list = LayoutWindowsOf(ws);
        const std::vector<RECT> rects = ComputeLayout(
            LayoutAreaOf(mi), static_cast<int>(list.size()), LayoutParamsOf(ws));
        for (size_t i = 1; i < list.size() && i < rects.size(); ++i) {
            tiles.push_back(StageTile{list[i], rects[i]});
        }
    }
    return tiles;
}

void StageDropWindow(HWND hwnd, POINT pt) {
    WinLoc loc;
    if (!Locate(hwnd, &loc) || loc.floating) return;
    ActiveWorkspace(loc.mon);  // also normalises m.active
    Monitor& m = g.monitors[loc.mon];
    if (loc.ws != m.active) return;
    Workspace& ws = m.workspaces[loc.ws];
    if (ws.layout != LayoutKind::Mainstage) return;

    const std::vector<HWND> list = LayoutWindowsOf(ws);
    const int count = static_cast<int>(list.size());
    const int from = IndexOf(list, hwnd);
    if (from < 0) return;
    const std::vector<RECT> slots =
        ComputeLayout(LayoutAreaOf(loc.mon), count, LayoutParamsOf(ws));
    int over = -1;
    for (int i = 0; i < count && i < static_cast<int>(slots.size()); ++i) {
        if (PtInRect(&slots[i], pt)) {
            over = i;
            break;
        }
    }

    if (over == from) {
        // Dropped where it came from.
    } else if (over == 0) {
        FocusWindow(hwnd);  // a side window dropped on the stage
    } else if (over > 0 && from == 0) {
        FocusWindow(list[over]);  // the stage window dropped on a tile
    } else if (over > 0) {
        const HWND other = list[over];
        std::swap(ws.tiled[IndexOf(ws.tiled, hwnd)],
                  ws.tiled[IndexOf(ws.tiled, other)]);
        std::swap(ws.stageSide[hwnd], ws.stageSide[other]);
    } else if (count >= 2 && PtInRect(&m.rcWork, pt) &&
               (pt.x < slots[0].left || pt.x >= slots[0].right)) {
        // Free space beside the stage: last tile of that side.
        ws.stageSide[hwnd] = pt.x < slots[0].left ? 0 : 1;
        RemoveFrom(ws.tiled, hwnd);
        ws.tiled.push_back(hwnd);
        if (from == 0) FocusWindow(list[1]);  // the next window takes the stage
    }
    Retile(loc.mon);
}

void RequestRetile() {
    if (g.engineHwnd) SetTimer(g.engineHwnd, kTimerRetile, 15, nullptr);
}

void SwitchWorkspace(int mon, int wsIndex) {
    if (mon < 0 || mon >= MonitorCount()) return;
    Monitor& m = g.monitors[mon];
    if (wsIndex < 0 || wsIndex >= static_cast<int>(m.workspaces.size())) {
        return;
    }
    Workspace& prev = ActiveWorkspace(mon);  // also normalises m.active
    if (wsIndex == m.active) {
        g.focusedMon = mon;
        if (g.settings.workspaceOsd) ShowWorkspaceOsd(mon);
        RequestBarUpdate();
        return;
    }
    Workspace& next = m.workspaces[wsIndex];
    ShowHiddenIn(next);
    HideShownIn(prev);
    m.active = wsIndex;
    g.focusedMon = mon;
    Retile(mon);
    FocusWorkspaceOrDesktop(next);
    UpdateFocusBorder();
    HT_LOG(L"workspace %d on monitor %d", wsIndex + 1, mon);
    if (g.settings.workspaceOsd) ShowWorkspaceOsd(mon);
    RequestBarUpdate();
}

void MoveWindowToWorkspace(HWND hwnd, int mon, int wsIndex) {
    WinLoc loc;
    if (!Locate(hwnd, &loc)) return;
    if (mon < 0 || mon >= MonitorCount()) return;
    Monitor& dstMon = g.monitors[mon];
    if (wsIndex < 0 || wsIndex >= static_cast<int>(dstMon.workspaces.size())) {
        return;
    }
    if (loc.mon == mon && loc.ws == wsIndex) return;

    Monitor& srcMon = g.monitors[loc.mon];
    Workspace& src = srcMon.workspaces[loc.ws];
    MoveWindowMembership(hwnd, loc, mon, wsIndex);

    const bool targetActive = wsIndex == dstMon.active;
    const bool sourceActive = loc.ws == srcMon.active;
    if (!targetActive) {
        HideForWorkspace(hwnd);
    } else {
        if (g.hiddenByUs.count(hwnd)) ShowForWorkspace(hwnd);
        if (loc.floating && loc.mon != mon) {
            CarryFloatingWindow(hwnd, srcMon.rcWork, dstMon.rcWork);
        }
    }

    Retile(loc.mon);
    if (mon != loc.mon) Retile(mon);
    if (!targetActive && sourceActive) FocusWorkspaceOrDesktop(src);
    UpdateFocusBorder();
    RequestBarUpdate();
}

void MoveWindowToWorkspaceAndFollow(HWND hwnd, int wsIndex) {
    WinLoc loc;
    if (!Locate(hwnd, &loc)) return;
    const int mon = loc.mon;
    Monitor& m = g.monitors[mon];
    if (wsIndex < 0 || wsIndex >= static_cast<int>(m.workspaces.size())) {
        return;
    }
    if (loc.ws == wsIndex) {
        if (g.settings.workspaceOsd) ShowWorkspaceOsd(mon);
        return;
    }

    ActiveWorkspace(mon);  // also normalises m.active
    if (loc.ws != m.active) {
        // Unusual: the window sits on a workspace that is not shown. Move it
        // silently, then switch, which shows it and focuses it.
        MoveWindowToWorkspace(hwnd, mon, wsIndex);
        SwitchWorkspace(mon, wsIndex);
        return;
    }

    // The window stays visible: SwitchWorkspace hides what is left behind and
    // focuses this window, because the target's lastFocus is now hwnd.
    MoveWindowMembership(hwnd, loc, mon, wsIndex);
    HT_LOG(L"window %p moved to workspace %d and followed", (void*)hwnd,
           wsIndex + 1);
    SwitchWorkspace(mon, wsIndex);
}

void MoveWindowToMonitor(HWND hwnd, int mon) {
    if (mon < 0 || mon >= MonitorCount()) return;
    MoveWindowToWorkspace(hwnd, mon, g.monitors[mon].active);
}

HWND CommandTarget() {
    if (MonitorCount() == 0) return nullptr;
    const HWND fg = GetForegroundWindow();
    if (Locate(fg, nullptr)) return fg;
    const HWND root = fg ? GetAncestor(fg, GA_ROOTOWNER) : nullptr;
    if (Locate(root, nullptr)) return root;
    if (g.focusedMon < 0 || g.focusedMon >= MonitorCount()) return nullptr;
    const HWND last = ActiveWorkspace(g.focusedMon).lastFocus;
    return Locate(last, nullptr) ? last : nullptr;
}

void OnFocusChanged(HWND hwnd) {
    WinLoc loc;
    HWND managed = nullptr;
    if (Locate(hwnd, &loc)) {
        managed = hwnd;
    } else {
        const HWND root = hwnd ? GetAncestor(hwnd, GA_ROOTOWNER) : nullptr;
        if (Locate(root, &loc)) managed = root;
    }
    if (managed) {
        g.focusedMon = loc.mon;
        g.monitors[loc.mon].workspaces[loc.ws].lastFocus = managed;
        if (g.attention.erase(managed) > 0) RequestBarUpdate();
        // The focused window takes the stage (Retile reads the z-order).
        if (!loc.floating && loc.ws == g.monitors[loc.mon].active &&
            g.monitors[loc.mon].workspaces[loc.ws].layout ==
                LayoutKind::Mainstage) {
            Retile(loc.mon);
        }
    }
    UpdateFocusBorder();
}

void ActivateWindowWorkspace(HWND hwnd) {
    WinLoc loc;
    if (!Locate(hwnd, &loc)) return;
    ActiveWorkspace(loc.mon);  // also normalises m.active
    g.monitors[loc.mon].workspaces[loc.ws].lastFocus = hwnd;
    if (loc.ws != g.monitors[loc.mon].active) {
        HT_LOG(L"following activation of %p to workspace %d",
               static_cast<void*>(hwnd), loc.ws + 1);
        SwitchWorkspace(loc.mon, loc.ws);
    }
    FocusWindow(hwnd);
}

void MarkAttention(HWND hwnd) {
    HWND managed = nullptr;
    if (Locate(hwnd, nullptr)) {
        managed = hwnd;
    } else {
        const HWND root = hwnd ? GetAncestor(hwnd, GA_ROOTOWNER) : nullptr;
        if (Locate(root, nullptr)) managed = root;
    }
    if (!managed) return;
    // A window the user is already looking at does not need a dot.
    const HWND fg = GetForegroundWindow();
    const HWND fgRoot = fg ? GetAncestor(fg, GA_ROOTOWNER) : nullptr;
    if (fg == managed || fgRoot == managed) return;
    if (g.attention.insert(managed).second) {
        HT_LOG(L"attention %p", static_cast<void*>(managed));
        RequestBarUpdate();
    }
}

bool WorkspaceHasAttention(int mon, int wsIndex) {
    if (!g.settings.attentionDots || g.attention.empty()) return false;
    if (mon < 0 || mon >= MonitorCount()) return false;
    const std::vector<Workspace>& wss = g.monitors[mon].workspaces;
    if (wsIndex < 0 || wsIndex >= static_cast<int>(wss.size())) return false;
    bool found = false;
    ForEachWindow(wss[wsIndex], [&found](HWND h) {
        if (g.attention.count(h) > 0) found = true;
    });
    return found;
}

void UpdateFocusBorder() {
    const HWND target = g.settings.focusBorder ? CommandTarget() : nullptr;
    if (target && target == g.borderedHwnd) {
        SetFocusBorder(target, g.settings.focusBorderColor);
        return;
    }
    if (g.borderedHwnd && IsWindow(g.borderedHwnd)) {
        ClearFocusBorder(g.borderedHwnd);
    }
    g.borderedHwnd = nullptr;
    if (target) {
        SetFocusBorder(target, g.settings.focusBorderColor);
        g.borderedHwnd = target;
    }
}

void OnUserDragEnd(HWND hwnd) {
    WinLoc loc;
    POINT dropPoint{};
    GetCursorPos(&dropPoint);
    int dropMon = -1;
    int dropWs = -1;
    if (Locate(hwnd, &loc) && OverviewDropTarget(dropPoint, &dropMon, &dropWs)) {
        // Dropped on a workspace of the overview (Alt held while dragging).
        MoveWindowToWorkspace(hwnd, dropMon, dropWs);
        OverviewDropDone();
    } else if (Locate(hwnd, &loc)) {
        const RECT frame = GetVisibleFrame(hwnd);
        const POINT center{(frame.left + frame.right) / 2,
                           (frame.top + frame.bottom) / 2};
        const HMONITOR hm = MonitorFromPoint(center, MONITOR_DEFAULTTONULL);
        const int target = MonitorIndexOf(hm);
        if (target >= 0 && target != loc.mon && loc.floating) {
            // The user already put it where it belongs; only membership
            // changes, so it hides and shows with the new monitor's workspace.
            Workspace& src = g.monitors[loc.mon].workspaces[loc.ws];
            RemoveFrom(src.floating, hwnd);
            if (src.lastFocus == hwnd) src.lastFocus = nullptr;
            Workspace& dst = ActiveWorkspace(target);
            dst.floating.push_back(hwnd);
            dst.lastFocus = hwnd;
            g.focusedMon = target;
            RequestBarUpdate();
        } else if (target >= 0 && target != loc.mon) {
            MoveWindowToMonitor(hwnd, target);
            RequestBarUpdate();
        } else if (!loc.floating) {
            Workspace& ws = g.monitors[loc.mon].workspaces[loc.ws];
            const std::vector<HWND> list = LayoutWindowsOf(ws);
            const int idx = IndexOf(list, hwnd);
            // Monocle windows share one rect, so there is no slot to compare.
            if (ws.layout != LayoutKind::Monocle && idx >= 0) {
                const int count = static_cast<int>(list.size());
                const RECT area = LayoutAreaOf(loc.mon);
                const LayoutParams params = LayoutParamsOf(ws);
                const std::vector<RECT> slots =
                    ComputeLayout(area, count, params);
                // Compare with the size at drag start: windows with a minimum
                // size never match their slot, so a plain move would otherwise
                // look like a resize.
                const RECT& before =
                    g.dragHwnd == hwnd ? g.dragStartFrame : slots[idx];
                const LONG dw =
                    (frame.right - frame.left) - (before.right - before.left);
                const LONG dh =
                    (frame.bottom - frame.top) - (before.bottom - before.top);
                if (dw > 2 || dw < -2 || dh > 2 || dh < -2) {
                    // Resized: the neighbours follow the new edges.
                    LayoutParams after = params;
                    if (ApplyResize(area, count, after, idx, frame)) {
                        for (int s = 0; s + 1 < count; ++s) {
                            const double was = SplitRatio(params, count, s);
                            const double now = SplitRatio(after, count, s);
                            if (was != now) {
                                HT_LOG(L"split %d ratio %.3f -> %.3f", s, was,
                                       now);
                            }
                        }
                        ws.ratios = after.ratios;
                        HT_LOG(L"resized window %p", (void*)hwnd);
                    }
                } else if (ws.layout == LayoutKind::Mainstage) {
                    // Moved: the mouse decides (a tile, a side, or back).
                    POINT cursor{};
                    GetCursorPos(&cursor);
                    StageDropWindow(hwnd, cursor);
                } else {
                    // Moved: swap with the tiled window under the drop point.
                    const int self = IndexOf(ws.tiled, hwnd);
                    const int n = static_cast<int>(ws.tiled.size());
                    for (int j = 0; j < n; ++j) {
                        if (j == self) continue;
                        const RECT other = GetVisibleFrame(ws.tiled[j]);
                        if (PtInRect(&other, center)) {
                            std::swap(ws.tiled[self], ws.tiled[j]);
                            break;
                        }
                    }
                }
            }
        }
    }
    RequestRetile();
}

bool BeginMouseResize(POINT cursor) {
    EndMouseResize();
    const HWND hwnd = CommandTarget();
    WinLoc loc;
    if (!Locate(hwnd, &loc)) return false;
    if (IsIconic(hwnd) || IsZoomed(hwnd)) return false;
    ActiveWorkspace(loc.mon);  // also normalises m.active
    const Monitor& m = g.monitors[loc.mon];
    if (m.active != loc.ws) return false;
    const Workspace& ws = m.workspaces[loc.ws];

    MouseResizeSession s;
    s.active = true;
    s.hwnd = hwnd;
    s.mon = loc.mon;
    s.ws = loc.ws;
    s.floating = loc.floating;
    if (loc.floating) {
        s.startRect = GetVisibleFrame(hwnd);
    } else {
        if (ws.layout == LayoutKind::Monocle) return false;
        const std::vector<HWND> list = LayoutWindowsOf(ws);
        const int count = static_cast<int>(list.size());
        const int index = IndexOf(list, hwnd);
        if (index < 0 || count < 2) return false;
        s.area = LayoutAreaOf(loc.mon);
        const std::vector<RECT> slots =
            ComputeLayout(s.area, count, LayoutParamsOf(ws));
        if (index >= static_cast<int>(slots.size())) return false;
        s.index = index;
        s.count = count;
        s.startRatios = ws.ratios;
        s.startRect = slots[index];
    }
    s.lastRect = s.startRect;
    s.startCursor = cursor;
    // Edges by where the cursor was relative to the window's center.
    const RECT& r = s.startRect;
    s.leftEdge = cursor.x < (r.left + r.right) / 2;
    s.topEdge = cursor.y < (r.top + r.bottom) / 2;
    gResize = s;
    return true;
}

void UpdateMouseResize(POINT cursor) {
    if (!gResize.active) return;
    if (!MouseResizeStillValid()) {
        // Moved, minimized or unmanaged since the start: stop without applying.
        gResize = MouseResizeSession{};
        return;
    }
    MouseResizeSession& s = gResize;
    const int dx = static_cast<int>(cursor.x - s.startCursor.x);
    const int dy = static_cast<int>(cursor.y - s.startCursor.y);
    if (dx == s.lastDx && dy == s.lastDy) return;
    s.lastDx = dx;
    s.lastDy = dy;

    // Each moving edge stays at least this far from its opposite edge.
    constexpr LONG kMinEdgeDistance = 50;
    const RECT work = g.monitors[s.mon].rcWork;
    RECT target = s.startRect;
    if (s.leftEdge) {
        target.left += dx;
        if (s.floating) target.left = std::max(target.left, work.left);
        target.left = std::min(target.left, target.right - kMinEdgeDistance);
    } else {
        target.right += dx;
        if (s.floating) target.right = std::min(target.right, work.right);
        target.right = std::max(target.right, target.left + kMinEdgeDistance);
    }
    if (s.topEdge) {
        target.top += dy;
        if (s.floating) target.top = std::max(target.top, work.top);
        target.top = std::min(target.top, target.bottom - kMinEdgeDistance);
    } else {
        target.bottom += dy;
        if (s.floating) target.bottom = std::min(target.bottom, work.bottom);
        target.bottom = std::max(target.bottom, target.top + kMinEdgeDistance);
    }

    // Only the active window moves while the keys are held; a tiled window
    // gets its layout once, in EndMouseResize.
    PlaceWindow(s.hwnd, target);
    s.lastRect = target;
}

void EndMouseResize() {
    if (!gResize.active) return;
    const MouseResizeSession s = gResize;
    const bool valid = MouseResizeStillValid();
    gResize = MouseResizeSession{};
    bool changed = false;
    if (valid && !EqualRect(&s.lastRect, &s.startRect)) {
        if (s.floating) {
            changed = true;
        } else {
            // Neighbours make room for the new edges; ratios from the start.
            Workspace& ws = g.monitors[s.mon].workspaces[s.ws];
            LayoutParams params = LayoutParamsOf(ws);
            params.ratios = s.startRatios;
            if (ApplyResize(s.area, s.count, params, s.index, s.lastRect)) {
                ws.ratios = params.ratios;
                changed = true;
            }
        }
    }
    if (!s.floating) Retile(s.mon);
    if (changed) {
        HT_LOG(L"mouse-resized window %p", static_cast<void*>(s.hwnd));
    }
}

bool MouseResizeActive() {
    return gResize.active;
}

void ExecuteCommand(const Command& cmd) {
    if (g.paused && cmd.kind != CommandKind::ToggleTiling &&
        cmd.kind != CommandKind::Rescan) {
        return;
    }
    if (cmd.kind == CommandKind::Rescan) {
        RefreshMonitors();
        AdoptExistingWindows();
        RetileAll();
        UpdateFocusBorder();
        return;
    }
    if (cmd.kind == CommandKind::ToggleTiling) {
        g.paused = !g.paused;
        HT_LOG(L"%s", g.paused ? L"paused" : L"resumed");
        // While paused this only takes the Mainstage side tiles away.
        RetileAll();
        return;
    }
    if (MonitorCount() == 0 || cmd.kind == CommandKind::None) return;

    const HWND cur = CommandTarget();
    WinLoc loc;
    const bool has = Locate(cur, &loc);
    const int fm = std::clamp(g.focusedMon, 0, MonitorCount() - 1);
    const int mon = has ? loc.mon : fm;
    Workspace& ws =
        has ? g.monitors[loc.mon].workspaces[loc.ws] : ActiveWorkspace(mon);
    const int wsCount = WsCount();

    switch (cmd.kind) {
        case CommandKind::MoveFocus:
            if (has) {
                FocusInDirection(cur, loc, cmd.dir);
            } else {
                FocusMonitor(fm);
            }
            break;
        case CommandKind::SwapWindow:
            if (has) MoveInDirection(cur, loc, cmd.dir);
            break;
        case CommandKind::Workspace:
            if (cmd.arg >= 1 && cmd.arg <= wsCount) {
                SwitchWorkspace(fm, cmd.arg - 1);
            }
            break;
        case CommandKind::MoveToWorkspace:
            if (has && cmd.arg >= 1 && cmd.arg <= wsCount) {
                MoveWindowToWorkspaceAndFollow(cur, cmd.arg - 1);
            }
            break;
        case CommandKind::MoveToWorkspaceSilent:
            if (has && cmd.arg >= 1 && cmd.arg <= wsCount) {
                MoveWindowToWorkspace(cur, loc.mon, cmd.arg - 1);
                if (g.settings.workspaceOsd) ShowWorkspaceOsd(loc.mon);
            }
            break;
        case CommandKind::FocusMonitorNext:
            if (MonitorCount() > 1) {
                const int next = (fm + 1) % MonitorCount();
                FocusMonitor(next);
                if (g.settings.workspaceOsd) ShowWorkspaceOsd(next);
            }
            break;
        case CommandKind::MoveToMonitorNext:
            if (has && MonitorCount() > 1) {
                MoveWindowToMonitor(cur, (loc.mon + 1) % MonitorCount());
                FocusWindow(cur);
            }
            break;
        case CommandKind::NextLayout:
            SetWorkspaceLayout(ws, NextLayout(ws.layout));
            Retile(mon);
            HT_LOG(L"layout %s on monitor %d", LayoutName(ws.layout), mon);
            break;
        case CommandKind::SetLayout:
            SetWorkspaceLayout(ws, cmd.layout);
            Retile(mon);
            HT_LOG(L"layout %s on monitor %d", LayoutName(ws.layout), mon);
            break;
        case CommandKind::ToggleFloating:
            if (has) ToggleFloating(cur, loc);
            break;
        case CommandKind::ToggleAlwaysFloat:
            if (has) ToggleAlwaysFloatApp(cur, loc);
            break;
        case CommandKind::ToggleMonocle:
            if (ws.layout == LayoutKind::Monocle) {
                ws.layout = ws.prevLayout == LayoutKind::Monocle
                                ? g.settings.defaultLayout
                                : ws.prevLayout;
            } else {
                ws.prevLayout = ws.layout;
                ws.layout = LayoutKind::Monocle;
            }
            Retile(mon);
            if (has) FocusWindow(cur);
            break;
        case CommandKind::SwapWithMaster:
            if (has && !loc.floating && ws.layout == LayoutKind::Mainstage) {
                // The stage follows focus: focusing the first side window swaps.
                const std::vector<HWND> list = LayoutWindowsOf(ws);
                if (list.size() > 1) FocusWindow(list[cur == list[0] ? 1 : 0]);
            } else if (has && !loc.floating && ws.tiled.size() > 1) {
                const int i = IndexOf(ws.tiled, cur);
                if (i == 0) {
                    std::swap(ws.tiled[0], ws.tiled[1]);
                } else {
                    ws.tiled.erase(ws.tiled.begin() + i);
                    ws.tiled.insert(ws.tiled.begin(), cur);
                }
                Retile(mon);
                FocusWindow(cur);
            }
            break;
        case CommandKind::GrowMaster:
        case CommandKind::ShrinkMaster: {
            // Moves split 0 as it is drawn now by one step. Two windows is the
            // smallest count that has a split.
            const int n =
                std::max(2, static_cast<int>(LayoutWindowsOf(ws).size()));
            const double cur = SplitRatio(LayoutParamsOf(ws), n, 0);
            const double step = g.settings.ratioStep;
            const double delta =
                cmd.kind == CommandKind::GrowMaster ? step : -step;
            if (ws.ratios.empty()) ws.ratios.resize(1, 0.0);
            ws.ratios[0] = std::clamp(cur + delta, 0.1, 0.9);
            Retile(mon);
            break;
        }
        case CommandKind::ToggleSplit:
            if (has && !loc.floating && ws.layout != LayoutKind::Monocle) {
                const std::vector<HWND> list = LayoutWindowsOf(ws);
                const int idx = IndexOf(list, cur);
                LayoutParams params = LayoutParamsOf(ws);
                if (ToggleSplitAt(params, static_cast<int>(list.size()), idx)) {
                    ws.flips = params.flips;
                    Retile(mon);
                    HT_LOG(L"toggled split of window %p on monitor %d",
                           (void*)cur, mon);
                }
            }
            break;
        case CommandKind::ResizeActive:
            if (ws.layout == LayoutKind::Mainstage) {
                // One size to change: right and up widen the stage, left and
                // down narrow it.
                const bool widen =
                    cmd.dir == Direction::Right || cmd.dir == Direction::Up;
                const double now = SplitRatio(LayoutParamsOf(ws), 1, 0);
                const double step = g.settings.ratioStep;
                if (ws.ratios.empty()) ws.ratios.resize(1, 0.0);
                ws.ratios[0] = std::clamp(now + (widen ? step : -step), 0.2, 0.9);
                Retile(mon);
            } else if (has && !loc.floating && ws.layout != LayoutKind::Monocle) {
                const std::vector<HWND> list = LayoutWindowsOf(ws);
                const int idx = IndexOf(list, cur);
                const int n = static_cast<int>(list.size());
                if (idx >= 0 && n >= 2) {
                    const RECT area = LayoutAreaOf(mon);
                    const LayoutParams params = LayoutParamsOf(ws);
                    const std::vector<RECT> slots = ComputeLayout(area, n, params);
                    const bool horizontal =
                        cmd.dir == Direction::Left || cmd.dir == Direction::Right;
                    const LONG extent = horizontal ? area.right - area.left
                                                   : area.bottom - area.top;
                    const LONG step =
                        std::max<LONG>(1, std::lround(g.settings.ratioStep * extent));
                    // Grow the edge on the dir side. Where there is no split on
                    // that side (the edge is the screen border), shrink the
                    // opposite edge instead, so the boundary still moves toward dir.
                    RECT grown = slots[idx];
                    RECT shrunk = slots[idx];
                    if (cmd.dir == Direction::Left) {
                        grown.left -= step;
                        shrunk.right -= step;
                    } else if (cmd.dir == Direction::Right) {
                        grown.right += step;
                        shrunk.left += step;
                    } else if (cmd.dir == Direction::Up) {
                        grown.top -= step;
                        shrunk.bottom -= step;
                    } else {
                        grown.bottom += step;
                        shrunk.top += step;
                    }
                    LayoutParams grownParams = params;
                    LayoutParams shrunkParams = params;
                    if (ApplyResize(area, n, grownParams, idx, grown)) {
                        ws.ratios = grownParams.ratios;
                    } else if (ApplyResize(area, n, shrunkParams, idx, shrunk)) {
                        ws.ratios = shrunkParams.ratios;
                    }
                    Retile(mon);
                }
            }
            break;
        case CommandKind::WorkspaceNext:
        case CommandKind::WorkspacePrev: {
            ActiveWorkspace(fm);  // also normalises m.active and ensures a workspace
            const int count = static_cast<int>(g.monitors[fm].workspaces.size());
            const int delta = cmd.kind == CommandKind::WorkspaceNext ? 1 : -1;
            SwitchWorkspace(fm, (g.monitors[fm].active + delta + count) % count);
            break;
        }
        case CommandKind::CloseWindow:
            if (has) PostMessageW(cur, WM_SYSCOMMAND, SC_CLOSE, 0);
            break;
        case CommandKind::Fullscreen:
            // The LOCATIONCHANGE handler takes it out of or back into the layout.
            if (has) {
                ShowWindowAsync(cur, IsZoomed(cur) ? SW_RESTORE : SW_MAXIMIZE);
            }
            break;
        default:
            break;
    }
}

void AdoptExistingWindows() {
    std::vector<HWND> all;
    EnumWindows(CollectWindow, reinterpret_cast<LPARAM>(&all));
    // EnumWindows lists top-most first; adopt bottom-most first so it ends up
    // as the first tile.
    std::reverse(all.begin(), all.end());
    int adopted = 0;
    for (HWND h : all) {
        if (Locate(h, nullptr)) continue;
        const Eligibility e = ClassifyWindow(h, g.settings);
        if (e == Eligibility::Ignore) continue;
        if (ManageWindow(h, e == Eligibility::Float)) ++adopted;
    }
    HT_LOG(L"adopted %d window(s)", adopted);
}

}  // namespace ht

// =============================================================================
// 55_osd.cpp — workspace indicator: a small topmost, never activated panel with
// one column per workspace of a monitor. Each column has a numbered badge;
// occupied workspaces also show one icon per app. As the popup it is
// click-through; as the workspace overview (hold left Alt) the same panel takes
// the mouse, see ShowWorkspaceOverview.
// Engine thread only; all coordinates are physical pixels (per-monitor DPI v2).
// =============================================================================

#include <cmath>
#include <shellscalingapi.h>
#include <unordered_map>
#include <windowsx.h>

namespace ht {

namespace {

// Geometry in 96-dpi units; every value goes through OsdScale.
constexpr int kOsdPadDip = 14;           // outer padding
constexpr int kOsdColGapDip = 6;         // gap between columns
constexpr int kOsdColumnDip = 36;        // every column, occupied or empty
constexpr int kOsdBadgeDip = 30;         // badge circle diameter, centered in its column
constexpr int kOsdIconTopDip = 10;       // badge bottom to the first icon row
constexpr int kOsdIconDip = 16;          // app icon size
constexpr int kOsdIconGapDip = 4;        // vertical gap between icon rows
constexpr int kOsdMaxRows = 8;           // icon rows per column; 7 icons + "+N" when crowded
constexpr int kOsdCountDip = 11;         // count badge diameter, centered on the icon's corner
constexpr int kOsdSmallFontDip = 8;      // count badge and "+N" font height
constexpr int kOsdNumberDip = 15;        // badge number font height
constexpr int kOsdDotDip = 10;           // notification dot diameter
constexpr int kOsdDotRingDip = 3;        // ring: 2 * 1.5 dip wider than the dot
constexpr double kOsdDotDiagonal = 0.7071;  // cos and sin of 45 degrees
constexpr int kOsdDragIconDip = 20;         // app icon while it is dragged
constexpr int kOsdHighlightCornerDip = 8;   // corner radius: drop target, column hover
constexpr int kOsdCellCornerDip = 4;        // corner radius: empty slot of a dragged icon
constexpr int kOsdBadgeItem = -1;           // hit test, press: the column's badge
constexpr int kOsdNoItem = -2;              // hit test, hover, press: no badge or icon there
constexpr int kOsdMaxCount = 9;
constexpr int kOsdDefaultDurationMs = 700;
constexpr int kOsdAlpha = 235;
constexpr size_t kOsdExeIconCacheMax = 128;
constexpr UINT_PTR kOsdHideTimer = 1;

// DWMWA_WINDOW_CORNER_PREFERENCE (Windows 11); DWMWCP_ROUND = 2.
constexpr DWORD kOsdCornerAttr = 33;
constexpr DWORD kOsdCornerRound = 2;

// Theme sources (HKCU). The OSD is shell UI, so it follows the system theme.
constexpr wchar_t kOsdPersonalizeKey[] =
    L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize";
constexpr wchar_t kOsdAccentKey[] =
    L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Accent";

// One app on a workspace as shown: its icon, how many windows it has there, and
// those windows (empty for the "+N" slot, which is never draggable).
struct OsdIcon {
    HICON icon = nullptr;  // always set once resolved
    int count = 1;
    std::vector<HWND> windows;
};

// One workspace column, laid out on show and painted as it is stored.
struct OsdColumn {
    int left = 0;                // client x, physical pixels
    int width = 0;               // physical pixels
    bool occupied = false;       // has tiled or floating windows
    bool attention = false;      // notification dot: not the active one, needs attention
    std::vector<OsdIcon> icons;  // one per app, in first-seen order
    int more = 0;                // N of the "+N" slot after the icons; 0 = none

    int slots() const {
        return static_cast<int>(icons.size()) + (more > 0 ? 1 : 0);
    }
};

struct OsdState {
    HWND hwnd = nullptr;
    HINSTANCE hinst = nullptr;
    bool classRegistered = false;
    bool failureLogged = false;  // creation errors are logged once per engine run
    HFONT numberBold = nullptr;
    HFONT numberNormal = nullptr;
    HFONT smallFont = nullptr;   // count badges and "+N"
    int dpi = 96;
    int count = 0;
    int active = 0;
    int rows = 0;  // most icon rows in any column
    std::vector<OsdColumn> columns;
    Theme theme{};

    // Overview (hold left Alt): takes mouse input and has no hide timer.
    bool interactive = false;
    int mon = -1;                  // monitor currently rendered
    bool closeAfterPress = false;  // Alt was released during a press: close when it ends
    // Press and drag. pressCol < 0 means no press; pressItem is kOsdBadgeItem or an icon index.
    int pressCol = -1;
    int pressItem = kOsdNoItem;
    POINT pressPt{};
    bool dragging = false;
    POINT cursor{};    // last mouse position, client pixels
    int dropCol = -1;  // column the dragged icon would land in; -1 = none
    // Hover: the column under the mouse while nothing is pressed; -1 = none.
    int hoverCol = -1;
    bool trackingLeave = false;  // TME_LEAVE is armed
};

OsdState osd;

// An exe's own icon, owned by the cache. Null when the file has no icon.
struct OsdCachedIcon {
    HICON icon = nullptr;
    unsigned stamp = 0;  // osdShowStamp of the last show that used it
};

// Exe path (lowercase) → its icon, destroyed when evicted or in DestroyOsd.
std::unordered_map<std::wstring, OsdCachedIcon> osdExeIcons;
unsigned osdShowStamp = 0;  // incremented once per show

// The windows of one executable on a workspace, shown as one icon.
struct OsdGroup {
    HWND hwnd = nullptr;  // first window seen; represents the group
    std::wstring path;    // lowercase exe path; empty when unreadable
    int count = 0;        // == windows.size()
    std::vector<HWND> windows;  // every window of the group, first-seen order
};

int OsdScale(int dip) {
    return MulDiv(dip, osd.dpi, 96);
}

HFONT OsdMakeFont(int heightDip, int weight) {
    return CreateFontW(-OsdScale(heightDip), 0, 0, 0, weight, FALSE, FALSE, FALSE,
                       DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                       CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
                       L"Segoe UI");
}

void OsdDeleteFonts() {
    if (osd.numberBold) DeleteObject(osd.numberBold);
    if (osd.numberNormal) DeleteObject(osd.numberNormal);
    if (osd.smallFont) DeleteObject(osd.smallFont);
    osd.numberBold = nullptr;
    osd.numberNormal = nullptr;
    osd.smallFont = nullptr;
}

// Replaces the font set for the current DPI. Any font may be null on failure;
// painting then falls back to the default GUI font.
void OsdRebuildFonts() {
    OsdDeleteFonts();
    osd.numberBold = OsdMakeFont(kOsdNumberDip, FW_BOLD);
    osd.numberNormal = OsdMakeFont(kOsdNumberDip, FW_NORMAL);
    osd.smallFont = OsdMakeFont(kOsdSmallFontDip, FW_BOLD);
}

// Per-channel linear mix: t = 0 gives a, t = 1 gives b. Rounded to nearest.
COLORREF OsdBlend(COLORREF a, COLORREF b, double t) {
    auto mix = [t](int x, int y) {
        return static_cast<int>(std::lround(x + (y - x) * t));
    };
    return RGB(mix(GetRValue(a), GetRValue(b)), mix(GetGValue(a), GetGValue(b)),
               mix(GetBValue(a), GetBValue(b)));
}

// Full image path of the window's process, or "" when it cannot be read
// (for example an elevated process).
std::wstring OsdProcessPath(HWND hwnd) {
    DWORD pid = 0;
    GetWindowThreadProcessId(hwnd, &pid);
    if (pid == 0) return {};
    HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (!process) return {};

    wchar_t buf[1024];
    DWORD len = 1024;
    std::wstring path;
    if (QueryFullProcessImageNameW(process, 0, buf, &len) && len > 0) {
        path.assign(buf, len);
    }
    CloseHandle(process);
    return path;
}

// Groups the workspace's windows (tiled, then floating) by lowercase exe path,
// in first-seen order. A window whose path cannot be read forms its own group.
std::vector<OsdGroup> OsdGroupWindows(const Workspace& ws) {
    std::vector<OsdGroup> groups;
    std::unordered_map<std::wstring, size_t> byKey;
    auto add = [&groups, &byKey](HWND hwnd) {
        std::wstring path = OsdProcessPath(hwnd);
        std::wstring key;
        if (path.empty()) {
            key = L"hwnd:" + std::to_wstring(static_cast<unsigned long long>(
                                 reinterpret_cast<UINT_PTR>(hwnd)));
        } else {
            for (wchar_t& c : path) c = static_cast<wchar_t>(std::towlower(c));
            key = path;
        }
        const auto [it, inserted] = byKey.try_emplace(key, groups.size());
        if (inserted) groups.push_back(OsdGroup{hwnd, path, 0});
        ++groups[it->second].count;
        groups[it->second].windows.push_back(hwnd);
    };
    for (HWND hwnd : ws.tiled) add(hwnd);
    for (HWND hwnd : ws.floating) add(hwnd);
    return groups;
}

// The exe's own icon, from the cache or read from the file. The cache owns the
// returned icon. Entries used by the current show are never evicted, and
// evicted icons are destroyed. Returns nullptr when the file has no icon.
HICON OsdCachedExeIcon(const std::wstring& path) {
    const auto hit = osdExeIcons.find(path);
    if (hit != osdExeIcons.end()) {
        hit->second.stamp = osdShowStamp;
        return hit->second.icon;
    }

    HICON large = nullptr;
    ExtractIconExW(path.c_str(), 0, &large, nullptr, 1);

    if (osdExeIcons.size() >= kOsdExeIconCacheMax) {
        // Least recently used entry that this show does not need.
        auto victim = osdExeIcons.end();
        for (auto it = osdExeIcons.begin(); it != osdExeIcons.end(); ++it) {
            if (it->second.stamp == osdShowStamp) continue;
            if (victim == osdExeIcons.end() ||
                it->second.stamp < victim->second.stamp) {
                victim = it;
            }
        }
        if (victim == osdExeIcons.end()) {
            // Unreachable: one show uses at most 9 workspaces x 9 icons.
            if (large) DestroyIcon(large);
            return nullptr;
        }
        if (victim->second.icon) DestroyIcon(victim->second.icon);
        osdExeIcons.erase(victim);
    }
    osdExeIcons.emplace(path, OsdCachedIcon{large, osdShowStamp});
    return large;
}

// Icon for one app. Order: the window's own big icon, then its small icons,
// then the window class icons, then the exe's icon, then the system default.
// Only the exe icon is owned (see OsdCachedExeIcon); the others belong to the
// window or the system and are never destroyed.
HICON OsdIconFor(HWND hwnd, const std::wstring& exePath) {
    constexpr UINT kIconSources[] = {ICON_BIG, ICON_SMALL2, ICON_SMALL};
    for (UINT source : kIconSources) {
        DWORD_PTR result = 0;
        if (SendMessageTimeoutW(hwnd, WM_GETICON, source, 0,
                                SMTO_ABORTIFHUNG | SMTO_BLOCK, 50, &result) != 0 &&
            result != 0) {
            return reinterpret_cast<HICON>(result);
        }
    }
    if (HICON icon = reinterpret_cast<HICON>(GetClassLongPtrW(hwnd, GCLP_HICON))) {
        return icon;
    }
    if (HICON icon = reinterpret_cast<HICON>(GetClassLongPtrW(hwnd, GCLP_HICONSM))) {
        return icon;
    }
    if (!exePath.empty()) {
        if (HICON icon = OsdCachedExeIcon(exePath)) return icon;
    }
    return LoadIconW(nullptr, IDI_APPLICATION);
}

// Count badge: a filled disc centered on an icon's bottom-right corner (cx, cy),
// holding the count ("9+" beyond nine). The pen is NULL_PEN for the whole paint.
void OsdPaintCountBadge(HDC dc, int cx, int cy, int count, HGDIOBJ font) {
    const int d = OsdScale(kOsdCountDip);
    RECT disc{cx - d / 2, cy - d / 2, cx - d / 2 + d, cy - d / 2 + d};

    HBRUSH brush = CreateSolidBrush(osd.theme.activeFill);
    HGDIOBJ oldBrush = SelectObject(dc, brush);
    Ellipse(dc, disc.left, disc.top, disc.right, disc.bottom);
    SelectObject(dc, oldBrush);
    DeleteObject(brush);

    const std::wstring label =
        count > 9 ? std::wstring(L"9+") : std::to_wstring(count);
    SelectObject(dc, font);
    SetTextColor(dc, osd.theme.activeText);
    DrawTextW(dc, label.c_str(), static_cast<int>(label.size()), &disc,
              DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
}

// Notification dot: a ring in the background color with the dot in the active
// fill on top, centered 45 degrees toward the badge's top-right.
void OsdPaintDot(HDC dc, const RECT& badgeCircle) {
    const double radius = (badgeCircle.right - badgeCircle.left) / 2.0;
    const double centerX = (badgeCircle.left + badgeCircle.right) / 2.0;
    const double centerY = (badgeCircle.top + badgeCircle.bottom) / 2.0;
    const int cx = static_cast<int>(std::lround(centerX + radius * kOsdDotDiagonal));
    const int cy = static_cast<int>(std::lround(centerY - radius * kOsdDotDiagonal));
    const int dot = OsdScale(kOsdDotDip);
    const int ring = dot + std::max(2, OsdScale(kOsdDotRingDip));

    auto disc = [dc, cx, cy](int d, COLORREF color) {
        HBRUSH brush = CreateSolidBrush(color);
        HGDIOBJ oldBrush = SelectObject(dc, brush);
        Ellipse(dc, cx - d / 2, cy - d / 2, cx - d / 2 + d, cy - d / 2 + d);
        SelectObject(dc, oldBrush);
        DeleteObject(brush);
    };
    disc(ring, osd.theme.background);
    disc(dot, osd.theme.activeFill);
}

// Rounded rectangle with corner radius `radius` (GDI takes the ellipse size).
void OsdRoundRect(HDC dc, const RECT& r, int radius) {
    RoundRect(dc, r.left, r.top, r.right, r.bottom, radius * 2, radius * 2);
}

// Every pixel measure the panel is laid out with, for the current DPI. Painting
// and hit testing both read it, so the two cannot disagree.
struct OsdMetrics {
    int pad = 0;       // outer padding
    int gap = 0;       // between columns
    int column = 0;    // column width
    int badge = 0;     // badge circle diameter
    int iconSize = 0;  // app icon size
    int iconGap = 0;   // gap between icon rows
    int iconTop = 0;   // top of the first icon row
    int step = 0;      // iconSize + iconGap: distance between icon rows

    int rowTop(int row) const { return iconTop + row * step; }
    // Vertical band of an icon row: the icon plus half the gap on each side.
    int bandTop(int row) const { return rowTop(row) - iconGap / 2; }
    int bandBottom(int row) const {
        return rowTop(row) + iconSize + iconGap - iconGap / 2;
    }
};

OsdMetrics OsdCurrentMetrics() {
    OsdMetrics m;
    m.pad = OsdScale(kOsdPadDip);
    m.gap = OsdScale(kOsdColGapDip);
    m.column = OsdScale(kOsdColumnDip);
    m.badge = OsdScale(kOsdBadgeDip);
    m.iconSize = OsdScale(kOsdIconDip);
    m.iconGap = OsdScale(kOsdIconGapDip);
    m.iconTop = m.pad + m.badge + OsdScale(kOsdIconTopDip);
    m.step = m.iconSize + m.iconGap;
    return m;
}

// Mouse zone of column i along x, in client pixels [left, right). The zones tile
// the client width: the first starts at 0 and the last ends at clientRight.
void OsdZoneBounds(size_t i, int clientRight, const OsdMetrics& m, int& left,
                   int& right) {
    const OsdColumn& col = osd.columns[i];
    left = i == 0 ? 0 : col.left - m.gap / 2;
    right = i + 1 == osd.columns.size() ? clientRight
                                        : col.left + col.width + (m.gap - m.gap / 2);
}

// What lies under a client point: the column zone (-1 outside the panel), and
// in that column the badge, an icon index, or nothing.
struct OsdHit {
    int col = -1;
    int item = kOsdNoItem;
};

OsdHit OsdHitTest(POINT pt) {
    OsdHit hit;
    RECT client{};
    GetClientRect(osd.hwnd, &client);
    if (!PtInRect(&client, pt)) return hit;

    const OsdMetrics m = OsdCurrentMetrics();
    for (size_t i = 0; i < osd.columns.size(); ++i) {
        int left = 0;
        int right = 0;
        OsdZoneBounds(i, client.right, m, left, right);
        if (pt.x < left || pt.x >= right) continue;

        hit.col = static_cast<int>(i);
        if (pt.y >= m.pad && pt.y < m.pad + m.badge) {
            hit.item = kOsdBadgeItem;
            break;
        }
        const int icons = static_cast<int>(osd.columns[i].icons.size());
        for (int s = 0; s < icons; ++s) {
            if (pt.y >= m.bandTop(s) && pt.y < m.bandBottom(s)) {
                hit.item = s;
                break;
            }
        }
        break;
    }
    return hit;
}

bool OsdPressed() {
    return osd.pressCol >= 0;
}

// Forgets the press and the drag. Nothing is repainted.
void OsdClearPress() {
    osd.pressCol = -1;
    osd.pressItem = kOsdNoItem;
    osd.dragging = false;
    osd.dropCol = -1;
}

// Drops a press in progress with no effect and gives the capture back. The state
// is cleared first, because ReleaseCapture sends WM_CAPTURECHANGED synchronously.
void OsdAbandonPress() {
    OsdClearPress();
    if (osd.hwnd && GetCapture() == osd.hwnd) ReleaseCapture();
}

void OsdInvalidate() {
    if (osd.hwnd) InvalidateRect(osd.hwnd, nullptr, FALSE);
}

// The rounded rectangle over the whole zone of column `col`, shared by the drop
// target and the hover. Normally a tint `tint` of the way from the background to
// the active fill; in high contrast (no tint to blend) a `frame`-px outline.
void OsdPaintColumnZone(HDC dc, size_t col, const RECT& client, const OsdMetrics& m,
                        double tint, int frame) {
    int left = 0;
    int right = 0;
    OsdZoneBounds(col, client.right, m, left, right);
    const RECT zone{left + 1, m.pad / 2, right - 1, client.bottom - m.pad / 2};
    const int corner = OsdScale(kOsdHighlightCornerDip);
    if (osd.theme.highContrast) {
        HPEN pen = CreatePen(PS_SOLID, frame, osd.theme.activeFill);
        HGDIOBJ oldPen = SelectObject(dc, pen);
        HGDIOBJ oldBrush = SelectObject(dc, GetStockObject(NULL_BRUSH));
        OsdRoundRect(dc, zone, corner);
        SelectObject(dc, oldBrush);
        SelectObject(dc, oldPen);
        DeleteObject(pen);
    } else {
        HBRUSH brush = CreateSolidBrush(OsdBlend(osd.theme.background, osd.theme.activeFill, tint));
        HGDIOBJ oldBrush = SelectObject(dc, brush);
        OsdRoundRect(dc, zone, corner);
        SelectObject(dc, oldBrush);
        DeleteObject(brush);
    }
}

// Draws into an off-screen bitmap and copies it once, so the layered window
// never shows a half-drawn state.
void OsdPaint(HWND hwnd, HDC target) {
    RECT client{};
    GetClientRect(hwnd, &client);
    const int w = client.right - client.left;
    const int h = client.bottom - client.top;

    HDC mem = CreateCompatibleDC(target);
    HBITMAP bmp = mem ? CreateCompatibleBitmap(target, w, h) : nullptr;
    if (!bmp) {
        if (mem) DeleteDC(mem);
        return;
    }
    HGDIOBJ oldBmp = SelectObject(mem, bmp);
    HGDIOBJ oldPen = SelectObject(mem, GetStockObject(NULL_PEN));
    HGDIOBJ defaultFont = GetStockObject(DEFAULT_GUI_FONT);
    HGDIOBJ oldFont = SelectObject(mem, defaultFont);
    SetBkMode(mem, TRANSPARENT);

    HBRUSH bg = CreateSolidBrush(osd.theme.background);
    FillRect(mem, &client, bg);
    DeleteObject(bg);

    const OsdMetrics m = OsdCurrentMetrics();
    const HGDIOBJ smallFont = osd.smallFont ? osd.smallFont : defaultFont;
    const int ncols = static_cast<int>(osd.columns.size());

    // The icon being dragged, if any. Its slot is only an outline until it is dropped.
    const OsdIcon* dragged = nullptr;
    if (osd.dragging && osd.pressCol >= 0 && osd.pressCol < ncols) {
        const std::vector<OsdIcon>& icons =
            osd.columns[static_cast<size_t>(osd.pressCol)].icons;
        if (osd.pressItem >= 0 && osd.pressItem < static_cast<int>(icons.size())) {
            dragged = &icons[static_cast<size_t>(osd.pressItem)];
        }
    }

    // Drop target: the whole zone of the column the dragged icon would land in.
    // Drop target: the whole zone of the column the dragged icon would land in.
    if (dragged && osd.dropCol >= 0 && osd.dropCol < ncols) {
        OsdPaintColumnZone(mem, static_cast<size_t>(osd.dropCol), client, m, 0.30, 2);
    }

    // Hover: the whole zone of the column under the mouse. Never while pressed.
    if (osd.interactive && !OsdPressed() && osd.hoverCol >= 0 && osd.hoverCol < ncols) {
        OsdPaintColumnZone(mem, static_cast<size_t>(osd.hoverCol), client, m, 0.15, 1);
    }

    for (size_t i = 0; i < osd.columns.size(); ++i) {
        const OsdColumn& col = osd.columns[i];
        const bool isActive = static_cast<int>(i) == osd.active;

        COLORREF fill = osd.theme.emptyFill;
        COLORREF text = osd.theme.emptyText;
        HFONT numberFont = osd.numberNormal;
        if (isActive) {
            fill = osd.theme.activeFill;
            text = osd.theme.activeText;
            numberFont = osd.numberBold;
        } else if (col.occupied) {
            fill = osd.theme.occupiedFill;
            text = osd.theme.occupiedText;
            numberFont = osd.numberBold;
        }

        // Badge: circle centered in the column, top at the padding.
        const int badgeLeft = col.left + (col.width - m.badge) / 2;
        RECT circle{badgeLeft, m.pad, badgeLeft + m.badge, m.pad + m.badge};
        HBRUSH brush = CreateSolidBrush(fill);
        HGDIOBJ oldBrush = SelectObject(mem, brush);
        Ellipse(mem, circle.left, circle.top, circle.right, circle.bottom);
        SelectObject(mem, oldBrush);
        DeleteObject(brush);

        SelectObject(mem, numberFont ? numberFont : defaultFont);
        SetTextColor(mem, text);
        // At most 9 workspaces, so the number is always a single digit.
        wchar_t number[2] = {static_cast<wchar_t>(L'1' + static_cast<int>(i)),
                             L'\0'};
        DrawTextW(mem, number, 1, &circle,
                  DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        if (col.attention) OsdPaintDot(mem, circle);

        // Icons: one per row, centered under the badge. Count badges are drawn
        // after all icons so that no icon covers another icon's badge. The row
        // after the icons holds "+N" when the workspace is crowded.
        const int iconLeft = col.left + (col.width - m.iconSize) / 2;
        const int stack = static_cast<int>(col.icons.size());
        // The dragged icon's own slot is skipped: it shows only as an outline.
        const int source = (dragged && static_cast<int>(i) == osd.pressCol) ? osd.pressItem : -1;
        for (int s = 0; s < stack; ++s) {
            if (s == source) continue;
            DrawIconEx(mem, iconLeft, m.rowTop(s),
                       col.icons[static_cast<size_t>(s)].icon, m.iconSize, m.iconSize,
                       0, nullptr, DI_NORMAL);
        }
        for (int s = 0; s < stack; ++s) {
            const OsdIcon& icon = col.icons[static_cast<size_t>(s)];
            if (s != source && icon.count > 1) {
                OsdPaintCountBadge(mem, iconLeft + m.iconSize, m.rowTop(s) + m.iconSize,
                                   icon.count, smallFont);
            }
        }
        if (source >= 0 && source < stack) {
            const int top = m.rowTop(source);
            const RECT cell{iconLeft, top, iconLeft + m.iconSize, top + m.iconSize};
            HPEN outline = CreatePen(PS_SOLID, 1, osd.theme.emptyText);
            HGDIOBJ oldOutlinePen = SelectObject(mem, outline);
            HGDIOBJ oldCellBrush = SelectObject(mem, GetStockObject(NULL_BRUSH));
            OsdRoundRect(mem, cell, OsdScale(kOsdCellCornerDip));
            SelectObject(mem, oldCellBrush);
            SelectObject(mem, oldOutlinePen);
            DeleteObject(outline);
        }
        if (col.more > 0) {
            const int top = m.rowTop(stack);
            RECT cell{iconLeft, top, iconLeft + m.iconSize, top + m.iconSize};
            const std::wstring more = L"+" + std::to_wstring(col.more);
            SelectObject(mem, smallFont);
            SetTextColor(mem, osd.theme.listText);
            DrawTextW(mem, more.c_str(), static_cast<int>(more.size()), &cell,
                      DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
        }
    }

    // The dragged icon, last, centered on the cursor.
    if (dragged) {
        const int size = OsdScale(kOsdDragIconDip);
        const int left = osd.cursor.x - size / 2;
        const int top = osd.cursor.y - size / 2;
        DrawIconEx(mem, left, top, dragged->icon, size, size, 0, nullptr, DI_NORMAL);
        if (dragged->count > 1) {
            OsdPaintCountBadge(mem, left + size, top + size, dragged->count, smallFont);
        }
    }

    BitBlt(target, 0, 0, w, h, mem, 0, 0, SRCCOPY);

    SelectObject(mem, oldFont);
    SelectObject(mem, oldPen);
    SelectObject(mem, oldBmp);
    DeleteObject(bmp);
    DeleteDC(mem);
}

// Renders the panel for monitor `mon` (defined below; the mouse handling
// re-renders after each action).
bool OsdRender(int mon);

LRESULT CALLBACK OsdWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
        case WM_PAINT: {
            PAINTSTRUCT ps{};
            HDC hdc = BeginPaint(hwnd, &ps);
            if (hdc) OsdPaint(hwnd, hdc);
            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_ERASEBKGND:
            return 1;  // OsdPaint covers the whole client area
        case WM_NCHITTEST:
            // Click-through as the popup; the overview takes the mouse.
            return osd.interactive ? HTCLIENT : HTTRANSPARENT;
        case WM_MOUSEACTIVATE:
            return MA_NOACTIVATE;
        case WM_SETCURSOR:
            if (osd.interactive && LOWORD(lp) == HTCLIENT) {
                POINT pt{};
                GetCursorPos(&pt);
                ScreenToClient(hwnd, &pt);
                const OsdHit hit = OsdHitTest(pt);
                // The whole panel is a target; a drag keeps its move cursor.
                const bool moving = osd.dragging;
                SetCursor(LoadCursorW(nullptr, moving ? IDC_SIZEALL
                                              : hit.col >= 0 ? IDC_HAND
                                                             : IDC_ARROW));
                return TRUE;
            }
            break;
        case WM_LBUTTONDOWN: {
            if (!osd.interactive) return 0;
            const POINT pt{GET_X_LPARAM(lp), GET_Y_LPARAM(lp)};
            const OsdHit hit = OsdHitTest(pt);
            if (hit.col < 0) return 0;
            osd.pressCol = hit.col;
            // Anywhere in the column but on an icon counts as the badge.
            osd.pressItem = hit.item == kOsdNoItem ? kOsdBadgeItem : hit.item;
            osd.pressPt = pt;
            osd.cursor = pt;
            osd.dragging = false;
            osd.dropCol = -1;
            // The press is the action now, so the hover goes.
            if (osd.hoverCol >= 0) {
                osd.hoverCol = -1;
                OsdInvalidate();
            }
            SetCapture(hwnd);
            return 0;
        }
        case WM_MOUSEMOVE: {
            if (!osd.interactive) return 0;
            const POINT pt{GET_X_LPARAM(lp), GET_Y_LPARAM(lp)};
            osd.cursor = pt;
            if (OsdPressed()) {
                // Only an icon drags, and only once the mouse has really moved.
                if (!osd.dragging && osd.pressItem >= 0) {
                    const int dx = pt.x - osd.pressPt.x;
                    const int dy = pt.y - osd.pressPt.y;
                    const int cx = GetSystemMetrics(SM_CXDRAG);
                    const int cy = GetSystemMetrics(SM_CYDRAG);
                    osd.dragging = dx > cx || dx < -cx || dy > cy || dy < -cy;
                }
                if (osd.dragging) {
                    const OsdHit hit = OsdHitTest(pt);
                    osd.dropCol = (hit.col >= 0 && hit.col != osd.pressCol) ? hit.col : -1;
                    SetCursor(LoadCursorW(nullptr, IDC_SIZEALL));
                    OsdInvalidate();
                }
                return 0;
            }

            // Not pressed: track the hover and ask for WM_MOUSELEAVE once.
            if (!osd.trackingLeave) {
                TRACKMOUSEEVENT tme{};
                tme.cbSize = sizeof(tme);
                tme.dwFlags = TME_LEAVE;
                tme.hwndTrack = hwnd;
                osd.trackingLeave = TrackMouseEvent(&tme) != FALSE;
            }
            // The column under the mouse, -1 outside the panel.
            const int hoverCol = OsdHitTest(pt).col;
            if (hoverCol != osd.hoverCol) {
                osd.hoverCol = hoverCol;
                OsdInvalidate();
            }
            return 0;
        }
        case WM_MOUSELEAVE:
            osd.trackingLeave = false;
            if (osd.hoverCol >= 0) {
                osd.hoverCol = -1;
                OsdInvalidate();
            }
            return 0;
        case WM_LBUTTONUP: {
            if (!OsdPressed()) return 0;
            // Take what the release needs and clear the press before ReleaseCapture,
            // which sends WM_CAPTURECHANGED synchronously and must find no press.
            const int mon = osd.mon;
            const int pressCol = osd.pressCol;
            const int pressItem = osd.pressItem;
            const bool dragging = osd.dragging;
            const int dropCol = osd.dropCol;
            std::vector<HWND> windows;  // the pressed icon's windows
            if (pressItem >= 0 && pressCol < static_cast<int>(osd.columns.size())) {
                const std::vector<OsdIcon>& icons =
                    osd.columns[static_cast<size_t>(pressCol)].icons;
                if (pressItem < static_cast<int>(icons.size())) {
                    windows = icons[static_cast<size_t>(pressItem)].windows;
                }
            }
            OsdClearPress();
            if (GetCapture() == hwnd) ReleaseCapture();

            const POINT pt{GET_X_LPARAM(lp), GET_Y_LPARAM(lp)};
            const OsdHit hit = OsdHitTest(pt);
            if (dragging) {
                if (dropCol >= 0) {
                    int moved = 0;
                    for (HWND window : windows) {
                        WinLoc loc;
                        if (Locate(window, &loc) && loc.mon == mon && loc.ws == pressCol) {
                            MoveWindowToWorkspace(window, mon, dropCol);
                            ++moved;
                        }
                    }
                    HT_LOG(L"overview: moved %d window(s) from workspace %d to %d",
                           moved, pressCol + 1, dropCol + 1);
                }
            } else if (hit.col == pressCol &&
                       (hit.item == kOsdNoItem ? kOsdBadgeItem : hit.item) == pressItem) {
                // Same mapping as the press: anywhere in the column but on an icon is the badge.
                if (pressItem == kOsdBadgeItem) {
                    SwitchWorkspace(mon, pressCol);
                } else {
                    for (HWND window : windows) {
                        WinLoc loc;
                        if (Locate(window, &loc)) {
                            ActivateWindowWorkspace(window);
                            break;
                        }
                    }
                }
            }
            // The actions above may have re-rendered the panel, so the flags are read now.
            if (osd.closeAfterPress) {
                CloseWorkspaceOverview(false);
            } else if (WorkspaceOverviewOpen()) {
                OsdRender(osd.mon);
            }
            return 0;
        }
        case WM_CAPTURECHANGED:
            // Another window took the mouse during a press: the press is dropped, no effect.
            if (OsdPressed()) {
                OsdClearPress();
                if (osd.closeAfterPress) {
                    CloseWorkspaceOverview(true);
                } else {
                    osd.hoverCol = -1;
                    OsdInvalidate();
                }
            }
            return 0;
        case WM_TIMER:
            if (wp == kOsdHideTimer) {
                KillTimer(hwnd, kOsdHideTimer);
                ShowWindow(hwnd, SW_HIDE);
            }
            return 0;
        default:
            break;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

// Logs the first creation failure only; later calls return quietly.
bool OsdFailed(const wchar_t* step, DWORD err) {
    if (!osd.failureLogged) {
        osd.failureLogged = true;
        HT_LOG(L"OSD: %s failed (error %lu)", step, err);
    }
    return false;
}

// Registers the class and creates the hidden overlay window on first use.
bool OsdEnsureWindow() {
    if (osd.hwnd && IsWindow(osd.hwnd)) return true;
    osd.hwnd = nullptr;

    if (!osd.classRegistered) {
        // Address of our own code identifies this module (the mod DLL).
        HINSTANCE hinst = nullptr;
        if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                                    GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                                reinterpret_cast<LPCWSTR>(&OsdWndProc),
                                &hinst)) {
            return OsdFailed(L"module lookup", GetLastError());
        }
        osd.hinst = hinst;

        WNDCLASSEXW wc{};
        wc.cbSize = sizeof(wc);
        wc.lpfnWndProc = OsdWndProc;
        wc.hInstance = osd.hinst;
        wc.lpszClassName = kOsdWindowClass;  // no cursor, no background: we paint all
        if (!RegisterClassExW(&wc)) {
            return OsdFailed(L"class registration", GetLastError());
        }
        osd.classRegistered = true;
    }

    const DWORD exStyle = WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW |
                          WS_EX_NOACTIVATE | WS_EX_TRANSPARENT;
    HWND hwnd = CreateWindowExW(exStyle, kOsdWindowClass, L"HawkTile workspaces",
                                WS_POPUP, 0, 0, 1, 1, nullptr, nullptr,
                                osd.hinst, nullptr);
    if (!hwnd) return OsdFailed(L"window creation", GetLastError());

    // A layered window with no alpha or color key is never displayed.
    if (!SetLayeredWindowAttributes(hwnd, 0, static_cast<BYTE>(kOsdAlpha),
                                    LWA_ALPHA)) {
        const DWORD err = GetLastError();
        DestroyWindow(hwnd);
        return OsdFailed(L"layered attributes", err);
    }

    // Rounded corners on Windows 11; older systems ignore the attribute.
    DWORD corner = kOsdCornerRound;
    DwmSetWindowAttribute(hwnd, kOsdCornerAttr, &corner, sizeof(corner));

    osd.hwnd = hwnd;
    return true;
}

// Click-through while the popup shows, mouse-capable while the overview is open.
// WS_EX_TRANSPARENT is what makes hit testing pass through, so it is removed for
// the overview and put back afterwards.
void OsdSetInteractive(bool on) {
    if (!osd.hwnd || !IsWindow(osd.hwnd)) return;
    const LONG_PTR ex = GetWindowLongPtrW(osd.hwnd, GWL_EXSTYLE);
    const LONG_PTR transparent = static_cast<LONG_PTR>(WS_EX_TRANSPARENT);
    const LONG_PTR next = on ? (ex & ~transparent) : (ex | transparent);
    if (next == ex) return;
    SetWindowLongPtrW(osd.hwnd, GWL_EXSTYLE, next);
    SetWindowPos(osd.hwnd, nullptr, 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE |
                     SWP_FRAMECHANGED);
}

// Builds the columns for monitor `mon` from its current workspaces and places the
// panel centered on its work area. Does not touch the hide timer. Returns false
// for a bad `mon` or when the window cannot be created.
bool OsdRender(int mon) {
    if (mon < 0 || mon >= static_cast<int>(g.monitors.size())) return false;
    if (!OsdEnsureWindow()) return false;
    // A hover survives the re-render; it is re-hit-tested below, once the layout is known.
    osd.mon = mon;

    const Monitor& m = g.monitors[mon];
    const int avail = static_cast<int>(m.workspaces.size());
    osd.count = std::clamp(avail, 1, kOsdMaxCount);
    osd.active = std::clamp(m.active, 0, osd.count - 1);
    osd.theme = ResolveTheme();

    // DPI differs per monitor, so the scale is taken from the target monitor.
    UINT dpiX = 96;
    UINT dpiY = 96;
    if (FAILED(GetDpiForMonitor(m.handle, MDT_EFFECTIVE_DPI, &dpiX, &dpiY)) ||
        dpiX == 0) {
        dpiX = 96;
    }
    osd.dpi = static_cast<int>(dpiX);
    OsdRebuildFonts();

    // Icons used by this show carry its stamp, so trimming the exe-icon cache
    // during the show never destroys one that is already in use.
    ++osdShowStamp;

    osd.columns.assign(static_cast<size_t>(osd.count), OsdColumn{});
    osd.rows = 0;
    for (int i = 0; i < osd.count && i < avail; ++i) {
        const Workspace& ws = m.workspaces[i];
        OsdColumn& col = osd.columns[static_cast<size_t>(i)];
        col.occupied = !ws.tiled.empty() || !ws.floating.empty();
        col.attention = i != osd.active && WorkspaceHasAttention(mon, i);
        if (!col.occupied) continue;

        const std::vector<OsdGroup> groups = OsdGroupWindows(ws);
        const int groupCount = static_cast<int>(groups.size());
        // Crowded: one icon fewer, and the last row shows "+N" for the rest.
        const int icons = groupCount > kOsdMaxRows ? kOsdMaxRows - 1 : groupCount;
        col.icons.reserve(static_cast<size_t>(icons));
        for (int j = 0; j < icons; ++j) {
            const OsdGroup& group = groups[static_cast<size_t>(j)];
            col.icons.push_back(
                OsdIcon{OsdIconFor(group.hwnd, group.path), group.count, group.windows});
        }
        if (groupCount > kOsdMaxRows) col.more = groupCount - icons;
        osd.rows = std::max(osd.rows, col.slots());
    }

    // Every column, occupied or empty, is the badge column width. The panel is
    // centered on the work area; 9 columns of this width always fit it.
    const OsdMetrics geo = OsdCurrentMetrics();
    int x = geo.pad;
    for (OsdColumn& col : osd.columns) {
        col.left = x;
        col.width = geo.column;
        x += geo.column + geo.gap;
    }
    const int width = x - geo.gap + geo.pad;
    // Down to the last icon row, or only the badge row when there are no icons.
    const int contentBottom =
        osd.rows > 0 ? geo.rowTop(osd.rows - 1) + geo.iconSize : geo.pad + geo.badge;
    const int height = contentBottom + geo.pad;
    const int areaW = static_cast<int>(m.rcWork.right - m.rcWork.left);
    const int areaH = static_cast<int>(m.rcWork.bottom - m.rcWork.top);

    const int x0 = static_cast<int>(m.rcWork.left) + (areaW - width) / 2;
    const int y0 = static_cast<int>(m.rcWork.top) + (areaH - height) / 2;
    SetWindowPos(osd.hwnd, HWND_TOPMOST, x0, y0, width, height,
                 SWP_NOACTIVATE | SWP_SHOWWINDOW);
    // Hover follows the cursor, not the old layout: only real mouse movement sets it,
    // so a panel that opens under a resting cursor has none.
    if (osd.hoverCol >= 0) {
        POINT pt{};
        GetCursorPos(&pt);
        ScreenToClient(osd.hwnd, &pt);
        osd.hoverCol = OsdHitTest(pt).col;
    }
    InvalidateRect(osd.hwnd, nullptr, FALSE);
    UpdateWindow(osd.hwnd);
    return true;
}

}  // namespace

// Resolves the theme from the current Windows settings; called on every show.
// High contrast wins. Otherwise the system light/dark setting picks the base
// colors, and the accent palette (or DWM's colorization color when the palette
// is unavailable) supplies the active fill.
Theme ResolveTheme() {
    Theme t{};

    HIGHCONTRASTW hc{};
    hc.cbSize = sizeof(hc);
    if (SystemParametersInfoW(SPI_GETHIGHCONTRAST, sizeof(hc), &hc, 0) &&
        (hc.dwFlags & HCF_HIGHCONTRASTON)) {
        t.highContrast = true;
        t.background = GetSysColor(COLOR_WINDOW);
        t.activeFill = GetSysColor(COLOR_HIGHLIGHT);
        t.activeText = GetSysColor(COLOR_HIGHLIGHTTEXT);
        t.occupiedFill = t.background;
        t.occupiedText = GetSysColor(COLOR_WINDOWTEXT);
        t.emptyFill = t.background;
        t.emptyText = GetSysColor(COLOR_GRAYTEXT);
        t.listText = GetSysColor(COLOR_WINDOWTEXT);
        t.activeListText = GetSysColor(COLOR_WINDOWTEXT);
        return t;
    }

    DWORD systemUsesLight = 0;
    DWORD dwordSize = sizeof(systemUsesLight);
    if (RegGetValueW(HKEY_CURRENT_USER, kOsdPersonalizeKey,
                     L"SystemUsesLightTheme", RRF_RT_REG_DWORD, nullptr,
                     &systemUsesLight, &dwordSize) != ERROR_SUCCESS) {
        systemUsesLight = 0;  // missing or unreadable: dark
    }

    // AccentPalette is 8 entries of R, G, B, A bytes: [0] Light3, [1] Light2,
    // [2] Light1, [3] Accent, [4] Dark1, [5] Dark2, [6] Dark3, [7] unused.
    BYTE palette[32]{};
    DWORD paletteSize = sizeof(palette);
    const bool havePalette =
        RegGetValueW(HKEY_CURRENT_USER, kOsdAccentKey, L"AccentPalette",
                     RRF_RT_REG_BINARY, nullptr, palette, &paletteSize) ==
            ERROR_SUCCESS &&
        paletteSize >= sizeof(palette);
    auto paletteEntry = [&palette](int i) {
        return RGB(palette[i * 4], palette[i * 4 + 1], palette[i * 4 + 2]);
    };

    COLORREF light2 = 0;
    COLORREF dark1 = 0;
    if (havePalette) {
        light2 = paletteEntry(1);
        dark1 = paletteEntry(4);
    } else {
        COLORREF base = RGB(0, 120, 212);
        DWORD argb = 0;
        BOOL opaque = FALSE;
        if (SUCCEEDED(DwmGetColorizationColor(&argb, &opaque))) {
            base = RGB((argb >> 16) & 0xFF, (argb >> 8) & 0xFF, argb & 0xFF);
        }
        light2 = OsdBlend(base, RGB(255, 255, 255), 0.45);
        dark1 = OsdBlend(base, RGB(0, 0, 0), 0.20);
    }

    // Accent fills use the lighter shade with dark text on a dark theme, and
    // the darker shade with white text on a light theme.
    if (systemUsesLight) {
        t.background = RGB(243, 243, 243);
        t.activeFill = dark1;
        t.activeText = RGB(255, 255, 255);
        t.occupiedFill = RGB(205, 205, 205);
        t.occupiedText = RGB(20, 20, 20);
        t.emptyFill = RGB(252, 252, 252);
        t.emptyText = RGB(150, 150, 150);
        t.listText = RGB(90, 90, 90);
        t.activeListText = RGB(25, 25, 25);
    } else {
        t.background = RGB(32, 32, 32);
        t.activeFill = light2;
        t.activeText = RGB(0, 0, 0);
        t.occupiedFill = RGB(72, 72, 72);
        t.occupiedText = RGB(255, 255, 255);
        t.emptyFill = RGB(22, 22, 22);
        t.emptyText = RGB(130, 130, 130);
        t.listText = RGB(170, 170, 170);
        t.activeListText = RGB(235, 235, 235);
    }
    return t;
}

void ShowWorkspaceOsd(int mon) {
    if (WorkspaceOverviewOpen()) {
        // The overview is open: re-render it in place, with no hide timer. A press in
        // progress is dropped first; if Alt was already released, the overview closes.
        if (OsdPressed()) {
            OsdAbandonPress();
            if (osd.closeAfterPress) {
                CloseWorkspaceOverview(true);
                return;
            }
        }
        OsdRender(mon);
        return;
    }
    // A leftover interactive mode would make the timed popup block clicks.
    if (osd.interactive) CloseWorkspaceOverview(true);
    if (!OsdRender(mon)) return;

    // Calling again while visible resets the timer, so the overlay stays up
    // for a full duration after the latest keypress.
    const int ms = g.settings.osdDurationMs > 0 ? g.settings.osdDurationMs
                                                : kOsdDefaultDurationMs;
    SetTimer(osd.hwnd, kOsdHideTimer, static_cast<UINT>(ms), nullptr);
}

void ShowWorkspaceOverview(int mon) {
    if (mon < 0 || mon >= static_cast<int>(g.monitors.size())) return;
    if (!OsdEnsureWindow()) return;

    const bool wasOpen = WorkspaceOverviewOpen();
    OsdAbandonPress();  // a press left over from an earlier overview is dropped
    KillTimer(osd.hwnd, kOsdHideTimer);
    osd.interactive = true;
    osd.closeAfterPress = false;
    OsdSetInteractive(true);
    OsdRender(mon);
    if (!wasOpen) HT_LOG(L"overview opened on monitor %d", mon);
}

void CloseWorkspaceOverview(bool cancelDrag) {
    // Keyed on the mode, not on visibility: an interactive panel that is hidden
    // for any reason must still go back to click-through.
    if (!osd.interactive) return;
    if (OsdPressed() && !cancelDrag) {
        // Hide once the press ends; the drop or click still happens.
        osd.closeAfterPress = true;
        return;
    }

    // Released over a column (no press here, so the hover is current): switch to
    // it once the panel is hidden.
    // Over the active workspace there is nothing to do (and SwitchWorkspace would
    // flash the timed popup).
    const bool switchTo = !cancelDrag && osd.hoverCol >= 0 && osd.mon >= 0 &&
                          osd.mon < static_cast<int>(g.monitors.size()) &&
                          osd.hoverCol != g.monitors[osd.mon].active;
    const int target = osd.hoverCol;
    const int mon = osd.mon;

    OsdAbandonPress();
    osd.hoverCol = -1;
    osd.trackingLeave = false;
    osd.interactive = false;
    osd.closeAfterPress = false;
    OsdSetInteractive(false);
    if (osd.hwnd) {
        KillTimer(osd.hwnd, kOsdHideTimer);
        ShowWindow(osd.hwnd, SW_HIDE);
    }
    if (switchTo) {
        HT_LOG(L"overview: released over workspace %d", target + 1);
        SwitchWorkspace(mon, target);
    }
}

bool WorkspaceOverviewOpen() {
    return osd.interactive && osd.hwnd != nullptr && IsWindowVisible(osd.hwnd) != FALSE;
}

bool OverviewDropTarget(POINT screen, int* mon, int* wsIndex) {
    if (!WorkspaceOverviewOpen()) return false;
    POINT pt = screen;
    ScreenToClient(osd.hwnd, &pt);
    const int col = OsdHitTest(pt).col;
    if (col != osd.hoverCol) {
        osd.hoverCol = col;
        OsdInvalidate();
    }
    if (col < 0) return false;
    if (mon) *mon = osd.mon;
    if (wsIndex) *wsIndex = col;
    return true;
}

void OverviewDropDone() {
    if (osd.hoverCol >= 0) {
        osd.hoverCol = -1;
        OsdInvalidate();
    }
}

void RefreshWorkspaceOverview() {
    if (WorkspaceOverviewOpen() && !OsdPressed()) OsdRender(osd.mon);
}

void DestroyOsd() {
    OsdAbandonPress();  // give the capture back before the window goes
    if (osd.hwnd && IsWindow(osd.hwnd)) {
        KillTimer(osd.hwnd, kOsdHideTimer);
        DestroyWindow(osd.hwnd);
    }
    // The class can only go once its last window is gone, hence after DestroyWindow.
    if (osd.classRegistered && !UnregisterClassW(kOsdWindowClass, osd.hinst)) {
        HT_LOG(L"OSD: class unregistration failed (error %lu)", GetLastError());
    }
    OsdDeleteFonts();
    for (const auto& entry : osdExeIcons) {
        if (entry.second.icon) DestroyIcon(entry.second.icon);
    }
    osdExeIcons.clear();
    osd = OsdState{};
}

}  // namespace ht

// =============================================================================
// 56_bar.cpp — taskbar workspace indicator: one small per-pixel-alpha window per
// monitor with a taskbar, drawn over the taskbar's left area as round workspace
// badges. Clicking a badge switches that monitor's workspace; the window never
// takes focus. Engine thread only; coordinates are physical pixels.
// =============================================================================

#include <gdiplus.h>
#include <shellscalingapi.h>
#include <utility>

namespace ht {

namespace {

// Geometry in 96-dpi units; every value goes through BarScale.
constexpr int kBarBadgeDip = 22;         // badge circle diameter
constexpr int kBarGapDip = 4;            // gap between badges
constexpr int kBarPadDip = 4;            // padding around the badge row
constexpr int kBarFontDip = 11;          // number font em size
constexpr int kBarDotDip = 8;            // notification dot diameter
constexpr int kBarDotRingDip = 3;        // ring: 2 * 1.5 dip wider than the dot
constexpr double kBarDotDiagonal = 0.7071;  // cos and sin of 45 degrees
constexpr int kBarMaxCount = 9;
constexpr int kBarMinVisibleHeight = 8;  // px; thinner visible taskbar = auto-hidden
constexpr int kBarMinVisibleWidth = 100; // px
constexpr BYTE kBarActiveAlpha = 255;
constexpr BYTE kBarOccupiedAlpha = 235;
constexpr BYTE kBarEmptyAlpha = 150;
constexpr wchar_t kBarFontFamily[] = L"Segoe UI";

struct BarWindow {
    HWND hwnd = nullptr;
    HMONITOR monitor = nullptr;
    int mon = -1;                 // index into g.monitors, refreshed every update
    std::vector<RECT> badges;     // client coords, one per workspace
    std::wstring signature;       // what the surface currently shows
    bool shown = false;
};

// A taskbar found on this update, with the monitor it belongs to.
struct BarTaskbar {
    RECT rect{};
    HMONITOR monitor = nullptr;
    int mon = -1;
};

// Everything one bar needs to be drawn for this update.
struct BarLayout {
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;
    int dpi = 96;
    int count = 1;
    int active = 0;
    std::vector<bool> occupied;
    std::vector<bool> attention;  // notification dot; never the active one
    std::vector<RECT> badges;     // client coords
};

std::vector<BarWindow> s_barWindows;
HINSTANCE s_barHinst = nullptr;
bool s_barClassRegistered = false;
bool s_barFailureLogged = false;  // failures are logged once per engine run
ULONG_PTR s_barGdiplusToken = 0;
bool s_barGdiplusStarted = false;

// Logs the first failure only, so the once-a-second refresh stays quiet.
bool BarFailed(const wchar_t* step, DWORD err) {
    if (!s_barFailureLogged) {
        s_barFailureLogged = true;
        HT_LOG(L"Taskbar indicator: %s failed (error %lu)", step, err);
    }
    return false;
}

int BarScale(int dip, int dpi) {
    return MulDiv(dip, dpi, 96);
}

int BarDpiOf(HMONITOR monitor) {
    UINT dpiX = 96;
    UINT dpiY = 96;
    if (FAILED(GetDpiForMonitor(monitor, MDT_EFFECTIVE_DPI, &dpiX, &dpiY)) ||
        dpiX == 0) {
        return 96;
    }
    return static_cast<int>(dpiX);
}

int BarMonitorIndex(HMONITOR monitor) {
    for (size_t i = 0; i < g.monitors.size(); ++i) {
        if (g.monitors[i].handle == monitor) return static_cast<int>(i);
    }
    return -1;
}

BarWindow* BarForMonitor(HMONITOR monitor) {
    for (BarWindow& bar : s_barWindows) {
        if (bar.monitor == monitor) return &bar;
    }
    return nullptr;
}

const BarTaskbar* BarTaskbarFor(const std::vector<BarTaskbar>& taskbars,
                                HMONITOR monitor) {
    for (const BarTaskbar& tb : taskbars) {
        if (tb.monitor == monitor) return &tb;
    }
    return nullptr;
}

std::wstring BarClassOf(HWND hwnd) {
    wchar_t buf[256];
    const int n = GetClassNameW(hwnd, buf, 256);
    return n > 0 ? std::wstring(buf, static_cast<size_t>(n)) : std::wstring();
}

bool BarIsTaskbarClass(const std::wstring& cls) {
    return cls == L"Shell_TrayWnd" || cls == L"Shell_SecondaryTrayWnd";
}

// Desktop and taskbar windows never count as a fullscreen app.
bool BarIsShellClass(const std::wstring& cls) {
    return BarIsTaskbarClass(cls) || cls == L"Progman" || cls == L"WorkerW";
}

// Start menu, search, quick settings and the tray overflow open above the
// taskbar; re-raising the bar while one of them is in front would draw it on
// top of the flyout.
bool BarShellFlyoutInFront() {
    const std::wstring cls = BarClassOf(GetForegroundWindow());
    return cls == L"Windows.UI.Core.CoreWindow" ||
           cls == L"XamlExplorerHostIslandWindow" ||
           cls == L"TopLevelWindowForOverflowXamlIsland" ||
           cls == L"NotifyIconOverflowWindow";
}

BOOL CALLBACK BarEnumTaskbarsProc(HWND hwnd, LPARAM lparam) {
    auto* found = reinterpret_cast<std::vector<BarTaskbar>*>(lparam);
    if (!IsWindowVisible(hwnd) || !BarIsTaskbarClass(BarClassOf(hwnd))) {
        return TRUE;
    }
    RECT rc{};
    if (!GetWindowRect(hwnd, &rc)) return TRUE;

    HMONITOR monitor = MonitorFromRect(&rc, MONITOR_DEFAULTTONEAREST);
    const int mon = BarMonitorIndex(monitor);
    if (mon < 0) return TRUE;
    for (const BarTaskbar& tb : *found) {
        if (tb.mon == mon) return TRUE;  // at most one bar per monitor
    }
    found->push_back(BarTaskbar{rc, monitor, mon});
    return TRUE;
}

// True when the foreground window is a fullscreen app on this monitor.
bool BarFullscreenOn(HMONITOR monitor, const RECT& rcMonitor) {
    HWND fg = GetForegroundWindow();
    if (!fg || !IsWindow(fg)) return false;
    if (MonitorFromWindow(fg, MONITOR_DEFAULTTONEAREST) != monitor) return false;
    if (BarIsShellClass(BarClassOf(fg))) return false;
    RECT rc{};
    if (!GetWindowRect(fg, &rc)) return false;
    return rc.left <= rcMonitor.left && rc.top <= rcMonitor.top &&
           rc.right >= rcMonitor.right && rc.bottom >= rcMonitor.bottom;
}

Gdiplus::Color BarArgb(BYTE alpha, COLORREF c) {
    return Gdiplus::Color(alpha, GetRValue(c), GetGValue(c), GetBValue(c));
}

bool BarEnsureGdiplus() {
    if (s_barGdiplusStarted) return true;
    Gdiplus::GdiplusStartupInput input;
    const Gdiplus::Status status =
        Gdiplus::GdiplusStartup(&s_barGdiplusToken, &input, nullptr);
    if (status != Gdiplus::Ok) {
        return BarFailed(L"GDI+ startup", static_cast<DWORD>(status));
    }
    s_barGdiplusStarted = true;
    return true;
}

LRESULT CALLBACK BarWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

// Registers the bar window class on first use.
bool BarEnsureClass() {
    if (s_barClassRegistered) return true;

    // Address of our own code identifies this module (the mod DLL).
    HINSTANCE hinst = nullptr;
    if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                                GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                            reinterpret_cast<LPCWSTR>(&BarWndProc), &hinst)) {
        return BarFailed(L"module lookup", GetLastError());
    }

    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = BarWndProc;
    wc.hInstance = hinst;
    wc.hCursor = LoadCursorW(nullptr, IDC_HAND);
    wc.lpszClassName = kBarWindowClass;  // no background: we paint all of it
    if (!RegisterClassExW(&wc)) {
        return BarFailed(L"class registration", GetLastError());
    }
    s_barHinst = hinst;
    s_barClassRegistered = true;
    return true;
}

HWND BarCreateWindow() {
    if (!BarEnsureClass()) return nullptr;
    const DWORD exStyle = WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW |
                          WS_EX_NOACTIVATE;
    HWND hwnd = CreateWindowExW(exStyle, kBarWindowClass, L"HawkTile taskbar",
                                WS_POPUP, 0, 0, 0, 0, nullptr, nullptr,
                                s_barHinst, nullptr);
    if (!hwnd) {
        BarFailed(L"window creation", GetLastError());
    }
    return hwnd;
}

void BarDestroy(BarWindow& bar) {
    if (bar.hwnd && IsWindow(bar.hwnd)) DestroyWindow(bar.hwnd);
    bar.hwnd = nullptr;
    bar.shown = false;
}

void BarHide(BarWindow& bar) {
    if (bar.shown) {
        ShowWindow(bar.hwnd, SW_HIDE);
        bar.shown = false;
    }
}

// Geometry for one bar: badge row sized for this monitor's DPI and workspace
// count, placed at the taskbar's left offset and centered on its visible part.
BarLayout BarComputeLayout(const BarTaskbar& tb, const Monitor& m, const RECT& visible) {
    BarLayout geo;
    geo.dpi = BarDpiOf(m.handle);
    geo.count = std::clamp(static_cast<int>(m.workspaces.size()), 1, kBarMaxCount);
    geo.active = std::clamp(m.active, 0, geo.count - 1);

    geo.occupied.assign(static_cast<size_t>(geo.count), false);
    const int withWorkspaces = std::min(geo.count, static_cast<int>(m.workspaces.size()));
    for (int i = 0; i < withWorkspaces; ++i) {
        const Workspace& ws = m.workspaces[static_cast<size_t>(i)];
        geo.occupied[static_cast<size_t>(i)] = !ws.tiled.empty() || !ws.floating.empty();
    }

    geo.attention.assign(static_cast<size_t>(geo.count), false);
    for (int i = 0; i < geo.count; ++i) {
        geo.attention[static_cast<size_t>(i)] =
            i != geo.active && WorkspaceHasAttention(tb.mon, i);
    }

    const int pad = BarScale(kBarPadDip, geo.dpi);
    const int badge = BarScale(kBarBadgeDip, geo.dpi);
    const int gap = BarScale(kBarGapDip, geo.dpi);
    geo.width = 2 * pad + geo.count * badge + (geo.count - 1) * gap;
    geo.height = 2 * pad + badge;

    geo.x = tb.rect.left + BarScale(g.settings.taskbarIndicatorOffset, geo.dpi);
    const int visibleHeight = visible.bottom - visible.top;
    geo.y = visible.top + (visibleHeight - geo.height) / 2;

    geo.badges.reserve(static_cast<size_t>(geo.count));
    for (int i = 0; i < geo.count; ++i) {
        const int left = pad + i * (badge + gap);
        geo.badges.push_back(RECT{left, pad, left + badge, pad + badge});
    }
    return geo;
}

// Everything that changes what the surface looks like, so an unchanged value
// means the existing surface is still correct.
std::wstring BarSignature(const BarLayout& geo, const Theme& theme) {
    std::wstring sig;
    auto put = [&sig](long long v) {
        sig += std::to_wstring(v);
        sig += L';';
    };
    put(geo.x);
    put(geo.y);
    put(geo.width);
    put(geo.height);
    put(geo.dpi);
    put(geo.count);
    put(geo.active);
    for (bool occupied : geo.occupied) sig += occupied ? L'1' : L'0';
    sig += L';';
    for (bool attention : geo.attention) sig += attention ? L'1' : L'0';
    sig += L';';
    put(theme.background);
    put(theme.activeFill);
    put(theme.activeText);
    put(theme.occupiedFill);
    put(theme.occupiedText);
    put(theme.emptyFill);
    put(theme.emptyText);
    return sig;
}

// Notification dot: a ring in the background color with the dot in the active
// fill on top, centered 45 degrees toward the badge's top-right.
void BarDrawDot(Gdiplus::Graphics& gr, const RECT& badge, int dpi, const Theme& theme) {
    const double radius = (badge.right - badge.left) / 2.0;
    const double cx = (badge.left + badge.right) / 2.0 + radius * kBarDotDiagonal;
    const double cy = (badge.top + badge.bottom) / 2.0 - radius * kBarDotDiagonal;
    const int dot = BarScale(kBarDotDip, dpi);
    const int ring = dot + std::max(2, BarScale(kBarDotRingDip, dpi));

    auto disc = [&gr, cx, cy](int diameter, COLORREF color) {
        const Gdiplus::RectF rect(static_cast<Gdiplus::REAL>(cx - diameter / 2.0),
                                  static_cast<Gdiplus::REAL>(cy - diameter / 2.0),
                                  static_cast<Gdiplus::REAL>(diameter),
                                  static_cast<Gdiplus::REAL>(diameter));
        Gdiplus::SolidBrush brush(BarArgb(255, color));
        gr.FillEllipse(&brush, rect);
    };
    disc(ring, theme.background);
    disc(dot, theme.activeFill);
}

// Paints the badges onto a transparent surface: active = accent fill and bold
// number, occupied = prominent fill and bold number, empty = dim, normal number.
void BarDrawBadges(Gdiplus::Graphics& gr, const BarLayout& geo, const Theme& theme) {
    const auto fontSize = static_cast<Gdiplus::REAL>(BarScale(kBarFontDip, geo.dpi));
    Gdiplus::Font boldFont(kBarFontFamily, fontSize, Gdiplus::FontStyleBold,
                           Gdiplus::UnitPixel);
    Gdiplus::Font regularFont(kBarFontFamily, fontSize, Gdiplus::FontStyleRegular,
                              Gdiplus::UnitPixel);
    Gdiplus::StringFormat format;
    format.SetAlignment(Gdiplus::StringAlignmentCenter);
    format.SetLineAlignment(Gdiplus::StringAlignmentCenter);

    for (int i = 0; i < geo.count; ++i) {
        const RECT& r = geo.badges[static_cast<size_t>(i)];
        const bool isActive = i == geo.active;
        const bool occupied = geo.occupied[static_cast<size_t>(i)];

        COLORREF fill = theme.emptyFill;
        COLORREF text = theme.emptyText;
        BYTE alpha = kBarEmptyAlpha;
        bool bold = false;
        if (isActive) {
            fill = theme.activeFill;
            text = theme.activeText;
            alpha = kBarActiveAlpha;
            bold = true;
        } else if (occupied) {
            fill = theme.occupiedFill;
            text = theme.occupiedText;
            alpha = kBarOccupiedAlpha;
            bold = true;
        }

        const Gdiplus::RectF rect(static_cast<Gdiplus::REAL>(r.left),
                                  static_cast<Gdiplus::REAL>(r.top),
                                  static_cast<Gdiplus::REAL>(r.right - r.left),
                                  static_cast<Gdiplus::REAL>(r.bottom - r.top));
        Gdiplus::SolidBrush fillBrush(BarArgb(alpha, fill));
        gr.FillEllipse(&fillBrush, rect);

        // At most 9 workspaces, so the number is always a single digit.
        const wchar_t label[2] = {static_cast<wchar_t>(L'1' + i), L'\0'};
        Gdiplus::SolidBrush textBrush(BarArgb(255, text));
        gr.DrawString(label, 1, bold ? &boldFont : &regularFont, rect, &format,
                      &textBrush);
        if (geo.attention[static_cast<size_t>(i)]) BarDrawDot(gr, r, geo.dpi, theme);
    }
}

// Draws the bar into a fresh top-down 32-bpp DIB and presents it with
// UpdateLayeredWindow. Returns false (window untouched) on any failure.
bool BarRender(HWND hwnd, const BarLayout& geo, const Theme& theme) {
    HDC screen = GetDC(nullptr);
    if (!screen) return BarFailed(L"screen DC", GetLastError());
    HDC mem = CreateCompatibleDC(screen);
    if (!mem) {
        const DWORD err = GetLastError();
        ReleaseDC(nullptr, screen);
        return BarFailed(L"memory DC", err);
    }

    BITMAPINFO info{};
    info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    info.bmiHeader.biWidth = geo.width;
    info.bmiHeader.biHeight = -geo.height;  // negative = top-down rows
    info.bmiHeader.biPlanes = 1;
    info.bmiHeader.biBitCount = 32;
    info.bmiHeader.biCompression = BI_RGB;

    void* bits = nullptr;
    HBITMAP dib = CreateDIBSection(screen, &info, DIB_RGB_COLORS, &bits, nullptr, 0);
    if (!dib || !bits) {
        const DWORD err = GetLastError();
        if (dib) DeleteObject(dib);
        DeleteDC(mem);
        ReleaseDC(nullptr, screen);
        return BarFailed(L"surface", err);
    }
    const HGDIOBJ oldBitmap = SelectObject(mem, dib);

    bool drawn = false;
    {
        // GDI+ writes straight into the DIB; the scope ends the drawing before
        // the surface is presented.
        Gdiplus::Bitmap surface(geo.width, geo.height, geo.width * 4,
                                PixelFormat32bppPARGB,
                                static_cast<BYTE*>(bits));
        if (surface.GetLastStatus() == Gdiplus::Ok) {
            Gdiplus::Graphics gr(&surface);
            gr.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
            gr.SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAliasGridFit);
            gr.Clear(Gdiplus::Color(0, 0, 0, 0));
            BarDrawBadges(gr, geo, theme);
            drawn = true;
        }
    }

    bool ok = false;
    DWORD err = 0;
    if (drawn) {
        POINT dst{geo.x, geo.y};
        SIZE size{geo.width, geo.height};
        POINT src{0, 0};
        BLENDFUNCTION blend{AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
        ok = UpdateLayeredWindow(hwnd, screen, &dst, &size, mem, &src, 0, &blend,
                                 ULW_ALPHA) != FALSE;
        if (!ok) err = GetLastError();
    } else {
        err = ERROR_INVALID_DATA;
    }

    SelectObject(mem, oldBitmap);
    DeleteObject(dib);
    DeleteDC(mem);
    ReleaseDC(nullptr, screen);
    return ok ? true : BarFailed(drawn ? L"UpdateLayeredWindow" : L"GDI+ surface", err);
}

// Switches to the workspace whose badge was clicked, if the click hit one.
bool BarHandleClick(HWND hwnd, LPARAM lp) {
    BarWindow* bar = nullptr;
    for (BarWindow& candidate : s_barWindows) {
        if (candidate.hwnd == hwnd) {
            bar = &candidate;
            break;
        }
    }
    if (!bar) return false;

    const double px = static_cast<short>(LOWORD(lp));
    const double py = static_cast<short>(HIWORD(lp));
    for (size_t i = 0; i < bar->badges.size(); ++i) {
        const RECT& r = bar->badges[i];
        const double cx = (r.left + r.right) / 2.0;
        const double cy = (r.top + r.bottom) / 2.0;
        const double radius = (r.right - r.left) / 2.0;
        const double dx = px - cx;
        const double dy = py - cy;
        if (dx * dx + dy * dy > radius * radius) continue;

        // Copy out before SwitchWorkspace, which may run arbitrary engine code.
        const int mon = bar->mon;
        const int wsIndex = static_cast<int>(i);
        if (mon < 0 || mon >= static_cast<int>(g.monitors.size())) return true;
        if (wsIndex >= static_cast<int>(g.monitors[static_cast<size_t>(mon)].workspaces.size())) {
            return true;
        }
        SwitchWorkspace(mon, wsIndex);
        return true;
    }
    return false;
}

LRESULT CALLBACK BarWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
        case WM_MOUSEACTIVATE:
            return MA_NOACTIVATE;
        case WM_NCHITTEST:
            return HTCLIENT;
        case WM_LBUTTONUP:
            if (BarHandleClick(hwnd, lp)) return 0;
            break;
        case WM_ERASEBKGND:
            return 1;  // BarRender covers the whole surface
        default:
            break;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

}  // namespace

void RequestBarUpdate() {
    if (g.engineHwnd) SetTimer(g.engineHwnd, kTimerBar, 30, nullptr);
}

void UpdateTaskbarIndicators() {
    if (!g.settings.taskbarIndicator || g.monitors.empty()) {
        for (BarWindow& bar : s_barWindows) BarHide(bar);
        return;
    }
    if (!BarEnsureGdiplus()) {
        for (BarWindow& bar : s_barWindows) BarHide(bar);
        return;
    }

    std::vector<BarTaskbar> taskbars;
    EnumWindows(BarEnumTaskbarsProc, reinterpret_cast<LPARAM>(&taskbars));

    // Drop bars whose monitor has no taskbar any more (or whose window vanished).
    for (auto it = s_barWindows.begin(); it != s_barWindows.end();) {
        const bool stillNeeded = BarTaskbarFor(taskbars, it->monitor) != nullptr &&
                                 it->hwnd && IsWindow(it->hwnd);
        if (stillNeeded) {
            ++it;
            continue;
        }
        BarDestroy(*it);
        it = s_barWindows.erase(it);
    }

    // Create a bar for each taskbar that has none yet.
    for (const BarTaskbar& tb : taskbars) {
        if (BarForMonitor(tb.monitor)) continue;
        HWND hwnd = BarCreateWindow();
        if (!hwnd) continue;
        BarWindow bar;
        bar.hwnd = hwnd;
        bar.monitor = tb.monitor;
        s_barWindows.push_back(std::move(bar));
    }

    const Theme theme = ResolveTheme();
    const bool mayRaise = !BarShellFlyoutInFront();
    for (BarWindow& bar : s_barWindows) {
        const BarTaskbar* tb = BarTaskbarFor(taskbars, bar.monitor);
        if (!tb) {
            BarHide(bar);
            continue;
        }
        bar.mon = tb->mon;
        const Monitor& m = g.monitors[static_cast<size_t>(tb->mon)];

        // The part of the taskbar that is on screen. An auto-hidden taskbar
        // leaves only a thin sliver, and a fullscreen app covers the bar.
        RECT visible{};
        const bool intersects = IntersectRect(&visible, &tb->rect, &m.rcMonitor) != FALSE;
        const bool autoHidden = !intersects ||
                                visible.bottom - visible.top < kBarMinVisibleHeight ||
                                visible.right - visible.left < kBarMinVisibleWidth;
        if (autoHidden || BarFullscreenOn(bar.monitor, m.rcMonitor)) {
            BarHide(bar);
            continue;
        }

        const BarLayout geo = BarComputeLayout(*tb, m, visible);
        bar.badges = geo.badges;
        const std::wstring sig = BarSignature(geo, theme);
        if (sig != bar.signature) {
            if (!BarRender(bar.hwnd, geo, theme)) {
                BarHide(bar);
                continue;  // signature stays stale, so the next update retries
            }
            bar.signature = sig;
        }

        if (!bar.shown) {
            ShowWindow(bar.hwnd, SW_SHOWNOACTIVATE);
            bar.shown = true;
        }
        // Clicking the taskbar can drop the bar below it; raise it again.
        if (mayRaise) {
            SetWindowPos(bar.hwnd, HWND_TOPMOST, 0, 0, 0, 0,
                         SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
        }
    }
}

void DestroyTaskbarIndicators() {
    for (BarWindow& bar : s_barWindows) BarDestroy(bar);
    s_barWindows.clear();

    // The class can only go once its last window is gone, hence after DestroyWindow.
    if (s_barClassRegistered &&
        !UnregisterClassW(kBarWindowClass, s_barHinst)) {
        HT_LOG(L"Taskbar indicator: class unregistration failed (error %lu)",
               GetLastError());
    }
    if (s_barGdiplusStarted) Gdiplus::GdiplusShutdown(s_barGdiplusToken);

    s_barHinst = nullptr;
    s_barClassRegistered = false;
    s_barFailureLogged = false;
    s_barGdiplusToken = 0;
    s_barGdiplusStarted = false;
}

}  // namespace ht

// =============================================================================
// 57_stage.cpp — Mainstage side tiles: one small window per side window, showing
// a live, scaled-down DWM thumbnail of it. Semantics: 10_contract.h.
// =============================================================================

namespace ht {

namespace {

constexpr DWORD kStageCornerAttr = 33;  // DWMWA_WINDOW_CORNER_PREFERENCE
constexpr DWORD kStageCornerRound = 2;  // DWMWCP_ROUND
constexpr UINT kStageRefitDelayMs = 250;
constexpr UINT_PTR kStageHoverTimer = 1;  // on a tile window, while hovered
constexpr UINT kStageHoverPollMs = 15;
constexpr ULONGLONG kStageZoomHoldMs = 500;

struct StageProxy {
    HWND source = nullptr;        // the real window, stacked on the stage
    HWND hwnd = nullptr;          // our tile window
    HTHUMBNAIL thumb = nullptr;   // null when DWM refused the thumbnail
    RECT cell{};                  // where the tile is, in screen pixels
    RECT drawn{};                 // the thumbnail inside the tile, client pixels
};

struct StageState {
    HINSTANCE hinst = nullptr;
    bool classRegistered = false;
    bool failureLogged = false;
    COLORREF background = RGB(32, 32, 32);
    std::vector<StageProxy> tiles;
    // Left button held on a tile: a click until the cursor leaves the drag
    // threshold, then a drag of the tile.
    HWND pressTile = nullptr;
    POINT pressCursor{};          // screen position at button down
    bool pressDouble = false;     // the press was the second of a double click
    bool dragging = false;
    // Left Alt held over a tile: after kStageZoomHoldMs the tile shows its
    // window at full size. The cursor is then held in place and mouse movement
    // drags the picture under it.
    HWND hoverTile = nullptr;     // the tile running kStageHoverTimer
    ULONGLONG altSince = 0;       // tick when Alt was first seen down; 0 = up
    HWND zoomTile = nullptr;      // the zoomed tile
    POINT zoomAt{};               // where the cursor is held, tile client pixels
    POINT zoomLock{};             // the same point on screen
    double zoomU = 0.0;           // the point of the window shown at zoomAt, as
    double zoomV = 0.0;           // fractions of its visible frame
};

StageState stage;

StageProxy* StageFindByTile(HWND hwnd) {
    for (StageProxy& tile : stage.tiles) {
        if (tile.hwnd == hwnd) return &tile;
    }
    return nullptr;
}

StageProxy* StageFindBySource(HWND source) {
    for (StageProxy& tile : stage.tiles) {
        if (tile.source == source) return &tile;
    }
    return nullptr;
}

// Maps a point of the tile (client pixels) through the thumbnail's scale to
// the matching screen point of the real window. False outside the thumbnail.
bool StageMapToSource(const StageProxy& tile, POINT client, POINT* screen) {
    const LONG drawnW = tile.drawn.right - tile.drawn.left;
    const LONG drawnH = tile.drawn.bottom - tile.drawn.top;
    if (drawnW <= 0 || drawnH <= 0 || !PtInRect(&tile.drawn, client) ||
        !IsWindow(tile.source)) {
        return false;
    }
    const RECT visible = GetVisibleFrame(tile.source);
    if (stage.zoomTile == tile.hwnd) {
        // Zoomed: full size around the held cursor point.
        screen->x = visible.left + (client.x - stage.zoomAt.x) +
                    std::lround(stage.zoomU * (visible.right - visible.left));
        screen->y = visible.top + (client.y - stage.zoomAt.y) +
                    std::lround(stage.zoomV * (visible.bottom - visible.top));
        return PtInRect(&visible, *screen) != FALSE;
    }
    screen->x = visible.left + MulDiv(client.x - tile.drawn.left,
                                      visible.right - visible.left, drawnW);
    screen->y = visible.top + MulDiv(client.y - tile.drawn.top,
                                     visible.bottom - visible.top, drawnH);
    return true;
}

// The innermost child window of `root` at a screen point. WindowFromPoint is no
// use here: the real window is covered by the stage window.
HWND StageDeepestChild(HWND root, POINT screen) {
    HWND current = root;
    for (int depth = 0; depth < 32; ++depth) {
        POINT pt = screen;
        ScreenToClient(current, &pt);
        const HWND child = ChildWindowFromPointEx(
            current, pt,
            CWP_SKIPINVISIBLE | CWP_SKIPDISABLED | CWP_SKIPTRANSPARENT);
        if (!child || child == current) break;
        current = child;
    }
    return current;
}

// The window that should get a forwarded mouse message for tile point `client`,
// and the point in its client coordinates and in screen coordinates. Null
// outside the thumbnail and outside that window's client area.
HWND StageForwardTarget(const StageProxy& tile, POINT client, POINT* inTarget,
                        POINT* screen) {
    if (!StageMapToSource(tile, client, screen)) return nullptr;
    const HWND target = StageDeepestChild(tile.source, *screen);
    *inTarget = *screen;
    ScreenToClient(target, inTarget);
    RECT area{};
    GetClientRect(target, &area);
    return PtInRect(&area, *inTarget) ? target : nullptr;
}

// Posts one click (move, down, up) into the real window. Posted, not sent: a
// hung app must not block the engine.
void StageForwardClick(const StageProxy& tile, POINT client, UINT down, UINT up,
                       WPARAM button) {
    POINT inTarget{};
    POINT screen{};
    const HWND target = StageForwardTarget(tile, client, &inTarget, &screen);
    if (!target) {
        HT_LOG(L"tile click at %ld,%ld not forwarded (thumbnail %ld,%ld-%ld,%ld)",
               client.x, client.y, tile.drawn.left, tile.drawn.top,
               tile.drawn.right, tile.drawn.bottom);
        return;
    }
    WCHAR cls[64] = {};
    GetClassNameW(target, cls, 64);
    HT_LOG(L"tile click forwarded to %p (%s%s) at %ld,%ld",
           static_cast<void*>(target), cls,
           target == tile.source ? L", the window itself" : L", a child",
           inTarget.x, inTarget.y);
    const LPARAM where = MAKELPARAM(inTarget.x, inTarget.y);
    PostMessageW(target, WM_MOUSEMOVE, 0, where);
    PostMessageW(target, down, button, where);
    PostMessageW(target, up, 0, where);
}

POINT StageClientPoint(LPARAM lp) {
    return POINT{static_cast<short>(LOWORD(lp)), static_cast<short>(HIWORD(lp))};
}

// Puts a dragged tile back in its cell, behind the other windows.
void StageRestoreTile(const StageProxy& tile) {
    SetWindowPos(tile.hwnd, HWND_NOTOPMOST, 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    SetWindowPos(tile.hwnd, HWND_BOTTOM, tile.cell.left, tile.cell.top,
                 tile.cell.right - tile.cell.left,
                 tile.cell.bottom - tile.cell.top, SWP_NOACTIVATE);
}

void StageFitThumbnail(StageProxy& tile);

void StageEndZoom() {
    const HWND before = stage.zoomTile;
    if (!before) return;
    stage.zoomTile = nullptr;
    if (StageProxy* tile = StageFindByTile(before)) StageFitThumbnail(*tile);
}

// kStageHoverTimer: runs while the cursor is over the tile and decides whether
// it is zoomed. Left Alt only, like the overview.
void StageHoverTick(HWND hwnd) {
    POINT cursor{};
    GetCursorPos(&cursor);
    const bool over = WindowFromPoint(cursor) == hwnd ||
                      (stage.pressTile == hwnd && !stage.dragging);
    if (!over || stage.dragging) {
        KillTimer(hwnd, kStageHoverTimer);
        if (stage.hoverTile == hwnd) stage.hoverTile = nullptr;
        stage.altSince = 0;
        if (stage.zoomTile == hwnd) StageEndZoom();
        return;
    }
    if (GetAsyncKeyState(VK_LMENU) >= 0) {
        stage.altSince = 0;
        if (stage.zoomTile == hwnd) StageEndZoom();
        return;
    }
    const ULONGLONG now = GetTickCount64();
    if (stage.altSince == 0) stage.altSince = now;
    if (now - stage.altSince < kStageZoomHoldMs) return;
    StageProxy* tile = StageFindByTile(hwnd);
    if (!tile) return;

    if (stage.zoomTile != hwnd) {
        // Start: the point under the cursor is the one shown at full size.
        const LONG drawnW = tile->drawn.right - tile->drawn.left;
        const LONG drawnH = tile->drawn.bottom - tile->drawn.top;
        if (drawnW <= 0 || drawnH <= 0) return;
        POINT client = cursor;
        ScreenToClient(hwnd, &client);
        client.x = std::clamp<LONG>(client.x, tile->drawn.left, tile->drawn.right);
        client.y = std::clamp<LONG>(client.y, tile->drawn.top, tile->drawn.bottom);
        StageEndZoom();
        stage.zoomTile = hwnd;
        stage.zoomAt = client;
        stage.zoomLock = client;
        ClientToScreen(hwnd, &stage.zoomLock);
        stage.zoomU = static_cast<double>(client.x - tile->drawn.left) / drawnW;
        stage.zoomV = static_cast<double>(client.y - tile->drawn.top) / drawnH;
        SetCursorPos(stage.zoomLock.x, stage.zoomLock.y);
        StageFitThumbnail(*tile);
        return;
    }
    // A held button is a click or the start of a tile drag: leave the cursor.
    if (stage.pressTile == hwnd) return;
    const int dx = cursor.x - stage.zoomLock.x;
    const int dy = cursor.y - stage.zoomLock.y;
    if (dx == 0 && dy == 0) return;
    // The picture follows the mouse; the cursor goes back to where it was.
    const RECT visible = GetVisibleFrame(tile->source);
    const LONG visibleW = std::max<LONG>(1, visible.right - visible.left);
    const LONG visibleH = std::max<LONG>(1, visible.bottom - visible.top);
    stage.zoomU = std::clamp(stage.zoomU - static_cast<double>(dx) / visibleW,
                             0.0, 1.0);
    stage.zoomV = std::clamp(stage.zoomV - static_cast<double>(dy) / visibleH,
                             0.0, 1.0);
    SetCursorPos(stage.zoomLock.x, stage.zoomLock.y);
    StageFitThumbnail(*tile);
}

LRESULT CALLBACK StageWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
        case WM_MOUSEACTIVATE:
            return MA_NOACTIVATE;

        case WM_TIMER:
            if (wp == kStageHoverTimer) StageHoverTick(hwnd);
            return 0;

        case WM_LBUTTONDOWN:
        case WM_LBUTTONDBLCLK:
            stage.pressTile = hwnd;
            stage.pressDouble = msg == WM_LBUTTONDBLCLK;
            stage.dragging = false;
            GetCursorPos(&stage.pressCursor);
            SetCapture(hwnd);
            return 0;

        case WM_MOUSEMOVE: {
            if (stage.hoverTile != hwnd) {
                if (stage.hoverTile) KillTimer(stage.hoverTile, kStageHoverTimer);
                StageEndZoom();
                stage.hoverTile = hwnd;
                stage.altSince = 0;
                SetTimer(hwnd, kStageHoverTimer, kStageHoverPollMs, nullptr);
            }
            if (stage.pressTile != hwnd || GetCapture() != hwnd) return 0;
            const StageProxy* tile = StageFindByTile(hwnd);
            if (!tile) return 0;
            POINT cursor{};
            GetCursorPos(&cursor);
            const int dx = cursor.x - stage.pressCursor.x;
            const int dy = cursor.y - stage.pressCursor.y;
            if (!stage.dragging) {
                // Generous, so a click with an unsteady hand stays a click.
                const int limitX = 2 * GetSystemMetrics(SM_CXDRAG);
                const int limitY = 2 * GetSystemMetrics(SM_CYDRAG);
                if (dx >= -limitX && dx <= limitX && dy >= -limitY &&
                    dy <= limitY) {
                    return 0;
                }
                stage.dragging = true;
                HT_LOG(L"tile drag started");
                StageEndZoom();
                // Above the other windows while it is carried. Only once: an
                // overview that opens later (Alt) must stay above the tile.
                SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0,
                             SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
            }
            SetWindowPos(hwnd, nullptr, tile->cell.left + dx, tile->cell.top + dy,
                         0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
            OverviewDropTarget(cursor, nullptr, nullptr);  // moves the highlight
            return 0;
        }

        case WM_LBUTTONUP: {
            if (stage.pressTile != hwnd) return 0;
            const bool dragged = stage.dragging;
            const bool second = stage.pressDouble;
            // Cleared first: ReleaseCapture sends WM_CAPTURECHANGED.
            stage.pressTile = nullptr;
            stage.dragging = false;
            ReleaseCapture();
            const StageProxy* tile = StageFindByTile(hwnd);
            if (!tile) return 0;
            if (dragged) {
                // Copy: the drop retiles, which may destroy this tile.
                const HWND source = tile->source;
                StageRestoreTile(*tile);
                POINT cursor{};
                GetCursorPos(&cursor);
                int mon = -1;
                int wsIndex = -1;
                if (OverviewDropTarget(cursor, &mon, &wsIndex)) {
                    HT_LOG(L"tile dropped on workspace %d", wsIndex + 1);
                    OverviewDropDone();
                    MoveWindowToWorkspace(source, mon, wsIndex);
                } else {
                    StageDropWindow(source, cursor);
                }
            } else {
                StageForwardClick(*tile, StageClientPoint(lp),
                                  second ? WM_LBUTTONDBLCLK : WM_LBUTTONDOWN,
                                  WM_LBUTTONUP, MK_LBUTTON);
            }
            return 0;
        }

        case WM_CAPTURECHANGED:
            // Capture taken away mid-press: the drag is off.
            if (stage.pressTile == hwnd) {
                const bool dragged = stage.dragging;
                stage.pressTile = nullptr;
                stage.dragging = false;
                if (dragged) {
                    OverviewDropDone();
                    if (const StageProxy* tile = StageFindByTile(hwnd)) {
                        StageRestoreTile(*tile);
                    }
                }
            }
            return 0;

        case WM_RBUTTONUP:
            if (const StageProxy* tile = StageFindByTile(hwnd)) {
                StageForwardClick(*tile, StageClientPoint(lp), WM_RBUTTONDOWN,
                                  WM_RBUTTONUP, MK_RBUTTON);
            }
            return 0;

        case WM_MBUTTONUP:
            if (const StageProxy* tile = StageFindByTile(hwnd)) {
                StageForwardClick(*tile, StageClientPoint(lp), WM_MBUTTONDOWN,
                                  WM_MBUTTONUP, MK_MBUTTON);
            }
            return 0;

        case WM_MOUSEWHEEL:
        case WM_MOUSEHWHEEL:
            if (const StageProxy* tile = StageFindByTile(hwnd)) {
                // Wheel messages carry screen coordinates, in and out.
                POINT client = StageClientPoint(lp);
                ScreenToClient(hwnd, &client);
                POINT inTarget{};
                POINT screen{};
                if (const HWND target =
                        StageForwardTarget(*tile, client, &inTarget, &screen)) {
                    PostMessageW(target, msg, wp, MAKELPARAM(screen.x, screen.y));
                }
            }
            return 0;

        case WM_ERASEBKGND: {
            RECT client{};
            GetClientRect(hwnd, &client);
            const HBRUSH brush = CreateSolidBrush(stage.background);
            FillRect(reinterpret_cast<HDC>(wp), &client, brush);
            DeleteObject(brush);
            return 1;
        }

        default:
            break;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

// Logs the first creation failure only; later calls return quietly.
bool StageFailed(const wchar_t* step, DWORD err) {
    if (!stage.failureLogged) {
        stage.failureLogged = true;
        HT_LOG(L"stage tiles: %s failed (error %lu)", step, err);
    }
    return false;
}

bool StageEnsureClass() {
    if (stage.classRegistered) return true;
    // Address of our own code identifies this module (the mod DLL).
    HINSTANCE hinst = nullptr;
    if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                                GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                            reinterpret_cast<LPCWSTR>(&StageWndProc), &hinst)) {
        return StageFailed(L"module lookup", GetLastError());
    }
    stage.hinst = hinst;

    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.style = CS_DBLCLKS;  // double clicks are forwarded as such
    wc.lpfnWndProc = StageWndProc;
    wc.hInstance = stage.hinst;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.lpszClassName = kStageWindowClass;  // no background: WM_ERASEBKGND paints
    if (!RegisterClassExW(&wc)) {
        return StageFailed(L"class registration", GetLastError());
    }
    stage.classRegistered = true;
    return true;
}

// Scales the thumbnail to fit the tile, centred, never larger than the window
// itself. The source is cropped to the visible frame (no resize borders).
// The zoomed tile instead shows the window at full size, with the point
// (stage.zoomU, stage.zoomV) of it at stage.zoomAt.
void StageFitThumbnail(StageProxy& tile) {
    if (!tile.thumb || !IsWindow(tile.source)) return;
    RECT window{};
    SIZE source{};
    if (!GetWindowRect(tile.source, &window) ||
        FAILED(DwmQueryThumbnailSourceSize(tile.thumb, &source))) {
        return;
    }
    const LONG windowW = window.right - window.left;
    const LONG windowH = window.bottom - window.top;
    if (windowW <= 0 || windowH <= 0 || source.cx <= 0 || source.cy <= 0) return;

    // DWM measures the source in its own pixels, which differ from ours for a
    // DPI-virtualised app; map the visible frame through that factor.
    const RECT visible = GetVisibleFrame(tile.source);
    const double fx = static_cast<double>(source.cx) / windowW;
    const double fy = static_cast<double>(source.cy) / windowH;
    RECT crop{std::lround((visible.left - window.left) * fx),
              std::lround((visible.top - window.top) * fy),
              std::lround((visible.right - window.left) * fx),
              std::lround((visible.bottom - window.top) * fy)};
    crop.left = std::clamp<LONG>(crop.left, 0, source.cx - 1);
    crop.top = std::clamp<LONG>(crop.top, 0, source.cy - 1);
    crop.right = std::clamp<LONG>(crop.right, crop.left + 1, source.cx);
    crop.bottom = std::clamp<LONG>(crop.bottom, crop.top + 1, source.cy);

    const LONG cellW = tile.cell.right - tile.cell.left;
    const LONG cellH = tile.cell.bottom - tile.cell.top;
    const LONG visibleW = std::max<LONG>(1, visible.right - visible.left);
    const LONG visibleH = std::max<LONG>(1, visible.bottom - visible.top);
    const double scale =
        std::min({1.0, static_cast<double>(cellW) / visibleW,
                  static_cast<double>(cellH) / visibleH});
    const LONG drawW = std::clamp<LONG>(std::lround(visibleW * scale), 1, cellW);
    const LONG drawH = std::clamp<LONG>(std::lround(visibleH * scale), 1, cellH);
    const LONG x = (cellW - drawW) / 2;
    const LONG y = (cellH - drawH) / 2;

    DWM_THUMBNAIL_PROPERTIES props{};
    props.dwFlags = DWM_TNP_RECTDESTINATION | DWM_TNP_RECTSOURCE |
                    DWM_TNP_VISIBLE | DWM_TNP_OPACITY |
                    DWM_TNP_SOURCECLIENTAREAONLY;
    tile.drawn = RECT{x, y, x + drawW, y + drawH};
    props.rcDestination = tile.drawn;
    props.rcSource = crop;
    if (stage.zoomTile == tile.hwnd && scale < 1.0) {
        // tile.drawn stays the fitted rect; StageMapToSource knows the zoom.
        const LONG left = stage.zoomAt.x - std::lround(stage.zoomU * visibleW);
        const LONG top = stage.zoomAt.y - std::lround(stage.zoomV * visibleH);
        // Only the part that lands inside the tile, and its part of the source.
        RECT dest{std::max<LONG>(left, 0), std::max<LONG>(top, 0),
                  std::min<LONG>(left + visibleW, cellW),
                  std::min<LONG>(top + visibleH, cellH)};
        if (dest.right > dest.left && dest.bottom > dest.top) {
            const double sx = static_cast<double>(crop.right - crop.left) / visibleW;
            const double sy = static_cast<double>(crop.bottom - crop.top) / visibleH;
            RECT part{crop.left + std::lround((dest.left - left) * sx),
                      crop.top + std::lround((dest.top - top) * sy),
                      crop.left + std::lround((dest.right - left) * sx),
                      crop.top + std::lround((dest.bottom - top) * sy)};
            part.right = std::clamp<LONG>(part.right, part.left + 1, crop.right);
            part.bottom = std::clamp<LONG>(part.bottom, part.top + 1, crop.bottom);
            props.rcDestination = dest;
            props.rcSource = part;
        }
    }
    props.opacity = 255;
    props.fVisible = TRUE;
    props.fSourceClientAreaOnly = FALSE;
    DwmUpdateThumbnailProperties(tile.thumb, &props);
}

void StageDestroyTile(StageProxy& tile) {
    if (stage.hoverTile == tile.hwnd) stage.hoverTile = nullptr;
    if (stage.zoomTile == tile.hwnd) stage.zoomTile = nullptr;
    if (tile.thumb) DwmUnregisterThumbnail(tile.thumb);
    if (tile.hwnd && IsWindow(tile.hwnd)) DestroyWindow(tile.hwnd);
    tile = StageProxy{};
}

// Creates the tile window for `source` at `cell`, behind every other window.
bool StageCreateTile(HWND source, const RECT& cell, StageProxy* out) {
    if (!StageEnsureClass()) return false;
    HWND hwnd = CreateWindowExW(WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE,
                                kStageWindowClass, L"HawkTile stage tile",
                                WS_POPUP, cell.left, cell.top,
                                cell.right - cell.left, cell.bottom - cell.top,
                                nullptr, nullptr, stage.hinst, nullptr);
    if (!hwnd) return StageFailed(L"window creation", GetLastError());

    // Rounded corners on Windows 11; older systems ignore the attribute.
    DWORD corner = kStageCornerRound;
    DwmSetWindowAttribute(hwnd, kStageCornerAttr, &corner, sizeof(corner));

    StageProxy tile;
    tile.source = source;
    tile.hwnd = hwnd;
    tile.cell = cell;
    const HRESULT hr = DwmRegisterThumbnail(hwnd, source, &tile.thumb);
    if (FAILED(hr)) {
        // The tile still works as a button for its window, just without a picture.
        tile.thumb = nullptr;
        StageFailed(L"thumbnail registration", static_cast<DWORD>(hr));
    }
    SetWindowPos(hwnd, HWND_BOTTOM, 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW);
    *out = tile;
    return true;
}

}  // namespace

void SyncStageProxies() {
    const std::vector<StageTile> wanted = StageSideTiles();

    // Drop tiles that lost their cell or their window.
    for (StageProxy& tile : stage.tiles) {
        bool keep = IsWindow(tile.source) && IsWindow(tile.hwnd);
        if (keep) {
            keep = false;
            for (const StageTile& want : wanted) {
                if (want.hwnd == tile.source) keep = true;
            }
        }
        if (!keep) StageDestroyTile(tile);
    }
    std::erase_if(stage.tiles,
                  [](const StageProxy& tile) { return tile.hwnd == nullptr; });
    if (wanted.empty()) return;

    const COLORREF background = ResolveTheme().background;
    const bool repaint = background != stage.background;
    stage.background = background;

    for (const StageTile& want : wanted) {
        StageProxy* tile = StageFindBySource(want.hwnd);
        if (!tile) {
            StageProxy created;
            if (!StageCreateTile(want.hwnd, want.cell, &created)) continue;
            stage.tiles.push_back(created);
            tile = &stage.tiles.back();
        } else if (!EqualRect(&tile->cell, &want.cell)) {
            tile->cell = want.cell;
            SetWindowPos(tile->hwnd, nullptr, want.cell.left, want.cell.top,
                         want.cell.right - want.cell.left,
                         want.cell.bottom - want.cell.top,
                         SWP_NOZORDER | SWP_NOACTIVATE);
        }
        if (repaint) InvalidateRect(tile->hwnd, nullptr, TRUE);
        StageFitThumbnail(*tile);
    }
    // The real windows reach their stage size asynchronously; fit once more.
    if (g.engineHwnd) {
        SetTimer(g.engineHwnd, kTimerStage, kStageRefitDelayMs, nullptr);
    }
}

void RefitStageProxies() {
    for (StageProxy& tile : stage.tiles) StageFitThumbnail(tile);
}

void DestroyStageProxies() {
    for (StageProxy& tile : stage.tiles) StageDestroyTile(tile);
    stage.tiles.clear();
    if (stage.classRegistered) {
        UnregisterClassW(kStageWindowClass, stage.hinst);
        stage.classRegistered = false;
    }
    stage.failureLogged = false;
}

}  // namespace ht

// =============================================================================
// 60_engine.cpp — engine thread, hidden engine window, WinEvent hooks and the
// lifecycle (start, settings reload, stop).
// =============================================================================

#ifndef EVENT_OBJECT_CLOAKED
#define EVENT_OBJECT_CLOAKED 0x8017
#endif
#ifndef EVENT_OBJECT_UNCLOAKED
#define EVENT_OBJECT_UNCLOAKED 0x8018
#endif
#ifndef DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2
#define DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 ((DPI_AWARENESS_CONTEXT)-4)
#endif
#ifndef HSHELL_FLASH
#define HSHELL_FLASH (0x0006 | 0x8000)  // HSHELL_REDRAW | HSHELL_HIGHBIT
#endif

namespace ht {

namespace {

constexpr DWORD kStartTimeoutMs = 10000;
constexpr DWORD kAbortWaitMs = 5000;
constexpr DWORD kStopTimeoutMs = 10000;
constexpr UINT kPendingDelayMs = 80;
constexpr UINT kDisplayDelayMs = 400;

HANDLE s_thread = nullptr;
DWORD s_threadId = 0;
HANDLE s_initEvent = nullptr;
bool s_initOk = false;
std::vector<HWINEVENTHOOK> s_hooks;
HINSTANCE s_hinst = nullptr;
UINT s_shellHookMsg = 0;  // "SHELLHOOK" registered message id

// Hold-left-Alt overview (see the rules at the end of 10_contract.h).
bool s_leftAltDown = false;  // left Alt is down, as seen by Raw Input
bool s_holdArmed = false;    // kTimerHold is pending for this Alt press
bool s_holdShown = false;    // this Alt press opened the overview

// Alt+Shift mouse resize (see the rules at the end of 10_contract.h).
bool s_resizePolling = false;  // kTimerMouseResize runs for the current chord
bool s_resizeGaveUp = false;   // this chord found no tiled window; wait for it
                               // to turn off before trying again
POINT s_resizeAnchor{};        // cursor position when the chord turned on

// Raw Input keyboard: generic desktop page (0x01), keyboard usage (0x06).
constexpr USHORT kRawUsagePage = 0x01;
constexpr USHORT kRawUsageKeyboard = 0x06;

struct EventRange {
    DWORD first;
    DWORD last;
};

// Narrow ranges only; hooking the whole 0x8000 block is far too noisy.
// EVENT_OBJECT_SHOW lies inside the DESTROY..HIDE range.
constexpr EventRange kEventRanges[] = {
    {EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND},
    {EVENT_SYSTEM_MOVESIZESTART, EVENT_SYSTEM_MOVESIZEEND},
    {EVENT_SYSTEM_MINIMIZESTART, EVENT_SYSTEM_MINIMIZEEND},
    {EVENT_OBJECT_DESTROY, EVENT_OBJECT_HIDE},
    {EVENT_OBJECT_LOCATIONCHANGE, EVENT_OBJECT_LOCATIONCHANGE},
    {EVENT_OBJECT_CLOAKED, EVENT_OBJECT_UNCLOAKED},
};

bool IsTopLevel(HWND hwnd) {
    return GetAncestor(hwnd, GA_ROOT) == hwnd;
}

void SchedulePending() {
    if (g.engineHwnd) {
        SetTimer(g.engineHwnd, kTimerPending, kPendingDelayMs, nullptr);
    }
}

// Runs the deferred checks queued by WinEventProc. Windows are only classified
// or revalidated here, after their state has had time to settle.
void ProcessPending() {
    bool changed = false;

    // Hidden windows that took the foreground (EVENT_SYSTEM_FOREGROUND). Only a
    // window still in the foreground after the delay is followed, so a window
    // that merely flashed past does not switch the workspace.
    std::vector<HWND> activations(g.pendingActivation.begin(),
                                  g.pendingActivation.end());
    g.pendingActivation.clear();
    for (HWND hwnd : activations) {
        const HWND fg = GetForegroundWindow();
        const HWND fgRoot = fg ? GetAncestor(fg, GA_ROOTOWNER) : nullptr;
        if (Locate(hwnd, nullptr) && g.hiddenByUs.count(hwnd) > 0 &&
            (fg == hwnd || fgRoot == hwnd)) {
            ActivateWindowWorkspace(hwnd);
            changed = true;
        }
    }

    std::vector<HWND> evals(g.pendingEval.begin(), g.pendingEval.end());
    g.pendingEval.clear();
    for (HWND hwnd : evals) {
        WinLoc loc;
        if (!IsWindow(hwnd) || Locate(hwnd, &loc)) {
            continue;
        }
        const Eligibility eligibility = ClassifyWindow(hwnd, g.settings);
        if (eligibility == Eligibility::Ignore) {
            continue;
        }
        if (ManageWindow(hwnd, eligibility == Eligibility::Float)) {
            changed = true;
        }
        if (hwnd == GetForegroundWindow()) {
            OnFocusChanged(hwnd);
        }
    }

    std::vector<HWND> visibility(g.pendingVisibility.begin(),
                                 g.pendingVisibility.end());
    g.pendingVisibility.clear();
    for (HWND hwnd : visibility) {
        WinLoc loc;
        if (!Locate(hwnd, &loc)) {
            continue;
        }
        if (!IsWindow(hwnd)) {
            UnmanageWindow(hwnd);
            changed = true;
        } else if (g.hiddenByUs.count(hwnd) > 0) {
            if (IsWindowVisible(hwnd)) {
                if (g.settings.followActivation &&
                    loc.ws != g.monitors[loc.mon].active) {
                    // Follow it to its own workspace. It stays in g.hiddenByUs
                    // until SwitchWorkspace shows it.
                    ActivateWindowWorkspace(hwnd);
                } else {
                    // The app showed a window we parked on an inactive workspace.
                    ShowForWorkspace(hwnd);
                    MoveWindowToWorkspace(hwnd, loc.mon,
                                          g.monitors[loc.mon].active);
                    HT_LOG(L"window %p shown by its app, moved to active workspace",
                           static_cast<void*>(hwnd));
                }
                changed = true;
            }
        } else if (!IsWindowVisible(hwnd) || IsWindowCloaked(hwnd)) {
            UnmanageWindow(hwnd);
            changed = true;
        }
    }

    if (changed) {
        RequestRetile();
        UpdateFocusBorder();
    }
}

void CALLBACK WinEventProc(HWINEVENTHOOK, DWORD event, HWND hwnd, LONG idObject,
                           LONG idChild, DWORD, DWORD) {
    if (!hwnd || idObject != OBJID_WINDOW || idChild != CHILDID_SELF ||
        hwnd == g.engineHwnd) {
        return;
    }

    WinLoc loc;
    const bool managed = Locate(hwnd, &loc);
    const bool hiddenByUs = g.hiddenByUs.count(hwnd) > 0;

    switch (event) {
        case EVENT_OBJECT_SHOW:
            if (managed && hiddenByUs) {
                g.pendingVisibility.insert(hwnd);
                SchedulePending();
            } else if (!managed && IsTopLevel(hwnd)) {
                g.pendingEval.insert(hwnd);
                SchedulePending();
            }
            break;

        case EVENT_OBJECT_UNCLOAKED:
            if (!managed && IsTopLevel(hwnd)) {
                g.pendingEval.insert(hwnd);
                SchedulePending();
            }
            break;

        case EVENT_OBJECT_HIDE:
        case EVENT_OBJECT_CLOAKED:
            // Our own hides are recognised by hiddenByUs and ignored.
            if (managed && !hiddenByUs) {
                g.pendingVisibility.insert(hwnd);
                SchedulePending();
            }
            break;

        case EVENT_OBJECT_DESTROY:
            g.pendingEval.erase(hwnd);
            // A destroyed HWND can be reused by a new window; drop stale state.
            g.pendingVisibility.erase(hwnd);
            g.pendingActivation.erase(hwnd);
            g.needsNudge.erase(hwnd);
            if (managed) {
                UnmanageWindow(hwnd);
                RequestRetile();
            } else if (hiddenByUs) {
                g.hiddenByUs.erase(hwnd);
                PersistHiddenList();
            }
            break;

        case EVENT_SYSTEM_MINIMIZESTART:
            if (managed) {
                RequestRetile();
            }
            break;

        case EVENT_SYSTEM_MINIMIZEEND:
            if (managed) {
                RequestRetile();
            } else {
                g.pendingEval.insert(hwnd);
                SchedulePending();
            }
            break;

        case EVENT_SYSTEM_FOREGROUND: {
            if (!managed) {
                g.pendingEval.insert(hwnd);
                SchedulePending();
            }
            // A hidden managed window (or one it owns) took the foreground.
            // ProcessPending decides after the delay whether to follow it.
            const HWND activated =
                managed ? hwnd : GetAncestor(hwnd, GA_ROOTOWNER);
            if (g.settings.followActivation && Locate(activated, nullptr) &&
                g.hiddenByUs.count(activated) > 0) {
                g.pendingActivation.insert(activated);
                SchedulePending();
            }
            OnFocusChanged(hwnd);
            RequestBarUpdate();
            break;
        }

        case EVENT_SYSTEM_MOVESIZESTART:
            // Lets OnUserDragEnd tell a resize from a move by the size change.
            if (managed) {
                g.dragHwnd = hwnd;
                g.dragStartFrame = GetVisibleFrame(hwnd);
            }
            break;

        case EVENT_SYSTEM_MOVESIZEEND:
            if (managed) {
                OnUserDragEnd(hwnd);
            }
            g.dragHwnd = nullptr;
            break;

        case EVENT_OBJECT_LOCATIONCHANGE: {
            if (!managed) {
                break;
            }
            const bool zoomed = IsZoomed(hwnd) != FALSE;
            const bool wasZoomed = g.zoomed.count(hwnd) > 0;
            if (zoomed != wasZoomed) {
                if (zoomed) {
                    g.zoomed.insert(hwnd);
                } else {
                    g.zoomed.erase(hwnd);
                    // Restored from maximized: the app may miss the resize.
                    g.needsNudge.insert(hwnd);
                    RequestNudge();
                }
                RequestRetile();
            }
            break;
        }

        default:
            break;
    }
}

// The keyboard registration used by both RegisterRawInputDevices calls.
RAWINPUTDEVICE KeyboardDevice(DWORD flags, HWND target) {
    RAWINPUTDEVICE device{};
    device.usUsagePage = kRawUsagePage;
    device.usUsage = kRawUsageKeyboard;
    device.dwFlags = flags;
    device.hwndTarget = target;
    return device;
}

bool KeyDown(int vk) {
    return (GetAsyncKeyState(vk) & 0x8000) != 0;
}

// The hold timer may run while the feature is on and tiling is not paused.
bool HoldEnabled() {
    return g.settings.holdOverview && !g.paused;
}

bool IsModifierVk(UINT vk) {
    switch (vk) {
        case VK_SHIFT:
        case VK_LSHIFT:
        case VK_RSHIFT:
        case VK_CONTROL:
        case VK_LCONTROL:
        case VK_RCONTROL:
        case VK_LWIN:
        case VK_RWIN:
            return true;
        default:
            return false;
    }
}

// True when some HawkTile binding uses Alt with this virtual key.
bool IsAltBoundVk(UINT vk) {
    for (const Binding& binding : g.settings.bindings) {
        if ((binding.modifiers & MOD_ALT) && binding.vk == vk) {
            return true;
        }
    }
    return false;
}

// Setting the timer again restarts it, which is how a bound key re-arms it.
void ArmHold() {
    SetTimer(g.engineHwnd, kTimerHold, static_cast<UINT>(g.settings.holdDelayMs),
             nullptr);
    s_holdArmed = true;
}

void DisarmHold() {
    if (s_holdArmed) {
        KillTimer(g.engineHwnd, kTimerHold);
        s_holdArmed = false;
    }
}

// Cancels the pending hold, stops polling and closes an overview this press
// opened, abandoning any press on it. Left Alt's own state is kept.
void DropHold() {
    DisarmHold();
    KillTimer(g.engineHwnd, kTimerHoldPoll);
    s_holdShown = false;
    CloseWorkspaceOverview(true);
}

// Alt+Shift chord: left Alt and Shift (either) down, Ctrl and Win up.
bool ResizeChordDown() {
    return KeyDown(VK_LMENU) && KeyDown(VK_SHIFT) && !KeyDown(VK_CONTROL) &&
           !KeyDown(VK_LWIN) && !KeyDown(VK_RWIN);
}

// Ends the mouse-resize poll for this chord and any session it started.
void StopMouseResize() {
    KillTimer(g.engineHwnd, kTimerMouseResize);
    s_resizePolling = false;
    EndMouseResize();
}

// kTimerMouseResize, every 16 ms while the chord is held. Starts a session once
// the cursor has left the anchor's drag-threshold box; then follows the cursor.
void PollMouseResize() {
    if (!ResizeChordDown()) {
        StopMouseResize();
        return;
    }
    POINT cursor{};
    GetCursorPos(&cursor);
    if (MouseResizeActive()) {
        UpdateMouseResize(cursor);
        return;
    }
    const int limitX = 2 * GetSystemMetrics(SM_CXDRAG);
    const int limitY = 2 * GetSystemMetrics(SM_CYDRAG);
    const int dx = cursor.x - s_resizeAnchor.x;
    const int dy = cursor.y - s_resizeAnchor.y;
    if (dx >= -limitX && dx <= limitX && dy >= -limitY && dy <= limitY) {
        return;
    }
    if (BeginMouseResize(s_resizeAnchor)) {
        // Without this key, releasing Alt or Shift would open the menu bar or
        // switch the input language.
        SendMenuMaskKey();
    } else {
        // Nothing tiled under the cursor: give up until the chord turns off.
        StopMouseResize();
        s_resizeGaveUp = true;
    }
}

// Alt+Shift mouse-resize part of one key event, after HoldKeyEvent. leftAlt is
// left Alt only; right Alt counts as any other key, as in the hold rules.
void ResizeKeyEvent(UINT vk, bool keyUp, bool leftAlt) {
    const bool shift = vk == VK_SHIFT || vk == VK_LSHIFT || vk == VK_RSHIFT;
    if (!ResizeChordDown()) {
        s_resizeGaveUp = false;
    }
    if (keyUp) {
        if ((leftAlt || shift) && s_resizePolling) {
            StopMouseResize();
        }
        return;
    }
    if (leftAlt || shift) {
        // Auto-repeat key-downs find the poll already running and do nothing.
        if (!s_resizePolling && !s_resizeGaveUp && ResizeChordDown() &&
            g.settings.mouseResize && !g.paused && !WorkspaceOverviewOpen()) {
            GetCursorPos(&s_resizeAnchor);
            SetTimer(g.engineHwnd, kTimerMouseResize, 16, nullptr);
            s_resizePolling = true;
        }
        return;
    }
    // Any other key ends the resize, keeping the ratios (Alt+Shift+1 does).
    if (s_resizePolling) {
        StopMouseResize();
    }
}

// kTimerHold: left Alt has been held for holdDelayMs. Opens the overview on the
// focused monitor when Alt is still down and the feature is still enabled. Never
// while an Alt+Shift mouse resize is in progress.
void OpenHoldOverview() {
    s_holdArmed = false;
    const int mon = g.focusedMon;
    if (s_resizePolling || !s_leftAltDown || !KeyDown(VK_LMENU) ||
        !HoldEnabled() || mon < 0 ||
        mon >= static_cast<int>(g.monitors.size())) {
        return;
    }
    // Without this key, releasing Alt would open the menu bar.
    SendMenuMaskKey();
    ShowWorkspaceOverview(mon);
    s_holdShown = WorkspaceOverviewOpen();
    if (s_holdShown) {
        SetTimer(g.engineHwnd, kTimerHoldPoll, 100, nullptr);
    }
}

// kTimerHoldPoll: runs while the overview is open. Stops once it has closed. If
// this Alt press opened it and Alt is no longer down (a key-up that Raw Input
// never delivered), closes it. Keeps polling while a drag holds it open.
void PollHoldOverview() {
    if (!WorkspaceOverviewOpen()) {
        KillTimer(g.engineHwnd, kTimerHoldPoll);
        s_holdShown = false;
        return;
    }
    if (s_holdShown && !KeyDown(VK_LMENU)) {
        s_leftAltDown = false;
        s_holdShown = false;
        CloseWorkspaceOverview(false);
    }
}

// Hold-left-Alt rules for one key event (masking key and filler already
// dropped by OnRawKey). Key codes and characters are never logged.
void HoldKeyEvent(UINT vk, bool keyUp, bool leftAlt) {
    if (leftAlt) {
        if (keyUp) {
            s_leftAltDown = false;
            DisarmHold();
            if (s_holdShown) {
                s_holdShown = false;
                CloseWorkspaceOverview(false);
            }
        } else if (!s_leftAltDown || (!s_holdArmed && !s_holdShown)) {
            // A held key repeats; a repeat must not restart a pending hold.
            // With nothing pending, this is a new press after a key-up that
            // Raw Input never delivered (secure desktop), or Alt still held
            // alone after a modifier that blocked arming was released.
            s_leftAltDown = true;
            if (HoldEnabled() && !KeyDown(VK_SHIFT) && !KeyDown(VK_CONTROL) &&
                !KeyDown(VK_LWIN) && !KeyDown(VK_RWIN)) {
                ArmHold();
            }
        }
        return;
    }

    // Only key-downs of other keys matter.
    if (keyUp) {
        return;
    }
    const bool modifier = IsModifierVk(vk);
    if (s_holdShown && WorkspaceOverviewOpen()) {
        // Modifiers and HawkTile keys keep the overview open. Any other key,
        // Escape included, abandons it.
        if (!modifier && !IsAltBoundVk(vk)) {
            s_holdShown = false;
            CloseWorkspaceOverview(true);
        }
    } else if (s_leftAltDown) {
        // A HawkTile key re-arms the hold, so Alt+2 followed by holding Alt still
        // opens the overview. Any other key, modifiers included, disarms it.
        if (!modifier && IsAltBoundVk(vk) && HoldEnabled()) {
            ArmHold();
        } else {
            DisarmHold();
        }
    }
}

// Raw Input keyboard report: the hold-left-Alt rules, then the Alt+Shift
// mouse-resize rules, both on the same key event.
void OnRawKey(const RAWKEYBOARD& key) {
    const UINT vk = key.VKey;
    // Our own masking key, and the 0xFF filler some drivers report.
    if (vk == kMenuMaskVk || vk == 0xFF) {
        return;
    }
    const bool keyUp = (key.Flags & RI_KEY_BREAK) != 0;
    // Right Alt (E0) is AltGr on many layouts, so it counts as any other key.
    const bool leftAlt =
        (vk == VK_MENU || vk == VK_LMENU) && (key.Flags & RI_KEY_E0) == 0;
    HoldKeyEvent(vk, keyUp, leftAlt);
    ResizeKeyEvent(vk, keyUp, leftAlt);
}

LRESULT CALLBACK EngineWndProc(HWND hwnd, UINT msg, WPARAM wParam,
                               LPARAM lParam) {
    // The registered message id is not a compile-time constant, so no case label.
    if (s_shellHookMsg != 0 && msg == s_shellHookMsg) {
        // Only activation clears attention (OnFocusChanged); a plain
        // HSHELL_REDRAW that arrives while a window flashes is ignored.
        if (wParam == HSHELL_FLASH) {
            MarkAttention(reinterpret_cast<HWND>(lParam));
        }
        return 0;
    }

    switch (msg) {
        case WM_HOTKEY: {
            // Hotkey ids are kHotkeyIdBase + binding index.
            const std::vector<Binding>& bindings = g.settings.bindings;
            const int index = static_cast<int>(wParam) - kHotkeyIdBase;
            if (index >= 0 && index < static_cast<int>(bindings.size())) {
                const UINT modifiers = bindings[index].modifiers;
                const Command cmd = bindings[index].cmd;
                // A released Alt or Win would open a menu bar or Start menu.
                if (modifiers & (MOD_ALT | MOD_WIN)) {
                    SendMenuMaskKey();
                }
                ExecuteCommand(cmd);
            }
            return 0;
        }

        case WM_INPUT: {
            // Keyboard input for the hold-Alt overview. This case deliberately
            // leaves the switch: DefWindowProcW must still see WM_INPUT.
            RAWINPUT input{};
            UINT size = sizeof(RAWINPUT);
            const UINT read = GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam),
                                              RID_INPUT, &input, &size,
                                              sizeof(RAWINPUTHEADER));
            if (read != static_cast<UINT>(-1) &&
                input.header.dwType == RIM_TYPEKEYBOARD) {
                OnRawKey(input.data.keyboard);
            }
            break;
        }

        case WM_TIMER:
            switch (wParam) {
                case kTimerRetile:
                    KillTimer(hwnd, kTimerRetile);
                    RetileAll();
                    break;
                case kTimerPending:
                    KillTimer(hwnd, kTimerPending);
                    ProcessPending();
                    break;
                case kTimerDisplay:
                    KillTimer(hwnd, kTimerDisplay);
                    RefreshMonitors();
                    RetileAll();
                    UpdateFocusBorder();
                    UpdateTaskbarIndicators();
                    break;
                case kTimerSettle:
                    KillTimer(hwnd, kTimerSettle);
                    RetileAll();
                    UpdateFocusBorder();
                    break;
                case kTimerBar:
                    KillTimer(hwnd, kTimerBar);
                    UpdateTaskbarIndicators();
                    RefreshWorkspaceOverview();
                    break;
                case kTimerBarPoll:
                    UpdateTaskbarIndicators();
                    break;
                case kTimerHold:
                    KillTimer(hwnd, kTimerHold);
                    OpenHoldOverview();
                    break;
                case kTimerHoldPoll:
                    PollHoldOverview();
                    break;
                case kTimerMouseResize:
                    PollMouseResize();
                    break;
                case kTimerNudge: {
                    KillTimer(hwnd, kTimerNudge);
                    const std::vector<HWND> pending(g.needsNudge.begin(),
                                                    g.needsNudge.end());
                    g.needsNudge.clear();
                    int nudged = 0;
                    for (HWND window : pending) {
                        WinLoc loc;
                        if (!Locate(window, &loc) ||
                            g.hiddenByUs.count(window) > 0 ||
                            loc.ws != g.monitors[loc.mon].active) {
                            continue;
                        }
                        NudgeWindow(window);
                        ++nudged;
                    }
                    if (nudged > 0) {
                        HT_LOG(L"nudged %d window(s)", nudged);
                    }
                    break;
                }
                case kTimerStage:
                    KillTimer(hwnd, kTimerStage);
                    RefitStageProxies();
                    break;
                default:
                    break;
            }
            return 0;

        case WM_DISPLAYCHANGE:
            SetTimer(hwnd, kTimerDisplay, kDisplayDelayMs, nullptr);
            return 0;

        case WM_SETTINGCHANGE:
            if (wParam == SPI_SETWORKAREA) {
                SetTimer(hwnd, kTimerDisplay, kDisplayDelayMs, nullptr);
                return 0;
            }
            break;

        case WM_HT_RELOAD: {
            UnregisterBindings(hwnd, g.settings.bindings);
            LoadSettings(g.settings);
            const int registered = RegisterBindings(hwnd, g.settings.bindings);
            HT_LOG(L"settings reloaded, %d of %d hotkeys registered",
                   registered, static_cast<int>(g.settings.bindings.size()));
            ApplySettingsChange();
            UpdateTaskbarIndicators();
            if (!g.settings.holdOverview) {
                DropHold();
            }
            if (!g.settings.mouseResize) {
                StopMouseResize();
            }
            return 0;
        }

        // Windhawk does not call the uninit when the session ends, so the
        // hidden workspace windows are shown here before the apps are closed.
        case WM_QUERYENDSESSION:
            return TRUE;

        case WM_ENDSESSION:
            if (wParam == TRUE) {
                HT_LOG(L"session ending, showing hidden windows");
                const std::vector<HWND> hidden(g.hiddenByUs.begin(),
                                               g.hiddenByUs.end());
                for (HWND window : hidden) {
                    if (IsWindow(window)) {
                        ShowForWorkspace(window);
                    } else {
                        g.hiddenByUs.erase(window);
                    }
                }
                PersistHiddenList();

                if (g.borderedHwnd && IsWindow(g.borderedHwnd)) {
                    ClearFocusBorder(g.borderedHwnd);
                    g.borderedHwnd = nullptr;
                }
                DropHold();
                StopMouseResize();
                g.paused = true;
            }
            return 0;

        default:
            break;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

// Reports a failed start to StartEngine. The thread exit code is 1.
DWORD AbortInit() {
    s_initOk = false;
    SetEvent(s_initEvent);
    return 1;
}

// Not message-only: broadcasts such as WM_DISPLAYCHANGE and WM_SETTINGCHANGE
// are never delivered to HWND_MESSAGE windows. The window is never shown.
HWND CreateEngineWindow() {
    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = EngineWndProc;
    wc.hInstance = s_hinst;
    wc.lpszClassName = kEngineWindowClass;
    if (!RegisterClassExW(&wc)) {
        HT_LOG(L"RegisterClassExW failed (%lu)", GetLastError());
        return nullptr;
    }

    HWND hwnd = CreateWindowExW(WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE,
                                kEngineWindowClass, L"HawkTile", WS_POPUP, 0, 0,
                                0, 0, nullptr, nullptr, s_hinst, nullptr);
    if (!hwnd) {
        HT_LOG(L"CreateWindowExW failed (%lu)", GetLastError());
        UnregisterClassW(kEngineWindowClass, s_hinst);
    }
    return hwnd;
}

// A failed range is logged and skipped; the engine still runs without it.
void InstallHooks() {
    for (const EventRange& range : kEventRanges) {
        HWINEVENTHOOK hook = SetWinEventHook(
            range.first, range.last, nullptr, WinEventProc, 0, 0,
            WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS);
        if (hook) {
            s_hooks.push_back(hook);
        } else {
            HT_LOG(L"SetWinEventHook(0x%04lX..0x%04lX) failed (%lu)",
                   range.first, range.last, GetLastError());
        }
    }
}

// Always runs in this order, also after a stop request while idle.
void TeardownEngine(HWND hwnd) {
    for (HWINEVENTHOOK hook : s_hooks) {
        UnhookWinEvent(hook);
    }
    s_hooks.clear();

    UnregisterBindings(hwnd, g.settings.bindings);
    // Best effort: the window is destroyed below anyway.
    const RAWINPUTDEVICE keyboard = KeyboardDevice(RIDEV_REMOVE, nullptr);
    RegisterRawInputDevices(&keyboard, 1, sizeof(keyboard));
    KillTimer(hwnd, kTimerRetile);
    KillTimer(hwnd, kTimerPending);
    KillTimer(hwnd, kTimerDisplay);
    KillTimer(hwnd, kTimerSettle);
    KillTimer(hwnd, kTimerBar);
    KillTimer(hwnd, kTimerBarPoll);
    KillTimer(hwnd, kTimerHold);
    KillTimer(hwnd, kTimerHoldPoll);
    KillTimer(hwnd, kTimerMouseResize);
    KillTimer(hwnd, kTimerNudge);
    KillTimer(hwnd, kTimerStage);

    // The hold state belongs to this engine run: the next start begins with Alt
    // released, and an open overview closes before its window is destroyed.
    s_leftAltDown = false;
    s_holdArmed = false;
    s_holdShown = false;
    CloseWorkspaceOverview(true);

    // Ends a mouse-resize session (ratios kept) and clears its poll state.
    StopMouseResize();
    s_resizeGaveUp = false;

    // No window may stay hidden behind a dead engine.
    const std::vector<HWND> hidden(g.hiddenByUs.begin(), g.hiddenByUs.end());
    for (HWND window : hidden) {
        if (IsWindow(window)) {
            ShowForWorkspace(window);
        } else {
            g.hiddenByUs.erase(window);
        }
    }
    PersistHiddenList();

    if (g.borderedHwnd && IsWindow(g.borderedHwnd)) {
        ClearFocusBorder(g.borderedHwnd);
    }

    DestroyTaskbarIndicators();
    DestroyStageProxies();
    DestroyOsd();
    DeregisterShellHookWindow(hwnd);
    s_shellHookMsg = 0;
    DestroyWindow(hwnd);
    UnregisterClassW(kEngineWindowClass, s_hinst);
    g = EngineState{};
    HT_LOG(L"engine stopped");
}

DWORD WINAPI EngineThreadProc(LPVOID) {
    // All coordinates the engine handles must be physical pixels.
    if (!SetThreadDpiAwarenessContext(
            DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2)) {
        HT_LOG(L"SetThreadDpiAwarenessContext failed (%lu)", GetLastError());
    }

    if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                                GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                            reinterpret_cast<LPCWSTR>(&EngineWndProc),
                            &s_hinst)) {
        HT_LOG(L"GetModuleHandleExW failed (%lu)", GetLastError());
        return AbortInit();
    }

    const HWND hwnd = CreateEngineWindow();
    if (!hwnd) {
        return AbortInit();
    }
    g.engineHwnd = hwnd;

    s_shellHookMsg = RegisterWindowMessageW(L"SHELLHOOK");
    if (!RegisterShellHookWindow(hwnd)) {
        HT_LOG(L"RegisterShellHookWindow failed (%lu)", GetLastError());
    }

    LoadSettings(g.settings);
    // RefreshMonitors creates the workspaces with these layouts.
    g.appliedWorkspaceLayouts = g.settings.workspaceLayouts;
    LoadAlwaysFloatApps();
    RestoreLeftoverHiddenWindows();
    RefreshMonitors();
    AdoptExistingWindows();

    const int registered = RegisterBindings(hwnd, g.settings.bindings);
    HT_LOG(L"hotkeys registered: %d of %d", registered,
           static_cast<int>(g.settings.bindings.size()));

    // Keyboard Raw Input for the hold-Alt overview. Without it the overview never
    // opens, but hotkeys and tiling work as before.
    const RAWINPUTDEVICE keyboard = KeyboardDevice(RIDEV_INPUTSINK, hwnd);
    if (!RegisterRawInputDevices(&keyboard, 1, sizeof(keyboard))) {
        HT_LOG(L"RegisterRawInputDevices failed (%lu), workspace overview disabled",
               GetLastError());
    }

    InstallHooks();

    RetileAll();
    OnFocusChanged(GetForegroundWindow());
    // Windows restored from maximized at adoption may finish asynchronously,
    // possibly before the hooks existed; one late retile catches them.
    SetTimer(hwnd, kTimerSettle, 300, nullptr);
    UpdateTaskbarIndicators();
    SetTimer(hwnd, kTimerBarPoll, 1000, nullptr);

    int managed = 0;
    for (const Monitor& m : g.monitors) {
        for (const Workspace& ws : m.workspaces) {
            managed += static_cast<int>(ws.tiled.size() + ws.floating.size());
        }
    }
    HT_LOG(L"engine started: %d monitor(s), %d managed window(s)",
           static_cast<int>(g.monitors.size()), managed);

    s_initOk = true;
    SetEvent(s_initEvent);

    MSG msg{};
    BOOL result;
    while ((result = GetMessageW(&msg, nullptr, 0, 0)) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    if (result < 0) {
        HT_LOG(L"GetMessageW failed (%lu)", GetLastError());
    }

    TeardownEngine(hwnd);
    return 0;
}

void ResetHandles() {
    if (s_thread) {
        CloseHandle(s_thread);
    }
    if (s_initEvent) {
        CloseHandle(s_initEvent);
    }
    s_thread = nullptr;
    s_threadId = 0;
    s_initEvent = nullptr;
    s_initOk = false;
}

}  // namespace

bool StartEngine() {
    if (s_thread) {
        return s_initOk;
    }

    s_initEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    if (!s_initEvent) {
        HT_LOG(L"CreateEventW failed (%lu)", GetLastError());
        return false;
    }

    s_initOk = false;
    s_thread =
        CreateThread(nullptr, 0, EngineThreadProc, nullptr, 0, &s_threadId);
    if (!s_thread) {
        HT_LOG(L"CreateThread failed (%lu)", GetLastError());
        ResetHandles();
        return false;
    }

    const HANDLE waits[] = {s_initEvent, s_thread};
    const DWORD result =
        WaitForMultipleObjects(2, waits, FALSE, kStartTimeoutMs);
    if (result == WAIT_OBJECT_0) {
        if (s_initOk) {
            return true;
        }
        HT_LOG(L"engine init failed");
        WaitForSingleObject(s_thread, kAbortWaitMs);
    } else if (result == WAIT_OBJECT_0 + 1) {
        HT_LOG(L"engine thread exited during init");
    } else {
        if (result == WAIT_TIMEOUT) {
            HT_LOG(L"engine init timed out after %lu ms", kStartTimeoutMs);
        } else {
            HT_LOG(L"WaitForMultipleObjects failed (%lu)", GetLastError());
        }
        PostThreadMessageW(s_threadId, WM_QUIT, 0, 0);
        WaitForSingleObject(s_thread, kAbortWaitMs);
    }

    ResetHandles();
    return false;
}

void NotifySettingsChanged() {
    if (g.engineHwnd) {
        PostMessageW(g.engineHwnd, WM_HT_RELOAD, 0, 0);
    }
}

void StopEngine() {
    if (!s_thread) {
        return;
    }

    PostThreadMessageW(s_threadId, WM_QUIT, 0, 0);
    if (WaitForSingleObject(s_thread, kStopTimeoutMs) != WAIT_OBJECT_0) {
        HT_LOG(L"engine thread did not stop within %lu ms", kStopTimeoutMs);
    }
    ResetHandles();
}

}  // namespace ht


// =============================================================================
// 90_toolmod.cpp — Windhawk entry points.
// =============================================================================

BOOL WhTool_ModInit() {
    HT_LOG(L"HawkTile init");
    return ht::StartEngine() ? TRUE : FALSE;
}

void WhTool_ModSettingsChanged() {
    HT_LOG(L"HawkTile settings changed");
    ht::NotifySettingsChanged();
}

void WhTool_ModUninit() {
    HT_LOG(L"HawkTile uninit");
    ht::StopEngine();
}

////////////////////////////////////////////////////////////////////////////////
// Standard snippet from the Windhawk wiki, as published in MIT-licensed mods in
// ramensoftware/windhawk-mods; changed to wait for a previous instance. See
// THIRD_PARTY_NOTICES.md.
//
// Windhawk tool mod implementation for mods which don't need to inject to other
// processes or hook other functions. Context:
// https://github.com/ramensoftware/windhawk/wiki/Mods-as-tools:-Running-mods-in-a-dedicated-process
//
// The mod will load and run in a dedicated windhawk.exe process.
//
// Paste the code below as part of the mod code, and use these callbacks:
// * WhTool_ModInit
// * WhTool_ModSettingsChanged
// * WhTool_ModUninit
//
// Currently, other callbacks are not supported.

bool g_isToolModProcessLauncher;
HANDLE g_toolModProcessMutex;

void WINAPI EntryPoint_Hook() {
    Wh_Log(L">");
    ExitThread(0);
}

BOOL Wh_ModInit() {
    DWORD sessionId;
    if (ProcessIdToSessionId(GetCurrentProcessId(), &sessionId) &&
        sessionId == 0) {
        return FALSE;
    }

    bool isExcluded = false;
    bool isToolModProcess = false;
    bool isCurrentToolModProcess = false;
    int argc;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLine(), &argc);
    if (!argv) {
        Wh_Log(L"CommandLineToArgvW failed");
        return FALSE;
    }

    for (int i = 1; i < argc; i++) {
        if (wcscmp(argv[i], L"-service") == 0 ||
            wcscmp(argv[i], L"-service-start") == 0 ||
            wcscmp(argv[i], L"-service-stop") == 0) {
            isExcluded = true;
            break;
        }
    }

    for (int i = 1; i < argc - 1; i++) {
        if (wcscmp(argv[i], L"-tool-mod") == 0) {
            isToolModProcess = true;
            if (wcscmp(argv[i + 1], WH_MOD_ID) == 0) {
                isCurrentToolModProcess = true;
            }
            break;
        }
    }

    LocalFree(argv);

    if (isExcluded) {
        return FALSE;
    }

    if (isCurrentToolModProcess) {
        g_toolModProcessMutex =
            CreateMutex(nullptr, TRUE, L"windhawk-tool-mod_" WH_MOD_ID);
        if (!g_toolModProcessMutex) {
            Wh_Log(L"CreateMutex failed");
            ExitProcess(1);
        }

        if (GetLastError() == ERROR_ALREADY_EXISTS) {
            // On a recompile or settings reload Windhawk starts the new tool
            // process while the old one may still be shutting down (it
            // re-shows hidden windows first). Wait for it to exit and take
            // over its mutex instead of quitting, or no instance survives.
            const DWORD wait = WaitForSingleObject(g_toolModProcessMutex, 15000);
            if (wait != WAIT_OBJECT_0 && wait != WAIT_ABANDONED) {
                Wh_Log(L"Tool mod already running (%s)", WH_MOD_ID);
                ExitProcess(1);
            }
        }

        if (!WhTool_ModInit()) {
            ExitProcess(1);
        }

        IMAGE_DOS_HEADER* dosHeader =
            (IMAGE_DOS_HEADER*)GetModuleHandle(nullptr);
        IMAGE_NT_HEADERS* ntHeaders =
            (IMAGE_NT_HEADERS*)((BYTE*)dosHeader + dosHeader->e_lfanew);

        DWORD entryPointRVA = ntHeaders->OptionalHeader.AddressOfEntryPoint;
        void* entryPoint = (BYTE*)dosHeader + entryPointRVA;

        Wh_SetFunctionHook(entryPoint, (void*)EntryPoint_Hook, nullptr);
        return TRUE;
    }

    if (isToolModProcess) {
        return FALSE;
    }

    g_isToolModProcessLauncher = true;
    return TRUE;
}

void Wh_ModAfterInit() {
    if (!g_isToolModProcessLauncher) {
        return;
    }

    WCHAR currentProcessPath[MAX_PATH];
    switch (GetModuleFileName(nullptr, currentProcessPath,
                              ARRAYSIZE(currentProcessPath))) {
        case 0:
        case ARRAYSIZE(currentProcessPath):
            Wh_Log(L"GetModuleFileName failed");
            return;
    }

    WCHAR
    commandLine[MAX_PATH + 2 +
                (sizeof(L" -tool-mod \"" WH_MOD_ID "\"") / sizeof(WCHAR)) - 1];
    swprintf_s(commandLine, L"\"%s\" -tool-mod \"%s\"", currentProcessPath,
               WH_MOD_ID);

    HMODULE kernelModule = GetModuleHandle(L"kernelbase.dll");
    if (!kernelModule) {
        kernelModule = GetModuleHandle(L"kernel32.dll");
        if (!kernelModule) {
            Wh_Log(L"No kernelbase.dll/kernel32.dll");
            return;
        }
    }

    using CreateProcessInternalW_t = BOOL(WINAPI*)(
        HANDLE hUserToken, LPCWSTR lpApplicationName, LPWSTR lpCommandLine,
        LPSECURITY_ATTRIBUTES lpProcessAttributes,
        LPSECURITY_ATTRIBUTES lpThreadAttributes, WINBOOL bInheritHandles,
        DWORD dwCreationFlags, LPVOID lpEnvironment, LPCWSTR lpCurrentDirectory,
        LPSTARTUPINFOW lpStartupInfo,
        LPPROCESS_INFORMATION lpProcessInformation,
        PHANDLE hRestrictedUserToken);
    CreateProcessInternalW_t pCreateProcessInternalW =
        (CreateProcessInternalW_t)GetProcAddress(kernelModule,
                                                 "CreateProcessInternalW");
    if (!pCreateProcessInternalW) {
        Wh_Log(L"No CreateProcessInternalW");
        return;
    }

    STARTUPINFO si{
        .cb = sizeof(STARTUPINFO),
        .dwFlags = STARTF_FORCEOFFFEEDBACK,
    };
    PROCESS_INFORMATION pi;
    if (!pCreateProcessInternalW(nullptr, currentProcessPath, commandLine,
                                 nullptr, nullptr, FALSE, NORMAL_PRIORITY_CLASS,
                                 nullptr, nullptr, &si, &pi, nullptr)) {
        Wh_Log(L"CreateProcess failed");
        return;
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}

void Wh_ModSettingsChanged() {
    if (g_isToolModProcessLauncher) {
        return;
    }

    WhTool_ModSettingsChanged();
}

void Wh_ModUninit() {
    if (g_isToolModProcessLauncher) {
        return;
    }

    WhTool_ModUninit();
    ExitProcess(0);
}
