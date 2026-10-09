// =============================================================================
// layout_test.cpp — checks for 20_layout.cpp. Concatenated after 10_contract.h
// and 20_layout.cpp, so it calls ht:: directly.
// =============================================================================

namespace {

int g_passed = 0;
int g_failed = 0;

// Stringify, then widen: L##"text" needs an already-expanded string literal.
#define HT_TEST_STR_I(x) #x
#define HT_TEST_STR(x) HT_TEST_STR_I(x)
#define HT_TEST_WSTR_I(x) L##x
#define HT_TEST_WSTR_E(x) HT_TEST_WSTR_I(x)
#define HT_TEST_WSTR(x) HT_TEST_WSTR_E(HT_TEST_STR(x))

#define CHECK(expr)                                                              \
    do {                                                                         \
        if (expr) {                                                              \
            ++g_passed;                                                          \
        } else {                                                                 \
            ++g_failed;                                                          \
            fwprintf(stderr, L"" __FILE__ L":%d: CHECK failed: %ls\n", __LINE__,  \
                     HT_TEST_WSTR(expr));                                        \
        }                                                                        \
    } while (0)

using ht::Direction;
using ht::LayoutKind;
using ht::LayoutParams;

constexpr LayoutKind kAllKinds[] = {LayoutKind::Dwindle, LayoutKind::MasterStack,
                                    LayoutKind::Columns, LayoutKind::Rows,
                                    LayoutKind::Monocle};
const RECT kArea{0, 0, 1920, 1080};

// Ratio sets run through the invariant checks: all defaults, and a mix of
// overrides and defaults.
const std::vector<std::vector<double>> kRatioSets = {{}, {0.3, 0.0, 0.7}};

// Flip sets run through the same checks: none, and a mix of inverted and automatic
// Dwindle splits.
const std::vector<std::vector<int>> kFlipSets = {{}, {1, 0, 1}};

RECT Rc(LONG left, LONG top, LONG right, LONG bottom) {
    return RECT{left, top, right, bottom};
}

bool SameRect(const RECT& a, const RECT& b) {
    return a.left == b.left && a.top == b.top && a.right == b.right &&
           a.bottom == b.bottom;
}

void PrintRect(const RECT& rc) {
    fwprintf(stderr, L"{%ld,%ld,%ld,%ld}", rc.left, rc.top, rc.right, rc.bottom);
}

// Compares a whole layout result with expected rects, in order.
void ExpectLayout(const wchar_t* what, const std::vector<RECT>& got,
                  const std::vector<RECT>& want) {
    if (got.size() != want.size()) {
        ++g_failed;
        fwprintf(stderr, L"%ls: got %d rects, want %d\n", what,
                 static_cast<int>(got.size()), static_cast<int>(want.size()));
        return;
    }
    for (size_t i = 0; i < got.size(); ++i) {
        if (SameRect(got[i], want[i])) {
            ++g_passed;
            continue;
        }
        ++g_failed;
        fwprintf(stderr, L"%ls: rect %d is ", what, static_cast<int>(i));
        PrintRect(got[i]);
        fwprintf(stderr, L", want ");
        PrintRect(want[i]);
        fwprintf(stderr, L"\n");
    }
}

// True when got is within tol px of want.
bool WithinPx(LONG got, LONG want, LONG tol) {
    const LONG d = got - want;
    return d <= tol && -d <= tol;
}

// Ratios are computed in double; this is far tighter than any pixel effect.
bool Near(double got, double want) {
    return std::fabs(got - want) < 1e-9;
}

// First violated invariant of a layout result for `kArea`, or nullptr.
const wchar_t* InvariantViolation(const std::vector<RECT>& rects, int count,
                                  LayoutKind kind) {
    if (static_cast<int>(rects.size()) != count) {
        return L"size != count";
    }
    if (rects.empty()) {
        return nullptr;
    }
    for (const RECT& rc : rects) {
        if (rc.left < kArea.left || rc.top < kArea.top || rc.right > kArea.right ||
            rc.bottom > kArea.bottom) {
            return L"rect outside area";
        }
        if (rc.right - rc.left < 1 || rc.bottom - rc.top < 1) {
            return L"rect smaller than 1 px";
        }
    }

    LONG left = rects[0].left;
    LONG top = rects[0].top;
    LONG right = rects[0].right;
    LONG bottom = rects[0].bottom;
    for (const RECT& rc : rects) {
        left = std::min(left, rc.left);
        top = std::min(top, rc.top);
        right = std::max(right, rc.right);
        bottom = std::max(bottom, rc.bottom);
    }
    if (left != kArea.left || top != kArea.top || right != kArea.right ||
        bottom != kArea.bottom) {
        return L"bounding box != area";
    }

    if (kind == LayoutKind::Monocle) {
        for (const RECT& rc : rects) {
            if (!SameRect(rc, kArea)) {
                return L"monocle rect != area";
            }
        }
        return nullptr;
    }

    for (size_t i = 0; i < rects.size(); ++i) {
        for (size_t j = i + 1; j < rects.size(); ++j) {
            const RECT& a = rects[i];
            const RECT& b = rects[j];
            if (a.left < b.right && b.left < a.right && a.top < b.bottom &&
                b.top < a.bottom) {
                return L"rects overlap";
            }
        }
    }
    return nullptr;
}

void TestInvariants() {
    const int gaps[] = {0, 8, 7};
    for (const std::vector<double>& ratios : kRatioSets) {
        for (const std::vector<int>& flips : kFlipSets) {
            for (LayoutKind kind : kAllKinds) {
                for (int gap : gaps) {
                    for (int count = 0; count <= 7; ++count) {
                        LayoutParams p;
                        p.kind = kind;
                        p.innerGap = gap;
                        p.ratios = ratios;
                        p.flips = flips;
                        const std::vector<RECT> rects = ht::ComputeLayout(kArea, count, p);
                        const wchar_t* why = InvariantViolation(rects, count, kind);
                        if (why != nullptr) {
                            ++g_failed;
                            fwprintf(stderr,
                                     L"" __FILE__ L": %ls (layout=%ls gap=%d count=%d "
                                     L"ratios=%d flips=%d)\n",
                                     why, ht::LayoutName(kind), gap, count,
                                     static_cast<int>(ratios.size()),
                                     static_cast<int>(flips.size()));
                        } else {
                            ++g_passed;
                        }
                    }
                }
            }
        }
    }
}

// Gap between two non-overlapping rects that share a perpendicular extent.
// Returns -1 for overlapping pairs and for diagonal neighbours.
int Separation(const RECT& a, const RECT& b) {
    const bool sharesY = a.top < b.bottom && b.top < a.bottom;
    const bool sharesX = a.left < b.right && b.left < a.right;
    if (sharesY) {
        if (a.right <= b.left) {
            return static_cast<int>(b.left - a.right);
        }
        if (b.right <= a.left) {
            return static_cast<int>(a.left - b.right);
        }
    }
    if (sharesX) {
        if (a.bottom <= b.top) {
            return static_cast<int>(b.top - a.bottom);
        }
        if (b.bottom <= a.top) {
            return static_cast<int>(a.top - b.bottom);
        }
    }
    return -1;
}

// With gap g, every pair of neighbours is exactly g apart, and no pair is closer.
void TestGapSpacing() {
    const LayoutKind kinds[] = {LayoutKind::Dwindle, LayoutKind::MasterStack,
                                LayoutKind::Columns, LayoutKind::Rows};
    const int gaps[] = {0, 7, 8};
    for (const std::vector<double>& ratios : kRatioSets) {
        for (const std::vector<int>& flips : kFlipSets) {
            for (LayoutKind kind : kinds) {
                for (int gap : gaps) {
                    for (int count = 2; count <= 7; ++count) {
                        LayoutParams p;
                        p.kind = kind;
                        p.innerGap = gap;
                        p.masterRatio = 0.6;
                        p.ratios = ratios;
                        p.flips = flips;
                        const std::vector<RECT> rects = ht::ComputeLayout(kArea, count, p);
                        int smallest = -1;
                        bool tooClose = false;
                        for (size_t i = 0; i < rects.size(); ++i) {
                            for (size_t j = i + 1; j < rects.size(); ++j) {
                                const int sep = Separation(rects[i], rects[j]);
                                if (sep < 0) {
                                    continue;
                                }
                                if (sep < gap) {
                                    tooClose = true;
                                }
                                if (smallest < 0 || sep < smallest) {
                                    smallest = sep;
                                }
                            }
                        }
                        if (tooClose || smallest != gap) {
                            ++g_failed;
                            fwprintf(stderr,
                                     L"" __FILE__ L": neighbour gap (layout=%ls gap=%d "
                                     L"count=%d smallest=%d ratios=%d flips=%d)\n",
                                     ht::LayoutName(kind), gap, count, smallest,
                                     static_cast<int>(ratios.size()),
                                     static_cast<int>(flips.size()));
                        } else {
                            ++g_passed;
                        }
                    }
                }
            }
        }
    }
}

void TestExplicitLayouts() {
    LayoutParams p;

    p.kind = LayoutKind::Dwindle;
    p.masterRatio = 0.5;
    p.innerGap = 0;
    ExpectLayout(L"dwindle n=3 gap=0", ht::ComputeLayout(kArea, 3, p),
                 {Rc(0, 0, 960, 1080), Rc(960, 0, 1920, 540), Rc(960, 540, 1920, 1080)});
    p.innerGap = 8;
    ExpectLayout(L"dwindle n=3 gap=8", ht::ComputeLayout(kArea, 3, p),
                 {Rc(0, 0, 956, 1080), Rc(964, 0, 1920, 536), Rc(964, 544, 1920, 1080)});

    p.kind = LayoutKind::MasterStack;
    p.masterRatio = 0.6;
    p.innerGap = 0;
    ExpectLayout(L"master n=3 gap=0", ht::ComputeLayout(kArea, 3, p),
                 {Rc(0, 0, 1152, 1080), Rc(1152, 0, 1920, 540), Rc(1152, 540, 1920, 1080)});
    p.innerGap = 8;
    ExpectLayout(L"master n=3 gap=8", ht::ComputeLayout(kArea, 3, p),
                 {Rc(0, 0, 1149, 1080), Rc(1157, 0, 1920, 536), Rc(1157, 544, 1920, 1080)});

    p.kind = LayoutKind::Columns;
    p.innerGap = 0;
    ExpectLayout(L"columns n=3 gap=0", ht::ComputeLayout(kArea, 3, p),
                 {Rc(0, 0, 640, 1080), Rc(640, 0, 1280, 1080), Rc(1280, 0, 1920, 1080)});
    ExpectLayout(L"columns n=4 gap=0", ht::ComputeLayout(kArea, 4, p),
                 {Rc(0, 0, 480, 1080), Rc(480, 0, 960, 1080), Rc(960, 0, 1440, 1080),
                  Rc(1440, 0, 1920, 1080)});
    p.innerGap = 8;
    // Expanded width 1928: cut 0 = -4 + lround(642.67) = 639; cut 1 = 639 +
    // lround(642.5) = 1282. Final rects are offset by a = 4 and b = 4.
    ExpectLayout(L"columns n=3 gap=8", ht::ComputeLayout(kArea, 3, p),
                 {Rc(0, 0, 635, 1080), Rc(643, 0, 1278, 1080), Rc(1286, 0, 1920, 1080)});
    p.innerGap = 7;
    // Expanded width 1927 (a = 3, b = 4): cut = -3 + lround(963.5) = 961.
    ExpectLayout(L"columns n=2 gap=7", ht::ComputeLayout(kArea, 2, p),
                 {Rc(0, 0, 957, 1080), Rc(964, 0, 1920, 1080)});

    // Explicit overrides: split 0 at 0.25, split 1 at 0.5 of the remainder.
    p.innerGap = 0;
    p.ratios = {0.25, 0.5};
    ExpectLayout(L"columns n=3 ratios={0.25,0.5}", ht::ComputeLayout(kArea, 3, p),
                 {Rc(0, 0, 480, 1080), Rc(480, 0, 1200, 1080), Rc(1200, 0, 1920, 1080)});
    p.ratios.clear();

    p.kind = LayoutKind::Rows;
    p.innerGap = 0;
    ExpectLayout(L"rows n=3 gap=0", ht::ComputeLayout(kArea, 3, p),
                 {Rc(0, 0, 1920, 360), Rc(0, 360, 1920, 720), Rc(0, 720, 1920, 1080)});

    p.kind = LayoutKind::Monocle;
    p.innerGap = 8;
    ExpectLayout(L"monocle n=3 gap=8", ht::ComputeLayout(kArea, 3, p),
                 {kArea, kArea, kArea});

    for (LayoutKind kind : kAllKinds) {
        p.kind = kind;
        p.innerGap = 8;
        ExpectLayout(L"single window fills area", ht::ComputeLayout(kArea, 1, p),
                     {kArea});
    }
}

void TestMasterRatioClamp() {
    LayoutParams p;
    p.kind = LayoutKind::MasterStack;
    p.innerGap = 0;
    p.masterRatio = 0.0;
    ExpectLayout(L"ratio 0 clamps to 0.05", ht::ComputeLayout(kArea, 2, p),
                 {Rc(0, 0, 96, 1080), Rc(96, 0, 1920, 1080)});
    p.masterRatio = 2.0;
    ExpectLayout(L"ratio 2 clamps to 0.95", ht::ComputeLayout(kArea, 2, p),
                 {Rc(0, 0, 1824, 1080), Rc(1824, 0, 1920, 1080)});
}

void TestSplitRatio() {
    LayoutParams p;

    // Defaults per layout.
    p.kind = LayoutKind::Dwindle;
    p.masterRatio = 0.6;
    CHECK(Near(ht::SplitRatio(p, 3, 0), 0.6));
    CHECK(Near(ht::SplitRatio(p, 3, 1), 0.5));

    p.kind = LayoutKind::MasterStack;
    CHECK(Near(ht::SplitRatio(p, 4, 0), 0.6));
    CHECK(Near(ht::SplitRatio(p, 4, 1), 1.0 / 3.0));
    CHECK(Near(ht::SplitRatio(p, 4, 2), 0.5));

    p.kind = LayoutKind::Columns;
    CHECK(Near(ht::SplitRatio(p, 4, 0), 0.25));
    CHECK(Near(ht::SplitRatio(p, 4, 1), 1.0 / 3.0));
    CHECK(Near(ht::SplitRatio(p, 4, 2), 0.5));

    p.kind = LayoutKind::Rows;
    CHECK(Near(ht::SplitRatio(p, 4, 0), 0.25));

    // Out-of-range split indices.
    CHECK(ht::SplitRatio(p, 4, -1) == 0.5);
    CHECK(ht::SplitRatio(p, 4, 3) == 0.5);
    CHECK(ht::SplitRatio(p, 1, 0) == 0.5);

    // Overrides apply where 0 < r < 1, clamped to [0.05, 0.95].
    p.kind = LayoutKind::Columns;
    p.ratios = {0.3};
    CHECK(Near(ht::SplitRatio(p, 4, 0), 0.3));
    CHECK(Near(ht::SplitRatio(p, 4, 1), 1.0 / 3.0));

    // Invalid overrides fall back to the defaults.
    p.ratios = {0.0, 1.0, -1.0};
    CHECK(Near(ht::SplitRatio(p, 4, 0), 0.25));
    CHECK(Near(ht::SplitRatio(p, 4, 1), 1.0 / 3.0));
    CHECK(Near(ht::SplitRatio(p, 4, 2), 0.5));

    p.ratios = {0.001, 0.999};
    CHECK(Near(ht::SplitRatio(p, 4, 0), 0.05));
    CHECK(Near(ht::SplitRatio(p, 4, 1), 0.95));
    p.ratios.clear();

    // masterRatio is clamped as well.
    p.kind = LayoutKind::Dwindle;
    p.masterRatio = 0.0;
    CHECK(Near(ht::SplitRatio(p, 3, 0), 0.05));
    p.masterRatio = 2.0;
    CHECK(Near(ht::SplitRatio(p, 3, 0), 0.95));
}

void TestApplyResize() {
    const int gaps[] = {0, 8};

    // Columns n=3: window 0's right edge dragged to x=900. Its cut moves. The cut
    // at window 1's right edge is pinned, so window 1's right edge and window 2
    // keep their places.
    for (int gap : gaps) {
        LayoutParams p;
        p.kind = LayoutKind::Columns;
        p.innerGap = gap;
        const std::vector<RECT> orig = ht::ComputeLayout(kArea, 3, p);
        RECT target = orig[0];
        target.right = 900;
        CHECK(ht::ApplyResize(kArea, 3, p, 0, target));
        CHECK(p.ratios[1] != 0.0);  // pinned
        const std::vector<RECT> rects = ht::ComputeLayout(kArea, 3, p);
        CHECK(WithinPx(rects[0].right, 900, 1));
        CHECK(WithinPx(rects[1].left, 900 + gap, 1));
        CHECK(WithinPx(rects[1].right, orig[1].right, 1));
        CHECK(WithinPx(rects[2].left, orig[2].left, 1));
        CHECK(WithinPx(rects[2].right, orig[2].right, 1));
    }

    // Columns n=3: window 1's left edge dragged to x=500. Window 0's right edge
    // follows; window 1's right edge and window 2 keep their places.
    for (int gap : gaps) {
        LayoutParams p;
        p.kind = LayoutKind::Columns;
        p.innerGap = gap;
        const std::vector<RECT> orig = ht::ComputeLayout(kArea, 3, p);
        RECT target = orig[1];
        target.left = 500;
        CHECK(ht::ApplyResize(kArea, 3, p, 1, target));
        const std::vector<RECT> rects = ht::ComputeLayout(kArea, 3, p);
        CHECK(WithinPx(rects[0].right, 500 - gap, 1));
        CHECK(WithinPx(rects[1].left, 500, 1));
        CHECK(WithinPx(rects[1].right, orig[1].right, 1));
        CHECK(WithinPx(rects[2].left, orig[2].left, 1));
        CHECK(WithinPx(rects[2].right, orig[2].right, 1));
    }

    // Columns n=3, gap 0: window 1 resized on both sides. Both splits move, and
    // the edges land where requested.
    {
        LayoutParams p;
        p.kind = LayoutKind::Columns;
        p.innerGap = 0;
        const std::vector<RECT> orig = ht::ComputeLayout(kArea, 3, p);
        RECT target = orig[1];
        target.left = 500;
        target.right = 1300;
        CHECK(ht::ApplyResize(kArea, 3, p, 1, target));
        CHECK(Near(p.ratios[0], 500.0 / 1920.0));
        CHECK(p.ratios[1] != 0.0);
        const std::vector<RECT> rects = ht::ComputeLayout(kArea, 3, p);
        CHECK(WithinPx(rects[0].right, 500, 1));
        CHECK(WithinPx(rects[1].left, 500, 1));
        CHECK(WithinPx(rects[1].right, 1300, 1));
        CHECK(WithinPx(rects[2].left, 1300, 1));
    }

    // Columns n=4, gap 0: window 2's right edge dragged to x=1400. Splits 0 and 1
    // are untouched, not pinned. Window 3 shares the dragged cut, so its left edge
    // follows; its right edge stays at the area edge.
    {
        LayoutParams p;
        p.kind = LayoutKind::Columns;
        p.innerGap = 0;
        const std::vector<RECT> orig = ht::ComputeLayout(kArea, 4, p);
        RECT target = orig[2];
        target.right = 1400;
        CHECK(ht::ApplyResize(kArea, 4, p, 2, target));
        CHECK(p.ratios[0] == 0.0);
        CHECK(p.ratios[1] == 0.0);
        const std::vector<RECT> rects = ht::ComputeLayout(kArea, 4, p);
        CHECK(SameRect(rects[0], orig[0]));
        CHECK(SameRect(rects[1], orig[1]));
        CHECK(WithinPx(rects[2].left, orig[2].left, 1));
        CHECK(WithinPx(rects[2].right, 1400, 1));
        CHECK(WithinPx(rects[3].left, 1400, 1));
        CHECK(WithinPx(rects[3].right, 1920, 1));
    }

    // Dwindle n=3, gap 0, ratio 0.5: window 2's top edge dragged up by 100 px.
    // Only split 1 changes; window 0 is unaffected.
    {
        LayoutParams p;
        p.kind = LayoutKind::Dwindle;
        p.masterRatio = 0.5;
        p.innerGap = 0;
        const std::vector<RECT> orig = ht::ComputeLayout(kArea, 3, p);
        RECT target = orig[2];
        target.top = 440;
        CHECK(ht::ApplyResize(kArea, 3, p, 2, target));
        CHECK(p.ratios[0] == 0.0);
        CHECK(Near(p.ratios[1], 440.0 / 1080.0));
        const std::vector<RECT> rects = ht::ComputeLayout(kArea, 3, p);
        CHECK(SameRect(rects[0], orig[0]));
        CHECK(WithinPx(rects[1].bottom, 440, 1));
        CHECK(WithinPx(rects[2].top, 440, 1));
    }

    // Dwindle n=3, gap 0, flips {0,1}: split 1 is inverted, so it is vertical and
    // window 1's right edge is its cut. Dragging that edge to x=1300 moves split 1
    // only; split 0 keeps its default.
    {
        LayoutParams p;
        p.kind = LayoutKind::Dwindle;
        p.masterRatio = 0.5;
        p.innerGap = 0;
        p.flips = {0, 1};
        const std::vector<RECT> orig = ht::ComputeLayout(kArea, 3, p);
        CHECK(SameRect(orig[1], Rc(960, 0, 1440, 1080)));
        RECT target = orig[1];
        target.right = 1300;
        CHECK(ht::ApplyResize(kArea, 3, p, 1, target));
        CHECK(p.ratios[0] == 0.0);
        CHECK(Near(p.ratios[1], 340.0 / 960.0));
        const std::vector<RECT> rects = ht::ComputeLayout(kArea, 3, p);
        CHECK(SameRect(rects[0], orig[0]));
        CHECK(WithinPx(rects[1].right, 1300, 1));
        CHECK(WithinPx(rects[2].left, 1300, 1));
        CHECK(WithinPx(rects[2].right, 1920, 1));
    }

    // Dwindle n=2, gap 0, flips {1}: split 0 is horizontal, so window 0's bottom
    // edge is its cut. Dragging it to y=700 moves split 0.
    {
        LayoutParams p;
        p.kind = LayoutKind::Dwindle;
        p.innerGap = 0;
        p.flips = {1};
        const std::vector<RECT> orig = ht::ComputeLayout(kArea, 2, p);
        RECT target = orig[0];
        target.bottom = 700;
        CHECK(ht::ApplyResize(kArea, 2, p, 0, target));
        CHECK(Near(p.ratios[0], 700.0 / 1080.0));
        const std::vector<RECT> rects = ht::ComputeLayout(kArea, 2, p);
        CHECK(WithinPx(rects[0].bottom, 700, 1));
        CHECK(WithinPx(rects[1].top, 700, 1));
    }

    // MasterStack n=3, gap 0, ratio 0.6: window 0's right edge moves split 0. The
    // stack's horizontal split is unaffected by an x-only change.
    {
        LayoutParams p;
        p.kind = LayoutKind::MasterStack;
        p.masterRatio = 0.6;
        p.innerGap = 0;
        const std::vector<RECT> orig = ht::ComputeLayout(kArea, 3, p);
        RECT target = orig[0];
        target.right = 900;
        CHECK(ht::ApplyResize(kArea, 3, p, 0, target));
        CHECK(Near(p.ratios[0], 900.0 / 1920.0));
        CHECK(p.ratios[1] == 0.0);
        const std::vector<RECT> rects = ht::ComputeLayout(kArea, 3, p);
        CHECK(WithinPx(rects[0].right, 900, 1));
        CHECK(SameRect(rects[1], Rc(900, 0, 1920, 540)));
        CHECK(SameRect(rects[2], Rc(900, 540, 1920, 1080)));
    }

    // Movement threshold: 2 px is not a move, 3 px is.
    {
        LayoutParams p;
        p.kind = LayoutKind::Columns;
        p.innerGap = 0;
        const std::vector<RECT> orig = ht::ComputeLayout(kArea, 3, p);
        RECT target = orig[0];
        CHECK(!ht::ApplyResize(kArea, 3, p, 0, target));
        target.right = 642;
        CHECK(!ht::ApplyResize(kArea, 3, p, 0, target));
        target.right = 643;
        CHECK(ht::ApplyResize(kArea, 3, p, 0, target));
    }

    // Monocle, count < 2 and out-of-range index are no-ops.
    {
        LayoutParams p;
        p.kind = LayoutKind::Monocle;
        CHECK(!ht::ApplyResize(kArea, 3, p, 0, kArea));
        CHECK(p.ratios.empty());
        p.kind = LayoutKind::Columns;
        CHECK(!ht::ApplyResize(kArea, 1, p, 0, kArea));
        CHECK(!ht::ApplyResize(kArea, 3, p, 3, kArea));
        CHECK(!ht::ApplyResize(kArea, 3, p, -1, kArea));
        CHECK(p.ratios.empty());
    }

    // Clamping: a far-side drag lands on 0.9 (or 0.1). Two windows, so no pinned
    // split sits on the far side of the dragged cut.
    {
        LayoutParams p;
        p.kind = LayoutKind::Columns;
        p.innerGap = 0;
        const std::vector<RECT> orig = ht::ComputeLayout(kArea, 2, p);
        RECT target = orig[0];
        target.right = 1900;
        CHECK(ht::ApplyResize(kArea, 2, p, 0, target));
        CHECK(p.ratios[0] == 0.9);
    }
    {
        LayoutParams p;
        p.kind = LayoutKind::Columns;
        p.innerGap = 0;
        const std::vector<RECT> orig = ht::ComputeLayout(kArea, 2, p);
        RECT target = orig[0];
        target.right = 10;
        CHECK(ht::ApplyResize(kArea, 2, p, 0, target));
        CHECK(p.ratios[0] == 0.1);
    }

    // Existing overrides are kept for splits that do not move.
    {
        LayoutParams p;
        p.kind = LayoutKind::Columns;
        p.innerGap = 0;
        p.ratios = {0.3, 0.0, 0.7};
        const std::vector<RECT> orig = ht::ComputeLayout(kArea, 3, p);
        RECT target = orig[0];
        target.right = 900;
        CHECK(ht::ApplyResize(kArea, 3, p, 0, target));
        CHECK(p.ratios[2] == 0.7);
        const std::vector<RECT> rects = ht::ComputeLayout(kArea, 3, p);
        CHECK(WithinPx(rects[0].right, 900, 1));
        CHECK(WithinPx(rects[1].right, orig[1].right, 1));
    }

    // With a gap, the adjusted edge lands where requested, and the neighbour
    // across the gap stays exactly innerGap away. Side A subtracts b from the cut
    // and side B adds a, so odd and even gaps both need checking.
    const int mapGaps[] = {7, 8};
    for (int gap : mapGaps) {
        LayoutParams p;
        p.kind = LayoutKind::Columns;
        p.innerGap = gap;
        std::vector<RECT> orig = ht::ComputeLayout(kArea, 3, p);
        RECT target = orig[0];
        target.right = 900;
        CHECK(ht::ApplyResize(kArea, 3, p, 0, target));
        std::vector<RECT> rects = ht::ComputeLayout(kArea, 3, p);
        CHECK(WithinPx(rects[0].right, 900, 1));
        CHECK(Separation(rects[0], rects[1]) == gap);
        CHECK(WithinPx(rects[1].right, orig[1].right, 1));

        p = LayoutParams();
        p.kind = LayoutKind::Columns;
        p.innerGap = gap;
        orig = ht::ComputeLayout(kArea, 3, p);
        target = orig[1];
        target.left = 500;
        CHECK(ht::ApplyResize(kArea, 3, p, 1, target));
        rects = ht::ComputeLayout(kArea, 3, p);
        CHECK(WithinPx(rects[1].left, 500, 1));
        CHECK(Separation(rects[0], rects[1]) == gap);
        CHECK(WithinPx(rects[1].right, orig[1].right, 1));

        p = LayoutParams();
        p.kind = LayoutKind::Dwindle;
        p.masterRatio = 0.5;
        p.innerGap = gap;
        orig = ht::ComputeLayout(kArea, 3, p);
        target = orig[2];
        target.top = 440;
        CHECK(ht::ApplyResize(kArea, 3, p, 2, target));
        rects = ht::ComputeLayout(kArea, 3, p);
        CHECK(WithinPx(rects[2].top, 440, 1));
        CHECK(Separation(rects[1], rects[2]) == gap);
        CHECK(WithinPx(rects[0].right, orig[0].right, 1));
    }
}

void TestToggleSplit() {
    // Dwindle n=2, gap 0: the default split is vertical. Toggling window 0 makes it
    // horizontal; toggling again restores it.
    {
        LayoutParams p;
        p.kind = LayoutKind::Dwindle;
        p.innerGap = 0;
        p.masterRatio = 0.5;
        ExpectLayout(L"dwindle n=2 default", ht::ComputeLayout(kArea, 2, p),
                     {Rc(0, 0, 960, 1080), Rc(960, 0, 1920, 1080)});
        CHECK(ht::ToggleSplitAt(p, 2, 0));
        ExpectLayout(L"dwindle n=2 toggled", ht::ComputeLayout(kArea, 2, p),
                     {Rc(0, 0, 1920, 540), Rc(0, 540, 1920, 1080)});
        CHECK(ht::ToggleSplitAt(p, 2, 0));
        ExpectLayout(L"dwindle n=2 toggled back", ht::ComputeLayout(kArea, 2, p),
                     {Rc(0, 0, 960, 1080), Rc(960, 0, 1920, 1080)});
    }

    // Dwindle n=3: windows 1 and 2 both own split 1 (window 2 shares the previous
    // split), so either toggle flips split 1 and leaves split 0 alone. Windows 1
    // and 2 end up side by side instead of stacked.
    {
        const int owners[] = {1, 2};
        const std::vector<RECT> want = {Rc(0, 0, 960, 1080), Rc(960, 0, 1440, 1080),
                                        Rc(1440, 0, 1920, 1080)};
        for (int owner : owners) {
            LayoutParams p;
            p.kind = LayoutKind::Dwindle;
            p.innerGap = 0;
            p.masterRatio = 0.5;
            CHECK(ht::ToggleSplitAt(p, 3, owner));
            CHECK(p.flips.size() == 2);
            CHECK(p.flips[0] == 0);
            CHECK(p.flips[1] == 1);
            ExpectLayout(L"dwindle n=3 toggled split 1", ht::ComputeLayout(kArea, 3, p),
                         want);
        }
    }

    // Window 0 owns split 0 and nothing else changes in flips.
    {
        LayoutParams p;
        p.kind = LayoutKind::Dwindle;
        CHECK(ht::ToggleSplitAt(p, 3, 0));
        CHECK(p.flips.size() == 2);
        CHECK(p.flips[0] == 1);
        CHECK(p.flips[1] == 0);
    }

    // Other layouts refuse the toggle and keep their flips. Their geometry ignores
    // flips entirely.
    const LayoutKind others[] = {LayoutKind::MasterStack, LayoutKind::Columns,
                                 LayoutKind::Rows, LayoutKind::Monocle};
    for (LayoutKind kind : others) {
        LayoutParams plain;
        plain.kind = kind;
        LayoutParams p = plain;
        p.flips = {1, 0};
        CHECK(!ht::ToggleSplitAt(p, 3, 0));
        CHECK(p.flips == std::vector<int>({1, 0}));
        ExpectLayout(L"flips ignored by non-dwindle", ht::ComputeLayout(kArea, 3, p),
                     ht::ComputeLayout(kArea, 3, plain));
    }

    // count < 2 and out-of-range indices are refused and leave flips empty.
    {
        LayoutParams p;
        p.kind = LayoutKind::Dwindle;
        CHECK(!ht::ToggleSplitAt(p, 1, 0));
        CHECK(!ht::ToggleSplitAt(p, 0, 0));
        CHECK(!ht::ToggleSplitAt(p, 3, 3));
        CHECK(!ht::ToggleSplitAt(p, 3, -1));
        CHECK(p.flips.empty());
    }
}

void TestFindNeighbor() {
    using D = Direction;

    // 2x2 grid: 0=TL, 1=TR, 2=BL, 3=BR.
    const std::vector<RECT> grid = {Rc(0, 0, 960, 540), Rc(960, 0, 1920, 540),
                                    Rc(0, 540, 960, 1080), Rc(960, 540, 1920, 1080)};
    CHECK(ht::FindNeighbor(grid, 0, D::Right) == 1);
    CHECK(ht::FindNeighbor(grid, 0, D::Down) == 2);
    CHECK(ht::FindNeighbor(grid, 0, D::Left) == -1);
    CHECK(ht::FindNeighbor(grid, 0, D::Up) == -1);
    CHECK(ht::FindNeighbor(grid, 3, D::Left) == 2);
    CHECK(ht::FindNeighbor(grid, 3, D::Up) == 1);
    CHECK(ht::FindNeighbor(grid, 1, D::Left) == 0);
    CHECK(ht::FindNeighbor(grid, 2, D::Right) == 3);
    CHECK(ht::FindNeighbor(grid, 2, D::Up) == 0);
    CHECK(ht::FindNeighbor(grid, -1, D::Right) == -1);
    CHECK(ht::FindNeighbor(grid, 4, D::Right) == -1);
    CHECK(ht::FindNeighbor({}, 0, D::Right) == -1);

    // Overlap preference: from the bottom-right window, window 1's centre is
    // closer, but it does not share a vertical extent, so the tall left window wins.
    const std::vector<RECT> stacked = {Rc(0, 0, 960, 1080), Rc(900, 0, 1920, 520),
                                       Rc(960, 520, 1920, 1080)};
    CHECK(ht::FindNeighbor(stacked, 2, D::Left) == 0);
    CHECK(ht::FindNeighbor(stacked, 2, D::Up) == 1);

    // Identical rects are never candidates for each other.
    const std::vector<RECT> twins = {Rc(0, 0, 100, 100), Rc(0, 0, 100, 100),
                                     Rc(200, 0, 300, 100)};
    CHECK(ht::FindNeighbor(twins, 0, D::Right) == 2);
    CHECK(ht::FindNeighbor(twins, 0, D::Left) == -1);

    const std::vector<RECT> rows = {Rc(0, 0, 1920, 360), Rc(0, 360, 1920, 720),
                                    Rc(0, 720, 1920, 1080)};
    CHECK(ht::FindNeighbor(rows, 1, D::Up) == 0);
    CHECK(ht::FindNeighbor(rows, 1, D::Down) == 2);
    CHECK(ht::FindNeighbor(rows, 0, D::Down) == 1);
    CHECK(ht::FindNeighbor(rows, 0, D::Right) == -1);
}

void TestNextLayout() {
    LayoutKind k = LayoutKind::Dwindle;
    const LayoutKind want[] = {LayoutKind::MasterStack, LayoutKind::Columns,
                               LayoutKind::Rows, LayoutKind::Monocle,
                               LayoutKind::Dwindle};
    for (LayoutKind w : want) {
        k = ht::NextLayout(k);
        CHECK(k == w);
    }
}

bool ParsesTo(const wchar_t* text, LayoutKind want) {
    LayoutKind got = LayoutKind::Count_;
    return ht::ParseLayoutName(text, &got) && got == want;
}

bool RejectsName(const wchar_t* text) {
    LayoutKind got = LayoutKind::Count_;
    return !ht::ParseLayoutName(text, &got);
}

void TestLayoutNames() {
    for (LayoutKind kind : kAllKinds) {
        CHECK(ParsesTo(ht::LayoutName(kind), kind));
    }
    CHECK(std::wcscmp(ht::LayoutName(LayoutKind::Dwindle), L"dwindle") == 0);
    CHECK(std::wcscmp(ht::LayoutName(LayoutKind::MasterStack), L"master") == 0);
    CHECK(std::wcscmp(ht::LayoutName(LayoutKind::Count_), L"unknown") == 0);

    CHECK(ParsesTo(L"DWINDLE", LayoutKind::Dwindle));
    CHECK(ParsesTo(L"BSP", LayoutKind::Dwindle));
    CHECK(ParsesTo(L"Master", LayoutKind::MasterStack));
    CHECK(ParsesTo(L"Master-Stack", LayoutKind::MasterStack));
    CHECK(ParsesTo(L"COLUMNS", LayoutKind::Columns));
    CHECK(ParsesTo(L"rows", LayoutKind::Rows));
    CHECK(ParsesTo(L"Monocle", LayoutKind::Monocle));

    CHECK(RejectsName(nullptr));
    CHECK(RejectsName(L""));
    CHECK(RejectsName(L"foo"));
    CHECK(RejectsName(L"dwindles"));
    CHECK(RejectsName(L"mono"));
    CHECK(RejectsName(L"unknown"));
}

}  // namespace

int main() {
    TestInvariants();
    TestGapSpacing();
    TestExplicitLayouts();
    TestMasterRatioClamp();
    TestSplitRatio();
    TestApplyResize();
    TestToggleSplit();
    TestFindNeighbor();
    TestNextLayout();
    TestLayoutNames();

    fwprintf(stdout, L"%d passed, %d failed\n", g_passed, g_failed);
    return g_failed == 0 ? 0 : 1;
}
