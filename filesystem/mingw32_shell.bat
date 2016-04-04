:
@echo off

set "WD=%__CD__%"

if NOT EXIST "%WD%msys-2.0.dll" set "WD=%~dp0usr\bin\"

set MSYSTEM=MINGW32

rem To activate windows native symlinks uncomment next line
rem set MSYS=winsymlinks:nativestrict

rem Set debugging program for errors
rem set MSYS=error_start:%WD%../../mingw32/bin/qtcreator.exe^|-debug^|^<process-id^>

rem To export full current PATH from environment into MSYS2 uncomment next line
rem set SET_FULL_PATH=1

if "x%~1" == "x-consolez" shift& set MSYSCON=console.exe
if "x%~1" == "x-mintty" shift& set MSYSCON=mintty.exe

if "x%MSYSCON%" == "xmintty.exe" goto startmintty
if "x%MSYSCON%" == "xconsole.exe" goto startconsolez

if NOT EXIST "%WD%mintty.exe" goto startsh
set MSYSCON=mintty.exe
:startmintty
start "%MSYSTEM%" "%WD%mintty" -i /msys2.ico /usr/bin/bash --login %1 %2 %3 %4 %5 %6 %7 %8 %9
exit /b %ERRORLEVEL%

:startconsolez
cd %WD%..\lib\ConsoleZ
start console -t "MinGW" -r %1 %2 %3 %4 %5 %6 %7 %8 %9
exit /b %ERRORLEVEL%

:startsh
start "%MSYSTEM%" "%WD%sh" --login -i %1 %2 %3 %4 %5 %6 %7 %8 %9
exit /b %ERRORLEVEL%

:EOF
