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

function Require-JsonProperty([object]$Object, [string]$Name, [string]$Context) {
    if ($null -eq $Object -or $null -eq $Object.PSObject.Properties[$Name]) {
        throw "Required producer field is missing: $Context.$Name"
    }
    return $Object.$Name
}

function Require-JsonBoolean([object]$Object, [string]$Name, [string]$Context) {
    $value = Require-JsonProperty $Object $Name $Context
    if ($value -isnot [bool]) {
        throw "Producer field must be boolean: $Context.$Name"
    }
    return $value
}

function Require-JsonString([object]$Object, [string]$Name, [string]$Context) {
    $value = Require-JsonProperty $Object $Name $Context
    if ($value -isnot [string] -or [string]::IsNullOrWhiteSpace($value)) {
        throw "Producer field must be a non-empty string: $Context.$Name"
    }
    return $value
}

function Require-JsonNumber([object]$Object, [string]$Name, [string]$Context) {
    $value = Require-JsonProperty $Object $Name $Context
    if ($value -isnot [byte] -and $value -isnot [int16] -and $value -isnot [int32] -and
        $value -isnot [int64] -and $value -isnot [single] -and $value -isnot [double] -and
        $value -isnot [decimal]) {
        throw "Producer field must be numeric: $Context.$Name"
    }
    return $value
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
    foreach ($name in @(
        'QT_SCALE_FACTOR', 'QT_SCREEN_SCALE_FACTORS', 'QT_AUTO_SCREEN_SCALE_FACTOR',
        'QT_DEVICE_PIXEL_RATIO', 'QT_FONT_DPI', 'QT_USE_PHYSICAL_DPI',
        'QT_SCALE_FACTOR_ROUNDING_POLICY'
    )) {
        [void]$startInfo.Environment.Remove($name)
    }
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
foreach ($key in @(
    'schemaVersion', 'product', 'version', 'workspaceMode', 'workspaceVersion',
    'theme', 'paletteSide', 'ribbonVisible', 'boundActionKeys', 'missingActionKeys',
    'bindings', 'bindingMismatches', 'visibleToolbars', 'docks'
)) {
    [void](Require-JsonProperty $uiContract $key 'uiContract')
}
if ($uiContract.schemaVersion -ne 2 -or
    $uiContract.product -ne 'Kuubik Draw' -or
    $uiContract.version -ne '0.2.0-preview.2' -or
    $uiContract.workspaceMode -ne 'kuubik' -or
    $uiContract.workspaceVersion -ne 2 -or
    $uiContract.theme -ne 'kuubik-dark' -or
    $uiContract.paletteSide -ne 'right' -or
    -not (Require-JsonBoolean $uiContract 'ribbonVisible' 'uiContract')) {
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

if ((Require-JsonBoolean $uiContract 'menuBarVisible' 'uiContract') -or
    -not (Require-JsonBoolean $uiContract 'classicMenuBarVisible' 'uiContract')) {
    throw 'Menu bar must be hidden in Kuubik workspace and visible in Classic workspace.'
}

$layerSelector = Require-JsonProperty $uiContract 'kuubikCurrentLayerSelector' 'uiContract'
if (-not (Require-JsonBoolean $layerSelector 'present' 'uiContract.kuubikCurrentLayerSelector') -or
    -not (Require-JsonBoolean $layerSelector 'enabled' 'uiContract.kuubikCurrentLayerSelector') -or
    -not (Require-JsonBoolean $layerSelector 'embeddedInRibbon' 'uiContract.kuubikCurrentLayerSelector') -or
    (Require-JsonNumber $layerSelector 'width' 'uiContract.kuubikCurrentLayerSelector') -lt 100) {
    throw 'Kuubik current-layer selector is absent or disabled.'
}
$selectorCurrentLayer = Require-JsonString $layerSelector 'currentLayer' 'uiContract.kuubikCurrentLayerSelector'
$selectorNativeLayer = Require-JsonString $layerSelector 'nativeCurrentLayer' 'uiContract.kuubikCurrentLayerSelector'
if ($selectorCurrentLayer -ne $selectorNativeLayer) {
    throw 'Kuubik current-layer selector does not match the native current layer.'
}

$propertiesDock = Require-JsonProperty $uiContract 'kuubikPropertiesDock' 'uiContract'
if (-not (Require-JsonBoolean $propertiesDock 'present' 'uiContract.kuubikPropertiesDock') -or
    (Require-JsonString $propertiesDock 'area' 'uiContract.kuubikPropertiesDock') -ne 'right' -or
    -not (Require-JsonBoolean $propertiesDock 'modifyEntityNativeBinding' 'uiContract.kuubikPropertiesDock') -or
    (Require-JsonNumber $propertiesDock 'width' 'uiContract.kuubikPropertiesDock') -lt 280) {
    throw 'Kuubik Properties dock is absent or not on the right.'
}
$propertiesTabGroup = @(Require-JsonProperty $propertiesDock 'tabGroupObjectNames' 'uiContract.kuubikPropertiesDock')
if ($propertiesTabGroup.Count -lt 3 -or
    'kuubikPropertiesDock' -notin $propertiesTabGroup -or
    'layer_dockwidget' -notin $propertiesTabGroup -or
    'block_dockwidget' -notin $propertiesTabGroup -or
    (Require-JsonString $propertiesDock 'activeTabObjectName' 'uiContract.kuubikPropertiesDock') -ne 'kuubikPropertiesDock') {
    throw 'Properties, Layers, and Blocks must share a right-side tab group with Properties selected by default.'
}

$ribbonPanels = @(Require-JsonProperty $uiContract 'ribbonPanels' 'uiContract')
if ($ribbonPanels.Count -eq 0) {
    throw 'UI contract has no ribbon panel collapse/action records.'
}
foreach ($panel in $ribbonPanels) {
    $panelContext = 'uiContract.ribbonPanels'
    [void](Require-JsonString $panel 'tab' $panelContext)
    [void](Require-JsonString $panel 'title' $panelContext)
    [void](Require-JsonBoolean $panel 'collapsed' $panelContext)
    if (@(Require-JsonProperty $panel 'actionKeys' $panelContext).Count -eq 0 -or
        -not (Require-JsonBoolean $panel 'actionIdentityValid' $panelContext)) {
        throw "Ribbon panel action identity is incomplete or invalid: $($panel.tab)/$($panel.title)"
    }
}
$panelKeys = @($ribbonPanels | ForEach-Object { "$($_.tab)/$($_.title)" })
foreach ($expectedPanel in @(
    'Home/Draw', 'Home/Modify', 'Home/Annotation', 'Home/Layers', 'Home/Block',
    'Home/Properties', 'Insert/Blocks', 'Annotate/Text', 'Annotate/Dimensions',
    'Annotate/Lines', 'Annotate/Cuts & Details', 'View/Navigate', 'View/Display',
    'View/Snaps', 'Manage/Layers', 'Manage/Blocks', 'Manage/Properties',
    'Output/File', 'Output/Plot'
)) {
    if ($expectedPanel -notin $panelKeys) {
        throw "Required ribbon panel is missing: $expectedPanel"
    }
}

$classicSampleAction = Require-JsonProperty $uiContract 'classicSampleAction' 'uiContract'
if ((Require-JsonString $classicSampleAction 'key' 'uiContract.classicSampleAction') -ne 'DrawLineBisector' -or
    -not (Require-JsonBoolean $classicSampleAction 'presentInMenu' 'uiContract.classicSampleAction')) {
    throw 'Classic workspace does not retain the sampled inherited LibreCAD menu command.'
}

$commandLine = Require-JsonProperty $uiContract 'commandLine' 'uiContract'
if (-not (Require-JsonBoolean $commandLine 'present' 'uiContract.commandLine') -or
    -not (Require-JsonBoolean $commandLine 'nativeBinding' 'uiContract.commandLine')) {
    throw 'Command line is absent or not bound to the native command path.'
}
$statusControls = @(Require-JsonProperty $uiContract 'statusControls' 'uiContract')
if ($statusControls.Count -eq 0) {
    throw 'UI contract has no native status-control bindings.'
}
foreach ($control in $statusControls) {
    $controlContext = 'uiContract.statusControls'
    [void](Require-JsonString $control 'objectName' $controlContext)
    [void](Require-JsonString $control 'actionKey' $controlContext)
    if (-not (Require-JsonBoolean $control 'nativeBinding' $controlContext)) {
        throw "Status control is not bound to its native QAction: $($control.objectName)"
    }
}
$expectedStatusActions = @(
    'ViewGrid', 'RestrictOrthogonal', 'SnapEnd', 'SnapMiddle', 'SnapCenter', 'SnapIntersection'
)
$actualStatusActions = @($statusControls | ForEach-Object actionKey | Sort-Object -Unique)
if ($actualStatusActions.Count -ne $expectedStatusActions.Count) {
    throw 'Kuubik status controls do not expose the complete native action set.'
}
foreach ($expectedAction in $expectedStatusActions) {
    if ($expectedAction -notin $actualStatusActions) {
        throw "Kuubik status control is missing: $expectedAction"
    }
}

$dpi = Require-JsonProperty $uiContract 'dpi' 'uiContract'
foreach ($key in @('logicalDpiX', 'logicalDpiY', 'devicePixelRatio', 'windowLogicalWidth', 'windowLogicalHeight')) {
    $value = Require-JsonNumber $dpi $key 'uiContract.dpi'
    if ($value -le 0) {
        throw "DPI metadata must be positive: uiContract.dpi.$key"
    }
}

$dpiEvidenceRoot = Join-Path $smokeRoot 'dpi-evidence'
New-Item -ItemType Directory -Path $dpiEvidenceRoot | Out-Null
foreach ($dpiCase in @(
    @{ Name = '100'; Factor = '1.00'; Expected = 1.0 },
    @{ Name = '125'; Factor = '1.25'; Expected = 1.25 },
    @{ Name = '150'; Factor = '1.50'; Expected = 1.5 }
)) {
    $dpiDirectory = Join-Path $dpiEvidenceRoot $dpiCase.Name
    New-Item -ItemType Directory -Path $dpiDirectory | Out-Null
    $dpiContractPath = Join-Path $dpiDirectory 'kuubik-ui-contract.json'
    $dpiScreenshotPath = Join-Path $dpiDirectory 'workspace.png'
    $dpiEnvironment = $portableQtEnvironment.Clone()
    $dpiEnvironment['QT_QPA_PLATFORM'] = 'windows'
    $dpiEnvironment['QT_ENABLE_HIGHDPI_SCALING'] = '1'
    $dpiEnvironment['QT_SCALE_FACTOR'] = $dpiCase.Factor
    $dpiEnvironment['KUUBIK_UI_CONTRACT_PATH'] = $dpiContractPath
    $dpiEnvironment['KUUBIK_UI_SCREENSHOT_PATH'] = $dpiScreenshotPath
    $dpiEnvironment['KUUBIK_UI_CONTRACT_WIDTH'] = '1280'
    $dpiEnvironment['KUUBIK_UI_CONTRACT_HEIGHT'] = '800'
    Run-Native -Executable $portableExe -Arguments @() -Label "Kuubik $($dpiCase.Name)% DPI contract" -Environment $dpiEnvironment
    Require-Path $dpiContractPath
    Require-Path $dpiScreenshotPath

    $dpiContract = Get-Content -LiteralPath $dpiContractPath -Raw | ConvertFrom-Json
    $dpiState = Require-JsonProperty $dpiContract 'dpi' "dpiContract.$($dpiCase.Name)"
    $actualScale = Require-JsonNumber $dpiState 'devicePixelRatio' "dpiContract.$($dpiCase.Name).dpi"
    $requestedScale = Require-JsonNumber $dpiState 'requestedScaleFactor' "dpiContract.$($dpiCase.Name).dpi"
    $screenshotPixelWidth = Require-JsonNumber $dpiState 'screenshotPixelWidth' "dpiContract.$($dpiCase.Name).dpi"
    $screenshotPixelHeight = Require-JsonNumber $dpiState 'screenshotPixelHeight' "dpiContract.$($dpiCase.Name).dpi"
    if ([Math]::Abs($actualScale - $dpiCase.Expected) -gt 0.06 -or
        [Math]::Abs($requestedScale - $dpiCase.Expected) -gt 0.001 -or
        (Require-JsonNumber $dpiState 'windowLogicalWidth' "dpiContract.$($dpiCase.Name).dpi") -ne 1280 -or
        (Require-JsonNumber $dpiState 'windowLogicalHeight' "dpiContract.$($dpiCase.Name).dpi") -ne 800 -or
        [Math]::Abs($screenshotPixelWidth - (1280 * $dpiCase.Expected)) -gt 2 -or
        [Math]::Abs($screenshotPixelHeight - (800 * $dpiCase.Expected)) -gt 2 -or
        -not (Require-JsonBoolean $dpiState 'screenshotSaved' "dpiContract.$($dpiCase.Name).dpi") -or
        (Get-Item -LiteralPath $dpiScreenshotPath).Length -lt 10000) {
        throw "Kuubik $($dpiCase.Name)% DPI evidence is incomplete or uses the wrong scale."
    }
}

$guiSmokeDirectory = Join-Path $smokeRoot 'gui-evidence'
New-Item -ItemType Directory -Path $guiSmokeDirectory | Out-Null
$guiSmokeEnvironment = $offscreenEnvironment.Clone()
$guiSmokeEnvironment['KUUBIK_GUI_SMOKE_DIR'] = $guiSmokeDirectory
$guiSmokeEnvironment['KUUBIK_GUI_SMOKE_INPUT_DXF'] = $fixture
Run-Native -Executable $portableExe -Arguments @() -Label 'Ribbon LINE mouse workflow smoke' -Environment $guiSmokeEnvironment

$guiSmokeReportPath = Join-Path $guiSmokeDirectory 'line-gui-smoke.json'
$guiActiveImagePath = Join-Path $guiSmokeDirectory 'line-active.png'
$guiCommittedImagePath = Join-Path $guiSmokeDirectory 'line-committed.png'
$guiDxfPath = Join-Path $guiSmokeDirectory 'line-gui-smoke.dxf'
Require-Path $guiSmokeReportPath
Require-Path $guiActiveImagePath
Require-Path $guiCommittedImagePath
Require-Path $guiDxfPath

$guiSmoke = Get-Content -LiteralPath $guiSmokeReportPath -Raw | ConvertFrom-Json
foreach ($key in @(
    'schemaVersion', 'status', 'prerequisites', 'ribbonActionKey', 'ribbonMouseEvent',
    'actionActiveAfterRibbon', 'windowWidth', 'windowHeight', 'entitiesBefore',
    'entitiesAfterFirstClick', 'entitiesAfterSecondClick', 'linesBefore',
    'linesAfterSecondClick', 'dxfSaved', 'sourceDxfLoaded', 'documentLifecycle',
    'fullPropertiesAction'
)) {
    [void](Require-JsonProperty $guiSmoke $key 'guiSmoke')
}
if ($guiSmoke.schemaVersion -ne 2 -or
    $guiSmoke.status -ne 'PASS' -or
    -not $guiSmoke.prerequisites -or
    $guiSmoke.ribbonActionKey -ne 'DrawLine' -or
    -not $guiSmoke.ribbonMouseEvent -or
    -not $guiSmoke.actionActiveAfterRibbon -or
    -not $guiSmoke.sourceDxfLoaded -or
    $guiSmoke.windowWidth -ne 1920 -or
    $guiSmoke.windowHeight -ne 1080 -or
    $guiSmoke.entitiesAfterFirstClick -ne $guiSmoke.entitiesBefore -or
    $guiSmoke.entitiesAfterSecondClick -ne ($guiSmoke.entitiesBefore + 1) -or
    $guiSmoke.linesAfterSecondClick -ne ($guiSmoke.linesBefore + 1) -or
    -not $guiSmoke.dxfSaved) {
    throw 'Ribbon LINE mouse workflow did not create exactly one native LINE.'
}
$guiLayerSelector = Require-JsonProperty $guiSmoke 'layerSelector' 'guiSmoke'
if (-not (Require-JsonBoolean $guiLayerSelector 'present' 'guiSmoke.layerSelector') -or
    -not (Require-JsonBoolean $guiLayerSelector 'enabled' 'guiSmoke.layerSelector')) {
    throw 'GUI smoke did not use an enabled native current-layer selector.'
}
$selectedLayer = Require-JsonString $guiLayerSelector 'selectedLayer' 'guiSmoke.layerSelector'
$nativeCurrentLayer = Require-JsonString $guiLayerSelector 'nativeCurrentLayer' 'guiSmoke.layerSelector'
$createdLineLayer = Require-JsonString $guiLayerSelector 'createdLineLayer' 'guiSmoke.layerSelector'
if ($selectedLayer -ne $nativeCurrentLayer -or $selectedLayer -ne $createdLineLayer) {
    throw 'Layer selector, native document, and created LINE layer do not match.'
}

$propertiesStates = Require-JsonProperty $guiSmoke 'propertiesStates' 'guiSmoke'
foreach ($expected in @(
    @{ Name = 'document'; Mode = 'document'; Count = 0 },
    @{ Name = 'single'; Mode = 'single'; Count = 1 },
    @{ Name = 'multiple'; Mode = 'multiple'; Count = 2 }
)) {
    $stateContext = "guiSmoke.propertiesStates.$($expected.Name)"
    $state = Require-JsonProperty $propertiesStates $expected.Name 'guiSmoke.propertiesStates'
    if (-not (Require-JsonBoolean $state 'nativeCallback' $stateContext) -or
        (Require-JsonString $state 'mode' $stateContext) -ne $expected.Mode -or
        (Require-JsonNumber $state 'count' $stateContext) -lt $expected.Count -or
        $null -eq (Require-JsonProperty $state 'summary' $stateContext)) {
        throw "Properties smoke state is incomplete: $stateContext"
    }
    if ($expected.Name -eq 'document' -and (Require-JsonNumber $state 'count' $stateContext) -ne 0) {
        throw 'Properties document state must have zero selected entities.'
    }
    if ($expected.Name -eq 'single') {
        $summary = Require-JsonProperty $state 'summary' $stateContext
        [void](Require-JsonString $summary 'type' "$stateContext.summary")
        [void](Require-JsonString $summary 'layer' "$stateContext.summary")
    }
}
$documentLifecycle = Require-JsonProperty $guiSmoke 'documentLifecycle' 'guiSmoke'
foreach ($key in @(
    'reopenedNativeDxf', 'originalRestoredAfterSwitch',
    'originalRestoredAfterClose', 'passed'
)) {
    if (-not (Require-JsonBoolean $documentLifecycle $key 'guiSmoke.documentLifecycle')) {
        throw "Native DXF reopen or multi-document lifecycle failed: $key"
    }
}
$fullPropertiesAction = Require-JsonProperty $guiSmoke 'fullPropertiesAction' 'guiSmoke'
if ((Require-JsonString $fullPropertiesAction 'actionKey' 'guiSmoke.fullPropertiesAction') -ne 'ModifyEntity' -or
    -not (Require-JsonBoolean $fullPropertiesAction 'nativeIdentity' 'guiSmoke.fullPropertiesAction') -or
    -not (Require-JsonBoolean $fullPropertiesAction 'nativeActionActive' 'guiSmoke.fullPropertiesAction')) {
    throw 'Open Full Properties did not activate the native ModifyEntity action.'
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
