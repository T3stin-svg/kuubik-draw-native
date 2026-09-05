[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)][string]$Executable,
    [Parameter(Mandatory = $true)][string]$InputDxf,
    [Parameter(Mandatory = $true)][string]$OutputDirectory,
    [switch]$Compatibility,
    [switch]$ModelOnly
)
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
$executablePath = (Resolve-Path -LiteralPath $Executable).Path
$inputPath = (Resolve-Path -LiteralPath $InputDxf).Path
$outputPath = [IO.Path]::GetFullPath($OutputDirectory)
if (Test-Path -LiteralPath $outputPath) { throw 'Use a fresh evidence directory.' }
New-Item -ItemType Directory -Path $outputPath | Out-Null
# Reuse the existing process timeout and native/Qt settings isolation checks.
$tokens = $null
$parseErrors = $null
$ast = [System.Management.Automation.Language.Parser]::ParseFile(
    (Join-Path $PSScriptRoot 'test-kuubik-portable.ps1'), [ref]$tokens, [ref]$parseErrors)
if ($parseErrors.Count) { throw $parseErrors }
foreach ($definition in $ast.FindAll({ param($node)
    $node -is [System.Management.Automation.Language.FunctionDefinitionAst]
}, $false)) { Invoke-Expression $definition.Extent.Text }
$script:isolatedSettingsChecks = 0
$smokeRoot = $outputPath
$before = Get-NativeSettingsRegistrySnapshot
try {
    $environment = @{
        QT_QPA_PLATFORM = 'offscreen'
        KUUBIK_GUI_SMOKE_DIR = $outputPath
        KUUBIK_PAPERSPACE_GUARD_INPUT_DXF = $inputPath
    }
    if ($Compatibility) { $environment.KUUBIK_PAPERSPACE_GUARD_COMPATIBILITY = '1' }
    if ($ModelOnly) { $environment.KUUBIK_PAPERSPACE_GUARD_MODEL_ONLY = '1' }
    Run-Native $executablePath @() 'Paperspace save guard' $environment 45
} finally {
    $unchanged = $before -ceq (Get-NativeSettingsRegistrySnapshot)
    [ordered]@{registryUnchanged=$unchanged; isolatedSettingsChecks=$script:isolatedSettingsChecks} |
        ConvertTo-Json | Set-Content (Join-Path $outputPath 'settings-validation.json')
    if (-not $unchanged) { throw 'User registry changed.' }
}
$report = Get-Content -LiteralPath (Join-Path $outputPath 'paperspace-save-guard.json') -Raw | ConvertFrom-Json
if ($report.passed -ne $true) { throw 'Native save guard failed.' }
Write-Output 'PASS native paperspace Save/Save As/autosave/export protection; isolated settings and registry unchanged'
