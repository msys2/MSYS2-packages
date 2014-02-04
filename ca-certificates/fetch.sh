#!/bin/sh
exec cvs -d :pserver:anonymous@cvs-mirror.mozilla.org:/cvsroot   \
     co -p mozilla/security/nss/lib/ckfw/builtins/certdata.txt   \
     > certdata.txt
