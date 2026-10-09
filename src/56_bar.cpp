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
