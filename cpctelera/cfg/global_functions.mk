##-----------------------------LICENSE NOTICE------------------------------------
##  This file is part of CPCtelera: An Amstrad CPC Game Engine 
##  Copyright (C) 2015 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
##     CPCTELERA ENGINE: Example of use of arkos tracker player routines ##
##              General Utility functions for the Makefile               ##
##-----------------------------------------------------------------------##
## This file contines general function definitions that are useful to    ##
## simplify the main makefile and make it much more easy to understand.  ##
## Usually, this file should be left unchanged. To configure your build  ##
## you should change the build_config.mk                                 ##
###########################################################################

# ANSI Sequences for terminal colored printing
COLOR_RED=\033[1;31;49m
COLOR_YELLOW=\033[1;33;49m
COLOR_NORMAL=\033[0;39;49m

#################
# PRINT: Print a nice and colorful message
#
# $(1): Subsystem that shows the message
# $(2): Message to print
#
define PRINT
	@printf "$(COLOR_RED)["
	@printf $(1)
	@printf "]$(COLOR_YELLOW) "
	@printf $(2)
	@printf "$(COLOR_NORMAL)\n"
endef

#################
# GETLOADADDRESS: Get load address from a created binary file (parsing hex2bin's log)
#
# $(1): Variable where to store the address 
# $(2): Hex2bin log file to parse 
#
define GETLOADADDRESS
  $(eval $(1):=$(shell sed -n 's/^Binary file start = 0000\([0-9A-Fa-f]*\).*$$/\1/p' < $(2)))
endef

#################
# GETRUNADDRESS: Get run addresses from a created binary file (parsing SDCC generated symbol map in .map file)
#
# $(1): Variable where to store the address
# $(2): File.map that has been compiled (and has .bin.log and .map files associated)
#
define GETRUNADDRESS
  $(eval $(1)   = $(shell sed -n 's/^ *0000\([0-9A-F]*\) *cpc_run_address  *.*$$/\1/p' < $(2);))
  $(eval $(1)_1 = $(shell sed -n 's/^ *0000\([0-9A-F]*\) *init  *.*$$/\1/p'            < $(2);))
  $(eval $(1)_2 = $(shell sed -n 's/^ *0000\([0-9A-F]*\) *_main  *.*$$/\1/p'           < $(2);))
  $(eval $(1)   = $(shell if [ -z "$($(1))" ]; then echo "$($(1)_1)"; fi; ) )
  $(eval $(1)   = $(shell if [ -z "$($(1))" ]; then echo "$($(1)_2)"; fi; ) )
endef

#################
# CHECKVARIABLEISSET: Checks if a given variable is set. If not, it prints out an error message and aborts generation
#
# $(1): Variable to check 
#
define CHECKVARIABLEISSET
  if [ "$($(1))" = "" ]; then \
    echo "**!!ERROR!!**: $(1) is not set. Aborting."; \
    exit 1; \
  fi
endef

#################
# CREATEDSK: Create a DSK file with the BINARY added to it and converted to AMSDOS BINARY
#
# $(1): DSK file to be created
# $(2): Binary file to be included in the DSK file
# $(3): Memory address where binary will be loaded (LOAD ADDRESS)
# $(4): Memory address where main program starts (RUN ADDRESS)
#
define CREATEDSK
  @$(IDSK) $(2).tmp -n -i $(1) -e $(4) -c $(3) -t 1 
  @mv -vf  $(2).tmp $(2);
endef

#################
# CREATEEMPTYDSK: Creates an empty dsk file if the file doesn't exist previously.
#   If the file already existed, nothing is done.
#
# $(1): DSK file name to be created
#
define CREATEEMPTYDSK
	if [ ! -e $(1) ]; then \
  		$(IDSK) $(1) -n; \
  	fi
endef

#################
# ADDCODEFILETODSK: General rule to include a compiled binary file of a program into a 
#    DSK automatically. It takes into account memory load point and entry point for the binary
#
# $(1): DSK file where the binary will be included
# $(2): Binary file to be included 
# $(3): Memory address where binary will be loaded (LOAD ADDRESS)
# $(4): Memory address where main program starts (RUN ADDRESS)
# $(5): Blank object file generated to flag that this inclusion is already done (not to repeat it)
#
define ADDCODEFILETODSK
	@$(IDSK) $(1) -i $(2) -e $(4) -c $(3) -t 1 -f &> /dev/null
	@touch $(5)
	@$(call PRINT,$(1),"Added '$(2:$(DSKFILESDIR)/%=%)'")
endef

#################
# ADDBINFILETODSK: General rule to include a binary file into a DSK automatically
#
# $(1): DSK file where the binary will be included
# $(2): Binary file to be included 
# $(3): Blank object file generated to flag that this inclusion is already done (not to repeat it)
#
define ADDBINFILETODSK
$(3): $(2)
	@$(IDSK) $(1) -i $(2) -t 1 -f &> /dev/null
	@touch $(3)
	@$(call PRINT,$(1),"Added '$(2:$(DSKFILESDIR)/%=%)'")

endef

#################
# CREATECDT: Create a CDT file with the BINARY added to it and converted to AMSDOS BINARY
#
# $(1): Binary file to be inserted in the CDT
# $(2): Name (up to 16 chars) that the file will have inside the CDT (displayed when loading)
# $(3): CDT file to be created
# $(4): Memory address where binary will be loaded (LOAD ADDRESS)
# $(5): Memory address where main program starts (RUN ADDRESS)
#
define CREATECDT
  @$(2CDT) -n -X 0x$(5) -L 0x$(4) -r $(2) $(1) $(3) > /dev/null
endef

#################
# CREATEBLANKCDT: Create a Blank CDT file
#
# $(1): CDT file to be created
#
define CREATEBLANKCDT
	@$(2CDT) -n . $(1) > /dev/null
endef

#################
# ADDBASICFILETOCDT: Adds a BASIC file to a CDT file
#
# $(1): CDT file where the BASIC file will be added
# $(2): BASIC file to be added (path to it in the filesystem)
# $(3): Name (up to 16 characters) to assign to the file inside the CDT (displayed when loading)
#
define ADDBASICFILETOCDT
	@$(2CDT) -F 0 $(2) -r $(3) $(1) > /dev/null
endef

#################
# ADDBINARYFILETOCDT: Adds a BINARY file to a CDT file
#
# $(1): CDT file where the BINARY file will be added
# $(2): Binary file to be inserted in the CDT
# $(3): Name (up to 16 characters) to assign to the file inside the CDT (displayed when loading)
# $(4): Memory address where binary will be loaded (LOAD ADDRESS)
# $(5): Memory address where main program starts (RUN ADDRESS)
#
define ADDBINARYFILETOCDT
	@$(2CDT) -X 0x$(5) -L 0x$(4) -r $(3) $(2) $(1) > /dev/null
endef

#################
# COMPILECFILE: General rule to compile a C file for Z80, having source file and object file in different places
#
# $(1): Object file to be created  (with its relative path)
# $(2): Source file to be compiled (with its relative path)
#
define COMPILECFILE
$(1): $(2)
	$(Z80CC) $(Z80CCINCLUDE) -mz80 $(Z80CCFLAGS) -c $(2) -o $(1)
endef

#################
# COMPILEASMFILE: General rule to compile an ASM file for Z80, having source file and object file in different places
#
# $(1): Object file to be created  (with its relative path)
# $(2): Source file to be compiled (with its relative path)
#
define COMPILEASMFILE
$(1): $(2)
	$(Z80ASM) $(Z80ASMFLAGS) $(Z80CCINCLUDE) $(1) $(2) 
endef

#################
# BINFILE2C: General rule to convert binary files into C Arrays using cpct_bin2c
#
# $(1): C File to be created  (with its relative path)
# $(2): Binary source file to be converted (with its relative path)
#
define BINFILE2C
$(1): $(2)
	$(BIN2C) $(2) -h "cpctelera.h" > $(1)
endef

#################
# IMG2SPRITES: General rule to convert images into C arrays representing
# sprites. Updates IMGCFILES and OBJS2CLEAN adding new C files
# that result from image conversions
#
# $(1): Image file to be converted into C sprite
# $(2): Graphics mode (0,1,2) for the generated values
# $(3): Prefix to add to all C-identifiers generated
# $(4): Width in pixels of each sprite/tile/etc that will be generated
# $(5): Height in pixels of each sprite/tile/etc that will be generated
# $(6): Firmware palette used to convert the image file into C values
# $(7): (mask,tileset,) "mask":    generate interlaced mask for all sprites converted
#                       "tileset": generate a tileset array including pointers to all sprites
# $(8): Output subfolder for generated .C and .H files (inside project folder)
# $(9): (hwpalette) "hwpalette":   output palette array as hardware values
# $(10): Aditional options (you can use this to pass aditional modifiers to cpct_img2tileset)
#
define IMG2SPRITES
$(eval I2S_C  := $(basename $(1)).c)
$(eval I2S_H  := $(basename $(1)).h)
$(eval I2S_NC := $(notdir $(I2S_C)))
$(eval I2S_NH := $(notdir $(I2S_H)))
$(eval I2S_C2 := $(shell if [ ! "$(8)" = "" ]; then A="$(8)"; A="$${A%%/}"; echo "$${A}/$(I2S_NC)"; else echo "$(I2S_C)"; fi))
$(eval I2S_H2 := $(shell if [ ! "$(8)" = "" ]; then A="$(8)"; A="$${A%%/}"; echo "$${A}/$(I2S_NH)"; else echo "$(I2S_H)"; fi))
$(eval I2S_CH := $(I2S_C2) $(I2S_H2))
$(eval I2S_P  := $(shell if [ "$(7)" = "mask" ]; then echo "-nt -im"; elif [ ! "$(7)" = "tileset" ]; then echo "-nt"; fi))
$(eval I2S_P  := $(I2S_P) $(shell if [ "$(9)" = "hwpalette" ]; then echo "-oph"; fi))
.SECONDARY: $(I2S_CH)
$(I2S_CH): $(1)
	@$(call PRINT,$(PROJNAME),"Converting $(1) into C-arrays...")
	cpct_img2tileset $(I2S_P) -m "$(2)" -bn "$(3)" -tw "$(4)" -th "$(5)" -pf $(6) $(10) $(1);
	@$(call PRINT,$(PROJNAME),"Moving generated files:")
	@$(call PRINT,$(PROJNAME)," - '$(I2S_C)' > '$(I2S_C2)'")
	@$(call PRINT,$(PROJNAME)," - '$(I2S_H)' > '$(I2S_H2)'")
	@if [ ! "$(8)" = "" ]; then \
	   mv "$(I2S_C)" "$(I2S_C2)"; \
	   mv "$(I2S_H)" "$(I2S_H2)"; \
	fi
IMGCFILES  := $(I2S_C2) $(IMGCFILES)
OBJS2CLEAN := $(I2S_CH) $(OBJS2CLEAN)
endef


#################
# TMX2C: General rule to convert TMX tilemaps into C arrays.
# Updates IMGCFILES and OBJS2CLEAN adding new C files that result from 
# tmx conversions
#
# $(1): TMX file to be converted to C array
# $(2): C identifier for the generated C array
# $(3): Output folder for C and H files generated (Default same folder)
# $(4): Aditional options (you can use this to pass aditional modifiers to cpct_tmx2csv)
#
define TMX2C
$(eval T2C_C  := $(basename $(1)).c)
$(eval T2C_H  := $(basename $(1)).h)
$(eval T2C_NC := $(notdir $(T2C_C)))
$(eval T2C_NH := $(notdir $(T2C_H)))
$(eval T2C_OF := $(shell if [ ! "$(3)" = "" ]; then echo "-of $(3)"; else echo ""; fi))
$(eval T2C_C2 := $(shell if [ ! "$(3)" = "" ]; then A="$(3)"; A="$${A%%/}"; echo "$${A}/$(T2C_NC)"; else echo "$(T2C_C)"; fi))
$(eval T2C_H2 := $(shell if [ ! "$(3)" = "" ]; then A="$(3)"; A="$${A%%/}"; echo "$${A}/$(T2C_NH)"; else echo "$(T2C_H)"; fi))
$(eval T2C_CH := $(T2C_C2) $(T2C_H2))
.SECONDARY: $(T2C_CH)
$(T2C_CH): $(1)
	@$(call PRINT,$(PROJNAME),"Converting tilemap in $(1) into C-arrays...")
	cpct_tmx2csv -gh -ci $(2) $(T2C_OF) $(4) $(1)
IMGCFILES  := $(T2C_C2) $(IMGCFILES)
OBJS2CLEAN := $(T2C_CH) $(OBJS2CLEAN)
endef
