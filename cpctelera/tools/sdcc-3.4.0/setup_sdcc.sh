#!/bin/bash

##
## This script configures SDCC to be built. 
## It must be run from inside the SDCC source code directory.
##
INSTALL_DIR=${PWD}/bin

${PWD}/configure --prefix="${INSTALLDIR}" \
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
