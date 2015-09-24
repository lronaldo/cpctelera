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
include $(CPCT_PATH)cfg/global_main_makefile.mk

##-------------------------------------------------------------------------------------------------------------------
## Generate code from assets automatically
##   - Convert assets/credits.png into src/sprites/credits.c and src/sprites/credits.h using cpct_img2tileset
##   - Check build_config.mk and look for GENERATED_ASSETS variable. It makes credits.c be a required input for 
##     compilation, triggering this rule
##-------------------------------------------------------------------------------------------------------------------

# FW_PALETE    - Firmware palette used in this game is required to convert the PNG sprite into C-arrays
# CODE_OUT_DIR - Folder where code files will be placed after converting PNG
# ASSETS_DIR   - Folder where assets are placed (PNG files should be there)
FW_PALETTE   = { 11 15 3 24 13 20 6 26 0 2 1 18 8 5 16 9 }
CODE_OUT_DIR = $(SRCDIR)/sprites
ASSETS_DIR   = assets

# Credits.c depends on credits.png. Each tiem credits.png is updated, this rule will be 
# automatically triggered to update corresponding code.
$(CODE_OUT_DIR)/credits.c: $(ASSETS_DIR)/credits.png
	@## Enter assets folder and generate code with cpct_img2tileset 
	@cd $(ASSETS_DIR) && cpct_img2tileset -tw 40 -th 27 -bn G -pf $(FW_PALETTE) -m 0 -nt credits.png
	@## Move generated code files to corresponding code folder
	@mv $(ASSETS_DIR)/credits.c $(CODE_OUT_DIR)
	@mv $(ASSETS_DIR)/credits.h $(CODE_OUT_DIR)
