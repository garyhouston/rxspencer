#!/bin/sh
libtoolize --automake --force
aclocal
autoconf -Wall --force
automake -Wall --add-missing --force-missing
