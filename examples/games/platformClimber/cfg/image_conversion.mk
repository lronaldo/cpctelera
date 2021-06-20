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


## Convert credits.png into credits.c and credits.h
##   This is a mode 0, 40x27 pixels sprite, that is used as authors' credits
## minibanner. The image will be converted into a C-array called G_credits 
## (G is prefix for _credits) without interlaced mask. credits.c and credits.h
## will be output to src/sprites.
#$(eval $(call IMG2SPRITES,assets/credits.png,0,G,40,27,$(PALETTE),,src/sprites/))

## Firmware palette definition in cpct_img2tileset format
PALETTE=11 15 3 24 13 20 6 26 0 2 1 18 8 5 16 9

## Default values
$(eval $(call IMG2SP, SET_FOLDER      , src/sprites/ ))
$(eval $(call IMG2SP, SET_PALETTE_FW  , $(PALETTE)   ))
$(eval $(call IMG2SP, CONVERT         , assets/credits.png, 40, 27, G_credits, g_palette))

