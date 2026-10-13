# Clean-room statement

HawkTile is an independent implementation of a tiling window manager. This page
explains how it was written, what was and wasn't looked at, and where the limits
of that claim are, so you can judge it for yourself. Everything here can be
checked against the records in [docs/provenance](docs/provenance/README.md).

*This is a factual record, not legal advice.*

## In short

- HawkTile was written from scratch in C++ for Windhawk, from Microsoft's
  Win32/DWM APIs, Windhawk's documentation and templates, and the project
  owner's requirements.
- The source code, binaries, documentation, configuration format and command
  line of **komorebi** were never opened while building it. The project owner
  mentioned komorebi as the kind of tool they wanted; its repository link was
  never followed. The same applies to the code of every other tiling window
  manager.
- An audit of every action taken during development supports this: the agents
  that wrote the code made **no network access at all** and never touched another
  window manager's material ([results](docs/provenance/audit/results.md)).

## How it was built

| Role | Who | What they did |
|---|---|---|
| Owner | Jonatan Jacobsson | Requirements, live testing, decisions |
| Planner | Claude Opus 5.5 | Architecture, the shared contract (`src/10_contract.h`), a precise brief per task, review and small integration fixes |
| Implementers | Claude Haiku 5.5, 26 separate sessions | Each wrote or changed one or a few source fragments against the contract. Every brief told them not to look at, search for or reproduce code from other window managers |
| Researcher | Claude Opus 5.5, one session | Read other Windhawk mods to learn Windhawk engineering practices. Mods that manage window layout were read as **readme and settings only** |

Every brief, follow-up and report is preserved verbatim in
[docs/provenance/agents](docs/provenance/agents-index.md).

## What was used

| Source | Used for |
|---|---|
| Microsoft Win32 and DWM behaviour (public API documentation) | All window management |
| Windhawk's `windhawk_api.h` and the [Windhawk wiki](https://github.com/ramensoftware/windhawk/wiki) | The mod API, settings and tool-mod model |
| The Windhawk tool-mod launcher snippet, via the MIT-licensed `command-on-window-focus.wh.cpp` | `src/90_toolmod.cpp` (see [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md)) |
| The settings block of an installed Windhawk mod (Windows 11 Taskbar Styler) | Confirming Windhawk's YAML syntax for lists of objects |
| [Omarchy's hotkeys page](https://learn.omacom.io/2/the-omarchy-manual/53/hotkeys) | The default key assignments, with Alt in place of Super |
| Hyprland's documented dispatcher names | The command vocabulary (`movefocus`, `swapwindow`, `movetoworkspace`, …). No Hyprland code |
| About 35 open-source Windhawk mods | Windhawk engineering practices, summarised in [docs/research-windhawk-mods.md](docs/research-windhawk-mods.md). No code copied |
| Ten Windhawk mods that arrange windows | Readme and settings text only, fetched through a filter that stopped before any code |

General ideas that are common to the whole field (binary-split "dwindle"
layouts, master-stack, monocle, per-monitor workspaces, gaps) were used as
ideas, not taken from any one implementation.

## Known limits of the claim

1. **Model training data.** A classic clean room requires that the people who
   write the code have never seen the original. The language models used here
   were trained on large amounts of public code, which very likely includes
   komorebi's public repository. That exposure can't be ruled out. What *can* be
   shown is that no such material was consulted while HawkTile was written, that
   the code is in a different language (C++ instead of Rust) with a different
   architecture (an in-process Windhawk tool mod instead of a daemon with a
   command-line client and a separate hotkey daemon), and that it was specified
   independently.
2. **General knowledge of other tools' behaviour.** The planner knew, from
   general knowledge, how other tiling window managers behave from a user's point
   of view. During development some command names echoed that vocabulary. Before
   the first public release (0.8.0) every command was renamed to Hyprland's
   documented dispatcher names. The earlier names remain visible in the verbatim
   provenance records, on purpose.
3. **Model-written code.** The code was written by AI models under human
   direction. Review it like any other contribution.

## Audit

[`docs/provenance/audit/audit-session.ps1`](docs/provenance/audit/audit-session.ps1)
reads the session transcripts and counts, for every participant, the web
lookups, network commands, references to other window managers and reads of the
restricted mods. [The results](docs/provenance/audit/results.md) show:

- **Implementers (26 sessions, across two development sessions):** zero in
  every column.
- **Researcher:** GitHub API reads of Windhawk mods and wiki pages only; the
  window-arranging mods were read through the readme-only filter.
- **Planner:** web lookups of the REA tool's landing page (not used), a
  Windhawk search, two Omarchy lookups and two Windhawk wiki guides; one
  download of the MIT template mod. The other flagged calls were searches *for*
  excluded names in this project's own files, made while auditing and cleaning
  up.
