#!/bin/bash

##
## This script configures SDCC for working with z80 architecture only.
## It must be called from within SDCC src/ directory
##

## Setup SDCC for building, with support for Z80 only
./configure \
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
        --disable-s08-port \
        --disable-tlcs90-port \
        --disable-stm8-port 


## Check if SDCC is configured
if [ $? -ne 0 ]; then
   echo "!!ERROR!!: SDCC was not adequately configured for some reason."
   exit 1
fi
