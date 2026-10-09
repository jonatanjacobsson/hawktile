# 19. Research Windhawk mods best practices

| | |
|---|---|
| Launched | 2026-10-09 08:20 UTC |
| Role | researcher (wrote no product code) |
| Model | Claude Opus 5.5 (same model as the planner) |
| Agent id | `a80fb20c7c74cd0e8` |

## Instructions from the planner (verbatim)

````text
Research task (read-only; do not modify any file outside the scratchpad). We are building HawkTile, a clean-room tiling window manager written as a Windhawk *tool mod* (runs in its own `windhawk.exe -tool-mod` process; uses SetWinEventHook out-of-context, RegisterHotKey, SetWindowPos async, ShowWindowAsync(SW_HIDE) for workspaces, a per-pixel-alpha GDI+ overlay window above the taskbar for workspace badges, RegisterShellHookWindow for HSHELL_FLASH notification dots, DWM border colors, Windhawk settings incl. arrays of objects). Source for context: <repo> (PLAN.md and src/10_contract.h summarise the design — read them first).

Goal: learn Windhawk best practices and useful techniques from OSS mods in https://github.com/ramensoftware/windhawk-mods (folder `mods/`). Fetch files with `gh api repos/ramensoftware/windhawk-mods/contents/mods/<name> -H "Accept: application/vnd.github.raw"`; save under <scratch>\mods\ if you need to grep. Also check the Windhawk wiki (https://github.com/ramensoftware/windhawk/wiki — e.g. "Creating a new mod", "Development tips", "Mods as tools", "Injection targets and critical system processes") and the repo's contribution/review guidelines (README / CONTRIBUTING / pull request template in ramensoftware/windhawk-mods).

STRICT CLEAN-ROOM RULE: for mods that themselves implement tiling / window-layout management — at least `hyprland-windows.wh.cpp`, `tiling-helper.wh.cpp`, `gnome-dynamic-desktops.wh.cpp`, `slick-window-arrangement.wh.cpp`, `two-sided-snapping.wh.cpp`, `snap-commander.wh.cpp` — read ONLY the metadata block, the `==WindhawkModReadme==` section and the `==WindhawkModSettings==` section (stop reading at `// ==/WindhawkModSettings==`; e.g. extract with a script that prints only up to that line). Never read or quote their code. Report only their feature ideas/UX.

For these mods you may read the code to learn Windhawk/Win32 engineering practices (not to copy code): taskbar-desktop-indicator, taskbar-vd-switcher, virtual-desktop-helper, taskbar-count-badges, taskbar-numberer, keyboard-shortcut-actions, move-window-to-monitor, gather-windows-to-monitor, alt-tab-per-monitor, prevent-focus-stealing, window-border-customizer, dynamic-island-for-windows, center-new-windows, taskbar-on-top, taskbar-z-order-override, desktop-window-cards, command-on-window-focus, theme-toggler-tray, plus any other you find relevant.

Look specifically for: (1) tool-mod lifecycle details (restart races, mutex handling, settings reload, ExitProcess paths, crash safety); (2) how mods draw overlays on/near the Windows 11 taskbar (separate topmost window vs XAML injection in explorer.exe; z-order upkeep; handling explorer restarts via the "TaskbarCreated" registered message; auto-hide/fullscreen detection — e.g. SHQueryUserNotificationState or ABM_GETSTATE); (3) hotkey handling (RegisterHotKey vs low-level keyboard hook, freeing Win+ keys, Alt menu suppression); (4) window enumeration/filtering heuristics (cloaked, tool windows, UWP ApplicationFrameHost/CoreWindow, owned popups, elevated windows/UIPI); (5) DPI handling; (6) settings conventions (YAML schema, $options, arrays, naming, migrations when renaming settings); (7) logging conventions and anything reviewers ask for (code style, @architecture, @include rules, licensing).

Deliverable: write a concise report to <repo>\docs\research-windhawk-mods.md (create the docs folder) with: a table of the mods you looked at (name, what it does, what we can learn — mark the clean-room ones as "readme only"), then a prioritized list of concrete recommendations for HawkTile (each: what to change, why, which mod/wiki page showed it, with links to https://github.com/ramensoftware/windhawk-mods/blob/main/mods/<file>), clearly separating "Windhawk best practice" from "feature idea". Keep the report under ~250 lines. In your final message, return a 10-line summary of the top recommendations.
````

## Report from the agent (2026-10-09 08:37 UTC)

````text
I wrote the report to `<repo>\docs\research-windhawk-mods.md`. It is 275 lines, a bit over the ~250 target, because 46 of them are link definitions; the body is about 230. It covers about 35 mods, the Windhawk wiki, the repo README, `pr_validation.py` and the PR template, and keeps best practice (§2) separate from feature ideas (§3). Clean-room held: for hyprland-windows, tiling-helper, gnome-dynamic-desktops, slick-window-arrangement, two-sided-snapping and snap-commander, plus six other snap/placement mods, only the metadata, readme and settings blocks were extracted. Their code was never read. I changed no project files.

Top recommendations:
1. **Queue WinEvents instead of acting in the callback.** `WinEventProc` changes the model directly, so an event can arrive mid-retile or mid-OSD while the thread waits in `SendMessageTimeout(WM_GETICON)` or any future COM call. Post them to the engine window, as desktop-window-cards does.
2. **Mark hidden windows with a window property** (`SetPropW`) before `SW_HIDE`, and on startup show any window that still has it. This is more robust than the `hwnd:pid` list (minimize-to-tray does it).
3. **React to the "TaskbarCreated" message.** Rebuild and re-raise the bars at once (today it waits for the 1 s poll), re-register the shell hook and re-create COM objects. Track each taskbar with a `LOCATIONCHANGE` hook scoped to its thread.
4. **Fix the fullscreen test (likely bug).** `BarFullscreenOn` only compares rectangles, so with an auto-hidden taskbar a maximized window counts as fullscreen and the bar should never appear. The fix is to also require no `WS_CAPTION`/`WS_THICKFRAME` (hide-taskbar-only-on-desktop).
5. **Handle Windows virtual desktops.** A desktop switch currently unmanages every window and re-adopts them later, losing order, workspace and ratios. Treat `DWM_CLOAKED_SHELL` windows as parked and use the documented `IVirtualDesktopManager`.
6. **Window robustness.**
   - Restore minimized windows with `AllowSetForegroundWindow` plus `SC_RESTORE`; restoring them from outside makes Chromium/Electron re-minimize.
   - Ignore `Ghost` windows and hung apps.
   - Tile owned windows that have `WS_EX_APPWINDOW`.
   - Put windows back if the app moves them within about 1.5 s of showing (Firefox does).
   - Cache OSD icons instead of fetching them on every show.
7. **Don't rename settings keys.** In your Windhawk registry the old `bindings[...]` and `osdDuration` values are still stored, and the new keys start at defaults, so custom key bindings were silently dropped by the rename to `keybindings`. Either read the old key as a fallback or add an upgrade note. Windhawk 2.0 editor hints (color picker, hotkey box, min/max, show-if) can be added behind the `#! ` marker so 1.7.3 still installs the mod.
8. **Logging and session end.** Make `HT_LOG` call `Wh_Log` directly so it is free when logging is off and keeps line numbers, and log why a window wasn't tiled. Show hidden windows on `WM_QUERYENDSESSION`, because uninit isn't called at sign-out; this one is my own inference, no mod does it.
9. **Windhawk 2.0 and Win+ keys.** Adding `@include windhawk-mod.exe` and `windhawk-mod-uiaccess.exe` lets 2.0 run HawkTile with UIAccess, so elevated windows can be tiled; virtual-desktop-helper does this. Keep `RegisterHotKey` and the vkE8 mask key. Freeing Win+ keys (M4) has to be a separate explorer.exe mod that hooks `RegisterHotKey`, because tool mods may not include explorer.exe.
10. **Before submitting.** Add `@github` (required, must match the PR author), never add `@architecture`, run `/ai-review` before `/ready-for-reviewer`, and mention the mutex-wait change to the standard snippet in the PR. The top feature idea is a readme "Conflicts" section: virtual-desktop-helper and snap-commander default keys clash with ours, and win11-accent-border fights the focus border. The rest of §3 lists indicator styles, a tray icon, dynamic workspaces and a floating-window toolkit.
````
