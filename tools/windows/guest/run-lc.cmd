@echo off
rem Starts the LeechCraft that build.ps1 -Install put into C:\lc\inst\<Qt version>, for Qt %1
rem (default 6.11.3). Until packaging collects the third-party DLLs, they come from their install
rem locations. A GUI needs the console session, so run this from a scheduled task, see README.md.
setlocal
set QT=%~1
if "%QT%"=="" set QT=6.11.3
set PATH=C:\lc\inst\%QT%;C:\Qt\%QT%\msvc2022_64\bin;C:\vcpkg\installed\x64-windows\bin;C:\lc\deps\%QT%\bin;%GSTREAMER_1_0_ROOT_MSVC_X86_64%\bin;%PATH%
rem A GUI subsystem executable has no console, and Qt would log to the debugger instead.
set QT_FORCE_STDERR_LOGGING=1
if not exist C:\lc\logs mkdir C:\lc\logs
cd /d C:\lc\inst\%QT%
leechcraft-qt6.exe > C:\lc\logs\lc-run-%QT%.log 2>&1
