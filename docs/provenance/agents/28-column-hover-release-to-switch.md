# 28. column hover + release to switch

| | |
|---|---|
| Launched | 2026-10-14 05:41 UTC |
| Role | implementer |
| Model | Claude Haiku 5.5 |
| Agent id | `a24b3ad97b5853582` |

## Instructions from the planner (verbatim)

````text
You are modifying one fragment of HawkTile, a tiling window manager written as a Windhawk mod (C++23, clang, mingw target). Project root: <repo>

## Ground rules
- Read `src/10_contract.h`, section "Workspace overview (hold left Alt)" (the comments above CloseWorkspaceOverview were just extended with the hover and release-to-switch rules you implement). Then read `src/55_osd.cpp` fully.
- Edit ONLY `src/55_osd.cpp`. Match its style (Osd* helpers in the anonymous namespace, `osd` state, OsdScale, GDI, comment density).
- Clean room: do NOT look at, search for or reproduce code from any other window manager or task switcher.
- Never run the mod or hawktile-dev.exe. Running `build.ps1 -Test` IS allowed and expected.

## What to change
The overview (hold left Alt) currently highlights only the hovered badge or icon. Change it so that:

1. Column hover. While nothing is pressed, hover = the column zone under the mouse (any y; use the existing OsdHitTest col). `osd.hoverItem` is no longer needed for painting; remove it or keep it only if still useful. In WM_MOUSEMOVE (not pressed) set hoverCol = hit.col (-1 outside) and repaint on change. WM_MOUSELEAVE clears it as today.
2. Paint: replace the badge-disc / icon-band hover with ONE rounded rectangle over the whole hovered column, same geometry as the drop-target highlight (zone width inset by 1 px, from pad/2 to height - pad/2, corner OsdScale(kOsdHighlightCornerDip)), drawn before the badges. Fill OsdBlend(theme.background, theme.activeFill, 0.15) — a lighter tint than the drop target's 0.30. High contrast: 1-px frame in theme.activeFill, no fill. Not drawn while pressed/dragging (the drop target highlight stays as is). Factor the shared rounded-column rectangle into a helper used by both.
3. Cursor: in the overview, IDC_HAND anywhere inside a column zone (i.e. anywhere on the panel), IDC_SIZEALL while dragging as today.
4. Press: WM_LBUTTONDOWN anywhere in a column that is not on an icon counts as the badge (pressItem = kOsdBadgeItem), so clicking anywhere in a column switches to it. Clear hover on press (hoverCol = -1). The click test at release must use the same mapping (a release in the same column, not on an icon, matches a badge press).
5. OsdRender: hover must only come from real mouse movement. Today it resets hover; instead: if hoverCol >= 0 before the re-render, after the new layout re-hit-test at the current cursor (GetCursorPos + ScreenToClient on osd.hwnd, after SetWindowPos) and set hoverCol to that column (-1 if outside); if there was no hover, keep none. So a panel opening under a resting cursor has no hover.
6. CloseWorkspaceOverview(false): when no press is in progress and hoverCol >= 0 and osd.mon is valid, remember col and mon, then do the normal close (hide, click-through, clear state) and AFTER hiding call SwitchWorkspace(mon, col) (it is a no-op for the active workspace except that it may call ShowWorkspaceOsd — which then shows the normal timed popup; that is acceptable and gives feedback). HT_LOG(L"overview: released over workspace %d", col + 1). cancelDrag == true never switches. The closeAfterPress path in WM_LBUTTONUP must not switch by hover: hover is already -1 there because a press clears it — verify that holds (also for WM_CAPTURECHANGED).

## Verify
- `powershell -NoProfile -File <repo>\build.ps1 -Check 55_osd.cpp` → errors=0 warnings=0.
- Full `... build.ps1` → both clang lines errors=0 warnings=0. `... build.ps1 -Test` → 0 failed.

## Report
Under 150 words: changes, verification results line by line, and anything ambiguous.
````

## Report from the agent (2026-10-14 05:42 UTC)

````text
Overview hover/press/release changes are in src/55_osd.cpp only.

Changes:
- hoverItem removed; hoverCol is the column under the mouse (-1 outside), set in WM_MOUSEMOVE when not pressed.
- New helper OsdPaintColumnZone shared by the drop target (tint 0.30, 2-px frame in high contrast) and the hover (tint 0.15, 1-px frame). The badge-disc and icon-band hover is gone; kOsdHoverGrowDip removed.
- WM_SETCURSOR: IDC_SIZEALL while dragging, IDC_HAND anywhere in a column, else arrow.
- WM_LBUTTONDOWN: any press in a column that is not on an icon is the badge; clears hover and invalidates. WM_LBUTTONUP maps a release the same way.
- OsdRender keeps hover and re-hit-tests at the cursor after SetWindowPos; no hover means none.
- CloseWorkspaceOverview(false) captures hover and mon, hides, then SwitchWorkspace(mon, col) with HT_LOG. cancelDrag never switches. Verified: the press clears hover, so the closeAfterPress path and WM_CAPTURECHANGED cannot switch by hover.

Verification:
- build.ps1 -Check 55_osd.cpp: clang exit=0 errors=0 warnings=0
- build.ps1: both clang lines errors=0 warnings=0
- build.ps1 -Test: 1035 passed, 0 failed

Ambiguous or beyond the spec:
- WM_SETCURSOR now returns IDC_SIZEALL while dragging. Without this, WM_SETCURSOR would flip the cursor to HAND on each move during a drag.
- Hover is not re-set on the release, so after a click the column stays unhovered until the next real mouse move.
````
