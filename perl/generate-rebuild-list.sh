#!/bin/bash

pacman -Fxq '/usr/lib/perl5/vendor_perl/auto/.*\.dll' | sed 's#^.*/##' | sort -u
