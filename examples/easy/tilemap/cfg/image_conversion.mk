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
############################################################################

# DEFINE PALETTE
# 16-colours mode 0 palette used in this example. This values are 
# firmware color values. You may consult this colour values at 
# http://lronaldo.github.io/cpctelera/files/video/cpct_setPalette-asm.html
#  0: Black           1: Blue              2: Bright Blue      3: Red
#  5: Mauve           6: Bright Red        9: Green           11: Sky Blue
# 15: Orange         18: Bright Green     19: Sea Green       20: Bright Cyan
# 21: Lime           22: Pastel Green     24: Bright Yellow   26: Bright White
PALETTE={0 1 2 3 5 6 9 11 15 18 19 20 21 22 24 26}

# CONVERT TILES:
#  Converts img/tiles.png into src/map/tiles.c & src/map/tiles.h
#
# It will split img/tiles.png into 8x8 pixel mode 0 tiles and generate an array 
# of screen pixel format values for each tile. Each array will be named using 
# prefix 'g_' followed by the name of the file 'tiles' and a suffix '_XX' with
# the number of the tile (For instance, g_tiles_00, g_tiles_01...). Conversion 
# will use previously defined $(PALETTE) to generate pixel format values. 
# 'zgtiles' option generates tiles in Zig-Zag pixel order and Gray-Code row order.
# This format is optimal for drawing to screen and required all functions with
# the suffix '_g' in their names, like cpct_etm_drawTilemap4x8_ag (_a: aligned, 
# _g: Zig-Zag pixel order, Gray-Code row order). Finally, a 'g_palette' array 
# containing hardware numbers for all 16 colours will also be generated. 
$(eval $(call IMG2SPRITES,img/tiles.png,0,g,8,8,$(PALETTE),zgtiles,src/map/,hwpalette))


############################################################################
##       GENERAL INSTRUCTIONS TO UNDERSTAND HOW THIS FILE WORKS           ##
##------------------------------------------------------------------------##
## This file is intended for users to automate image conversion from JPG, ##
## PNG, GIF, etc. into C-arrays.                                          ##
##                                                                        ##
## Macro used for conversion is IMG2SPRITES, which has up to 9 parameters:##
##  (1): Image file to be converted into C sprite (PNG, JPG, GIF, etc)    ##
##  (2): Graphics mode (0,1,2) for the generated C sprite                 ##
##  (3): Prefix to add to all C-identifiers generated                     ##
##  (4): Width in pixels of each sprite/tile/etc that will be generated   ##
##  (5): Height in pixels of each sprite/tile/etc that will be generated  ##
##  (6): Firmware palette used to convert the image file into C values    ##
##  (7): (mask / tileset /)                                               ##
##     - "mask":    generate interlaced mask for all sprites converted    ##
##     - "tileset": generate a tileset array with pointers to all sprites ##
##     - "zgtiles": generate tiles/sprites in Zig-Zag pixel order and     ##
##                  Gray Code row order                                   ##
##  (8): Output subfolder for generated .C/.H files (in project folder)   ##
##  (9): (hwpalette)                                                      ##
##     - "hwpalette": output palette array with hardware colour values    ##
## (10): Aditional options (you can use this to pass aditional modifiers  ##
##       to cpct_img2tileset)                                             ##
##                                                                        ##
## Macro is used in this way (one line for each image to be converted):   ##
##  $(eval $(call IMG2SPRITES,(1),(2),(3),(4),(5),(6),(7),(8),(9), (10))) ##
##                                                                        ##
## Important:                                                             ##
##  * Do NOT separate macro parameters with spaces, blanks or other chars.##
##    ANY character you put into a macro parameter will be passed to the  ##
##    macro. Therefore ...,src/sprites,... will represent "src/sprites"   ##
##    folder, whereas ...,  src/sprites,... means "  src/sprites" folder. ##
##                                                                        ##
##  * You can omit parameters but leaving them empty. Therefore, if you   ##
##  wanted to specify an output folder but do not want your sprites to    ##
##  have mask and/or tileset, you may omit parameter (7) leaving it empty ##
##     $(eval $(call IMG2SPRITES,imgs/1.png,0,g,4,8,$(PAL),,src/))        ##
############################################################################
