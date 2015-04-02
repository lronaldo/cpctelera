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

# Print a nice and colorful message
define PRINT
 echo -ne "\033[1;31;49m"
 echo -ne "[$(PROJNAME)]>"
 echo -ne "\033[1;33;49m $(1)"
 echo -e  "\033[0;39;49m"
endef

# Get Load and Run Addresses from a created binary file
# $(1) File.bin that has been compiled (and has .bin.log and .map files associated)
define GETBINADDRESSES
  $(eval LOADADDR = $(shell sed -n 's/^Binary file start = 0000\([0-9]*\).*$$/\1/p'      < $(1).log;))
  $(eval RUNADDR  = $(shell sed -n 's/^ *0000\([0-9A-F]*\) *cpc_run_address  *.*$$/\1/p' < $(1:.bin=.map);))
  $(eval RUNADDR1 = $(shell sed -n 's/^ *0000\([0-9A-F]*\) *init  *.*$$/\1/p'            < $(1:.bin=.map);))
  $(eval RUNADDR2 = $(shell sed -n 's/^ *0000\([0-9A-F]*\) *_main  *.*$$/\1/p'           < $(1:.bin=.map);))
  $(eval RUNADDR  = $(shell if [ -z "$(RUNADDR)" ]; then echo "$(RUNADDR1)"; fi;))
  $(eval RUNADDR  = $(shell if [ -z "$(RUNADDR)" ]; then echo "$(RUNADDR2)"; fi;))
  ( if [ -z "$(RUNADDR)" ]; then echo "**!!ERROR!!**: Cannot figure out run address for file $(1). Aborting."; exit 1; fi; )
endef

# Create a DSK file with the BINARY added to it and converted to AMSDOS BINARY
# * It requires to have RUNADDR & LOADADDR previously set with the addresses required to include the binary file
# $(1): DSK file to be created
# $(2): Binary file to be included in the DSK file
define CREATEDSK
  @$(IDSK) $2.tmp -n -i $1 -e $(RUNADDR) -c $(LOADADDR) -t 1 
  @mv -vf  $2.tmp $2;
endef

# Create a CDT file with the BINARY added to it and converted to AMSDOS BINARY
# * It requires to have RUNADDR & LOADADDR previously set with the addresses required to include the binary file
# $(1): Binary file to be inserted in the CDT
# $(2): Name (up to 16 chars) that the file will have inside the CDT (displayed when loading)
# $(3): CDT file to be created
define CREATECDT
  @$(2CDT) -n -X 0x$(RUNADDR) -L 0x$(LOADADDR) -r $(2) $(1) $(3) > /dev/null
endef

# General rule to compile a C file for Z80, having source file and object file in different places
# $(1): Object file to be created  (with its relative path)
# $(2): Source file to be compiled (with its relative path)
define COMPILECFILE
$(1): $(2)
	$(Z80CC) $(Z80CCINCLUDE) -mz80 $(Z80CCFLAGS) -c $(2) -o $(1)
endef
