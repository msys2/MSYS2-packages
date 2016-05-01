@echo off

REM Copyright (c) 2014, Ray Donnelly <mingw.android@gmail.com>

if "%1"=="" (
  echo Please re-run, passing the the location of an existing MSYS2 installation
  exit /b 1
)

if not exist %1\msys2_shell.cmd (
  echo '%1' does not seem to be the root folder of an existing MSYS2 installation?!
  exit /b 2
)

set "NEWMSYS=%CD:\=/%/newmsys/msys64"
set "NEWMSYSW=%CD%\newmsys\msys64"

echo.
echo ***********
echo * Warning *
echo ***********
echo.
echo This batch file will sync (forcibly) the 'base' group of the MSYS2 installation in '%1'
<nul set /p =.. Do you want to proceed? 
choice /N
if "%errorlevel%"=="1" (
  echo.
) else (
  exit /b 0
)

REM set "PATH=%1\bin;%PATH%"

echo Updating 'base' group of the existing MSYS2 installation
pushd %1
  REM %1\bin\bash -l -c "pacman -S base --force --noconfirm" | more /E /P
  timeout 1 > NUL
popd

echo Creating new MSYS2 installation containing only 'base'
timeout 1 > NUL
pushd %1
  mkdir %NEWMSYSW%\var\lib\pacman
  echo bin\bash -l -c "pacman -Syu --root %NEWMSYS%"
  bin\bash -l -c "pacman -Syu --root %NEWMSYS%"
  exit /b 999
  %1\bin\bash -l -c "pacman -S pacman --root %NEWMSYS%"
  %1\bin\bash -l -c "pacman -S base --force --root %NEWMSYS%" | more /E /P
  timeout 1 > NUL
popd

exit /b 0
