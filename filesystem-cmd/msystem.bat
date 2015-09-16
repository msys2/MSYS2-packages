@echo off
REM CMD shell integration for MSYS2
REM
REM Copyright (C) 2015 Stefan Zimmermann <zimmermann.code@gmail.com>
REM
REM Licensed under the Apache License, Version 2.0


if "%~1" == "/?" (
    echo Activates an MSYS2 environment inside a CMD shell,
    echo either in MSYS or MINGW32 or MINGW64 mode,
    echo by prepending the relevant MSYS2 bin\ paths to %%PATH%%.
    echo.
    echo MSYSTEM [MSYS ^| MINGW32 ^| MINGW64]
    echo.
    echo The MINGW modes do NOT include the MSYS mode bin\ paths.
    echo.
    echo In MSYS mode, an additional path containing wrapper scripts
    echo for BASH and PACMAN is prepended to %%PATH%%.
    echo.
    echo To work properly, this script must be located
    echo in the cmd\ sub-directory of the MSYS2 installation,
    echo or the root directory of the MSYS2 installation
    echo ^(usually C:\path\to\msys32 or ...\msys64^)
    echo must be defined in %%MSYS2_ROOT%%.
    echo.
    echo After successful activation,
    echo the environment name will be stored in %%MSYS2_SYSTEM%%
    echo and an according ^<MSYS^> or ^<MINGW32^> or ^<MINGW64^> tag
    echo will be prepended to %%PROMPT%%.
    echo.
    echo Call MSYSTEM without an argument
    echo to show the currently active MSYS2 environment.
    echo.
    echo Call MSYSTEM /D to deactivate an MSYS2 environment.
    echo.
    echo Be careful when using MSYSTEM together with other shell environments
    echo ^(like virtual Python environments, etc...^).
    echo Due to %%PATH%% changes,
    echo the precedence of commands with same names also changes.
    echo Future versions of MSYSTEM will try to automatically avoid conflicts
    echo with certain other environment types.
    echo.
    echo Call MSYSTEM /I with any of the following specifiers
    echo to install MSYS2 features into certain shell extension frameworks.
    echo.
    echo Install CLINK auto-completion for MSYSTEM and PACMAN with:
    echo.
    echo     MSYSTEM /I CLINK [clink settings directory]
    echo.
    echo If no CLINK settings directory is given,
    echo MSYSTEM will try to automatically find it.
    exit /b 0
)

REM --------------------------------------------------------------------------
REM No args

if "%~1" == "" (
    REM only show info and exit
    if "%MSYS2_SYSTEM%" == "" (
        echo No active MSYS2 environment.
        exit /b 1
    )
    echo %MSYS2_SYSTEM%
    exit /b 0
)

REM --------------------------------------------------------------------------
REM Handle install flag

if /i "%~1" == "/I" (
    REM install MSYS2 features into CMD shell extension frameworks
    goto install
)


REM ==========================================================================
REM Environment management

setlocal EnableDelayedExpansion

REM --------------------------------------------------------------------------
REM Handle environment deactivation flag

if /i "%~1" == "/D" (
    set MSYS2_SYSTEM=
    goto deactivate
)

REM --------------------------------------------------------------------------
REM Check for valid MSYS2 environment name and set %MSYS2_SYSTEM% accordingly

for %%M in (MSYS MINGW32 MINGW64) do if /i "%~1" == "%%M" (
    set MSYS2_SYSTEM=%%M
    goto prepare
)
echo Invalid argument '%~1'. Call MSYSTEM /? for help.
exit /b 1

REM --------------------------------------------------------------------------
REM Check system and create variables for MSYS2 (de)activation
:prepare

if not "%MSYS2_ROOT%" == "" (
    goto customRoot
)

REM no %MSYS2_ROOT% ==> assume that this script is in <MSYS2 root>\cmd\
set "cmdRoot=%~dp0"
REM remove trailing \ and cmd subdir
for %%D in ("%cmdRoot:~0,-1%") do (
    REM will not be exported on endlocal
    set "MSYS2_ROOT=%%~dpD"
)
REM remove trailing \
set "MSYS2_ROOT=%MSYS2_ROOT:~0,-1%"
if not exist "%MSYS2_ROOT%\msys2_shell.bat" (
    echo This script is not properly installed.
    echo Should be in ^<MSYS2 root^>\cmd\
    exit /b 1
)
goto deactivate

:customRoot

if not exist "%MSYS2_ROOT%\" (
    echo %%MSYS2_ROOT%% '%MSYS2_ROOT%' does not exist or is not a directory.
    exit /b 1
)

REM --------------------------------------------------------------------------
REM Always deactivate current MSYS2 environment before activating a new one
:deactivate

REM remove any existing MSYS2 and MINGW32/64 bin paths from %PATH%
if not "%MSYS2_PATH%" == "" (
    call set "PATH=%%PATH:%MSYS2_PATH%;=%%"
)
if not "%MSYS2_MINGW32_PATH%" == "" (
    call set "PATH=%%PATH:%MSYS2_MINGW32_PATH%;=%%"
)
if not "%MSYS2_MINGW64_PATH%" == "" (
    call set "PATH=%%PATH:%MSYS2_MINGW64_PATH%;=%%"
)
REM and also remove the extra MSYS tool wrapper scripts path
call set "PATH=%%PATH:%~dp0msys;=%%"

REM remove any <MSYS> or <MINGW32/64> tags from %PROMPT%
REM (use temporary %_cleanPrompt% to avoid too many %PROMPT% changes,
REM  which can result in strange shell behavior)
set "_cleanPrompt=%PROMPT:$LMSYS$G$S=%"
set "_cleanPrompt=%_cleanPrompt:$LMINGW32$G$S=%"
set "_cleanPrompt=%_cleanPrompt:$LMINGW64$G$S=%"

REM check if only deactivation requested (via MSYSTEM /D)
if "%MSYS2_SYSTEM%" == "" (
    set "_prompt=%_cleanPrompt%"

    set MSYS2_PATH=
    set MSYS2_MINGW32_PATH=
    set MSYS2_MINGW64_PATH=

    goto end
)

REM --------------------------------------------------------------------------
:activate

REM construct MSYS2 and MINGW32/64 bin paths
set "MSYS2_PATH=%MSYS2_ROOT%\usr\local\bin;%MSYS2_ROOT%\usr\bin;%MSYS2_ROOT%\bin"
set "MSYS2_MINGW32_PATH=%MSYS2_ROOT%\mingw32\bin"
set "MSYS2_MINGW64_PATH=%MSYS2_ROOT%\mingw64\bin"

REM prepend MSYS2 bin paths and/or MINGW32/64 bin paths to %PATH%
if "%MSYS2_SYSTEM%" == "MSYS" (
    REM also prepend the extra MSYS tool wrapper scripts path
    set "PATH=%~dp0msys;%MSYS2_PATH%;%PATH%"
)
if "%MSYS2_SYSTEM%" == "MINGW32" (
    set "PATH=%MSYS2_MINGW32_PATH%;%PATH%"
)
if "%MSYS2_SYSTEM%" == "MINGW64" (
    set "PATH=%MSYS2_MINGW64_PATH%;%PATH%"
)

REM prepend the appropriate <MSYS> or <MINGW32/64> tag to prompt
set "_prompt=$L%MSYS2_SYSTEM%$G$S%_cleanPrompt%"

REM --------------------------------------------------------------------------
REM Successfully finished MSYS2 environment (de)activation
:end

REM export new environment variables
endlocal & set "MSYS2_SYSTEM=%MSYS2_SYSTEM%" ^
         & set "MSYS2_PATH=%MSYS2_PATH%" ^
         & set "MSYS2_MINGW32_PATH=%MSYS2_MINGW32_PATH%" ^
         & set "MSYS2_MINGW64_PATH=%MSYS2_MINGW64_PATH%" ^
         & set "PATH=%PATH%" ^
         & set "PROMPT=%_prompt%"
exit /b 0


REM ==========================================================================
REM Install MSYS2 features into CMD shell extension frameworks
:install

if "%2" == "" (
    echo Missing specifier. Call MSYSTEM /? for help.
    exit /b 1
)
if /i "%2" == "clink" goto clink


REM --------------------------------------------------------------------------
REM Install MSYS2 auto-completion features into CLINK
:clink

setlocal EnableDelayedExpansion

if "%~3" == "" (
    REM no clink settings dir given ==> try to find
    set "clinkDir=%LOCALAPPDATA%\clink"
    if not exist "!clinkDir!\" (
        echo Could not find CLINK settings directory.
        echo Please provide it as additional argument.
        exit /b 1
    )
) else (
    if not exist "%~3\" (
        echo '%~3' does not exist or is not a directory.
        exit /b 1
    )
    set "clinkDir=%~3"
)

set nl=^


set "root=%~dp0"
REM remove trailing \
set "root=%root:~0,-1%"
REM create a LUA script that loads all auto-completion scripts from .\clink\
set msysLua=                                                             !nl!^
    local root = "%root:\=\\%"                                           !nl!^
    local scripts = {}                                                   !nl!^
    local p = io.popen("dir /b " .. root .. "\\clink\\*.lua")            !nl!^
    for file in p:lines() do                                             !nl!^
        table.insert(scripts, file)                                      !nl!^
    end                                                                  !nl!^
    if p:close() then                                                    !nl!^
       for _, file in next, scripts do                                   !nl!^
           dofile(root .. "\\clink\\" .. file)                           !nl!^
       end                                                               !nl!^
    end                                                                  !nl!^


echo Writing '%clinkDir%\msys2.lua'
echo !msysLua! > %clinkDir%\msys2.lua

endlocal
exit /b 0
