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
##                 Automatic image conversion file                        ##
##------------------------------------------------------------------------##
## This file is intended for users to automate tilemap conversion from    ##
## original files (like Tiled .tmx) into C-arrays.                        ##
############################################################################

## AUTOMATED TILEMAP CONVERSIONS
##
## We automatically convert 3 tilemaps into arrays: 
##	- img/frame_updown.tmx 		(Upper and Lower parts of the Fixed Frame)
##	- img/frame_leftright.tmx   (Left and Right parts of the Fixed Frame)
##	- img/building.tmx 			(Building tilemap that will be scrolled inside the frame)
##

## CONVERT: img/building.tmx to src/maps/building.c & src/maps/building.h
##	Generates a g_building C-Array, with 1-byte tile indexes defining the building
$(eval $(call TMX2C,img/building.tmx,g_building,src/maps/,))

## CONVERT: img/frame_updown.tmx to src/maps/frame_updown.c & src/maps/frame_updown.h
##	Generates a g_frame_ud C-Array, with 1-byte tile indexes defining Upper and Lower parts of the frame
$(eval $(call TMX2C,img/frame_updown.tmx,g_frame_ud,src/maps/,))

## CONVERT: img/frame_leftright.tmx to src/maps/frame_leftright.c & src/maps/frame_leftright.h
##	Generates a g_frame_lr C-Array, with 1-byte tile indexes defining Left and Right parts of the frame
$(eval $(call TMX2C,img/frame_leftright.tmx,g_frame_lr,src/maps/,))




############################################################################
##              DETAILED INSTRUCTIONS AND PARAMETERS                      ##
##------------------------------------------------------------------------##
##                                                                        ##
## Macro used for conversion is TMX2C, which has up to 4 parameters:      ##
##  (1): TMX file to be converted to C array                              ##
##  (2): C identifier for the generated C array                           ##
##  (3): Output folder for C and H files generated (Default same folder)  ##
##  (4): Bits per item (1,2,4 or 6 to codify tilemap into a bitarray).    ##
##       Blanck for normal integer tilemap array (8 bits per item)        ##
##  (5): Aditional options (aditional modifiers for cpct_tmx2csv)         ##
##                                                                        ##
## Macro is used in this way (one line for each image to be converted):   ##
##  $(eval $(call TMX2C,(1),(2),(3),(4),(5)))                             ##
##                                                                        ##
## Important:                                                             ##
##  * Do NOT separate macro parameters with spaces, blanks or other chars.##
##    ANY character you put into a macro parameter will be passed to the  ##
##    macro. Therefore ...,src/sprites,... will represent "src/sprites"   ##
##    folder, whereas ...,  src/sprites,... means "  src/sprites" folder. ##
##  * You can omit parameters by leaving them empty.                      ##
##  * Parameters (4) and (5) are optional and generally not required.     ##
############################################################################
