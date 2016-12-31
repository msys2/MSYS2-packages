MSYS2-packages
==============
[![TeaCI status](https://tea-ci.org/api/badges/Alexpux/MSYS2-packages/status.svg)](https://tea-ci.org/Alexpux/MSYS2-packages)
[![AppVeyor status](https://ci.appveyor.com/api/projects/status/github/Alexpux/MSYS2-packages?branch=master&svg=true)](https://ci.appveyor.com/project/Alexpux/MSYS2-packages)

Package scripts for MSYS2.

To build these, run msys2_shell.cmd then from the bash prompt.

    cd ${package-name}
    makepkg

To install the built package(s).

    pacman -U ${package-name}*.pkg.tar.xz

If you don't have the group base-devel installed, please install.

    pacman -S base-devel
