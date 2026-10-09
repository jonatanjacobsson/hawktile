<#
.SYNOPSIS
  Builds HawkTile with Windhawk's bundled clang.

  build.ps1                      concat fragments -> dist\hawktile.wh.cpp, compile-check as DLL
  build.ps1 -Test                build + run tests\layout_test.cpp
  build.ps1 -Standalone          build dist\hawktile-dev.exe (runs engine on the real desktop)
  build.ps1 -Check 30_win32.cpp  syntax-check one fragment against the contract only
#>
param(
    [switch]$Test,
    [switch]$Standalone,
    [string]$Check
)

$ErrorActionPreference = 'Stop'
$root = $PSScriptRoot
$src = Join-Path $root 'src'
$dist = Join-Path $root 'dist'
New-Item -ItemType Directory -Force $dist | Out-Null

$compilerDir = 'C:\Program Files\Windhawk\Compiler'
$cc = Join-Path $compilerDir 'bin\clang++.exe'
if (-not (Test-Path $cc)) { throw "Windhawk compiler not found at $cc" }

$flags = @(
    '-x', 'c++', '-std=c++23', '-target', 'x86_64-w64-mingw32',
    '-DUNICODE', '-D_UNICODE', '-DWINVER=0x0A00', '-D_WIN32_WINNT=0x0A00',
    '-D_WIN32_IE=0x0A00', '-DNTDDI_VERSION=0x0A000008',
    '-D__USE_MINGW_ANSI_STDIO=0', '-DWH_MOD', '-DWH_EDITING',
    '-I', (Join-Path $compilerDir 'include'),
    '-include', 'windhawk_api.h',
    '-Wall', '-Wextra', '-Wno-unused-parameter',
    '-Wno-missing-field-initializers', '-Wno-cast-function-type-mismatch'
)
$libs = @('-ldwmapi', '-lshell32', '-luser32', '-lshcore', '-lgdi32', '-ladvapi32', '-lgdiplus')

$modFragments = @(
    '00_header.wh.cpp', '10_contract.h', '20_layout.cpp', '30_win32.cpp',
    '40_config.cpp', '50_model.cpp', '55_osd.cpp', '56_bar.cpp',
    '60_engine.cpp',
    '90_toolmod.cpp'
)

function Join-Fragments([string[]]$paths, [string]$out) {
    $parts = foreach ($p in $paths) {
        if (-not (Test-Path $p)) { throw "Missing fragment: $p" }
        Get-Content -Raw -Encoding UTF8 $p
    }
    $text = ($parts -join "`n") -replace "`r`n", "`n"
    [System.IO.File]::WriteAllText($out, $text, (New-Object System.Text.UTF8Encoding($false)))
}

function Invoke-Clang([string[]]$arguments) {
    $ErrorActionPreference = 'Continue'  # clang writes diagnostics to stderr
    $output = & $cc @arguments 2>&1 | ForEach-Object { "$_" }
    $code = $LASTEXITCODE
    $output | ForEach-Object { Write-Host $_ }
    $warnings = @($output | Where-Object { $_ -match ': warning:' }).Count
    $errors = @($output | Where-Object { $_ -match ': error:' }).Count
    Write-Host ("clang exit={0} errors={1} warnings={2}" -f $code, $errors, $warnings)
    return $code
}

# Windhawk's toolchain links exes against its libc++/libunwind, imported under
# the names Windhawk ships them as (*.whl). Put copies next to our exes.
function Copy-Runtime {
    $bin = Join-Path $compilerDir 'x86_64-w64-mingw32\bin'
    Copy-Item (Join-Path $bin 'libc++.dll') (Join-Path $dist 'libc++.whl') -Force
    Copy-Item (Join-Path $bin 'libunwind.dll') (Join-Path $dist 'libunwind.whl') -Force
}

if ($Check) {
    $tmp = Join-Path $dist ("check_" + [IO.Path]::GetFileNameWithoutExtension($Check) + '.cpp')
    Join-Fragments @((Join-Path $src '10_contract.h'), (Join-Path $src $Check)) $tmp
    $code = Invoke-Clang ($flags + @('-DHT_STANDALONE', '-fsyntax-only', $tmp))
    exit $code
}

if ($Test) {
    $exe = Join-Path $dist 'layout_test.exe'
    $tmp = Join-Path $dist 'layout_test_all.cpp'
    Join-Fragments @((Join-Path $src '10_contract.h'), (Join-Path $src '20_layout.cpp'),
        (Join-Path $root 'tests\layout_test.cpp')) $tmp
    $code = Invoke-Clang ($flags + @('-DHT_STANDALONE', '-o', $exe, $tmp) + $libs)
    if ($code -ne 0) { exit $code }
    Copy-Runtime
    & $exe
    exit $LASTEXITCODE
}

if ($Standalone) {
    $exe = Join-Path $dist 'hawktile-dev.exe'
    $tmp = Join-Path $dist 'hawktile_standalone.cpp'
    $paths = @($modFragments | Where-Object { $_ -ne '90_toolmod.cpp' } | ForEach-Object { Join-Path $src $_ })
    $paths += (Join-Path $root 'tests\standalone_main.cpp')
    Join-Fragments $paths $tmp
    $code = Invoke-Clang ($flags + @('-DHT_STANDALONE', '-O2', '-o', $exe, $tmp) + $libs)
    if ($code -eq 0) { Copy-Runtime }
    exit $code
}

$out = Join-Path $dist 'hawktile.wh.cpp'
Join-Fragments @($modFragments | ForEach-Object { Join-Path $src $_ }) $out
Write-Host "Wrote $out"
$code = Invoke-Clang ($flags + @('-shared', '-O2', '-o', (Join-Path $dist 'hawktile-check.dll'), $out) + $libs)
if ($code -ne 0) { exit $code }

# Second pass the way Windhawk itself compiles: no WH_EDITING stubs, real
# WH_MOD_ID. A response file keeps the quoted macro values intact.
Write-Host 'Real-mode syntax check (no WH_EDITING):'
$realFlags = @($flags | Where-Object { $_ -ne '-DWH_EDITING' }) | ForEach-Object {
    if ($_ -match '[\s"]') { '"' + ($_ -replace '\\', '/' -replace '"', '\"') + '"' } else { $_ }
}
$rsp = Join-Path $dist 'real.rsp'
$rspText = ($realFlags + @('"-DWH_MOD_ID=L\"hawktile\""', '"-DWH_MOD_VERSION=L\"0.1.0\""', '-fsyntax-only')) -join ' '
[System.IO.File]::WriteAllText($rsp, $rspText)
$code = Invoke-Clang @("@$rsp", $out)
exit $code
