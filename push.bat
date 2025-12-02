@echo off
REM push.bat — double-click to run push.ps1 with ExecutionPolicy bypass
REM Place this file next to push.ps1 inside your repository folder.

REM Change to the directory where this batch file is located
cd /d "%~dp0"

REM Optional: a short message that will be passed to the script.
REM You can edit the message below, or leave it blank to let push.ps1 create a timestamped message.
set COMMIT_MSG=Updated SRMS code

REM Run the PowerShell script with bypass. The -NoProfile avoids loading user profile.
powershell -NoProfile -ExecutionPolicy Bypass -Command ^
    "& { Set-Location '%~dp0'; .\push.ps1 -Message \"%COMMIT_MSG%\" }"

REM Show a pause so you can read the output window
echo.
echo Press any key to close...
pause >nul
