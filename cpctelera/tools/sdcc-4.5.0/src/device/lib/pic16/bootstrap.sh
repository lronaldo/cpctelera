#!/bin/sh

aclocal
autoheader
autoconf
automake -a -c -f --foreign
