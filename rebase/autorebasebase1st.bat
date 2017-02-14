@echo off
REM Copyright (c) 2014, Ray Donnelly <mingw.android@gmail.com>

set PATH=%~dp0\usr\bin;%PATH%

echo Querying 'base' packages for DLLs ..
%~dp0\usr\bin\bash /usr/bin/pacman-rec-filename-grep base base-dlls-unix.txt ".*\.(dll|so|oct)$"
%~dp0\usr\bin\bash /usr/bin/paths-from-unix-to-windows base-dlls-unix.txt base-dlls.txt %CD:\=/%

echo Rebasing all DLLs, 'base' ones first ..
REM   -i is new; it means ignore database and
REM   rebase from end of {cygwin1,msys-2.0}.dll
REM   it work by avoiding passing -s to rebase.exe
REM   which has modifications to support this mode.
%~dp0\usr\bin\dash /usr/bin/rebaseall -T base-dlls.txt -i -p
