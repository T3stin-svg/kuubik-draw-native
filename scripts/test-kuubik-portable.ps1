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

function Assert-RibbonMouseInvocation([object]$Invocation, [string]$Context) {
    if (-not (Require-JsonBoolean $Invocation 'passed' $Context) -or
        -not (Require-JsonBoolean $Invocation 'actionTriggeredByMouse' $Context) -or
        -not (Require-JsonBoolean $Invocation 'sourceButtonEnabled' $Context) -or
        -not (Require-JsonBoolean $Invocation 'sourceButtonIdentity' $Context)) {
        throw "Ribbon invocation did not trigger the expected native QAction: $Context"
    }

    $surface = Require-JsonString $Invocation 'invocationSurface' $Context
    if ($surface -eq 'directButton') {
        if (-not (Require-JsonBoolean $Invocation 'sourceButtonVisible' $Context) -or
            (Require-JsonBoolean $Invocation 'panelCollapsed' $Context)) {
            throw "Direct ribbon invocation did not use a visible expanded button: $Context"
        }
    } elseif ($surface -eq 'collapsedPanelOverflow') {
        if ((Require-JsonBoolean $Invocation 'sourceButtonVisible' $Context) -or
            -not (Require-JsonBoolean $Invocation 'panelCollapsed' $Context) -or
            -not (Require-JsonBoolean $Invocation 'overflowButtonPresent' $Context) -or
            -not (Require-JsonBoolean $Invocation 'overflowButtonVisible' $Context) -or
            -not (Require-JsonBoolean $Invocation 'overflowButtonEnabled' $Context) -or
            -not (Require-JsonBoolean $Invocation 'overflowMenuPresent' $Context) -or
            -not (Require-JsonBoolean $Invocation 'overflowActionIdentity' $Context) -or
            -not (Require-JsonBoolean $Invocation 'overflowMenuInteractionRan' $Context) -or
            -not (Require-JsonBoolean $Invocation 'overflowMenuVisibleAfterOpen' $Context) -or
            -not (Require-JsonBoolean $Invocation 'overflowActionGeometryValid' $Context) -or
            -not (Require-JsonBoolean $Invocation 'overflowActionAtPoint' $Context) -or
            -not (Require-JsonBoolean $Invocation 'overflowMenuClosedAfterSelection' $Context)) {
            throw "Collapsed ribbon invocation did not use the visible native overflow menu row: $Context"
        }
    } else {
        throw "Unknown ribbon invocation surface '$surface': $Context"
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
$statusMenuScreenshotPath = Join-Path $smokeRoot 'status-bar-customization.png'
$env:KUUBIK_UI_CONTRACT_PATH = $uiContractPath
$env:KUUBIK_STATUS_MENU_SCREENSHOT_PATH = $statusMenuScreenshotPath
Run-Native -Executable $portableExe -Arguments @() -Label 'Kuubik UI contract smoke' -Environment $offscreenEnvironment
Remove-Item Env:KUUBIK_UI_CONTRACT_PATH
Remove-Item Env:KUUBIK_STATUS_MENU_SCREENSHOT_PATH
Require-Path $uiContractPath
Require-Path $statusMenuScreenshotPath

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
$activePropertiesTab = Require-JsonString $propertiesDock 'activeTabObjectName' 'uiContract.kuubikPropertiesDock'
if ($propertiesTabGroup.Count -lt 3 -or
    'kuubikPropertiesDock' -notin $propertiesTabGroup -or
    'layer_dockwidget' -notin $propertiesTabGroup -or
    'block_dockwidget' -notin $propertiesTabGroup -or
    $activePropertiesTab -ne 'kuubikPropertiesDock') {
    throw "Properties, Layers, and Blocks must share a right-side tab group with Properties selected by default. Active='$activePropertiesTab'; group='$($propertiesTabGroup -join ',')'."
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
    $bindingType = Require-JsonString $control 'bindingType' $controlContext
    if (-not (Require-JsonBoolean $control 'nativeBinding' $controlContext)) {
        throw "Status control is not bound to its native QAction: $($control.objectName)"
    }
    if ($bindingType -eq 'direct-action' -and
        -not (Require-JsonBoolean $control 'customIconOwned' $controlContext)) {
        throw "Native LibreCAD icon can replace the Kuubik status icon: $($control.actionKey)"
    }
}
$expectedStatusActions = @(
    'ViewGrid', 'SnapGrid', 'InferConstraints', 'DynamicInput',
    'RestrictOrthogonal', 'SnapAngle', 'IsometricDrafting', 'ObjectSnapMenu',
    'SnapTracking', 'ViewDraft', 'QuickProperties', 'Fullscreen'
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

$statusBarContract = Require-JsonProperty $uiContract 'statusBar' 'uiContract'
if (-not (Require-JsonBoolean $statusBarContract 'customizationButtonPresent' 'uiContract.statusBar') -or
    -not (Require-JsonBoolean $statusBarContract 'customizationMenuPresent' 'uiContract.statusBar')) {
    throw 'AutoCAD-familiar status-bar customization control is absent.'
}
if (-not (Require-JsonBoolean $statusBarContract 'customizationToggleRoundTrip' 'uiContract.statusBar')) {
    throw 'Status-bar customization did not hide and restore its native control.'
}
if ((Require-JsonNumber $statusBarContract 'customIconClickTests' 'uiContract.statusBar') -lt 3 -or
    -not (Require-JsonBoolean $statusBarContract 'customIconsStableAfterClick' 'uiContract.statusBar')) {
    throw 'A native LibreCAD icon replaced a Kuubik status icon after clicking it.'
}
if (-not (Require-JsonBoolean $statusBarContract 'coordinateDisplayPresent' 'uiContract.statusBar')) {
    throw 'Status bar has no native live coordinate display.'
}
if ((Require-JsonNumber $statusBarContract 'coordinateModeCount' 'uiContract.statusBar') -ne 4) {
    throw 'Status coordinate display does not expose all four supported formats.'
}
if ((Require-JsonNumber $statusBarContract 'osnapModeCount' 'uiContract.statusBar') -lt 14) {
    throw 'OSNAP status menu is missing native snap modes.'
}
if (-not (Require-JsonBoolean $statusBarContract 'sizeGripDisabled' 'uiContract.statusBar')) {
    throw 'Status bar retained a non-AutoCAD resize grip.'
}
$referenceFirstFive = Require-JsonProperty $statusBarContract 'referencePdfFirstFive' 'uiContract.statusBar'
if ((Require-JsonNumber $referenceFirstFive 'referencePageCount' 'uiContract.statusBar.referencePdfFirstFive') -ne 5) {
    throw 'The implemented reference batch is not limited to the approved first five PDF pages.'
}
foreach ($referenceFlag in @(
    'coordinatesVisibleByDefault', 'coordinateDisplaysZ',
    'coordinateModelGridOrdered', 'coordinateModelGridSingleRow',
    'classicCoordinateSlotRestored',
    'gridSettingsMenuPresent', 'gridStateSynchronized',
    'gridTooltipMatchesReference'
)) {
    if (-not (Require-JsonBoolean $referenceFirstFive $referenceFlag 'uiContract.statusBar.referencePdfFirstFive')) {
        throw "First-five-page status-bar contract failed: $referenceFlag"
    }
}
if ((Require-JsonString $referenceFirstFive 'modelIndicatorText' 'uiContract.statusBar.referencePdfFirstFive') -ne 'MODEL') {
    throw 'The first-five-page status cluster does not expose the MODEL indicator.'
}
$referenceCoordinateWidth = Require-JsonNumber $referenceFirstFive 'coordinateCompactWidth' 'uiContract.statusBar.referencePdfFirstFive'
if ($referenceCoordinateWidth -lt 176 -or $referenceCoordinateWidth -gt 190) {
    throw "Coordinate readout width diverges from the approved compact reference: $referenceCoordinateWidth"
}
if ((Require-JsonNumber $referenceFirstFive 'statusBarHeight' 'uiContract.statusBar.referencePdfFirstFive') -gt 29) {
    throw 'Status bar exceeds the compact single-row reference height.'
}
$referenceSixToEleven = Require-JsonProperty $statusBarContract 'referencePdfPagesSixToEleven' 'uiContract.statusBar'
if ((Require-JsonNumber $referenceSixToEleven 'referenceStartPage' 'uiContract.statusBar.referencePdfPagesSixToEleven') -ne 6 -or
    (Require-JsonNumber $referenceSixToEleven 'referenceEndPage' 'uiContract.statusBar.referencePdfPagesSixToEleven') -ne 11 -or
    (Require-JsonNumber $referenceSixToEleven 'referencePageCount' 'uiContract.statusBar.referencePdfPagesSixToEleven') -ne 6) {
    throw 'The second reference implementation batch must cover exactly PDF pages 6-11.'
}
foreach ($referenceFlag in @(
    'pageControlsVisible', 'pageControlsOrdered', 'snapSettingsOnRightClick',
    'snapF9Shortcut', 'inferenceUsesNativeSnapBundle',
    'inferenceIsHonestlyNonPersistent', 'dynamicF12Shortcut',
    'dynamicSettingsPresent', 'orthoNativeBinding', 'orthoF8Shortcut',
    'polarF10Shortcut', 'polarSnapEngineQuantizes',
    'isometricPlanesRoundTrip', 'isometricF5Shortcut',
    'isometricCtrlEShortcut'
)) {
    if (-not (Require-JsonBoolean $referenceSixToEleven $referenceFlag 'uiContract.statusBar.referencePdfPagesSixToEleven')) {
        throw "Pages 6-11 status-bar contract failed: $referenceFlag"
    }
}
if ((Require-JsonNumber $referenceSixToEleven 'snapModeChoiceCount' 'uiContract.statusBar.referencePdfPagesSixToEleven') -ne 2 -or
    (Require-JsonNumber $referenceSixToEleven 'polarPresetCount' 'uiContract.statusBar.referencePdfPagesSixToEleven') -lt 8 -or
    (Require-JsonNumber $referenceSixToEleven 'isoplaneCount' 'uiContract.statusBar.referencePdfPagesSixToEleven') -ne 3) {
    throw 'Pages 6-11 menus are missing Snap, Polar, or isoplane choices.'
}
$referenceTwelveToThirteen = Require-JsonProperty $statusBarContract 'referencePdfPagesTwelveToThirteen' 'uiContract.statusBar'
if ((Require-JsonNumber $referenceTwelveToThirteen 'referenceStartPage' 'uiContract.statusBar.referencePdfPagesTwelveToThirteen') -ne 12 -or
    (Require-JsonNumber $referenceTwelveToThirteen 'referenceEndPage' 'uiContract.statusBar.referencePdfPagesTwelveToThirteen') -ne 13 -or
    (Require-JsonNumber $referenceTwelveToThirteen 'referencePageCount' 'uiContract.statusBar.referencePdfPagesTwelveToThirteen') -ne 2) {
    throw 'The M1 precision reference batch must cover exactly PDF pages 12-13.'
}
foreach ($referenceFlag in @(
    'osnapControlVisible', 'trackingControlVisible', 'osnapStateRoundTrip',
    'osnapMouseKeyboardSynchronized', 'osnapPriorSetPersisted',
    'trackingStateRoundTrip', 'trackingMouseKeyboardSynchronized',
    'trackingSerializationRoundTrip', 'passed'
)) {
    if (-not (Require-JsonBoolean $referenceTwelveToThirteen $referenceFlag 'uiContract.statusBar.referencePdfPagesTwelveToThirteen')) {
        throw "Pages 12-13 status-bar contract failed: $referenceFlag"
    }
}
$snapFamilies = Require-JsonProperty $referenceTwelveToThirteen 'snapFamilies' 'uiContract.statusBar.referencePdfPagesTwelveToThirteen'
if (-not (Require-JsonBoolean $snapFamilies 'passed' 'uiContract.statusBar.referencePdfPagesTwelveToThirteen.snapFamilies')) {
    throw 'The native OSNAP geometry family matrix did not pass.'
}
foreach ($snapFamily in @(
    'endpoint', 'midpoint', 'center', 'quadrant', 'intersection',
    'perpendicular', 'tangent', 'nearest', 'extension', 'parallel'
)) {
    $familyResult = Require-JsonProperty $snapFamilies $snapFamily 'uiContract.statusBar.referencePdfPagesTwelveToThirteen.snapFamilies'
    $familyContext = "uiContract.statusBar.referencePdfPagesTwelveToThirteen.snapFamilies.$snapFamily"
    foreach ($coordinateField in @('actualX', 'actualY', 'expectedX', 'expectedY', 'error')) {
        [void](Require-JsonNumber $familyResult $coordinateField $familyContext)
    }
    if (-not (Require-JsonBoolean $familyResult 'passed' $familyContext) -or
        (Require-JsonNumber $familyResult 'error' $familyContext) -gt 0.000001) {
        throw "Native OSNAP geometry result is incorrect: $snapFamily"
    }
}
$trackingGeometry = Require-JsonProperty $referenceTwelveToThirteen 'trackingGeometry' 'uiContract.statusBar.referencePdfPagesTwelveToThirteen'
foreach ($trackingFlag in @(
    'candidateAcquired', 'orthogonalGuideVisible', 'polarGuideVisible',
    'objectSnapPrecedence', 'gridSnapPrecedence',
    'orthoRestrictionPrecedence', 'disabledClearsAcquisition',
    'documentEntitiesUnchanged', 'undoStateUnchanged', 'overlayOnly', 'passed'
)) {
    if (-not (Require-JsonBoolean $trackingGeometry $trackingFlag 'uiContract.statusBar.referencePdfPagesTwelveToThirteen.trackingGeometry')) {
        throw "Native object-snap tracking contract failed: $trackingFlag"
    }
}
foreach ($projectionName in @('orthogonalProjection', 'polarProjection')) {
    $projection = Require-JsonProperty $trackingGeometry $projectionName 'uiContract.statusBar.referencePdfPagesTwelveToThirteen.trackingGeometry'
    $projectionContext = "uiContract.statusBar.referencePdfPagesTwelveToThirteen.trackingGeometry.$projectionName"
    foreach ($coordinateField in @('actualX', 'actualY', 'expectedX', 'expectedY', 'error')) {
        [void](Require-JsonNumber $projection $coordinateField $projectionContext)
    }
    if (-not (Require-JsonBoolean $projection 'passed' $projectionContext)) {
        throw "Native object-snap tracking projection is incorrect: $projectionName"
    }
}
$trackingEntitiesBefore = Require-JsonNumber $trackingGeometry 'documentEntityCountBefore' 'uiContract.statusBar.referencePdfPagesTwelveToThirteen.trackingGeometry'
$trackingEntitiesAfter = Require-JsonNumber $trackingGeometry 'documentEntityCountAfter' 'uiContract.statusBar.referencePdfPagesTwelveToThirteen.trackingGeometry'
if ($trackingEntitiesBefore -ne $trackingEntitiesAfter) {
    throw 'Object-snap tracking inserted transient geometry into the native document.'
}
$trackingUndoBefore = Require-JsonNumber $trackingGeometry 'undoCycleCountBefore' 'uiContract.statusBar.referencePdfPagesTwelveToThirteen.trackingGeometry'
$trackingUndoAfter = Require-JsonNumber $trackingGeometry 'undoCycleCountAfter' 'uiContract.statusBar.referencePdfPagesTwelveToThirteen.trackingGeometry'
if ($trackingUndoBefore -ne $trackingUndoAfter) {
    throw 'Object-snap tracking inserted transient state into the native undo history.'
}
$pageThirtyThreeShortcuts = Require-JsonProperty $statusBarContract 'referencePdfPageThirtyThreeShortcuts' 'uiContract.statusBar'
foreach ($shortcutFlag in @(
    'f3Registered', 'f11Registered', 'fullscreenF11Removed',
    'f3UsesOsnapMainToggle', 'f11UsesTrackingToggle', 'passed'
)) {
    if (-not (Require-JsonBoolean $pageThirtyThreeShortcuts $shortcutFlag 'uiContract.statusBar.referencePdfPageThirtyThreeShortcuts')) {
        throw "Page 33 F3/F11 shortcut contract failed: $shortcutFlag"
    }
}
if (-not (Require-JsonBoolean $statusBarContract 'menuScreenshotSaved' 'uiContract.statusBar') -or
    (Require-JsonNumber $statusBarContract 'menuScreenshotWidth' 'uiContract.statusBar') -lt 180 -or
    (Require-JsonNumber $statusBarContract 'menuScreenshotHeight' 'uiContract.statusBar') -lt 250) {
    throw 'Status-bar customization menu did not render a usable evidence image.'
}
$customizationEntries = @(Require-JsonProperty $statusBarContract 'customizationEntries' 'uiContract.statusBar')
$expectedCustomizationKeys = @(
    'Coordinates', 'ModelSpace', 'Grid', 'SnapMode', 'InferConstraints',
    'DynamicInput', 'OrthoMode', 'SnapAngle', 'IsometricDrafting',
    'ObjectSnap', 'SnapTracking', 'Lineweight', 'QuickProperties', 'CleanScreen'
)
$actualCustomizationKeys = @($customizationEntries | ForEach-Object key | Sort-Object -Unique)
if ($actualCustomizationKeys.Count -ne $expectedCustomizationKeys.Count) {
    throw 'Status-bar customization menu does not expose the complete supported control set.'
}
foreach ($expectedKey in $expectedCustomizationKeys) {
    if ($expectedKey -notin $actualCustomizationKeys) {
        throw "Status-bar customization entry is missing: $expectedKey"
    }
}
foreach ($entry in $customizationEntries) {
    if (-not (Require-JsonBoolean $entry 'controlPresent' 'uiContract.statusBar.customizationEntries')) {
        throw "Status-bar customization entry has no matching control: $($entry.key)"
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
    @{ Name = '100'; Factor = '1.00'; Expected = 1.0; Width = 1280; Height = 600 },
    @{ Name = '125'; Factor = '1.25'; Expected = 1.25; Width = 1280; Height = 600 },
    @{ Name = '150'; Factor = '1.50'; Expected = 1.5; Width = 1200; Height = 600 }
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
    $dpiEnvironment['KUUBIK_UI_CONTRACT_WIDTH'] = [string]$dpiCase.Width
    $dpiEnvironment['KUUBIK_UI_CONTRACT_HEIGHT'] = [string]$dpiCase.Height
    Run-Native -Executable $portableExe -Arguments @() -Label "Kuubik $($dpiCase.Name)% DPI contract" -Environment $dpiEnvironment
    Require-Path $dpiContractPath
    Require-Path $dpiScreenshotPath

    $dpiContract = Get-Content -LiteralPath $dpiContractPath -Raw | ConvertFrom-Json
    $dpiState = Require-JsonProperty $dpiContract 'dpi' "dpiContract.$($dpiCase.Name)"
    $actualScale = Require-JsonNumber $dpiState 'devicePixelRatio' "dpiContract.$($dpiCase.Name).dpi"
    $requestedScale = Require-JsonNumber $dpiState 'requestedScaleFactor' "dpiContract.$($dpiCase.Name).dpi"
    $screenshotPixelWidth = Require-JsonNumber $dpiState 'screenshotPixelWidth' "dpiContract.$($dpiCase.Name).dpi"
    $screenshotPixelHeight = Require-JsonNumber $dpiState 'screenshotPixelHeight' "dpiContract.$($dpiCase.Name).dpi"
    $screenshotScale = Require-JsonNumber $dpiState 'screenshotDevicePixelRatio' "dpiContract.$($dpiCase.Name).dpi"
    $logicalWidth = Require-JsonNumber $dpiState 'windowLogicalWidth' "dpiContract.$($dpiCase.Name).dpi"
    $logicalHeight = Require-JsonNumber $dpiState 'windowLogicalHeight' "dpiContract.$($dpiCase.Name).dpi"
    $screenshotSaved = Require-JsonBoolean $dpiState 'screenshotSaved' "dpiContract.$($dpiCase.Name).dpi"
    $screenshotBytes = (Get-Item -LiteralPath $dpiScreenshotPath).Length
    Write-Output ("Kuubik {0}% DPI evidence: requested={1}; windowDpr={2}; screenshotDpr={3}; logical={4}x{5}; pixels={6}x{7}; saved={8}; bytes={9}" -f
        $dpiCase.Name, $requestedScale, $actualScale, $screenshotScale, $logicalWidth,
        $logicalHeight, $screenshotPixelWidth, $screenshotPixelHeight, $screenshotSaved,
        $screenshotBytes)
    if ([Math]::Abs($actualScale - $dpiCase.Expected) -gt 0.06 -or
        [Math]::Abs($requestedScale - $dpiCase.Expected) -gt 0.001 -or
        [Math]::Abs($screenshotScale - $dpiCase.Expected) -gt 0.06 -or
        $logicalWidth -ne $dpiCase.Width -or
        $logicalHeight -ne $dpiCase.Height -or
        [Math]::Abs($screenshotPixelWidth - ($dpiCase.Width * $dpiCase.Expected)) -gt 2 -or
        [Math]::Abs($screenshotPixelHeight - ($dpiCase.Height * $dpiCase.Expected)) -gt 2 -or
        -not $screenshotSaved -or
        $screenshotBytes -lt 10000) {
        throw "Kuubik $($dpiCase.Name)% DPI evidence is incomplete or uses the wrong scale; see the preceding measured values."
    }
}

$toolOptionsEvidenceRoot = Join-Path $smokeRoot 'tool-options-evidence'
New-Item -ItemType Directory -Path $toolOptionsEvidenceRoot | Out-Null
$toolOptionsEnvironment = $portableQtEnvironment.Clone()
$toolOptionsEnvironment['QT_QPA_PLATFORM'] = 'windows'
$toolOptionsEnvironment['QT_ENABLE_HIGHDPI_SCALING'] = '1'
$toolOptionsEnvironment['QT_SCALE_FACTOR'] = '1.00'
$toolOptionsEnvironment['KUUBIK_TOOL_OPTIONS_SMOKE_DIR'] = $toolOptionsEvidenceRoot
Run-Native -Executable $portableExe -Arguments @() -Label 'Native Tool Options 1280 smoke' -Environment $toolOptionsEnvironment

$toolOptionsReportPath = Join-Path $toolOptionsEvidenceRoot 'tool-options-1280.json'
Require-Path $toolOptionsReportPath
$toolOptionsReport = Get-Content -LiteralPath $toolOptionsReportPath -Raw | ConvertFrom-Json
if ((Require-JsonNumber $toolOptionsReport 'schemaVersion' 'toolOptionsReport') -ne 1 -or
    (Require-JsonString $toolOptionsReport 'status' 'toolOptionsReport') -ne 'PASS' -or
    (Require-JsonString $toolOptionsReport 'platform' 'toolOptionsReport') -ne 'windows' -or
    (Require-JsonNumber $toolOptionsReport 'windowWidth' 'toolOptionsReport') -ne 1280 -or
    (Require-JsonNumber $toolOptionsReport 'windowHeight' 'toolOptionsReport') -ne 600 -or
    [Math]::Abs((Require-JsonNumber $toolOptionsReport 'devicePixelRatio' 'toolOptionsReport') - 1.0) -gt 0.06) {
    throw 'Native Tool Options smoke did not run at 1280x600 logical pixels and 100% scale.'
}

$optionsToolbar = Require-JsonProperty $toolOptionsReport 'optionsToolbar' 'toolOptionsReport'
if (-not (Require-JsonBoolean $optionsToolbar 'present' 'toolOptionsReport.optionsToolbar') -or
    (Require-JsonString $optionsToolbar 'objectName' 'toolOptionsReport.optionsToolbar') -ne 'options_toolbar' -or
    -not (Require-JsonBoolean $optionsToolbar 'visible' 'toolOptionsReport.optionsToolbar') -or
    -not (Require-JsonBoolean $optionsToolbar 'hostPresent' 'toolOptionsReport.optionsToolbar') -or
    (Require-JsonString $optionsToolbar 'hostObjectName' 'toolOptionsReport.optionsToolbar') -ne 'kuubikOptionToolbarHost' -or
    -not (Require-JsonBoolean $optionsToolbar 'hostVisible' 'toolOptionsReport.optionsToolbar') -or
    -not (Require-JsonBoolean $optionsToolbar 'nativeToolbarInRibbon' 'toolOptionsReport.optionsToolbar') -or
    -not (Require-JsonBoolean $optionsToolbar 'directChildOfHost' 'toolOptionsReport.optionsToolbar') -or
    -not (Require-JsonBoolean $optionsToolbar 'containedByHost' 'toolOptionsReport.optionsToolbar') -or
    -not (Require-JsonBoolean $optionsToolbar 'containedThroughWindowAncestors' 'toolOptionsReport.optionsToolbar')) {
    throw 'The native options_toolbar is not visible and geometrically contained in the Kuubik ribbon host.'
}

$toolOptionsStates = @(Require-JsonProperty $toolOptionsReport 'states' 'toolOptionsReport')
if ($toolOptionsStates.Count -ne 2) {
    throw 'Tool Options smoke must contain DrawLine and DimLinear states.'
}
foreach ($expectedState in @(
    @{ ActionKey = 'DrawLine'; Widgets = @('Ui_LineOptions'); Screenshot = 'tool-options-line-1280.png' },
    @{ ActionKey = 'DimLinear'; Widgets = @('Ui_DimOptions', 'Ui_DimLinearOptions'); Screenshot = 'tool-options-dimlinear-1280.png' }
)) {
    $state = @($toolOptionsStates | Where-Object actionKey -eq $expectedState.ActionKey)
    if ($state.Count -ne 1) {
        throw "Tool Options action state is missing or duplicated: $($expectedState.ActionKey)"
    }
    $stateContext = "toolOptionsReport.states.$($expectedState.ActionKey)"
    if (-not (Require-JsonBoolean $state[0] 'actionPresent' $stateContext) -or
        -not (Require-JsonBoolean $state[0] 'actionEnabled' $stateContext) -or
        -not (Require-JsonBoolean $state[0] 'ribbonIdentity' $stateContext) -or
        -not (Require-JsonBoolean $state[0] 'nativeActionActive' $stateContext) -or
        -not (Require-JsonBoolean $state[0] 'optionsToolbarPositiveSize' $stateContext) -or
        -not (Require-JsonBoolean $state[0] 'optionsHostPositiveSize' $stateContext) -or
        -not (Require-JsonBoolean $state[0] 'optionsToolbarContainedByHost' $stateContext) -or
        -not (Require-JsonBoolean $state[0] 'optionsToolbarContainedThroughWindowAncestors' $stateContext) -or
        -not (Require-JsonBoolean $state[0] 'settledWidgetCounts' $stateContext) -or
        -not (Require-JsonBoolean $state[0] 'screenshotSaved' $stateContext) -or
        (Require-JsonNumber $state[0] 'screenshotPixelWidth' $stateContext) -ne 1280 -or
        (Require-JsonNumber $state[0] 'screenshotPixelHeight' $stateContext) -ne 600 -or
        [Math]::Abs((Require-JsonNumber $state[0] 'screenshotDevicePixelRatio' $stateContext) - 1.0) -gt 0.06 -or
        -not (Require-JsonBoolean $state[0] 'passed' $stateContext)) {
        throw "Tool Options native action or screenshot failed: $($expectedState.ActionKey)"
    }
    $visibleWidgetCounts = Require-JsonProperty $state[0] 'visibleNativeWidgetCounts' $stateContext
    $expectedLineCount = if ($expectedState.ActionKey -eq 'DrawLine') { 1 } else { 0 }
    $expectedDimensionCount = if ($expectedState.ActionKey -eq 'DimLinear') { 1 } else { 0 }
    if ((Require-JsonNumber $visibleWidgetCounts 'line' "$stateContext.visibleNativeWidgetCounts") -ne $expectedLineCount -or
        (Require-JsonNumber $visibleWidgetCounts 'dimension' "$stateContext.visibleNativeWidgetCounts") -ne $expectedDimensionCount -or
        (Require-JsonNumber $visibleWidgetCounts 'dimLinear' "$stateContext.visibleNativeWidgetCounts") -ne $expectedDimensionCount) {
        throw "Tool Options retained a stale or duplicate native widget: $($expectedState.ActionKey)"
    }
    $stateWidgets = @(Require-JsonProperty $state[0] 'widgets' $stateContext)
    if ($stateWidgets.Count -ne $expectedState.Widgets.Count) {
        throw "Tool Options widget count is incorrect: $($expectedState.ActionKey)"
    }
    foreach ($expectedWidget in $expectedState.Widgets) {
        $widget = @($stateWidgets | Where-Object objectName -eq $expectedWidget)
        if ($widget.Count -ne 1 -or
            -not (Require-JsonBoolean $widget[0] 'present' "$stateContext.widgets.$expectedWidget") -or
            -not (Require-JsonBoolean $widget[0] 'nativeType' "$stateContext.widgets.$expectedWidget") -or
            -not (Require-JsonBoolean $widget[0] 'visible' "$stateContext.widgets.$expectedWidget") -or
            -not (Require-JsonBoolean $widget[0] 'containedByToolbar' "$stateContext.widgets.$expectedWidget") -or
            -not (Require-JsonBoolean $widget[0] 'containedByHost' "$stateContext.widgets.$expectedWidget") -or
            -not (Require-JsonBoolean $widget[0] 'containedByWindow' "$stateContext.widgets.$expectedWidget") -or
            -not (Require-JsonBoolean $widget[0] 'containedThroughWindowAncestors' "$stateContext.widgets.$expectedWidget") -or
            -not (Require-JsonBoolean $widget[0] 'positiveSize' "$stateContext.widgets.$expectedWidget") -or
            -not (Require-JsonBoolean $widget[0] 'passed' "$stateContext.widgets.$expectedWidget")) {
            throw "Native Tool Options widget is missing, hidden, geometrically outside its parent chain, or the wrong type: $expectedWidget"
        }
    }
    $toolOptionsScreenshot = Join-Path $toolOptionsEvidenceRoot $expectedState.Screenshot
    Require-Path $toolOptionsScreenshot
    if ((Get-Item -LiteralPath $toolOptionsScreenshot).Length -lt 10000) {
        throw "Tool Options screenshot is unexpectedly small: $($expectedState.Screenshot)"
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
    'actionActiveAfterRibbon', 'lineRibbonPresentationStable', 'windowWidth', 'windowHeight', 'entitiesBefore',
    'entitiesAfterFirstClick', 'dynamicInputVisible', 'escapeCancelsAll', 'entitiesAfterSecondClick', 'linesBefore',
    'linesAfterSecondClick', 'dxfSaved', 'sourceDxfLoaded', 'documentLifecycle',
    'fullPropertiesAction', 'ribbonInvocation', 'propertiesLineRibbonInvocation',
    'lineEnter', 'polylineUndoRedo', 'copyUndoRedo', 'moveUndoRedo'
)) {
    [void](Require-JsonProperty $guiSmoke $key 'guiSmoke')
}
if ($guiSmoke.schemaVersion -ne 5 -or
    $guiSmoke.status -ne 'PASS' -or
    -not $guiSmoke.prerequisites -or
    $guiSmoke.ribbonActionKey -ne 'DrawLine' -or
    -not $guiSmoke.ribbonMouseEvent -or
    -not $guiSmoke.actionActiveAfterRibbon -or
    -not $guiSmoke.lineRibbonPresentationStable -or
    -not $guiSmoke.sourceDxfLoaded -or
    $guiSmoke.windowWidth -ne 1920 -or
    $guiSmoke.windowHeight -ne 1080 -or
    $guiSmoke.entitiesAfterFirstClick -ne $guiSmoke.entitiesBefore -or
    -not $guiSmoke.dynamicInputVisible -or
    -not $guiSmoke.escapeCancelsAll -or
    $guiSmoke.entitiesAfterSecondClick -ne ($guiSmoke.entitiesBefore + 1) -or
    $guiSmoke.linesAfterSecondClick -ne ($guiSmoke.linesBefore + 1) -or
    -not $guiSmoke.dxfSaved) {
    throw 'Ribbon LINE mouse workflow did not create exactly one native LINE.'
}
Assert-RibbonMouseInvocation (Require-JsonProperty $guiSmoke 'ribbonInvocation' 'guiSmoke') 'guiSmoke.ribbonInvocation'
Assert-RibbonMouseInvocation (Require-JsonProperty $guiSmoke 'propertiesLineRibbonInvocation' 'guiSmoke') 'guiSmoke.propertiesLineRibbonInvocation'
$lineEnter = Require-JsonProperty $guiSmoke 'lineEnter' 'guiSmoke'
if (-not (Require-JsonBoolean $lineEnter 'accepted' 'guiSmoke.lineEnter') -or
    -not (Require-JsonBoolean $lineEnter 'finishedAction' 'guiSmoke.lineEnter')) {
    throw 'Enter did not finish the native LINE action.'
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

$polylineUndoRedo = Require-JsonProperty $guiSmoke 'polylineUndoRedo' 'guiSmoke'
if (-not (Require-JsonBoolean $polylineUndoRedo 'passed' 'guiSmoke.polylineUndoRedo')) {
    throw 'Native PLINE and quick-access Undo/Redo workflow failed.'
}
$polylineEnter = Require-JsonProperty $polylineUndoRedo 'enter' 'guiSmoke.polylineUndoRedo'
if (-not (Require-JsonBoolean $polylineEnter 'accepted' 'guiSmoke.polylineUndoRedo.enter') -or
    -not (Require-JsonBoolean $polylineEnter 'finishedAction' 'guiSmoke.polylineUndoRedo.enter')) {
    throw 'Enter did not finish the native PLINE action.'
}
$polylineRibbon = Require-JsonProperty $polylineUndoRedo 'ribbon' 'guiSmoke.polylineUndoRedo'
if ((Require-JsonString $polylineRibbon 'actionKey' 'guiSmoke.polylineUndoRedo.ribbon') -ne 'DrawPolyline' -or
    -not (Require-JsonBoolean $polylineRibbon 'nativeIdentity' 'guiSmoke.polylineUndoRedo.ribbon') -or
    -not (Require-JsonBoolean $polylineRibbon 'nativeActionActive' 'guiSmoke.polylineUndoRedo.ribbon') -or
    -not (Require-JsonBoolean $polylineRibbon 'presentationStable' 'guiSmoke.polylineUndoRedo.ribbon')) {
    throw 'Ribbon PLINE did not activate the native DrawPolyline action through a mouse event.'
}
Assert-RibbonMouseInvocation $polylineRibbon 'guiSmoke.polylineUndoRedo.ribbon'
$polylineEntity = Require-JsonProperty $polylineUndoRedo 'polyline' 'guiSmoke.polylineUndoRedo'
if (-not (Require-JsonBoolean $polylineEntity 'created' 'guiSmoke.polylineUndoRedo.polyline') -or
    (Require-JsonBoolean $polylineEntity 'entityUndoneBeforeUndo' 'guiSmoke.polylineUndoRedo.polyline') -or
    -not (Require-JsonBoolean $polylineEntity 'entityUndoneAfterUndo' 'guiSmoke.polylineUndoRedo.polyline') -or
    (Require-JsonBoolean $polylineEntity 'entityUndoneAfterRedo' 'guiSmoke.polylineUndoRedo.polyline') -or
    (Require-JsonString $polylineEntity 'layer' 'guiSmoke.polylineUndoRedo.polyline') -ne 'KUUBIK-SMOKE-LAYER' -or
    (Require-JsonBoolean $polylineEntity 'closed' 'guiSmoke.polylineUndoRedo.polyline') -or
    (Require-JsonNumber $polylineEntity 'segmentCount' 'guiSmoke.polylineUndoRedo.polyline') -ne 2 -or
    (Require-JsonNumber $polylineEntity 'verticesExpected' 'guiSmoke.polylineUndoRedo.polyline') -ne 3 -or
    (Require-JsonNumber $polylineEntity 'activeCountBeforeUndo' 'guiSmoke.polylineUndoRedo.polyline') -ne
        ((Require-JsonNumber $polylineEntity 'activeCountBeforeCreate' 'guiSmoke.polylineUndoRedo.polyline') + 1) -or
    (Require-JsonNumber $polylineEntity 'activeCountAfterUndo' 'guiSmoke.polylineUndoRedo.polyline') -ne
        (Require-JsonNumber $polylineEntity 'activeCountBeforeCreate' 'guiSmoke.polylineUndoRedo.polyline') -or
    (Require-JsonNumber $polylineEntity 'activeCountAfterRedo' 'guiSmoke.polylineUndoRedo.polyline') -ne
        ((Require-JsonNumber $polylineEntity 'activeCountBeforeCreate' 'guiSmoke.polylineUndoRedo.polyline') + 1)) {
    throw 'Native PLINE entity, layer, segment count, or undo state is incorrect.'
}
foreach ($undoRedoName in @('undo', 'redo')) {
    $actionState = Require-JsonProperty $polylineUndoRedo $undoRedoName 'guiSmoke.polylineUndoRedo'
    $expectedActionKey = if ($undoRedoName -eq 'undo') { 'EditUndo' } else { 'EditRedo' }
    if ((Require-JsonString $actionState 'actionKey' "guiSmoke.polylineUndoRedo.$undoRedoName") -ne $expectedActionKey -or
        -not (Require-JsonBoolean $actionState 'nativeIdentity' "guiSmoke.polylineUndoRedo.$undoRedoName") -or
        -not (Require-JsonBoolean $actionState 'quickAccessButton' "guiSmoke.polylineUndoRedo.$undoRedoName") -or
        -not (Require-JsonBoolean $actionState 'visible' "guiSmoke.polylineUndoRedo.$undoRedoName") -or
        -not (Require-JsonBoolean $actionState 'enabledBeforeClick' "guiSmoke.polylineUndoRedo.$undoRedoName") -or
        -not (Require-JsonBoolean $actionState 'actionTriggeredByMouse' "guiSmoke.polylineUndoRedo.$undoRedoName") -or
        -not (Require-JsonBoolean $actionState 'firstLineStillActive' "guiSmoke.polylineUndoRedo.$undoRedoName")) {
        throw "Quick-access $expectedActionKey did not use the native QAction or changed the earlier LINE."
    }
}
$polylineFiles = Require-JsonProperty $polylineUndoRedo 'files' 'guiSmoke.polylineUndoRedo'
foreach ($fileState in @(
    @{ Name = 'beforeUndo'; Saved = 'beforeUndoSaved'; File = 'pline-before-undo.dxf' },
    @{ Name = 'afterUndo'; Saved = 'afterUndoSaved'; File = 'pline-after-undo.dxf' },
    @{ Name = 'afterRedo'; Saved = 'afterRedoSaved'; File = 'pline-after-redo.dxf' }
)) {
    if ((Require-JsonString $polylineFiles $fileState.Name 'guiSmoke.polylineUndoRedo.files') -ne $fileState.File -or
        -not (Require-JsonBoolean $polylineFiles $fileState.Saved 'guiSmoke.polylineUndoRedo.files')) {
        throw "PLINE Undo/Redo DXF producer state is incomplete: $($fileState.File)"
    }
    $polylineDxf = Join-Path $guiSmokeDirectory $fileState.File
    Require-Path $polylineDxf
    if ((Get-Item -LiteralPath $polylineDxf).Length -lt 500) {
        throw "PLINE Undo/Redo DXF is unexpectedly small: $($fileState.File)"
    }
}

$copyUndoRedo = Require-JsonProperty $guiSmoke 'copyUndoRedo' 'guiSmoke'
if (-not (Require-JsonBoolean $copyUndoRedo 'passed' 'guiSmoke.copyUndoRedo')) {
    throw 'Native COPY and quick-access Undo/Redo workflow failed.'
}
$copyRibbon = Require-JsonProperty $copyUndoRedo 'ribbon' 'guiSmoke.copyUndoRedo'
if ((Require-JsonString $copyRibbon 'actionKey' 'guiSmoke.copyUndoRedo.ribbon') -ne 'ModifyDuplicate' -or
    -not (Require-JsonBoolean $copyRibbon 'nativeIdentity' 'guiSmoke.copyUndoRedo.ribbon') -or
    (Require-JsonNumber $copyRibbon 'activeActionType' 'guiSmoke.copyUndoRedo.ribbon') -ne
        (Require-JsonNumber $copyRibbon 'expectedActionType' 'guiSmoke.copyUndoRedo.ribbon') -or
    -not (Require-JsonBoolean $copyRibbon 'nativeActionActive' 'guiSmoke.copyUndoRedo.ribbon')) {
    throw 'Ribbon COPY did not activate the native ModifyDuplicate action through a mouse event.'
}
Assert-RibbonMouseInvocation $copyRibbon 'guiSmoke.copyUndoRedo.ribbon'
$copyEntity = Require-JsonProperty $copyUndoRedo 'copy' 'guiSmoke.copyUndoRedo'
if (-not (Require-JsonBoolean $copyEntity 'created' 'guiSmoke.copyUndoRedo.copy') -or
    (Require-JsonNumber $copyEntity 'candidateCount' 'guiSmoke.copyUndoRedo.copy') -ne 1 -or
    -not (Require-JsonBoolean $copyEntity 'duplicateInPlace' 'guiSmoke.copyUndoRedo.copy') -or
    -not (Require-JsonBoolean $copyEntity 'inPlaceForcedForSmoke' 'guiSmoke.copyUndoRedo.copy') -or
    -not (Require-JsonBoolean $copyEntity 'sourceUnselectedBeforeAction' 'guiSmoke.copyUndoRedo.copy') -or
    -not (Require-JsonBoolean $copyEntity 'canvasPointInside' 'guiSmoke.copyUndoRedo.copy') -or
    -not (Require-JsonBoolean $copyEntity 'sourceDistinct' 'guiSmoke.copyUndoRedo.copy') -or
    (Require-JsonString $copyEntity 'activeLayerBeforeAction' 'guiSmoke.copyUndoRedo.copy') -ne 'KUUBIK-SMOKE-LAYER' -or
    (Require-JsonString $copyEntity 'sourceLayer' 'guiSmoke.copyUndoRedo.copy') -ne 'KUUBIK-SMOKE-LAYER' -or
    (Require-JsonString $copyEntity 'duplicateLayer' 'guiSmoke.copyUndoRedo.copy') -ne
        (Require-JsonString $copyEntity 'sourceLayer' 'guiSmoke.copyUndoRedo.copy') -or
    -not (Require-JsonBoolean $copyEntity 'startMatches' 'guiSmoke.copyUndoRedo.copy') -or
    -not (Require-JsonBoolean $copyEntity 'endMatches' 'guiSmoke.copyUndoRedo.copy') -or
    (Require-JsonBoolean $copyEntity 'entityUndoneBeforeUndo' 'guiSmoke.copyUndoRedo.copy') -or
    -not (Require-JsonBoolean $copyEntity 'entityUndoneAfterUndo' 'guiSmoke.copyUndoRedo.copy') -or
    (Require-JsonBoolean $copyEntity 'entityUndoneAfterRedo' 'guiSmoke.copyUndoRedo.copy') -or
    (Require-JsonNumber $copyEntity 'activeCountBeforeUndo' 'guiSmoke.copyUndoRedo.copy') -ne
        ((Require-JsonNumber $copyEntity 'activeCountBeforeCreate' 'guiSmoke.copyUndoRedo.copy') + 1) -or
    (Require-JsonNumber $copyEntity 'activeCountAfterUndo' 'guiSmoke.copyUndoRedo.copy') -ne
        (Require-JsonNumber $copyEntity 'activeCountBeforeCreate' 'guiSmoke.copyUndoRedo.copy') -or
    (Require-JsonNumber $copyEntity 'activeCountAfterRedo' 'guiSmoke.copyUndoRedo.copy') -ne
        ((Require-JsonNumber $copyEntity 'activeCountBeforeCreate' 'guiSmoke.copyUndoRedo.copy') + 1)) {
    throw 'Native COPY entity, layer, geometry, or undo state is incorrect.'
}
$copyCanvasPoint = Require-JsonProperty $copyEntity 'canvasPoint' 'guiSmoke.copyUndoRedo.copy'
if (-not (Require-JsonBoolean $copyCanvasPoint 'inside' 'guiSmoke.copyUndoRedo.copy.canvasPoint')) {
    throw 'Native COPY source canvas point was outside the graphic view.'
}
foreach ($coordinate in @('graphX', 'graphY', 'guiX', 'guiY')) {
    [void](Require-JsonNumber $copyCanvasPoint $coordinate 'guiSmoke.copyUndoRedo.copy.canvasPoint')
}
foreach ($undoRedoName in @('undo', 'redo')) {
    $actionState = Require-JsonProperty $copyUndoRedo $undoRedoName 'guiSmoke.copyUndoRedo'
    $expectedActionKey = if ($undoRedoName -eq 'undo') { 'EditUndo' } else { 'EditRedo' }
    if ((Require-JsonString $actionState 'actionKey' "guiSmoke.copyUndoRedo.$undoRedoName") -ne $expectedActionKey -or
        -not (Require-JsonBoolean $actionState 'nativeIdentity' "guiSmoke.copyUndoRedo.$undoRedoName") -or
        -not (Require-JsonBoolean $actionState 'quickAccessButton' "guiSmoke.copyUndoRedo.$undoRedoName") -or
        -not (Require-JsonBoolean $actionState 'visible' "guiSmoke.copyUndoRedo.$undoRedoName") -or
        -not (Require-JsonBoolean $actionState 'enabledBeforeClick' "guiSmoke.copyUndoRedo.$undoRedoName") -or
        -not (Require-JsonBoolean $actionState 'actionTriggeredByMouse' "guiSmoke.copyUndoRedo.$undoRedoName") -or
        -not (Require-JsonBoolean $actionState 'firstLineStillActive' "guiSmoke.copyUndoRedo.$undoRedoName") -or
        -not (Require-JsonBoolean $actionState 'priorPolylineStillActive' "guiSmoke.copyUndoRedo.$undoRedoName")) {
        throw "Quick-access $expectedActionKey did not isolate the native COPY undo cycle."
    }
}
$copyFiles = Require-JsonProperty $copyUndoRedo 'files' 'guiSmoke.copyUndoRedo'
foreach ($fileState in @(
    @{ Name = 'beforeUndo'; Saved = 'beforeUndoSaved'; File = 'copy-before-undo.dxf' },
    @{ Name = 'afterUndo'; Saved = 'afterUndoSaved'; File = 'copy-after-undo.dxf' },
    @{ Name = 'afterRedo'; Saved = 'afterRedoSaved'; File = 'copy-after-redo.dxf' }
)) {
    if ((Require-JsonString $copyFiles $fileState.Name 'guiSmoke.copyUndoRedo.files') -ne $fileState.File -or
        -not (Require-JsonBoolean $copyFiles $fileState.Saved 'guiSmoke.copyUndoRedo.files')) {
        throw "COPY Undo/Redo DXF producer state is incomplete: $($fileState.File)"
    }
    $copyDxf = Join-Path $guiSmokeDirectory $fileState.File
    Require-Path $copyDxf
    if ((Get-Item -LiteralPath $copyDxf).Length -lt 500) {
        throw "COPY Undo/Redo DXF is unexpectedly small: $($fileState.File)"
    }
}

$moveUndoRedo = Require-JsonProperty $guiSmoke 'moveUndoRedo' 'guiSmoke'
if (-not (Require-JsonBoolean $moveUndoRedo 'passed' 'guiSmoke.moveUndoRedo')) {
    throw 'Native MOVE, move dialog, and quick-access Undo/Redo workflow failed.'
}
$moveRibbon = Require-JsonProperty $moveUndoRedo 'ribbon' 'guiSmoke.moveUndoRedo'
if ((Require-JsonString $moveRibbon 'actionKey' 'guiSmoke.moveUndoRedo.ribbon') -ne 'ModifyMove' -or
    -not (Require-JsonBoolean $moveRibbon 'nativeIdentity' 'guiSmoke.moveUndoRedo.ribbon') -or
    -not (Require-JsonBoolean $moveRibbon 'selectionActionActive' 'guiSmoke.moveUndoRedo.ribbon') -or
    (Require-JsonNumber $moveRibbon 'initialActionType' 'guiSmoke.moveUndoRedo.ribbon') -ne
        (Require-JsonNumber $moveRibbon 'expectedSelectionActionType' 'guiSmoke.moveUndoRedo.ribbon') -or
    -not (Require-JsonBoolean $moveRibbon 'nativeActionActive' 'guiSmoke.moveUndoRedo.ribbon') -or
    (Require-JsonNumber $moveRibbon 'actionTypeAfterSelection' 'guiSmoke.moveUndoRedo.ribbon') -ne
        (Require-JsonNumber $moveRibbon 'expectedMoveActionType' 'guiSmoke.moveUndoRedo.ribbon')) {
    throw 'Ribbon MOVE did not advance from native SelectSingle to native ModifyMove.'
}
Assert-RibbonMouseInvocation $moveRibbon 'guiSmoke.moveUndoRedo.ribbon'

$moveEntity = Require-JsonProperty $moveUndoRedo 'move' 'guiSmoke.moveUndoRedo'
if (-not (Require-JsonBoolean $moveEntity 'created' 'guiSmoke.moveUndoRedo.move') -or
    (Require-JsonNumber $moveEntity 'candidateCount' 'guiSmoke.moveUndoRedo.move') -ne 1 -or
    -not (Require-JsonBoolean $moveEntity 'sourceUnselectedBeforeAction' 'guiSmoke.moveUndoRedo.move') -or
    -not (Require-JsonBoolean $moveEntity 'sourceSelectedByCanvas' 'guiSmoke.moveUndoRedo.move') -or
    (Require-JsonString $moveEntity 'activeLayerBeforeAction' 'guiSmoke.moveUndoRedo.move') -ne 'KUUBIK-SMOKE-LAYER' -or
    (Require-JsonString $moveEntity 'sourceLayer' 'guiSmoke.moveUndoRedo.move') -ne 'KUUBIK-SMOKE-LAYER' -or
    (Require-JsonString $moveEntity 'movedLayer' 'guiSmoke.moveUndoRedo.move') -ne
        (Require-JsonString $moveEntity 'sourceLayer' 'guiSmoke.moveUndoRedo.move') -or
    -not (Require-JsonBoolean $moveEntity 'offsetNonZero' 'guiSmoke.moveUndoRedo.move') -or
    -not (Require-JsonBoolean $moveEntity 'offsetMatchesBothEndpoints' 'guiSmoke.moveUndoRedo.move') -or
    -not (Require-JsonBoolean $moveEntity 'sourceUndoneBeforeUndo' 'guiSmoke.moveUndoRedo.move') -or
    (Require-JsonBoolean $moveEntity 'movedUndoneBeforeUndo' 'guiSmoke.moveUndoRedo.move') -or
    -not (Require-JsonBoolean $moveEntity 'sourceActiveAfterUndo' 'guiSmoke.moveUndoRedo.move') -or
    -not (Require-JsonBoolean $moveEntity 'movedUndoneAfterUndo' 'guiSmoke.moveUndoRedo.move') -or
    -not (Require-JsonBoolean $moveEntity 'sourceUndoneAfterRedo' 'guiSmoke.moveUndoRedo.move') -or
    -not (Require-JsonBoolean $moveEntity 'movedActiveAfterRedo' 'guiSmoke.moveUndoRedo.move') -or
    -not (Require-JsonBoolean $moveEntity 'snapModeTemporarilyCleared' 'guiSmoke.moveUndoRedo.move') -or
    -not (Require-JsonBoolean $moveEntity 'snapModeRestored' 'guiSmoke.moveUndoRedo.move') -or
    (Require-JsonNumber $moveEntity 'activeCountBeforeUndo' 'guiSmoke.moveUndoRedo.move') -ne
        (Require-JsonNumber $moveEntity 'activeCountBeforeMove' 'guiSmoke.moveUndoRedo.move') -or
    (Require-JsonNumber $moveEntity 'activeCountAfterUndo' 'guiSmoke.moveUndoRedo.move') -ne
        (Require-JsonNumber $moveEntity 'activeCountBeforeMove' 'guiSmoke.moveUndoRedo.move') -or
    (Require-JsonNumber $moveEntity 'activeCountAfterRedo' 'guiSmoke.moveUndoRedo.move') -ne
        (Require-JsonNumber $moveEntity 'activeCountBeforeMove' 'guiSmoke.moveUndoRedo.move')) {
    throw 'Native MOVE entity, layer, geometry, snap fixture, or undo state is incorrect.'
}
foreach ($pointName in @('selectionCanvasPoint', 'referenceCanvasPoint', 'targetCanvasPoint')) {
    $canvasPoint = Require-JsonProperty $moveEntity $pointName 'guiSmoke.moveUndoRedo.move'
    if (-not (Require-JsonBoolean $canvasPoint 'inside' "guiSmoke.moveUndoRedo.move.$pointName")) {
        throw "Native MOVE canvas point was outside the graphic view: $pointName"
    }
    foreach ($coordinate in @('x', 'y')) {
        [void](Require-JsonNumber $canvasPoint $coordinate "guiSmoke.moveUndoRedo.move.$pointName")
    }
}
foreach ($vectorName in @('sourceStart', 'sourceEnd', 'movedStart', 'movedEnd', 'offset')) {
    $vector = Require-JsonProperty $moveEntity $vectorName 'guiSmoke.moveUndoRedo.move'
    if (-not (Require-JsonBoolean $vector 'valid' "guiSmoke.moveUndoRedo.move.$vectorName")) {
        throw "Native MOVE vector is invalid: $vectorName"
    }
    foreach ($coordinate in @('x', 'y')) {
        [void](Require-JsonNumber $vector $coordinate "guiSmoke.moveUndoRedo.move.$vectorName")
    }
}

$moveDialog = Require-JsonProperty $moveUndoRedo 'dialog' 'guiSmoke.moveUndoRedo'
if ((Require-JsonString $moveDialog 'objectName' 'guiSmoke.moveUndoRedo.dialog') -ne 'QG_DlgMove' -or
    -not (Require-JsonBoolean $moveDialog 'timerRan' 'guiSmoke.moveUndoRedo.dialog') -or
    -not (Require-JsonBoolean $moveDialog 'found' 'guiSmoke.moveUndoRedo.dialog') -or
    -not (Require-JsonBoolean $moveDialog 'visible' 'guiSmoke.moveUndoRedo.dialog') -or
    -not (Require-JsonBoolean $moveDialog 'moveModeControlFound' 'guiSmoke.moveUndoRedo.dialog') -or
    -not (Require-JsonBoolean $moveDialog 'moveModeClickedByMouse' 'guiSmoke.moveUndoRedo.dialog') -or
    -not (Require-JsonBoolean $moveDialog 'moveModeSelected' 'guiSmoke.moveUndoRedo.dialog') -or
    -not (Require-JsonBoolean $moveDialog 'okFound' 'guiSmoke.moveUndoRedo.dialog') -or
    -not (Require-JsonBoolean $moveDialog 'okClickedByMouse' 'guiSmoke.moveUndoRedo.dialog') -or
    -not (Require-JsonBoolean $moveDialog 'acceptedByMouse' 'guiSmoke.moveUndoRedo.dialog') -or
    (Require-JsonBoolean $moveDialog 'safetyTriggered' 'guiSmoke.moveUndoRedo.dialog')) {
    throw 'Native QG_DlgMove was not completed through the expected mouse path.'
}

foreach ($undoRedoName in @('undo', 'redo')) {
    $actionState = Require-JsonProperty $moveUndoRedo $undoRedoName 'guiSmoke.moveUndoRedo'
    $expectedActionKey = if ($undoRedoName -eq 'undo') { 'EditUndo' } else { 'EditRedo' }
    if ((Require-JsonString $actionState 'actionKey' "guiSmoke.moveUndoRedo.$undoRedoName") -ne $expectedActionKey -or
        -not (Require-JsonBoolean $actionState 'nativeIdentity' "guiSmoke.moveUndoRedo.$undoRedoName") -or
        -not (Require-JsonBoolean $actionState 'quickAccessButton' "guiSmoke.moveUndoRedo.$undoRedoName") -or
        -not (Require-JsonBoolean $actionState 'visible' "guiSmoke.moveUndoRedo.$undoRedoName") -or
        -not (Require-JsonBoolean $actionState 'enabledBeforeClick' "guiSmoke.moveUndoRedo.$undoRedoName") -or
        -not (Require-JsonBoolean $actionState 'actionTriggeredByMouse' "guiSmoke.moveUndoRedo.$undoRedoName") -or
        -not (Require-JsonBoolean $actionState 'firstLineStillActive' "guiSmoke.moveUndoRedo.$undoRedoName") -or
        -not (Require-JsonBoolean $actionState 'copyStillActive' "guiSmoke.moveUndoRedo.$undoRedoName") -or
        -not (Require-JsonBoolean $actionState 'priorPolylineStillActive' "guiSmoke.moveUndoRedo.$undoRedoName")) {
        throw "Quick-access $expectedActionKey did not isolate the native MOVE undo cycle."
    }
}
$moveFiles = Require-JsonProperty $moveUndoRedo 'files' 'guiSmoke.moveUndoRedo'
foreach ($fileState in @(
    @{ Name = 'beforeUndo'; Saved = 'beforeUndoSaved'; File = 'move-before-undo.dxf' },
    @{ Name = 'afterUndo'; Saved = 'afterUndoSaved'; File = 'move-after-undo.dxf' },
    @{ Name = 'afterRedo'; Saved = 'afterRedoSaved'; File = 'move-after-redo.dxf' }
)) {
    if ((Require-JsonString $moveFiles $fileState.Name 'guiSmoke.moveUndoRedo.files') -ne $fileState.File -or
        -not (Require-JsonBoolean $moveFiles $fileState.Saved 'guiSmoke.moveUndoRedo.files')) {
        throw "MOVE Undo/Redo DXF producer state is incomplete: $($fileState.File)"
    }
    $moveDxf = Join-Path $guiSmokeDirectory $fileState.File
    Require-Path $moveDxf
    if ((Get-Item -LiteralPath $moveDxf).Length -lt 500) {
        throw "MOVE Undo/Redo DXF is unexpectedly small: $($fileState.File)"
    }
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
Write-Host "Native Tool Options evidence: $toolOptionsEvidenceRoot"
Write-Host "Ribbon LINE GUI evidence: $guiSmokeDirectory"
