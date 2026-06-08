#!/usr/bin/env bash

aclocal
autoheader
autoconf
automake -a -c -f --foreign
