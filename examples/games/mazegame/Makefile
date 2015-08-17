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
##                  Main Building Makefile for Projects                  ##
##-----------------------------------------------------------------------##
## This file contains the rules for building a CPCTelera project. These  ##
## These rules work generically for every CPCTelera project.             ##
## Usually, this file should be left unchanged:                          ##
##  * Project's build configuration is to be found in build_config.mk    ##
##  * Global paths and tool configuration is located at $(CPCT_PATH)/cfg/##
###########################################################################

##
## PROJECT CONFIGURATION (you may change things there to setup this project)
##
include cfg/build_config.mk

##
## USE GLOBAL MAKEFILE (general rules for building CPCtelera projects)
##
include $(CPCT_PATH)/cfg/global_main_makefile.mk

# Convert TMX OBJFILES
.PHONY: mazes tiles
mazes: $(TMX_OBJFILES)

%.$(CSV_EXT): %.$(TMX_EXT) 
	cpct_tmx2csv $? > $@
	@$(RM) obj/mazes/mazes.rel
	@$(call PRINT,$(PROJNAME),"Re-generated maze $@")

ASSETSDIR=src/assets/
SPRITESDIR=src/sprites/
TILESFILE=tiles.png
TILESBASENAME=g_tile
PLAYERFILE=character1.png
PLAYERBASENAME=g_player
PALETTE=0 2 6 8 12 16 20 24 26

tiles: 
	cd $(ASSETSDIR) && cpct_img2tileset -bn $(TILESBASENAME) -m 0 -tw 4 -th 4 -pf { $(PALETTE) } $(TILESFILE)
	mv $(ASSETSDIR)$(TILESFILE:.png=.c) $(SPRITESDIR)
	mv $(ASSETSDIR)$(TILESFILE:.png=.h) $(SPRITESDIR)

player: 
	cd $(ASSETSDIR) && cpct_img2tileset -bn $(PLAYERBASENAME) -m 0 -tw 12 -th 12 -pf { $(PALETTE) } $(PLAYERFILE)
	mv $(ASSETSDIR)$(PLAYERFILE:.png=.c) $(SPRITESDIR)$(PLAYERFILE:.png=.c.inc)
	mv $(ASSETSDIR)$(PLAYERFILE:.png=.h) $(SPRITESDIR)