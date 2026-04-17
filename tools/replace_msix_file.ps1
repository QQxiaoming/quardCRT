param(
    [Parameter(Mandatory = $true)]
    [string]$PackagePath,

    [Parameter(Mandatory = $true)]
    [string]$ReplacementPath,

    [string]$InternalFilePath = "quardCRT.exe",

    [string]$OutputPath,

    [string]$MakeAppxPath,

    [switch]$Force
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

function Write-Info {
    param([string]$Message)
    Write-Host "[INFO] $Message"
}

function Write-Step {
    param([string]$Message)
    Write-Host "`n==> $Message"
}

function Find-MakeAppx {
    param([string]$RequestedPath)

    if ($RequestedPath) {
        if (-not (Test-Path -LiteralPath $RequestedPath)) {
            throw "makeappx.exe was not found at $RequestedPath"
        }
        return (Resolve-Path -LiteralPath $RequestedPath).Path
    }

    $command = Get-Command makeappx.exe -ErrorAction SilentlyContinue
    if ($command) {
        return $command.Source
    }

    $kitsRoot = Join-Path ${env:ProgramFiles(x86)} "Windows Kits\10\bin"
    if (Test-Path -LiteralPath $kitsRoot) {
        $candidates = Get-ChildItem -Path $kitsRoot -Filter makeappx.exe -Recurse -ErrorAction SilentlyContinue |
            Where-Object { $_.FullName -match "\\x64\\makeappx\.exe$" } |
            Sort-Object FullName -Descending
        if ($candidates) {
            return $candidates[0].FullName
        }
    }

    throw "makeappx.exe was not found. Install Windows SDK or pass -MakeAppxPath explicitly."
}

function Invoke-NativeCommand {
    param(
        [string]$FilePath,
        [string[]]$Arguments
    )

    & $FilePath @Arguments
    if ($LASTEXITCODE -ne 0) {
        throw "Command failed with exit code ${LASTEXITCODE}: $FilePath $($Arguments -join ' ')"
    }
}

function Get-DefaultOutputPath {
    param([string]$ResolvedPackagePath)

    $packageDirectory = Split-Path -Parent $ResolvedPackagePath
    $packageBaseName = [System.IO.Path]::GetFileNameWithoutExtension($ResolvedPackagePath)
    $packageExtension = [System.IO.Path]::GetExtension($ResolvedPackagePath)
    return Join-Path $packageDirectory ("{0}_modified_unsigned{1}" -f $packageBaseName, $packageExtension)
}

function Get-RelativePackagePath {
    param(
        [string]$RootPath,
        [string]$ChildPath
    )

    $relativePath = $ChildPath.Substring($RootPath.Length).TrimStart([char[]]@([char]'\', [char]'/'))
    return ($relativePath -replace '/', '\')
}

function Get-ExeCandidatesMessage {
    param(
        [string]$RootPath,
        [string]$RequestedInternalPath
    )

    $requestedLeafName = [System.IO.Path]::GetFileName($RequestedInternalPath)
    $exeFiles = @(Get-ChildItem -Path $RootPath -Recurse -File -Filter "*.exe" -ErrorAction SilentlyContinue |
        Sort-Object FullName)

    if (-not $exeFiles) {
        return "No .exe files were found in the unpacked package."
    }

    $preferredFiles = @($exeFiles | Where-Object { $_.Name -ieq $requestedLeafName })
    $candidateFiles = @(if ($preferredFiles.Count -gt 0) { $preferredFiles } else { $exeFiles })

    $candidateLines = @($candidateFiles | Select-Object -First 20 | ForEach-Object {
        " - {0}" -f (Get-RelativePackagePath -RootPath $RootPath -ChildPath $_.FullName)
    })

    $message = "Available .exe paths inside the package:`n{0}" -f ($candidateLines -join "`n")
    if ($candidateFiles.Count -gt 20) {
        $message += "`n - ..."
    }

    return $message
}

$resolvedPackagePath = (Resolve-Path -LiteralPath $PackagePath).Path
$resolvedReplacementPath = (Resolve-Path -LiteralPath $ReplacementPath).Path

if (-not $OutputPath) {
    $OutputPath = Get-DefaultOutputPath -ResolvedPackagePath $resolvedPackagePath
}

$resolvedOutputPath = [System.IO.Path]::GetFullPath($OutputPath)
$resolvedMakeAppxPath = Find-MakeAppx -RequestedPath $MakeAppxPath

$packageExtension = [System.IO.Path]::GetExtension($resolvedPackagePath)
if ($packageExtension -notin @(".msix", ".appx")) {
    throw "Only .msix and .appx packages are supported. Input package: $resolvedPackagePath"
}

$normalizedInternalFilePath = $InternalFilePath.TrimStart([char[]]@([char]'\', [char]'/')) -replace '/', '\\'
if ([string]::IsNullOrWhiteSpace($normalizedInternalFilePath)) {
    throw "InternalFilePath must not be empty."
}

if ((Test-Path -LiteralPath $resolvedOutputPath) -and (-not $Force)) {
    throw "Output package already exists: $resolvedOutputPath. Use -Force to overwrite it."
}

$tempRoot = Join-Path ([System.IO.Path]::GetTempPath()) ("msix_replace_{0}" -f [guid]::NewGuid().ToString("N"))
$unpackDirectory = Join-Path $tempRoot "unpacked"

try {
    New-Item -ItemType Directory -Path $unpackDirectory -Force | Out-Null

    Write-Step "Unpacking package"
    Invoke-NativeCommand -FilePath $resolvedMakeAppxPath -Arguments @(
        "unpack",
        "/p", $resolvedPackagePath,
        "/d", $unpackDirectory,
        "/o"
    )

    $targetFilePath = Join-Path $unpackDirectory $normalizedInternalFilePath
    if (-not (Test-Path -LiteralPath $targetFilePath)) {
        $candidateMessage = Get-ExeCandidatesMessage -RootPath $unpackDirectory -RequestedInternalPath $normalizedInternalFilePath
        throw "Target file was not found inside the package: $normalizedInternalFilePath`n$candidateMessage"
    }

    Write-Step "Replacing file"
    Copy-Item -LiteralPath $resolvedReplacementPath -Destination $targetFilePath -Force

    $signaturePath = Join-Path $unpackDirectory "AppxSignature.p7x"
    if (Test-Path -LiteralPath $signaturePath) {
        Write-Info "Removing old package signature"
        Remove-Item -LiteralPath $signaturePath -Force
    }

    if (Test-Path -LiteralPath $resolvedOutputPath) {
        Remove-Item -LiteralPath $resolvedOutputPath -Force
    }

    Write-Step "Repacking package"
    Invoke-NativeCommand -FilePath $resolvedMakeAppxPath -Arguments @(
        "pack",
        "/d", $unpackDirectory,
        "/p", $resolvedOutputPath,
        "/o"
    )

    Write-Host ""
    Write-Host "Package created: $resolvedOutputPath"
    Write-Host "The package is unsigned. Install or distribution will require signing later."
} finally {
    if (Test-Path -LiteralPath $tempRoot) {
        Remove-Item -LiteralPath $tempRoot -Recurse -Force
    }
}