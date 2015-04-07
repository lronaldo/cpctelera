#!/bin/bash

##
## This script configures SDCC for working with z80 architecture only.
## It must be called from within SDCC src/ directory
##

## Setup SDCC for building, with support for Z80 only
INSTALL_DIR=${PWD}/../bin
SRC_DIR=${PWD}
${SRC_DIR}/configure --prefix="${INSTALLDIR}" \
        --disable-mcs51-port \
        --disable-z180-port \
        --disable-r2k-port \
        --disable-r3ka-port \
        --disable-gbz80-port \
        --disable-ds390-port \
        --disable-ds400-port \
        --disable-pic14-port \
        --disable-pic16-port \
        --disable-hc08-port \
        --disable-s08-port

## Check if SDCC is configured
if [ $? -ne 0 ]; then
   echo "!!ERROR!!: SDCC was not adequately configured for some reason."
   echo "    Check directories: "
   echo "    DIR (${SRC_DIR}) should be home of SDCC source code."
   echo "    DIR (${INSTALL_DIR}) should exist and be writable as install dir for SDCC."
   echo
   exit 1
fi
