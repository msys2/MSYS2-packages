#!/bin/sh

print_gem_default_target() {
  echo 'The default location of gem installs is $HOME/.gem/ruby'
  echo 'Add the following line to your PATH if you plan to install using gem'
  echo '$(ruby -rubygems -e "puts Gem.user_dir")/bin'
  echo 'If you want to install to the system wide location, you must either:'
  echo 'edit /etc/gemrc or run gem with the --no-user-install flag.'
}

# arg 1:  the new package version
post_install() {
  print_gem_default_target
}

# arg 1:  the new package version
# arg 2:  the old package version
post_upgrade() {
  if [ "$(vercmp $2 1.9.3_p125-4)" -lt 0 ]; then
    print_gem_default_target
  fi
}
