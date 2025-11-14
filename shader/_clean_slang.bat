@echo off
REM This batch file embeds a PowerShell script to clean up compiled SPIR-V files.
REM It calls PowerShell internally, bypassing the need for a separate .ps1 file
REM and handling the execution policy automatically for this single run.

powershell -ExecutionPolicy Bypass -NoProfile -File ../script/clean_slang_spv.ps1
pause