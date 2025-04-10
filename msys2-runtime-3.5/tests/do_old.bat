set MSYS64=%1
set REPOMSYS2=%2
%MSYS64%/usr/bin/bash.exe --login -i -c "pacman -U --noconfirm /var/cache/pacman/pkg/msys2-runtime*.xz"
%MSYS64%/usr/bin/bash.exe --login -i -c "LANG=C cd %REPOMSYS2%/msys2-runtime && sed -i 's,^#define DISABLE_NEW_STUFF .*$,#define DISABLE_NEW_STUFF 1,g' ./src/msys2-runtime/winsup/cygwin/mount.cc && pushd src/build-x86_64-pc-msys && make EXTRA_CPPFLAGS=fbreak-it && make -j1 DESTDIR=%REPOMSYS2%/msys2-runtime/src/dest install && rm -rf %REPOMSYS2%/msys2-runtime/src/dest/etc && popd && makepkg -RLf && pacman -U --noconfirm msys2-runtime*.xz && exit"
%MSYS64%/usr/bin/strace.exe %MSYS64%/usr/bin/ls.exe /bin > meh-bin-old.txt
%MSYS64%/usr/bin/strace.exe %MSYS64%/usr/bin/g++ > meh-g++-old.txt
