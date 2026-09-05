[CmdletBinding()]
param()
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
Add-Type -AssemblyName System.IO.Compression.FileSystem
$builder = Join-Path $PSScriptRoot 'build-ai-handoff.ps1'
$testBase = [IO.Path]::GetFullPath([IO.Path]::GetTempPath()).TrimEnd('\') + '\'
$testRoot = [IO.Path]::GetFullPath((Join-Path $testBase ('kuubik-handoff-test-' + [Guid]::NewGuid().ToString('N'))))
if (-not $testRoot.StartsWith($testBase, [StringComparison]::OrdinalIgnoreCase)) { throw 'Unsafe test directory' }

function Write-Fixture([string]$Path, [string]$Text) {
    [IO.File]::WriteAllText($Path, $Text, [Text.UTF8Encoding]::new($false))
}
function Write-Checksum([string]$Path) {
    Write-Fixture "$Path.sha256" ((Get-FileHash -LiteralPath $Path -Algorithm SHA256).Hash.ToLowerInvariant() + '  ' + (Split-Path $Path -Leaf))
}
function Assert-Rejected([scriptblock]$Operation, [string]$Message) {
    $rejected = $false
    try { & $Operation } catch {
        if ($_.Exception.Message -notlike "*$Message*") { throw }
        $rejected = $true
    }
    if (-not $rejected) { throw "Expected rejection: $Message" }
}

New-Item -ItemType Directory -Path $testRoot | Out-Null
try {
    $repository = Join-Path $testRoot 'repository'
    $package = Join-Path $testRoot 'package'
    $evidence = Join-Path $testRoot 'evidence'
    New-Item -ItemType Directory -Path $repository,$package,$evidence | Out-Null
    New-Item -ItemType Directory -Path (Join-Path $repository 'docs') | Out-Null
    $rootFiles = @('AI_START.md','AGENTS.md','NEXT_TASKS.md','PROMPT_FOR_NEXT_AI.md','README.md',
                  'README_TEST.md','FORK_NOTICE.md','LICENSE','THIRD_PARTY_NOTICES.md')
    $docFiles = @('PROJECT_STATE.md','DECISIONS.md','ROADMAP.md','TEST_REPORT.md')
    foreach ($name in $rootFiles) { Write-Fixture (Join-Path $repository $name) "Synthetic committed fixture: $name" }
    foreach ($name in $docFiles) { Write-Fixture (Join-Path $repository "docs/$name") "Synthetic committed fixture: $name" }
    & git -C $repository init --quiet
    if ($LASTEXITCODE -ne 0) { throw 'Fixture git init failed' }
    & git -C $repository add -- $rootFiles docs
    if ($LASTEXITCODE -ne 0) { throw 'Fixture git add failed' }
    & git -C $repository -c user.name='Kuubik fixture' -c user.email='fixture@example.invalid' commit --quiet -m 'Synthetic handoff fixture'
    if ($LASTEXITCODE -ne 0) { throw 'Fixture git commit failed' }
    $sourceCommit = (& git -C $repository rev-parse HEAD).Trim()
    $version = '0.0.0-handoff-test'
    $buildManifest = @{ sourceCommit = $sourceCommit; version = $version; githubRunId = '123'; syntheticFixture = $true }
    Write-Fixture (Join-Path $package 'build-manifest.json') ($buildManifest | ConvertTo-Json)
    Write-Fixture (Join-Path $evidence 'synthetic.json') '{"syntheticFixture":true}'
    $portableZip = Join-Path $testRoot 'portable.zip'
    $evidenceZip = Join-Path $testRoot 'evidence.zip'
    [IO.Compression.ZipFile]::CreateFromDirectory($package, $portableZip)
    [IO.Compression.ZipFile]::CreateFromDirectory($evidence, $evidenceZip)
    Write-Checksum $portableZip
    Write-Checksum $evidenceZip
    $arguments = @{ RepositoryRoot = $repository; Version = $version; SourceRef = 'HEAD';
                    ExecutableSourceCommit = $sourceCommit; DevelopmentPreview = $true;
                    PortableZip = $portableZip; EvidenceZip = $evidenceZip;
                    OutputDirectory = (Join-Path $testRoot 'output') }
    # Deliberately dirty prose must not leak into the exact-source handoff.
    Write-Fixture (Join-Path $repository 'AI_START.md') 'UNCOMMITTED: must not enter the archive'
    & $builder @arguments
    $archivePath = @(Get-ChildItem -LiteralPath $arguments.OutputDirectory -Filter '*.zip')
    if ($archivePath.Count -ne 1) { throw 'Expected exactly one development handoff' }
    $archive = [IO.Compression.ZipFile]::OpenRead($archivePath[0].FullName)
    try {
        $reader = [IO.StreamReader]::new($archive.GetEntry('HANDOFF_MANIFEST.json').Open())
        try { $manifest = $reader.ReadToEnd() | ConvertFrom-Json } finally { $reader.Dispose() }
        if ($manifest.checkpointKind -ne 'development-preview' -or $null -ne $manifest.releaseTag -or
            $manifest.executableSourceCommit -ne $sourceCommit -or $manifest.githubRunId -ne '123') {
            throw 'Development handoff metadata is incorrect'
        }
        $reader = [IO.StreamReader]::new($archive.GetEntry('AI_START.md').Open())
        try { $prose = $reader.ReadToEnd() } finally { $reader.Dispose() }
        if ($prose -ne 'Synthetic committed fixture: AI_START.md') { throw 'Dirty working prose entered handoff' }
    } finally { $archive.Dispose() }
    Assert-Rejected { & $builder @arguments } 'Refusing to overwrite'
    Write-Fixture "$evidenceZip.sha256" (('0' * 64) + '  evidence.zip')
    Assert-Rejected { & $builder @arguments } 'Archive checksum mismatch'
    Write-Checksum $evidenceZip
    $arguments.Version = '0.0.0-wrong-version'
    Assert-Rejected { & $builder @arguments } 'Portable build manifest does not match'
    $arguments.Version = $version
    $arguments.ExecutableSourceCommit = $sourceCommit.Substring(0, 8)
    Assert-Rejected { & $builder @arguments } 'full immutable commit SHA'
    $arguments.ExecutableSourceCommit = $sourceCommit
    # The release path still requires a tag. This tag exists only in the
    # disposable synthetic test repository, never in the product checkout.
    & git -C $repository tag "v$version"
    if ($LASTEXITCODE -ne 0) { throw 'Fixture tag failed' }
    $arguments.DevelopmentPreview = $false
    & $builder @arguments
    $releasePath = Join-Path $arguments.OutputDirectory "KuubikDraw-$version-exe-source-AI.zip"
    $releaseArchive = [IO.Compression.ZipFile]::OpenRead($releasePath)
    try {
        $reader = [IO.StreamReader]::new($releaseArchive.GetEntry('HANDOFF_MANIFEST.json').Open())
        try { $release = $reader.ReadToEnd() | ConvertFrom-Json } finally { $reader.Dispose() }
        if ($release.checkpointKind -ne 'release' -or $release.releaseTag -ne "v$version") {
            throw 'Existing release-mode contract changed'
        }
    } finally { $releaseArchive.Dispose() }
    Write-Host 'PASS: development/release handoff, immutable prose, no overwrite, checksum/version/source rejection'
} finally {
    $finalRoot = [IO.Path]::GetFullPath($testRoot)
    if (-not $finalRoot.StartsWith($testBase, [StringComparison]::OrdinalIgnoreCase) -or
        -not (Split-Path $finalRoot -Leaf).StartsWith('kuubik-handoff-test-')) { throw 'Unsafe test cleanup' }
    Remove-Item -LiteralPath $finalRoot -Recurse -Force
}
