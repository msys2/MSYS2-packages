@echo off

set dir=%~dp0
set dll=%dir%usr\bin\msys-2.0.dll
powershell /c "Get-Process | Where-Object {($_.Modules | Where-Object {$_.FileName -like '"%dll%"'}).Length -gt 0} | Stop-Process -Force"

set PATH=%~dp0\usr\bin;%PATH%
%~dp0\usr\bin\dash /usr/bin/rebaseall -p
