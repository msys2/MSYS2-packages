# MSYS2-packages


[![Gitter chat][1]][2]&nbsp;&nbsp;
[![AppVeyor status][3]][4]&nbsp;&nbsp;
[![Azure status][5]][6]&nbsp;&nbsp;

[1]: https://badges.gitter.im/msys2/msys2.png
[2]: https://gitter.im/msys2/msys2
[3]: https://ci.appveyor.com/api/projects/status/github/Alexpux/MSYS2-packages?branch=master&svg=true
[4]: https://ci.appveyor.com/project/Alexpux/MSYS2-packages
[5]: https://dev.azure.com/msys2/mingw/_apis/build/status/msys2.MSYS2-packages?branchName=master&svg=true
[6]: https://dev.azure.com/msys2/mingw/_build/latest?definitionId=5&branchName=master


Package scripts for MSYS2.

To build these, run msys2_shell.cmd then from the bash prompt. Packages from
the msys2-devel and base-devel groups are implicit build time dependencies.
Make sure both are installed before attempting to build any package:

    pacman -S --needed base-devel msys2-devel
    cd ${package-name}
    makepkg

To install the built package(s).

    pacman -U ${package-name}*.pkg.tar.xz

## License

MSYS2-packages is licensed under BSD 3-Clause "New" or "Revised" License.
A full copy of the license is provided in [LICENSE](LICENSE).
