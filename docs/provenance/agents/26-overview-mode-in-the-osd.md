# 26. overview mode in the OSD

| | |
|---|---|
| Launched | 2026-10-13 07:58 UTC |
| Role | implementer |
| Model | Claude Haiku 5.5 |
| Agent id | `a7ad3143be3bc483b` |

## Instructions from the planner (verbatim)

````text
You are modifying one fragment of HawkTile, a tiling window manager written as a Windhawk mod (C++23, clang, mingw target). Project root: <repo>

## Ground rules
- Read `src/10_contract.h` fully first. It is the shared contract; do not change it. The new section "Workspace overview (hold left Alt)" (next to ShowWorkspaceOsd) defines the four functions you implement: ShowWorkspaceOverview, CloseWorkspaceOverview, WorkspaceOverviewOpen, RefreshWorkspaceOverview, plus the changed behaviour of ShowWorkspaceOsd while the overview is open. Then read `src/55_osd.cpp` fully.
- Edit ONLY `src/55_osd.cpp`. Another agent is editing `src/60_engine.cpp` and `src/40_config.cpp` at the same time (the engine detects the Alt hold and calls your functions); do not touch them.
- Keep the existing look and behaviour of the timed popup exactly as it is. Match the file's style: comment density, naming (Osd* helpers in the anonymous namespace, `osd` state struct), physical pixels via OsdScale, GDI only.
- Clean room: do NOT look at, search for or reproduce code from any other window manager or task switcher. Use only the Win32 API documentation you know and this repository.
- NEVER run the mod or any built executable.

## What to build
Today the panel is a click-through popup (WS_EX_TRANSPARENT, HTTRANSPARENT) that hides after a timer. Add an INTERACTIVE "overview" mode for the same window: it stays up with no timer while the user holds left Alt, takes mouse input without ever activating (keep WS_EX_NOACTIVATE and MA_NOACTIVATE), and lets the user drag app icons between workspace columns.

1. Data
   - `OsdGroup` gets `std::vector<HWND> windows;` (every window of the group, first-seen order; `count` stays = windows.size()). `OsdIcon` gets `std::vector<HWND> windows;` copied from its group. The "+N" slot has no windows and is never draggable.
   - `OsdState` gets: `bool interactive` (overview mode), `int mon = -1` (monitor currently rendered), `bool closeAfterPress` (Alt was released during a press), press/drag state (`int pressCol = -1`, `int pressItem` where -1 = badge and >= 0 = icon index, `POINT pressPt`, `bool dragging`, `POINT cursor`, `int dropCol = -1`), hover state (`int hoverCol = -1`, `int hoverItem`, `bool trackingLeave`). Use a named constant for "no item" (e.g. `kOsdNoItem = -2`).

2. Refactor ShowWorkspaceOsd
   - Move its body (everything except the hide timer) into `bool OsdRender(int mon)` that also stores `osd.mon = mon`, resets hover, and returns false on bad `mon` / window failure.
   - `ShowWorkspaceOsd(mon)`: if WorkspaceOverviewOpen() → abandon any press/drag (no effect, release capture), OsdRender(mon), no timer, return. Otherwise exactly today's behaviour (render + hide timer).

3. Shared geometry
   - One helper (e.g. `struct OsdMetrics` built from osd.dpi) used by BOTH painting and hit-testing so they can never disagree: pad, gap, badge, iconSize, iconGap, iconTop = pad + badge + OsdScale(kOsdIconTopDip), step = iconSize + iconGap.
   - Hit test of a client point → {col, item}:
     - Column zone i: x in [col.left - gap/2, col.left + col.width + (gap - gap/2)), with the first zone extended to x = 0 and the last to the client's right edge; any y inside the client rect. Outside the client rect → col = -1.
     - item = -1 (badge) when y in [pad, pad + badge); item = s (icon s, s < icons.size()) when y in [iconTop + s*step - iconGap/2, iconTop + s*step + iconSize + iconGap - iconGap/2); otherwise kOsdNoItem.

4. Interactive window
   - `OsdSetInteractive(bool)` adds/removes WS_EX_TRANSPARENT (GetWindowLongPtrW/SetWindowLongPtrW GWL_EXSTYLE, then SetWindowPos with SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_FRAMECHANGED).
   - WM_NCHITTEST → `osd.interactive ? HTCLIENT : HTTRANSPARENT`.
   - WM_SETCURSOR (interactive, LOWORD(lp) == HTCLIENT): IDC_HAND over a badge or an icon, else IDC_ARROW; return TRUE. Not interactive → default handling. While capture is held WM_SETCURSOR is not sent, so while dragging call SetCursor(IDC_SIZEALL) yourself on drag start and on every WM_MOUSEMOVE.
   - Mouse coordinates with GET_X_LPARAM / GET_Y_LPARAM (`#include <windowsx.h>` at the top with the other includes).

5. Mouse (left button only, only while osd.interactive)
   - WM_LBUTTONDOWN on a badge or an icon → record pressCol/pressItem/pressPt, SetCapture(hwnd). Anything else → ignore.
   - WM_MOUSEMOVE: store osd.cursor. If an ICON is pressed and not yet dragging and the cursor moved more than GetSystemMetrics(SM_CXDRAG) horizontally or SM_CYDRAG vertically → dragging = true. While dragging: dropCol = the column zone under the cursor if it differs from pressCol, else -1; repaint. When not pressed: update hover (repaint only when it changes) and arm TrackMouseEvent(TME_LEAVE) once; WM_MOUSELEAVE clears hover.
   - WM_LBUTTONUP: copy what you need into locals and CLEAR the press/drag state BEFORE calling ReleaseCapture (ReleaseCapture sends WM_CAPTURECHANGED synchronously, which must then see no press). Then:
     - was dragging and dropCol >= 0 → drop: for each hwnd of the pressed icon's `windows`, if `Locate(hwnd, &loc) && loc.mon == osd.mon && loc.ws == pressCol` → MoveWindowToWorkspace(hwnd, osd.mon, dropCol); count them; HT_LOG(L"overview: moved %d window(s) from workspace %d to %d", n, pressCol + 1, dropCol + 1).
     - not dragging and the hit test at release is the same col and item as the press → click: badge → SwitchWorkspace(osd.mon, pressCol); icon → ActivateWindowWorkspace(first hwnd of the icon's windows for which Locate() succeeds; none → nothing).
     - Afterwards: if closeAfterPress → close the overview (as CloseWorkspaceOverview does); else if still open → OsdRender(osd.mon). Note the model functions may re-enter ShowWorkspaceOsd (SwitchWorkspace calls it) — that must be safe because the press state is already cleared.
   - WM_CAPTURECHANGED while a press is still recorded (capture taken by someone else) → abandon it with no effect; then close if closeAfterPress, else repaint.

6. Painting additions (keep NULL_PEN as the default pen; select and restore any pen you need)
   - Draw order: background; drop-target highlight; hover highlight; then the existing badges/icons/count badges/"+N"; then the dragged icon.
   - Drop target (dragging && dropCol >= 0): a rounded rectangle (corner OsdScale(8)) spanning the drop column's zone width inset by 1 px, from pad/2 to height - pad/2, filled with OsdBlend(theme.background, theme.activeFill, 0.30). In high contrast (theme.highContrast) draw a 2-px frame in theme.activeFill instead of the fill.
   - Hover (interactive, not pressed): a rounded rectangle behind the hovered badge (its circle inflated by OsdScale(3)) or the hovered icon (column width x step, centered on the icon), filled with OsdBlend(theme.background, theme.occupiedFill, 0.5).
   - While dragging, the source slot (pressCol, pressItem) draws neither its icon nor its count badge; draw a 1-px rounded outline of the icon cell in theme.emptyText instead. The dragged icon is drawn last, centered on osd.cursor, OsdScale(20) px, with its count badge when count > 1.

7. Public functions (in namespace ht, outside the anonymous namespace, next to ShowWorkspaceOsd)
   - ShowWorkspaceOverview(mon): bad mon or OsdEnsureWindow failure → return. KillTimer(kOsdHideTimer); interactive = true; closeAfterPress = false; OsdSetInteractive(true); OsdRender(mon). HT_LOG(L"overview opened on monitor %d", mon) only when it was not already open.
   - CloseWorkspaceOverview(cancelDrag): not open → return. A press/drag in progress and !cancelDrag → closeAfterPress = true, return. Otherwise clear press/drag/hover, release capture if GetCapture() == osd.hwnd, interactive = false, closeAfterPress = false, OsdSetInteractive(false), KillTimer(kOsdHideTimer), ShowWindow(SW_HIDE).
   - WorkspaceOverviewOpen(): osd.interactive && osd.hwnd && IsWindowVisible(osd.hwnd).
   - RefreshWorkspaceOverview(): open and no press/drag → OsdRender(osd.mon).
   - DestroyOsd(): release capture first if we hold it; the rest as today.

## Verify
- `powershell -NoProfile -File <repo>\build.ps1 -Check 55_osd.cpp` must show errors=0 warnings=0. This is the authoritative check for your work.
- Then the full build `powershell -NoProfile -File <repo>\build.ps1` (both clang lines should be errors=0 warnings=0) and `... build.ps1 -Test` (0 failed). If the full build fails ONLY because of 60_engine.cpp or 40_config.cpp (the other agent's files, possibly mid-edit), say so and do not touch them.
- Re-read your WM_LBUTTONUP / WM_CAPTURECHANGED / CloseWorkspaceOverview code once more for re-entrancy: state cleared before ReleaseCapture and before calling any model function.

## Report
Under 200 words: what you changed (functions added/changed), the verification results line by line, and anything in the spec you could not follow or found ambiguous.
````

## Report from the agent (2026-10-13 08:05 UTC)

````text
Edited only src/55_osd.cpp (the new file includes windowsx.h and the overview code). Nothing was run.

Changes:
- Data: OsdIcon and OsdGroup carry `windows`. OsdState has the overview, press, drag and hover fields. kOsdNoItem = -2, kOsdBadgeItem = -1.
- ShowWorkspaceOsd now calls the new OsdRender(mon) for the body. Painting and hit testing both use OsdCurrentMetrics/OsdZoneBounds/OsdHitTest, so popup geometry is unchanged.
- Added OsdSetInteractive, the mouse messages (NCHITTEST, SETCURSOR, LBUTTONDOWN/UP, MOUSEMOVE/LEAVE, CAPTURECHANGED), and the drop, hover, source-outline and dragged-icon painting.
- Public: ShowWorkspaceOverview, CloseWorkspaceOverview, WorkspaceOverviewOpen, RefreshWorkspaceOverview added. DestroyOsd releases capture first.

Verification:
- build.ps1 -Check 55_osd.cpp: clang exit=0 errors=0 warnings=0
- build.ps1 (DLL line): clang exit=0 errors=0 warnings=0
- build.ps1 (real-mode line): clang exit=0 errors=0 warnings=0
- build.ps1 -Test: NOT run. It executes the built layout_test.exe, which the ground rules forbid. It also never compiles 55_osd.cpp, so this change cannot affect it.

Spec gaps and choices:
1. ShowWorkspaceOsd during an overview press with closeAfterPress set: abandons the press and closes, instead of only re-rendering. Otherwise the overview would stay open with nothing to end the press.
2. ShowWorkspaceOverview abandons any leftover press before opening.
3. The badge hover is a disc (radius = half its height). "corner OsdScale(8)" is read as an 8 px radius.
4. High contrast: occupiedFill equals background, so the hover fill is invisible there. The spec does not cover hover in high contrast. A 1 px activeFill frame would fix it; not implemented.
5. The badge band covers the full column width, as the spec's y-only rule implies.
````
