<p align="center">
  <a title="msys2.github.io" href="https://msys2.github.io"><img src="https://img.shields.io/website.svg?label=msys2.github.io&longCache=true&style=flat-square&url=http%3A%2F%2Fmsys2.github.io%2Findex.html&logo=github"></a><!--
  -->
  <a title="Join the chat at https://gitter.im/msys2/msys2" href="https://gitter.im/msys2/msys2"><img src="https://img.shields.io/badge/chat-on%20gitter-4db797.svg?longCache=true&style=flat-square&logo=gitter&logoColor=e8ecef"></a><!--
  -->
  <a title="GitHub Actions" href="https://github.com/msys2/MSYS2-packages/actions?query=workflow%3Amain"><img alt="'main' workflow Status" src="https://img.shields.io/github/workflow/status/msys2/MSYS2-packages/main?longCache=true&style=flat-square&label=build&logo=github"></a><!--
  -->
  <a title="AppVeyor" href="https://ci.appveyor.com/project/Alexpux/MSYS2-packages"><img src="https://img.shields.io/appveyor/ci/Alexpux/MSYS2-packages/master.svg?logo=appveyor&logoColor=e8ecef&style=flat-square"></a><!--
  -->
</p>

# MSYS2-packages

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
