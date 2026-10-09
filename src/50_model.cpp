#include <cmath>

namespace ht {

EngineState g;

namespace {

int MonitorCount() {
    return static_cast<int>(g.monitors.size());
}

int WsCount() {
    return std::clamp(g.settings.workspaceCount, 1, 9);
}

Workspace MakeWorkspace() {
    Workspace ws;
    ws.layout = g.settings.defaultLayout;
    ws.prevLayout = g.settings.defaultLayout;
    return ws;
}

int IndexOf(const std::vector<HWND>& v, HWND hwnd) {
    for (size_t i = 0; i < v.size(); ++i) {
        if (v[i] == hwnd) return static_cast<int>(i);
    }
    return -1;
}

bool RemoveFrom(std::vector<HWND>& v, HWND hwnd) {
    const int i = IndexOf(v, hwnd);
    if (i < 0) return false;
    v.erase(v.begin() + i);
    return true;
}

bool InWorkspace(const Workspace& ws, HWND hwnd) {
    return hwnd && (IndexOf(ws.tiled, hwnd) >= 0 ||
                    IndexOf(ws.floating, hwnd) >= 0);
}

HWND FirstWindow(const Workspace& ws) {
    if (InWorkspace(ws, ws.lastFocus)) return ws.lastFocus;
    if (!ws.tiled.empty()) return ws.tiled.front();
    if (!ws.floating.empty()) return ws.floating.front();
    return nullptr;
}

int MonitorIndexOf(HMONITOR handle) {
    for (int i = 0; i < MonitorCount(); ++i) {
        if (g.monitors[i].handle == handle) return i;
    }
    return -1;
}

std::vector<RECT> MonitorRects() {
    std::vector<RECT> rects;
    for (const Monitor& m : g.monitors) rects.push_back(m.rcMonitor);
    return rects;
}

std::vector<RECT> FramesOf(const std::vector<HWND>& hwnds) {
    std::vector<RECT> frames;
    for (HWND h : hwnds) frames.push_back(GetVisibleFrame(h));
    return frames;
}

// Visible, non-iconic tiled windows in layout order. With appendCur, `cur`
// goes last so that directional focus can reach it while it floats.
std::vector<HWND> Candidates(const Workspace& ws, HWND cur, bool appendCur) {
    std::vector<HWND> out;
    for (HWND h : ws.tiled) {
        if (IsWindowVisible(h) && !IsIconic(h)) out.push_back(h);
    }
    if (appendCur) out.push_back(cur);
    return out;
}

// Tiled windows that Retile lays out, in layout order. Our own bookkeeping
// decides visibility, not IsWindowVisible: a window we just showed with
// ShowWindowAsync may not report visible yet.
std::vector<HWND> LayoutWindowsOf(const Workspace& ws) {
    std::vector<HWND> out;
    for (HWND h : ws.tiled) {
        if (!g.hiddenByUs.count(h) && IsWindow(h) && !IsIconic(h) &&
            !IsZoomed(h)) {
            out.push_back(h);
        }
    }
    return out;
}

// Work area of monitor `mon` inset by outerGap on every side; the plain work
// area when that inset would leave less than 50 px in either direction.
RECT LayoutAreaOf(int mon) {
    const RECT work = g.monitors[mon].rcWork;
    const int gap = g.settings.outerGap;
    RECT area{work.left + gap, work.top + gap, work.right - gap,
              work.bottom - gap};
    if (area.right - area.left < 50 || area.bottom - area.top < 50) {
        area = work;
    }
    return area;
}

// The layout parameters every geometry computation for `ws` uses.
LayoutParams LayoutParamsOf(const Workspace& ws) {
    return LayoutParams{ws.layout, g.settings.masterRatio, g.settings.innerGap,
                        ws.ratios, ws.flips};
}

void SetWorkspaceLayout(Workspace& ws, LayoutKind kind) {
    // Remember the layout we leave so togglemonocle can return to it.
    if (kind == LayoutKind::Monocle && ws.layout != LayoutKind::Monocle) {
        ws.prevLayout = ws.layout;
    }
    // Split i means different things in different layouts, so only split 0
    // (the master ratio) carries over. Monocle has no splits; its ratios stay.
    if (kind != ws.layout && kind != LayoutKind::Monocle &&
        ws.layout != LayoutKind::Monocle) {
        ws.ratios.resize(std::min<size_t>(ws.ratios.size(), 1));
        ws.flips.clear();  // split orientations do not carry over either
    }
    ws.layout = kind;
}

template <typename Fn>
void ForEachWindow(const Workspace& ws, Fn&& fn) {
    for (HWND h : ws.tiled) fn(h);
    for (HWND h : ws.floating) fn(h);
}

// Shows the windows of `ws` that we hid. Returns how many were shown.
int ShowHiddenIn(const Workspace& ws) {
    int shown = 0;
    ForEachWindow(ws, [&shown](HWND h) {
        if (!g.hiddenByUs.count(h)) return;
        ShowForWorkspace(h);
        ++shown;
    });
    return shown;
}

void HideShownIn(const Workspace& ws) {
    ForEachWindow(ws, [](HWND h) {
        if (!g.hiddenByUs.count(h)) HideForWorkspace(h);
    });
}

void AppendWindows(Workspace& dst, const Workspace& src) {
    dst.tiled.insert(dst.tiled.end(), src.tiled.begin(), src.tiled.end());
    dst.floating.insert(dst.floating.end(), src.floating.begin(),
                        src.floating.end());
}

// Moves a floating window into the work area `to`, keeping its size and its
// offset from the work-area origin `from`, clamped so it stays inside `to`.
void CarryFloatingWindow(HWND hwnd, const RECT& from, const RECT& to) {
    const RECT vis = GetVisibleFrame(hwnd);
    const LONG w = vis.right - vis.left;
    const LONG h = vis.bottom - vis.top;
    const LONG x = std::clamp(to.left + vis.left - from.left, to.left,
                              std::max(to.left, to.right - w));
    const LONG y = std::clamp(to.top + vis.top - from.top, to.top,
                              std::max(to.top, to.bottom - h));
    PlaceWindow(hwnd, RECT{x, y, x + w, y + h});
}

// Moves the membership of `hwnd` (currently at `loc`) to workspace wsIndex of
// monitor mon: out of its tiled or floating list, into the same kind of list of
// the target, and the target's lastFocus. No visibility or layout changes here.
void MoveWindowMembership(HWND hwnd, const WinLoc& loc, int mon, int wsIndex) {
    Workspace& src = g.monitors[loc.mon].workspaces[loc.ws];
    Workspace& dst = g.monitors[mon].workspaces[wsIndex];
    std::vector<HWND>& fromList = loc.floating ? src.floating : src.tiled;
    std::vector<HWND>& toList = loc.floating ? dst.floating : dst.tiled;
    RemoveFrom(fromList, hwnd);
    toList.push_back(hwnd);
    dst.lastFocus = hwnd;
    if (src.lastFocus == hwnd) src.lastFocus = nullptr;
}

// Focuses the workspace's first window. An empty workspace hands focus to the
// desktop, so keystrokes never go to a window we just hid.
void FocusWorkspaceOrDesktop(const Workspace& ws) {
    if (HWND first = FirstWindow(ws)) {
        FocusWindow(first);
    } else if (HWND shell = GetShellWindow()) {
        SetForegroundWindow(shell);
    }
}

void FocusMonitor(int mon) {
    if (mon < 0 || mon >= MonitorCount()) return;
    g.focusedMon = mon;
    if (HWND first = FirstWindow(ActiveWorkspace(mon))) FocusWindow(first);
}

void FocusInDirection(HWND cur, const WinLoc& loc, Direction dir) {
    Workspace& ws = g.monitors[loc.mon].workspaces[loc.ws];
    if (ws.layout == LayoutKind::Monocle && !loc.floating) {
        const std::vector<HWND> shown = Candidates(ws, cur, false);
        const int i = IndexOf(shown, cur);
        const int n = static_cast<int>(shown.size());
        if (i >= 0 && n > 1) {
            const bool back = dir == Direction::Left || dir == Direction::Up;
            FocusWindow(shown[(i + (back ? n - 1 : 1)) % n]);
        }
        return;
    }
    const std::vector<HWND> cands = Candidates(ws, cur, loc.floating);
    const int from = IndexOf(cands, cur);
    if (from >= 0) {
        const int nb = FindNeighbor(FramesOf(cands), from, dir);
        if (nb >= 0) {
            FocusWindow(cands[nb]);
            return;
        }
    }
    const int m = FindNeighbor(MonitorRects(), loc.mon, dir);
    if (m >= 0) FocusMonitor(m);
}

void MoveInDirection(HWND cur, const WinLoc& loc, Direction dir) {
    Workspace& ws = g.monitors[loc.mon].workspaces[loc.ws];
    if (loc.floating) {
        const int m = FindNeighbor(MonitorRects(), loc.mon, dir);
        if (m >= 0) {
            MoveWindowToMonitor(cur, m);
            FocusWindow(cur);
        }
        return;
    }
    if (ws.layout == LayoutKind::Monocle) {
        const bool back = dir == Direction::Left || dir == Direction::Up;
        const int i = IndexOf(ws.tiled, cur);
        const int j = back ? i - 1 : i + 1;
        if (i >= 0 && j >= 0 && j < static_cast<int>(ws.tiled.size())) {
            std::swap(ws.tiled[i], ws.tiled[j]);
            Retile(loc.mon);
            FocusWindow(cur);
            return;
        }
    } else {
        const std::vector<HWND> cands = Candidates(ws, cur, false);
        const int from = IndexOf(cands, cur);
        const int nb =
            from < 0 ? -1 : FindNeighbor(FramesOf(cands), from, dir);
        if (nb >= 0) {
            const int a = IndexOf(ws.tiled, cur);
            const int b = IndexOf(ws.tiled, cands[nb]);
            std::swap(ws.tiled[a], ws.tiled[b]);
            Retile(loc.mon);
            FocusWindow(cur);
            return;
        }
    }
    const int m = FindNeighbor(MonitorRects(), loc.mon, dir);
    if (m >= 0) {
        MoveWindowToMonitor(cur, m);
        FocusWindow(cur);
    }
}

void ToggleFloating(HWND cur, const WinLoc& loc) {
    Workspace& ws = g.monitors[loc.mon].workspaces[loc.ws];
    if (loc.floating) {
        RemoveFrom(ws.floating, cur);
        ws.tiled.push_back(cur);
        if (g.settings.tileMaximized && IsZoomed(cur)) {
            ShowWindowAsync(cur, SW_RESTORE);
            HT_LOG(L"window %p was maximized; restoring it for tiling", (void*)cur);
        }
    } else {
        RemoveFrom(ws.tiled, cur);
        ws.floating.push_back(cur);
        const RECT work = g.monitors[loc.mon].rcWork;
        const LONG areaW = work.right - work.left;
        const LONG areaH = work.bottom - work.top;
        const LONG w = areaW * 6 / 10;
        const LONG h = areaH * 6 / 10;
        const LONG x = work.left + (areaW - w) / 2;
        const LONG y = work.top + (areaH - h) / 2;
        PlaceWindow(cur, RECT{x, y, x + w, y + h});
    }
    Retile(loc.mon);
    UpdateFocusBorder();
}

struct FoundMonitor {
    HMONITOR handle = nullptr;
    RECT rcMonitor{};
    RECT rcWork{};
};

BOOL CALLBACK CollectMonitor(HMONITOR handle, HDC, LPRECT, LPARAM lparam) {
    auto* out = reinterpret_cast<std::vector<FoundMonitor>*>(lparam);
    MONITORINFO info{};
    info.cbSize = sizeof(info);
    if (GetMonitorInfoW(handle, &info)) {
        out->push_back(FoundMonitor{handle, info.rcMonitor, info.rcWork});
    }
    return TRUE;
}

BOOL CALLBACK CollectWindow(HWND hwnd, LPARAM lparam) {
    reinterpret_cast<std::vector<HWND>*>(lparam)->push_back(hwnd);
    return TRUE;
}

}  // namespace

Workspace& ActiveWorkspace(int mon) {
    static Workspace fallback;  // only for invalid indices; callers check first
    if (mon < 0 || mon >= MonitorCount()) {
        fallback = Workspace{};
        return fallback;
    }
    Monitor& m = g.monitors[mon];
    if (m.workspaces.empty()) m.workspaces.push_back(MakeWorkspace());
    const int last = static_cast<int>(m.workspaces.size()) - 1;
    m.active = std::clamp(m.active, 0, last);
    return m.workspaces[m.active];
}

bool Locate(HWND hwnd, WinLoc* loc) {
    if (hwnd) {
        for (int mi = 0; mi < MonitorCount(); ++mi) {
            const std::vector<Workspace>& wss = g.monitors[mi].workspaces;
            for (int wi = 0; wi < static_cast<int>(wss.size()); ++wi) {
                const int t = IndexOf(wss[wi].tiled, hwnd);
                if (t >= 0) {
                    if (loc) *loc = WinLoc{mi, wi, false, t};
                    return true;
                }
                const int f = IndexOf(wss[wi].floating, hwnd);
                if (f >= 0) {
                    if (loc) *loc = WinLoc{mi, wi, true, f};
                    return true;
                }
            }
        }
    }
    if (loc) *loc = WinLoc{};
    return false;
}

int MonitorIndexFromWindow(HWND hwnd) {
    if (MonitorCount() == 0) return -1;
    const HMONITOR hm = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
    const int byHandle = MonitorIndexOf(hm);
    if (byHandle >= 0) return byHandle;
    const RECT vis = GetVisibleFrame(hwnd);
    const POINT center{(vis.left + vis.right) / 2, (vis.top + vis.bottom) / 2};
    for (int i = 0; i < MonitorCount(); ++i) {
        if (PtInRect(&g.monitors[i].rcMonitor, center)) return i;
    }
    return 0;
}

bool ManageWindow(HWND hwnd, bool floating) {
    if (!hwnd || Locate(hwnd, nullptr)) return false;
    const int mon = MonitorIndexFromWindow(hwnd);
    if (mon < 0) return false;
    Workspace& ws = ActiveWorkspace(mon);
    std::vector<HWND>& list = floating ? ws.floating : ws.tiled;
    list.push_back(hwnd);
    if (IsZoomed(hwnd)) g.zoomed.insert(hwnd);
    if (!floating && g.settings.tileMaximized && IsZoomed(hwnd)) {
        // Stays in g.zoomed; the restore's LOCATIONCHANGE retiles it.
        ShowWindowAsync(hwnd, SW_RESTORE);
        HT_LOG(L"window %p was maximized; restoring it for tiling", (void*)hwnd);
    }
    HT_LOG(L"managed %p floating=%d", (void*)hwnd, floating ? 1 : 0);
    RequestBarUpdate();
    return true;
}

void UnmanageWindow(HWND hwnd) {
    if (!hwnd) return;
    for (Monitor& m : g.monitors) {
        for (Workspace& ws : m.workspaces) {
            RemoveFrom(ws.tiled, hwnd);
            RemoveFrom(ws.floating, hwnd);
            if (ws.lastFocus == hwnd) ws.lastFocus = nullptr;
        }
    }
    if (g.hiddenByUs.count(hwnd)) {
        // A live window we stop managing must not stay invisible.
        if (IsWindow(hwnd)) {
            ShowForWorkspace(hwnd);  // also removes it from hiddenByUs
        } else {
            g.hiddenByUs.erase(hwnd);
            PersistHiddenList();
        }
    }
    g.zoomed.erase(hwnd);
    g.pendingVisibility.erase(hwnd);
    g.attention.erase(hwnd);
    if (g.borderedHwnd == hwnd) {
        if (IsWindow(hwnd)) ClearFocusBorder(hwnd);
        g.borderedHwnd = nullptr;
    }
    RequestBarUpdate();
}

void RefreshMonitors() {
    std::vector<FoundMonitor> found;
    EnumDisplayMonitors(nullptr, nullptr, CollectMonitor,
                        reinterpret_cast<LPARAM>(&found));
    if (found.empty()) {
        HT_LOG(L"monitor enumeration returned nothing; keeping old list");
        return;
    }
    std::sort(found.begin(), found.end(),
              [](const FoundMonitor& a, const FoundMonitor& b) {
                  if (a.rcMonitor.left != b.rcMonitor.left) {
                      return a.rcMonitor.left < b.rcMonitor.left;
                  }
                  return a.rcMonitor.top < b.rcMonitor.top;
              });

    // Match by handle first, then by geometry (handles change on re-plug).
    std::vector<Monitor> old;
    old.swap(g.monitors);
    std::vector<bool> used(old.size(), false);
    std::vector<int> oldFor(found.size(), -1);
    for (size_t f = 0; f < found.size(); ++f) {
        for (size_t o = 0; o < old.size() && oldFor[f] < 0; ++o) {
            if (!used[o] && old[o].handle == found[f].handle) {
                oldFor[f] = static_cast<int>(o);
                used[o] = true;
            }
        }
    }
    for (size_t f = 0; f < found.size(); ++f) {
        for (size_t o = 0; o < old.size() && oldFor[f] < 0; ++o) {
            if (!used[o] &&
                EqualRect(&old[o].rcMonitor, &found[f].rcMonitor)) {
                oldFor[f] = static_cast<int>(o);
                used[o] = true;
            }
        }
    }

    for (size_t f = 0; f < found.size(); ++f) {
        Monitor m;
        if (oldFor[f] >= 0) {
            m = std::move(old[oldFor[f]]);
        } else {
            m.workspaces.assign(WsCount(), MakeWorkspace());
        }
        m.handle = found[f].handle;
        m.rcMonitor = found[f].rcMonitor;
        m.rcWork = found[f].rcWork;
        if (m.workspaces.empty()) m.workspaces.push_back(MakeWorkspace());
        const int last = static_cast<int>(m.workspaces.size()) - 1;
        m.active = std::clamp(m.active, 0, last);
        g.monitors.push_back(std::move(m));
    }

    // Windows of vanished monitors join the active workspace of monitor 0.
    Workspace& dst = ActiveWorkspace(0);
    for (size_t o = 0; o < old.size(); ++o) {
        if (used[o]) continue;
        HT_LOG(L"monitor vanished; moving its windows to monitor 0");
        for (Workspace& ws : old[o].workspaces) {
            ShowHiddenIn(ws);
            AppendWindows(dst, ws);
        }
    }

    g.focusedMon = std::clamp(g.focusedMon, 0, MonitorCount() - 1);
    HT_LOG(L"%d monitor(s)", MonitorCount());
    for (int i = 0; i < MonitorCount(); ++i) {
        const RECT& rc = g.monitors[i].rcMonitor;
        HT_LOG(L"  monitor %d: %ld,%ld-%ld,%ld", i, rc.left, rc.top, rc.right,
               rc.bottom);
    }
    RequestBarUpdate();
}

void ApplySettingsChange() {
    const int count = WsCount();
    for (int mi = 0; mi < MonitorCount(); ++mi) {
        Monitor& m = g.monitors[mi];
        while (static_cast<int>(m.workspaces.size()) < count) {
            m.workspaces.push_back(MakeWorkspace());
        }
        if (static_cast<int>(m.workspaces.size()) > count) {
            Workspace& last = m.workspaces[count - 1];
            for (size_t wi = static_cast<size_t>(count);
                 wi < m.workspaces.size(); ++wi) {
                const Workspace& gone = m.workspaces[wi];
                AppendWindows(last, gone);
                if (gone.lastFocus && !InWorkspace(last, last.lastFocus)) {
                    last.lastFocus = gone.lastFocus;
                }
            }
            m.workspaces.resize(static_cast<size_t>(count));
        }
        m.active = std::clamp(m.active, 0, count - 1);
        // Covers a removed active workspace and windows moved into it.
        ShowHiddenIn(m.workspaces[m.active]);
    }
    RetileAll();
    UpdateFocusBorder();
    RequestBarUpdate();
}

void Retile(int mon) {
    if (g.paused || mon < 0 || mon >= MonitorCount()) return;
    Workspace& ws = ActiveWorkspace(mon);
    const std::vector<HWND> visible = LayoutWindowsOf(ws);
    if (visible.empty()) return;

    const std::vector<RECT> rects =
        ComputeLayout(LayoutAreaOf(mon), static_cast<int>(visible.size()),
                      LayoutParamsOf(ws));
    std::vector<HWND> failed;
    for (size_t i = 0; i < visible.size() && i < rects.size(); ++i) {
        if (!PlaceWindow(visible[i], rects[i])) failed.push_back(visible[i]);
    }
    for (HWND h : failed) {
        RemoveFrom(ws.tiled, h);
        ws.floating.push_back(h);
        HT_LOG(L"window %p cannot be tiled; now floating", (void*)h);
    }
    if (!failed.empty()) RequestRetile();
}

void RetileAll() {
    for (int i = 0; i < MonitorCount(); ++i) Retile(i);
}

void RequestRetile() {
    if (g.engineHwnd) SetTimer(g.engineHwnd, kTimerRetile, 15, nullptr);
}

void SwitchWorkspace(int mon, int wsIndex) {
    if (mon < 0 || mon >= MonitorCount()) return;
    Monitor& m = g.monitors[mon];
    if (wsIndex < 0 || wsIndex >= static_cast<int>(m.workspaces.size())) {
        return;
    }
    Workspace& prev = ActiveWorkspace(mon);  // also normalises m.active
    if (wsIndex == m.active) {
        g.focusedMon = mon;
        if (g.settings.workspaceOsd) ShowWorkspaceOsd(mon);
        RequestBarUpdate();
        return;
    }
    Workspace& next = m.workspaces[wsIndex];
    ShowHiddenIn(next);
    HideShownIn(prev);
    m.active = wsIndex;
    g.focusedMon = mon;
    Retile(mon);
    FocusWorkspaceOrDesktop(next);
    UpdateFocusBorder();
    HT_LOG(L"workspace %d on monitor %d", wsIndex + 1, mon);
    if (g.settings.workspaceOsd) ShowWorkspaceOsd(mon);
    RequestBarUpdate();
}

void MoveWindowToWorkspace(HWND hwnd, int mon, int wsIndex) {
    WinLoc loc;
    if (!Locate(hwnd, &loc)) return;
    if (mon < 0 || mon >= MonitorCount()) return;
    Monitor& dstMon = g.monitors[mon];
    if (wsIndex < 0 || wsIndex >= static_cast<int>(dstMon.workspaces.size())) {
        return;
    }
    if (loc.mon == mon && loc.ws == wsIndex) return;

    Monitor& srcMon = g.monitors[loc.mon];
    Workspace& src = srcMon.workspaces[loc.ws];
    MoveWindowMembership(hwnd, loc, mon, wsIndex);

    const bool targetActive = wsIndex == dstMon.active;
    const bool sourceActive = loc.ws == srcMon.active;
    if (!targetActive) {
        HideForWorkspace(hwnd);
    } else {
        if (g.hiddenByUs.count(hwnd)) ShowForWorkspace(hwnd);
        if (loc.floating && loc.mon != mon) {
            CarryFloatingWindow(hwnd, srcMon.rcWork, dstMon.rcWork);
        }
    }

    Retile(loc.mon);
    if (mon != loc.mon) Retile(mon);
    if (!targetActive && sourceActive) FocusWorkspaceOrDesktop(src);
    UpdateFocusBorder();
    RequestBarUpdate();
}

void MoveWindowToWorkspaceAndFollow(HWND hwnd, int wsIndex) {
    WinLoc loc;
    if (!Locate(hwnd, &loc)) return;
    const int mon = loc.mon;
    Monitor& m = g.monitors[mon];
    if (wsIndex < 0 || wsIndex >= static_cast<int>(m.workspaces.size())) {
        return;
    }
    if (loc.ws == wsIndex) {
        if (g.settings.workspaceOsd) ShowWorkspaceOsd(mon);
        return;
    }

    ActiveWorkspace(mon);  // also normalises m.active
    if (loc.ws != m.active) {
        // Unusual: the window sits on a workspace that is not shown. Move it
        // silently, then switch, which shows it and focuses it.
        MoveWindowToWorkspace(hwnd, mon, wsIndex);
        SwitchWorkspace(mon, wsIndex);
        return;
    }

    // The window stays visible: SwitchWorkspace hides what is left behind and
    // focuses this window, because the target's lastFocus is now hwnd.
    MoveWindowMembership(hwnd, loc, mon, wsIndex);
    HT_LOG(L"window %p moved to workspace %d and followed", (void*)hwnd,
           wsIndex + 1);
    SwitchWorkspace(mon, wsIndex);
}

void MoveWindowToMonitor(HWND hwnd, int mon) {
    if (mon < 0 || mon >= MonitorCount()) return;
    MoveWindowToWorkspace(hwnd, mon, g.monitors[mon].active);
}

HWND CommandTarget() {
    if (MonitorCount() == 0) return nullptr;
    const HWND fg = GetForegroundWindow();
    if (Locate(fg, nullptr)) return fg;
    const HWND root = fg ? GetAncestor(fg, GA_ROOTOWNER) : nullptr;
    if (Locate(root, nullptr)) return root;
    if (g.focusedMon < 0 || g.focusedMon >= MonitorCount()) return nullptr;
    const HWND last = ActiveWorkspace(g.focusedMon).lastFocus;
    return Locate(last, nullptr) ? last : nullptr;
}

void OnFocusChanged(HWND hwnd) {
    WinLoc loc;
    HWND managed = nullptr;
    if (Locate(hwnd, &loc)) {
        managed = hwnd;
    } else {
        const HWND root = hwnd ? GetAncestor(hwnd, GA_ROOTOWNER) : nullptr;
        if (Locate(root, &loc)) managed = root;
    }
    if (managed) {
        g.focusedMon = loc.mon;
        g.monitors[loc.mon].workspaces[loc.ws].lastFocus = managed;
        if (g.attention.erase(managed) > 0) RequestBarUpdate();
    }
    UpdateFocusBorder();
}

void ActivateWindowWorkspace(HWND hwnd) {
    WinLoc loc;
    if (!Locate(hwnd, &loc)) return;
    ActiveWorkspace(loc.mon);  // also normalises m.active
    g.monitors[loc.mon].workspaces[loc.ws].lastFocus = hwnd;
    if (loc.ws != g.monitors[loc.mon].active) {
        HT_LOG(L"following activation of %p to workspace %d",
               static_cast<void*>(hwnd), loc.ws + 1);
        SwitchWorkspace(loc.mon, loc.ws);
    }
    FocusWindow(hwnd);
}

void MarkAttention(HWND hwnd) {
    HWND managed = nullptr;
    if (Locate(hwnd, nullptr)) {
        managed = hwnd;
    } else {
        const HWND root = hwnd ? GetAncestor(hwnd, GA_ROOTOWNER) : nullptr;
        if (Locate(root, nullptr)) managed = root;
    }
    if (!managed) return;
    // A window the user is already looking at does not need a dot.
    const HWND fg = GetForegroundWindow();
    const HWND fgRoot = fg ? GetAncestor(fg, GA_ROOTOWNER) : nullptr;
    if (fg == managed || fgRoot == managed) return;
    if (g.attention.insert(managed).second) {
        HT_LOG(L"attention %p", static_cast<void*>(managed));
        RequestBarUpdate();
    }
}

bool WorkspaceHasAttention(int mon, int wsIndex) {
    if (!g.settings.attentionDots || g.attention.empty()) return false;
    if (mon < 0 || mon >= MonitorCount()) return false;
    const std::vector<Workspace>& wss = g.monitors[mon].workspaces;
    if (wsIndex < 0 || wsIndex >= static_cast<int>(wss.size())) return false;
    bool found = false;
    ForEachWindow(wss[wsIndex], [&found](HWND h) {
        if (g.attention.count(h) > 0) found = true;
    });
    return found;
}

void UpdateFocusBorder() {
    const HWND target = g.settings.focusBorder ? CommandTarget() : nullptr;
    if (target && target == g.borderedHwnd) {
        SetFocusBorder(target, g.settings.focusBorderColor);
        return;
    }
    if (g.borderedHwnd && IsWindow(g.borderedHwnd)) {
        ClearFocusBorder(g.borderedHwnd);
    }
    g.borderedHwnd = nullptr;
    if (target) {
        SetFocusBorder(target, g.settings.focusBorderColor);
        g.borderedHwnd = target;
    }
}

void OnUserDragEnd(HWND hwnd) {
    WinLoc loc;
    if (Locate(hwnd, &loc)) {
        const RECT frame = GetVisibleFrame(hwnd);
        const POINT center{(frame.left + frame.right) / 2,
                           (frame.top + frame.bottom) / 2};
        const HMONITOR hm = MonitorFromPoint(center, MONITOR_DEFAULTTONULL);
        const int target = MonitorIndexOf(hm);
        if (target >= 0 && target != loc.mon && loc.floating) {
            // The user already put it where it belongs; only membership
            // changes, so it hides and shows with the new monitor's workspace.
            Workspace& src = g.monitors[loc.mon].workspaces[loc.ws];
            RemoveFrom(src.floating, hwnd);
            if (src.lastFocus == hwnd) src.lastFocus = nullptr;
            Workspace& dst = ActiveWorkspace(target);
            dst.floating.push_back(hwnd);
            dst.lastFocus = hwnd;
            g.focusedMon = target;
            RequestBarUpdate();
        } else if (target >= 0 && target != loc.mon) {
            MoveWindowToMonitor(hwnd, target);
            RequestBarUpdate();
        } else if (!loc.floating) {
            Workspace& ws = g.monitors[loc.mon].workspaces[loc.ws];
            const std::vector<HWND> list = LayoutWindowsOf(ws);
            const int idx = IndexOf(list, hwnd);
            // Monocle windows share one rect, so there is no slot to compare.
            if (ws.layout != LayoutKind::Monocle && idx >= 0) {
                const int count = static_cast<int>(list.size());
                const RECT area = LayoutAreaOf(loc.mon);
                const LayoutParams params = LayoutParamsOf(ws);
                const std::vector<RECT> slots =
                    ComputeLayout(area, count, params);
                // Compare with the size at drag start: windows with a minimum
                // size never match their slot, so a plain move would otherwise
                // look like a resize.
                const RECT& before =
                    g.dragHwnd == hwnd ? g.dragStartFrame : slots[idx];
                const LONG dw =
                    (frame.right - frame.left) - (before.right - before.left);
                const LONG dh =
                    (frame.bottom - frame.top) - (before.bottom - before.top);
                if (dw > 2 || dw < -2 || dh > 2 || dh < -2) {
                    // Resized: the neighbours follow the new edges.
                    LayoutParams after = params;
                    if (ApplyResize(area, count, after, idx, frame)) {
                        for (int s = 0; s + 1 < count; ++s) {
                            const double was = SplitRatio(params, count, s);
                            const double now = SplitRatio(after, count, s);
                            if (was != now) {
                                HT_LOG(L"split %d ratio %.3f -> %.3f", s, was,
                                       now);
                            }
                        }
                        ws.ratios = after.ratios;
                        HT_LOG(L"resized window %p", (void*)hwnd);
                    }
                } else {
                    // Moved: swap with the tiled window under the drop point.
                    const int self = IndexOf(ws.tiled, hwnd);
                    const int n = static_cast<int>(ws.tiled.size());
                    for (int j = 0; j < n; ++j) {
                        if (j == self) continue;
                        const RECT other = GetVisibleFrame(ws.tiled[j]);
                        if (PtInRect(&other, center)) {
                            std::swap(ws.tiled[self], ws.tiled[j]);
                            break;
                        }
                    }
                }
            }
        }
    }
    RequestRetile();
}

void ExecuteCommand(const Command& cmd) {
    if (g.paused && cmd.kind != CommandKind::ToggleTiling &&
        cmd.kind != CommandKind::Rescan) {
        return;
    }
    if (cmd.kind == CommandKind::Rescan) {
        RefreshMonitors();
        AdoptExistingWindows();
        RetileAll();
        UpdateFocusBorder();
        return;
    }
    if (cmd.kind == CommandKind::ToggleTiling) {
        g.paused = !g.paused;
        HT_LOG(L"%s", g.paused ? L"paused" : L"resumed");
        if (!g.paused) RetileAll();
        return;
    }
    if (MonitorCount() == 0 || cmd.kind == CommandKind::None) return;

    const HWND cur = CommandTarget();
    WinLoc loc;
    const bool has = Locate(cur, &loc);
    const int fm = std::clamp(g.focusedMon, 0, MonitorCount() - 1);
    const int mon = has ? loc.mon : fm;
    Workspace& ws =
        has ? g.monitors[loc.mon].workspaces[loc.ws] : ActiveWorkspace(mon);
    const int wsCount = WsCount();

    switch (cmd.kind) {
        case CommandKind::MoveFocus:
            if (has) {
                FocusInDirection(cur, loc, cmd.dir);
            } else {
                FocusMonitor(fm);
            }
            break;
        case CommandKind::SwapWindow:
            if (has) MoveInDirection(cur, loc, cmd.dir);
            break;
        case CommandKind::Workspace:
            if (cmd.arg >= 1 && cmd.arg <= wsCount) {
                SwitchWorkspace(fm, cmd.arg - 1);
            }
            break;
        case CommandKind::MoveToWorkspace:
            if (has && cmd.arg >= 1 && cmd.arg <= wsCount) {
                MoveWindowToWorkspaceAndFollow(cur, cmd.arg - 1);
            }
            break;
        case CommandKind::MoveToWorkspaceSilent:
            if (has && cmd.arg >= 1 && cmd.arg <= wsCount) {
                MoveWindowToWorkspace(cur, loc.mon, cmd.arg - 1);
                if (g.settings.workspaceOsd) ShowWorkspaceOsd(loc.mon);
            }
            break;
        case CommandKind::FocusMonitorNext:
            if (MonitorCount() > 1) {
                const int next = (fm + 1) % MonitorCount();
                FocusMonitor(next);
                if (g.settings.workspaceOsd) ShowWorkspaceOsd(next);
            }
            break;
        case CommandKind::MoveToMonitorNext:
            if (has && MonitorCount() > 1) {
                MoveWindowToMonitor(cur, (loc.mon + 1) % MonitorCount());
                FocusWindow(cur);
            }
            break;
        case CommandKind::NextLayout:
            SetWorkspaceLayout(ws, NextLayout(ws.layout));
            Retile(mon);
            HT_LOG(L"layout %s on monitor %d", LayoutName(ws.layout), mon);
            break;
        case CommandKind::SetLayout:
            SetWorkspaceLayout(ws, cmd.layout);
            Retile(mon);
            HT_LOG(L"layout %s on monitor %d", LayoutName(ws.layout), mon);
            break;
        case CommandKind::ToggleFloating:
            if (has) ToggleFloating(cur, loc);
            break;
        case CommandKind::ToggleMonocle:
            if (ws.layout == LayoutKind::Monocle) {
                ws.layout = ws.prevLayout == LayoutKind::Monocle
                                ? g.settings.defaultLayout
                                : ws.prevLayout;
            } else {
                ws.prevLayout = ws.layout;
                ws.layout = LayoutKind::Monocle;
            }
            Retile(mon);
            if (has) FocusWindow(cur);
            break;
        case CommandKind::SwapWithMaster:
            if (has && !loc.floating && ws.tiled.size() > 1) {
                const int i = IndexOf(ws.tiled, cur);
                if (i == 0) {
                    std::swap(ws.tiled[0], ws.tiled[1]);
                } else {
                    ws.tiled.erase(ws.tiled.begin() + i);
                    ws.tiled.insert(ws.tiled.begin(), cur);
                }
                Retile(mon);
                FocusWindow(cur);
            }
            break;
        case CommandKind::GrowMaster:
        case CommandKind::ShrinkMaster: {
            // Moves split 0 as it is drawn now by one step. Two windows is the
            // smallest count that has a split.
            const int n =
                std::max(2, static_cast<int>(LayoutWindowsOf(ws).size()));
            const double cur = SplitRatio(LayoutParamsOf(ws), n, 0);
            const double step = g.settings.ratioStep;
            const double delta =
                cmd.kind == CommandKind::GrowMaster ? step : -step;
            if (ws.ratios.empty()) ws.ratios.resize(1, 0.0);
            ws.ratios[0] = std::clamp(cur + delta, 0.1, 0.9);
            Retile(mon);
            break;
        }
        case CommandKind::ToggleSplit:
            if (has && !loc.floating && ws.layout != LayoutKind::Monocle) {
                const std::vector<HWND> list = LayoutWindowsOf(ws);
                const int idx = IndexOf(list, cur);
                LayoutParams params = LayoutParamsOf(ws);
                if (ToggleSplitAt(params, static_cast<int>(list.size()), idx)) {
                    ws.flips = params.flips;
                    Retile(mon);
                    HT_LOG(L"toggled split of window %p on monitor %d",
                           (void*)cur, mon);
                }
            }
            break;
        case CommandKind::ResizeActive:
            if (has && !loc.floating && ws.layout != LayoutKind::Monocle) {
                const std::vector<HWND> list = LayoutWindowsOf(ws);
                const int idx = IndexOf(list, cur);
                const int n = static_cast<int>(list.size());
                if (idx >= 0 && n >= 2) {
                    const RECT area = LayoutAreaOf(mon);
                    const LayoutParams params = LayoutParamsOf(ws);
                    const std::vector<RECT> slots = ComputeLayout(area, n, params);
                    const bool horizontal =
                        cmd.dir == Direction::Left || cmd.dir == Direction::Right;
                    const LONG extent = horizontal ? area.right - area.left
                                                   : area.bottom - area.top;
                    const LONG step =
                        std::max<LONG>(1, std::lround(g.settings.ratioStep * extent));
                    // Grow the edge on the dir side. Where there is no split on
                    // that side (the edge is the screen border), shrink the
                    // opposite edge instead, so the boundary still moves toward dir.
                    RECT grown = slots[idx];
                    RECT shrunk = slots[idx];
                    if (cmd.dir == Direction::Left) {
                        grown.left -= step;
                        shrunk.right -= step;
                    } else if (cmd.dir == Direction::Right) {
                        grown.right += step;
                        shrunk.left += step;
                    } else if (cmd.dir == Direction::Up) {
                        grown.top -= step;
                        shrunk.bottom -= step;
                    } else {
                        grown.bottom += step;
                        shrunk.top += step;
                    }
                    LayoutParams grownParams = params;
                    LayoutParams shrunkParams = params;
                    if (ApplyResize(area, n, grownParams, idx, grown)) {
                        ws.ratios = grownParams.ratios;
                    } else if (ApplyResize(area, n, shrunkParams, idx, shrunk)) {
                        ws.ratios = shrunkParams.ratios;
                    }
                    Retile(mon);
                }
            }
            break;
        case CommandKind::WorkspaceNext:
        case CommandKind::WorkspacePrev: {
            ActiveWorkspace(fm);  // also normalises m.active and ensures a workspace
            const int count = static_cast<int>(g.monitors[fm].workspaces.size());
            const int delta = cmd.kind == CommandKind::WorkspaceNext ? 1 : -1;
            SwitchWorkspace(fm, (g.monitors[fm].active + delta + count) % count);
            break;
        }
        case CommandKind::CloseWindow:
            if (has) PostMessageW(cur, WM_SYSCOMMAND, SC_CLOSE, 0);
            break;
        case CommandKind::Fullscreen:
            // The LOCATIONCHANGE handler takes it out of or back into the layout.
            if (has) {
                ShowWindowAsync(cur, IsZoomed(cur) ? SW_RESTORE : SW_MAXIMIZE);
            }
            break;
        default:
            break;
    }
}

void AdoptExistingWindows() {
    std::vector<HWND> all;
    EnumWindows(CollectWindow, reinterpret_cast<LPARAM>(&all));
    // EnumWindows lists top-most first; adopt bottom-most first so it ends up
    // as the first tile.
    std::reverse(all.begin(), all.end());
    int adopted = 0;
    for (HWND h : all) {
        if (Locate(h, nullptr)) continue;
        const Eligibility e = ClassifyWindow(h, g.settings);
        if (e == Eligibility::Ignore) continue;
        if (ManageWindow(h, e == Eligibility::Float)) ++adopted;
    }
    HT_LOG(L"adopted %d window(s)", adopted);
}

}  // namespace ht
