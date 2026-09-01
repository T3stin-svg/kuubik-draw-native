[CmdletBinding()]
param(
    [string]$RepositoryRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path,
    [string]$Version = '0.2.0-preview.2'
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

function Assert-Success([string]$Step) {
    if ($LASTEXITCODE -ne 0) {
        throw "$Step failed with exit code $LASTEXITCODE"
    }
}

function Ensure-Directory([string]$Path) {
    if (-not (Test-Path -LiteralPath $Path)) {
        New-Item -ItemType Directory -Path $Path | Out-Null
    }
}

$repository = (Resolve-Path -LiteralPath $RepositoryRoot).Path
$buildDirectory = Join-Path $repository 'windows'
$sourceExe = Join-Path $buildDirectory 'KuubikDraw.exe'
if (-not (Test-Path -LiteralPath $sourceExe)) {
    throw "Missing native build output: $sourceExe"
}

$artifactRoot = Join-Path $repository 'artifacts'
$packageName = "KuubikDraw-$Version-win64"
$packageDirectory = Join-Path $artifactRoot $packageName
$zipPath = Join-Path $artifactRoot "$packageName.zip"
$artifactRootFull = [IO.Path]::GetFullPath($artifactRoot).TrimEnd([IO.Path]::DirectorySeparatorChar) + [IO.Path]::DirectorySeparatorChar
$packageFull = [IO.Path]::GetFullPath($packageDirectory)
if (-not $packageFull.StartsWith($artifactRootFull, [StringComparison]::OrdinalIgnoreCase)) {
    throw "Refusing to recreate package outside artifact root: $packageFull"
}

Ensure-Directory $artifactRoot
if (Test-Path -LiteralPath $packageDirectory) {
    Remove-Item -LiteralPath $packageDirectory -Recurse -Force
}
if (Test-Path -LiteralPath $zipPath) {
    Remove-Item -LiteralPath $zipPath -Force
}

Write-Host 'Compiling application and plugin translations...'
$translationSources = @(
    Get-ChildItem -LiteralPath (Join-Path $repository 'librecad\ts') -Filter '*.ts' -File
    Get-ChildItem -LiteralPath (Join-Path $repository 'plugins\ts') -Filter '*.ts' -File
)
foreach ($source in $translationSources) {
    & lrelease.exe $source.FullName | Out-Host
    Assert-Success "lrelease $($source.Name)"
}

Write-Host 'Deploying Qt runtime...'
& windeployqt.exe --release --compiler-runtime --force --verbose 1 $sourceExe | Out-Host
Assert-Success 'windeployqt'

# windeployqt places the Visual C++ redistributable installer beside MSVC
# builds.  A portable preview must not require an installer or administrator
# rights, so copy the redistributable CRT DLLs from the active Visual Studio
# toolchain and remove the installer from the payload.
$vcRedistRoot = $env:VCToolsRedistDir
if ([string]::IsNullOrWhiteSpace($vcRedistRoot) -or -not (Test-Path -LiteralPath $vcRedistRoot)) {
    throw 'VCToolsRedistDir is unavailable; cannot create a self-contained MSVC package.'
}
$vcRuntimeDirectory = Get-ChildItem -LiteralPath (Join-Path $vcRedistRoot 'x64') -Directory -Filter 'Microsoft.VC*.CRT' |
    Sort-Object Name -Descending |
    Select-Object -First 1
if ($null -eq $vcRuntimeDirectory) {
    throw "No x64 Visual C++ CRT directory found below $vcRedistRoot"
}
Copy-Item -Path (Join-Path $vcRuntimeDirectory.FullName '*.dll') -Destination $buildDirectory -Force
Get-ChildItem -LiteralPath $buildDirectory -Filter 'vc_redist*.exe' -File -ErrorAction SilentlyContinue |
    Remove-Item -Force

Ensure-Directory $packageDirectory
Copy-Item -Path (Join-Path $buildDirectory '*') -Destination $packageDirectory -Recurse -Force

$resourceRoot = Join-Path $packageDirectory 'resources'
$qmRoot = Join-Path $resourceRoot 'qm'
Ensure-Directory $resourceRoot
Ensure-Directory $qmRoot

Copy-Item -LiteralPath (Join-Path $repository 'librecad\support\fonts') -Destination $resourceRoot -Recurse -Force
Copy-Item -LiteralPath (Join-Path $repository 'librecad\support\patterns') -Destination $resourceRoot -Recurse -Force
Copy-Item -LiteralPath (Join-Path $repository 'librecad\support\library') -Destination $resourceRoot -Recurse -Force
Get-ChildItem -LiteralPath (Join-Path $repository 'librecad\ts') -Filter '*.qm' -File | Copy-Item -Destination $qmRoot -Force
Get-ChildItem -LiteralPath (Join-Path $repository 'plugins\ts') -Filter '*.qm' -File | Copy-Item -Destination $qmRoot -Force

$qtPrefix = (& qmake.exe -query QT_INSTALL_PREFIX).Trim()
Assert-Success 'qmake -query QT_INSTALL_PREFIX'
$qtTranslations = (& qmake.exe -query QT_INSTALL_TRANSLATIONS).Trim()
Assert-Success 'qmake -query QT_INSTALL_TRANSLATIONS'
if (Test-Path -LiteralPath $qtTranslations) {
    Get-ChildItem -LiteralPath $qtTranslations -Filter 'qt*_en.qm' -File -ErrorAction SilentlyContinue |
        Copy-Item -Destination $qmRoot -Force
}

$licenseRoot = Join-Path $packageDirectory 'licenses'
Ensure-Directory $licenseRoot
Copy-Item -LiteralPath (Join-Path $repository 'LICENSE') -Destination (Join-Path $packageDirectory 'LICENSE') -Force
Copy-Item -LiteralPath (Join-Path $repository 'FORK_NOTICE.md') -Destination $packageDirectory -Force
Copy-Item -LiteralPath (Join-Path $repository 'README_TEST.md') -Destination $packageDirectory -Force
Copy-Item -LiteralPath (Join-Path $repository 'THIRD_PARTY_NOTICES.md') -Destination $packageDirectory -Force
Copy-Item -Path (Join-Path $repository 'licenses\*') -Destination $licenseRoot -Recurse -Force

$boostLicense = Join-Path (Split-Path $repository -Parent) 'boost\LICENSE_1_0.txt'
if (Test-Path -LiteralPath $boostLicense) {
    Copy-Item -LiteralPath $boostLicense -Destination (Join-Path $licenseRoot 'BOOST_LICENSE_1_0.txt') -Force
}
$qtLicenseRoot = Join-Path $licenseRoot 'qt'
Ensure-Directory $qtLicenseRoot
Get-ChildItem -LiteralPath $qtPrefix -Filter 'LICENSE*' -File -ErrorAction SilentlyContinue |
    Copy-Item -Destination $qtLicenseRoot -Force
if (Test-Path -LiteralPath (Join-Path $qtPrefix 'LICENSES')) {
    Copy-Item -LiteralPath (Join-Path $qtPrefix 'LICENSES') -Destination $qtLicenseRoot -Recurse -Force
}

$forbiddenExtensions = @('.pdb', '.ilk', '.obj', '.exp', '.lib', '.idb', '.tlog')
Get-ChildItem -LiteralPath $packageDirectory -Recurse -File |
    Where-Object { $forbiddenExtensions -contains $_.Extension.ToLowerInvariant() } |
    Remove-Item -Force
Get-ChildItem -LiteralPath $packageDirectory -Filter 'vc_redist*.exe' -Recurse -File -ErrorAction SilentlyContinue |
    Remove-Item -Force

$sourceCommit = (& git -C $repository rev-parse HEAD).Trim()
Assert-Success 'git rev-parse HEAD'
$manifest = [ordered]@{
    product = 'Kuubik Draw'
    version = $Version
    executable = 'KuubikDraw.exe'
    architecture = 'windows-x64'
    toolchain = 'MSVC / Qt 5.15'
    qtVersion = (& qmake.exe -query QT_VERSION).Trim()
    sourceRepository = 'https://github.com/T3stin-svg/kuubik-draw-native'
    sourceCommit = $sourceCommit
    upstreamRepository = 'https://github.com/LibreCAD/LibreCAD'
    upstreamTag = 'v2.2.1.5'
    upstreamCommit = '7ebab007d9eb4c68609388b835a2487648f0877b'
    guaranteedFileWorkflows = @('DXF open/save', 'vector PDF export')
    notCertified = @('DWG roundtrip', 'DWT', 'XREF parity')
    builtAtUtc = [DateTime]::UtcNow.ToString('o')
    githubRunId = $env:GITHUB_RUN_ID
}
$manifest | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath (Join-Path $packageDirectory 'build-manifest.json') -Encoding UTF8

$checksumPath = Join-Path $packageDirectory 'SHA256SUMS.txt'
$checksums = Get-ChildItem -LiteralPath $packageDirectory -Recurse -File |
    Where-Object { $_.FullName -ne $checksumPath } |
    Sort-Object FullName |
    ForEach-Object {
        $relative = $_.FullName.Substring($packageDirectory.Length + 1).Replace('\', '/')
        $hash = (Get-FileHash -LiteralPath $_.FullName -Algorithm SHA256).Hash.ToLowerInvariant()
        "$hash  $relative"
    }
$checksums | Set-Content -LiteralPath $checksumPath -Encoding UTF8

Compress-Archive -Path (Join-Path $packageDirectory '*') -DestinationPath $zipPath -CompressionLevel Optimal
$zipHash = (Get-FileHash -LiteralPath $zipPath -Algorithm SHA256).Hash.ToLowerInvariant()
"$zipHash  $packageName.zip" | Set-Content -LiteralPath (Join-Path $artifactRoot "$packageName.zip.sha256") -Encoding ASCII

Write-Host "Portable package: $zipPath"
Write-Host "SHA-256: $zipHash"
