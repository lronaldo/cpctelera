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
## This file is intended for users to automate image conversion from JPG, ##
## PNG, GIF, etc. into C-arrays.                                          ##
############################################################################

## DEFINITION OF THE FIRMWARE PALETTE TO BE USED IN THIS EXAMPLE
##
## Using firmware palette values, we define the 4 colours that will
## be used in this example, in ascending order, as
## |=========|==========|================|
## | Palette | Firmware |    Colour      |
## |  Index  |  Colour  |     Name       |
## |=========|==========|================|
## |    0    |     0    |  Black         |
## |    1    |    24    |  Bright Yellow |
## |    2    |    26    |  Bright White  |
## |    3    |    13    |  White         |
## |=========|==========|================|
##
PALETTE={ 0 24 26 13 }

## AUTOMATED IMAGE CONVERSION 
##
##  This macro will proceed to convert img/ctlogo.png image into two files:
## src/ctlogo.c and str/ctlogo.h. This conversion will be performed before
## starting to compile project files. This conversion will use previously
## defined PALETTE as Amstrad colours to be used. The original PNG will then
## be transformed into an sprite array definition using only PALETTE colours.
## Resulting sprite will be produced in slices of 48x62 pixels. As this is the
## full size of ctlogo.png, only one slice will be produced (a single sprite
## array). Produced sprite will be codified into MODE 1 colours, and the 
## array will be named as ctlogo with G_ prefix, resulting in G_ctlogo name.
## Also, a G_palette array will be produced with the converted values of 
## PALETTE variable into Hardware values, which are the ones that the 
## Amstrad CPC understands and CPCtelera uses.
##
$(eval $(call IMG2SPRITES,img/ctlogo.png,1,G,48,62,$(PALETTE),,src/,hwpalette))




############################################################################
##              DETAILED INSTRUCTIONS AND PARAMETERS                      ##
##------------------------------------------------------------------------##
##                                                                        ##
## Macro used for conversion is IMG2SPRITES, which has up to 9 parameters:##
##  (1): Image file to be converted into C sprite (PNG, JPG, GIF, etc)    ##
##  (2): Graphics mode (0,1,2) for the generated C sprite                 ##
##  (3): Prefix to add to all C-identifiers generated                     ##
##  (4): Width in pixels of each sprite/tile/etc that will be generated   ##
##  (5): Height in pixels of each sprite/tile/etc that will be generated  ##
##  (6): Firmware palette used to convert the image file into C values    ##
##  (7): (mask / tileset / zgtiles)                                       ##
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
