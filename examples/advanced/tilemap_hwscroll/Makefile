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

## PHONY rules for automatically converting files
.PHONY: tilemaps tileset

## Convert Tilemaps TMX files to CSV
##   tilemaps target has to be added to TARGET variable in build_config.mk
##
tilemaps: src/tilemap.csv 

## Convert PNG tilesets to .C and .H
##   tilesets target has to be added to TARGET variable in build_config.mk
##
tilesets: obj/tiles.tls.ok

## Rule for converting TMX to CSV
##
src/tilemap.csv: src/tilemap.tmx
	@$(call PRINT,$(PROJNAME),"Converting tilemap $? to $@...")
	@cpct_tmx2csv $? > $@
	@$(RM) obj/tilemap.rel
	@$(call PRINT,$(PROJNAME),"Tilemap $? converted to $@")

## Rule for generating the tileset
##
PALETTE=0 6 13 10 12 3 9 18 26 15 25 1 14
obj/tiles.tls.ok: src/tiles.png
	@$(call PRINT,$(PROJNAME),"Generating tileset from $?...")
	@cpct_img2tileset -bn g -m 0 -tw 4 -th 4 -pf { $(PALETTE) } $?
	@$(call PRINT,$(PROJNAME),"Generated tileset from $?")
	@touch $@