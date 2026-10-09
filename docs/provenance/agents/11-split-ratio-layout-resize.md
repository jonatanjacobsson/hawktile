# 11. split-ratio layout + resize

| | |
|---|---|
| Launched | 2026-10-09 06:57 UTC |
| Role | implementer |
| Model | Claude Haiku 5.5 |
| Agent id | `af93c1d3777ad183b` |

## Instructions from the planner (verbatim)

````text
You are modifying the layout fragment of HawkTile, a clean-room tiling window manager written as a Windhawk mod (C++23, clang, mingw target). Project root: <repo>

## Ground rules
- Read `src/10_contract.h` — the "Layout (20_layout.cpp)" section was just rewritten and is the spec: LayoutParams gained `ratios`, ComputeLayout is now defined as a sequence of binary splits, and there are two new functions SplitRatio and ApplyResize. Then read `src/20_layout.cpp` and `tests/layout_test.cpp` fully.
- Edit ONLY `src/20_layout.cpp` and `tests/layout_test.cpp`.
- Pure functions only (RECT and math; no window APIs). Keep the existing gap algorithm: g = max(0, innerGap); a = g/2; b = g - a; Expanded = {L-a, T-a, R+b, B+b}; raw slots partition Expanded exactly; final rect = {l+a, t+a, r-b, b-b} clamped to >= 1 px. Splits operate on the Expanded coordinates.
- Clean room: do NOT look at or reproduce code from komorebi, GlazeWM, Hyprland, i3, bspwm or any other window manager.
- Verify with `powershell -NoProfile -File <repo>\build.ps1 -Test` (must compile with 0 warnings and report 0 failed) and `... build.ps1 -Check 20_layout.cpp` (errors=0 warnings=0). Do NOT run the full build (another agent is changing 50_model.cpp in parallel and it may not link yet).
- Final report under 200 words.

## Implementation
1. ComputeLayout: rewrite as the split sequence in the contract. Implement a file-local helper that walks the splits and records, for each split i, its orientation (vertical/horizontal), the remaining region's extent [lo, hi) along the cut axis (Expanded coordinates) and the cut position; both ComputeLayout and ApplyResize use it, so geometry is identical. Monocle: every rect == area. count == 1: the whole area.
2. SplitRatio(p, count, split): exactly as the contract says (overrides valid when 0 < r < 1, clamped to [0.05, 0.95]; defaults per layout; masterRatio clamped; out-of-range split → 0.5).
3. ApplyResize(area, count, p, index, newRect): exactly as the contract says. Pad p.ratios with 0.0 up to count-1. Mapping between a visible edge and a cut, given the gap offsets: for side A (left/top part) the slot's right/bottom edge = cut - b, so newCut = newEdge + b; for side B (right/bottom part) the slot's left/top edge = cut + a, so newCut = newEdge - a. New ratio = (newCut - lo) / (hi - lo), clamped to [0.1, 0.9]. Only write splits whose edge moved by more than 2 px. Process candidate splits in increasing order and recompute the split geometry (and the window's slot) after each write. Return true if any ratio was written with a different value. Monocle, count < 2, or index out of range → false and p unchanged.

## Tests (tests/layout_test.cpp; keep the existing CHECK style)
- Keep all existing invariant tests (sizes, inside area, bounding box, no overlap, gap spacing) and run them also with a non-empty `ratios` vector (e.g. {0.3, 0.0, 0.7}).
- Re-derive any exact-coordinate expectations that changed because of the new cut formula (lround on the Expanded extent); compute them by hand from the spec and keep the old ones that still hold. Required: Dwindle n=3 gap 0 ratio 0.5 on {0,0,1920,1080} → {0,0,960,1080},{960,0,1920,540},{960,540,1920,1080}; MasterStack n=3 ratio 0.6 gap 0 → {0,0,1152,1080},{1152,0,1920,540},{1152,540,1920,1080}; Columns n=4 gap 0 on width 1920 → 480 each.
- SplitRatio: defaults per layout (Dwindle 0→masterRatio, 1→0.5; MasterStack 2 of n=4 → 1/2; Columns 0 of n=4 → 0.25), overrides, invalid overrides (0.0, 1.0, -1) fall back, clamping.
- ApplyResize (gap 0 and gap 8, area {0,0,1920,1080}):
  * Columns n=3: window 0's right edge dragged from its slot edge to x=900 → ratios[0] set so ComputeLayout gives window 0 right edge == 900 (within 1 px), window 1 left edge follows, window 2 unchanged.
  * Columns n=3: window 1's left edge dragged to x=500 → split 0 changes (window 0 shrinks), split 1 untouched.
  * Columns n=3: window 1 resized on BOTH sides → both splits change; result matches the requested edges within 1 px.
  * Dwindle n=3: window 2's top edge dragged up by 100 px → split 1 changes (window 1 gets shorter), split 0 untouched.
  * MasterStack n=3: window 0's right edge → split 0 (master ratio) changes.
  * Edge moved by only 2 px → returns false; Monocle → false; count 1 → false.
  * Clamping: dragging an edge to the far side clamps the ratio to 0.9 / 0.1.
  * With gap 8, after ApplyResize the adjusted edge lands exactly where requested (± 1 px) — this checks the gap offset mapping.

Run the test and check commands until both are clean.
````

## Report from the agent (2026-10-09 07:04 UTC)

````text
Layout fragment done. Edited only src/20_layout.cpp and tests/layout_test.cpp.

Verification: build.ps1 -Check 20_layout.cpp gives errors=0 warnings=0. build.ps1 -Test gives 0 warnings and 552 passed, 0 failed. Full build not run.

Implementation: file-local WalkSplits is shared by ComputeLayout and ApplyResize. SplitRatio and ApplyResize follow the contract. "Moved" is judged against the pre-call layout, and ratios are mapped onto the current remaining region.

Changed expectations (hand-derived): Columns n=3 gap 8 is now {0,0,635,1080},{643,0,1278,1080},{1286,0,1920,1080}. Columns n=2 gap 7 is now {0,0,957,1080},{964,0,1920,1080}. The other brief examples hold.

Decision needed: "window 2 unchanged" (Columns n=3, window 0 right edge to 900) and "window 1's right edge stays" (window 1 left edge to 500) cannot both hold under the contract. Ratios are relative to the remaining region and only moved splits are written, so unmoved far edges re-centre. Window 2 moves from x=1280 to 1410, and window 1's right edge moves from 1280 to 1210. The tests assert the contract (the unmoved split's ratio stays 0.0), not window 2's geometry. Fixing it means also writing non-moved neighbour splits to pin their cuts, which contradicts "only moved splits are written". Tell me if you want that change.
````

## Follow-up from the planner (2026-10-09 07:05 UTC)

````text
Planner here — good catch, thank you. Decision: only the dragged edges may move; all other cuts keep their absolute positions. I've rewritten the ApplyResize contract in src/10_contract.h (re-read that comment block): walk the old geometry and record each split's orientation + absolute cut; desired cut = new edge for moved splits, old cut for the rest; then walk from split 0 upward with the ratios written so far — moved split → write its ratio (clamp [0.1, 0.9]); unmoved split with unchanged orientation whose cut would now land elsewhere (>= 1 px) → write a pinning ratio for its old position (clamp [0.05, 0.95]); otherwise leave its entry alone (stays 0.0 = default).

Please update ApplyResize in src/20_layout.cpp accordingly and change the tests to assert geometry, not just ratios:
- Columns n=3 (gap 0 and 8), window 0 right edge → 900: window 0 right ≈ 900, window 1 left follows, window 1 RIGHT edge and window 2 unchanged (±1 px). ratios[1] is now written (pinned).
- Columns n=3, window 1 left edge → 500: window 0 right follows, window 1 right edge and window 2 unchanged.
- Columns n=4, window 2 right edge moved: splits 0 and 1 stay 0.0 (untouched, not pinned), window 3 unchanged.
- Dwindle n=3, window 2 top edge up 100 px: only split 1 changes; window 0 unchanged.
- Keep the rest of your ApplyResize tests. Same constraints as before: edit only src/20_layout.cpp and tests/layout_test.cpp, run build.ps1 -Test (0 warnings, 0 failed) and build.ps1 -Check 20_layout.cpp. The model fragment is finished now, so you may also run the full `build.ps1` at the end; report its result. Reply with a short report.
````

## Report from the agent (2026-10-09 07:08 UTC)

````text
Layout fragment done. Edited only src/20_layout.cpp and tests/layout_test.cpp.

ApplyResize follows the new contract. Pass 1 records each split's orientation and old cut and marks the dragged cuts; "moved" is judged against the pre-call slot edges. Pass 2 walks from split 0 upward: a moved split gets its ratio (clamped to [0.1, 0.9]); an unmoved split whose cut would drift by 1 px or more gets a pinning ratio (clamped to [0.05, 0.95]). A split whose orientation changed is skipped.

Results: build.ps1 -Check 20_layout.cpp gives errors=0 warnings=0. build.ps1 -Test gives 0 warnings and 588 passed, 0 failed. Full build.ps1 gives errors=0 warnings=0 (host and real-mode syntax checks).

Tests assert geometry for the cases you listed. Columns n=3 at gaps 0 and 8: window 0's right edge to 900 pins window 1's right edge and leaves window 2 in place. Window 1's left edge to 500 keeps its right edge and window 2 in place. Columns n=4: window 2's right edge leaves splits 0 and 1 at exactly 0.0. Dwindle: only split 1 changes.

Two points for you to decide:
1. Columns n=4, window 2's right edge: window 3 shares that cut, so its left edge must follow. "Window 3 unchanged" cannot hold. The test asserts window 3's right edge stays at 1920 and its left edge equals window 2's new right edge.
2. The clamp test uses two windows. With three, a drag past the 0.9 clamp crosses window 1's pinned cut, and the 0.05 pin then shifts window 2. This only happens when a clamped drag crosses an unmoved cut.
````
