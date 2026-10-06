function Invoke-MigRequest {
    param([scriptblock]$Operation)

    for ($attempt = 1; $attempt -le 4; $attempt++) {
        try {
            return & $Operation
        } catch {
            $response = $_.Exception.Response
            $status = if ($response) { [int]$response.StatusCode } else { 0 }
            $permanent = $status -ge 400 -and $status -lt 500 -and $status -notin @(408, 429)
            if ($permanent -or $attempt -eq 4 -or
                $_.Exception -is [UnauthorizedAccessException] -or
                $_.Exception -is [IO.IOException]) {
                throw
            }
            $delay = [int][Math]::Pow(2, $attempt)
            Write-Warning "Request failed (attempt $attempt/4); retrying in ${delay}s: $($_.Exception.Message)"
            Start-Sleep -Seconds $delay
        }
    }
}

function Get-MigArtifact {
    param([string]$Uri, [string]$Destination, [string]$Sha256)

    $target = [IO.Path]::GetFullPath($Destination)
    if (Test-Path -LiteralPath $target) {
        $valid = (Get-Item -LiteralPath $target).Length -gt 0
        if ($Sha256) {
            $valid = $valid -and (Get-FileHash -LiteralPath $target -Algorithm SHA256).Hash -eq $Sha256
        }
        if ($valid) { return }
        Write-Warning "Discarding invalid cached artifact: $target"
        Remove-Item -LiteralPath $target -Force
    }

    $parent = Split-Path $target
    New-Item -ItemType Directory -Force -Path $parent | Out-Null
    $temporary = Join-Path $parent ([IO.Path]::GetRandomFileName() + '.download')
    Write-Host "Downloading $(Split-Path $target -Leaf) from $(([Uri]$Uri).Host)"
    try {
        Invoke-MigRequest {
            Invoke-WebRequest -UseBasicParsing -Uri $Uri -OutFile $temporary -TimeoutSec 90 -ErrorAction Stop
        }
        if ((Get-Item -LiteralPath $temporary).Length -eq 0) {
            throw "Downloaded artifact is empty: $Uri"
        }
        if ($Sha256 -and (Get-FileHash -LiteralPath $temporary -Algorithm SHA256).Hash -ne $Sha256) {
            throw "Downloaded artifact SHA256 mismatch: $Uri"
        }
        Move-Item -LiteralPath $temporary -Destination $target -Force
    } finally {
        if (Test-Path -LiteralPath $temporary) {
            Remove-Item -LiteralPath $temporary -Force
        }
    }
}
