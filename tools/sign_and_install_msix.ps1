param(
    [Parameter(Mandatory = $true)]
    [string]$PackagePath,

    [string]$Publisher,

    [string]$PfxPath,

    [string]$CerPath,

    [string]$Password = "123456",

    [string]$SignToolPath,

    [switch]$SkipInstall,

    [switch]$ForceNewCertificate,

    [switch]$RemoveExistingPackage,

    [string]$TimestampServer
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

function Test-IsAdministrator {
    $currentIdentity = [Security.Principal.WindowsIdentity]::GetCurrent()
    $principal = New-Object Security.Principal.WindowsPrincipal($currentIdentity)
    return $principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
}

function Get-PackageIdentity {
    param([string]$ArchivePath)

    Add-Type -AssemblyName System.IO.Compression.FileSystem
    $archive = [System.IO.Compression.ZipFile]::OpenRead($ArchivePath)
    try {
        $manifestEntry = $archive.GetEntry("AppxManifest.xml")
        if (-not $manifestEntry) {
            $manifestEntry = $archive.GetEntry("AppxMetadata/AppxBundleManifest.xml")
        }
        if (-not $manifestEntry) {
            throw "AppxManifest.xml or AppxBundleManifest.xml was not found in $ArchivePath"
        }

        $stream = $manifestEntry.Open()
        try {
            $reader = New-Object System.IO.StreamReader($stream)
            try {
                $manifestContent = $reader.ReadToEnd()
            } finally {
                $reader.Dispose()
            }
        } finally {
            $stream.Dispose()
        }

        $xml = [xml]$manifestContent
        $identityNode = $xml.SelectSingleNode("//*[local-name()='Identity']")
        if (-not $identityNode) {
            throw "Identity node was not found in package manifest"
        }

        $name = $identityNode.GetAttribute("Name")
        $publisher = $identityNode.GetAttribute("Publisher")
        $version = $identityNode.GetAttribute("Version")
        $processorArchitecture = $identityNode.GetAttribute("ProcessorArchitecture")
        $resourceId = $identityNode.GetAttribute("ResourceId")

        [PSCustomObject]@{
            Name = $name
            Publisher = $publisher
            Version = $version
            ProcessorArchitecture = $processorArchitecture
            ResourceId = $resourceId
        }
    } finally {
        $archive.Dispose()
    }
}

function Find-SignTool {
    param([string]$RequestedPath)

    if ($RequestedPath) {
        if (-not (Test-Path -LiteralPath $RequestedPath)) {
            throw "SignTool was not found at $RequestedPath"
        }
        return (Resolve-Path -LiteralPath $RequestedPath).Path
    }

    $command = Get-Command signtool.exe -ErrorAction SilentlyContinue
    if ($command) {
        return $command.Source
    }

    $kitsRoot = Join-Path ${env:ProgramFiles(x86)} "Windows Kits\10\bin"
    if (Test-Path -LiteralPath $kitsRoot) {
        $candidates = Get-ChildItem -Path $kitsRoot -Filter signtool.exe -Recurse -ErrorAction SilentlyContinue |
            Where-Object { $_.FullName -match "\\x64\\signtool\.exe$" } |
            Sort-Object FullName -Descending
        if ($candidates) {
            return $candidates[0].FullName
        }
    }

    throw "signtool.exe was not found. Install Windows SDK or pass -SignToolPath explicitly."
}

function Get-CertificateFromPfx {
    param(
        [string]$Path,
        [Security.SecureString]$SecurePassword
    )

    $flags = [System.Security.Cryptography.X509Certificates.X509KeyStorageFlags]::Exportable
    $flags = $flags -bor [System.Security.Cryptography.X509Certificates.X509KeyStorageFlags]::PersistKeySet
    $certificate = New-Object System.Security.Cryptography.X509Certificates.X509Certificate2
    $certificate.Import($Path, $SecurePassword, $flags)
    return $certificate
}

function Test-CodeSigningCertificate {
    param([System.Security.Cryptography.X509Certificates.X509Certificate2]$Certificate)

    if (-not $Certificate) {
        return $false
    }
    if (-not $Certificate.HasPrivateKey) {
        return $false
    }
    if ($Certificate.NotAfter -le (Get-Date)) {
        return $false
    }

    $enhancedKeyUsageExtension = $Certificate.Extensions |
        Where-Object { $_.Oid -and $_.Oid.Value -eq "2.5.29.37" } |
        Select-Object -First 1
    if (-not $enhancedKeyUsageExtension) {
        return $false
    }

    $enhancedKeyUsages = New-Object System.Security.Cryptography.X509Certificates.X509EnhancedKeyUsageExtension($enhancedKeyUsageExtension, $false)
    foreach ($usage in $enhancedKeyUsages.EnhancedKeyUsages) {
        if ($usage.Value -eq "1.3.6.1.5.5.7.3.3") {
            return $true
        }
    }

    return $false
}

function New-OrReuseTestCertificate {
    param(
        [string]$ExpectedPublisher,
        [string]$ResolvedPfxPath,
        [string]$ResolvedCerPath,
        [Security.SecureString]$SecurePassword,
        [bool]$CreateFresh
    )

    if ((-not $CreateFresh) -and (Test-Path -LiteralPath $ResolvedPfxPath)) {
        Write-Info "Reusing existing PFX at $ResolvedPfxPath"
        $existingCertificate = Get-CertificateFromPfx -Path $ResolvedPfxPath -SecurePassword $SecurePassword
        if ($existingCertificate.Subject -ne $ExpectedPublisher) {
            throw "Existing PFX subject '$($existingCertificate.Subject)' does not match publisher '$ExpectedPublisher'"
        }
        if (-not (Test-CodeSigningCertificate -Certificate $existingCertificate)) {
            Write-Info "Existing PFX certificate is not a valid code-signing certificate. A new certificate will be created."
            $existingCertificate = $null
        }
        if ($existingCertificate) {
        if (-not (Test-Path -LiteralPath $ResolvedCerPath)) {
            Export-Certificate -Cert $existingCertificate -FilePath $ResolvedCerPath | Out-Null
        }
        return $existingCertificate
        }
    }

    $reusableCertificate = $null
    if (-not $CreateFresh) {
        $reusableCertificate = Get-ChildItem Cert:\CurrentUser\My |
            Where-Object { $_.Subject -eq $ExpectedPublisher -and $_.HasPrivateKey } |
            Sort-Object NotAfter -Descending |
            Select-Object -First 1
        if ($reusableCertificate -and (-not (Test-CodeSigningCertificate -Certificate $reusableCertificate))) {
            Write-Info "Existing certificate in CurrentUser\\My is not suitable for code signing. A new certificate will be created."
            $reusableCertificate = $null
        }
    }

    if (-not $reusableCertificate) {
        Write-Info "Creating a new self-signed test certificate for $ExpectedPublisher"
        $reusableCertificate = New-SelfSignedCertificate `
            -Type CodeSigningCert `
            -Subject $ExpectedPublisher `
            -KeyAlgorithm RSA `
            -KeyLength 2048 `
            -HashAlgorithm SHA256 `
            -KeyUsage DigitalSignature `
            -KeyExportPolicy Exportable `
            -FriendlyName "quardCRT MSIX Local Test" `
            -CertStoreLocation "Cert:\CurrentUser\My"
    } else {
        Write-Info "Reusing certificate from CurrentUser\\My with thumbprint $($reusableCertificate.Thumbprint)"
    }

    Export-PfxCertificate -Cert $reusableCertificate -FilePath $ResolvedPfxPath -Password $SecurePassword | Out-Null
    Export-Certificate -Cert $reusableCertificate -FilePath $ResolvedCerPath -Force | Out-Null
    return $reusableCertificate
}

function Import-CertificateToStore {
    param(
        [System.Security.Cryptography.X509Certificates.X509Certificate2]$Certificate,
        [string]$StoreName,
        [string]$StoreLocation
    )

    $existingCertificate = Get-ChildItem ("Cert:\{0}\{1}" -f $StoreLocation, $StoreName) -ErrorAction SilentlyContinue |
        Where-Object { $_.Thumbprint -eq $Certificate.Thumbprint } |
        Select-Object -First 1
    if ($existingCertificate) {
        Write-Info "Certificate already exists in $StoreLocation\\$StoreName"
        return
    }

    Write-Info "Importing certificate into $StoreLocation\\$StoreName"
    $store = New-Object System.Security.Cryptography.X509Certificates.X509Store($StoreName, $StoreLocation)
    try {
        $store.Open([System.Security.Cryptography.X509Certificates.OpenFlags]::ReadWrite)
        $store.Add($Certificate)
    } finally {
        $store.Close()
    }
    Write-Info "Imported certificate into $StoreLocation\\$StoreName"
}

function Ensure-CertificateTrusted {
    param([System.Security.Cryptography.X509Certificates.X509Certificate2]$Certificate)

    $publicCertificateBytes = $Certificate.Export([System.Security.Cryptography.X509Certificates.X509ContentType]::Cert)
    $publicCertificate = New-Object System.Security.Cryptography.X509Certificates.X509Certificate2(,$publicCertificateBytes)

    try {
        Import-CertificateToStore -Certificate $publicCertificate -StoreName "TrustedPeople" -StoreLocation "CurrentUser"
        Write-Info "Windows may show a trust confirmation dialog for a new root certificate. Approve it if prompted."
        Import-CertificateToStore -Certificate $publicCertificate -StoreName "Root" -StoreLocation "CurrentUser"

        if (Test-IsAdministrator) {
            Import-CertificateToStore -Certificate $publicCertificate -StoreName "TrustedPeople" -StoreLocation "LocalMachine"
            Import-CertificateToStore -Certificate $publicCertificate -StoreName "Root" -StoreLocation "LocalMachine"
        } else {
            Write-Info "Current PowerShell session is not elevated. If Add-AppxPackage still reports 0x800B0109, rerun this script as Administrator so the certificate can also be imported into LocalMachine trust stores."
        }
    } finally {
        $publicCertificate.Dispose()
    }
}

function Install-Package {
    param([string]$ResolvedPackagePath)

    try {
        Add-AppxPackage -Path $ResolvedPackagePath -ForceUpdateFromAnyVersion
    } catch {
        $message = $_.Exception.Message
        if ($message -match "0x800B0109") {
            $extraMessage = "Package signature root is not trusted by the AppX deployment service. Run this script from an elevated PowerShell window so the certificate can be imported into LocalMachine\\TrustedPeople and LocalMachine\\Root, then retry."
            throw "$message`n$extraMessage"
        }
        if ($message -match "0x80073CFB") {
            $extraMessage = "A package with the same identity is already installed but has different contents. Rerun this script with -RemoveExistingPackage from an elevated PowerShell window, or uninstall the old package for all users before installing this one."
            throw "$message`n$extraMessage"
        }
        throw
    }
}

function Sign-Package {
    param(
        [string]$ResolvedSignToolPath,
        [string]$ResolvedPackagePath,
        [System.Security.Cryptography.X509Certificates.X509Certificate2]$Certificate,
        [string]$ResolvedPfxPath,
        [string]$PlainTextPassword,
        [string]$TimeStampUrl
    )

    $arguments = @(
        "sign",
        "/fd", "SHA256"
    )

    if ($Certificate -and $Certificate.Thumbprint -and $Certificate.HasPrivateKey) {
        Write-Info "Signing with certificate thumbprint $($Certificate.Thumbprint) from CurrentUser\\My"
        $arguments += @(
            "/sha1", $Certificate.Thumbprint,
            "/s", "My"
        )
    } else {
        Write-Info "Signing with exported PFX at $ResolvedPfxPath"
        $arguments += @(
            "/f", $ResolvedPfxPath,
            "/p", $PlainTextPassword
        )
    }

    if ($TimeStampUrl) {
        $arguments += @("/tr", $TimeStampUrl, "/td", "SHA256")
    }

    $arguments += $ResolvedPackagePath

    Write-Info "Signing package with $ResolvedSignToolPath"
    & $ResolvedSignToolPath @arguments
    if ($LASTEXITCODE -ne 0) {
        throw "signtool.exe failed with exit code $LASTEXITCODE"
    }
}

function Remove-InstalledPackageIfRequested {
    param(
        [string]$IdentityName,
        [bool]$ShouldRemove
    )

    if (-not $ShouldRemove) {
        return
    }

    $installedPackages = @()
    $installedPackages += Get-AppxPackage -Name $IdentityName -ErrorAction SilentlyContinue

    if (Test-IsAdministrator) {
        $installedPackages += Get-AppxPackage -AllUsers -Name $IdentityName -ErrorAction SilentlyContinue
    }

    $installedPackages = $installedPackages |
        Where-Object { $_ } |
        Sort-Object PackageFullName -Unique

    foreach ($installedPackage in $installedPackages) {
        Write-Info "Removing installed package $($installedPackage.PackageFullName)"
        if (Test-IsAdministrator) {
            Remove-AppxPackage -Package $installedPackage.PackageFullName -AllUsers -ErrorAction SilentlyContinue
        }
        Remove-AppxPackage -Package $installedPackage.PackageFullName -ErrorAction SilentlyContinue
    }
}

function Show-InstalledPackageInfo {
    param([string]$IdentityName)

    $installedPackages = Get-AppxPackage -Name $IdentityName -ErrorAction SilentlyContinue
    if (-not $installedPackages) {
        Write-Info "Package is not currently installed."
        return
    }

    $installedPackage = $installedPackages | Sort-Object Version -Descending | Select-Object -First 1
    Write-Host ""
    Write-Host "Package identity summary"
    Write-Host "  Name:               $($installedPackage.Name)"
    Write-Host "  Publisher:          $($installedPackage.Publisher)"
    Write-Host "  PackageFullName:    $($installedPackage.PackageFullName)"
    Write-Host "  PackageFamilyName:  $($installedPackage.PackageFamilyName)"
    Write-Host "  InstallLocation:    $($installedPackage.InstallLocation)"
}

$resolvedPackagePath = (Resolve-Path -LiteralPath $PackagePath).Path
$packageIdentity = Get-PackageIdentity -ArchivePath $resolvedPackagePath

if (-not $Publisher) {
    $Publisher = $packageIdentity.Publisher
}
if (-not $Publisher) {
    throw "Publisher could not be determined from the package manifest. Pass -Publisher explicitly."
}

$packageDirectory = Split-Path -Parent $resolvedPackagePath
$packageBaseName = [System.IO.Path]::GetFileNameWithoutExtension($resolvedPackagePath)

if (-not $PfxPath) {
    $PfxPath = Join-Path $packageDirectory ("{0}-test-signing.pfx" -f $packageBaseName)
}
if (-not $CerPath) {
    $CerPath = Join-Path $packageDirectory ("{0}-test-signing.cer" -f $packageBaseName)
}

$resolvedPfxPath = $PfxPath
$resolvedCerPath = $CerPath
$securePassword = ConvertTo-SecureString $Password -AsPlainText -Force
$resolvedSignToolPath = Find-SignTool -RequestedPath $SignToolPath

Write-Step "Package information"
Write-Host "Name:                  $($packageIdentity.Name)"
Write-Host "Publisher:             $Publisher"
Write-Host "Version:               $($packageIdentity.Version)"
Write-Host "Architecture:          $($packageIdentity.ProcessorArchitecture)"
Write-Host "Package path:          $resolvedPackagePath"
Write-Host "SignTool path:         $resolvedSignToolPath"
Write-Host "PFX path:              $resolvedPfxPath"
Write-Host "CER path:              $resolvedCerPath"

Write-Step "Prepare signing certificate"
$certificate = New-OrReuseTestCertificate `
    -ExpectedPublisher $Publisher `
    -ResolvedPfxPath $resolvedPfxPath `
    -ResolvedCerPath $resolvedCerPath `
    -SecurePassword $securePassword `
    -CreateFresh $ForceNewCertificate.IsPresent
Ensure-CertificateTrusted -Certificate $certificate

Write-Step "Sign package"
Sign-Package `
    -ResolvedSignToolPath $resolvedSignToolPath `
    -ResolvedPackagePath $resolvedPackagePath `
    -Certificate $certificate `
    -ResolvedPfxPath $resolvedPfxPath `
    -PlainTextPassword $Password `
    -TimeStampUrl $TimestampServer

if (-not $SkipInstall) {
    Write-Step "Install package"
    Remove-InstalledPackageIfRequested -IdentityName $packageIdentity.Name -ShouldRemove $RemoveExistingPackage.IsPresent
    Install-Package -ResolvedPackagePath $resolvedPackagePath
}

Write-Step "Installed package summary"
Show-InstalledPackageInfo -IdentityName $packageIdentity.Name

Write-Host ""
Write-Host "Completed successfully."