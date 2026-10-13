// =============================================================================
// 40_config.cpp — settings loading, hotkey / command / rule parsing and hotkey
// registration.
// =============================================================================

namespace ht {

namespace {

struct KeyName {
    const wchar_t* name;
    UINT vk;
};

constexpr KeyName kKeyNames[] = {
    {L"left", VK_LEFT},
    {L"right", VK_RIGHT},
    {L"up", VK_UP},
    {L"down", VK_DOWN},
    {L"enter", VK_RETURN},
    {L"return", VK_RETURN},
    {L"space", VK_SPACE},
    {L"tab", VK_TAB},
    {L"escape", VK_ESCAPE},
    {L"esc", VK_ESCAPE},
    {L"backspace", VK_BACK},
    {L"delete", VK_DELETE},
    {L"del", VK_DELETE},
    {L"insert", VK_INSERT},
    {L"ins", VK_INSERT},
    {L"home", VK_HOME},
    {L"end", VK_END},
    {L"pageup", VK_PRIOR},
    {L"pgup", VK_PRIOR},
    {L"pagedown", VK_NEXT},
    {L"pgdn", VK_NEXT},
    {L"minus", VK_OEM_MINUS},
    {L"plus", VK_OEM_PLUS},
    {L"equals", VK_OEM_PLUS},
    {L"comma", VK_OEM_COMMA},
    {L"period", VK_OEM_PERIOD},
    {L"semicolon", VK_OEM_1},
    {L"slash", VK_OEM_2},
    {L"backtick", VK_OEM_3},
    {L"grave", VK_OEM_3},
    {L"bracketleft", VK_OEM_4},
    {L"backslash", VK_OEM_5},
    {L"bracketright", VK_OEM_6},
    {L"quote", VK_OEM_7},
};

struct NoArgCommand {
    const wchar_t* name;
    CommandKind kind;
};

constexpr NoArgCommand kNoArgCommands[] = {
    {L"nextlayout", CommandKind::NextLayout},
    {L"togglefloating", CommandKind::ToggleFloating},
    {L"togglemonocle", CommandKind::ToggleMonocle},
    {L"fullscreen", CommandKind::Fullscreen},
    {L"swapwithmaster", CommandKind::SwapWithMaster},
    {L"growmaster", CommandKind::GrowMaster},
    {L"shrinkmaster", CommandKind::ShrinkMaster},
    {L"rescan", CommandKind::Rescan},
    {L"toggletiling", CommandKind::ToggleTiling},
    {L"closewindow", CommandKind::CloseWindow},
    {L"togglesplit", CommandKind::ToggleSplit},
};

struct RulePrefix {
    const wchar_t* text;
    RuleField field;
};

constexpr RulePrefix kRulePrefixes[] = {
    {L"process:", RuleField::Process},
    {L"class:", RuleField::Class},
    {L"title:", RuleField::Title},
};

bool IsSpace(wchar_t c) {
    return std::iswspace(c) != 0;
}

std::wstring ToLower(const std::wstring& s) {
    std::wstring out = s;
    for (wchar_t& c : out) {
        c = static_cast<wchar_t>(std::towlower(c));
    }
    return out;
}

std::wstring Trim(const std::wstring& s) {
    size_t begin = 0;
    size_t end = s.size();
    while (begin < end && IsSpace(s[begin])) {
        ++begin;
    }
    while (end > begin && IsSpace(s[end - 1])) {
        --end;
    }
    return s.substr(begin, end - begin);
}

// Keeps empty fields, so "" yields one empty part.
std::vector<std::wstring> SplitChar(const std::wstring& s, wchar_t sep) {
    std::vector<std::wstring> parts;
    size_t start = 0;
    for (;;) {
        const size_t pos = s.find(sep, start);
        if (pos == std::wstring::npos) {
            parts.push_back(s.substr(start));
            return parts;
        }
        parts.push_back(s.substr(start, pos - start));
        start = pos + 1;
    }
}

// Splits on runs of whitespace; leading, trailing and repeated spaces vanish.
std::vector<std::wstring> SplitWords(const std::wstring& s) {
    std::vector<std::wstring> words;
    size_t i = 0;
    while (i < s.size()) {
        while (i < s.size() && IsSpace(s[i])) {
            ++i;
        }
        const size_t start = i;
        while (i < s.size() && !IsSpace(s[i])) {
            ++i;
        }
        if (i > start) {
            words.push_back(s.substr(start, i - start));
        }
    }
    return words;
}

bool ParseDecimal(const std::wstring& s, int* out) {
    if (s.empty() || s.size() > 3) {
        return false;
    }
    int value = 0;
    for (wchar_t c : s) {
        if (c < L'0' || c > L'9') {
            return false;
        }
        value = value * 10 + (c - L'0');
    }
    *out = value;
    return true;
}

bool ParseHex(const std::wstring& s, unsigned* out) {
    if (s.empty() || s.size() > 8) {
        return false;
    }
    unsigned value = 0;
    for (wchar_t c : s) {
        unsigned digit = 0;
        if (c >= L'0' && c <= L'9') {
            digit = c - L'0';
        } else if (c >= L'a' && c <= L'f') {
            digit = c - L'a' + 10;
        } else if (c >= L'A' && c <= L'F') {
            digit = c - L'A' + 10;
        } else {
            return false;
        }
        value = value * 16 + digit;
    }
    *out = value;
    return true;
}

// `name` is already lowercase.
bool ResolveKey(const std::wstring& name, UINT* vk) {
    if (name.size() == 1) {
        const wchar_t c = name[0];
        if (c >= L'a' && c <= L'z') {
            *vk = static_cast<UINT>(L'A' + (c - L'a'));
            return true;
        }
        if (c >= L'0' && c <= L'9') {
            *vk = static_cast<UINT>(c);
            return true;
        }
        return false;
    }
    if (name.size() >= 2 && name[0] == L'f') {
        int n = 0;
        if (ParseDecimal(name.substr(1), &n) && n >= 1 && n <= 24) {
            *vk = static_cast<UINT>(VK_F1 + n - 1);
            return true;
        }
    }
    if (name.size() > 2 && name.compare(0, 2, L"vk") == 0) {
        unsigned code = 0;
        if (ParseHex(name.substr(2), &code) && code >= 1 && code <= 254) {
            *vk = code;
            return true;
        }
        return false;
    }
    for (const KeyName& key : kKeyNames) {
        if (name == key.name) {
            *vk = key.vk;
            return true;
        }
    }
    return false;
}

bool ParseDirection(const std::wstring& word, Direction* out) {
    if (word == L"left" || word == L"l") {
        *out = Direction::Left;
    } else if (word == L"right" || word == L"r") {
        *out = Direction::Right;
    } else if (word == L"up" || word == L"u") {
        *out = Direction::Up;
    } else if (word == L"down" || word == L"d") {
        *out = Direction::Down;
    } else {
        return false;
    }
    return true;
}

bool ParseWorkspaceNumber(const std::wstring& word, int* out) {
    int n = 0;
    if (!ParseDecimal(word, &n) || n < 1 || n > 9) {
        return false;
    }
    *out = n;
    return true;
}

bool MakeBinding(const std::wstring& keys, const std::wstring& command, Binding* out) {
    Binding binding;
    if (!ParseHotkey(keys.c_str(), &binding.modifiers, &binding.vk) ||
        !ParseCommand(command.c_str(), &binding.cmd)) {
        return false;
    }
    binding.keysText = keys;
    binding.commandText = command;
    *out = binding;
    return true;
}

#ifndef HT_STANDALONE

constexpr int kMaxListEntries = 256;

// Wh_GetStringSetting may hand back null for a missing value. A non-null result
// is copied and released.
template <typename... Args>
std::wstring SettingString(PCWSTR name, Args... args) {
    const PCWSTR value = Wh_GetStringSetting(name, args...);
    std::wstring out = value ? value : L"";
    if (value) {
        Wh_FreeStringSetting(value);
    }
    return out;
}

bool ParseColor(const std::wstring& text, COLORREF* out) {
    std::wstring hex = Trim(text);
    if (!hex.empty() && hex[0] == L'#') {
        hex.erase(0, 1);
    }
    unsigned value = 0;
    if (hex.size() != 6 || !ParseHex(hex, &value)) {
        return false;
    }
    *out = RGB((value >> 16) & 0xFF, (value >> 8) & 0xFF, value & 0xFF);
    return true;
}

// Reads name[0], name[1], ... until an empty entry or kMaxListEntries.
std::vector<MatchRule> ReadRules(const wchar_t* format, const wchar_t* label) {
    std::vector<MatchRule> rules;
    for (int i = 0; i < kMaxListEntries; ++i) {
        const std::wstring text = SettingString(format, i);
        if (text.empty()) {
            break;
        }
        MatchRule rule;
        if (ParseRule(text.c_str(), &rule)) {
            rules.push_back(rule);
        } else {
            HT_LOG(L"Skipping invalid %s rule '%s'", label, text.c_str());
        }
    }
    return rules;
}

void ReadBindings(Settings& s) {
    std::vector<Binding> bindings;
    for (int i = 0; i < kMaxListEntries; ++i) {
        const std::wstring keys = SettingString(L"binds[%d].keys", i);
        const std::wstring command = SettingString(L"binds[%d].command", i);
        if (keys.empty() && command.empty()) {
            break;
        }
        Binding binding;
        if (MakeBinding(keys, command, &binding)) {
            bindings.push_back(binding);
        } else {
            HT_LOG(L"Skipping invalid binding %d: keys '%s', command '%s'", i,
                   keys.c_str(), command.c_str());
        }
    }
    if (bindings.empty()) {
        HT_LOG(L"No valid key bindings in settings, keeping the defaults");
        return;
    }
    s.bindings = std::move(bindings);
}

void ReadWindhawkSettings(Settings& s) {
    // Read first: LoadSettings passes g.settings, and LogLine checks debugLog,
    // so the rest of this load is written to the file when it is on.
    s.debugLog = Wh_GetIntSetting(L"advanced.debugLog") != 0;
    s.outerGap = std::clamp(Wh_GetIntSetting(L"layout.outerGap"), 0, 200);
    s.innerGap = std::clamp(Wh_GetIntSetting(L"layout.innerGap"), 0, 200);
    s.workspaceCount = std::clamp(Wh_GetIntSetting(L"layout.workspaces"), 1, 9);

    const std::wstring layoutName = SettingString(L"layout.default");
    LayoutKind layout = LayoutKind::Dwindle;
    if (ParseLayoutName(layoutName.c_str(), &layout)) {
        s.defaultLayout = layout;
    } else {
        HT_LOG(L"Invalid defaultLayout '%s', keeping %s", layoutName.c_str(),
               LayoutName(s.defaultLayout));
    }

    s.masterRatio = std::clamp(Wh_GetIntSetting(L"layout.masterRatio"), 15, 85) / 100.0;
    s.ratioStep = std::clamp(Wh_GetIntSetting(L"layout.resizeStep"), 1, 25) / 100.0;

    s.focusBorder = Wh_GetIntSetting(L"border.enabled") != 0;
    const std::wstring colorText = SettingString(L"border.color");
    COLORREF color = 0;
    if (ParseColor(colorText, &color)) {
        s.focusBorderColor = color;
    } else {
        HT_LOG(L"Invalid focusBorderColor '%s', keeping the default",
               colorText.c_str());
    }
    s.floatNonResizable = Wh_GetIntSetting(L"windows.floatNonResizable") != 0;
    s.floatTopmost = Wh_GetIntSetting(L"windows.floatTopmost") != 0;

    s.tileMaximized = Wh_GetIntSetting(L"windows.tileMaximized") != 0;
    s.workspaceOsd = Wh_GetIntSetting(L"indicators.popup") != 0;
    const int osdDuration = Wh_GetIntSetting(L"indicators.popupDuration");
    s.osdDurationMs =
        std::clamp(osdDuration <= 0 ? 2000 : osdDuration, 100, 10000);
    s.holdOverview = Wh_GetIntSetting(L"indicators.holdAlt") != 0;
    const int holdDelay = Wh_GetIntSetting(L"indicators.holdDelay");
    s.holdDelayMs = std::clamp(holdDelay <= 0 ? 400 : holdDelay, 100, 2000);
    s.taskbarIndicator = Wh_GetIntSetting(L"indicators.taskbar") != 0;
    s.attentionDots = Wh_GetIntSetting(L"indicators.notificationDots") != 0;
    s.followActivation = Wh_GetIntSetting(L"windows.followActivation") != 0;
    s.taskbarIndicatorOffset =
        std::clamp(Wh_GetIntSetting(L"indicators.taskbarOffset"), 0, 2000);

    s.floatRules = ReadRules(L"rules.float[%d]", L"float");
    s.ignoreRules = ReadRules(L"rules.ignore[%d]", L"ignore");
    ReadBindings(s);
}

#endif  // HT_STANDALONE

}  // namespace

bool ParseHotkey(const wchar_t* text, UINT* modifiers, UINT* vk) {
    if (!text || !modifiers || !vk) {
        return false;
    }
    UINT mods = 0;
    UINT key = 0;
    int keyCount = 0;
    for (const std::wstring& raw : SplitChar(ToLower(text), L'+')) {
        const std::wstring token = Trim(raw);
        if (token.empty()) {
            return false;
        }
        if (token == L"alt") {
            mods |= MOD_ALT;
        } else if (token == L"ctrl" || token == L"control") {
            mods |= MOD_CONTROL;
        } else if (token == L"shift") {
            mods |= MOD_SHIFT;
        } else if (token == L"win" || token == L"super") {
            mods |= MOD_WIN;
        } else {
            if (!ResolveKey(token, &key)) {
                return false;
            }
            ++keyCount;
        }
    }
    if (keyCount != 1) {
        return false;
    }
    *modifiers = mods;
    *vk = key;
    return true;
}

bool ParseCommand(const wchar_t* text, Command* out) {
    if (!text || !out) {
        return false;
    }
    const std::vector<std::wstring> words = SplitWords(ToLower(text));
    if (words.empty()) {
        return false;
    }
    const std::wstring& verb = words[0];
    const size_t argCount = words.size() - 1;
    Command cmd;
    if (verb == L"movefocus" || verb == L"swapwindow" ||
        verb == L"resizeactive") {
        if (argCount != 1 || !ParseDirection(words[1], &cmd.dir)) {
            return false;
        }
        if (verb == L"movefocus") {
            cmd.kind = CommandKind::MoveFocus;
        } else if (verb == L"swapwindow") {
            cmd.kind = CommandKind::SwapWindow;
        } else {
            cmd.kind = CommandKind::ResizeActive;
        }
    } else if (verb == L"workspace") {
        if (argCount != 1) {
            return false;
        }
        if (words[1] == L"next") {
            cmd.kind = CommandKind::WorkspaceNext;
        } else if (words[1] == L"prev") {
            cmd.kind = CommandKind::WorkspacePrev;
        } else if (ParseWorkspaceNumber(words[1], &cmd.arg)) {
            cmd.kind = CommandKind::Workspace;
        } else {
            return false;
        }
    } else if (verb == L"movetoworkspace" || verb == L"movetoworkspacesilent") {
        if (argCount != 1 || !ParseWorkspaceNumber(words[1], &cmd.arg)) {
            return false;
        }
        if (verb == L"movetoworkspace") {
            cmd.kind = CommandKind::MoveToWorkspace;
        } else {
            cmd.kind = CommandKind::MoveToWorkspaceSilent;
        }
    } else if (verb == L"focusmonitor" || verb == L"movetomonitor") {
        if (argCount != 1 || words[1] != L"next") {
            return false;
        }
        if (verb == L"focusmonitor") {
            cmd.kind = CommandKind::FocusMonitorNext;
        } else {
            cmd.kind = CommandKind::MoveToMonitorNext;
        }
    } else if (verb == L"layout") {
        if (argCount != 1 || !ParseLayoutName(words[1].c_str(), &cmd.layout)) {
            return false;
        }
        cmd.kind = CommandKind::SetLayout;
    } else {
        bool found = false;
        for (const NoArgCommand& entry : kNoArgCommands) {
            if (verb == entry.name) {
                found = true;
                cmd.kind = entry.kind;
                break;
            }
        }
        if (!found || argCount != 0) {
            return false;
        }
    }
    *out = cmd;
    return true;
}

bool ParseRule(const wchar_t* text, MatchRule* out) {
    if (!text || !out) {
        return false;
    }
    const std::wstring full = Trim(text);
    const std::wstring lower = ToLower(full);
    RuleField field = RuleField::Process;
    size_t prefixLength = 0;
    for (const RulePrefix& prefix : kRulePrefixes) {
        const size_t length = std::wcslen(prefix.text);
        if (lower.compare(0, length, prefix.text) == 0) {
            field = prefix.field;
            prefixLength = length;
            break;
        }
    }
    const std::wstring pattern = ToLower(Trim(full.substr(prefixLength)));
    if (pattern.empty()) {
        return false;
    }
    out->field = field;
    out->pattern = pattern;
    return true;
}

bool RuleMatches(const MatchRule& rule, const WindowFacts& facts) {
    if (rule.pattern.empty()) {
        return false;
    }
    const std::wstring pattern = ToLower(rule.pattern);
    if (rule.field == RuleField::Process) {
        return ToLower(facts.processName) == pattern;
    }
    if (rule.field == RuleField::Class) {
        return ToLower(facts.className) == pattern;
    }
    return ToLower(facts.title).find(pattern) != std::wstring::npos;
}

bool AnyRuleMatches(const std::vector<MatchRule>& rules, const WindowFacts& facts) {
    for (const MatchRule& rule : rules) {
        if (RuleMatches(rule, facts)) {
            return true;
        }
    }
    return false;
}

int RegisterBindings(HWND hwnd, const std::vector<Binding>& bindings) {
    int registered = 0;
    for (size_t i = 0; i < bindings.size(); ++i) {
        const Binding& binding = bindings[i];
        const int id = kHotkeyIdBase + static_cast<int>(i);
        if (RegisterHotKey(hwnd, id, binding.modifiers | MOD_NOREPEAT, binding.vk)) {
            ++registered;
        } else {
            HT_LOG(L"RegisterHotKey failed for '%s' (error %lu)",
                   binding.keysText.c_str(), GetLastError());
        }
    }
    return registered;
}

void UnregisterBindings(HWND hwnd, const std::vector<Binding>& bindings) {
    for (size_t i = 0; i < bindings.size(); ++i) {
        UnregisterHotKey(hwnd, kHotkeyIdBase + static_cast<int>(i));
    }
}

void LoadSettings(Settings& s) {
    s = Settings{};

    for (const wchar_t* text : kDefaultFloatRules) {
        MatchRule rule;
        if (ParseRule(text, &rule)) {
            s.floatRules.push_back(rule);
        } else {
            HT_LOG(L"Invalid default float rule '%s'", text);
        }
    }
    for (const DefaultBinding& def : kDefaultBindings) {
        Binding binding;
        if (MakeBinding(def.keys, def.command, &binding)) {
            s.bindings.push_back(binding);
        } else {
            HT_LOG(L"Invalid default binding '%s' -> '%s'", def.keys, def.command);
        }
    }

#ifndef HT_STANDALONE
    ReadWindhawkSettings(s);
#endif

    HT_LOG(L"Settings: gaps outer=%d inner=%d, workspaces=%d, layout=%s, "
           L"%d float rules, %d ignore rules, %d bindings, "
           L"tileMaximized=%d, workspaceOsd=%d, indicatorDuration=%d ms, "
           L"taskbarIndicator=%d, taskbarIndicatorOffset=%d px, "
           L"attentionDots=%d, followActivation=%d, holdOverview=%d, "
           L"holdDelayMs=%d, debugLog=%d",
           s.outerGap, s.innerGap, s.workspaceCount, LayoutName(s.defaultLayout),
           static_cast<int>(s.floatRules.size()),
           static_cast<int>(s.ignoreRules.size()),
           static_cast<int>(s.bindings.size()),
           static_cast<int>(s.tileMaximized), static_cast<int>(s.workspaceOsd),
           s.osdDurationMs, static_cast<int>(s.taskbarIndicator),
           s.taskbarIndicatorOffset, static_cast<int>(s.attentionDots),
           static_cast<int>(s.followActivation), static_cast<int>(s.holdOverview),
           s.holdDelayMs, static_cast<int>(s.debugLog));
}

}  // namespace ht
