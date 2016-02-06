echo off
set MSYSTEM=MSYS
set id=%RANDOM%

if "%1" == "-c" (
    start C:\ProgramData\msys32\usr\bin\script -q -f -e -c '/usr/bin/bash.exe -l -c "echo $$ > /tmp/pid.%id% ; bash -e %* ; echo \$? > /tmp/exit_code.%id%"' /tmp/typescript.%id%
) else (
    start C:\ProgramData\msys32\usr\bin\script -q -f -e -c '/usr/bin/bash.exe -l -c "echo $$ > /tmp/pid.%id% ; bash -e `/usr/bin/cygpath -u \'%~f1\'` ; echo \$? > /tmp/exit_code.%id%"' /tmp/typescript.%id%
)

C:\ProgramData\msys32\usr\bin\bash -l -c "sleep 15"
C:\ProgramData\msys32\usr\bin\bash -l -c "tail -F /tmp/typescript.%id% --pid=`cat /tmp/pid.%id%` | script_clean.pl"
C:\ProgramData\msys32\usr\bin\bash -l -c "if test -f /tmp/exit_code.%id%; then cat /tmp/exit_code.%id%; (exit `cat /tmp/exit_code.%id%`); fi"
