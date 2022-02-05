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

## Default values
#$(eval $(call IMG2SP, SET_MODE        , 0                  ))  { 0, 1, 2 }
#$(eval $(call IMG2SP, SET_MASK        , none               ))  { interlaced, none }
#$(eval $(call IMG2SP, SET_EXTRAPAR    ,                    ))
#$(eval $(call IMG2SP, SET_IMG_FORMAT  , sprites            ))	{ sprites, zgtiles, screen }
#$(eval $(call IMG2SP, SET_OUTPUT      , c                  ))  { bin, c }
#$(eval $(call IMG2SP, CONVERT         , img.png , w, h, array, palette, tileset))

## Define a Macro with our desired 16 colours palette
PALETTE=13 1 2 3 6 4 5 8 17 9 18 15 24 0 11 26

# Set up conversion palette and output folder
$(eval $(call IMG2SP, SET_PALETTE_FW  , $(PALETTE) ))
$(eval $(call IMG2SP, SET_FOLDER      , src/sprites ))

# Convert the palette macro values into a C-array of hardware values
$(eval $(call IMG2SP, CONVERT_PALETTE , $(PALETTE), g_palette ))

# Converting images. (w,h) = (0,0) means use the whole width and height of the PNG image
$(eval $(call IMG2SP, CONVERT         , img/balloon.png     , 0, 0, g_balloon))
$(eval $(call IMG2SP, CONVERT         , img/square.png      , 0, 0, g_square))
$(eval $(call IMG2SP, CONVERT         , img/star_trans.png  , 0, 0, g_star_trans))
$(eval $(call IMG2SP, CONVERT         , img/circle_trans.png, 0, 0, g_circle_trans))
$(eval $(call IMG2SP, CONVERT         , img/roof.png        , 0, 0, g_roof))
$(eval $(call IMG2SP, CONVERT         , img/cloud.png       , 0, 0, g_cloud))
