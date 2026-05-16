param(
    [string]$Version = "4.0.0",
    [string]$OutputDir = "Dist"
)

$ErrorActionPreference = "Stop"

$Root = Resolve-Path (Join-Path $PSScriptRoot "..")
$PackageName = "HelsincyCrosshair-v$Version"
$StageRoot = Join-Path $Root $OutputDir
$StageDir = Join-Path $StageRoot $PackageName
$ZipPath = Join-Path $StageRoot "$PackageName.zip"

$ExcludedRootNames = @(
    ".git",
    ".vs",
    ".vscode",
    ".idea",
    ".codex",
    ".superpowers",
    "Binaries",
    "Intermediate",
    "Saved",
    "DerivedDataCache",
    "BuildPluginPackage",
    "Package",
    "Dist"
)

$ExcludedFileNames = @(
    "progress.md",
    "task_plan.md",
    "findings.md"
)

if (Test-Path $StageDir) {
    Remove-Item -LiteralPath $StageDir -Recurse -Force
}

New-Item -ItemType Directory -Force -Path $StageDir | Out-Null

Get-ChildItem -LiteralPath $Root -Force | ForEach-Object {
    if ($ExcludedRootNames -contains $_.Name) {
        return
    }

    if (-not $_.PSIsContainer -and $ExcludedFileNames -contains $_.Name) {
        return
    }

    $Destination = Join-Path $StageDir $_.Name
    Copy-Item -LiteralPath $_.FullName -Destination $Destination -Recurse -Force
}

if (-not (Test-Path (Join-Path $StageDir "Content"))) {
    throw "Package validation failed: Content directory is missing."
}

if (-not (Test-Path (Join-Path $StageDir "Docs"))) {
    throw "Package validation failed: Docs directory is missing."
}

foreach ($ForbiddenFile in $ExcludedFileNames) {
    if (Test-Path (Join-Path $StageDir $ForbiddenFile)) {
        throw "Package validation failed: forbidden file '$ForbiddenFile' was included."
    }
}

foreach ($ForbiddenDir in @("Binaries", "Intermediate", "Saved", "DerivedDataCache")) {
    if (Test-Path (Join-Path $StageDir $ForbiddenDir)) {
        throw "Package validation failed: forbidden Unreal cache directory '$ForbiddenDir' was included."
    }
}

if (Test-Path $ZipPath) {
    Remove-Item -LiteralPath $ZipPath -Force
}

Compress-Archive -Path (Join-Path $StageDir "*") -DestinationPath $ZipPath -Force

Write-Output "Release package created: $ZipPath"
