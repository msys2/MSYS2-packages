@echo off
setlocal EnableDelayedExpansion

set "WD=%__CD__%"
if NOT EXIST "%WD%msys-2.0.dll" set "WD=%~dp0usr\bin\"
set "LOGINSHELL=bash"
set /a msys2_shiftCounter=0

rem To activate windows native symlinks uncomment next line
rem set MSYS=winsymlinks:nativestrict

rem Set debugging program for errors
rem set MSYS=error_start:%WD%../../mingw64/bin/qtcreator.exe^|-debug^|^<process-id^>

rem To export full current PATH from environment into MSYS2 use '-use-full-path' parameter
rem or uncomment next line
rem set MSYS2_PATH_TYPE=inherit

:checkparams
rem Help option
if "x%~1" == "x-help" (
  call :printhelp "%~nx0"
  exit /b %ERRORLEVEL%
)
if "x%~1" == "x--help" (
  call :printhelp "%~nx0"
  exit /b %ERRORLEVEL%
)
if "x%~1" == "x-?" (
  call :printhelp "%~nx0"
  exit /b %ERRORLEVEL%
)
if "x%~1" == "x/?" (
  call :printhelp "%~nx0"
  exit /b %ERRORLEVEL%
)
rem Shell types
if "x%~1" == "x-msys" shift& set /a msys2_shiftCounter+=1& set MSYSTEM=MSYS& goto :checkparams
if "x%~1" == "x-msys2" shift& set /a msys2_shiftCounter+=1& set MSYSTEM=MSYS& goto :checkparams
if "x%~1" == "x-mingw32" shift& set /a msys2_shiftCounter+=1& set MSYSTEM=MINGW32& goto :checkparams
if "x%~1" == "x-mingw64" shift& set /a msys2_shiftCounter+=1& set MSYSTEM=MINGW64& goto :checkparams
if "x%~1" == "x-ucrt64" shift& set /a msys2_shiftCounter+=1& set MSYSTEM=UCRT64& goto :checkparams
if "x%~1" == "x-clang64" shift& set /a msys2_shiftCounter+=1& set MSYSTEM=CLANG64& goto :checkparams
if "x%~1" == "x-clang32" shift& set /a msys2_shiftCounter+=1& set MSYSTEM=CLANG32& goto :checkparams
if "x%~1" == "x-clangarm64" shift& set /a msys2_shiftCounter+=1& set MSYSTEM=CLANGARM64& goto :checkparams
if "x%~1" == "x-mingw" shift& set /a msys2_shiftCounter+=1& (if exist "%WD%..\..\mingw64" (set MSYSTEM=MINGW64) else (set MSYSTEM=MINGW32))& goto :checkparams
rem Console types
if "x%~1" == "x-mintty" shift& set /a msys2_shiftCounter+=1& set MSYSCON=mintty.exe& goto :checkparams
if "x%~1" == "x-conemu" shift& set /a msys2_shiftCounter+=1& set MSYSCON=conemu& goto :checkparams
if "x%~1" == "x-defterm" shift& set /a msys2_shiftCounter+=1& set MSYSCON=defterm& goto :checkparams
rem Other parameters
if "x%~1" == "x-full-path" shift& set /a msys2_shiftCounter+=1& set MSYS2_PATH_TYPE=inherit& goto :checkparams
if "x%~1" == "x-use-full-path" shift& set /a msys2_shiftCounter+=1& set MSYS2_PATH_TYPE=inherit& goto :checkparams
if "x%~1" == "x-here" shift& set /a msys2_shiftCounter+=1& set CHERE_INVOKING=enabled_from_arguments& goto :checkparams
if "x%~1" == "x-where" (
  if "x%~2" == "x" (
    echo Working directory is not specified for -where parameter. 1>&2
    exit /b 2
  )
  cd /d "%~2" || (
    echo Cannot set specified working diretory "%~2". 1>&2
    exit /b 2
  )
  set CHERE_INVOKING=enabled_from_arguments

  rem Ensure parentheses in argument do not interfere with FOR IN loop below.
  set msys2_arg="%~2"
  call :substituteparens msys2_arg
  call :removequotes msys2_arg

  rem Increment msys2_shiftCounter by number of words in argument (as cmd.exe saw it).
  rem (Note that this form of FOR IN loop uses same delimiters as parameters.)
  for %%a in (!msys2_arg!) do set /a msys2_shiftCounter+=1
)& shift& shift& set /a msys2_shiftCounter+=1& goto :checkparams
if "x%~1" == "x-no-start" shift& set /a msys2_shiftCounter+=1& set MSYS2_NOSTART=yes& goto :checkparams
if "x%~1" == "x-shell" (
  if "x%~2" == "x" (
    echo Shell not specified for -shell parameter. 1>&2
    exit /b 2
  )
  set LOGINSHELL="%~2"
  call :removequotes LOGINSHELL
  
  set msys2_arg="%~2"
  call :substituteparens msys2_arg
  call :removequotes msys2_arg
  for %%a in (!msys2_arg!) do set /a msys2_shiftCounter+=1
)& shift& shift& set /a msys2_shiftCounter+=1& goto :checkparams

rem Collect remaining command line arguments to be passed to shell
if %msys2_shiftCounter% equ 0 set SHELL_ARGS=%* & goto cleanvars
set msys2_full_cmd=%*
for /f "tokens=%msys2_shiftCounter%,* delims=,;=	 " %%i in ("!msys2_full_cmd!") do set SHELL_ARGS=%%j

:cleanvars
set msys2_arg=
set msys2_shiftCounter=
set msys2_full_cmd=

rem Setup proper title and icon
if "%MSYSTEM%" == "MINGW32" (
  set "CONTITLE=MinGW x32"
  set "CONICON=mingw32.ico"
) else if "%MSYSTEM%" == "MINGW64" (
  set "CONTITLE=MinGW x64"
  set "CONICON=mingw64.ico"
) else if "%MSYSTEM%" == "UCRT64" (
  set "CONTITLE=MinGW UCRT x64"
  set "CONICON=ucrt64.ico"
) else if "%MSYSTEM%" == "CLANG64" (
  set "CONTITLE=MinGW Clang x64"
  set "CONICON=clang64.ico"
) else if "%MSYSTEM%" == "CLANG32" (
  set "CONTITLE=MinGW Clang x32"
  set "CONICON=clang32.ico"
) else if "%MSYSTEM%" == "CLANGARM64" (
  set "CONTITLE=MinGW Clang ARM64"
  set "CONICON=clangarm64.ico"
) else (
  set "CONTITLE=MSYS2 MSYS"
  set "CONICON=msys2.ico"
)

if "x%MSYSCON%" == "xmintty.exe" goto startmintty
if "x%MSYSCON%" == "xconemu" goto startconemu
if "x%MSYSCON%" == "xdefterm" goto startsh

if NOT EXIST "%WD%mintty.exe" goto startsh
set MSYSCON=mintty.exe
:startmintty
if not defined MSYS2_NOSTART (
  start "%CONTITLE%" "%WD%mintty" -i "/%CONICON%" -t "%CONTITLE%" "/usr/bin/%LOGINSHELL%" -l !SHELL_ARGS!
) else (
  "%WD%mintty" -i "/%CONICON%" -t "%CONTITLE%" "/usr/bin/%LOGINSHELL%" -l !SHELL_ARGS!
)
exit /b %ERRORLEVEL%

:startconemu
call :conemudetect || (
  echo ConEmu not found. Exiting. 1>&2
  exit /b 1
)
if not defined MSYS2_NOSTART (
  start "%CONTITLE%" "%ComEmuCommand%" /Here /Icon "%WD%..\..\%CONICON%" /cmd "%WD%\%LOGINSHELL%" -l !SHELL_ARGS!
) else (
  "%ComEmuCommand%" /Here /Icon "%WD%..\..\%CONICON%" /cmd "%WD%\%LOGINSHELL%" -l !SHELL_ARGS!
)
exit /b %ERRORLEVEL%

:startsh
set MSYSCON=
if not defined MSYS2_NOSTART (
  start "%CONTITLE%" "%WD%\%LOGINSHELL%" -l !SHELL_ARGS!
) else (
  "%WD%\%LOGINSHELL%" -l !SHELL_ARGS!
)
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

:printhelp
echo Usage:
echo     %~1 [options] [login shell parameters]
echo.
echo Options:
echo     -mingw32 ^| -mingw64 ^| -ucrt64 ^| -clang64 ^| -msys[2]   Set shell type
echo     -defterm ^| -mintty ^| -conemu                            Set terminal type
echo     -here                            Use current directory as working
echo                                      directory
echo     -where DIRECTORY                 Use specified DIRECTORY as working
echo                                      directory
echo     -[use-]full-path                 Use full current PATH variable
echo                                      instead of trimming to minimal
echo     -no-start                        Do not use "start" command and
echo                                      return login shell resulting 
echo                                      errorcode as this batch file 
echo                                      resulting errorcode
echo     -shell SHELL                     Set login shell
echo     -help ^| --help ^| -? ^| /?         Display this help and exit
echo.
echo Any parameter that cannot be treated as valid option and all
echo following parameters are passed as login shell command parameters.
echo.
exit /b 0

:removequotes
FOR /F "delims=" %%A IN ('echo %%%1%%') DO set %1=%%~A
GOTO :eof

:substituteparens
SETLOCAL
FOR /F "delims=" %%A IN ('echo %%%1%%') DO (
    set value=%%A
    set value=!value:^(=x!
    set value=!value:^)=x!
)
ENDLOCAL & set %1=%value%
GOTO :eof
