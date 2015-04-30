##-----------------------------LICENSE NOTICE------------------------------------
##  This file is part of CPCtelera: An Amstrad CPC Game Engine 
##  Copyright (C) 2015 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
##     CPCTELERA ENGINE: Example of use of arkos tracker player routines ##
##              General Utility functions for the Makefile               ##
##-----------------------------------------------------------------------##
## This file contines general function definitions that are useful to    ##
## simplify the main makefile and make it much more easy to understand.  ##
## Usually, this file should be left unchanged. To configure your build  ##
## you should change the build_config.mk                                 ##
###########################################################################

# ANSI Sequences for terminal colored printing
COLOR_RED=\033[1;31;49m
COLOR_YELLOW=\033[1;33;49m
COLOR_NORMAL=\033[0;39;49m

#################
# PRINT: Print a nice and colorful message
#
# $(1): Subsystem that shows the message
# $(2): Message to print
#
define PRINT
	@printf "$(COLOR_RED)["
	@printf $(1)
	@printf "]$(COLOR_YELLOW) "
	@printf $(2)
	@printf "$(COLOR_NORMAL)\n"
endef

#################
# GETLOADADDRESS: Get load address from a created binary file (parsing hex2bin's log)
#
# $(1): Variable where to store the address 
# $(2): Hex2bin log file to parse 
#
define GETLOADADDRESS
  $(eval $(1):=$(shell sed -n 's/^Binary file start = 0000\([0-9]*\).*$$/\1/p' < $(2)))
endef

#################
# GETRUNADDRESS: Get run addresses from a created binary file (parsing SDCC generated symbol map in .map file)
#
# $(1): Variable where to store the address
# $(2): File.map that has been compiled (and has .bin.log and .map files associated)
#
define GETRUNADDRESS
  $(eval $(1)   = $(shell sed -n 's/^ *0000\([0-9A-F]*\) *cpc_run_address  *.*$$/\1/p' < $(2);))
  $(eval $(1)_1 = $(shell sed -n 's/^ *0000\([0-9A-F]*\) *init  *.*$$/\1/p'            < $(2);))
  $(eval $(1)_2 = $(shell sed -n 's/^ *0000\([0-9A-F]*\) *_main  *.*$$/\1/p'           < $(2);))
  $(eval $(1)   = $(shell if [ -z "$($(1))" ]; then echo "$($(1)_1)"; fi; ) )
  $(eval $(1)   = $(shell if [ -z "$($(1))" ]; then echo "$($(1)_2)"; fi; ) )
endef

#################
# CHECKVARIABLEISSET: Checks if a given variable is set. If not, it prints out an error message and aborts generation
#
# $(1): Variable to check 
#
define CHECKVARIABLEISSET
  if [ "$($(1))" = "" ]; then \
    echo "**!!ERROR!!**: $(1) is not set. Aborting."; \
    exit 1; \
  fi
endef

#################
# CREATEDSK: Create a DSK file with the BINARY added to it and converted to AMSDOS BINARY
#
# $(1): DSK file to be created
# $(2): Binary file to be included in the DSK file
# $(3): Memory address where binary will be loaded (LOAD ADDRESS)
# $(4): Memory address where main program starts (RUN ADDRESS)
#
define CREATEDSK
  @$(IDSK) $(2).tmp -n -i $(1) -e $(4) -c $(3) -t 1 
  @mv -vf  $(2).tmp $(2);
endef

#################
# CREATECDT: Create a CDT file with the BINARY added to it and converted to AMSDOS BINARY
#
# $(1): Binary file to be inserted in the CDT
# $(2): Name (up to 16 chars) that the file will have inside the CDT (displayed when loading)
# $(3): CDT file to be created
# $(4): Memory address where binary will be loaded (LOAD ADDRESS)
# $(5): Memory address where main program starts (RUN ADDRESS)
#
define CREATECDT
  @$(2CDT) -n -X 0x$(5) -L 0x$(4) -r $(2) $(1) $(3) > /dev/null
endef

#################
# COMPILECFILE: General rule to compile a C file for Z80, having source file and object file in different places
#
# $(1): Object file to be created  (with its relative path)
# $(2): Source file to be compiled (with its relative path)
#
define COMPILECFILE
$(1): $(2)
	$(Z80CC) $(Z80CCINCLUDE) -mz80 $(Z80CCFLAGS) -c $(2) -o $(1)
endef

#################
# COMPILEASMFILE: General rule to compile an ASM file for Z80, having source file and object file in different places
#
# $(1): Object file to be created  (with its relative path)
# $(2): Source file to be compiled (with its relative path)
#
define COMPILEASMFILE
$(1): $(2)
	$(Z80ASM) $(Z80ASMFLAGS) $(Z80CCINCLUDE) $(1) $(2) 
endef
