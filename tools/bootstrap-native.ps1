param(
    [string]$Destination = "$PSScriptRoot/../build/native-deps",
    [ValidateSet('x64', 'ARM64')][string]$Architecture = 'x64',
    [switch]$SkipHands
)
$ErrorActionPreference = 'Stop'
$root = [IO.Path]::GetFullPath($Destination)
New-Item -ItemType Directory -Force -Path $root | Out-Null
$version = '0.10.35'
$metadata = Invoke-RestMethod "https://pypi.org/pypi/mediapipe/$version/json"
$wheelPlatform = if ($Architecture -eq 'ARM64') { 'win_arm64' } else { 'win_amd64' }
$wheel = $metadata.urls | Where-Object filename -eq "mediapipe-$version-py3-none-$wheelPlatform.whl"
if (-not $wheel) {
    throw "Official Windows $Architecture MediaPipe archive not found."
}
$archive = Join-Path $root 'mediapipe.zip'
if (-not (Test-Path $archive)) {
    Invoke-WebRequest -UseBasicParsing $wheel.url -OutFile $archive
}
if ((Get-FileHash $archive -Algorithm SHA256).Hash.ToLowerInvariant() -ne $wheel.digests.sha256) {
    throw 'MediaPipe SHA256 mismatch.'
}
$pinnedHash = if ($Architecture -eq 'ARM64') {
    '46255326a6213118aaa518a7aa25e35f93337e82677960cc2a945f117bff8444'
} else {
    'b08f001cf3c3cd0d88d9ed68f3368dc8a4913f568281a93117f083115aa672ba'
}
if ((Get-FileHash $archive -Algorithm SHA256).Hash.ToLowerInvariant() -ne $pinnedHash) {
    throw 'MediaPipe pinned release hash mismatch.'
}
Expand-Archive -LiteralPath $archive -DestinationPath "$root/mediapipe-package" -Force
$headers = [Collections.Generic.HashSet[string]]::new()
$pending = [Collections.Generic.Queue[string]]::new()
$pending.Enqueue('mediapipe/tasks/c/vision/pose_landmarker/pose_landmarker.h')
if (-not $SkipHands) {
        $pending.Enqueue('mediapipe/tasks/c/vision/hand_landmarker/hand_landmarker.h')
}
while ($pending.Count) {
    $relative = $pending.Dequeue()
    if (-not $headers.Add($relative)) {
        continue
    }
    $target = Join-Path "$root/include" $relative
    New-Item -ItemType Directory -Force -Path (Split-Path $target) | Out-Null
    if (-not (Test-Path $target)) {
        Invoke-WebRequest -UseBasicParsing "https://raw.githubusercontent.com/google-ai-edge/mediapipe/v$version/$relative" -OutFile $target
    }
    $source = Get-Content -Raw $target
    foreach ($match in [regex]::Matches($source, '#include\s+"(mediapipe/[^"\r\n]+)"')) {
        $pending.Enqueue($match.Groups[1].Value)
    }
}
New-Item -ItemType Directory -Force -Path "$root/include/nlohmann" | Out-Null
if (-not (Test-Path "$root/include/nlohmann/json.hpp")) {
    Invoke-WebRequest -UseBasicParsing 'https://raw.githubusercontent.com/nlohmann/json/v3.11.3/single_include/nlohmann/json.hpp' -OutFile "$root/include/nlohmann/json.hpp"
}
if ((Get-FileHash "$root/include/nlohmann/json.hpp" -Algorithm SHA256).Hash.ToLowerInvariant() -ne '9bea4c8066ef4a1c206b2be5a36302f8926f7fdc6087af5d20b417d0cf103ea6') {
    throw 'JSON header SHA256 mismatch.'
}
if (-not (Test-Path "$root/nlohmann-LICENSE")) {
    Invoke-WebRequest -UseBasicParsing 'https://raw.githubusercontent.com/nlohmann/json/v3.11.3/LICENSE.MIT' -OutFile "$root/nlohmann-LICENSE"
}
New-Item -ItemType Directory -Force -Path "$root/models" | Out-Null
if (-not (Test-Path "$root/models/pose_landmarker_full.task")) {
    Invoke-WebRequest -UseBasicParsing 'https://storage.googleapis.com/mediapipe-models/pose_landmarker/pose_landmarker_full/float16/1/pose_landmarker_full.task' -OutFile "$root/models/pose_landmarker_full.task"
}
if ((Get-FileHash "$root/models/pose_landmarker_full.task" -Algorithm SHA256).Hash.ToLowerInvariant() -ne '5134a3aad27a58b93da0088d431f366da362b44e3ccfbe3462b3827a839011b1') {
    throw 'Pose model SHA256 mismatch.'
}
if (-not (Test-Path "$root/models/pose_landmarker_lite.task")) {
    Invoke-WebRequest -UseBasicParsing 'https://storage.googleapis.com/mediapipe-models/pose_landmarker/pose_landmarker_lite/float16/1/pose_landmarker_lite.task' -OutFile "$root/models/pose_landmarker_lite.task"
}
if ((Get-FileHash "$root/models/pose_landmarker_lite.task" -Algorithm SHA256).Hash.ToLowerInvariant() -ne '59929e1d1ee95287735ddd833b19cf4ac46d29bc7afddbbf6753c459690d574a') {
    throw 'Lite pose model SHA256 mismatch.'
}
if (-not $SkipHands -and -not (Test-Path "$root/models/hand_landmarker.task")) {
    Invoke-WebRequest -UseBasicParsing 'https://storage.googleapis.com/mediapipe-models/hand_landmarker/hand_landmarker/float16/1/hand_landmarker.task' -OutFile "$root/models/hand_landmarker.task"
}
if (-not $SkipHands -and (Get-FileHash "$root/models/hand_landmarker.task" -Algorithm SHA256).Hash.ToLowerInvariant() -ne 'fbc2a30080c3c557093b5ddfc334698132eb341044ccee322ccf8bcf3607cde1') {
    throw 'Hand model SHA256 mismatch.'
}
Write-Output "Native dependencies ready: $root (no Python runtime installed or used)"
Get-ChildItem -Recurse "$root/mediapipe-package" -Filter '*.dll' | Select-Object FullName
