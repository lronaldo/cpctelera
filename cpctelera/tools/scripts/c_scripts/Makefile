##-----------------------------LICENSE NOTICE------------------------------------
##  This file is part of CPCtelera: An Amstrad CPC Game Engine 
##  Copyright (C) 2018 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
##                     Makefile for building c_scripts                   ##
##-----------------------------------------------------------------------##
## This makefile calls all the makefiles and build systems of the        ##
## c_scripts required by CPCtelera to work.                              ##
## In general, there is no need to make changes to this file.            ##
###########################################################################
# INCLUDES
include ../../../cfg/global_functions.mk

TOOLS      :=bin2sna tmx2data binpatch
TOOLSYSTEM :=ToolMaker::c_scripts
SCRIPTS_DIR:=..

# TOOLS CONFIG
RM   :=rm -f
MKDIR:=mkdir
MV   :=mv

define MAKERULE
$(1): $(1)/Makefile
	$(call PRINT,$(TOOLSYSTEM),"Building $(1)...")
	@$(MAKE) -C $(1)
	$(call PRINT,$(TOOLSYSTEM),"Moving $(1) binaries to scripts...")
	$(MV) $(1)/bin/* $(SCRIPTS_DIR)

endef

define MAKETOOL
	@$(MAKE) -C $(1) $(2)

endef

# Set optimization only for non-arm devices to prevent Raspberry Pi's to hang due to SD i/o block
define SETCOMPILATIONOPTIMIZATIONS
	$(eval MAKE=$(shell if uname -a | grep arm > /dev/null 2>&1; then \
	 echo make;\
	else\
	 echo make -f;\
	fi))
endef

# TARGETS
.PHONY: all clean cleanall $(TOOLS)

# MAIN TARGET
all: setOptim $(TOOLS)

# Set up compilation optimizations
setOptim:
	$(call SETCOMPILATIONOPTIMIZATIONS)

## RULES TO MAKE ALL THE TOOLS
$(foreach T, $(TOOLS), $(eval $(call MAKERULE, $(T))))

# CLEANING TARGETS
clean cleanall: 
	$(foreach T, $(TOOLS), $(call MAKETOOL, $(T), $@))
