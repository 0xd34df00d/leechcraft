@echo off
if "%1"=="" goto :help
if "%2"=="" goto :help

SET ARCHIVER="7za.exe"

md %TEMP%\%1
%ARCHIVER% x -o"%TEMP%\%1" "%1"
%ARCHIVER% x -o"%2" "%TEMP%\%1\*"
rmdir /s /q "%TEMP%\%1"
goto :EOF

:help
echo Usage: xtract.cmd ^<filename^> ^<directory^>
echo     where <filename> is tar.gz archive.
