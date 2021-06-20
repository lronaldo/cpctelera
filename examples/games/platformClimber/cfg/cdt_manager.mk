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
############################################################################
##                        CPCTELERA ENGINE                                ##
##                      Cassette file manager                             ##
##------------------------------------------------------------------------##
## This file is intended for users to automate the inclusion of files and ##
## loaders in the final production CDT file.                              ##
############################################################################

## Add BASIC loader PCLIMBER.BAS first
$(eval $(call CDTMAN, SET_FILENAME, PLATFORM CLIMBER))
$(eval $(call CDTMAN, ADDFILE, basic, dsk/PCLIMBER.BAS))
## Then add the screen.scr file as a firmware file to be loaded at video memory location
$(eval $(call CDTMAN, SET_FILENAME, SCREEN.SCR))
$(eval $(call CDTMAN, ADDFILE, firmware, dsk/SCREEN.SCR, 0xC000, 0xC000))
## Finally, add the generated game binary, that the basic loader will look for
$(eval $(call CDTMAN, SET_FILENAME, GAME))
$(eval $(call CDTMAN, ADD_GENERATED_BIN))
