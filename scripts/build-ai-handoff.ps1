[CmdletBinding()]
param(
    [string]$RepositoryRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path,
    [string]$Version = '0.2.0-preview.2',
    [string]$SourceRef = 'HEAD',
    [string]$ExecutableSourceCommit = '171d95915f6f5a34b8d9fcb487dd3429de8cda74',
    [string]$PortableZip,
    [string]$EvidenceZip
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

function Require-Path([string]$Path) {
    if (-not (Test-Path -LiteralPath $Path)) {
        throw "Required handoff input is missing: $Path"
    }
}

function Assert-GitSuccess([string]$Step) {
    if ($LASTEXITCODE -ne 0) {
        throw "$Step failed with exit code $LASTEXITCODE"
    }
}

$repository = (Resolve-Path -LiteralPath $RepositoryRoot).Path
$artifactRoot = Join-Path $repository 'artifacts'
if ([string]::IsNullOrWhiteSpace($PortableZip)) {
    $PortableZip = Join-Path $artifactRoot "KuubikDraw-$Version-win64.zip"
}
if ([string]::IsNullOrWhiteSpace($EvidenceZip)) {
    $EvidenceZip = Join-Path $artifactRoot "KuubikDraw-$Version-evidence.zip"
}

Require-Path $PortableZip
Require-Path "$PortableZip.sha256"
Require-Path $EvidenceZip
Require-Path "$EvidenceZip.sha256"
Require-Path (Join-Path $repository 'AI_START.md')
Require-Path (Join-Path $repository 'AGENTS.md')
Require-Path (Join-Path $repository 'NEXT_TASKS.md')
Require-Path (Join-Path $repository 'docs\PROJECT_STATE.md')
Require-Path (Join-Path $repository 'docs\DECISIONS.md')
Require-Path (Join-Path $repository 'docs\ROADMAP.md')
Require-Path (Join-Path $repository 'docs\TEST_REPORT.md')

$handoffCommit = (& git -C $repository rev-parse $SourceRef).Trim()
Assert-GitSuccess 'git rev-parse source ref'
$tagCommit = (& git -C $repository rev-list -n 1 "v$Version").Trim()
Assert-GitSuccess 'git rev-list release tag'
if ($tagCommit -ne $ExecutableSourceCommit) {
    throw "Release tag commit $tagCommit does not match executable source $ExecutableSourceCommit"
}

$tempBase = [IO.Path]::GetTempPath()
$stageName = "kuubik-draw-ai-handoff-$Version-$($handoffCommit.Substring(0, 8))-$([Guid]::NewGuid().ToString('N'))"
$stageRoot = Join-Path $tempBase $stageName
$resolvedTempBase = [IO.Path]::GetFullPath($tempBase).TrimEnd([IO.Path]::DirectorySeparatorChar) + [IO.Path]::DirectorySeparatorChar
$resolvedStage = [IO.Path]::GetFullPath($stageRoot)
if (-not $resolvedStage.StartsWith($resolvedTempBase, [StringComparison]::OrdinalIgnoreCase) -or
    -not (Split-Path $resolvedStage -Leaf).StartsWith('kuubik-draw-ai-handoff-')) {
    throw "Unsafe handoff staging path: $resolvedStage"
}

$outputName = "KuubikDraw-$Version-exe-source-AI.zip"
$outputZip = Join-Path $artifactRoot $outputName
$outputSha = "$outputZip.sha256"
if ((Test-Path -LiteralPath $outputZip) -or (Test-Path -LiteralPath $outputSha)) {
    throw "Refusing to overwrite an existing handoff artifact: $outputZip"
}

New-Item -ItemType Directory -Path $stageRoot | Out-Null
try {
    $binaryRoot = Join-Path $stageRoot 'BINARY'
    $sourceRoot = Join-Path $stageRoot 'SOURCE'
    $evidenceRoot = Join-Path $stageRoot 'EVIDENCE'
    New-Item -ItemType Directory -Path $binaryRoot,$sourceRoot,$evidenceRoot | Out-Null

    Copy-Item -LiteralPath $PortableZip,"$PortableZip.sha256" -Destination $binaryRoot
    Copy-Item -LiteralPath $EvidenceZip,"$EvidenceZip.sha256" -Destination $evidenceRoot

    $sourceZipName = "KuubikDraw-$Version-source-$($handoffCommit.Substring(0, 8)).zip"
    $sourceZip = Join-Path $sourceRoot $sourceZipName
    & git -C $repository archive --format=zip --output=$sourceZip $SourceRef
    Assert-GitSuccess 'git archive source snapshot'
    $sourceHash = (Get-FileHash -LiteralPath $sourceZip -Algorithm SHA256).Hash.ToLowerInvariant()
    "$sourceHash  $sourceZipName" | Set-Content -LiteralPath "$sourceZip.sha256" -Encoding ASCII

    $handoffFiles = @(
        'AI_START.md',
        'AGENTS.md',
        'NEXT_TASKS.md',
        'PROMPT_FOR_NEXT_AI.md',
        'README.md',
        'README_TEST.md',
        'FORK_NOTICE.md',
        'LICENSE',
        'THIRD_PARTY_NOTICES.md'
    )
    foreach ($relative in $handoffFiles) {
        Copy-Item -LiteralPath (Join-Path $repository $relative) -Destination $stageRoot
    }
    Copy-Item -LiteralPath (Join-Path $repository 'docs') -Destination $stageRoot -Recurse

    $portableHash = (Get-FileHash -LiteralPath $PortableZip -Algorithm SHA256).Hash.ToLowerInvariant()
    $evidenceHash = (Get-FileHash -LiteralPath $EvidenceZip -Algorithm SHA256).Hash.ToLowerInvariant()
    $manifest = [ordered]@{
        schemaVersion = 1
        product = 'Kuubik Draw Native'
        version = $Version
        repository = 'https://github.com/T3stin-svg/kuubik-draw-native'
        defaultBranch = 'kuubik/visual-v0.2'
        handoffSourceCommit = $handoffCommit
        executableSourceCommit = $ExecutableSourceCommit
        releaseTag = "v$Version"
        upstreamRepository = 'https://github.com/LibreCAD/LibreCAD'
        upstreamCommit = '7ebab007d9eb4c68609388b835a2487648f0877b'
        portableZip = [ordered]@{
            name = (Split-Path $PortableZip -Leaf)
            sha256 = $portableHash
        }
        sourceZip = [ordered]@{
            name = $sourceZipName
            sha256 = $sourceHash
        }
        evidenceZip = [ordered]@{
            name = (Split-Path $EvidenceZip -Leaf)
            sha256 = $evidenceHash
        }
        startFile = 'AI_START.md'
        generatedAtUtc = [DateTime]::UtcNow.ToString('o')
    }
    $manifest | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath (Join-Path $stageRoot 'HANDOFF_MANIFEST.json') -Encoding UTF8

    $checksumPath = Join-Path $stageRoot 'SHA256SUMS.txt'
    $checksums = Get-ChildItem -LiteralPath $stageRoot -Recurse -File |
        Where-Object { $_.FullName -ne $checksumPath } |
        Sort-Object FullName |
        ForEach-Object {
            $relative = $_.FullName.Substring($stageRoot.Length + 1).Replace('\', '/')
            $hash = (Get-FileHash -LiteralPath $_.FullName -Algorithm SHA256).Hash.ToLowerInvariant()
            "$hash  $relative"
        }
    $checksums | Set-Content -LiteralPath $checksumPath -Encoding UTF8

    Compress-Archive -Path (Join-Path $stageRoot '*') -DestinationPath $outputZip -CompressionLevel Optimal
    $outputHash = (Get-FileHash -LiteralPath $outputZip -Algorithm SHA256).Hash.ToLowerInvariant()
    "$outputHash  $outputName" | Set-Content -LiteralPath $outputSha -Encoding ASCII

    Write-Host "AI handoff: $outputZip"
    Write-Host "SHA-256: $outputHash"
    Write-Host "Handoff source: $handoffCommit"
    Write-Host "Executable source: $ExecutableSourceCommit"
} finally {
    if (Test-Path -LiteralPath $stageRoot) {
        $finalStage = [IO.Path]::GetFullPath($stageRoot)
        if (-not $finalStage.StartsWith($resolvedTempBase, [StringComparison]::OrdinalIgnoreCase) -or
            -not (Split-Path $finalStage -Leaf).StartsWith('kuubik-draw-ai-handoff-')) {
            throw "Refusing to remove unsafe handoff staging path: $finalStage"
        }
        Remove-Item -LiteralPath $finalStage -Recurse -Force
    }
}
