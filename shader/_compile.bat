@echo off
REM This batch file embeds a PowerShell script to compile GLSL source code into SPIR-V files.
REM It calls PowerShell internally, bypassing the need for a separate .ps1 file
REM and handling the execution policy automatically for this single run.

powershell -ExecutionPolicy Bypass -NoProfile -File ../script/compile_glsl_spv.ps1
pause