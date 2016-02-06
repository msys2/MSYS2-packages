#!/bin/bash

pushd `dirname $0` > /dev/null
SCRIPTPATH=`pwd`
popd > /dev/null

cd $SCRIPTPATH

set -x

# cd to git repository root directory
gittop=`git rev-parse --show-toplevel`
cd $gittop

# fetch first changed file
changed=`git show --pretty="format:" --name-only | grep . | head -1`
dir=`dirname $changed`
if [ "$dir" = "." ]
then
    # self test
    cd appveyor
else
    cd $dir
fi

pwd
ls
makepkg -f -s --noconfirm --skippgpcheck --noprogressbar
