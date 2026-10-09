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
