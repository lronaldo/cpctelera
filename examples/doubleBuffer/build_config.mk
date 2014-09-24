##-----------------------------LICENSE NOTICE------------------------------------
##  This file is part of CPCtelera: An Amstrad CPC Game Engine 
##  Copyright (C) 2014 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
##     CPCTELERA ENGINE: Example of use of cpct_setVideoMemoryPage       ##
##                       to create a hardware double buffer              ##
##                     Build configuration file                          ##
##-----------------------------------------------------------------------##
## This file is intendend for you to be able to config the way in which  ##
## you would like to build this example of use of the CPCtelera Engine.  ##
## Below you will find several configuration sections with the macros    ##
## available to configure the build, along with explanation comments to  ##
## help you understand what they do. Please, change everything you want. ##
###########################################################################

####
## SECTION 1: TOOL PATH CONFIGURATION
##
## Macros in this section take care of the absolute paths where compilation tools
## are located. If your compilation tools (like SDCC, HEX2BIN, etc) are not 
## located in standard paths, you should configure the exact paths of the 
## binary directory of each tool.
####

# CPCtelera library root path
CPCT_PATH=../../cpctelera/

# SDCC Compiler binary path (/path/to/sdcc/bin)
#SDCCBIN_PATH=../../../cpc-dev-tool-chain/tool/sdcc/sdcc-3.3.0.installtree/bin/

# HEX2BIN binary path (/path/to/HexToBin/)
#HEX2BIN_PATH=../../../cpc-dev-tool-chain/tool/hex2bin/Hex2bin-1.0.10/

# iDSK binary path (/path/to/iDSK/)
#IDSK_PATH=../../../cpc-dev-tool-chain/tool/idsk/iDSK.0.13/iDSK/src/

####
## SECTION 2: COMPILATION CONFIGURATION
##
## Under this section you find the macros that set up the compiler executables
## that will be used for code compilation. Under Linux systems you would normally
## have your compiler installed in a directory that is added to the path (like
## /usr/bin, for instance). If this is not the case, or you are running under
## Windows, or Cygwin, try to put the executables with full path.
#####

# CPCtelera
CPCT_SRC=$(CPCT_PATH)src
CPCT_LIB=$(CPCT_PATH)cpctelera.lib

# Compilation, linkage and binary generation macros
Z80CODELOC=0x0100
Z80CC=$(SDCCBIN_PATH)sdcc
Z80CCFLAGS= 
Z80CCINCLUDE=-I$(CPCT_SRC)
Z80CCLINKARGS=-mz80 --no-std-crt0 -Wl-u --code-loc $(Z80CODELOC) --data-loc 0 -l$(CPCT_LIB)

# Compiler tools and flags
Z80ASM=$(SDCCBIN_PATH)sdasz80
Z80ASMFLAGS=-l -o -s
Z80LNK=$(SDCCBIN_PATH)/sdar

# Amsdos binary generation tool
HEX2BIN=$(HEX2BIN_PATH)hex2bin

# iDSK interface to generate DSK files
IDSK=$(IDSK_PATH)iDSK

####
## SECTION 3: Project configuration 
##
## This section establishes source and object subfolders and the binary objects to
## be built. Normally, you want to change the OBJ files you want to be built, selecting
## only the ones that contain the actual code that will be used by you in your application.
#####

SRCDIR=src
OBJDIR=obj
CSRCFILES=$(foreach file,$(wildcard $(SRCDIR)/*.c),$(subst $(SRCDIR)/,,$(file)))
OBJFILES=$(CSRCFILES:.c=.rel)
#CSRCFILES=$(wildcard *.c)
#OBJFILES=main.rel
