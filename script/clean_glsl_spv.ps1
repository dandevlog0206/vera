<#
.SYNOPSIS
    Deletes all compiled .spv shader files from the output directory.

.DESCRIPTION
    This PowerShell script should be placed inside the root shader source directory,
    alongside the 'compile.ps1' script. It finds the sibling 'spv' directory
    and recursively removes all files ending with the .spv extension.

.NOTES
    Author: Gemini
    Version: 1.0
#>

# --- Script Configuration ---

# The script automatically determines its own location to find the shaders directory.
# $scriptPath = $PSScriptRoot
$inputPath = Join-Path $PSScriptRoot "..\shader"

$scriptPath = Resolve-Path -Path $inputPath
# Set the relative path to the output directory where compiled .spv files are stored.
# $outputDir = Join-Path $scriptPath "..\spv"
$outputDir = $scriptPath

# --- End Configuration ---

Write-Host "[INFO] Starting cleanup of compiled SPIR-V files..." -ForegroundColor Cyan
Write-Host "[INFO] Target directory: '$outputDir'"
Write-Host ""

# 1. Check if the output directory exists.
if (-not (Test-Path $outputDir)) {
    Write-Host "[SKIP] Output directory '$outputDir' not found. Nothing to clean." -ForegroundColor Yellow
    exit 0
}

# 2. Find all .spv files recursively within the output directory.
$spvFiles = Get-ChildItem -Path $outputDir -File -Recurse -Filter *.spv

if ($spvFiles.Count -eq 0) {
    Write-Host "[SKIP] No .spv files found to delete." -ForegroundColor Yellow
} else {
    Write-Host "[CLEAN] Deleting $($spvFiles.Count) .spv file(s)..." -ForegroundColor Green
    
    # 3. Loop through and delete each found .spv file.
    foreach ($file in $spvFiles) {
        Write-Host "  - Removing $($file.FullName)"
        Remove-Item -Path $file.FullName -Force
    }

    Write-Host ""
    Write-Host "[SUCCESS] Cleanup complete." -ForegroundColor Green
}

# 4. Optional: Clean up empty directories left behind after deleting files.
$subdirectories = Get-ChildItem -Path $outputDir -Recurse -Directory
# Sort by FullName length in descending order to process deepest directories first.
$sortedSubdirectories = $subdirectories | Sort-Object { $_.FullName.Length } -Descending

foreach ($dir in $sortedSubdirectories) {
    # Check if the directory is empty
    if ((Get-ChildItem -Path $dir.FullName).Count -eq 0) {
        Write-Host "  - Removing empty directory $($dir.FullName)"
        Remove-Item -Path $dir.FullName -Force
    }
}


Write-Host ""
Write-Host "===================================================" -ForegroundColor White
Write-Host "Cleanup script finished."
Write-Host "===================================================" -ForegroundColor White
Write-Host ""
