:
@echo off

if NOT "x%WD%" == "x" set WD=

if NOT EXIST %WD%msys-2.0.dll set WD=%~dp0usr\bin\

set MSYSTEM=MSYS

rem To activate windows native symlinks uncomment next line
rem set MSYS=winsymlinks:nativestrict

rem Set debugging program for errors
rem set MSYS=error_start:%WD%../../mingw32/bin/qtcreator.exe^|-debug^|^<process-id^>

rem To export full current PATH from environment into MSYS2 uncomment next line
rem set SET_FULL_PATH=1

set MSYSCON=mintty.exe
if "x%1" == "x-consolez" set MSYSCON=console.exe
if "x%1" == "x-mintty" set MSYSCON=mintty.exe

if "x%MSYSCON%" == "xmintty.exe" goto startmintty
if "x%MSYSCON%" == "xconsole.exe" goto startconsolez

:startmintty
if NOT EXIST %WD%mintty.exe goto startsh
start %WD%mintty -i /msys2.ico /usr/bin/bash --login %*
exit

:startconsolez
cd %WD%..\lib\ConsoleZ
start console -t "MSys2" -r "%*"
exit

:startsh
start %WD%sh --login -i %*
exit

:EOF
