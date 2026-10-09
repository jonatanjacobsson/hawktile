# Audit results

Generated 2026-10-09 12:27 UTC with [audit-session.ps1](audit-session.ps1) over the
planner transcript and all 25 subagent transcripts of the development session.
Columns count tool calls that could have fetched or read something; the
planner's own instructions, which name excluded projects in their "do not look
at" rules, are not counted. Agent ids match the records in
[agents-index.md](../agents-index.md).

| Participant | Tool calls | Web tools | Network cmds | Other-WM access | Restricted-mod access | Web targets |
|---|---|---|---|---|---|---|
| planner (Opus 5.5) | 334 | 7 | 17 | 8 | 2 | https://morluto.github.io/rea/<br>search: windhawk "tool mod" windhawk.exe -tool-mod Wh_ModInit dedicated process<br>search: Omarchy hotkeys keybindings manual SUPER arrow move window workspace<br>https://learn.omacom.io/2/the-omarchy-manual/53/hotkeys.md<br>https://github.com/ramensoftware/windhawk/wiki/creating-a-new-mod<br>https://github.com/ramensoftware/windhawk/wiki/Global-objects-and-process-shutdown<br>https://github.com/ramensoftware/windhawk/wiki/creating-a-new-mod |
| a6bdf9130388e37cb | 13 | 0 | 0 | 0 | 0 |  |
| a39ea4b3ac814ed23 | 14 | 0 | 0 | 0 | 0 |  |
| ae25e5156536488dd | 23 | 0 | 0 | 0 | 0 |  |
| aa55080ea2f39229f | 8 | 0 | 0 | 0 | 0 |  |
| a6faf51d8a36a416c | 32 | 0 | 0 | 0 | 0 |  |
| a6b3619db44833f87 | 23 | 0 | 0 | 0 | 0 |  |
| a126de84b4b84f1f9 | 8 | 0 | 0 | 0 | 0 |  |
| a118dd5f46104f07a | 26 | 0 | 0 | 0 | 0 |  |
| a1fea1b6538021bc4 | 15 | 0 | 0 | 0 | 0 |  |
| a24aa4c9d3e740d97 | 9 | 0 | 0 | 0 | 0 |  |
| a230225487e1e90f7 | 16 | 0 | 0 | 0 | 0 |  |
| af93c1d3777ad183b | 21 | 0 | 0 | 0 | 0 |  |
| a55a57df33a4de093 | 28 | 0 | 0 | 0 | 0 |  |
| a88f44d11215fddff | 14 | 0 | 0 | 0 | 0 |  |
| a83ad1470dc3cb6b8 | 9 | 0 | 0 | 0 | 0 |  |
| a9b7b51d4a8e0cf6b | 14 | 0 | 0 | 0 | 0 |  |
| a31e8d6925534c537 | 18 | 0 | 0 | 0 | 0 |  |
| a6e13cd2b967757fd | 17 | 0 | 0 | 0 | 0 |  |
| a892773f36390a269 | 16 | 0 | 0 | 0 | 0 |  |
| ad05488633b380543 | 17 | 0 | 0 | 0 | 0 |  |
| a49a51b9c31e44806 | 17 | 0 | 0 | 0 | 0 |  |
| a80fb20c7c74cd0e8 | 99 | 0 | 12 | 0 | 4 |  |
| a50046250c3127a48 | 38 | 0 | 0 | 0 | 0 |  |
| af5e93597452c1a85 | 14 | 0 | 0 | 0 | 0 |  |
| ae7e7a13fe6e182ad | 11 | 0 | 0 | 0 | 0 |  |

## Notes on the non-zero cells

**Implementers (24 Haiku sessions):** all zero in every column. They only read
and edited files in the repository, read Windhawk's `windhawk_api.h`, and ran
the build and tests.

**Researcher (`a80fb20c7c74cd0e8`, Opus):**
- *Network:* `gh api` downloads of Windhawk mods and wiki pages from
  `ramensoftware/windhawk-mods` / `ramensoftware/windhawk`.
- *Restricted-mod access:* the window-layout mods were streamed through a
  filter that stopped at the end of their `==WindhawkModSettings==` block, so
  only metadata, readme and settings text was stored or read
  (`awk '{print} /\/\/ ==\/WindhawkModSettings==/{exit}'`). The agent then
  checked the extracts with `grep -l "#include"` to make sure no code had come
  through.

**Planner (Opus):**
- *Web:* the REA landing page (not used further), one search about Windhawk
  tool mods, the Omarchy hotkeys page, and Windhawk's wiki pages "Creating a
  new mod" and "Global objects and process shutdown" (to check the mod against
  Windhawk's guide).
- *Network:* downloading the MIT-licensed `command-on-window-focus.wh.cpp` (the
  tool-mod launcher template, see THIRD_PARTY_NOTICES.md); listing file names in
  `windhawk-mods/mods` (to find related mods and check that the mod id is free);
  reading the signed-in GitHub user name for the `@github` field; a local
  `127.0.0.1` server used to preview `docs/hero.svg`; and URLs that appear
  inside text that was written (readme links).
- *Other-WM / restricted-mod access:* every hit is a command that **searched
  for** those names in this project's own files or in the session transcripts
  (the audits that produced this file, and the scans that removed old names
  before the first release). None of them read material from those projects.