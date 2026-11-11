<#
.SYNOPSIS
  Builds HawkTile and installs it into the local Windhawk, the way Windhawk's
  own editor does when you click "Compile Mod" (Windhawk 1.7.x, installed, not
  portable).

  deploy.ps1                                   build, compile, install, reload
  deploy.ps1 -Bind 'alt+period=resizeactive left'
                                               also add a key binding to the
                                               saved settings if those keys
                                               are not bound yet
  deploy.ps1 -Set 'advanced.debugLog=1'        also change a saved setting

  What it does, following vscode-windhawk's compilerUtils.ts / modConfigUtils.ts:
    1. build.ps1 (fragments -> dist\hawktile.wh.cpp, compile checks)
    2. compiles the 32-bit and 64-bit mod DLLs into dist\deploy
    3. one elevated step (UAC prompt): copies the DLLs to
       <AppData>\Engine\Mods\32 and \64, copies the source to ModsSource, and
       sets Version and LibraryFileName under <RegistryKey>\Engine\Mods\<id>.
       Windhawk watches that key and reloads the mod when LibraryFileName
       changes. Saved settings are left alone (apart from -Bind).
    4. waits for the new tool-mod process

  Settings that are new in this version get their defaults only when the mod is
  compiled in Windhawk's editor; until then they read as 0 / empty.
#>
param(
    [string]$ModId = 'local@hawktile',
    [string[]]$Bind = @(),
    [string[]]$Set = @(),
    # Internal: the elevated half. Not meant to be called by hand.
    [switch]$Install,
    [string]$DllName,
    [string]$Version
)

$ErrorActionPreference = 'Stop'
$root = $PSScriptRoot
$stage = Join-Path $root 'dist\deploy'
$source = Join-Path $root 'dist\hawktile.wh.cpp'
$log = Join-Path $stage 'install.log'

$windhawkDir = 'C:\Program Files\Windhawk'
$ini = Get-Content (Join-Path $windhawkDir 'windhawk.ini')
function Get-IniValue([string]$name) {
    $line = $ini | Where-Object { $_ -match "^$name=" } | Select-Object -First 1
    if (-not $line) { throw "windhawk.ini has no $name" }
    return [Environment]::ExpandEnvironmentVariables($line.Substring($name.Length + 1))
}
if ((Get-IniValue 'Portable') -ne '0') { throw 'Portable Windhawk is not supported by this script' }
$appData = Get-IniValue 'AppDataPath'
$enginePath = Join-Path $windhawkDir (Get-IniValue 'EnginePath')
$compilerDir = Join-Path $windhawkDir (Get-IniValue 'CompilerPath')
$modKey = 'Registry::' + (Get-IniValue 'RegistryKey') + "\Engine\Mods\$ModId"
$modsDir = Join-Path $appData 'Engine\Mods'

# ---- Elevated half ----------------------------------------------------------
if ($Install) {
    try {
        Start-Transcript -Path $log -Force | Out-Null
        if (-not (Test-Path $modKey)) { throw "Mod $ModId is not installed in Windhawk ($modKey)" }
        foreach ($bits in '32', '64') {
            Copy-Item (Join-Path $stage "$bits\$DllName") (Join-Path $modsDir "$bits\$DllName") -Force
        }
        Copy-Item $source (Join-Path $appData "ModsSource\$ModId.wh.cpp") -Force

        # The elevated call gets all bindings as one ';'-joined argument.
        foreach ($pair in @($Bind | ForEach-Object { $_ -split ';' } | Where-Object { $_ })) {
            $keys, $command = $pair -split '=', 2
            $settingsKey = "$modKey\Settings"
            $saved = Get-ItemProperty $settingsKey
            $names = @($saved.PSObject.Properties.Name | Where-Object { $_ -match '^binds\[(\d+)\]\.keys$' })
            if ($names | Where-Object { $saved.$_ -eq $keys }) {
                Write-Host "binding $keys already exists"
                continue
            }
            $next = 0
            if ($names) { $next = 1 + ($names | ForEach-Object { [int]($_ -replace '\D', '') } | Measure-Object -Maximum).Maximum }
            Set-ItemProperty $settingsKey -Name "binds[$next].keys" -Value $keys -Type String
            Set-ItemProperty $settingsKey -Name "binds[$next].command" -Value $command -Type String
            $now = [int]([DateTimeOffset]::UtcNow.ToUnixTimeSeconds() -band 0x7fffffff)
            Set-ItemProperty $modKey -Name 'SettingsChangeTime' -Value $now -Type DWord
            Write-Host "added binding $next`: $keys -> $command"
        }

        foreach ($pair in @($Set | ForEach-Object { $_ -split ';' } | Where-Object { $_ })) {
            $name, $value = $pair -split '=', 2
            $settingsKey = "$modKey\Settings"
            $item = Get-Item $settingsKey
            $kind = if ($item.GetValueNames() -contains $name) { $item.GetValueKind($name) }
                    elseif ($value -match '^-?\d+$') { 'DWord' } else { 'String' }
            Set-ItemProperty $settingsKey -Name $name -Value $value -Type $kind
            $now = [int]([DateTimeOffset]::UtcNow.ToUnixTimeSeconds() -band 0x7fffffff)
            Set-ItemProperty $modKey -Name 'SettingsChangeTime' -Value $now -Type DWord
            Write-Host "setting $name = $value ($kind)"
        }

        # LibraryFileName last: its change is what makes the engine reload.
        Set-ItemProperty $modKey -Name 'Version' -Value $Version -Type String
        Set-ItemProperty $modKey -Name 'LibraryFileName' -Value $DllName -Type String
        Write-Host "registered $DllName"

        # Old builds; the one still loaded cannot be deleted yet and is left
        # for the next run.
        Start-Sleep -Seconds 3
        foreach ($bits in '32', '64') {
            Get-ChildItem (Join-Path $modsDir $bits) -Filter "${ModId}_*.dll" |
                Where-Object { $_.Name -ne $DllName -and $_.BaseName -match '_\d+$' } |
                ForEach-Object { try { Remove-Item $_.FullName -Force } catch { Write-Host "kept $($_.Name) (in use)" } }
        }
        Write-Host 'INSTALL OK'
    } catch {
        Write-Host "INSTALL FAILED: $_"
        exit 1
    } finally {
        Stop-Transcript | Out-Null
    }
    exit 0
}

# ---- Normal half ------------------------------------------------------------
$shell = (Get-Process -Id $PID).Path
& $shell -NoProfile -File (Join-Path $root 'build.ps1')
if ($LASTEXITCODE -ne 0) { throw 'build.ps1 failed' }

$header = Get-Content $source -TotalCount 40
$Version = ($header | Where-Object { $_ -match '^// @version\s+(\S+)' } | Select-Object -First 1) -replace '^// @version\s+', ''
$options = ($header | Where-Object { $_ -match '^// @compilerOptions\s+' } | Select-Object -First 1) -replace '^// @compilerOptions\s+', ''
if (-not $Version) { throw 'No @version in the mod header' }

do {
    $DllName = '{0}_{1}_{2}.dll' -f $ModId, $Version, (Get-Random -Minimum 100000 -Maximum 1000000)
} while ((Test-Path (Join-Path $modsDir "32\$DllName")) -or (Test-Path (Join-Path $modsDir "64\$DllName")))

Remove-Item $stage -Recurse -Force -ErrorAction SilentlyContinue
$cc = Join-Path $compilerDir 'bin\clang++.exe'
function Quote([string]$text) { '"' + ($text -replace '\\', '/' -replace '"', '\"') + '"' }
$targets = @(@{ Bits = '32'; Triple = 'i686-w64-mingw32' }, @{ Bits = '64'; Triple = 'x86_64-w64-mingw32' })
foreach ($target in $targets) {
    $outDir = Join-Path $stage $target.Bits
    New-Item -ItemType Directory -Force $outDir | Out-Null
    # Same arguments, in the same order, as Windhawk's editor.
    $arguments = @(
        '-std=c++23', '-O2', '-shared', '-DUNICODE', '-D_UNICODE',
        '-DWINVER=0x0A00', '-D_WIN32_WINNT=0x0A00', '-D_WIN32_IE=0x0A00',
        '-DNTDDI_VERSION=0x0A000008', '-D__USE_MINGW_ANSI_STDIO=0', '-DWH_MOD',
        (Quote "-DWH_MOD_ID=L`"$ModId`""), (Quote "-DWH_MOD_VERSION=L`"$Version`""),
        (Quote (Join-Path $enginePath "$($target.Bits)\windhawk.lib")),
        '-x', 'c++', (Quote $source),
        '-I', (Quote (Join-Path $compilerDir 'include')), '-include', 'windhawk_api.h',
        '-target', $target.Triple, '-Wl,--export-all-symbols',
        '-o', (Quote (Join-Path $outDir $DllName)), $options
    )
    $rsp = Join-Path $outDir 'compile.rsp'
    [System.IO.File]::WriteAllText($rsp, ($arguments -join ' '))
    Push-Location $compilerDir
    try { & $cc "@$rsp" 2>&1 | ForEach-Object { Write-Host "$_" } } finally { Pop-Location }
    if ($LASTEXITCODE -ne 0 -or -not (Test-Path (Join-Path $outDir $DllName))) {
        throw "Compiling the $($target.Bits)-bit mod failed"
    }
    Write-Host "compiled $($target.Bits)-bit $DllName"
}

$started = Get-Date
$installArgs = @('-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', (Quote $PSCommandPath), '-Install',
    '-ModId', $ModId, '-DllName', $DllName, '-Version', $Version)
if ($Bind) { $installArgs += '-Bind'; $installArgs += Quote ($Bind -join ';') }
if ($Set) { $installArgs += '-Set'; $installArgs += Quote ($Set -join ';') }
Write-Host 'Installing (approve the Windows elevation prompt)...'
$process = Start-Process $shell -ArgumentList $installArgs -Verb RunAs -WindowStyle Hidden -Wait -PassThru
if (Test-Path $log) { Get-Content $log | Where-Object { $_ -match '^(binding|added binding|setting|registered|kept|INSTALL)' } | ForEach-Object { Write-Host $_ } }
if ($process.ExitCode -ne 0) { throw "The elevated install step failed (exit $($process.ExitCode)), see $log" }

# The tray process loads the new DLL and starts a fresh tool-mod process.
$fresh = $null
for ($i = 0; $i -lt 40 -and -not $fresh; $i++) {
    Start-Sleep -Milliseconds 500
    $fresh = Get-CimInstance Win32_Process -Filter "name='windhawk.exe'" |
        Where-Object { $_.CommandLine -match '-tool-mod' -and $_.CommandLine -match [regex]::Escape($ModId) -and $_.CreationDate -gt $started }
}
if ($fresh) {
    Write-Host "HawkTile $Version is running (process $($fresh.ProcessId), $DllName)"
} else {
    Write-Host "Installed $DllName, but no new tool-mod process appeared within 20 s. Check that Windhawk is running and the mod is enabled."
    exit 2
}
