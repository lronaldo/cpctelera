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
##                     Makefile for building SDCC                        ##
##-----------------------------------------------------------------------##
## This makefile configures & builds SDCC for z80 processor architecture ##
## In general, there is no need to make changes to this file.            ##
###########################################################################

# LOCATIONS
INSTALLDIRS=bin share
SRCDIR=src
OBJDIR=obj
CONFIG=configure_sdcc_for_z80.sh
TARGET=sdcc

# TARGETS
.PHONY: all clean cleanall $(TARGET)

# MAIN TARGET
all: $(TARGET)

# SDCC
sdcc:
	@echo "Configuring SDCC for z80..."
	cd $(SRCDIR) && ./$(CONFIG)
	@echo "Compiling SDCC for z80..."
	$(MAKE) -C $(OBJDIR)
	$(MAKE) -C $(OBJDIR) install

# CLEANING TARGETS
cleanall: clean
	@echo "Removing install directories ($(INSTALLDIRS))..."
	@rm -rf $(INSTALLDIRS)
clean:
	@echo "Removing all object files..."
	-@$(MAKE) -C $(OBJDIR) distclean
	@rm -rf $(OBJDIR)
