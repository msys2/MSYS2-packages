@echo off

set "WD=%__CD__%"
if NOT EXIST "%WD%msys-2.0.dll" set "WD=%~dp0usr\bin\"

rem To activate windows native symlinks uncomment next line
rem set MSYS=winsymlinks:nativestrict

rem Set debugging program for errors
rem set MSYS=error_start:%WD%../../mingw64/bin/qtcreator.exe^|-debug^|^<process-id^>

rem To export full current PATH from environment into MSYS2 use '-use-full-path' parameter
rem or uncomment next line
rem set MSYS2_PATH_TYPE=inherit

:checkparams
rem Shell types
if "x%~1" == "x-msys" shift& set MSYSTEM=MSYS& goto :checkparams
if "x%~1" == "x-msys2" shift& set MSYSTEM=MSYS& goto :checkparams
if "x%~1" == "x-mingw32" shift& set MSYSTEM=MINGW32& goto :checkparams
if "x%~1" == "x-mingw64" shift& set MSYSTEM=MINGW64& goto :checkparams
if "x%~1" == "x-mingw" shift& (
  if exist "%WD%..\..\mingw64" (set MSYSTEM=MINGW64) else (set MSYSTEM=MINGW32)
)& goto :checkparams
rem Console types
if "x%~1" == "x-consolez" shift& set MSYSCON=console.exe& goto :checkparams
if "x%~1" == "x-mintty" shift& set MSYSCON=mintty.exe& goto :checkparams
if "x%~1" == "x-conemu" shift& set MSYSCON=conemu& goto :checkparams
if "x%~1" == "x-defterm" shift& set MSYSCON=defterm& goto :checkparams
rem Other parameters
if "x%~1" == "x-use-full-path" shift& set MSYS2_PATH_TYPE=inherit& goto :checkparams


rem Autodetect shell type if not specified
if not defined MSYSTEM (
   if exist "%WD%..\..\mingw64" (
     set MSYSTEM=MINGW64
   ) else if exist "%WD%..\..\mingw32" (
     set MSYSTEM=MINGW32
   ) else (set MSYSTEM=MSYS)
)

rem Setup proper title
if "%MSYSTEM%" == "MSYS" (
  set CONTITLE=MSys2
) else if "%MSYSTEM%" == "MINGW32" (
  set "CONTITLE=MinGW x32"
) else if "%MSYSTEM%" == "MINGW64" (
  set "CONTITLE=MinGW x64"
)

if "x%MSYSCON%" == "xmintty.exe" goto startmintty
if "x%MSYSCON%" == "xconsole.exe" goto startconsolez
if "x%MSYSCON%" == "xconemu" goto startconemu
if "x%MSYSCON%" == "xdefterm" goto startsh

if NOT EXIST "%WD%mintty.exe" goto startsh
set MSYSCON=mintty.exe
:startmintty
start "%CONTITLE%" "%WD%mintty" -i /msys2.ico /usr/bin/bash --login %1 %2 %3 %4 %5 %6 %7 %8 %9
exit /b %ERRORLEVEL%

:startconsolez
cd %WD%..\lib\ConsoleZ
start console -t "%CONTITLE%" -r %1 %2 %3 %4 %5 %6 %7 %8 %9
exit /b %ERRORLEVEL%

:startconemu
call :conemudetect || (
  echo ConEmu not found. Exiting. 1>&2
  exit /b 1
)
start "%CONTITLE%" "%ComEmuCommand%" /Here /Icon "%WD%..\..\msys2.ico" /cmd %WD%bash --login %1 %2 %3 %4 %5 %6 %7 %8 %9
exit /b %ERRORLEVEL%

:startsh
set MSYSCON=
start "%CONTITLE%" "%WD%bash" --login %1 %2 %3 %4 %5 %6 %7 %8 %9
exit /b %ERRORLEVEL%

:EOF
exit /b 0

:conemudetect
set ComEmuCommand=
if defined ConEmuDir (
  if exist "%ConEmuDir%\ConEmu64.exe" (
    set "ComEmuCommand=%ConEmuDir%\ConEmu64.exe"
    set MSYSCON=conemu64.exe
  ) else if exist "%ConEmuDir%\ConEmu.exe" (
    set "ComEmuCommand=%ConEmuDir%\ConEmu.exe"
    set MSYSCON=conemu.exe
  )
)
if not defined ComEmuCommand (
  ConEmu64.exe /Exit 2>nul && (
    set ComEmuCommand=ConEmu64.exe
    set MSYSCON=conemu64.exe
  ) || (
    ConEmu.exe /Exit 2>nul && (
      set ComEmuCommand=ConEmu.exe
      set MSYSCON=conemu.exe
    )
  )
)
if not defined ComEmuCommand (
  FOR /F "tokens=*" %%A IN ('reg.exe QUERY "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\App Paths\ConEmu64.exe" /ve 2^>nul ^| find "REG_SZ"') DO (
    set "ComEmuCommand=%%A"
  )
  if defined ComEmuCommand (
    call set "ComEmuCommand=%%ComEmuCommand:*REG_SZ    =%%"
    set MSYSCON=conemu64.exe
  ) else (
    FOR /F "tokens=*" %%A IN ('reg.exe QUERY "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\App Paths\ConEmu.exe" /ve 2^>nul ^| find "REG_SZ"') DO (
      set "ComEmuCommand=%%A"
    )
    if defined ComEmuCommand (
      call set "ComEmuCommand=%%ComEmuCommand:*REG_SZ    =%%"
      set MSYSCON=conemu.exe
    )
  )
)
if not defined ComEmuCommand exit /b 2
exit /b 0
