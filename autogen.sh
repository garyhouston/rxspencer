#!/bin/sh
libtoolize --automake --force
aclocal
autoconf
automake --add-missing
