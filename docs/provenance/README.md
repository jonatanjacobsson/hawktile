# Provenance

This folder is the paper trail of how HawkTile was written. It is a historical
record: nothing here is edited after the fact except for replacing local paths
with `<repo>`, `<scratch>` and `%USERPROFILE%`.

| What | Where |
|---|---|
| Every instruction the planner gave to an implementing or research agent, every follow-up, and every report that came back, verbatim and in order | [agents-index.md](agents-index.md) → [agents/](agents/) |
| The script that audits the session transcripts, and its results | [audit/](audit/) |
| The shared contract every implementer coded against | [`src/10_contract.h`](../../src/10_contract.h) (current version) |

Because these are verbatim records, they contain things the released code no
longer does: the early command names that were replaced before the first public
release, settings keys that were renamed, and the names of the projects the
agents were told **not** to consult (they appear in the "clean room" rule at the
top of each instruction).

See [CLEANROOM.md](../../CLEANROOM.md) for what the record shows.

## Regenerating

The records were extracted from the Claude Code session transcripts
(`%USERPROFILE%\.claude\projects\<project>\<session>.jsonl` and its
`subagents\` folder). The audit can be re-run with:

```powershell
pwsh -File docs/provenance/audit/audit-session.ps1 -SessionDir "<path to the session folder>"
```
