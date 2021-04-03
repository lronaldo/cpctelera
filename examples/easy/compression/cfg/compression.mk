###-----------------------------LICENSE NOTICE------------------------------------
##  This file is part of CPCtelera: An Amstrad CPC Game Engine
##  Copyright (C) 2021 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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


##-------------------------------------------------------------------------
##                      COMPRESSION EXAMPLE
##-------------------------------------------------------------------------
## This example shows how a binary file is compressed using three
## different compression algorithms.
##-------------------------------------------------------------------------
## For this example, an image is previously converted to binary data using
## configuration file 'cfg/image_conversion.mk', to generate file
## 'img/screenformat/savage_colors.bin' in screen format. With the
## following calls, this binary file is compressed into three separate
## compressed packs, each one using a different compression algorithm (ZX0,
## ZX0 Backwards, and ZX7 Backwards). In this example each compressed pack
## only contains one binary file, but it's also possible to add multiple
## files into one compressed pack, just by using several calls to ADD2PACK
## before calling to the PACKZX0/PACKZX0B/PACKZX7B macro.
##-------------------------------------------------------------------------

## Binary file img/screenformat/savage_colors.bin is added into data_zx7b pack:
$(eval $(call ADD2PACK,data_zx7b,img/screenformat/savage_colors.bin))
## Pack data_zx7b is compressed using ZX7B method:
$(eval $(call PACKZX7B,data_zx7b,src/compressed/))

## Binary file img/screenformat/savage_colors.bin is added into data_zx0 pack:
$(eval $(call ADD2PACK,data_zx0,img/screenformat/savage_colors.bin))
## Pack data_zx0 is compressed using ZX0 method:
$(eval $(call PACKZX0,data_zx0,src/compressed/))

## Binary file img/screenformat/savage_colors.bin is added into data_zx0b pack:
$(eval $(call ADD2PACK,data_zx0b,img/screenformat/savage_colors.bin))
## Pack data_zx0b is compressed using ZX0B method:
$(eval $(call PACKZX0B,data_zx0b,src/compressed/))




############################################################################
##                        CPCTELERA ENGINE                                ##
##                 Automatic compression utilities                        ##
##------------------------------------------------------------------------##
## This file is intended for users to automate the generation of          ##
## compressed files and their inclusion in users' projects.               ##
############################################################################

############################################################################
##              DETAILED INSTRUCTIONS AND PARAMETERS                      ##
##------------------------------------------------------------------------##
##                                                                        ##
## Macros used for compression are ADD2PACK and PACKZX7B:                 ##
##                                                                        ##
##  ADD2PACK: Adds files to packed (compressed) groups. Each call to this ##
##            macro will add a file to a named compressed group.          ##
##  PACKZX7B: Compresses, using ZX7B algorithm, all files in a group into ##
##            a single binary and generates a C-array and a header to     ##
##            comfortably use it from inside your code.                   ##
##  PACKZX0:  Compresses, using ZX0 algorithm, all files in a group into  ##
##            a single binary and generates a C-array and a header to     ##
##            comfortably use it from inside your code.                   ##
##  PACKZX0B: Compresses, using ZX0B algorithm, all files in a group into ##
##            a single binary and generates a C-array and a header to     ##
##            comfortably use it from inside your code.                   ##
##                                                                        ##
##------------------------------------------------------------------------##
##                                                                        ##
##  $(eval $(call ADD2PACK,<packname>,<file>))                            ##
##                                                                        ##
##      Sequentially adds <file> to compressed group <packname>. Each     ##
## call to this macro adds a new <file> after the latest one added.       ##
## packname could be any valid C identifier.                              ##
##                                                                        ##
##  Parameters:                                                           ##
##  (packname): Name of the compressed group where the file will be added ##
##  (file)    : File to be added at the end of the compressed group       ##
##                                                                        ##
##------------------------------------------------------------------------##
##                                                                        ##
##  $(eval $(call PACKZX7B,<packname>,<dest_path>))                       ##
##                                                                        ##
##      Compresses all files in the <packname> group using ZX7B algorithm ##
## and generates 2 files: <packname>.c and <packname>.h that contain a    ##
## C-array with the compressed data and a header file for declarations.   ##
## Generated files are moved to the folder <dest_path>.                   ##
##                                                                        ##
##  Parameters:                                                           ##
##  (packname) : Name of the compressed group to use for packing          ##
##  (dest_path): Destination path for generated output files              ##
##                                                                        ##
##------------------------------------------------------------------------##
##                                                                        ##
##  $(eval $(call PACKZX0,<packname>,<dest_path>))                        ##
##                                                                        ##
##     Compresses all files in the <packname> group using ZX0 algorithm   ##
## and generates 2 files: <packname>.c and <packname>.h that contain a    ##
## C-array with the compressed data and a header file for declarations.   ##
## Generated files are moved to the folder <dest_path>.                   ##
##                                                                        ##
##  Parameters:                                                           ##
##  (packname) : Name of the compressed group to use for packing          ##
##  (dest_path): Destination path for generated output files              ##
##                                                                        ##
##------------------------------------------------------------------------##
##                                                                        ##
##  $(eval $(call PACKZX0B,<packname>,<dest_path>))                       ##
##                                                                        ##
##      Compresses all files in the <packname> group using ZX0B algorithm ##
## and generates 2 files: <packname>.c and <packname>.h that contain a    ##
## C-array with the compressed data and a header file for declarations.   ##
## Generated files are moved to the folder <dest_path>.                   ##
##                                                                        ##
##  Parameters:                                                           ##
##  (packname) : Name of the compressed group to use for packing          ##
##  (dest_path): Destination path for generated output files              ##
##                                                                        ##
############################################################################
##                                                                        ##
## Important:                                                             ##
##  * Do NOT separate macro parameters with spaces, blanks or other chars.##
##    ANY character you put into a macro parameter will be passed to the  ##
##    macro. Therefore ...,src/sprites,... will represent "src/sprites"   ##
##    folder, whereas ...,  src/sprites,... means "  src/sprites" folder. ##
##  * You can omit parameters by leaving them empty.                      ##
##  * Parameters (4) and (5) are optional and generally not required.     ##
############################################################################
