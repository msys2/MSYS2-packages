MSYS2-packages
==============

Package scripts for MSYS2.

To build these, run start_shell.cmd then from the bash prompt.

    cd ${package-name}
    makepkg

To install the built package(s).

    pacman -U ${package-name}*.pkg.tar.xz

If you don't have the group base-devel installed, please install.

    pacman -S base-devel
