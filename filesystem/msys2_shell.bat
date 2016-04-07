:
@echo off

rem To activate windows native symlinks uncomment next line
rem set MSYS=winsymlinks:nativestrict

rem Set debugging program for errors
rem set MSYS=error_start:%WD%../../mingw32/bin/qtcreator.exe^|-debug^|^<process-id^>

rem To export full current PATH from environment into MSYS2 uncomment next line
rem set MSYS2_PATH_TYPE=inherit

call "%~dp0start_shell.cmd" -msys %*
:EOF
