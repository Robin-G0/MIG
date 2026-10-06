param([switch]$Check)
$ErrorActionPreference = 'Stop'
$projectRoot = [IO.Path]::GetFullPath("$PSScriptRoot/..")
$formatTool = Get-Command clang-format -ErrorAction SilentlyContinue
if ($formatTool) {
    $formatterPath = $formatTool.Source
} else {
    $vswherePath = "${env:ProgramFiles(x86)}/Microsoft Visual Studio/Installer/vswhere.exe"
    if (-not (Test-Path $vswherePath)) {
        throw 'Install clang-format 16+ or the Visual Studio LLVM tools.'
    }
    $vsPath = & $vswherePath -latest -products '*' -property installationPath
    $formatterPath = Join-Path $vsPath 'VC/Tools/Llvm/bin/clang-format.exe'
}
if (-not (Test-Path $formatterPath)) {
    throw 'clang-format not found. Install LLVM or the Visual Studio LLVM component.'
}
[string[]]$formatArguments = if ($Check) { @('--dry-run', '--Werror') } else { @('-i') }
# First-party sources only; never traverse build/, installed files or downloaded SDKs.
foreach ($directory in @('src', 'tests', 'examples', 'integrations')) {
    $files = Get-ChildItem -LiteralPath (Join-Path $projectRoot $directory) -Recurse -File |
        Where-Object { $_.Extension -in @('.cpp', '.hpp', '.h') }
    foreach ($file in $files) {
        & $formatterPath @formatArguments $file.FullName
        if ($LASTEXITCODE) {
            throw "Formatting failed: $($file.FullName)"
        }
    }
}
