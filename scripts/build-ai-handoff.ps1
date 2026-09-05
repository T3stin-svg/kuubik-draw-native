[CmdletBinding()]
param(
    [string]$RepositoryRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path,
    [string]$Version = '0.2.0-preview.2',
    [string]$SourceRef = 'HEAD',
    [string]$ExecutableSourceCommit = '171d95915f6f5a34b8d9fcb487dd3429de8cda74',
    [string]$PortableZip,
    [string]$EvidenceZip,
    [switch]$DevelopmentPreview,
    [string]$OutputDirectory
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

function Assert-ArchiveChecksum([string]$Path) {
    $record = (Get-Content -LiteralPath "$Path.sha256" -Raw).Trim()
    if ($record -notmatch '^([a-fA-F0-9]{64})\s+') {
        throw "Invalid archive checksum record: $Path.sha256"
    }
    $expected = $Matches[1].ToLowerInvariant()
    $actual = (Get-FileHash -LiteralPath $Path -Algorithm SHA256).Hash.ToLowerInvariant()
    if ($actual -ne $expected) { throw "Archive checksum mismatch: $Path" }
}

$repository = (Resolve-Path -LiteralPath $RepositoryRoot).Path
$artifactRoot = if ([string]::IsNullOrWhiteSpace($OutputDirectory)) {
    Join-Path $repository 'artifacts'
} else { [IO.Path]::GetFullPath($OutputDirectory) }
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
Assert-ArchiveChecksum $PortableZip
Assert-ArchiveChecksum $EvidenceZip
Require-Path (Join-Path $repository 'AI_START.md')
Require-Path (Join-Path $repository 'AGENTS.md')
Require-Path (Join-Path $repository 'NEXT_TASKS.md')
Require-Path (Join-Path $repository 'docs\PROJECT_STATE.md')
Require-Path (Join-Path $repository 'docs\DECISIONS.md')
Require-Path (Join-Path $repository 'docs\ROADMAP.md')
Require-Path (Join-Path $repository 'docs\TEST_REPORT.md')

$handoffCommit = (& git -C $repository rev-parse $SourceRef).Trim()
Assert-GitSuccess 'git rev-parse source ref'
$resolvedExecutableCommit = (& git -C $repository rev-parse "$ExecutableSourceCommit^{commit}").Trim()
Assert-GitSuccess 'git rev-parse executable source'
if ($resolvedExecutableCommit -ne $ExecutableSourceCommit) {
    throw 'ExecutableSourceCommit must be the full immutable commit SHA'
}
& git -C $repository merge-base --is-ancestor $ExecutableSourceCommit $handoffCommit
Assert-GitSuccess 'Executable source must be an ancestor of the handoff snapshot'

Add-Type -AssemblyName System.IO.Compression.FileSystem
$portableArchive = [IO.Compression.ZipFile]::OpenRead((Resolve-Path -LiteralPath $PortableZip).Path)
try {
    $manifestEntries = @($portableArchive.Entries | Where-Object { $_.Name -eq 'build-manifest.json' })
    if ($manifestEntries.Count -ne 1) { throw 'Portable ZIP must contain exactly one build-manifest.json' }
    $reader = [IO.StreamReader]::new($manifestEntries[0].Open())
    try { $buildManifest = $reader.ReadToEnd() | ConvertFrom-Json } finally { $reader.Dispose() }
    if ($buildManifest.sourceCommit -ne $ExecutableSourceCommit -or $buildManifest.version -ne $Version) {
        throw 'Portable build manifest does not match the requested executable source/version'
    }
} finally { $portableArchive.Dispose() }

$releaseTag = $null
$checkpointKind = 'development-preview'
if (-not $DevelopmentPreview) {
    $releaseTag = "v$Version"
    $tagOutput = & git -C $repository rev-list -n 1 $releaseTag
    Assert-GitSuccess 'git rev-list release tag'
    $tagCommit = ([string]$tagOutput).Trim()
    if ($tagCommit -ne $ExecutableSourceCommit) {
        throw "Release tag commit $tagCommit does not match executable source $ExecutableSourceCommit"
    }
    $checkpointKind = 'release'
} elseif ([string]$buildManifest.githubRunId -notmatch '^\d+$') {
    throw 'Development preview must identify its Windows CI run in the portable manifest'
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

$outputName = if ($DevelopmentPreview) {
    "KuubikDraw-$Version-dev-$($ExecutableSourceCommit.Substring(0, 8))-exe-source-AI.zip"
} else { "KuubikDraw-$Version-exe-source-AI.zip" }
$outputZip = Join-Path $artifactRoot $outputName
$outputSha = "$outputZip.sha256"
if ((Test-Path -LiteralPath $outputZip) -or (Test-Path -LiteralPath $outputSha)) {
    throw "Refusing to overwrite an existing handoff artifact: $outputZip"
}

New-Item -ItemType Directory -Path $stageRoot | Out-Null
if (-not (Test-Path -LiteralPath $artifactRoot)) {
    New-Item -ItemType Directory -Path $artifactRoot | Out-Null
}
try {
    $binaryRoot = Join-Path $stageRoot 'BINARY'
    $sourceRoot = Join-Path $stageRoot 'SOURCE'
    $evidenceRoot = Join-Path $stageRoot 'EVIDENCE'
    New-Item -ItemType Directory -Path $binaryRoot,$sourceRoot,$evidenceRoot | Out-Null

    Copy-Item -LiteralPath $PortableZip,"$PortableZip.sha256" -Destination $binaryRoot
    Copy-Item -LiteralPath $EvidenceZip,"$EvidenceZip.sha256" -Destination $evidenceRoot

    $sourceZipName = "KuubikDraw-$Version-source-$($handoffCommit.Substring(0, 8)).zip"
    $sourceZip = Join-Path $sourceRoot $sourceZipName
    & git -C $repository archive --format=zip --output=$sourceZip $handoffCommit
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
    # Handoff prose must come from SourceRef too, never from a possibly dirty
    # working tree that disagrees with the source ZIP and manifest commit.
    $sourceArchive = [IO.Compression.ZipFile]::OpenRead($sourceZip)
    try {
        foreach ($entry in $sourceArchive.Entries) {
            if ($entry.Name.Length -eq 0 -or
                ($entry.FullName -notin $handoffFiles -and -not $entry.FullName.StartsWith('docs/'))) { continue }
            $target = [IO.Path]::GetFullPath((Join-Path $stageRoot $entry.FullName))
            if (-not $target.StartsWith($resolvedStage + [IO.Path]::DirectorySeparatorChar,
                                       [StringComparison]::OrdinalIgnoreCase)) {
                throw "Unsafe source archive member: $($entry.FullName)"
            }
            $targetParent = Split-Path $target -Parent
            if (-not (Test-Path -LiteralPath $targetParent)) {
                New-Item -ItemType Directory -Path $targetParent | Out-Null
            }
            [IO.Compression.ZipFileExtensions]::ExtractToFile($entry, $target, $false)
        }
        foreach ($relative in $handoffFiles) { Require-Path (Join-Path $stageRoot $relative) }
    } finally { $sourceArchive.Dispose() }

    $portableHash = (Get-FileHash -LiteralPath $PortableZip -Algorithm SHA256).Hash.ToLowerInvariant()
    $evidenceHash = (Get-FileHash -LiteralPath $EvidenceZip -Algorithm SHA256).Hash.ToLowerInvariant()
    $manifest = [ordered]@{
        schemaVersion = 2
        product = 'Kuubik Draw Native'
        version = $Version
        repository = 'https://github.com/T3stin-svg/kuubik-draw-native'
        defaultBranch = 'kuubik/visual-v0.2'
        handoffSourceCommit = $handoffCommit
        executableSourceCommit = $ExecutableSourceCommit
        checkpointKind = $checkpointKind
        releaseTag = $releaseTag
        githubRunId = $buildManifest.githubRunId
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
