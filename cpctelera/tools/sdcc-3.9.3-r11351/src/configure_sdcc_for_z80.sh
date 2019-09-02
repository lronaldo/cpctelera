#!/bin/bash
##-----------------------------LICENSE NOTICE------------------------------------
##  This file is part of CPCtelera: An Amstrad CPC Game Engine 
##  Copyright (C) 2015 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
##  Copyright (C) 2013 cpcitor
##
##  This program is free software: you can redistribute it and/or modify
##  it under the terms of the GNU General Public License as published by
##  the Free Software Foundation, either version 3 of the License, or
##  (at your option) any later version.
##
##  This program is distributed in the hope that it will be useful,
##  but WITHOUT ANY WARRANTY; without even the implied warranty of
##  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##  GNU General Public License for more details.
##
##  You should have received a copy of the GNU General Public License
##  along with this program.  If not, see <http://www.gnu.org/licenses/>.
##------------------------------------------------------------------------------

###########################################################################
##                          CPCTELERA ENGINE                             ##
##                     Makefile for building SDCC                        ##
##-----------------------------------------------------------------------##
## This makefile configures SDCC for z80 processor architecture          ##
## In general, there is no need to make changes to this file.            ##
##-----------------------------------------------------------------------##
## This file has been build as a modification of the original            ##
## sdcc_configure.sh found at cpc-dev-tool-chain                         ##
###########################################################################

##
## This script configures SDCC for working with z80 architecture only.
## It must be called from within SDCC src/ directory
##

# Configure console
#set -eu
#set -xv

# Source DIR is the present dir
SRCDIR="${PWD}"

# Set BINDIR propperly
cd "${PWD}/../"
INSTALLDIR="${PWD}"
cd -

# Set OBJDIR propperly and do configure
OBJDIR="${PWD}/../obj/"
mkdir -p "${OBJDIR}"
cd "${OBJDIR}"
OBJDIR="${PWD}"

# Set LDFLAGS accordingly for static build on Cygwin
if [[ "$(uname)" =~ "CYGWIN" ]]; then 
   ADDFLAGS="-static-libstdc++"; 
else
   ADDFLAGS=""
fi

## Setup SDCC for building, with support for Z80 only
"${SRCDIR}/configure" --prefix="${INSTALLDIR}" \
        LDFLAGS="${ADDFLAGS}" \
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
        --disable-stm8-port \
        --disable-ucsim \
	--disable-pdk13-port \
	--disable-pdk14-port \
	--disable-pdk15-port \
	--disable-ez80_z80-port \
	--disable-non-free

## Check if SDCC is configured
if [ $? -ne 0 ]; then
   echo "!!ERROR!!: SDCC was not adequately configured for some reason."
   exit 1
fi
