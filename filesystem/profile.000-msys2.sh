# Setting XDG_DATA_DIRS allows bash-completion to find completion files
# installed in the mingw package prefixes as well
if [ ! "${MINGW_PREFIX}" = "" ]; then
    XDG_DATA_DIRS="/usr/local/share/:/usr/share/"
    export XDG_DATA_DIRS="$MINGW_PREFIX/share/:$XDG_DATA_DIRS"
fi

# Warn the user on the first login shell in case we detect a too old Windows version
_warn_deprecated_winver()
{
    if [ "$__MSYS2_WINDOWS_VERSION_WARNING_DONE" = "true" ]; then
        return;
    fi

    local winver
    winver=$(uname -s)   # looks like `MINGW64_NT-10.0-22621`
    winver=${winver#*-}  # strip off `<prefix>-`
    winver=${winver%%-*} # strip off `-<suffix>`, if any
    if [ "$winver" = "6.1" ] || [ "$winver" = "6.2" ]; then
        export __MSYS2_WINDOWS_VERSION_WARNING_DONE="true"
        printf "\e[1;33mThe MSYS2 project no longer supports Windows 7 and 8.0.\e[1;0m\n" 1>&2;
        printf "\e[1;33mFor more information visit https://www.msys2.org/docs/windows_support\e[1;0m\n" 1>&2;
    fi
}

_warn_deprecated_winver;
unset _warn_deprecated_winver;
