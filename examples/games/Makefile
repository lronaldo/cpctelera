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
##                      Examples Building Makefile                       ##
##-----------------------------------------------------------------------##
## This file is intended to build all the examples included in CPCtelera ##
## In general, there is no need to make changes to this file.            ##
###########################################################################
# Calls Makefiles inside all subdirectories of the present directory, with the specified target
# $(1): target to be called
define BUILDSUBDIRS
	@for D in *; do \
	  if [ -d "$$D" ]; then \
	    $(MAKE) -C $$D $(1); \
	  fi \
	done
endef

# TARGETs
.PHONY: all clean cleanall
all clean cleanall:
	$(call BUILDSUBDIRS,$@)
