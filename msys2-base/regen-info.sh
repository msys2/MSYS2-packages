#!/usr/bin/env bash

INFODIR=/usr/share/info
rm -f $INFODIR/dir

for F in $INFODIR/*.info.gz; do
	echo -n "--> Installing $F ..."
	/usr/bin/install-info --info-file=$F --dir-file=$INFODIR/dir
	echo " done"
done

exit 0