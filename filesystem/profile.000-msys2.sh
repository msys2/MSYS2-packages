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

# Set the user id
# USER="$(exec id -un)" # Heavy
export USER="${USERNAME}"

# Default to removing the write permission for group and other
#  (files normally created with mode 777 become 755; files created with
#  mode 666 become 644)
umask 022

# Start MSYS in selected folder
# c:\msys\usr\bin\bash -c "cd '%curdir'; export
# CHERE_INVOKING=1; exec /usr/bin/bash --login -i"
#
# Make sure we start in home if invoked without -here option and not in a nested shell
if [ -n "${CHERE_INVOKING}" ]; then
    CHERE_INVOKING_VISIBLE_FOR_USER="${CHERE_INVOKING}"
    unset CHERE_INVOKING
elif [ ${SHLVL:-0} -le 1 ]; then
    cd "${HOME}" || echo "WARNING: Failed attempt to cd into ${HOME}!"
fi
