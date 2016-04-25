##-----------------------------LICENSE NOTICE------------------------------------
##  This file is part of CPCtelera: An Amstrad CPC Game Engine 
##  Copyright (C) 2015 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
##
##  This program is free software: you can redistribute it and/or modify
##  it under the terms of the GNU Lesser General Public License as published by
##  the Free Software Foundation, either version 3 of the License, or
##  (at your option) any later version.
##
##  This program is distributed in the hope that it will be useful,
##  but WITHOUT ANY WARRANTY; without even the implied warranty of
##  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##  GNU Lesser General Public License for more details.
##
##  You should have received a copy of the GNU Lesser General Public License
##  along with this program.  If not, see <http://www.gnu.org/licenses/>.
##------------------------------------------------------------------------------

###########################################################################
##                          CPCTELERA ENGINE                             ##
##         Global path configuration for accessing building tools        ##
##-----------------------------------------------------------------------##
## This file is used to configure the paths where the binaries of the    ##
## tools are located. All project and example makefiles include this     ##
## to be able to use the building tools.                                 ##
## If you wanted to use concrete tools for a specific project or example,##
## it is not necessary to modify this file. Just overwrite the values of ##
## this variables in the main makefile of your project/example.          ##
###########################################################################

####
## TOOL PATH CONFIGURATION
##
## Macros in this section take care of the absolute paths where compilation tools
## are located. If your compilation tools (like SDCC, HEX2BIN, etc) are not 
## located in standard paths, you should configure the exact paths of the 
## binary directory of each tool. If you have this compilation tools installed in
## your system, and they are in the global path, you can comment this variables
## and your installed binaries will be used
##
## Important: Take into account that main path to CPCtelera directory works 
##            best if defined as absolute path (Otherwise, it may vary depending
##            on where your project directory is located)
##
####

# CPCtelera library root path (Gets the path where this file is located)
#   Warning: this only works if you are including this file. Copying it
#            to other location will give a different value. It is 
#            preferably to manually set it up with your CPCTelera path
#
THIS_FILE_PATH := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
CPCT_PATH      := $(patsubst %cfg/,%,$(THIS_FILE_PATH))

# PATHs FOR BUILDING TOOLS BINARIES
#  Absolute paths for SDCC, Hex2bin, iDSK and 2CDT 
SDCCBIN_PATH := $(CPCT_PATH)tools/sdcc-3.5.5/bin/
HEX2BIN_PATH := $(CPCT_PATH)tools/hex2bin-2.0/bin/
IDSK_PATH    := $(CPCT_PATH)tools/iDSK-0.13/bin/
2CDT_PATH    := $(CPCT_PATH)tools/2cdt/bin/
SCRIPTS_PATH := $(CPCT_PATH)tools/scripts/

# PATHs FOR CPCTELERA SOURCES AND LIBRARY FILE
CPCT_SRC := $(CPCT_PATH)src
CPCT_LIB := $(CPCT_PATH)cpctelera.lib

# BINARY EXECUTABLE FILES FOR COMPILERs, LINKERs 
Z80CC    := $(SDCCBIN_PATH)sdcc
Z80ASM   := $(SDCCBIN_PATH)sdasz80
Z80LNK   := $(SDCCBIN_PATH)sdar

# BINARY EXECUTABLE FILES FOR TOOLS
HEX2BIN  := $(HEX2BIN_PATH)hex2bin
IDSK     := $(IDSK_PATH)iDSK
2CDT     := $(2CDT_PATH)2cdt
BIN2C    := $(SCRIPTS_PATH)cpct_bin2c

## SHELL TOOLS ALIASES
TOUCH := touch
MKDIR := mkdir -p
RM    := rm -f
TEE   := tee
