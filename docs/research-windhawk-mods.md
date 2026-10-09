# Windhawk mods research: lessons for HawkTile

*2026-10-09. Sources: about 35 mods from ramensoftware/windhawk-mods (`mods/`), the Windhawk
wiki, the repo README, `.github/pr_validation.py` and the PR template. Clean-room rule: for mods
that manage window layout ("readme only" below), only the metadata, readme and settings blocks were
read. None of their code was read, and none of it is quoted. "HawkTile today" refers to `src/` at v0.6 (development).*

## 1. Mods reviewed

| Mod | What it does | What HawkTile can learn | Read |
|---|---|---|---|
| [desktop-window-cards] | Tool mod: minimized windows become thumbnail cards | The best-structured tool mod we found. It uses a worker thread with a message-only controller and **posts WinEvents to its loop** to avoid re-entrancy. It keeps a crash-recovery flag in mod storage, detects VD switches with `IVirtualDesktopManager` and `RegNotifyChangeKeyValue`, restores windows with `SC_RESTORE` after `AllowSetForegroundWindow`, has UIPI fallbacks, runs a PMv2 thread and measures frame insets per window | code |
| [minimize-to-tray] | Tool mod: hides windows into tray icons | **Puts a window property on every window before hiding it**. At startup `EnumWindows` finds and shows marked windows. It restores all windows on uninit and re-adds icons on TaskbarCreated | code |
| [dynamic-island-for-windows] | Tool mod: topmost layered overlay | Re-registers the shell hook on TaskbarCreated. Lets SHELLHOOK and TaskbarCreated through UIPI with `ChangeWindowMessageFilterEx`. Installs its LL hook only while needed and only posts from it. Posts hotkey (un)registration to the owner thread. Migrates old values in mod storage | code |
| [hide-taskbar-only-on-desktop] | Tool mod: hides taskbars per monitor | Its fullscreen test: no WS_CAPTION/WS_THICKFRAME, rect equal to rcMonitor ±2 px, shell classes excluded. Reads auto-hide with `ABM_GETSTATE`. Uses a top-level (not message-only) window so it receives TaskbarCreated | code |
| [taskbar-shadow-border] | Tool mod: layered window over the taskbar | A LOCATIONCHANGE hook **scoped to the taskbar's pid/tid** plus a debounce timer. Re-asserts HWND_TOPMOST on every update | code |
| [numbered-taskbar] | Tool mod: numbers over taskbar icons | A topmost layered overlay drawn with `UpdateLayeredWindow`, shown only while Win is held | code |
| [gather-windows-to-monitor] | Tool mod: move or gather windows to displays | Logs a skip reason for every window it leaves alone. Numbers displays by DISPLAY name, with an override. When two actions share a hotkey, the older action keeps it. Its readme has "Updating ..." notes for changed settings | code |
| [move-window-to-monitor] | Tool mod: hotkeys move a window to a monitor | Same thread model as HawkTile: settings changes are posted to the thread that owns the hotkeys | code |
| [virtual-desktop-helper] | Tool mod: virtual desktop hotkeys | `@include` of windhawk.exe + windhawk-mod.exe + **windhawk-mod-uiaccess.exe**. Its undocumented VD COM needs a "Windows version" setting | code |
| [theme-toggler-tray] | Tool mod: tray toggle | Minimal tray icon, re-added on TaskbarCreated | code |
| [command-on-window-focus] | Tool mod: runs a command on focus | Reads arrays with `Wh_GetStringSetting(L"x[%d]")` until the value is empty | code |
| [simple-window-switcher], [overhaulded-alt-tab] | Alt+Tab replacements | Alt-Tab eligibility rules: WS_EX_APPWINDOW overrides NOACTIVATE and owner checks, tool windows in the owner chain, ghost windows of hung apps, VD-aware cloaking | code |
| [center-new-windows] | Centers new windows (injected into apps) | Curated ignore lists. Apps that restore their saved position about 0.7 s after showing need a grace period. Explains UWP frame vs CoreWindow and DPI-virtualization pitfalls | code |
| [prevent-focus-stealing] | Blocks focus stealing per app | Settings with arrays of objects that contain string arrays | code |
| [disable-windows-shortcuts] | Frees or blocks Win+ shortcuts | Hooks `RegisterHotKey` in explorer.exe and returns `ERROR_HOTKEY_ALREADY_REGISTERED`. Hard-wired combos need an LL hook in dwm.exe. Uses vkE8 and vkFF mask keys | code |
| [keyboard-shortcut-actions] | Hotkeys trigger actions | A hotkey text grammar. Logs the result of every registration. "Run app" and "send keys" actions | code |
| [taskbar-desktop-indicator], [taskbar-vd-switcher] | VD indicator and buttons inside the taskbar (XAML) | Indicator UX ideas: markers, names, hide when there is one desktop, an auto grid. Its readme says renamed keys are *not* migrated | code |
| [taskbar-count-badges], [taskbar-numberer] | XAML badges on taskbar buttons | XAML gives native placement but depends on symbols and must run in explorer.exe. A separate window is right for a tool mod | skim |
| [taskbar-z-order-override], [taskbar-on-top] | Taskbar z-order and position (explorer hooks) | Explorer re-promotes the taskbar with `SetWindowPos(HWND_TOPMOST)` and window bands, so overlays must re-raise themselves | skim |
| [alt-tab-per-monitor] | Per-monitor Alt+Tab | A good companion to per-monitor workspaces | skim |
| [win11-accent-border], [window-border-customizer] | Border colors | Hooks `DwmSetWindowAttribute(DWMWA_BORDER_COLOR)` in every process, which fights our focus border | skim |
| [hyprland-windows] | Win+mouse move/resize, title bars, borders, Win+Tab | Feature ideas (§3) and a model readme "Conflicts" section | readme only |
| [tiling-helper] | On-demand master-stack tiling | Feature ideas (§3) | readme only |
| [gnome-dynamic-desktops] | GNOME-style dynamic desktops | Feature idea (§3) | readme only |
| [slick-window-arrangement], [edge-snap-throw] | Sliding and throw animations, snapping | Feature ideas (§3) | readme only |
| [snap-commander] | Rectangle-style snap hotkeys | Feature ideas. Its default keys clash with ours | readme only |
| [two-sided-snapping], [snap-restore-position], [windows-11-native-four-column-snap-layout], [window-center-resize] | Snap and placement tweaks | Feature ideas | readme only |
| [taskbar-top-window-guard] | Keeps windows out from under a top taskbar | Readme points: recheck a window for about 1.5 s after it shows; cache auto-hide state instead of doing Explorer IPC in hot paths; skip hung apps | readme only |

## 2. Windhawk best practice, prioritized

1. **Post WinEvents to the message loop instead of acting in the callback.** `WinEventProc`
   (60_engine.cpp) changes the model directly: `UnmanageWindow`, `OnFocusChanged`,
   `OnUserDragEnd`. Out-of-context events arrive whenever the engine thread pumps messages. That
   includes the time it waits inside a cross-thread send (the OSD's
   `SendMessageTimeoutW(WM_GETICON)`), modal loops, and any future COM call. A model change can
   therefore land in the middle of a retile or the OSD loop. Instead, post `(event, hwnd)` to the
   engine window, or push it onto a queue, and handle it in `EngineWndProc`. Filter frequent events
   cheaply in the callback first. Source: the comment above WinEventProc in [desktop-window-cards].
2. **Mark hidden windows with a window property.** Call `SetPropW(hwnd, L"HawkTile.Hidden", 1)`
   *before* `ShowWindowAsync(SW_HIDE)`, and don't hide the window if the property can't be set. At
   startup, `EnumWindows` → for each hidden window that has the property → show it and remove the
   property. This beats the `hwnd:pid` list in mod storage: there is no HWND-reuse guesswork, and a
   failed storage write loses nothing. Keep the list as a second safety net.
   Source: [minimize-to-tray] (`HIDDEN_WINDOW_PROPERTY`, `RecoverWindowProc`).
3. **Handle Explorer restarts with `RegisterWindowMessageW(L"TaskbarCreated")`.** The engine
   window is top-level, so it gets the broadcast. When it arrives:
   - rebuild the bars at once (today it can take up to 1 s, the poll interval) and re-raise them;
   - re-install the taskbar-scoped hooks (item 4) and re-create COM objects (item 5);
   - re-register the shell hook window. It costs little, and [dynamic-island-for-windows] does it.

   If HawkTile ever runs at a higher integrity level (UIAccess or elevated host), let
   TaskbarCreated and SHELLHOOK through UIPI with `ChangeWindowMessageFilterEx`, as
   [dynamic-island-for-windows] does. See also [theme-toggler-tray] and [hide-taskbar-only-on-desktop].
4. **Track the taskbar in real time, not by polling every second.**
   - **Follow the taskbar:** call `SetWinEventHook(EVENT_OBJECT_LOCATIONCHANGE, …, pid, tid,
     WINEVENT_OUTOFCONTEXT)` for each taskbar's thread and debounce through `kTimerBar`. This follows
     auto-hide slide-in and taskbar moves ([taskbar-shadow-border]).
   - **Keep re-raising:** keep re-raising on `EVENT_SYSTEM_FOREGROUND`, because Explorer re-promotes
     the taskbar with `HWND_TOPMOST` ([taskbar-z-order-override]).
   - **Fullscreen test (needed fix):** our `BarFullscreenOn` only compares rectangles. A maximized
     window under an auto-hidden taskbar covers rcMonitor, so it counts as "fullscreen" and the bar
     never appears when the taskbar slides in. Add the style test from
     [hide-taskbar-only-on-desktop]: fullscreen only if there is no `WS_CAPTION`/`WS_THICKFRAME`,
     the rect equals rcMonitor ±2 px, and shell classes are excluded.
   - **Auto-hide reads:** call `SHAppBarMessage(ABM_GETSTATE)` only outside hot paths. It is
     synchronous Explorer IPC ([taskbar-top-window-guard] readme).
   - **Extra signal (our idea):** `SHQueryUserNotificationState` gives a cheap session-wide hint
     (QUNS_RUNNING_D3D_FULL_SCREEN, QUNS_PRESENTATION_MODE). None of the reviewed mods use it.
5. **Be aware of Windows virtual desktops.** Windows on another virtual desktop are cloaked with
   `DWM_CLOAKED_SHELL`. Today, a desktop switch makes `ProcessPending` unmanage every window, and
   they are re-adopted (appended) when the user returns, so order, workspace and ratios are lost.
   - **Park, don't drop:** treat CLOAKED_SHELL windows as "parked", not gone.
   - **Ask Windows:** use the documented `IVirtualDesktopManager::IsWindowOnCurrentVirtualDesktop`.
     Re-create it after `RPC_E_DISCONNECTED` or `RPC_S_SERVER_UNAVAILABLE` (Explorer restarted).
   - **Detect switches:** use `RegNotifyChangeKeyValue` on `HKCU\…\Explorer\VirtualDesktops` (Win11)
     and on `…\SessionInfo\<session>\VirtualDesktops` (Win10).

   Avoid the undocumented internal VD interfaces: mods that use them need a "Windows version"
   setting. Sources: [desktop-window-cards], [simple-window-switcher], [virtual-desktop-helper].
6. **Window-handling robustness.**
   - **Restoring minimized windows:** call `AllowSetForegroundWindow(pid)` and then
     `PostMessage(WM_SYSCOMMAND, SC_RESTORE)`. Fall back to `ShowWindowAsync` if UIPI blocks the
     post. Restoring a window forcibly from outside makes Chromium/Electron re-minimize themselves
     ([desktop-window-cards] `RestoreTarget`). This affects `FocusWindow` and the `tileMaximized`
     restore.
   - **Hung apps:** ignore windows of class `Ghost` (the stand-in Windows shows for a hung app) and
     don't place `IsHungAppWindow` targets ([simple-window-switcher]; [taskbar-top-window-guard] readme).
   - **WS_EX_APPWINDOW:** an owned or `WS_EX_NOACTIVATE` window that also has `WS_EX_APPWINDOW` is a
     real app window, so tile it ([simple-window-switcher], [desktop-window-cards]).
   - **More ignore entries:** borrow from [center-new-windows]' lists.
     - Classes: `Microsoft.UI.Content.PopupWindowSiteBridge`, `DropdownWindow`, `MsoSplash`,
       `Windows.UI.Input.InputSite.WindowClass`.
     - Processes: textinputhost, searchhost, startmenuexperiencehost, shellexperiencehost, lockapp,
       widgets.
   - **Apps that move themselves:** some apps re-apply their saved position after they appear
     (Firefox after about 0.7 s). If a newly managed tiled window moves within about 1.5 s and the
     user isn't dragging it, put it back in its tile ([center-new-windows];
     [taskbar-top-window-guard] readme).
   - **Icons:** don't fetch icons synchronously on every OSD show. Three 50 ms `SendMessageTimeout`
     calls per window can stall the engine behind hung apps. Cache icons per HWND and fill the cache
     with `SendMessageCallbackW(WM_GETICON)`.
   - **Restore position:** if "restore from maximize" should land on the tile, use
     `SetWindowPlacement` with `WPF_ASYNCWINDOWPLACEMENT`. Note that `rcNormalPosition` is in
     workspace coordinates ([desktop-window-cards]).
7. **Session end.** Windhawk does not call `Wh_ModUninit` when the host process exits at sign-out
   or shutdown ([Global objects] wiki). Handle `WM_QUERYENDSESSION`/`WM_ENDSESSION` on the engine
   window by showing every hidden window again, so "Save changes?" prompts can be reached. This is
   our inference: no reviewed mod does it. HawkTile's globals only own heap memory, so the
   destructors that run at `ExitProcess` are safe. Keep GDI+ objects out of statics.
8. **Logging.**
   - **Use `Wh_Log` directly:** make `HT_LOG` expand to `Wh_Log(fmt, ...)`. The macro adds
     `[line:function]` and only evaluates its arguments when the user turns on logging in the mod's
     Advanced tab. `LogLine` formats every message even when logging is off, and loses that line
     information.
   - **File log:** keep writing the file only behind the `debugLog` setting.
   - **Skip reasons:** log *why* a window wasn't tiled, using a reason enum like
     [gather-windows-to-monitor]'s `SkipReasonText`. Users will paste exactly this into issues.

   Source: the `Wh_Log` section of [Creating a new mod]; `windhawk_api.h`.
9. **Tool-mod lifecycle.**
   - **Mutex wait:** HawkTile waits for the mutex (v0.3.1), unlike the wiki snippet, which exits
     when the mutex already exists. Every reviewed tool mod uses the unmodified snippet. Keep our
     bounded 15 s wait, because it fixes the recompile race, but explain it in the PR.
   - **Windhawk 2.0 / UIAccess:** the snippet keeps working on 2.0, where Windhawk launches the host
     itself. Add `@include windhawk-mod.exe` and `@include windhawk-mod-uiaccess.exe`. This is an
     allowed combination and [virtual-desktop-helper] uses it. On 2.0, HawkTile then runs with
     UIAccess and can move elevated windows instead of auto-floating them.
   - **Include and architecture rules:** never add `explorer.exe`, because a tool mod must use
     exactly one allowed include set. Never add `@architecture`.

   Sources: [Mods as tools] wiki, [pr_validation].
10. **Settings.**
    - **Renamed keys:** don't rename keys once users have them. Seen during development (Windhawk
      1.7.3): after renaming a setting, the old key stays in
      `HKLM\SOFTWARE\Windhawk\Engine\Mods\<mod id>\Settings` and the new key starts at its
      default, so **customised values are silently dropped**. (HawkTile renamed its settings
      before its first public release; from now on keys stay stable.)
    - **Fix:** read the old key when the new one is empty or still at its default, or add an
      "Upgrading" readme note ([taskbar-vd-switcher], [gather-windows-to-monitor]). Fix names and
      nested groups ([hyprland-windows]) before the first catalog release.
    - **2.0 editor hints:** add them behind the `#! ` marker, so 1.7.3 still validates and installs
      the mod.
      - `#! $format: colorRgb` for `focusBorderColor`: the picker stores `4C9AFF` without `#`.
      - `#! $min`/`$max` (plus `slider`) for gaps, ratios and the duration.
      - `#! $showIf` to hide indicator options while the indicator is off.
      - `#! $format: hotkey` for `keys`: it stores `alt+72` (a decimal VK), so ParseHotkey should
        read a number of 2+ digits as a VK code.
    - **Clamping:** still clamp every value after reading, because 1.7.3 enforces none of these.
    - **DPI:** state pixel settings as "px at 100% scaling" and scale them per monitor DPI
      ([desktop-window-cards], [hyprland-windows]). Gaps are raw pixels today.
    - **Reading strings:** `WindhawkUtils::StringSetting` (`windhawk_utils.h`) frees them
      automatically.

    Source: [Creating a new mod] wiki (settings annotations, 1.7.3 compatibility).
11. **Hotkeys.**
    - **Keep `RegisterHotKey`.** It fires whatever the integrity level of the foreground window,
      while a medium-IL `WH_KEYBOARD_LL` hook doesn't see keys typed into elevated windows (UIPI)
      and sits on the system input path ([dynamic-island-for-windows] comments).
    - **Keep the vkE8 mask key.** It is the same trick as [disable-windows-shortcuts], which uses
      vkFF for a lone Win press.
    - **Win+ combos (M4):** build a *separate* explorer.exe mod that hooks `RegisterHotKey`, returns
      FALSE with `ERROR_HOTKEY_ALREADY_REGISTERED` for the chosen combos, then asks for an Explorer
      restart ([disable-windows-shortcuts]). Combos that Windows handles outside RegisterHotKey
      (Win+L, Win+arrows, Win+Tab) need its LL hook in dwm.exe.
    - **Conflicts:** log error 1409 together with the key text.
12. **Submission checklist** ([README], [pr_validation], [PR template], [Review process]).
    - **Metadata:**
      - add `@github https://github.com/<you>` (required; it must match the PR author);
      - `@compilerOptions` may contain only `-l…` flags;
      - `@license` must be an SPDX id;
      - the name must be 6–68 characters and the description 30–250.
    - **Each update:** the PR contains the single file `mods/hawktile.wh.cpp`; bump `@version`;
      fill the changelog markers in the PR description.
    - **Content:** readme images only from i.imgur.com or raw.githubusercontent.com; no BOM or hidden
      characters (the dist file is clean); `.clang-format` style is Chromium with IndentWidth 4.
    - **Review flow:** tick the authorship box (AI-assisted). Run `/ai-review` and answer its
      findings before `/ready-for-reviewer`.

## 3. Feature ideas, prioritized

1. **Readme "Conflicts" section**, in the format of [hyprland-windows]. Overlapping mods and
   default-key clashes:
   - [virtual-desktop-helper]: Alt+1–9 and Alt+Shift+1–9;
   - [snap-commander]: Alt+H/J/K/M/T/Q/W/E/R…;
   - [tiling-helper]: Alt+D/R;
   - [hyprland-windows]: borders and Win+drag;
   - [win11-accent-border]: borders;
   - [center-new-windows] and [taskbar-top-window-guard]: they move new windows;
   - [slick-window-arrangement]: it slides windows after a drag.
2. **Taskbar indicator styles** ([taskbar-desktop-indicator], [taskbar-vd-switcher]): a markers
   mode (⬤/○ with dimming), workspace names, hide when only one workspace is in use, and an auto grid
   for many workspaces on tall taskbars.
3. **Show the OSD while the modifier key is held** ([numbered-taskbar] shows numbers only while Win
   is down).
4. **Tray icon** with pause/resume, retile and the current workspace ([theme-toggler-tray]; re-add
   it on TaskbarCreated).
5. **Dynamic workspaces**: always exactly one empty workspace at the end; empty ones are removed
   once you leave them ([gnome-dynamic-desktops], readme only).
6. **Remember ratios per monitor and per virtual desktop, plus a manual "tile now" command**
   ([tiling-helper], readme only).
7. **Floating-window toolkit** ([snap-commander], [window-center-resize], readme only):
   - halves, corners, center and "almost maximize" (fill the screen but keep the gaps);
   - size cycling and "restore the pre-snap size";
   - gap distribution: even, full on every side, or screen edges only;
   - exe rules that also apply to already-open windows when settings are saved;
   - one global modifier setting combined with per-action keys.
8. **Hyprland-style extras** ([hyprland-windows], readme only):
   - move with mod+drag and resize with mod+right-drag, anywhere on the window;
   - active and inactive border colors, with an `accent` keyword and a fade;
   - a hide-title-bar toggle with an exception list;
   - cycling to the next occupied workspace;
   - magnetic edges for floating windows.
9. **Stable monitor identity** ([gather-windows-to-monitor]): number monitors by DISPLAY name, with
   an override, and keep workspaces across reconnects. Add `movetomonitor N` and `exec <cmd>`
   commands ([keyboard-shortcut-actions]).
10. **Motion** ([slick-window-arrangement], [edge-snap-throw], readme only): an optional animated
    retile. For our own windows, cloaking them and calling `DwmFlush` before revealing avoids
    flicker ([desktop-window-cards]).
11. **Per-app exceptions for `followActivation`**, using the per-app list UX of [prevent-focus-stealing].

[desktop-window-cards]: https://github.com/ramensoftware/windhawk-mods/blob/main/mods/desktop-window-cards.wh.cpp
[minimize-to-tray]: https://github.com/ramensoftware/windhawk-mods/blob/main/mods/minimize-to-tray.wh.cpp
[dynamic-island-for-windows]: https://github.com/ramensoftware/windhawk-mods/blob/main/mods/dynamic-island-for-windows.wh.cpp
[hide-taskbar-only-on-desktop]: https://github.com/ramensoftware/windhawk-mods/blob/main/mods/hide-taskbar-only-on-desktop.wh.cpp
[taskbar-shadow-border]: https://github.com/ramensoftware/windhawk-mods/blob/main/mods/taskbar-shadow-border.wh.cpp
[numbered-taskbar]: https://github.com/ramensoftware/windhawk-mods/blob/main/mods/numbered-taskbar.wh.cpp
[gather-windows-to-monitor]: https://github.com/ramensoftware/windhawk-mods/blob/main/mods/gather-windows-to-monitor.wh.cpp
[move-window-to-monitor]: https://github.com/ramensoftware/windhawk-mods/blob/main/mods/move-window-to-monitor.wh.cpp
[virtual-desktop-helper]: https://github.com/ramensoftware/windhawk-mods/blob/main/mods/virtual-desktop-helper.wh.cpp
[theme-toggler-tray]: https://github.com/ramensoftware/windhawk-mods/blob/main/mods/theme-toggler-tray.wh.cpp
[command-on-window-focus]: https://github.com/ramensoftware/windhawk-mods/blob/main/mods/command-on-window-focus.wh.cpp
[simple-window-switcher]: https://github.com/ramensoftware/windhawk-mods/blob/main/mods/simple-window-switcher.wh.cpp
[overhaulded-alt-tab]: https://github.com/ramensoftware/windhawk-mods/blob/main/mods/overhaulded-alt-tab.wh.cpp
[center-new-windows]: https://github.com/ramensoftware/windhawk-mods/blob/main/mods/center-new-windows.wh.cpp
[prevent-focus-stealing]: https://github.com/ramensoftware/windhawk-mods/blob/main/mods/prevent-focus-stealing.wh.cpp
[disable-windows-shortcuts]: https://github.com/ramensoftware/windhawk-mods/blob/main/mods/disable-windows-shortcuts.wh.cpp
[keyboard-shortcut-actions]: https://github.com/ramensoftware/windhawk-mods/blob/main/mods/keyboard-shortcut-actions.wh.cpp
[taskbar-desktop-indicator]: https://github.com/ramensoftware/windhawk-mods/blob/main/mods/taskbar-desktop-indicator.wh.cpp
[taskbar-vd-switcher]: https://github.com/ramensoftware/windhawk-mods/blob/main/mods/taskbar-vd-switcher.wh.cpp
[taskbar-count-badges]: https://github.com/ramensoftware/windhawk-mods/blob/main/mods/taskbar-count-badges.wh.cpp
[taskbar-numberer]: https://github.com/ramensoftware/windhawk-mods/blob/main/mods/taskbar-numberer.wh.cpp
[taskbar-z-order-override]: https://github.com/ramensoftware/windhawk-mods/blob/main/mods/taskbar-z-order-override.wh.cpp
[taskbar-on-top]: https://github.com/ramensoftware/windhawk-mods/blob/main/mods/taskbar-on-top.wh.cpp
[alt-tab-per-monitor]: https://github.com/ramensoftware/windhawk-mods/blob/main/mods/alt-tab-per-monitor.wh.cpp
[win11-accent-border]: https://github.com/ramensoftware/windhawk-mods/blob/main/mods/win11-accent-border.wh.cpp
[window-border-customizer]: https://github.com/ramensoftware/windhawk-mods/blob/main/mods/window-border-customizer.wh.cpp
[hyprland-windows]: https://github.com/ramensoftware/windhawk-mods/blob/main/mods/hyprland-windows.wh.cpp
[tiling-helper]: https://github.com/ramensoftware/windhawk-mods/blob/main/mods/tiling-helper.wh.cpp
[gnome-dynamic-desktops]: https://github.com/ramensoftware/windhawk-mods/blob/main/mods/gnome-dynamic-desktops.wh.cpp
[slick-window-arrangement]: https://github.com/ramensoftware/windhawk-mods/blob/main/mods/slick-window-arrangement.wh.cpp
[edge-snap-throw]: https://github.com/ramensoftware/windhawk-mods/blob/main/mods/edge-snap-throw.wh.cpp
[snap-commander]: https://github.com/ramensoftware/windhawk-mods/blob/main/mods/snap-commander.wh.cpp
[two-sided-snapping]: https://github.com/ramensoftware/windhawk-mods/blob/main/mods/two-sided-snapping.wh.cpp
[snap-restore-position]: https://github.com/ramensoftware/windhawk-mods/blob/main/mods/snap-restore-position.wh.cpp
[windows-11-native-four-column-snap-layout]: https://github.com/ramensoftware/windhawk-mods/blob/main/mods/windows-11-native-four-column-snap-layout.wh.cpp
[window-center-resize]: https://github.com/ramensoftware/windhawk-mods/blob/main/mods/window-center-resize.wh.cpp
[taskbar-top-window-guard]: https://github.com/ramensoftware/windhawk-mods/blob/main/mods/taskbar-top-window-guard.wh.cpp
[Mods as tools]: https://github.com/ramensoftware/windhawk/wiki/Mods-as-tools:-Running-mods-in-a-dedicated-process
[Creating a new mod]: https://github.com/ramensoftware/windhawk/wiki/Creating-a-new-mod
[Global objects]: https://github.com/ramensoftware/windhawk/wiki/Global-objects-and-process-shutdown
[Review process]: https://github.com/ramensoftware/windhawk/wiki/Mod-submission-review-process
[README]: https://github.com/ramensoftware/windhawk-mods/blob/main/README.md
[pr_validation]: https://github.com/ramensoftware/windhawk-mods/blob/main/.github/pr_validation.py
[PR template]: https://github.com/ramensoftware/windhawk-mods/blob/main/.github/pull_request_template.md
