# Set path to perl scriptdirs if they exist
# https://wiki.archlinux.org/index.php/Perl_Policy#Binaries_and_scripts
# Added /usr/bin/*_perl dirs for scripts

[ -d /usr/bin/site_perl ] && setenv PATH ${PATH}:/usr/bin/site_perl
[ -d /usr/bin/vendor_perl ] && setenv PATH ${PATH}:/usr/bin/vendor_perl
[ -d /usr/bin/core_perl ] && setenv PATH ${PATH}:/usr/bin/core_perl

# If you have modules in non-standard directories you can add them here.
#export PERLLIB=dir1:dir2
