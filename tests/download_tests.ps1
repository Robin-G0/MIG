$ErrorActionPreference = 'Stop'
. "$PSScriptRoot/../tools/download.ps1"

function Assert-Download([bool]$condition, [string]$message) {
    if (-not $condition) { throw $message }
}

function Invoke-WebRequest {
    [CmdletBinding()]
    param([string]$Uri, [string]$OutFile, [switch]$UseBasicParsing, [int]$TimeoutSec)

    $script:requests++
    Assert-Download ($TimeoutSec -eq 90) 'Requests must have a bounded timeout.'
    [IO.File]::WriteAllText($OutFile, 'partial')
    if ($script:requests -le $script:failures) {
        throw [Net.WebException]::new('Simulated connection timeout.')
    }
    [IO.File]::WriteAllText($OutFile, $script:payload)
}

function Start-Sleep {
    param([int]$Seconds)
    $script:delays += $Seconds
}

function Reset-Download([int]$failures, [string]$payload = 'verified') {
    $script:requests = 0
    $script:failures = $failures
    $script:payload = $payload
    $script:delays = @()
}

function Assert-DownloadFailure([scriptblock]$operation, [string]$message) {
    $failed = $false
    try { & $operation } catch { $failed = $true }
    Assert-Download $failed $message
}

$folder = Join-Path ([IO.Path]::GetTempPath()) ([IO.Path]::GetRandomFileName())
New-Item -ItemType Directory -Path $folder | Out-Null
$target = Join-Path $folder 'dependency.zip'
try {
    [IO.File]::WriteAllText($target, 'verified')
    $hash = (Get-FileHash -LiteralPath $target -Algorithm SHA256).Hash
    Reset-Download 4
    Get-MigArtifact -Uri 'https://artifact.invalid/dependency' -Destination $target -Sha256 $hash
    Assert-Download ($script:requests -eq 0) 'A valid cache must not require the network.'

    [IO.File]::WriteAllText($target, 'damaged cached download')
    Reset-Download 2
    Get-MigArtifact -Uri 'https://artifact.invalid/dependency' -Destination $target -Sha256 $hash
    Assert-Download ($script:requests -eq 3) 'Transient failures must recover.'
    Assert-Download (($script:delays -join ',') -eq '2,4') 'Retry delays must increase.'
    Assert-Download ((Get-FileHash -LiteralPath $target).Hash -eq $hash) 'Corrupt cache must be replaced.'

    Remove-Item -LiteralPath $target
    Reset-Download 4
    Assert-DownloadFailure {
        Get-MigArtifact -Uri 'https://artifact.invalid/dependency' -Destination $target -Sha256 $hash
    } 'Exhausted retries must fail the build.'
    Assert-Download ($script:requests -eq 4) 'Retries must be bounded.'
    Assert-Download (-not (Test-Path -LiteralPath $target)) 'Interrupted downloads must never be cached.'
    Assert-Download (@(Get-ChildItem -LiteralPath $folder -Filter '*.download').Count -eq 0) 'Temporary downloads must be removed.'

    Reset-Download 0 'wrong checksum'
    Assert-DownloadFailure {
        Get-MigArtifact -Uri 'https://artifact.invalid/dependency' -Destination $target -Sha256 $hash
    } 'A checksum mismatch must fail the build.'
    Assert-Download ($script:requests -eq 1) 'Integrity failures must not be hidden by retries.'
    Assert-Download (-not (Test-Path -LiteralPath $target)) 'Unverified bytes must never be cached.'

    Reset-Download 0 ''
    Assert-DownloadFailure {
        Get-MigArtifact -Uri 'https://artifact.invalid/license' -Destination $target
    } 'Empty unhashed artifacts must fail.'
    Assert-Download (-not (Test-Path -LiteralPath $target)) 'Empty artifacts must never be cached.'

    $script:requests = 0
    $value = Invoke-MigRequest {
        $script:requests++
        if ($script:requests -eq 1) { throw [Net.WebException]::new('Metadata timeout.') }
        @{ version = '1.0.0' }
    }
    Assert-Download ($script:requests -eq 2 -and $value.version -eq '1.0.0') 'Metadata requests must recover and return their value.'

    foreach ($status in @(404, 408, 429, 503)) {
        Reset-Download 0
        Assert-DownloadFailure {
            Invoke-MigRequest {
                $script:requests++
                $failure = [Exception]::new("Simulated HTTP $status")
                $failure | Add-Member -NotePropertyName Response -NotePropertyValue @{ StatusCode = $status }
                throw $failure
            }
        } "HTTP $status must eventually fail."
        $expected = if ($status -eq 404) { 1 } else { 4 }
        Assert-Download ($script:requests -eq $expected) "Incorrect retry policy for HTTP $status."
    }
    Write-Output 'Download tests passed: cache recovery, retries, cleanup, integrity, metadata and HTTP status policy.'
} finally {
    Get-ChildItem -LiteralPath $folder -File | Remove-Item -Force
    Remove-Item -LiteralPath $folder
}
