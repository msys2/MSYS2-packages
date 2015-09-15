@echo off
REM CMD shell integration for MSYS2
REM
REM Copyright (C) 2015 Stefan Zimmermann <zimmermann.code@gmail.com>
REM
REM Licensed under the Apache License, Version 2.0

%~dp0\bash.bat -c "pacman --color auto %*"
