# 3. config/hotkey parsing fragment

| | |
|---|---|
| Launched | 2026-10-09 06:18 UTC |
| Role | implementer |
| Model | Claude Haiku 5.5 |
| Agent id | `a6faf51d8a36a416c` |

## Instructions from the planner (verbatim)

````text
You are implementing one fragment of HawkTile, a clean-room tiling window manager written as a Windhawk mod (C++23, clang, mingw target). Project root: <repo>

## Ground rules (apply to every fragment)
- First read `src/10_contract.h` completely. It is the binding contract: types, globals and every cross-module function signature. Implement exactly those signatures. Do NOT edit 10_contract.h or any file other than the ones assigned to you. If you believe the contract is wrong, implement it as written and mention the problem in your final report.
- Fragments are concatenated into ONE translation unit after 10_contract.h, so do not #include project files. The contract already includes <windows.h>, <dwmapi.h>, <shellapi.h>, <algorithm>, <cstdio>, <cwchar>, <cwctype>, <string>, <unordered_set>, <vector>. You may add another standard-library #include at the top of your fragment if truly needed.
- Put all code inside `namespace ht { ... }`. File-local helpers go in an anonymous namespace inside `ht`.
- Log with `HT_LOG(L"format %s", x)` only (the format must be a string literal).
- Clean room: do NOT look at, search for, or reproduce code or config formats from komorebi/whkd, GlazeWM, Hyprland, i3, bspwm or any other window manager. Use only your own knowledge.
- Style: 4-space indent, `//` comments only where something is non-obvious, Chromium-like formatting, no exceptions.
- Verify with: `powershell -NoProfile -File <repo>\build.ps1 -Check <your-fragment-file-name>` — it must end with `errors=0 warnings=0`. Fix every warning. The check defines HT_STANDALONE; the real build does not, so both branches of any `#ifdef HT_STANDALONE` must be valid code.
- Never run anything that moves, hides or focuses real windows on this machine.
- Final report (keep it under 200 words): what you implemented, any deviation from the contract, any concerns.

## Your assignment: `src/40_config.cpp`

Implement: LoadSettings, ParseHotkey, ParseCommand, ParseRule, RuleMatches, AnyRuleMatches, RegisterBindings, UnregisterBindings. Semantics and grammars are documented in the contract. Read `src/00_header.wh.cpp` too: its `==WindhawkModSettings==` YAML block defines the setting names you read.

Windhawk settings API (declared in `C:\Program Files\Windhawk\Compiler\include\windhawk_api.h`; read it to confirm):
- `int Wh_GetIntSetting(PCWSTR name, ...)` — printf-style name, e.g. `Wh_GetIntSetting(L"outerGap")`. Booleans are read as ints (0/1).
- `PCWSTR Wh_GetStringSetting(PCWSTR name, ...)` — e.g. `Wh_GetStringSetting(L"bindings[%d].keys", i)`, `Wh_GetStringSetting(L"floatRules[%d]", i)`. Always release with `Wh_FreeStringSetting(p)`. Out-of-range array indexes return an empty string. Write a small RAII wrapper or helper that copies into std::wstring and frees.

LoadSettings(Settings& s):
1. `s = Settings{};` then fill floatRules from kDefaultFloatRules (via ParseRule) and bindings from kDefaultBindings (via ParseHotkey + ParseCommand; also set keysText/commandText).
2. `#ifndef HT_STANDALONE`: read outerGap, innerGap (clamp 0..200), workspaceCount (clamp 1..9), defaultLayout (ParseLayoutName; keep default and log if invalid), masterRatio (percent, clamp 15..85, store /100.0), ratioStep (percent, clamp 1..25, /100.0), focusBorder, focusBorderColor ("#RRGGBB" or "RRGGBB" hex → RGB(r,g,b); keep default and log if invalid), floatNonResizable, floatTopmost.
   floatRules and ignoreRules: read `name[i]` for i = 0.. until an empty string or i == 256; parse each with ParseRule; log and skip invalid ones. These REPLACE the defaults (an empty list is legitimate).
   bindings: for i = 0..255 read `bindings[%d].keys` and `bindings[%d].command`; stop when both are empty; skip (with a log line) entries where either fails to parse. If at least one valid binding was read, it REPLACES the defaults; otherwise keep the defaults and log that.
3. Log a one-line summary (gaps, workspaces, layout name via LayoutName, number of rules and bindings).

ParseHotkey: per contract. Case-insensitive, split on '+', trim spaces, reject empty tokens, duplicate non-modifier keys, or zero non-modifier keys. Letters → 'A'..'Z', digits → '0'..'9', f1..f24 → VK_F1 + n - 1. "vkXX": hex after "vk", value 1..254. Remember LayoutName etc. are in another fragment; ParseLayoutName is available.

ParseCommand: per contract grammar. Trim, lowercase, collapse multiple spaces. Workspace numbers must be 1..9. "layout X" uses ParseLayoutName. Unknown → false and out unchanged.

ParseRule: prefix "process:", "class:", "title:" (prefix case-insensitive); no prefix → process. Pattern is trimmed and stored lowercase; an empty pattern → false.

RuleMatches: compare lowercase versions (facts.className / facts.title may be mixed case; processName is already lowercase but lowercase it anyway). Process and Class: equality. Title: substring (find != npos).

RegisterBindings / UnregisterBindings: per contract (id = kHotkeyIdBase + index; MOD_NOREPEAT added on register). Log failures with keysText and GetLastError().

Run `build.ps1 -Check 40_config.cpp` until it reports errors=0 warnings=0. Additionally, write a throwaway test in the dist folder if you like (NOT in src/ or tests/) to exercise ParseHotkey/ParseCommand/ParseRule — e.g. concatenate 10_contract.h + src/20_layout.cpp stubs... note 20_layout.cpp is being written in parallel by someone else and may not exist yet; if you need ParseLayoutName for a throwaway test, provide a local stub in that throwaway file only. Make sure every entry of kDefaultBindings and kDefaultFloatRules parses successfully.
````

## Report from the agent (2026-10-09 06:26 UTC)

````text
Implemented src/40_config.cpp: LoadSettings, ParseHotkey, ParseCommand, ParseRule, RuleMatches, AnyRuleMatches, RegisterBindings, UnregisterBindings, plus file-local helpers. No contract deviations. I did not touch 10_contract.h or any other fragment.

Verification:
- build.ps1 -Check 40_config.cpp: errors=0 warnings=0.
- Real (non-standalone) branch compiled with the build flags and the Windhawk header: 0 warnings, 0 errors.
- Throwaway test with a mocked Windhawk settings API and stub LayoutName/ParseLayoutName: passes. It covers hotkey, command and rule edge cases, every kDefaultBindings and kDefaultFloatRules entry parsing, clamping, invalid entries being skipped, and the list-stop rules. The YAML defaults reproduce the code defaults exactly. Throwaway files removed from dist.

Concerns:
- Wh_GetIntSetting returns 0 for a missing value, so a missing int becomes 0 (gap 0, workspaces 1). Harmless while Windhawk supplies the YAML defaults, but it cannot be detected.
- Missing floatRules or ignoreRules replace the defaults with an empty list, as the spec says.
- Test executables built with the Windhawk clang do not start here (STATUS_DLL_NOT_FOUND) unless linked with -static. The -Test and -Standalone link lines in build.ps1 omit -static, so they will likely fail the same way. I did not verify that path.
- A prefix-less rule containing ':' (e.g. "proc:foo") is accepted as a process rule that never matches. I can reject that if you want.
````
