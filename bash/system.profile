# To the extent possible under law, the author(s) have dedicated all 
# copyright and related and neighboring rights to this software to the 
# public domain worldwide. This software is distributed without any warranty. 
# You should have received a copy of the CC0 Public Domain Dedication along 
# with this software. 
# If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 


# System-wide profile file

# Some resources...
# Customizing Your Shell: http://www.dsl.org/cookbook/cookbook_5.html#SEC69
# Consistent BackSpace and Delete Configuration:
#   http://www.ibb.net/~anne/keyboard.html
# The Linux Documentation Project: http://www.tldp.org/
# The Linux Cookbook: http://www.tldp.org/LDP/linuxcookbook/html/
# Greg's Wiki http://mywiki.wooledge.org/

# Setup some default paths. Note that this order will allow user installed
# software to override 'system' software.
# Modifying these default path settings can be done in different ways.
# To learn more about startup files, refer to your shell's man page.
PATH="/usr/local/bin:/usr/bin:${PATH}"
MANPATH="/usr/local/man:/usr/share/man:/usr/man:${MANPATH}"
INFOPATH="/usr/local/info:/usr/share/info:/usr/info:${INFOPATH}"
MAYBE_FIRST_START=false
SYSCONFDIR="${SYSCONFDIR:=/etc}"

# TMP and TEMP as defined in the Windows environment must be kept
# for windows apps, even if started from cygwin. However, leaving
# them set to the default Windows temporary directory or unset
# can have unexpected consequences for cygwin apps, so we define 
# our own to match GNU/Linux behaviour.
ORIGINAL_TMP=$TMP
ORIGINAL_TEMP=$TEMP
unset TMP TEMP
tmp=$(cygpath -w "$ORIGINAL_TMP" 2> /dev/null)
temp=$(cygpath -w "$ORIGINAL_TEMP" 2> /dev/null)
TMP="/tmp"
TEMP="/tmp"

# Define default printer
p='/proc/registry/HKEY_CURRENT_USER/Software/Microsoft/Windows NT/CurrentVersion/Windows/Device'
if [ -e "${p}" ] ; then
  read -r PRINTER < "${p}" 
  PRINTER=${PRINTER%%,*}
fi
unset p

print_flags ()
{
  (( $1 & 0x0002 )) && echo -n "binary" || echo -n "text"
  (( $1 & 0x0010 )) && echo -n ",exec"
  (( $1 & 0x0040 )) && echo -n ",cygexec"
  (( $1 & 0x0100 )) && echo -n ",notexec"
}

maybe_create_fstab ()
{
  local FSTAB="${SYSCONFDIR}/fstab"
  local FSTABDIR="${SYSCONFDIR}/fstab.d"
  
  # Create fstab file if it doesn't exist.
  if [ -e "${FSTAB}" -a ! -f "${FSTAB}" ]
  then
    # Try to move
    mv -f "${FSTAB}" "${FSTAB}.orig"
    if [ -e "${FSTAB}" -a ! -f "${FSTAB}" ]
    then
      echo
      echo "${FSTAB} is existant but not a file."
      echo "Since this file is specifying the mount points, this might"
      echo "result in unexpected trouble.  Please fix that manually."
      echo
    fi
  fi

  if [ ! -e "${FSTAB}" ]
  then

    # Create fstab default header
    cat > ${FSTAB} << EOF
# For a description of the file format, see the Users Guide
# http://cygwin.com/cygwin-ug-net/using.html#mount-table

# DO NOT REMOVE NEXT LINE. It remove cygdrive prefix from path
none / cygdrive binary,posix=0,noacl,user 0 0
EOF
    MAYBE_FIRST_START=true
  fi

  # Check for ${FSTABDIR} directory
  if [ -e "${FSTABDIR}" -a ! -d "${FSTABDIR}" ]
  then 
    # No mercy.  Try to remove.
    rm -f "${FSTABDIR}"
    if [ -e "${FSTABDIR}" -a ! -d "${FSTABDIR}" ]
    then 
      echo
      echo "${FSTABDIR} is existant but not a directory."
      echo "Please fix that manually."
      echo
      exit 1
    fi
  fi

  # Create it if necessary
  if [ ! -e "${FSTABDIR}" ]
  then
    mkdir -m 1777 "${FSTABDIR}"
    if [ ! -e "${FSTABDIR}" ]
    then
      echo
      echo "Creating ${FSTABDIR} directory failed."
      echo "Please fix that manually."
      echo
      exit 1
    fi
  fi
}

maybe_create_mtab ()
{
  local MTAB="${SYSCONFDIR}/mtab"
  # Create /etc/mtab as symlink to /proc/mounts
  [ ! -L "${MTAB}" ] && ln -sf /proc/mounts ${MTAB}
}

maybe_create_devs ()
{
  local DEVDIR=/dev
  # Check for ${DEVDIR} directory
  if [ -e "${DEVDIR}" -a ! -d "${DEVDIR}" ]
  then 
    # No mercy.  Try to remove.
    rm -f "${DEVDIR}"
    if [ -e "${DEVDIR}" -a ! -d "${DEVDIR}" ]
    then 
      echo
      echo "${DEVDIR} is existant but not a directory."
      echo "Please fix that manually, otherwise you WILL get problems."
      echo
      exit 1
    fi
  fi

  # Create it if necessary
  mkdir -m 755 "${DEVDIR}" 2> /dev/null
  if [ ! -e "${DEVDIR}" ]
  then
    echo
    echo "Creating ${DEVDIR} directory failed."
    echo "Please fix that manually, otherwise you WILL get problems."
    echo
    exit 1
  fi

  # Check for ${DEVDIR}/shm directory (for POSIX semaphores and POSIX shared mem)
  if [ -e "${DEVDIR}/shm" -a ! -d "${DEVDIR}/shm" ]
  then
    # No mercy.  Try to remove.
    rm -f "${DEVDIR}/shm"
    if [ -e "${DEVDIR}/shm" -a ! -d "${DEVDIR}/shm" ]
    then 
      echo
      echo "${DEVDIR}/shm is existant but not a directory."
      echo "POSIX semaphores and POSIX shared memory will not work"
      echo
    fi
  fi

  # Create it if necessary
  if [ ! -e "${DEVDIR}/shm" ]
  then
    mkdir -m 1777 "${DEVDIR}/shm"
    if [ ! -e "${DEVDIR}/shm" ]
    then
      echo
      echo "Creating ${DEVDIR}/shm directory failed."
      echo "POSIX semaphores and POSIX shared memory will not work"
      echo
    fi
  else
    chmod 1777 "${DEVDIR}/shm"
  fi

  # Check for ${DEVDIR}/mqueue directory (for POSIX message queues)
  if [ -e "${DEVDIR}/mqueue" -a ! -d "${DEVDIR}/mqueue" ]
  then
    # No mercy.  Try to remove.
    rm -f "${DEVDIR}/mqueue"
    if [ -e "${DEVDIR}/mqueue" -a ! -d "${DEVDIR}/mqueue" ]
    then 
      echo
      echo "${DEVDIR}/mqueue is existant but not a directory."
      echo "POSIX message queues will not work"
      echo
    fi
  fi

  # Create it if necessary
  if [ ! -e "${DEVDIR}/mqueue" ]
  then
    mkdir -m 1777 "${DEVDIR}/mqueue"
    if [ ! -e "${DEVDIR}/mqueue" ]
    then
      echo
      echo "Creating ${DEVDIR}/mqueue directory failed."
      echo "POSIX message queues will not work"
      echo
    fi
  else
    chmod 1777 "${DEVDIR}/mqueue"
  fi
  
  # Install /dev/fd, /dev/std{in,out,err}.  The bash builtin test was compiled
  # to assume these exist, so use /bin/test to really check.
  /bin/test -h /dev/stdin  || ln -sf /proc/self/fd/0 /dev/stdin
  /bin/test -h /dev/stdout || ln -sf /proc/self/fd/1 /dev/stdout
  /bin/test -h /dev/stderr || ln -sf /proc/self/fd/2 /dev/stderr
  /bin/test -h /dev/fd     || ln -sf /proc/self/fd   /dev/fd
}


maybe_create_passwd ()
{
  # Create default /etc/passwd and /etc/group files
  local created_passwd=no
  local created_group=no

  if [ ! -e /etc/passwd -a ! -L /etc/passwd ] ; then
    mkpasswd.exe -l -c > /etc/passwd
    chmod 644 /etc/passwd
    created_passwd=yes
    MAYBE_FIRST_START=true
  fi

  if [ ! -e /etc/group -a ! -L /etc/group ] ; then
    mkgroup.exe -l -c > /etc/group
    chmod 644 /etc/group
    created_group=yes
    MAYBE_FIRST_START=true
  fi

  cp -fp /etc/group /tmp/group.mkgroup && \
  ( [ -w /etc/group ] || chmod --silent a+w /etc/group ; ) && \
  echo "root:S-1-5-32-544:0:" > /etc/group && \
  sed -e '/root:S-1-5-32-544:0:/d' /tmp/group.mkgroup >> /etc/group && \
  chmod --silent --reference=/etc/passwd /etc/group
  rm -f /tmp/group.mkgroup

   # Deferred to be sure root group entry exists
  [ "$created_passwd" = "yes" ] && chgrp --silent root /etc/passwd
  [ "$created_group" = "yes"  ] && chgrp --silent root /etc/group
}

maybe_create_winetc ()
{
  local FILES="hosts protocols services networks"
  local WINSYS32HOME="$(/usr/bin/cygpath -S -w)"
  local WINETC="${WINSYS32HOME}\\drivers\\etc"

  if [ ! -d "${WINETC}" ]; then
    echo "Directory ${WINETC} does not exist; exiting" >&2
    echo "If directory name is garbage you need to update your msys package" >&2
    exit 1
  fi

  local mketc=
  for mketc in ${FILES}
  do
    if [ ! -e "/etc/${mketc}" -a ! -L "/etc/${mketc}" ]
    then
      # Windows only uses the first 8 characters
      local WFILE="${WINETC}\\$(expr substr "${mketc}" 1 8)"
      /usr/bin/cp -p -v "${WFILE}" "/etc/${mketc}"
    fi
  done

  /usr/bin/chmod 1777 /tmp 2>/dev/null
}


maybe_create_home ()
{
  # Set the user id
  USER="$(id -un)"

  # Default to removing the write permission for group and other
  #  (files normally created with mode 777 become 755; files created with
  #  mode 666 become 644)
  umask 022

  # Here is how HOME is set, in order of priority, when starting from Windows
  #  1) From existing HOME in the Windows environment, translated to a Posix path
  #  2) from /etc/passwd, if there is an entry with a non empty directory field
  #  3) from HOMEDRIVE/HOMEPATH
  #  4) / (root)
  # If the home directory doesn't exist, create it.
  if [ ! -d "${HOME}" ]; then
    if mkdir -p "${HOME}"; then
      echo "Copying skeleton files."
      echo "These files are for the users to personalise their cygwin experience."
      echo
      echo "They will never be overwritten nor automatically updated."
      echo
      cd /etc/skel || echo "WARNING: Failed attempt to cd into /etc/skel!"
      local f=
      /usr/bin/find . -type f | while read f; do
        local fDest=${f#.}
        if [ ! -e "${HOME}${fDest}" -a ! -L "${HOME}${fDest}" ]; then
          /usr/bin/install -D -p -v "${f}" "${HOME}/${fDest}"
        fi
      done
    else
      echo "${HOME} could not be created."
      { [ -d "${TEMP}" ] && HOME="${TEMP}"; } ||
      { [ -d "${TMP}" ] && HOME="${TMP}"; } ||
      { [ -d /tmp ] && HOME=/tmp; } ||
      HOME=/
      echo "Setting HOME to ${HOME}."
    fi
  fi

  # Make sure we start in home unless invoked by CHERE
  if [ ! -z "${CHERE_INVOKING}" ]; then
    unset CHERE_INVOKING
  else
    cd "${HOME}" || echo "WARNING: Failed attempt to cd into ${HOME}!"
  fi
}

# Call functions
maybe_create_fstab
maybe_create_mtab
maybe_create_devs
maybe_create_passwd
maybe_create_winetc
maybe_create_home

# Shell dependent settings
profile_d ()
{
  local file=
  for file in $(export LC_COLLATE=C; echo /etc/profile.d/*.$1); do
    [ -e "${file}" ] && . "${file}"
  done
}

if [ ! "x${BASH_VERSION}" = "x"  ]; then
  HOSTNAME="$(/usr/bin/hostname)"
  profile_d sh
  [ -f "/etc/bash.bashrc" ] && . "/etc/bash.bashrc"
elif [ ! "x${KSH_VERSION}" = "x" ]; then
  typeset -l HOSTNAME="$(/usr/bin/hostname)"
  profile_d sh
  PS1=$(print '\033]0;${PWD}\n\033[32m${USER}@${HOSTNAME} \033[33m${PWD/${HOME}/~}\033[0m\n$ ')
elif [ ! "x${ZSH_VERSION}" = "x" ]; then
  HOSTNAME="$(/usr/bin/hostname)"
  profile_d zsh
  PS1='(%n@%m)[%h] %~ %% '
elif [ ! "x${POSH_VERSION}" = "x" ]; then
  HOSTNAME="$(/usr/bin/hostname)"
  PS1="$ "
else 
  HOSTNAME="$(/usr/bin/hostname)"
  profile_d sh
  PS1="$ "
fi

export PATH MANPATH INFOPATH USER TMP TEMP PRINTER HOSTNAME PS1 SHELL tmp temp
export TERM=xterm-256color

# Check to see if mkpasswd/mkgroup needs to be run try and cut down the emails
#   about this on the lists!
#
# The following are the conditions under which the group name special cases 
#   will appear (where uid and gid are the effective user and group ids
#   for the current user, and pgsid is the primary group associated with the
#   SID for the current user):
#       mkpasswd:
#         gid is not in /etc/group
#         uid is not in /etc/passwd
#       passwd/group_GID_clash(<gid>/<pgsid>):
#         gid is not in /etc/group
#         uid is in /etc/passwd
#         pgsid is in /etc/group (and does not match gid)
#       mkgroup:
#         gid is not in /etc/group
#         uid is in /etc/passwd
#         pgsid is not in /etc/group
#
# If this message keeps appearing and you are sure it's a mistake (ie, don't
#   email about it!), comment out the test below.
case "$(id -ng)" in
mkpasswd )
  echo "Your group is currently \"mkpasswd\".  This indicates that your"
  echo "gid is not in /etc/group and your uid is not in /etc/passwd."
  echo
  echo "The /etc/passwd (and possibly /etc/group) files should be rebuilt."
  echo "See the man pages for mkpasswd and mkgroup then, for example, run"
  echo
  echo "mkpasswd -l [-d] > /etc/passwd"
  echo "mkgroup  -l [-d] > /etc/group"
  echo
  echo "Note that the -d switch is necessary for domain users."
  ;;
passwd/group_GID_clash* )
  echo "Your group is currently \"passwd/group_GID_clash(gid/pgsid)\".  This"
  echo "indicates that your gid is not in /etc/group, but the pgsid (primary "
  echo "group associated with your SID) is in /etc/group."
  echo
  echo "The /etc/passwd (and possibly /etc/group) files should be rebuilt."
  echo "See the man pages for mkpasswd and mkgroup then, for example, run"
  echo
  echo "mkpasswd -l [-d] > /etc/passwd"
  echo "mkgroup  -l [-d] > /etc/group"
  echo
  echo "Note that the -d switch is necessary for domain users."
  ;;
mkgroup )
  echo "Your group is currently \"mkgroup\".  This indicates that neither"
  echo "your gid nor your pgsid (primary group associated with your SID)"
  echo "is in /etc/group."
  echo
  echo "The /etc/group (and possibly /etc/passwd) files should be rebuilt."
  echo "See the man pages for mkpasswd and mkgroup then, for example, run"
  echo
  echo "mkpasswd -l [-d] > /etc/passwd"
  echo "mkgroup  -l [-d] > /etc/group"
  echo
  echo "Note that the -d switch is necessary for domain users."
  ;;
esac



if [ "$MAYBE_FIRST_START" = "true" ]; then
  clear
  echo
  echo
  echo "###################################################################"
  echo "#                                                                 #"
  echo "#                                                                 #"
  echo "#                   C   A   U   T   I   O   N                     #"
  echo "#                                                                 #"
  echo "#                  This is first start of MSYS2.                  #"
  echo "#       You MUST restart shell to apply necessary actions.        #"
  echo "#                                                                 #"
  echo "#                                                                 #"
  echo "###################################################################"
  echo
  echo
fi
unset MAYBE_FIRST_START

