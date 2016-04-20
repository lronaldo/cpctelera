##-----------------------------LICENSE NOTICE------------------------------------
##  This file is part of CPCtelera: An Amstrad CPC Game Engine 
##  Copyright (C) 2015-2016 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
## This is to automate the generation of a custom CDT file. We have to add the file
## we want to generate (mygame.cdt) to TARGET variable. This has to be done BEFORE
## global_main_makefile.mk is included, as TARGET variable is used there. Bellow you 
## will find detailed code to generate this CDT file and add Amstrad files to it.
##
CUSTOM_CDT_FILE := mygame.cdt
TARGET          := $(TARGET) $(CUSTOM_CDT_FILE)

##
## USE GLOBAL MAKEFILE (general rules for building CPCtelera projects)
##
include $(CPCT_PATH)cfg/global_main_makefile.mk

##-------------------------------------------------------------------------------------------------------------------
## GENERATE A COMPLETE CDT FILE
##   This rule generates a blank CDT file and then populates it with all the files of this project in the 
## appropriate order.
##   0) CDT file has to be added to TARGET variable to be automatically generated. This has to be done BEFORE
##      global_main_makefile.mk is included. So, you'll find 2 lines to do this previously on this file
##   1) Adds game loader    (BASIC File)
##   2) Adds loading screen (SCR file, data to be loaded directly to video memory in MODE 0)
##   3) Adds game binary    (BINARY file with the game)
##-------------------------------------------------------------------------------------------------------------------

## Game Loader File, in BASIC. Files on a CDT can have a completely different 
## name from its original name, up to 16 characters (and no extension is required)
FILE1             := assets/dsk_files/PCLIMBER.BAS
FILE1_NAMEONCDT   := mygreatgame

## Loading Screen. Binary file that has to load directly to screen
## so it will load to 0xC000 by default. RunAddress won't be used
FILE2             := assets/dsk_files/SCREEN.SCR
FILE2_NAMEONCDT   := screen.scr
FILE2_LOADADDRESS := C000
FILE2_RUNADDRESS  := C000

## Game binary file. Loading Address and run address depend on the
## game and may vary every time the game is compiled. These 2 values
## are stored in LOADADDR and RUNADDR variables, generated during 
## compilation. Note that FILE3_LOADADDRESS and FILE3_RUNADDRESS are 
## defined with = instead of := to make them macros and not pre-calculated
## values. LOADADDR and RUNADDR have no value previous to the execution of
## the rule.
FILE3             := obj/game.bin
FILE3_NAMEONCDT   := game
FILE3_LOADADDRESS = $(LOADADDR)
FILE3_RUNADDRESS  = $(RUNADDR)

## This rule generates the Custom CDT file every time FILE1, FILE2 or FILE3 are
## changed. This is done creating a Blank CDT and adding the files sequentially.
## If more files were to be added, a for-each construct could be used for simplicity. 
$(CUSTOM_CDT_FILE): $(FILE1) $(FILE2) $(FILE3)
	$(call     CREATEBLANKCDT,$(CUSTOM_CDT_FILE))
	$(call  ADDBASICFILETOCDT,$(CUSTOM_CDT_FILE),$(FILE1),$(FILE1_NAMEONCDT))
	$(call ADDBINARYFILETOCDT,$(CUSTOM_CDT_FILE),$(FILE2),$(FILE2_NAMEONCDT),$(FILE2_LOADADDRESS),$(FILE2_RUNADDRESS))
	$(call ADDBINARYFILETOCDT,$(CUSTOM_CDT_FILE),$(FILE3),$(FILE3_NAMEONCDT),$(FILE3_LOADADDRESS),$(FILE3_RUNADDRESS))
