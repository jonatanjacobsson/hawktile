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
