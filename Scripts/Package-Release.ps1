param(
    [string]$Version = "4.0.2",
    [string]$OutputDir = "Dist"
)

$ErrorActionPreference = "Stop"

$Root = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$PackageName = "HelsincyCrosshair-v$Version"
$StageRoot = Join-Path $Root $OutputDir
$RootFull = [System.IO.Path]::GetFullPath($Root)
$StageRootFull = [System.IO.Path]::GetFullPath($StageRoot)
$RootPrefix = $RootFull.TrimEnd('\', '/') + [System.IO.Path]::DirectorySeparatorChar

if (-not (($StageRootFull + [System.IO.Path]::DirectorySeparatorChar).StartsWith($RootPrefix, [System.StringComparison]::OrdinalIgnoreCase))) {
    throw "OutputDir must resolve inside the plugin root."
}

$StageRoot = $StageRootFull
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

$ExcludedRelativePaths = @(
    (Join-Path "Docs" "superpowers")
)

$RequiredPackagePaths = @(
    "HelsincyCrosshair.uplugin",
    "Source",
    "Content",
    "Config",
    "Docs",
    "Resources",
    "Image",
    "README.md",
    "README_EN.md",
    "CHANGELOG.md",
    (Join-Path "Docs" "Release-Notes-v$Version.md")
)

if (Test-Path $StageDir) {
    Remove-Item -LiteralPath $StageDir -Recurse -Force
}

New-Item -ItemType Directory -Force -Path $StageDir | Out-Null

Get-ChildItem -LiteralPath $Root -Force | ForEach-Object {
    if ($ExcludedRootNames -contains $_.Name) {
        return
    }

    if ($_.PSIsContainer -and $_.Name -like "ue_hitmarker_frames_*") {
        return
    }

    if (-not $_.PSIsContainer -and $ExcludedFileNames -contains $_.Name) {
        return
    }

    if (-not $_.PSIsContainer -and $_.Name -like "ue_*.png") {
        return
    }

    $Destination = Join-Path $StageDir $_.Name
    Copy-Item -LiteralPath $_.FullName -Destination $Destination -Recurse -Force
}

foreach ($ExcludedRelativePath in $ExcludedRelativePaths) {
    $StagedExcludedPath = Join-Path $StageDir $ExcludedRelativePath
    if (Test-Path $StagedExcludedPath) {
        Remove-Item -LiteralPath $StagedExcludedPath -Recurse -Force
    }
}

foreach ($RequiredPath in $RequiredPackagePaths) {
    if (-not (Test-Path (Join-Path $StageDir $RequiredPath))) {
        throw "Package validation failed: required path '$RequiredPath' is missing."
    }
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

foreach ($ForbiddenRelativePath in $ExcludedRelativePaths) {
    if (Test-Path (Join-Path $StageDir $ForbiddenRelativePath)) {
        throw "Package validation failed: forbidden path '$ForbiddenRelativePath' was included."
    }
}

if (Get-ChildItem -LiteralPath $StageDir -Force | Where-Object { $_.Name -like "ue_*.png" -or $_.Name -like "ue_hitmarker_frames_*" }) {
    throw "Package validation failed: temporary UE debug captures were included."
}

if (Test-Path $ZipPath) {
    Remove-Item -LiteralPath $ZipPath -Force
}

Compress-Archive -Path (Join-Path $StageDir "*") -DestinationPath $ZipPath -Force

Write-Output "Release package created: $ZipPath"
