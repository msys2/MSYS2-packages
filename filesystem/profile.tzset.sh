# To the extent possible under law, the author(s) have dedicated all 
# copyright and related and neighboring rights to this software to the 
# public domain worldwide. This software is distributed without any warranty. 
# You should have received a copy of the CC0 Public Domain Dedication along 
# with this software. 
# If not, see <https://creativecommons.org/publicdomain/zero/1.0/>. 

# /etc/profile.d/tzset.sh: sourced by /etc/profile.

# The latest version as installed by the MSYS2 Setup program can
# always be found at /etc/defaults/etc/profile.d/tzset.sh

# Modifying /etc/profile.d/tzset.sh directly will prevent
# setup from updating it.

# System-wide tzset.sh file

#Uses the geographical location setting of the user to find the right
#mapping, rather than the locale setting.  Only on Windows 2000 which
#doesn't know about the user's geographical location, or if fetching
#the geographical location fails, it falls back to the user's locale.
test -z "$TZ" && export TZ=$(exec /usr/bin/tzset)
