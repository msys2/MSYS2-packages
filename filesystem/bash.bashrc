# To the extent possible under law, the author(s) have dedicated all
# copyright and related and neighboring rights to this software to the
# public domain worldwide. This software is distributed without any warranty.
# You should have received a copy of the CC0 Public Domain Dedication along
# with this software.
# If not, see <https://creativecommons.org/publicdomain/zero/1.0/>.

# /etc/bash.bashrc: executed by bash(1) for interactive shells.

# System-wide bashrc file

# If not running interactively, don't do anything
[[ "$-" != *i* ]] && return

# Check that we haven't already been sourced.
[[ -z "${CYG_SYS_BASHRC:-}" ]] && CYG_SYS_BASHRC="1" || return

# If started from sshd, make sure profile is sourced
if [[ -n "${SSH_CONNECTION:-}" ]] && [[ "$PATH" != *:/usr/bin* ]]; then
    source /etc/profile
fi

# If MSYS2_PS1 is set, use that as default PS1;
# if a PS1 is already set and exported, use that;
# otherwise set a default prompt
# of user@host, MSYSTEM variable, and current_directory
[[ -n "${MSYS2_PS1:-}" ]] && export PS1="${MSYS2_PS1}"
# if we have the "High Mandatory Level" group, it means we're elevated
if [[ -n "$(command -v getent)" ]] && id -G | grep -q "$(getent -w group 'S-1-16-12288' | cut -d: -f2)"
  then _ps1_symbol='\[\e[1m\]#\[\e[0m\]'
  else _ps1_symbol='\$'
fi
case "$(declare -p PS1 2>/dev/null)" in
'declare -x '*) ;; # okay
*)
  export PS1='\[\e]0;\w\a\]\n\[\e[32m\]\u@\h \[\e[35m\]$MSYSTEM\[\e[0m\] \[\e[33m\]\w\[\e[0m\]\n'"${_ps1_symbol}"' '
  ;;
esac
unset _ps1_symbol

# Uncomment to use the terminal colours set in DIR_COLORS
# eval "$(dircolors -b /etc/DIR_COLORS)"
