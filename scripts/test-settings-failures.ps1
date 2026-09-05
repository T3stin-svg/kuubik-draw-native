[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)][string]$Executable,
    [Parameter(Mandatory = $true)][string]$OutputDirectory
)
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
$executablePath = (Resolve-Path -LiteralPath $Executable).Path
$outputPath = [IO.Path]::GetFullPath($OutputDirectory)
if (Test-Path -LiteralPath $outputPath) { throw 'Use fresh failure evidence.' }
New-Item -ItemType Directory -Path $outputPath | Out-Null
$tokens = $null
$parseErrors = $null
$ast = [System.Management.Automation.Language.Parser]::ParseFile(
    (Join-Path $PSScriptRoot 'test-kuubik-portable.ps1'), [ref]$tokens, [ref]$parseErrors)
if ($parseErrors.Count) { throw $parseErrors }
foreach ($definition in $ast.FindAll({ param($node)
    $node -is [System.Management.Automation.Language.FunctionDefinitionAst]
}, $false)) { Invoke-Expression $definition.Extent.Text }
$before = Get-NativeSettingsRegistrySnapshot
try {
    foreach ($case in @('report-open', 'settings-sync', 'combined')) {
        $smokeRoot = Join-Path $outputPath $case
        $settings = Join-Path $smokeRoot 'settings'
        New-Item -ItemType Directory -Path $settings | Out-Null
        $lock = $null
        if ($case -ne 'settings-sync') {
            New-Item -ItemType Directory -Path (Join-Path $settings 'settings-isolation.json') | Out-Null
        }
        if ($case -ne 'report-open') {
            $iniDirectory = Join-Path $settings 'Kuubik Projekt OÜ'
            New-Item -ItemType Directory -Path $iniDirectory | Out-Null
            # A real Windows sharing violation: QSettings cannot replace this INI.
            $lock = [IO.File]::Open((Join-Path $iniDirectory 'Kuubik Draw.ini'),
                [IO.FileMode]::CreateNew, [IO.FileAccess]::ReadWrite, [IO.FileShare]::Read)
        }
        try {
            $environment = @{
                QT_QPA_PLATFORM = 'offscreen'
                KUUBIK_UI_CONTRACT_PATH = (Join-Path $smokeRoot 'contract.json')
            }
            $failed = $false
            try { Run-Native $executablePath @() $case $environment 30 }
            catch {
                if ($_.Exception.Message -notlike '*Native stderr:*' -or $_.Exception.Message -like '*timeout=True*') { throw }
                $failed = $true
            }
            if (-not $failed) { throw "$case unexpectedly started successfully" }
            $logs = @(Get-ChildItem -LiteralPath $smokeRoot -Filter 'native-failure-*.log')
            $expectedStage = if ($case -eq 'settings-sync') { 'settings-sync' } else { 'report-open' }
            if ($logs.Count -ne 1 -or (Get-Content -Raw -LiteralPath $logs[0].FullName) -notlike "*stage=$expectedStage*") {
                throw "$case did not retain its native failure stage"
            }
            if ($case -ne 'report-open' -and (Get-Content -Raw -LiteralPath $logs[0].FullName) -notlike
                '*native=1 qt=1 cleanup=1 nativeToQt=1 qtToNative=1*') { throw 'Primary settings failure was hidden' }
            if (Test-Path -LiteralPath (Join-Path $smokeRoot 'contract.json')) { throw 'Failed isolation reached the UI' }
            if ($case -eq 'settings-sync') {
                $report = Get-Content -Raw (Join-Path $settings 'settings-isolation.json') | ConvertFrom-Json
                if ($report.passed -ne $false -or $report.nativeSyncStatus -ne 1 -or $report.qtSyncStatus -ne 1 -or
                    $report.cleanupSyncStatus -ne 1 -or $report.nativeWriteQtRead -ne $true -or
                    $report.qtWriteNativeRead -ne $true) { throw 'Expected retained QSettings AccessError with successful reads' }
            }
            Write-Output "PASS ${case}: refused startup, retained native diagnostic"
        } finally { if ($null -ne $lock) { $lock.Dispose() } }
    }
} finally {
    $unchanged = $before -ceq (Get-NativeSettingsRegistrySnapshot)
    @{registryUnchanged=$unchanged} | ConvertTo-Json | Set-Content (Join-Path $outputPath 'registry-check.json')
    if (-not $unchanged) { throw 'Native user registry changed' }
}
