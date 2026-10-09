// =============================================================================
// 20_layout.cpp — tiling geometry. Pure functions; the only Win32 type used is
// RECT. Signatures and semantics: 10_contract.h.
// =============================================================================

#include <cmath>

namespace ht {
namespace {

LONG AbsLong(LONG v) {
    return v < 0 ? -v : v;
}

bool RectEquals(const RECT& a, const RECT& b) {
    return a.left == b.left && a.top == b.top && a.right == b.right &&
           a.bottom == b.bottom;
}

// Clamps r to [lo, hi]. Written so that NaN takes the lower bound.
double ClampRatio(double r, double lo, double hi) {
    if (!(r >= lo)) {
        return lo;
    }
    if (r > hi) {
        return hi;
    }
    return r;
}

// Final rect of a raw slot. Low sides move in by `a`, high sides by `b`, so two
// slots that share an edge end up exactly a + b = innerGap apart.
RECT FinalizeSlot(const RECT& slot, LONG a, LONG b) {
    RECT rc{slot.left + a, slot.top + a, slot.right - b, slot.bottom - b};
    rc.right = std::max(rc.right, rc.left + 1);
    rc.bottom = std::max(rc.bottom, rc.top + 1);
    return rc;
}

// Cut that splits [lo, hi) at ratio r.
LONG SplitCut(LONG lo, LONG hi, double r) {
    return lo + std::lround(static_cast<double>(hi - lo) * r);
}

// Ratio that puts the cut at `cut` inside the remaining region [lo, hi).
double RatioTo(LONG lo, LONG hi, LONG cut) {
    return static_cast<double>(cut - lo) / static_cast<double>(hi - lo);
}

// One binary split of a layout. [lo, hi) is the remaining region's extent along
// the cut axis, in Expanded coordinates, and `cut` lies inside it.
struct LayoutSplit {
    bool vertical = true;  // vertical: the cut is the line x = cut; else y = cut
    LONG lo = 0;
    LONG hi = 0;
    LONG cut = 0;
};

// Orientation of split `split` for a split-based layout (not Monocle), given the
// remaining region `rest` that it cuts. Dwindle follows the region's shape unless
// p.flips[split] is set, which inverts it.
bool IsVertical(const LayoutParams& p, int split, const RECT& rest) {
    switch (p.kind) {
    case LayoutKind::MasterStack:
        return split == 0;
    case LayoutKind::Columns:
        return true;
    case LayoutKind::Rows:
        return false;
    default: {  // Dwindle
        const bool wider = rest.right - rest.left > rest.bottom - rest.top;
        const size_t index = static_cast<size_t>(split);
        const bool flipped = index < p.flips.size() && p.flips[index] != 0;
        return wider != flipped;
    }
    }
}

// Walks the count-1 splits of `expanded` (count >= 1, not Monocle). `splits` gets
// one entry per split. `slots` gets the raw slot of every window: window i is
// side A of split i (left or top part), and the last window takes what remains.
// ComputeLayout and ApplyResize both use this, so their geometry is identical.
void WalkSplits(const RECT& expanded, int count, const LayoutParams& p,
                std::vector<LayoutSplit>* splits, std::vector<RECT>* slots) {
    splits->clear();
    slots->clear();
    RECT rest = expanded;
    for (int i = 0; i + 1 < count; ++i) {
        LayoutSplit s;
        s.vertical = IsVertical(p, i, rest);
        s.lo = s.vertical ? rest.left : rest.top;
        s.hi = s.vertical ? rest.right : rest.bottom;
        s.cut = SplitCut(s.lo, s.hi, SplitRatio(p, count, i));
        splits->push_back(s);

        RECT sideA = rest;
        if (s.vertical) {
            sideA.right = s.cut;
            rest.left = s.cut;
        } else {
            sideA.bottom = s.cut;
            rest.top = s.cut;
        }
        slots->push_back(sideA);
    }
    slots->push_back(rest);
}

// Ranking key of a FindNeighbor candidate; smaller is better.
struct NeighborKey {
    int overlapRank = 1;  // 0 when the perpendicular extents overlap
    LONG along = 0;       // doubled centre distance in the direction
    LONG perp = 0;        // doubled perpendicular centre distance
};

bool KeyBetter(const NeighborKey& x, const NeighborKey& y) {
    if (x.overlapRank != y.overlapRank) {
        return x.overlapRank < y.overlapRank;
    }
    if (x.along != y.along) {
        return x.along < y.along;
    }
    return x.perp < y.perp;
}

// Centres are doubled (left + right) so all arithmetic stays integer. Returns
// false when `c` is not strictly in direction `dir` from `f`.
bool MakeNeighborKey(const RECT& f, const RECT& c, Direction dir, NeighborKey* key) {
    const LONG fx = f.left + f.right;
    const LONG fy = f.top + f.bottom;
    const LONG cx = c.left + c.right;
    const LONG cy = c.top + c.bottom;
    const bool horizontal = dir == Direction::Left || dir == Direction::Right;

    LONG along = 0;
    switch (dir) {
    case Direction::Left:
        along = fx - cx;
        break;
    case Direction::Right:
        along = cx - fx;
        break;
    case Direction::Up:
        along = fy - cy;
        break;
    case Direction::Down:
        along = cy - fy;
        break;
    }
    if (along <= 0) {
        return false;
    }

    const bool overlaps = horizontal ? (c.top < f.bottom && f.top < c.bottom)
                                     : (c.left < f.right && f.left < c.right);
    key->overlapRank = overlaps ? 0 : 1;
    key->along = along;
    key->perp = AbsLong(horizontal ? cy - fy : cx - fx);
    return true;
}

struct LayoutNameEntry {
    const wchar_t* name;
    LayoutKind kind;
};

// Accepted spellings for ParseLayoutName. LayoutName uses the first entry per kind.
constexpr LayoutNameEntry kLayoutNames[] = {
    {L"dwindle", LayoutKind::Dwindle},
    {L"bsp", LayoutKind::Dwindle},
    {L"master", LayoutKind::MasterStack},
    {L"master-stack", LayoutKind::MasterStack},
    {L"columns", LayoutKind::Columns},
    {L"rows", LayoutKind::Rows},
    {L"monocle", LayoutKind::Monocle},
};

// Case-insensitive comparison of a whole string against a literal.
bool EqualsIgnoreCase(const wchar_t* text, const wchar_t* literal) {
    for (; *text != L'\0' && *literal != L'\0'; ++text, ++literal) {
        if (std::towlower(*text) != std::towlower(*literal)) {
            return false;
        }
    }
    return *text == *literal;
}

}  // namespace

std::vector<RECT> ComputeLayout(const RECT& area, int count, const LayoutParams& p) {
    std::vector<RECT> out;
    if (count <= 0) {
        return out;
    }
    out.reserve(static_cast<size_t>(count));
    if (p.kind == LayoutKind::Monocle) {
        out.assign(static_cast<size_t>(count), area);
        return out;
    }

    const int gap = std::max(0, p.innerGap);
    const LONG a = gap / 2;
    const LONG b = gap - a;
    const RECT expanded{area.left - a, area.top - a, area.right + b, area.bottom + b};

    std::vector<LayoutSplit> splits;
    std::vector<RECT> slots;
    WalkSplits(expanded, count, p, &splits, &slots);
    for (const RECT& slot : slots) {
        out.push_back(FinalizeSlot(slot, a, b));
    }
    return out;
}

double SplitRatio(const LayoutParams& p, int count, int split) {
    if (split < 0 || split >= count - 1) {
        return 0.5;
    }
    const size_t index = static_cast<size_t>(split);
    if (index < p.ratios.size()) {
        const double r = p.ratios[index];
        if (0.0 < r && r < 1.0) {
            return ClampRatio(r, 0.05, 0.95);
        }
    }

    const double master = ClampRatio(p.masterRatio, 0.05, 0.95);
    switch (p.kind) {
    case LayoutKind::MasterStack:
        return split == 0 ? master : 1.0 / (count - split);
    case LayoutKind::Columns:
    case LayoutKind::Rows:
        return 1.0 / (count - split);
    default:  // Dwindle (Monocle has no splits)
        return split == 0 ? master : 0.5;
    }
}

bool ApplyResize(const RECT& area, int count, LayoutParams& p, int index,
                 const RECT& newRect) {
    if (p.kind == LayoutKind::Monocle || count < 2 || index < 0 || index >= count) {
        return false;
    }
    const size_t splitCount = static_cast<size_t>(count - 1);
    if (p.ratios.size() < splitCount) {
        p.ratios.resize(splitCount, 0.0);
    }

    const int gap = std::max(0, p.innerGap);
    const LONG a = gap / 2;
    const LONG b = gap - a;
    const RECT expanded{area.left - a, area.top - a, area.right + b, area.bottom + b};

    // Step 1: the current geometry. Every split keeps its orientation and absolute
    // cut unless the drag moves it. "Moved" is judged against the slot edges the
    // user saw, before this call.
    std::vector<LayoutSplit> oldSplits;
    std::vector<RECT> oldSlots;
    WalkSplits(expanded, count, p, &oldSplits, &oldSlots);
    const RECT oldSlot = oldSlots[static_cast<size_t>(index)];
    const RECT oldShown = FinalizeSlot(oldSlot, a, b);

    // Desired cut: the dragged edge for a moved split, the old cut for all others.
    // Candidates are split `index` (the window is side A, so its right or bottom
    // edge is the cut) and every earlier split whose cut is the window's left or
    // top edge (side B). Later splits lie entirely in the other part.
    std::vector<bool> moved(splitCount, false);
    std::vector<LONG> desired(splitCount, 0);
    for (size_t k = 0; k < splitCount; ++k) {
        desired[k] = oldSplits[k].cut;
    }
    const int last = std::min(index, count - 2);
    for (int i = 0; i <= last; ++i) {
        const size_t k = static_cast<size_t>(i);
        const LayoutSplit& was = oldSplits[k];
        if (i == index) {
            // Side A: visible edge = cut - b, so cut = edge + b.
            const LONG target = was.vertical ? newRect.right : newRect.bottom;
            const LONG shown = was.vertical ? oldShown.right : oldShown.bottom;
            if (AbsLong(target - shown) > 2) {
                moved[k] = true;
                desired[k] = target + b;
            }
        } else {
            // Side B: the window's left or top edge must lie on this cut.
            // Visible edge = cut + a, so cut = edge - a.
            const LONG onCut = was.vertical ? oldSlot.left : oldSlot.top;
            if (onCut != was.cut) {
                continue;
            }
            const LONG target = was.vertical ? newRect.left : newRect.top;
            const LONG shown = was.vertical ? oldShown.left : oldShown.top;
            if (AbsLong(target - shown) > 2) {
                moved[k] = true;
                desired[k] = target - a;
            }
        }
    }

    // Step 2: walk from split 0 upward, recomputing each remaining region with the
    // ratios written so far. A split whose orientation has changed no longer cuts
    // the same edges, so it is left alone.
    bool changed = false;
    std::vector<LayoutSplit> splits;
    std::vector<RECT> slots;
    for (int i = 0; i + 1 < count; ++i) {
        const size_t k = static_cast<size_t>(i);
        WalkSplits(expanded, count, p, &splits, &slots);
        const LayoutSplit& now = splits[k];
        if (now.vertical != oldSplits[k].vertical || now.hi <= now.lo) {
            continue;
        }
        double ratio = 0.0;
        if (moved[k]) {
            ratio = ClampRatio(RatioTo(now.lo, now.hi, desired[k]), 0.1, 0.9);
        } else if (AbsLong(now.cut - desired[k]) >= 1) {
            // Not dragged, but earlier writes would move this cut: pin it in place.
            ratio = ClampRatio(RatioTo(now.lo, now.hi, desired[k]), 0.05, 0.95);
        } else {
            continue;  // stays at its current entry (0.0 = default)
        }
        double& stored = p.ratios[k];
        if (stored != ratio) {
            stored = ratio;
            changed = true;
        }
    }
    return changed;
}

bool ToggleSplitAt(LayoutParams& p, int count, int index) {
    if (p.kind != LayoutKind::Dwindle || count < 2 || index < 0 || index >= count) {
        return false;
    }
    // The last window has no split of its own; it shares the previous one.
    const int split = index < count - 1 ? index : count - 2;
    const size_t splitCount = static_cast<size_t>(count - 1);
    if (p.flips.size() < splitCount) {
        p.flips.resize(splitCount, 0);
    }
    int& flip = p.flips[static_cast<size_t>(split)];
    flip = flip != 0 ? 0 : 1;
    return true;
}

int FindNeighbor(const std::vector<RECT>& rects, int from, Direction dir) {
    const int n = static_cast<int>(rects.size());
    if (from < 0 || from >= n) {
        return -1;
    }
    int best = -1;
    NeighborKey bestKey;
    for (int i = 0; i < n; ++i) {
        // Also skips `from` itself.
        if (RectEquals(rects[i], rects[from])) {
            continue;
        }
        NeighborKey key;
        if (!MakeNeighborKey(rects[from], rects[i], dir, &key)) {
            continue;
        }
        // Strict comparison keeps the lowest index on full ties.
        if (best < 0 || KeyBetter(key, bestKey)) {
            best = i;
            bestKey = key;
        }
    }
    return best;
}

LayoutKind NextLayout(LayoutKind k) {
    constexpr int kCount = static_cast<int>(LayoutKind::Count_);
    const int v = static_cast<int>(k);
    if (v < 0 || v >= kCount) {
        return LayoutKind::Dwindle;
    }
    return static_cast<LayoutKind>((v + 1) % kCount);
}

const wchar_t* LayoutName(LayoutKind k) {
    switch (k) {
    case LayoutKind::Dwindle:
        return L"dwindle";
    case LayoutKind::MasterStack:
        return L"master";
    case LayoutKind::Columns:
        return L"columns";
    case LayoutKind::Rows:
        return L"rows";
    case LayoutKind::Monocle:
        return L"monocle";
    case LayoutKind::Count_:
        break;
    }
    return L"unknown";
}

bool ParseLayoutName(const wchar_t* s, LayoutKind* out) {
    if (s == nullptr || out == nullptr) {
        return false;
    }
    for (const LayoutNameEntry& entry : kLayoutNames) {
        if (EqualsIgnoreCase(s, entry.name)) {
            *out = entry.kind;
            return true;
        }
    }
    return false;
}

}  // namespace ht
