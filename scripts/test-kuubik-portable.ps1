[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string]$PackageDirectory,
    [string]$RepositoryRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

function Require-Path([string]$Path) {
    if (-not (Test-Path -LiteralPath $Path)) {
        throw "Required portable file is missing: $Path"
    }
}

function Run-Native([string]$Executable, [string[]]$Arguments, [string]$Label) {
    & $Executable @Arguments
    if ($LASTEXITCODE -ne 0) {
        throw "$Label failed with exit code $LASTEXITCODE"
    }
}

$package = (Resolve-Path -LiteralPath $PackageDirectory).Path
$exe = Join-Path $package 'KuubikDraw.exe'
Require-Path $exe
Require-Path (Join-Path $package 'platforms\qwindows.dll')
Require-Path (Join-Path $package 'imageformats\qsvg.dll')
Require-Path (Join-Path $package 'resources\fonts')
Require-Path (Join-Path $package 'resources\patterns')
Require-Path (Join-Path $package 'resources\library')
Require-Path (Join-Path $package 'resources\qm\librecad_en.qm')
Require-Path (Join-Path $package 'LICENSE')
Require-Path (Join-Path $package 'FORK_NOTICE.md')
Require-Path (Join-Path $package 'SHA256SUMS.txt')
Require-Path (Join-Path $package 'build-manifest.json')

$forbidden = Get-ChildItem -LiteralPath $package -Recurse -File | Where-Object {
    $_.Extension.ToLowerInvariant() -in @('.pdb', '.ilk', '.obj', '.exp', '.lib', '.idb', '.tlog') -or
    $_.Name.ToLowerInvariant() -in @('node.exe', 'python.exe', 'python3.exe', 'uninstall.exe')
}
if ($forbidden) {
    throw "Forbidden build/install files found: $($forbidden.FullName -join ', ')"
}

$manifest = Get-Content -LiteralPath (Join-Path $package 'build-manifest.json') -Raw | ConvertFrom-Json
if ($manifest.executable -ne 'KuubikDraw.exe' -or $manifest.upstreamCommit -ne '7ebab007d9eb4c68609388b835a2487648f0877b') {
    throw 'Build manifest product or upstream provenance is incorrect.'
}

$tempRoot = if ([string]::IsNullOrWhiteSpace($env:RUNNER_TEMP)) {
    [IO.Path]::GetTempPath()
} else {
    $env:RUNNER_TEMP
}
$smokeRoot = Join-Path $tempRoot 'Kuubik Draw portable smoke'
if (Test-Path -LiteralPath $smokeRoot) {
    Remove-Item -LiteralPath $smokeRoot -Recurse -Force
}
New-Item -ItemType Directory -Path $smokeRoot | Out-Null
$portableCopy = Join-Path $smokeRoot 'Program Files'
Copy-Item -LiteralPath $package -Destination $portableCopy -Recurse -Force
$portableExe = Join-Path $portableCopy 'KuubikDraw.exe'

$fixture = Join-Path $smokeRoot 'preview-smoke.dxf'
Copy-Item -LiteralPath (Join-Path $RepositoryRoot 'tests\fixtures\preview-smoke.dxf') -Destination $fixture -Force
$pdf = Join-Path $smokeRoot 'preview-smoke.pdf'
Run-Native -Executable $portableExe -Arguments @('dxf2pdf', '--fit', '--paper', '210x297', '--outfile', $pdf, $fixture) -Label 'DXF to PDF smoke'
Run-Native -Executable $portableExe -Arguments @('dxf2svg', '--outfile', 'preview-smoke.svg', $fixture) -Label 'DXF to SVG smoke'

Require-Path $pdf
Require-Path (Join-Path $smokeRoot 'preview-smoke.svg')
if ((Get-Item -LiteralPath $pdf).Length -lt 1000) {
    throw 'Generated PDF is unexpectedly small.'
}

$env:QT_QPA_PLATFORM = 'offscreen'
$guiProcess = Start-Process -FilePath $portableExe -PassThru
Start-Sleep -Seconds 8
if ($guiProcess.HasExited) {
    throw "GUI startup smoke exited early with code $($guiProcess.ExitCode)"
}
Stop-Process -Id $guiProcess.Id -Force
Remove-Item Env:QT_QPA_PLATFORM

Write-Host "Portable native smoke passed from a path containing spaces: $portableCopy"
Write-Host "PDF: $pdf"
Write-Host "SVG: $(Join-Path $smokeRoot 'preview-smoke.svg')"
