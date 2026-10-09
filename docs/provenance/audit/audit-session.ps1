<#
.SYNOPSIS
  Audits a Claude Code session transcript (planner + subagents) for what each
  participant could have seen: web access, network commands, references to
  other window managers, and reads of the window-layout Windhawk mods that
  were restricted to "readme only".

.EXAMPLE
  pwsh -File audit-session.ps1 -SessionDir "$env:USERPROFILE\.claude\projects\<project>\<session-id>"

  The session folder contains a "subagents" subfolder; the planner transcript
  is the .jsonl file with the same name next to the folder. Requires
  PowerShell 7 (ConvertFrom-Json -Depth).
#>
param([Parameter(Mandatory)] [string]$SessionDir)

$ErrorActionPreference = 'Stop'
$otherWms = '(?i)komorebi|lgug2z|glazewm|bspwm|\bi3wm\b|hyprland/hyprland|github\.com/hyprwm'
$restrictedMods = 'hyprland-windows|tiling-helper|gnome-dynamic-desktops|slick-window-arrangement|two-sided-snapping|snap-commander'
$network = '(?i)\bgh api\b|\bcurl\b|Invoke-WebRequest|\biwr\b|\bwget\b|https?://'

function Get-ToolCalls([string]$path) {
    foreach ($line in [IO.File]::ReadLines($path)) {
        if ($line -notmatch '"tool_use"') { continue }
        try { $o = $line | ConvertFrom-Json -Depth 64 } catch { continue }
        foreach ($c in @($o.message.content)) {
            if ($c.type -eq 'tool_use') {
                [pscustomobject]@{ Name = $c.name; Input = ($c.input | ConvertTo-Json -Depth 32 -Compress) }
            }
        }
    }
}

function Summarize([string]$label, [string]$path, [bool]$isPlanner) {
    $calls = @(Get-ToolCalls $path)
    # The planner's own instructions name excluded projects in "do not look at"
    # rules; only count calls that could have fetched or read something.
    $readers = @($calls | Where-Object { $_.Name -notin 'Agent', 'SendMessage', 'Write', 'Edit', 'SubagentHandback' })
    [pscustomobject]@{
        Participant   = $label
        ToolCalls     = $calls.Count
        WebTools      = @($calls | Where-Object { $_.Name -in 'WebFetch', 'WebSearch' }).Count
        NetworkCmds   = @($readers | Where-Object { $_.Input -match $network }).Count
        OtherWmAccess = @($readers | Where-Object { $_.Input -match $otherWms }).Count
        RestrictedMod = @($readers | Where-Object { $_.Input -match $restrictedMods }).Count
        WebTargets    = (@($calls | Where-Object { $_.Name -in 'WebFetch', 'WebSearch' } | ForEach-Object {
                            $i = $_.Input | ConvertFrom-Json; if ($i.url) { $i.url } else { "search: $($i.query)" } }) -join '<br>')
    }
}

$rows = @()
$planner = Get-ChildItem (Split-Path $SessionDir) -Filter ((Split-Path $SessionDir -Leaf) + '.jsonl')
if ($planner) { $rows += Summarize 'planner (Opus 5.5)' $planner.FullName $true }
foreach ($f in Get-ChildItem (Join-Path $SessionDir 'subagents') -Filter 'agent-*.jsonl' | Sort-Object LastWriteTime) {
    $rows += Summarize ($f.BaseName -replace '^agent-', '') $f.FullName $false
}

'| Participant | Tool calls | Web tools | Network cmds | Other-WM access | Restricted-mod access | Web targets |'
'|---|---|---|---|---|---|---|'
foreach ($r in $rows) {
    "| $($r.Participant) | $($r.ToolCalls) | $($r.WebTools) | $($r.NetworkCmds) | $($r.OtherWmAccess) | $($r.RestrictedMod) | $($r.WebTargets) |"
}
