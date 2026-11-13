// =============================================================================
// 57_stage.cpp — Mainstage side tiles: one small window per side window, showing
// a live, scaled-down DWM thumbnail of it. Semantics: 10_contract.h.
// =============================================================================

namespace ht {

namespace {

constexpr DWORD kStageCornerAttr = 33;  // DWMWA_WINDOW_CORNER_PREFERENCE
constexpr DWORD kStageCornerRound = 2;  // DWMWCP_ROUND
constexpr UINT kStageRefitDelayMs = 250;
constexpr UINT_PTR kStageHoverTimer = 1;  // on a tile window, while hovered
constexpr UINT kStageHoverPollMs = 15;
constexpr ULONGLONG kStageZoomHoldMs = 500;

struct StageProxy {
    HWND source = nullptr;        // the real window, stacked on the stage
    HWND hwnd = nullptr;          // our tile window
    HTHUMBNAIL thumb = nullptr;   // null when DWM refused the thumbnail
    RECT cell{};                  // where the tile is, in screen pixels
    RECT drawn{};                 // the thumbnail inside the tile, client pixels
};

struct StageState {
    HINSTANCE hinst = nullptr;
    bool classRegistered = false;
    bool failureLogged = false;
    COLORREF background = RGB(32, 32, 32);
    std::vector<StageProxy> tiles;
    // Left button held on a tile: a click until the cursor leaves the drag
    // threshold, then a drag of the tile.
    HWND pressTile = nullptr;
    POINT pressCursor{};          // screen position at button down
    bool pressDouble = false;     // the press was the second of a double click
    bool dragging = false;
    // Left Alt held over a tile: after kStageZoomHoldMs the tile shows its
    // window at full size. The cursor is then held in place and mouse movement
    // drags the picture under it.
    HWND hoverTile = nullptr;     // the tile running kStageHoverTimer
    ULONGLONG altSince = 0;       // tick when Alt was first seen down; 0 = up
    HWND zoomTile = nullptr;      // the zoomed tile
    POINT zoomAt{};               // where the cursor is held, tile client pixels
    POINT zoomLock{};             // the same point on screen
    double zoomU = 0.0;           // the point of the window shown at zoomAt, as
    double zoomV = 0.0;           // fractions of its visible frame
};

StageState stage;

StageProxy* StageFindByTile(HWND hwnd) {
    for (StageProxy& tile : stage.tiles) {
        if (tile.hwnd == hwnd) return &tile;
    }
    return nullptr;
}

StageProxy* StageFindBySource(HWND source) {
    for (StageProxy& tile : stage.tiles) {
        if (tile.source == source) return &tile;
    }
    return nullptr;
}

// Maps a point of the tile (client pixels) through the thumbnail's scale to
// the matching screen point of the real window. False outside the thumbnail.
bool StageMapToSource(const StageProxy& tile, POINT client, POINT* screen) {
    const LONG drawnW = tile.drawn.right - tile.drawn.left;
    const LONG drawnH = tile.drawn.bottom - tile.drawn.top;
    if (drawnW <= 0 || drawnH <= 0 || !PtInRect(&tile.drawn, client) ||
        !IsWindow(tile.source)) {
        return false;
    }
    const RECT visible = GetVisibleFrame(tile.source);
    if (stage.zoomTile == tile.hwnd) {
        // Zoomed: full size around the held cursor point.
        screen->x = visible.left + (client.x - stage.zoomAt.x) +
                    std::lround(stage.zoomU * (visible.right - visible.left));
        screen->y = visible.top + (client.y - stage.zoomAt.y) +
                    std::lround(stage.zoomV * (visible.bottom - visible.top));
        return PtInRect(&visible, *screen) != FALSE;
    }
    screen->x = visible.left + MulDiv(client.x - tile.drawn.left,
                                      visible.right - visible.left, drawnW);
    screen->y = visible.top + MulDiv(client.y - tile.drawn.top,
                                     visible.bottom - visible.top, drawnH);
    return true;
}

// The innermost child window of `root` at a screen point. WindowFromPoint is no
// use here: the real window is covered by the stage window.
HWND StageDeepestChild(HWND root, POINT screen) {
    HWND current = root;
    for (int depth = 0; depth < 32; ++depth) {
        POINT pt = screen;
        ScreenToClient(current, &pt);
        const HWND child = ChildWindowFromPointEx(
            current, pt,
            CWP_SKIPINVISIBLE | CWP_SKIPDISABLED | CWP_SKIPTRANSPARENT);
        if (!child || child == current) break;
        current = child;
    }
    return current;
}

// The window that should get a forwarded mouse message for tile point `client`,
// and the point in its client coordinates and in screen coordinates. Null
// outside the thumbnail and outside that window's client area.
HWND StageForwardTarget(const StageProxy& tile, POINT client, POINT* inTarget,
                        POINT* screen) {
    if (!StageMapToSource(tile, client, screen)) return nullptr;
    const HWND target = StageDeepestChild(tile.source, *screen);
    *inTarget = *screen;
    ScreenToClient(target, inTarget);
    RECT area{};
    GetClientRect(target, &area);
    return PtInRect(&area, *inTarget) ? target : nullptr;
}

// Posts one click (move, down, up) into the real window. Posted, not sent: a
// hung app must not block the engine.
void StageForwardClick(const StageProxy& tile, POINT client, UINT down, UINT up,
                       WPARAM button) {
    POINT inTarget{};
    POINT screen{};
    const HWND target = StageForwardTarget(tile, client, &inTarget, &screen);
    if (!target) {
        HT_LOG(L"tile click at %ld,%ld not forwarded (thumbnail %ld,%ld-%ld,%ld)",
               client.x, client.y, tile.drawn.left, tile.drawn.top,
               tile.drawn.right, tile.drawn.bottom);
        return;
    }
    WCHAR cls[64] = {};
    GetClassNameW(target, cls, 64);
    HT_LOG(L"tile click forwarded to %p (%s%s) at %ld,%ld",
           static_cast<void*>(target), cls,
           target == tile.source ? L", the window itself" : L", a child",
           inTarget.x, inTarget.y);
    const LPARAM where = MAKELPARAM(inTarget.x, inTarget.y);
    PostMessageW(target, WM_MOUSEMOVE, 0, where);
    PostMessageW(target, down, button, where);
    PostMessageW(target, up, 0, where);
}

POINT StageClientPoint(LPARAM lp) {
    return POINT{static_cast<short>(LOWORD(lp)), static_cast<short>(HIWORD(lp))};
}

// Puts a dragged tile back in its cell, behind the other windows.
void StageRestoreTile(const StageProxy& tile) {
    SetWindowPos(tile.hwnd, HWND_NOTOPMOST, 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    SetWindowPos(tile.hwnd, HWND_BOTTOM, tile.cell.left, tile.cell.top,
                 tile.cell.right - tile.cell.left,
                 tile.cell.bottom - tile.cell.top, SWP_NOACTIVATE);
}

void StageFitThumbnail(StageProxy& tile);

void StageEndZoom() {
    const HWND before = stage.zoomTile;
    if (!before) return;
    stage.zoomTile = nullptr;
    if (StageProxy* tile = StageFindByTile(before)) StageFitThumbnail(*tile);
}

// kStageHoverTimer: runs while the cursor is over the tile and decides whether
// it is zoomed. Left Alt only, like the overview.
void StageHoverTick(HWND hwnd) {
    POINT cursor{};
    GetCursorPos(&cursor);
    const bool over = WindowFromPoint(cursor) == hwnd ||
                      (stage.pressTile == hwnd && !stage.dragging);
    if (!over || stage.dragging) {
        KillTimer(hwnd, kStageHoverTimer);
        if (stage.hoverTile == hwnd) stage.hoverTile = nullptr;
        stage.altSince = 0;
        if (stage.zoomTile == hwnd) StageEndZoom();
        return;
    }
    if (GetAsyncKeyState(VK_LMENU) >= 0) {
        stage.altSince = 0;
        if (stage.zoomTile == hwnd) StageEndZoom();
        return;
    }
    const ULONGLONG now = GetTickCount64();
    if (stage.altSince == 0) stage.altSince = now;
    if (now - stage.altSince < kStageZoomHoldMs) return;
    StageProxy* tile = StageFindByTile(hwnd);
    if (!tile) return;

    if (stage.zoomTile != hwnd) {
        // Start: the point under the cursor is the one shown at full size.
        const LONG drawnW = tile->drawn.right - tile->drawn.left;
        const LONG drawnH = tile->drawn.bottom - tile->drawn.top;
        if (drawnW <= 0 || drawnH <= 0) return;
        POINT client = cursor;
        ScreenToClient(hwnd, &client);
        client.x = std::clamp<LONG>(client.x, tile->drawn.left, tile->drawn.right);
        client.y = std::clamp<LONG>(client.y, tile->drawn.top, tile->drawn.bottom);
        StageEndZoom();
        stage.zoomTile = hwnd;
        stage.zoomAt = client;
        stage.zoomLock = client;
        ClientToScreen(hwnd, &stage.zoomLock);
        stage.zoomU = static_cast<double>(client.x - tile->drawn.left) / drawnW;
        stage.zoomV = static_cast<double>(client.y - tile->drawn.top) / drawnH;
        SetCursorPos(stage.zoomLock.x, stage.zoomLock.y);
        StageFitThumbnail(*tile);
        return;
    }
    // A held button is a click or the start of a tile drag: leave the cursor.
    if (stage.pressTile == hwnd) return;
    const int dx = cursor.x - stage.zoomLock.x;
    const int dy = cursor.y - stage.zoomLock.y;
    if (dx == 0 && dy == 0) return;
    // The picture follows the mouse; the cursor goes back to where it was.
    const RECT visible = GetVisibleFrame(tile->source);
    const LONG visibleW = std::max<LONG>(1, visible.right - visible.left);
    const LONG visibleH = std::max<LONG>(1, visible.bottom - visible.top);
    stage.zoomU = std::clamp(stage.zoomU - static_cast<double>(dx) / visibleW,
                             0.0, 1.0);
    stage.zoomV = std::clamp(stage.zoomV - static_cast<double>(dy) / visibleH,
                             0.0, 1.0);
    SetCursorPos(stage.zoomLock.x, stage.zoomLock.y);
    StageFitThumbnail(*tile);
}

LRESULT CALLBACK StageWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
        case WM_MOUSEACTIVATE:
            return MA_NOACTIVATE;

        case WM_TIMER:
            if (wp == kStageHoverTimer) StageHoverTick(hwnd);
            return 0;

        case WM_LBUTTONDOWN:
        case WM_LBUTTONDBLCLK:
            stage.pressTile = hwnd;
            stage.pressDouble = msg == WM_LBUTTONDBLCLK;
            stage.dragging = false;
            GetCursorPos(&stage.pressCursor);
            SetCapture(hwnd);
            return 0;

        case WM_MOUSEMOVE: {
            if (stage.hoverTile != hwnd) {
                if (stage.hoverTile) KillTimer(stage.hoverTile, kStageHoverTimer);
                StageEndZoom();
                stage.hoverTile = hwnd;
                stage.altSince = 0;
                SetTimer(hwnd, kStageHoverTimer, kStageHoverPollMs, nullptr);
            }
            if (stage.pressTile != hwnd || GetCapture() != hwnd) return 0;
            const StageProxy* tile = StageFindByTile(hwnd);
            if (!tile) return 0;
            POINT cursor{};
            GetCursorPos(&cursor);
            const int dx = cursor.x - stage.pressCursor.x;
            const int dy = cursor.y - stage.pressCursor.y;
            if (!stage.dragging) {
                // Generous, so a click with an unsteady hand stays a click.
                const int limitX = 2 * GetSystemMetrics(SM_CXDRAG);
                const int limitY = 2 * GetSystemMetrics(SM_CYDRAG);
                if (dx >= -limitX && dx <= limitX && dy >= -limitY &&
                    dy <= limitY) {
                    return 0;
                }
                stage.dragging = true;
                HT_LOG(L"tile drag started");
                StageEndZoom();
                // Above the other windows while it is carried. Only once: an
                // overview that opens later (Alt) must stay above the tile.
                SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0,
                             SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
            }
            SetWindowPos(hwnd, nullptr, tile->cell.left + dx, tile->cell.top + dy,
                         0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
            OverviewDropTarget(cursor, nullptr, nullptr);  // moves the highlight
            return 0;
        }

        case WM_LBUTTONUP: {
            if (stage.pressTile != hwnd) return 0;
            const bool dragged = stage.dragging;
            const bool second = stage.pressDouble;
            // Cleared first: ReleaseCapture sends WM_CAPTURECHANGED.
            stage.pressTile = nullptr;
            stage.dragging = false;
            ReleaseCapture();
            const StageProxy* tile = StageFindByTile(hwnd);
            if (!tile) return 0;
            if (dragged) {
                // Copy: the drop retiles, which may destroy this tile.
                const HWND source = tile->source;
                StageRestoreTile(*tile);
                POINT cursor{};
                GetCursorPos(&cursor);
                int mon = -1;
                int wsIndex = -1;
                if (OverviewDropTarget(cursor, &mon, &wsIndex)) {
                    HT_LOG(L"tile dropped on workspace %d", wsIndex + 1);
                    OverviewDropDone();
                    MoveWindowToWorkspace(source, mon, wsIndex);
                } else {
                    StageDropWindow(source, cursor);
                }
            } else {
                StageForwardClick(*tile, StageClientPoint(lp),
                                  second ? WM_LBUTTONDBLCLK : WM_LBUTTONDOWN,
                                  WM_LBUTTONUP, MK_LBUTTON);
            }
            return 0;
        }

        case WM_CAPTURECHANGED:
            // Capture taken away mid-press: the drag is off.
            if (stage.pressTile == hwnd) {
                const bool dragged = stage.dragging;
                stage.pressTile = nullptr;
                stage.dragging = false;
                if (dragged) {
                    OverviewDropDone();
                    if (const StageProxy* tile = StageFindByTile(hwnd)) {
                        StageRestoreTile(*tile);
                    }
                }
            }
            return 0;

        case WM_RBUTTONUP:
            if (const StageProxy* tile = StageFindByTile(hwnd)) {
                StageForwardClick(*tile, StageClientPoint(lp), WM_RBUTTONDOWN,
                                  WM_RBUTTONUP, MK_RBUTTON);
            }
            return 0;

        case WM_MBUTTONUP:
            if (const StageProxy* tile = StageFindByTile(hwnd)) {
                StageForwardClick(*tile, StageClientPoint(lp), WM_MBUTTONDOWN,
                                  WM_MBUTTONUP, MK_MBUTTON);
            }
            return 0;

        case WM_MOUSEWHEEL:
        case WM_MOUSEHWHEEL:
            if (const StageProxy* tile = StageFindByTile(hwnd)) {
                // Wheel messages carry screen coordinates, in and out.
                POINT client = StageClientPoint(lp);
                ScreenToClient(hwnd, &client);
                POINT inTarget{};
                POINT screen{};
                if (const HWND target =
                        StageForwardTarget(*tile, client, &inTarget, &screen)) {
                    PostMessageW(target, msg, wp, MAKELPARAM(screen.x, screen.y));
                }
            }
            return 0;

        case WM_ERASEBKGND: {
            RECT client{};
            GetClientRect(hwnd, &client);
            const HBRUSH brush = CreateSolidBrush(stage.background);
            FillRect(reinterpret_cast<HDC>(wp), &client, brush);
            DeleteObject(brush);
            return 1;
        }

        default:
            break;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

// Logs the first creation failure only; later calls return quietly.
bool StageFailed(const wchar_t* step, DWORD err) {
    if (!stage.failureLogged) {
        stage.failureLogged = true;
        HT_LOG(L"stage tiles: %s failed (error %lu)", step, err);
    }
    return false;
}

bool StageEnsureClass() {
    if (stage.classRegistered) return true;
    // Address of our own code identifies this module (the mod DLL).
    HINSTANCE hinst = nullptr;
    if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                                GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                            reinterpret_cast<LPCWSTR>(&StageWndProc), &hinst)) {
        return StageFailed(L"module lookup", GetLastError());
    }
    stage.hinst = hinst;

    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.style = CS_DBLCLKS;  // double clicks are forwarded as such
    wc.lpfnWndProc = StageWndProc;
    wc.hInstance = stage.hinst;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.lpszClassName = kStageWindowClass;  // no background: WM_ERASEBKGND paints
    if (!RegisterClassExW(&wc)) {
        return StageFailed(L"class registration", GetLastError());
    }
    stage.classRegistered = true;
    return true;
}

// Scales the thumbnail to fit the tile, centred, never larger than the window
// itself. The source is cropped to the visible frame (no resize borders).
// The zoomed tile instead shows the window at full size, with the point
// (stage.zoomU, stage.zoomV) of it at stage.zoomAt.
void StageFitThumbnail(StageProxy& tile) {
    if (!tile.thumb || !IsWindow(tile.source)) return;
    RECT window{};
    SIZE source{};
    if (!GetWindowRect(tile.source, &window) ||
        FAILED(DwmQueryThumbnailSourceSize(tile.thumb, &source))) {
        return;
    }
    const LONG windowW = window.right - window.left;
    const LONG windowH = window.bottom - window.top;
    if (windowW <= 0 || windowH <= 0 || source.cx <= 0 || source.cy <= 0) return;

    // DWM measures the source in its own pixels, which differ from ours for a
    // DPI-virtualised app; map the visible frame through that factor.
    const RECT visible = GetVisibleFrame(tile.source);
    const double fx = static_cast<double>(source.cx) / windowW;
    const double fy = static_cast<double>(source.cy) / windowH;
    RECT crop{std::lround((visible.left - window.left) * fx),
              std::lround((visible.top - window.top) * fy),
              std::lround((visible.right - window.left) * fx),
              std::lround((visible.bottom - window.top) * fy)};
    crop.left = std::clamp<LONG>(crop.left, 0, source.cx - 1);
    crop.top = std::clamp<LONG>(crop.top, 0, source.cy - 1);
    crop.right = std::clamp<LONG>(crop.right, crop.left + 1, source.cx);
    crop.bottom = std::clamp<LONG>(crop.bottom, crop.top + 1, source.cy);

    const LONG cellW = tile.cell.right - tile.cell.left;
    const LONG cellH = tile.cell.bottom - tile.cell.top;
    const LONG visibleW = std::max<LONG>(1, visible.right - visible.left);
    const LONG visibleH = std::max<LONG>(1, visible.bottom - visible.top);
    const double scale =
        std::min({1.0, static_cast<double>(cellW) / visibleW,
                  static_cast<double>(cellH) / visibleH});
    const LONG drawW = std::clamp<LONG>(std::lround(visibleW * scale), 1, cellW);
    const LONG drawH = std::clamp<LONG>(std::lround(visibleH * scale), 1, cellH);
    const LONG x = (cellW - drawW) / 2;
    const LONG y = (cellH - drawH) / 2;

    DWM_THUMBNAIL_PROPERTIES props{};
    props.dwFlags = DWM_TNP_RECTDESTINATION | DWM_TNP_RECTSOURCE |
                    DWM_TNP_VISIBLE | DWM_TNP_OPACITY |
                    DWM_TNP_SOURCECLIENTAREAONLY;
    tile.drawn = RECT{x, y, x + drawW, y + drawH};
    props.rcDestination = tile.drawn;
    props.rcSource = crop;
    if (stage.zoomTile == tile.hwnd && scale < 1.0) {
        // tile.drawn stays the fitted rect; StageMapToSource knows the zoom.
        const LONG left = stage.zoomAt.x - std::lround(stage.zoomU * visibleW);
        const LONG top = stage.zoomAt.y - std::lround(stage.zoomV * visibleH);
        // Only the part that lands inside the tile, and its part of the source.
        RECT dest{std::max<LONG>(left, 0), std::max<LONG>(top, 0),
                  std::min<LONG>(left + visibleW, cellW),
                  std::min<LONG>(top + visibleH, cellH)};
        if (dest.right > dest.left && dest.bottom > dest.top) {
            const double sx = static_cast<double>(crop.right - crop.left) / visibleW;
            const double sy = static_cast<double>(crop.bottom - crop.top) / visibleH;
            RECT part{crop.left + std::lround((dest.left - left) * sx),
                      crop.top + std::lround((dest.top - top) * sy),
                      crop.left + std::lround((dest.right - left) * sx),
                      crop.top + std::lround((dest.bottom - top) * sy)};
            part.right = std::clamp<LONG>(part.right, part.left + 1, crop.right);
            part.bottom = std::clamp<LONG>(part.bottom, part.top + 1, crop.bottom);
            props.rcDestination = dest;
            props.rcSource = part;
        }
    }
    props.opacity = 255;
    props.fVisible = TRUE;
    props.fSourceClientAreaOnly = FALSE;
    DwmUpdateThumbnailProperties(tile.thumb, &props);
}

void StageDestroyTile(StageProxy& tile) {
    if (stage.hoverTile == tile.hwnd) stage.hoverTile = nullptr;
    if (stage.zoomTile == tile.hwnd) stage.zoomTile = nullptr;
    if (tile.thumb) DwmUnregisterThumbnail(tile.thumb);
    if (tile.hwnd && IsWindow(tile.hwnd)) DestroyWindow(tile.hwnd);
    tile = StageProxy{};
}

// Creates the tile window for `source` at `cell`, behind every other window.
bool StageCreateTile(HWND source, const RECT& cell, StageProxy* out) {
    if (!StageEnsureClass()) return false;
    HWND hwnd = CreateWindowExW(WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE,
                                kStageWindowClass, L"HawkTile stage tile",
                                WS_POPUP, cell.left, cell.top,
                                cell.right - cell.left, cell.bottom - cell.top,
                                nullptr, nullptr, stage.hinst, nullptr);
    if (!hwnd) return StageFailed(L"window creation", GetLastError());

    // Rounded corners on Windows 11; older systems ignore the attribute.
    DWORD corner = kStageCornerRound;
    DwmSetWindowAttribute(hwnd, kStageCornerAttr, &corner, sizeof(corner));

    StageProxy tile;
    tile.source = source;
    tile.hwnd = hwnd;
    tile.cell = cell;
    const HRESULT hr = DwmRegisterThumbnail(hwnd, source, &tile.thumb);
    if (FAILED(hr)) {
        // The tile still works as a button for its window, just without a picture.
        tile.thumb = nullptr;
        StageFailed(L"thumbnail registration", static_cast<DWORD>(hr));
    }
    SetWindowPos(hwnd, HWND_BOTTOM, 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW);
    *out = tile;
    return true;
}

}  // namespace

void SyncStageProxies() {
    const std::vector<StageTile> wanted = StageSideTiles();

    // Drop tiles that lost their cell or their window.
    for (StageProxy& tile : stage.tiles) {
        bool keep = IsWindow(tile.source) && IsWindow(tile.hwnd);
        if (keep) {
            keep = false;
            for (const StageTile& want : wanted) {
                if (want.hwnd == tile.source) keep = true;
            }
        }
        if (!keep) StageDestroyTile(tile);
    }
    std::erase_if(stage.tiles,
                  [](const StageProxy& tile) { return tile.hwnd == nullptr; });
    if (wanted.empty()) return;

    const COLORREF background = ResolveTheme().background;
    const bool repaint = background != stage.background;
    stage.background = background;

    for (const StageTile& want : wanted) {
        StageProxy* tile = StageFindBySource(want.hwnd);
        if (!tile) {
            StageProxy created;
            if (!StageCreateTile(want.hwnd, want.cell, &created)) continue;
            stage.tiles.push_back(created);
            tile = &stage.tiles.back();
        } else if (!EqualRect(&tile->cell, &want.cell)) {
            tile->cell = want.cell;
            SetWindowPos(tile->hwnd, nullptr, want.cell.left, want.cell.top,
                         want.cell.right - want.cell.left,
                         want.cell.bottom - want.cell.top,
                         SWP_NOZORDER | SWP_NOACTIVATE);
        }
        if (repaint) InvalidateRect(tile->hwnd, nullptr, TRUE);
        StageFitThumbnail(*tile);
    }
    // The real windows reach their stage size asynchronously; fit once more.
    if (g.engineHwnd) {
        SetTimer(g.engineHwnd, kTimerStage, kStageRefitDelayMs, nullptr);
    }
}

void RefitStageProxies() {
    for (StageProxy& tile : stage.tiles) StageFitThumbnail(tile);
}

void DestroyStageProxies() {
    for (StageProxy& tile : stage.tiles) StageDestroyTile(tile);
    stage.tiles.clear();
    if (stage.classRegistered) {
        UnregisterClassW(kStageWindowClass, stage.hinst);
        stage.classRegistered = false;
    }
    stage.failureLogged = false;
}

}  // namespace ht
