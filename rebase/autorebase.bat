@echo off
rem Postinstall scripts are always started from the Cygwin root dir
rem so we can just call dash from here
cd %~dp0
path .\bin;%path%
dash /bin/rebaseall -p
