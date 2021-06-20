##-----------------------------LICENSE NOTICE------------------------------------
##  This file is part of CPCtelera: An Amstrad CPC Game Engine 
##  Copyright (C) 2016 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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

## Palette used in this example: Only colors 0-6 (among 16 possible in mode 0)
## are used in the code. [0: Black, 18: Bright Green, 5: Mauve, 6: Bright Red,
## 9: Green, 11: Sky Blue, 12: Yellow].
PALETTE={0 18 5 6 9 11 12}

## Alien image conversion
##    This command will convert img/alien.png into src/sprites/alien.{c|h} files.
##    A C-array called g_alien[6*24] will be generated with the definition
##    of the image alien.png in mode 0 screen pixel format, without interlaced mask.
##    The palette used for conversion is given through the PALETTE variable and
##    a g_palette[16] array will be generated with the 7 palette colours as 
##	  hardware colour values.
$(eval $(call IMG2SPRITES,img/alien.png,0,g,12,24,$(PALETTE),,src/sprites/,hwpalette))

## Background tiles image conversion
##    This command will convert img/tiles.png into src/sprites/tiles.{c|h} files.
##    25 C-arrays called g_tileXX (with XX ranging from 00 to 24) will be generated 
##    with the definition in mode 0 screen pixel format of each one of the 4x4 tiles 
##    contained in img/tiles.png, without interlaced mask. Moreover, "tileset" parameter
##    will force the creation of a 25-pointer C-Array with pointers to each one of the
##    25 tiles. This array will be called g_tileset.
##    The palette used for conversion is given through the PALETTE, thus using the same
##    palette values as for the Alien image (no need to generate hwpalette again)
$(eval $(call IMG2SPRITES,img/tiles.png,0,g,4,4,$(PALETTE),tileset,src/sprites/,))