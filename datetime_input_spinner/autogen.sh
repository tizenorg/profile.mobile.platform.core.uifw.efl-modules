#!/bin/sh
# Run this to generate all the initial makefiles, etc.

set  -x
autopoint -f
aclocal
autoconf
libtoolize --copy --force
autoheader
automake --foreign --add-missing --copy

