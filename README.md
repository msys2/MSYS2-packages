<p align="center">
  <a title="msys2.github.io" href="https://msys2.github.io"><img src="https://img.shields.io/website.svg?label=msys2.github.io&longCache=true&style=flat-square&url=http%3A%2F%2Fmsys2.github.io%2Findex.html&logo=github"></a><!--
  -->
  <a title="Join the chat at https://gitter.im/msys2/msys2" href="https://gitter.im/msys2/msys2"><img src="https://img.shields.io/badge/chat-on%20gitter-4db797.svg?longCache=true&style=flat-square&logo=gitter&logoColor=e8ecef"></a><!--
  -->
  <a title="GitHub Actions" href="https://github.com/msys2/MSYS2-packages/actions?query=workflow%3Amain"><img alt="'main' workflow Status" src="https://img.shields.io/github/actions/workflow/status/msys2/MSYS2-packages/main.yml?branch=master&longCache=true&style=flat-square&label=build&logo=github"></a><!--
  -->
  <a title="Follow msys2org on Twitter" href="https://twitter.com/msys2org"><img src="https://img.shields.io/twitter/follow/msys2org?color=31A4F1&logo=twitter&logoColor=white&style=flat-square"></a><!--
  -->
  <a title="Follow msys2org on Mastodon" href="https://fosstodon.org/@msys2org"><img src="https://img.shields.io/mastodon/follow/109365079526574177?color=000197&domain=https%3A%2F%2Ffosstodon.org%2F&logo=mastodon&logoColor=white&style=flat-square"></a><!--
  -->
  <a title="Join the community on Discord" href="https://discord.com/invite/jPQdRdDcT9"><img src="https://img.shields.io/discord/792780131906617355?color=5865F2&label=Discord&logo=discord&logoColor=white&style=flat-square"></a><!--
  -->
</p>

# MSYS2-packages

Package scripts for MSYS2.

To build these, run msys2_shell.cmd then from the bash prompt. Packages from
the base-devel package is an implicit build time dependency.
Make sure it is installed before attempting to build any package:

    pacman -S --needed base-devel
    cd ${package-name}
    makepkg

To install the built package(s).

    pacman -U ${package-name}*.pkg.tar.xz

## License

MSYS2-packages is licensed under BSD 3-Clause "New" or "Revised" License.
A full copy of the license is provided in [LICENSE](LICENSE).
