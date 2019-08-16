##-----------------------------LICENSE NOTICE------------------------------------
##  This file is part of CPCtelera: An Amstrad CPC Game Engine 
##  Copyright (C) 2018 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
##              General Utility functions for the Makefile               ##
##-----------------------------------------------------------------------##
## This file contines general function definitions that are useful to    ##
## simplify the main makefile and make it much more easy to understand.  ##
## Usually, this file should be left unchanged. To configure your build  ##
## you should change the build_config.mk                                 ##
###########################################################################

# Get directory path of this file at the moment of including it
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))

# Get the other makefiles
include $(THIS_DIR)/modules/utils.mk
include $(THIS_DIR)/modules/pack.mk
include $(THIS_DIR)/modules/img2sp.mk
include $(THIS_DIR)/modules/tmx2data.mk
include $(THIS_DIR)/modules/cdtman.mk
include $(THIS_DIR)/modules/aks2data.mk

#################
# GETLOADADDRESS: Get load address from a created binary file (parsing hex2bin's log)
#
# $(1): Variable where to store the address 
# $(2): Hex2bin log file to parse 
#
define GETLOADADDRESS
  $(eval $(1):=$(shell sed -n 's/^Binary file start = 0000\([0-9A-Fa-f]*\).*$$/\1/p' < $(2)))
endef

#################
# GETRUNADDRESS: Get run addresses from a created binary file (parsing SDCC generated symbol map in .map file)
#
# $(1): Variable where to store the address
# $(2): File.map that has been compiled (and has .bin.log and .map files associated)
#
define GETRUNADDRESS
  $(eval $(1)   := $(shell sed -n 's/^ *0000\([0-9A-F]*\) *cpc_run_address  *.*$$/\1/p' < $(2);))
  $(eval $(1)_1 := $(shell sed -n 's/^ *0000\([0-9A-F]*\) *init  *.*$$/\1/p'            < $(2);))
  $(eval $(1)_2 := $(shell sed -n 's/^ *0000\([0-9A-F]*\) *_main  *.*$$/\1/p'           < $(2);))
  $(eval $(1)   := $(shell if [ -z "$($(1))" ]; then echo "$($(1)_1)"; fi; ) )
  $(eval $(1)   := $(shell if [ -z "$($(1))" ]; then echo "$($(1)_2)"; fi; ) )
endef

#################
# GETALLADDRESSES: Gets Load and Run Addresses and checks they are OK
#
# $(1): Binary file generated (from which to get addresses)
#
define GETALLADDRESSES 
	@$(call GETLOADADDRESS,LOADADDR,$1.log)
	@$(call GETRUNADDRESS,RUNADDR,$(1:.bin=.map))
	@$(call CHECKVARIABLEISSET,LOADADDR)
	@$(call CHECKVARIABLEISSET,RUNADDR)
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
# CREATEEMPTYDSK: Creates an empty dsk file if the file doesn't exist previously.
#   If the file already existed, nothing is done.
#
# $(1): DSK file name to be created
#
define CREATEEMPTYDSK
	@if [ -e "$(1)" ]; then \
        rm -f "$(1)"; \
        echo "Removed preexisting $(1) to generate a new one"; \
 	fi 
	@$(IDSK) $(1) -n; 
endef

#################
# ADDCODEFILETODSK: General rule to include a compiled binary file of a program into a 
#    DSK automatically. It takes into account memory load point and entry point for the binary
#
# $(1): DSK file where the binary will be included
# $(2): Binary file to be included 
# $(3): Memory address where binary will be loaded (LOAD ADDRESS)
# $(4): Memory address where main program starts (RUN ADDRESS)
# $(5): Blank object file generated to flag that this inclusion is already done (not to repeat it)
#
define ADDCODEFILETODSK
	@$(IDSK) $(1) -i $(2) -e $(4) -c $(3) -t 1 -f &> /dev/null
	@touch $(5)
	@$(call PRINT,$(1),"Added '$(2:$(DSKFILESDIR)/%=%)'")
endef

#################
# ADDBINFILETODSK: General rule to include a binary file into a DSK automatically
#
# $(1): DSK file where the binary will be included
# $(2): Binary file to be included 
# $(3): Blank object file generated to flag that this inclusion is already done (not to repeat it)
#
define ADDBINFILETODSK
$(3): $(2)
	@$(IDSK) $(1) -i $(2) -t 1 -f &> /dev/null
	@touch $(3)
	@$(call PRINT,$(1),"Added '$(2:$(DSKFILESDIR)/%=%)'")

endef

#################
# CREATESNA: Create a SNA file with the BINARY added to it 
#
# $(1): Binary file to be inserted in the SNA
# $(2): SNA file to be created
# $(3): Memory address where binary will be loaded (LOAD ADDRESS)
# $(4): Memory address where main program starts (RUN ADDRESS)
#
define CREATESNA
  @$(BIN2SNA) -pc 0x$(4) $(1) 0x$(3) > $(2)
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

#################
# BINFILE2C: General rule to convert binary files into C Arrays using cpct_bin2c
#
# $(1): C File to be created  (with its relative path)
# $(2): Binary source file to be converted (with its relative path)
#
define BINFILE2C
$(1): $(2)
	$(BIN2C) $(2) -i "cpctelera.h" > $(1)
endef
