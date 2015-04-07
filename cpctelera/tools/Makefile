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
##                          CPCTELERA ENGINE                             ##
##                     Makefile for building tools                       ##
##-----------------------------------------------------------------------##
## This makefile calls all the makefiles and build systems of the various##
## tools required to use CPCTelera and its examples for the creation of  ##
## CPC software and games.                                               ##
## In general, there is no need to make changes to this file.            ##
###########################################################################
# INCLUDES
#include build_config.mk

TOOLS=2cdt hex2bin-2.0 iDSK-0.13 sdcc-3.4.3
TOOLSYSTEM=ToolMaker

# TOOLS CONFIG
RM=rm -f
MKDIR=mkdir

# UTILITY FUNCTIONS
define PRINT
	@echo -ne "\033[1;31;49m"
	@echo -ne "[$(TOOLSYSTEM)]>"
	@echo -ne "\033[1;33;49m $(1)"
	@echo -e  "\033[0;39;49m"
endef

define MAKERULE
$(1): $(1)/Makefile
	$(call PRINT, "Building $(1)...")
	#@echo "Building $(1)..."
	@$(MAKE) -C $(1)
endef

define MAKETOOL
	@$(MAKE) -C $(1) $(2)

endef

# TARGETS
.PHONY: all clean cleanall $(TOOLS)

# MAIN TARGET
all: $(TOOLS)

## RULES TO MAKE ALL THE TOOLS
$(foreach T, $(TOOLS), $(eval $(call MAKERULE, $(T))))

# CLEANING TARGETS
clean cleanall: 
	$(foreach T, $(TOOLS), $(call MAKETOOL, $(T), $@))
