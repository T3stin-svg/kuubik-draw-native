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

function Run-Native(
    [string]$Executable,
    [string[]]$Arguments,
    [string]$Label,
    [hashtable]$Environment = @{}
) {
    $startInfo = [Diagnostics.ProcessStartInfo]::new()
    $startInfo.FileName = $Executable
    $startInfo.UseShellExecute = $false
    foreach ($entry in $Environment.GetEnumerator()) {
        $startInfo.Environment[$entry.Key] = $entry.Value
    }
    foreach ($argument in $Arguments) {
        $startInfo.ArgumentList.Add($argument)
    }
    $process = [Diagnostics.Process]::Start($startInfo)
    if ($null -eq $process) {
        throw "$Label failed to start."
    }
    $process.WaitForExit()
    if ($process.ExitCode -ne 0) {
        throw "$Label failed with exit code $($process.ExitCode)"
    }
}

$package = (Resolve-Path -LiteralPath $PackageDirectory).Path
$exe = Join-Path $package 'KuubikDraw.exe'
Require-Path $exe
Require-Path (Join-Path $package 'platforms\qwindows.dll')
Require-Path (Join-Path $package 'platforms\qoffscreen.dll')
Require-Path (Join-Path $package 'imageformats\qsvg.dll')
Require-Path (Join-Path $package 'msvcp140.dll')
Require-Path (Join-Path $package 'vcruntime140.dll')
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
    $_.Name.ToLowerInvariant() -in @('node.exe', 'python.exe', 'python3.exe', 'uninstall.exe', 'vc_redist.x64.exe', 'vc_redist.x86.exe')
}
if ($forbidden) {
    throw "Forbidden build/install files found: $($forbidden.FullName -join ', ')"
}

$manifest = Get-Content -LiteralPath (Join-Path $package 'build-manifest.json') -Raw | ConvertFrom-Json
if ($manifest.executable -ne 'KuubikDraw.exe' -or
    $manifest.version -ne '0.2.0-preview.2' -or
    $manifest.upstreamCommit -ne '7ebab007d9eb4c68609388b835a2487648f0877b') {
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
$portableQtEnvironment = @{
    QT_PLUGIN_PATH = $portableCopy
    QT_QPA_PLATFORM_PLUGIN_PATH = (Join-Path $portableCopy 'platforms')
}
$offscreenEnvironment = @{
    QT_PLUGIN_PATH = $portableCopy
    QT_QPA_PLATFORM_PLUGIN_PATH = (Join-Path $portableCopy 'platforms')
    QT_QPA_PLATFORM = 'offscreen'
}

$fixture = Join-Path $smokeRoot 'preview-smoke.dxf'
Copy-Item -LiteralPath (Join-Path $RepositoryRoot 'tests\fixtures\preview-smoke.dxf') -Destination $fixture -Force
$pdf = Join-Path $smokeRoot 'preview-smoke.pdf'
Run-Native -Executable $portableExe -Arguments @('dxf2pdf', '--fit', '--paper', '210x297', '--outfile', $pdf, $fixture) -Label 'DXF to PDF smoke' -Environment $portableQtEnvironment
Run-Native -Executable $portableExe -Arguments @('dxf2svg', '--outfile', 'preview-smoke.svg', $fixture) -Label 'DXF to SVG smoke' -Environment $portableQtEnvironment

Require-Path $pdf
Require-Path (Join-Path $smokeRoot 'preview-smoke.svg')
if ((Get-Item -LiteralPath $pdf).Length -lt 1000) {
    throw 'Generated PDF is unexpectedly small.'
}

$uiContractPath = Join-Path $smokeRoot 'kuubik-ui-contract.json'
$env:KUUBIK_UI_CONTRACT_PATH = $uiContractPath
Run-Native -Executable $portableExe -Arguments @() -Label 'Kuubik UI contract smoke' -Environment $offscreenEnvironment
Remove-Item Env:KUUBIK_UI_CONTRACT_PATH
Require-Path $uiContractPath

$uiContract = Get-Content -LiteralPath $uiContractPath -Raw | ConvertFrom-Json
if ($uiContract.product -ne 'Kuubik Draw' -or
    $uiContract.version -ne '0.2.0-preview.2' -or
    $uiContract.workspaceMode -ne 'kuubik' -or
    $uiContract.workspaceVersion -ne 1 -or
    $uiContract.theme -ne 'kuubik-dark' -or
    $uiContract.paletteSide -ne 'right' -or
    -not $uiContract.ribbonVisible) {
    throw 'Kuubik UI contract identity, theme, ribbon, or workspace state is incorrect.'
}
if (@($uiContract.missingActionKeys).Count -ne 0 -or
    @($uiContract.bindingMismatches).Count -ne 0 -or
    @($uiContract.boundActionKeys).Count -lt 40) {
    throw 'Ribbon QAction binding contract is incomplete or invalid.'
}
$visibleToolbarNames = @($uiContract.visibleToolbars)
if ('kuubikRibbonToolbar' -notin $visibleToolbarNames) {
    throw 'Kuubik ribbon toolbar is not visible in the native UI contract.'
}
$layerDock = @($uiContract.docks | Where-Object objectName -eq 'layer_dockwidget')
$blockDock = @($uiContract.docks | Where-Object objectName -eq 'block_dockwidget')
$commandDock = @($uiContract.docks | Where-Object objectName -eq 'command_dockwidget')
if ($layerDock.Count -ne 1 -or $layerDock[0].area -ne 'right' -or
    $blockDock.Count -ne 1 -or $blockDock[0].area -ne 'right' -or
    $commandDock.Count -ne 1 -or $commandDock[0].area -ne 'bottom') {
    throw 'Primary dock layout does not match the Kuubik workspace contract.'
}

$guiSmokeDirectory = Join-Path $smokeRoot 'gui-evidence'
New-Item -ItemType Directory -Path $guiSmokeDirectory | Out-Null
$env:KUUBIK_GUI_SMOKE_DIR = $guiSmokeDirectory
Run-Native -Executable $portableExe -Arguments @() -Label 'Ribbon LINE mouse workflow smoke' -Environment $offscreenEnvironment
Remove-Item Env:KUUBIK_GUI_SMOKE_DIR

$guiSmokeReportPath = Join-Path $guiSmokeDirectory 'line-gui-smoke.json'
$guiActiveImagePath = Join-Path $guiSmokeDirectory 'line-active.png'
$guiCommittedImagePath = Join-Path $guiSmokeDirectory 'line-committed.png'
$guiDxfPath = Join-Path $guiSmokeDirectory 'line-gui-smoke.dxf'
Require-Path $guiSmokeReportPath
Require-Path $guiActiveImagePath
Require-Path $guiCommittedImagePath
Require-Path $guiDxfPath

$guiSmoke = Get-Content -LiteralPath $guiSmokeReportPath -Raw | ConvertFrom-Json
if ($guiSmoke.status -ne 'PASS' -or
    -not $guiSmoke.prerequisites -or
    $guiSmoke.ribbonActionKey -ne 'DrawLine' -or
    -not $guiSmoke.ribbonMouseEvent -or
    -not $guiSmoke.actionActiveAfterRibbon -or
    $guiSmoke.windowWidth -ne 1920 -or
    $guiSmoke.windowHeight -ne 1080 -or
    $guiSmoke.entitiesAfterFirstClick -ne $guiSmoke.entitiesBefore -or
    $guiSmoke.entitiesAfterSecondClick -ne ($guiSmoke.entitiesBefore + 1) -or
    $guiSmoke.linesAfterSecondClick -ne ($guiSmoke.linesBefore + 1) -or
    -not $guiSmoke.dxfSaved) {
    throw 'Ribbon LINE mouse workflow did not create exactly one native LINE.'
}
if ((Get-Item -LiteralPath $guiActiveImagePath).Length -lt 10000 -or
    (Get-Item -LiteralPath $guiCommittedImagePath).Length -lt 10000 -or
    (Get-Item -LiteralPath $guiDxfPath).Length -lt 500) {
    throw 'Ribbon LINE evidence output is unexpectedly small.'
}

$guiProcess = Start-Process -FilePath $portableExe -PassThru
Start-Sleep -Seconds 8
if ($guiProcess.HasExited) {
    throw "GUI startup smoke exited early with code $($guiProcess.ExitCode)"
}
Stop-Process -Id $guiProcess.Id -Force

Write-Host "Portable native smoke passed from a path containing spaces: $portableCopy"
Write-Host "PDF: $pdf"
Write-Host "SVG: $(Join-Path $smokeRoot 'preview-smoke.svg')"
Write-Host "UI contract: $uiContractPath"
Write-Host "Ribbon LINE GUI evidence: $guiSmokeDirectory"
