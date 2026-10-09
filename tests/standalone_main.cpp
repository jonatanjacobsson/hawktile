
// =============================================================================
// standalone_main.cpp — dev harness. Appended after 60_engine.cpp when building
// with -Standalone (HT_STANDALONE defined). Runs the engine with default
// settings against the real desktop. Press Enter to quit; hidden windows are
// shown again on exit.
// =============================================================================

int main() {
    fwprintf(stderr, L"HawkTile standalone. Press Enter to quit.\n");
    if (!ht::StartEngine()) {
        fwprintf(stderr, L"StartEngine failed\n");
        return 1;
    }
    getchar();
    ht::StopEngine();
    fwprintf(stderr, L"Stopped.\n");
    return 0;
}
