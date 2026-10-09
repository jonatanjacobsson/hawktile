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
