@echo off
setlocal

powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0scripts\build-optiscaler.ps1" %*
exit /b %ERRORLEVEL%
