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


// =============================================================================
// 10_contract.h — shared contract for all HawkTile fragments.
//
// build.ps1 concatenates the fragments in order into ONE translation unit:
//   00_header.wh.cpp, 10_contract.h, 20_layout.cpp, 30_win32.cpp,
//   40_config.cpp, 50_model.cpp, 60_engine.cpp, 90_toolmod.cpp
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
inline constexpr int kHotkeyIdBase = 0x100;           // hotkey id = base + binding index
inline constexpr BYTE kMenuMaskVk = 0xE8;             // unassigned VK used to mask Alt/Win

// =============================================================================
// Layout (20_layout.cpp) — pure functions, no Win32 calls except RECT type.
// =============================================================================

enum class LayoutKind { Dwindle, MasterStack, Columns, Rows, Monocle, Count_ };
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
};

// Returns exactly `count` rects tiling `area` (count <= 0 → empty vector).
// Gap rule: outer edges of the result coincide with `area`; neighbouring
// rects are separated by exactly innerGap px. Every rect has width/height >= 1.
//
// Every layout except Monocle is a sequence of count-1 binary splits of the
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
std::vector<RECT> ComputeLayout(const RECT& area, int count, const LayoutParams& p);

// Effective ratio of split i: p.ratios[i] if present and 0 < r < 1 (clamped to
// [0.05, 0.95]), otherwise the default:
//   Dwindle:      i == 0 ? masterRatio : 0.5
//   MasterStack:  i == 0 ? masterRatio : 1.0 / (count - i)   (equal stack rows)
//   Columns/Rows: 1.0 / (count - i)                          (equal slots)
// masterRatio is clamped to [0.05, 0.95]. Returns 0.5 for out-of-range i.
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
const wchar_t* LayoutName(LayoutKind k);       // "dwindle","master","columns","rows","monocle"
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
    Rescan,                 // re-read monitors, adopt new windows, retile
    ToggleTiling,           // pause / resume tiling
    CloseWindow,
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
    bool focusBorder = true;         // border.enabled
    COLORREF focusBorderColor = RGB(0x4C, 0x9A, 0xFF);  // border.color ("RRGGBB" or "#RRGGBB")
    bool floatNonResizable = true;   // windows.floatNonResizable
    bool floatTopmost = true;        // windows.floatTopmost
    bool tileMaximized = true;       // windows.tileMaximized
    bool workspaceOsd = true;        // indicators.popup
    int osdDurationMs = 2000;        // indicators.popupDuration; <= 0 → 2000, clamped 100..10000
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
    {L"alt+j", L"togglesplit"},
    {L"alt+l", L"nextlayout"},
    {L"alt+equals", L"resizeactive left"},
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
//   nextlayout               layout dwindle|master|columns|rows|monocle
//   togglefloating  togglemonocle  fullscreen  togglesplit  swapwithmaster
//   growmaster  shrinkmaster  rescan  toggletiling  closewindow
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
//  Float:  matches settings.floatRules; has an owner window; (floatTopmost &&
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
void HideForWorkspace(HWND hwnd);
void ShowForWorkspace(HWND hwnd);

// Mod storage key "hiddenWindows": comma-separated "hwnd:pid" hex pairs of
// g.hiddenByUs, via Wh_SetStringValue. RestoreLeftoverHiddenWindows reads it
// (Wh_GetStringValue), shows every listed window that IsWindow(), still
// belongs to the saved pid and is !IsWindowVisible() with
// ShowWindowAsync(SW_SHOWNA), then clears the value.
// Under HT_STANDALONE both are no-ops.
void PersistHiddenList();
void RestoreLeftoverHiddenWindows();

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
// remaining one and are shown if that one is active), then RetileAll() and
// UpdateFocusBorder().
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
// moved to the floating list (logged). No-op while g.paused.
void Retile(int mon);
void RetileAll();
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
// window: if its visible-frame center is now on another monitor → a floating
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
void ShowWorkspaceOsd(int mon);
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
    s.taskbarIndicator = Wh_GetIntSetting(L"indicators.taskbar") != 0;
    s.attentionDots = Wh_GetIntSetting(L"indicators.notificationDots") != 0;
    s.followActivation = Wh_GetIntSetting(L"windows.followActivation") != 0;
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
           L"attentionDots=%d, followActivation=%d, debugLog=%d",
           s.outerGap, s.innerGap, s.workspaceCount, LayoutName(s.defaultLayout),
           static_cast<int>(s.floatRules.size()),
           static_cast<int>(s.ignoreRules.size()),
           static_cast<int>(s.bindings.size()),
           static_cast<int>(s.tileMaximized), static_cast<int>(s.workspaceOsd),
           s.osdDurationMs, static_cast<int>(s.taskbarIndicator),
           s.taskbarIndicatorOffset, static_cast<int>(s.attentionDots),
           static_cast<int>(s.followActivation), static_cast<int>(s.debugLog));
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

Workspace MakeWorkspace() {
    Workspace ws;
    ws.layout = g.settings.defaultLayout;
    ws.prevLayout = g.settings.defaultLayout;
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
    return LayoutParams{ws.layout, g.settings.masterRatio, g.settings.innerGap,
                        ws.ratios, ws.flips};
}

void SetWorkspaceLayout(Workspace& ws, LayoutKind kind) {
    // Remember the layout we leave so togglemonocle can return to it.
    if (kind == LayoutKind::Monocle && ws.layout != LayoutKind::Monocle) {
        ws.prevLayout = ws.layout;
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
        const int nb = FindNeighbor(FramesOf(cands), from, dir);
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
        const int nb =
            from < 0 ? -1 : FindNeighbor(FramesOf(cands), from, dir);
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
        const RECT work = g.monitors[loc.mon].rcWork;
        const LONG areaW = work.right - work.left;
        const LONG areaH = work.bottom - work.top;
        const LONG w = areaW * 6 / 10;
        const LONG h = areaH * 6 / 10;
        const LONG x = work.left + (areaW - w) / 2;
        const LONG y = work.top + (areaH - h) / 2;
        PlaceWindow(cur, RECT{x, y, x + w, y + h});
    }
    Retile(loc.mon);
    UpdateFocusBorder();
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

}  // namespace

Workspace& ActiveWorkspace(int mon) {
    static Workspace fallback;  // only for invalid indices; callers check first
    if (mon < 0 || mon >= MonitorCount()) {
        fallback = Workspace{};
        return fallback;
    }
    Monitor& m = g.monitors[mon];
    if (m.workspaces.empty()) m.workspaces.push_back(MakeWorkspace());
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
            m.workspaces.assign(WsCount(), MakeWorkspace());
        }
        m.handle = found[f].handle;
        m.rcMonitor = found[f].rcMonitor;
        m.rcWork = found[f].rcWork;
        if (m.workspaces.empty()) m.workspaces.push_back(MakeWorkspace());
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
            m.workspaces.push_back(MakeWorkspace());
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
        // Covers a removed active workspace and windows moved into it.
        ShowHiddenIn(m.workspaces[m.active]);
    }
    RetileAll();
    UpdateFocusBorder();
    RequestBarUpdate();
}

void Retile(int mon) {
    if (g.paused || mon < 0 || mon >= MonitorCount()) return;
    Workspace& ws = ActiveWorkspace(mon);
    const std::vector<HWND> visible = LayoutWindowsOf(ws);
    if (visible.empty()) return;

    const std::vector<RECT> rects =
        ComputeLayout(LayoutAreaOf(mon), static_cast<int>(visible.size()),
                      LayoutParamsOf(ws));
    std::vector<HWND> failed;
    for (size_t i = 0; i < visible.size() && i < rects.size(); ++i) {
        if (!PlaceWindow(visible[i], rects[i])) failed.push_back(visible[i]);
    }
    for (HWND h : failed) {
        RemoveFrom(ws.tiled, h);
        ws.floating.push_back(h);
        HT_LOG(L"window %p cannot be tiled; now floating", (void*)h);
    }
    if (!failed.empty()) RequestRetile();
}

void RetileAll() {
    for (int i = 0; i < MonitorCount(); ++i) Retile(i);
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
    if (Locate(hwnd, &loc)) {
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
        if (!g.paused) RetileAll();
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
            if (has && !loc.floating && ws.tiled.size() > 1) {
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
            if (has && !loc.floating && ws.layout != LayoutKind::Monocle) {
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
// 55_osd.cpp — workspace indicator: a small topmost, click-through, never
// activated panel with one column per workspace of a monitor. Each column has
// a numbered badge; occupied workspaces also show one icon per app.
// Engine thread only; all coordinates are physical pixels (per-monitor DPI v2).
// =============================================================================

#include <cmath>
#include <shellscalingapi.h>
#include <unordered_map>

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

// One app on a workspace as shown: its icon and how many windows it has there.
struct OsdIcon {
    HICON icon = nullptr;  // always set once resolved
    int count = 1;
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
    int count = 0;
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

    const int pad = OsdScale(kOsdPadDip);
    const int badge = OsdScale(kOsdBadgeDip);
    const int iconSize = OsdScale(kOsdIconDip);
    const int iconGap = OsdScale(kOsdIconGapDip);
    const int iconTop = pad + badge + OsdScale(kOsdIconTopDip);
    const int step = iconSize + iconGap;
    const HGDIOBJ smallFont = osd.smallFont ? osd.smallFont : defaultFont;

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
        const int badgeLeft = col.left + (col.width - badge) / 2;
        RECT circle{badgeLeft, pad, badgeLeft + badge, pad + badge};
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
        const int iconLeft = col.left + (col.width - iconSize) / 2;
        const int stack = static_cast<int>(col.icons.size());
        auto rowTop = [iconTop, step](int row) { return iconTop + row * step; };
        for (int s = 0; s < stack; ++s) {
            DrawIconEx(mem, iconLeft, rowTop(s),
                       col.icons[static_cast<size_t>(s)].icon, iconSize, iconSize,
                       0, nullptr, DI_NORMAL);
        }
        for (int s = 0; s < stack; ++s) {
            const OsdIcon& icon = col.icons[static_cast<size_t>(s)];
            if (icon.count > 1) {
                OsdPaintCountBadge(mem, iconLeft + iconSize, rowTop(s) + iconSize,
                                   icon.count, smallFont);
            }
        }
        if (col.more > 0) {
            const int top = rowTop(stack);
            RECT cell{iconLeft, top, iconLeft + iconSize, top + iconSize};
            const std::wstring more = L"+" + std::to_wstring(col.more);
            SelectObject(mem, smallFont);
            SetTextColor(mem, osd.theme.listText);
            DrawTextW(mem, more.c_str(), static_cast<int>(more.size()), &cell,
                      DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
        }
    }

    BitBlt(target, 0, 0, w, h, mem, 0, 0, SRCCOPY);

    SelectObject(mem, oldFont);
    SelectObject(mem, oldPen);
    SelectObject(mem, oldBmp);
    DeleteObject(bmp);
    DeleteDC(mem);
}

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
            return HTTRANSPARENT;
        case WM_MOUSEACTIVATE:
            return MA_NOACTIVATE;
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
    if (mon < 0 || mon >= static_cast<int>(g.monitors.size())) return;
    if (!OsdEnsureWindow()) return;

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
                OsdIcon{OsdIconFor(group.hwnd, group.path), group.count});
        }
        if (groupCount > kOsdMaxRows) col.more = groupCount - icons;
        osd.rows = std::max(osd.rows, col.slots());
    }

    // Every column, occupied or empty, is the badge column width. The panel is
    // centered on the work area; 9 columns of this width always fit it.
    const int pad = OsdScale(kOsdPadDip);
    const int gap = OsdScale(kOsdColGapDip);
    const int colWidth = OsdScale(kOsdColumnDip);
    const int badge = OsdScale(kOsdBadgeDip);
    const int iconSize = OsdScale(kOsdIconDip);
    const int iconGap = OsdScale(kOsdIconGapDip);

    int x = pad;
    for (OsdColumn& col : osd.columns) {
        col.left = x;
        col.width = colWidth;
        x += colWidth + gap;
    }
    const int width = x - gap + pad;
    const int height =
        2 * pad + badge +
        (osd.rows > 0 ? OsdScale(kOsdIconTopDip) + osd.rows * iconSize +
                            (osd.rows - 1) * iconGap
                      : 0);
    const int areaW = static_cast<int>(m.rcWork.right - m.rcWork.left);
    const int areaH = static_cast<int>(m.rcWork.bottom - m.rcWork.top);

    const int x0 = static_cast<int>(m.rcWork.left) + (areaW - width) / 2;
    const int y0 = static_cast<int>(m.rcWork.top) + (areaH - height) / 2;
    SetWindowPos(osd.hwnd, HWND_TOPMOST, x0, y0, width, height,
                 SWP_NOACTIVATE | SWP_SHOWWINDOW);
    InvalidateRect(osd.hwnd, nullptr, FALSE);
    UpdateWindow(osd.hwnd);

    // Calling again while visible resets the timer, so the overlay stays up
    // for a full duration after the latest keypress.
    const int ms = g.settings.osdDurationMs > 0 ? g.settings.osdDurationMs
                                                : kOsdDefaultDurationMs;
    SetTimer(osd.hwnd, kOsdHideTimer, static_cast<UINT>(ms), nullptr);
}

void DestroyOsd() {
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
                }
                RequestRetile();
            }
            break;
        }

        default:
            break;
    }
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
                    break;
                case kTimerBarPoll:
                    UpdateTaskbarIndicators();
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
    KillTimer(hwnd, kTimerRetile);
    KillTimer(hwnd, kTimerPending);
    KillTimer(hwnd, kTimerDisplay);
    KillTimer(hwnd, kTimerSettle);
    KillTimer(hwnd, kTimerBar);
    KillTimer(hwnd, kTimerBarPoll);

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
    RestoreLeftoverHiddenWindows();
    RefreshMonitors();
    AdoptExistingWindows();

    const int registered = RegisterBindings(hwnd, g.settings.bindings);
    HT_LOG(L"hotkeys registered: %d of %d", registered,
           static_cast<int>(g.settings.bindings.size()));

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
