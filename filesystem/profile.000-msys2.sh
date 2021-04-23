# Setting XDG_DATA_DIRS allows bash-completion to find completion files
# installed in the mingw package prefixes as well
if [ ! "${MINGW_PREFIX}" = "" ]; then
    XDG_DATA_DIRS="/usr/local/share/:/usr/share/"
    export XDG_DATA_DIRS="$MINGW_PREFIX/share/:$XDG_DATA_DIRS"
fi
