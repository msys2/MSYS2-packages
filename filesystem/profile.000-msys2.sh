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
    local winver;
    winver="$(uname -s | sed -ne 's/\([^-]*\)-\([^-]*\).*/\2/p')"
    if [ "$winver" = "6.1" ] || [ "$winver" = "6.2" ]; then
        export __MSYS2_WINDOWS_VERSION_WARNING_DONE="true"
        printf "\e[1;33mThe MSYS2 project is planning to drop active support of Windows 7\e[1;0m\n" 1>&2;
        printf "\e[1;33mand 8.0 sometime during 2022. For more information visit\e[1;0m\n" 1>&2;
        printf "\e[1;33mhttps://www.msys2.org/docs/windows_support\e[1;0m\n" 1>&2;
    fi
}

_warn_deprecated_winver;
unset _warn_deprecated_winver;
