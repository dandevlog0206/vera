<#
.SYNOPSIS
    Recursively compiles GLSL shaders to Vulkan SPIR-V (.spv) using glslangValidator.

.DESCRIPTION
    This PowerShell script should be placed inside the root directory for your shader source files.
    It recursively scans for all glsl shader files and compiles them to a sibling 'spv' directory,
    preserving the original subdirectory structure.

    The script is efficient and will only compile a shader if:
    1. The destination .spv file does not exist.
    2. The source .glsl file has been modified more recently than the existing .spv file.

.NOTES
    Author: Gemini
    Version: 3.0
#>

# --- Script Configuration ---

# The script automatically determines its own location to find the shaders.
# $scriptPath = $PSScriptRoot
$inputPath = Join-Path $PSScriptRoot "..\shader"

$scriptPath = Resolve-Path -Path $inputPath
# Set the relative path to the output directory for compiled SPIR-V files.
# $outputDir = Join-Path $scriptPath "..\spv"
$outputDir = $scriptPath

# --- End Configuration ---

# 1. Check if glslangValidator.exe is available in the system's PATH.
if (-not (Get-Command glslangValidator.exe -ErrorAction SilentlyContinue)) {
    Write-Host "[ERROR] 'glslangValidator.exe' not found in your system's PATH." -ForegroundColor Red
    Write-Host "Please install the Vulkan SDK and ensure its 'Bin' directory is added to your PATH environment variable."
    Write-Host "Download SDK from: https://vulkan.lunarg.com/"
    Read-Host "Press Enter to exit..."
    exit 1
}

# 2. Create the root output directory if it doesn't already exist.
if (-not (Test-Path $outputDir)) {
    Write-Host "[INFO] Creating output directory: $outputDir" -ForegroundColor Cyan
    New-Item -ItemType Directory -Force -Path $outputDir | Out-Null
}

Write-Host "[INFO] Starting recursive shader compilation check..." -ForegroundColor Cyan
Write-Host "[INFO] Source: '$scriptPath' -> Destination: '$outputDir'"
Write-Host ""

$filesCompiled = 0
$filesSkipped = 0

# 3. Get all glsl shader files in the script's directory and all subdirectories.
$shaderFiles = Get-ChildItem -Path $scriptPath -File -Recurse -Include *glsl, *.vert, *geom, *.frag, *.comp

# 4. Loop through each shader file to check if it needs compilation.
foreach ($sourceFile in $shaderFiles) {
    # Determine the relative path of the source file to replicate the folder structure.
    $relativePath = $sourceFile.FullName.Substring($scriptPath.Length + 1)
    $destFile = "$relativePath.spv"
    $destSubDir = Split-Path -Path $destFile -Parent

    $doCompile = $false

    if (-not (Test-Path $destFile)) {
        # Condition 1: Compile because the destination .spv file doesn't exist.
        $doCompile = $true
    } else {
        # Condition 2: The .spv file exists. Compare modification timestamps.
        if ($sourceFile.LastWriteTime -gt (Get-Item $destFile).LastWriteTime) {
            $doCompile = $true
        }
    }

    # 5. Execute compilation if needed.
    if ($doCompile) {
        # Ensure the destination subdirectory exists before compiling.
        if (-not (Test-Path $destSubDir)) {
            New-Item -ItemType Directory -Force -Path $destSubDir | Out-Null
        }
        
        Write-Host "[COMPILE] Compiling $relativePath..." -ForegroundColor Green
        
        # We use Start-Process to get reliable exit codes and suppress output.
        $process = Start-Process -FilePath "glslangValidator.exe" -ArgumentList "-gVS --target-env vulkan1.3 -V -o `"$destFile`" `"$($sourceFile.FullName)`"" -Wait -PassThru -NoNewWindow
        
        if ($process.ExitCode -ne 0) {
            Write-Host "[ERROR] Failed to compile $relativePath. Halting script." -ForegroundColor Red
            Read-Host "Press Enter to exit..."
            exit 1
        }
        $filesCompiled++
    } else {
        Write-Host "[SKIP] $relativePath is up to date." -ForegroundColor Yellow
        $filesSkipped++
    }
}

# --- Final Summary ---
Write-Host ""
Write-Host "===================================================" -ForegroundColor White
Write-Host "[SUCCESS] Shader compilation check finished." -ForegroundColor Green
Write-Host "  - Compiled: $filesCompiled"
Write-Host "  - Skipped:  $filesSkipped"
Write-Host "===================================================" -ForegroundColor White
Write-Host ""

