# To the extent possible under law, the author(s) have dedicated all 
# copyright and related and neighboring rights to this software to the 
# public domain worldwide. This software is distributed without any warranty. 
# You should have received a copy of the CC0 Public Domain Dedication along 
# with this software. 
# If not, see <https://creativecommons.org/publicdomain/zero/1.0/>. 

# ~/.profile: executed by the command interpreter for login shells.

# The copy in your home directory (~/.profile) is yours, please
# feel free to customise it to create a shell
# environment to your liking.  If you feel a change
# would be benificial to all, please feel free to send
# a patch to the msys2 mailing list.

# User dependent .profile file

# Set user-defined locale
export LANG=$(locale -uU)

# This file is not read by bash(1) if ~/.bash_profile or ~/.bash_login
# exists.
#
# if running bash
if [ -n "${BASH_VERSION}" ]; then
  if [ -f "${HOME}/.bashrc" ]; then
    source "${HOME}/.bashrc"
  fi
fi

agm2() {
  local MINGW_DIRS="mingw32 mingw64"
  local AG_FIND=
  
  for dir in ${MINGW_DIRS}; do
    if type -p /${dir}/bin/ag >/dev/null; then
      AG_FIND=/${dir}/bin/ag
    fi
  done
  
  if ! type -p /usr/bin/git >/dev/null; then
    echo "bash: git: command not found. Please install \"git\" package."
    exit 1
  fi
  
  if [ -n "$AG_FIND" ]; then
    $AG_FIND --makepkg --depth 1 "$@" $(git rev-parse --show-toplevel)
  else
    echo "bash: ag: conmmand not found. Please install \"mingw-w64-i686-ag\" or \"mingw-w64-x86_64-ag\" package."
    exit 1
  fi
}
