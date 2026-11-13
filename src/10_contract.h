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
