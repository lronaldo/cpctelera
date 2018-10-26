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
##                      Cassette file manager                             ##
##------------------------------------------------------------------------##
## This file is intended for users to automate the inclusion of files and ##
## loaders in the final production CDT file.                              ##
############################################################################

## Set the name of your main game file in the CDT here
$(eval $(call CDTMAN, SET_FILENAME, Game))

###############################################################################
##                    CASSETE FILE MANAGER HELP INDEX                        ##
##---------------------------------------------------------------------------##
## 	SECTION 1: BASIC CDT GENERATION                                          ##
## 	SECTION 2: AUTOMATIC GENERATION OF A CASSETE LOADER WITH MINILOAD        ##
##  SECTION 3: ADVANCED INClUSION OF FILES INTO CDT                          ##
##                                                                           ##
##  Just search for the title of the section to jump and read it.            ##
##                                                                           ##
###############################################################################
## SECTION 1: BASIC CDT GENERATION                                           ##
##---------------------------------------------------------------------------##
##    For a basic CDT generation, you don't need to edit or add any line to  ##
## this configuration file. By default, your compiled binary will be added   ## 
## as an AMSDOS file to the CDT and will be easy load by typing in RUN".     ##
##                                                                           ##
##    However, you may be interested in setting the name of your file. This  ##
## name will be displayed on the Loading message from the firmware, after    ##
## you type in RUN". To set this name, use the command SET_FILENAME, as in   ##
## the following example.                                                    ##
##                                                                           ##
## EXAMPLE:                                                                  ##
##                                                                           ##
## ## Set the filename of the game to "FIGHTING GAME"                        ##
## $(eval $(call CDTMAN, SET_FILENAME, Fighting Game))                       ##
##                                                                           ##
##    As you guessed, this command sets the name of the filename, in this    ##
## case to "FIGHTING GAME". Filenames can have up to 16 characters including ##
## spaces, but all letters will always be uppercase.                         ##
##                                                                           ##
###############################################################################
##          AUTOMATIC GENERATION OF A CASSETE LOADER WITH MINILOAD           ##
##---------------------------------------------------------------------------##
## A miniload basic loader includes 3 files to be loaded consecutively,      ##
##                                                                           ##
##   - 1) LOADER: will load and execute next 2 parts consecutively.          ##
##   - 2) SCREEN SHOWER: will unpack an image directly to your video         ##
##                       memory, showing your loading screen.                ##
##   - 3) GAME BINARY: this is your game, loading and executing.             ##
##                                                                           ##
## For this to work properly you need to set up next parameters,             ##
##                                                                           ##
##   - SET_MINILOAD_MODE: (1: video mode {0,1,2})                            ##
##        Sets the video mode that will be used to display loading screen.   ##
##        Default: 0.                                                        ##
##                                                                           ##
##   - SET_MINILOAD_PALETTE_FW: (1: firmware colour for border,              ##
##                               2: firmware palette colour list)            ##
##        Sets the border colour and palette that will be used when          ##
##        displaying loading screen. If variable holding firmware palette    ##
##        values was defined in image_conversion.mk, you may use it here     ##
##        (if you want same palette). Both border colour and palette must    ##
##        be decimal integers from 0 to 26. Palette must be 16 values at     ##
##        most, separated by whitespaces.                                    ##
##        Default: 0, 11 15 3 24 13 20 6 26 0 2 1 18 8 5 16 9                ##
##                                                                           ##
##   - GEN_MINILOADER: (1: image file {*png, *jpg, *gif, *bmp...})           ##
##        Generates the 3 files and composes them in the CDT file            ##
##        in the appropriate order and formats. It requires the image        ##
##        file that will be used as loading screen. This image file must     ##
##        be of the exact size for the video mode selected (160x200 for      ##
##        mode 0, 320x200 for mode 1 and 640x400 for mode 2). The image      ##
##        will be converted automatically using selected palette with        ##
##        img2cpc, packed with ZX7B compressor and added at the end of       ##
##        the SCREEN SHOWER binary.                                          ##
##                                                                           ##
## Optionally, you could also configure this,                                ##
##                                                                           ##
##    - SET_MINILOAD_LOADER_ADDRESS: (1: 16-bits memory address)             ##
##        Sets up the memory address where the LOADER program will be        ##
##        loaded. The memory address can be inserted as one decimal or       ##
##        hexadecimal number (with 0x prefix).                               ##
##        By default, LOADER program is loaded at memory address 0xBF2A,     ##
##        taking 146 bytes up to 0xBFBB. These addresses have been tested    ##
##        to be usually safe for 464, 664 and 6128 systems. Firmware uses    ##
##        this place for the program stack. However, you may have many       ##
##        reasons to load your LOADER program at a different address.        ##
##        Default: 0xBF2A                                                    ##
##                                                                           ##
## Some technical details to consider,                                       ##
##                                                                           ##
##    By default, these are the loading addresses and sizes of binaries,     ##
##       - LOADER:        0xBF2A - 0xBFBB (146 bytes)                        ##
##       - SCREEN SHOWER: 0x4000 - 0x409C (157 bytes) [ + packed img ]       ##
##           Configured loading image is packed and added at the end.        ##
##       - GAME:          0x4000 - (depends on your game size)               ##
##                                                                           ##
##    With CPCtelera and Miniloader you can configure your GAME's loading    ##
## address and the LOADER's. SCREEN SHOWER cannot be relocated; it will be   ##
## loaded and executed at 0x4000. However, this is generally not a problem.  ##
## If it was a problem for you, you should consider the advanced section.    ##
## You may then want to add and configure the CDT files manually to suit     ##
## your needs.                                                               ##
##                                                                           ##
## EXAMPLE 1:                                                                ##
##                                                                           ##
## ## GENERATE SIMPLE MINILOADER                                             ##
## ##  Generates a loader that will show a loading screen in mode 0,         ##
## ##  using border colour 0 (BLACK) and default loading screen palette      ##
## ##  (11 15 3 24 13 20 6 26 0 2 1 18 8 5 16 9). Cassette will be filled in ##
## ##  with: 1) the loader, 2) Screen shower with image img/ldscreen.gif     ##
## ##  that will be unpacked to video memory and, 3) CPCtelera project built ## 
## ##  binary that will be finally loaded and executed. Loader will display  ##
## ##  then name "RACING YEAH" when loading after typing in RUN"             ##
## ##                                                                        ##
## $(eval $(call CDTMAN, SET_FILENAME   , Racing Yeah))                      ##
## $(eval $(call CDTMAN, GEN_MINILOADER , img/ldscreen.gif))                 ##
##                                                                           ##
## EXAMPLE 2:                                                                ##
##                                                                           ##
## ## GENERATE STANDARD MINILOADER                                           ##
## ##  Generates a loader that will show a loading screen in mode 1,         ##
## ##  using border colour 3 (RED) and setting up the palette (3 4 5 6)      ##
## ##  (RED, MAGENTA, MAUVE, BRIGHT RED). Cassette will be filled up with:   ##
## ##  1) the loader, 2) Screen shower with image assets/fight.png that will ##
## ##  be unpacked to video memory and, 3) CPCtelera project built binary    ## 
## ##  that will be finally loaded and executed. Loader will display the     ##
## ##  name "JU FIGHTERS" when loading after typing in RUN"                  ##
## ##                                                                        ##
## SCR_PAL=3 4 5 6                                                           ##
## $(eval $(call CDTMAN, SET_FILENAME            , Ju Fighters     ))        ##
## $(eval $(call CDTMAN, SET_MINILOAD_MODE       , 1               ))        ##
## $(eval $(call CDTMAN, SET_MINILOAD_PALETTE_FW , 3, $(SCR_PAL)   ))        ##
## $(eval $(call CDTMAN, GEN_MINILOADER          , assets/fight.png))        ##
##                                                                           ##
##                                                                           ##
## EXAMPLE 3:                                                                ##
##                                                                           ##
## ## GENERATE ADVANCED MINILOADER                                           ##
## ##  Generates a loader that will show a loading screen in mode 2,         ##
## ##  using border colour 26 (BRIGHT WHITE) and setting up the palette      ##
## ##  (9 15) (GREEN, ORANGE). Cassette will be filled up with: 1) loader,   ## 
## ##  2) Screen shower with image data/fate.jpg that will be unpacked to    ## 
## ##  video memory and, 3) CPCtelera project built binary that will be      ## 
## ##  finally loaded and executed. Loader will be loaded at memory address  ##
## ##  0x40 up to address 0xD1 (146 bytes) and will display the name         ##
## ##  "MINIMAL FATE" when loading after typing in RUN". Two more files will ##
## ##  be added to the cassette after CPCtelera build binary:data/level2.bin ##
## ##  and data/level3.bin (in this order). Both files will be add in raw    ##
## ##  miniload format with no header. They will be able to be loaded by the ##
## ##  main program using <cpct_miniload> function.                          ##
## ##                                                                        ##
## $(eval $(call CDTMAN, SET_FILENAME                , Minimal Fate  ))      ##
## $(eval $(call CDTMAN, SET_MINILOAD_LOADER_ADDRESS , 0x0040        ))      ##
## $(eval $(call CDTMAN, SET_MINILOAD_MODE           , 2             ))      ##
## $(eval $(call CDTMAN, SET_MINILOAD_PALETTE_FW     , 26, 9 15      ))      ##
## $(eval $(call CDTMAN, GEN_MINILOADER              , data/fate.jpg ))      ##
## $(eval $(call CDTMAN, ADDFILE, miniload, data/level2.bin ))               ##
## $(eval $(call CDTMAN, ADDFILE, miniload, data/level3.bin ))               ##
##                                                                           ##
###############################################################################
## SECTION 3: ADVANCED INClUSION OF FILES INTO CDT                           ##
##---------------------------------------------------------------------------##
##    Whenever you wanted to include many files in your CDT with different   ##
## purposes, you will have to do it manually using the command ADDFILE. You  ##
## can use as many ADDFILE commands as you want, and all of them will be     ##
## processed sequentially, in order, one by one. That lets you add as many   ##
## files as you want to the CDT and in the order you want.                   ##
##                                                                           ##
##    ADDFILE can add 3 types of files:                                      ##
##      - 'firmware'                                                         ##
##           It will add an AMSDOS header to the file. This enables the      ##
##           firmware to read the file when you type RUN". Resulting file    ##
##           will be saved in 2 blocks, the first 2K in block 1, and the     ##
##           rest in block 2. Saving will use 2000 bauds with standard       ##
##           firmware codification.                                          ##
##      - 'basic'                                                            ##
##           It will add binary-codified BASIC file (not valid for BASIC     ##
##           files saved as ASCII text). The file will be added using the    ##
##           currently set name (set previously using SET_FILENAME). BASIC   ##
##           files are like firmware files but do not require load or run    ##
##           addresses. They will also be added using 2 blocks and 2000      ##
##           bauds(like firmware files). They will be able to be read and    ##
##           executed using standard RUN/LOAD BASIC commands.                ##
##      - 'miniload'                                                         ##
##           This will add a raw file, without any header, only its binary   ##
##           data to the CDT. This file will be codified in miniload format: ##
##           pulses of length 740T for a 0, and 1480T for a 1, being         ##
##           1T = 1/3500000 seconds. These files can then be easily read     ##
##           by using <cpct_miniload> function. You may create files with    ##
##           data and read them wherever you wanted in memory. This enables  ##
##           you to create your own loaders or multiload games, for instance.##
##                                                                           ##
##    IMPORTANT: Any file you give to ADDFILE will be inserted in the CDT as ##
## binary data. Files should not have any header you did not want them to    ##
## have. AMSDOS headers are added when using 'firmware' type. This means     ##
## that you do not have to add them by yourself or your file will end up     ##
## with two headers. Give just binary data to ADDFILE and use the type you   ##
## needed.                                                                   ##
##                                                                           ##
## USAGE,                                                                    ##
##                                <type>                                     ##
## $(eval $(call CDTMAN, ADDFILE, miniload, <file>))                         ##
## $(eval $(call CDTMAN, ADDFILE, basic   , <file>))                         ##
## $(eval $(call CDTMAN, ADDFILE, firmware, <file>, <load_add>, <run_add>))  ##
##                                                                           ##
##    MANDATORY PARAMETERS                                                   ##
##     - <type>    : filetype of the file {'firmware', 'basic', 'miniload'}  ##
##     - <file>    : file you want to add (relative path)                    ##
##                                                                           ##
##    PARAMETERS ONLY FOR 'firmware' FILES                                   ##
##     - <load_add>: CPC memory address where the file will be loaded.       ##
##     - <run_add> : CPC memory address where executable code starts.        ##
##                                                                           ##
## EXAMPLE 1:                                                                ##
##                                                                           ##
## ## CUSTOM LOADER                                                          ##
## ## Adds own loader, screen shower executable and game executable          ##
##                                                                           ##
## ## First of all, set the filename that the loader will use                ##
## $(eval $(call CDTMAN, SET_FILENAME, Superman Game))                       ##
##                                                                           ##
## ## Second, add the loader to the CDT. This needs to be of type 'firmware' ##
## ## to be loaded by the firmware, with RUN". We need to know load address  ##
## ## and run address. On a binary created with CPCtelera, load address is   ##
## ## configured in build_config.mk as Z80CODELOC. Run address is the place  ##
## ## where the main function starts, and that appears on the compilation    ##
## ## messages, at the end. Both values can be consulted in the object file  ##
## ## obj/binaryAddresses.log. For this case, they are 0x0040 and 0x005A.    ##
## $(eval $(call CDTMAN, ADDFILE, firmware, bin/ownloader.bin, 0x40, 0x5A))  ##
##                                                                           ##
## ## Next we add the binary that will show an image in the screen and       ##
## ## return. This will be loaded using cpct_miniload function, so it must   ##
## ## be in 'miniload' raw format.                                           ##
## $(eval $(call CDTMAN, ADDFILE, miniload, bin/loadScreenShower.bin ))      ##
##                                                                           ##
## ## Finally, we add our own game, that will be loaded and executed by our  ##
## ## custom loader ownloader.bin. It is also in 'miniload' raw format to be ## 
## ## loaded using cpct_miniload function. This is the binary produced by    ##
## ## current CPCtelera project, so it will be in the obj/ folder.           ##
## $(eval $(call CDTMAN, ADDFILE, miniload, obj/game.bin ))                  ##
##                                                                           ##
## ## As you see in this example, 'miniload' files do no include load and/or ##
## ## run addresses. However, these files will be loaded at a given memory   ##
## ## address and, if they include executable code, a jump to another memory ##
## ## address will be required. As these files are raw binaries, this data   ##
## ## is not included. Therefore, the program that loads these files must    ##
## ## know this information (the ownloader.bin in this case). Moreover, that ##
## ## program must also know the size of these files, in order to load them  ##
## ## from the CDT.                                                          ##
##                                                                           ##
## EXAMPLE 2:                                                                ##
##                                                                           ##
## ## MULTILOAD GAME                                                         ##
## ## This code adds a game binary that loads next level data from CDT each  ##
## ## time the user finishes one level.                                      ##
##                                                                           ##
## ## First of all, set the filename that the loader will use                ##
## $(eval $(call CDTMAN, SET_FILENAME, Superman Game))                       ##
##                                                                           ##
## ## The game will load directly from BASIC using RUN", so it will be added ##
## ## as a 'firmware' file for this purpose. Afterwards, the game will use   ##
## ## cpct_miniload function to load next levels. Game binary loads at       ##
## ## 0x4000 and takes 32K, up to 0xBFFF. Main function is placed at 0x52AA  ##
## $(eval $(call CDTMAN, ADDFILE, firmware, obj/mygame.bin, 0x4000, 0x52AA)) ##
##                                                                           ##
## ## Level data is placed from 0x0000 to 0x3FFF, with each level taking 16K ##
## ## Levels are added to the CDT consecutively, in miniload format. The     ##
## ## game code will load them when required, using cpct_miniload function.  ##
## $(eval $(call CDTMAN, ADDFILE, miniload, levels/lev1.bin))                ##
## $(eval $(call CDTMAN, ADDFILE, miniload, levels/lev2.bin))                ##
## $(eval $(call CDTMAN, ADDFILE, miniload, levels/lev3.bin))                ##
## $(eval $(call CDTMAN, ADDFILE, miniload, levels/lev4.bin))                ##
## $(eval $(call CDTMAN, ADDFILE, miniload, levels/lev5.bin))                ##
##                                                                           ##
###############################################################################
