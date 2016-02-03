#!/bin/bash

set -e

git config --global user.email "ci@msys2.org"
git config --global user.name "MSYS2 Build Bot"

# fetch first changed file, assume at most one package touched per commit
TOUCHED=`git show --pretty="format:" --name-only | grep . | head -1`
PKGDIR=`dirname $TOUCHED`
if [ "$PKGDIR" = "." ]
then
    echo Nothing to test
else
    pushd $PKGDIR > /dev/null
    makepkg -f -s --noconfirm --skippgpcheck --noprogressbar
    popd > /dev/null
fi
