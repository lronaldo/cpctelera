#!/bin/bash
##
## This scripts checks in which system is going to be compiled img2cpc
## and setups Makefiles and libraries for appropriate compilation.
##

## Include bash library
source $(dirname $0)/../scripts/lib/bash_library.sh

## Setup depending on the system
if checkSystem cygwin; then
   ## Setup for cygwin
   echo "[img2cpc]: Setting up compilation for Cygwin"
   rm -f lib/libfreeimage.a 
   cp Makefile.cygwin Makefile.this
   if checkSystem cygwin64; then
      echo "[img2cpc]: linking Cygwin64 specific libraries."
      ln -s libfreeimage64.a lib/libfreeimage.a
   else
      echo "[img2cpc]: linking Cygwin32 specific libraries."
      ln -s libfreeimage32.a lib/libfreeimage.a
   fi
else
   echo "[img2cpc]: Setting up compilation for Linux/MacOSX"
   cp Makefile.others Makefile.this
fi
