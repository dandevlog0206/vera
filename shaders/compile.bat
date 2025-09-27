@echo off
setlocal enabledelayedexpansion

REM ============================================================================
REM GLSL to SPIR-V Batch Compiler (using glslangValidator)
REM
REM This script should be placed INSIDE the directory with your shaders.
REM It compiles all .vert and .frag files from the current directory
REM to a sibling directory (e.g., ../spv). It only compiles a file if:
REM 1. The destination .spv file does not exist.
REM 2. The source .glsl file has been modified more recently than the
REM    existing .spv file.
REM ============================================================================


REM --- Configuration ---
REM The script assumes it's in the shader source directory.
set SHADER_DIR=.

REM Set the relative path to the output directory.
set OUTPUT_DIR=.
REM --- End Configuration ---


REM 1. Check if glslangValidator.exe is available in the system's PATH.
where glslangValidator >nul 2>nul
if %errorlevel% neq 0 (
    echo [ERROR] 'glslangValidator.exe' not found in your system's PATH.
    echo.
    echo Please install the Vulkan SDK and ensure the 'Bin' directory
    echo is added to your system's PATH environment variable.
    echo Download the SDK from: https://vulkan.lunarg.com/
    echo.
    pause
    exit /b 1
)

REM 2. Create the output directory if it doesn't already exist.
if not exist "%OUTPUT_DIR%" (
    echo [INFO] Creating output directory: %OUTPUT_DIR%
    mkdir "%OUTPUT_DIR%"
)

echo [INFO] Starting shader compilation check...
echo [INFO]      Source: %SHADER_DIR%
echo [INFO] Destination: %OUTPUT_DIR%
echo.

set "files_compiled=0"
set "files_skipped=0"

REM 3. Loop through all .vert and .frag files in the current directory.
for %%f in ("%SHADER_DIR%\*.vert.glsl", "%SHADER_DIR%\*.frag.glsl") do (
    set "source_file=%%~f"
    set "dest_file=%OUTPUT_DIR%\%%~nxf.spv"

    REM 4. Decide whether to compile the file.
    set "do_compile=0"
    if not exist "!dest_file!" (
        REM Compile because the destination .spv file doesn't exist.
        set "do_compile=1"
    ) else (
        REM The .spv file exists. The original xcopy trick was flawed because
        REM it only compares files with the same name. We will use a more
        REM reliable PowerShell command to compare the timestamps of the
        REM .glsl source and its corresponding .spv destination file.
        powershell -NoProfile -Command "if((Get-Item -LiteralPath '!source_file!').LastWriteTime -gt (Get-Item -LiteralPath '!dest_file!').LastWriteTime) { exit 0 } else { exit 1 }" >nul
        if !errorlevel! equ 0 (
            REM PowerShell exited with 0, meaning the source is newer. Compile.
            set "do_compile=1"
        )
    )

    REM 5. Execute compilation if needed.
    if !do_compile! equ 1 (
        echo [COMPILE] Compiling %%~nxf...
        glslangValidator.exe -V -o "!dest_file!" "!source_file!"
        
        REM Check if compilation was successful.
        if !errorlevel! neq 0 (
            echo [ERROR] Failed to compile %%~nxf. Halting script.
            pause
            exit /b 1
        )
        set /a files_compiled+=1
    ) else (
        echo [SKIP] %%~nxf is up to date.
        set /a files_skipped+=1
    )
)

echo.
echo ===================================================
echo [SUCCESS] Shader compilation check finished.
echo   - Compiled: %files_compiled%
echo   - Skipped:  %files_skipped%
echo ===================================================
echo.

pause

endlocal
