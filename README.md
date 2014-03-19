MSYS2-packages
==============

Package scripts for MSYS2.

To build these, run msys2_shell.bat then from the bash prompt:
cd <package-name>
makepkg

To install the build package(s):
pacman -U <package-name>*.pkg.tar.xz
