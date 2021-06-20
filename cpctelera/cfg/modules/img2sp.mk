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

###########################################################################
##           		 IMG2SP General functionalities                      ##
##-----------------------------------------------------------------------##
## This file contains all macros related to IMG2SP general functionality ##
## used to automate the conversion between images and sprites.           ##
## IMG2SP is the general macro that controls all the other macros in     ##
## this file.                                                            ##
## IMG2SPRITES old macro is also maintained for backwards compatibility  ##
###########################################################################

#
# Default values for all IMG2SP functions
#
I2S_PAL  := 
I2S_MODE := -m 0
I2S_MASK := 
I2S_FMT  := 
I2S_OUT  := -of c
I2S_FOLD := src/
I2S_EXTP := 
I2S_ERR  := <<ERROR>> [IMG2SP - 

# Ensure that image_conversion.mk exists for compatibility with older CPCtelera projects
I2S_DEPEND := cfg/image_conversion.mk
TOUCHIFNOTEXIST := $(TOUCHIFNOTEXIST) $(I2S_DEPEND)


#################
# IMG2SP_SET_PALETTE_FW: Sets the firmware palette to be used in following IMG2SP CONVERT commands
#
# $(1): Firmware palette array (Array of integers from 0 to 26, max size 16)
#
# Updates variable I2S_PAL
#
define IMG2SP_SET_PALETTE_FW
	# Verify passed firmware palette
	$(call VERIFY_FW_PALETTE,$(1),IMG2SP - SET_PALETTE_FW)

	# All checks passed, add brackets to palette
	$(eval I2S_PAL := -pf { $(1) })
endef

#################
# IMG2SP_SET_MODE: Sets the Amstrad CPC pixel mode to be used in following IMG2SP CONVERT commands
#
# $(1): Mode {0 1 2}
#
# Updates variable I2S_MODE
#
define IMG2SP_SET_MODE
	$(eval I2S_VALIDMODES := 0 1 2)
	$(eval I2S_MODE := $(filter $(I2S_VALIDMODES),$(1)))
	$(if $(filter-out "$(I2S_MODE)",""),,$(error $(I2S_ERR) SET_MODE]: '$(1)' is not valid. Valid items are: {$(I2S_VALIDMODES)}))
	$(eval I2S_MODE := -m $(I2S_MODE))
endef

#################
# IMG2SP_SET_MASK: Sets how masks are to be treated on following IMG2SP CONVERT commands
#
# $(1): Mask mode {interlaced none}
#
# Updates variable I2S_MASK
#
define IMG2SP_SET_MASK
	# Configure mask option
	$(eval I2S_MASK_VALS :=  interlaced none)
	$(eval I2S_MASK_OPS  :=  -im)
	$(eval $(call CONVERTVALUE,$(1),I2S_MASK_VALS,I2S_MASK_OPS,default,I2S_MASK))
	$(if $(filter-out $(I2S_MASK),default),,$(error $(I2S_ERR) SET_MASK]: Value '$(1)' is not valid. Expected values are: {$(I2S_MASK_VALS)}))
endef

#################
# IMG2SP_SET_OUTPUT: Sets the output format for following IMG2SP CONVERT commands
#
# $(1): Output format {c bin}
#
# Updates variable I2S_OUT
#
define IMG2SP_SET_OUTPUT
	# Configure output option
	$(eval _VALID := c bin)	
	$(call ENSURE_SINGLE_VALUE,$(1),$(I2S_ERR) SET_OUTPUT]: '$(1)' is not valid. It must be a SINGLE value from: {$(_VALID)})
	$(call ENSUREVALID,$(1),$(_VALID),is not valid [IMG2SP - SET_OUTPUT])
	$(eval I2S_OUT := -of $(1))
endef

#################
# IMG2SP_SET_FOLDER: Sets the output folder to be used for following IMG2SP CONVERT commands
#
# $(1): Output folder
#
# Updates variable I2S_FOLD
#
define IMG2SP_SET_FOLDER
	$(call ENSUREFILEEXISTS,$(1),$(I2S_ERR) SET_FOLDER]: Folder '$(1)' does not exist or is not accessible)
	$(eval I2S_FOLD := $(1))
endef

#################
# IMG2SP_SET_IMG_FORMAT: Sets the pixel format to be used for following IMG2SP CONVERT commands
#
# $(1): Pixel Format { sprites, zgtiles, screen }
#
# Updates variable I2S_FOLD
#
define IMG2SP_SET_IMG_FORMAT
	# Convert selected format into options
	$(eval _VALID := zgtiles screen sprites)
	$(eval _CNV := -z'-g -scr '')
	$(call CONVERTVALUE,$(1),_VALID,_CNV,none,_F)
	
	# Check that selected format is valid
	$(if $(call EQUALS,$(_F),none),$(error $(I2S_ERR) SET_IMG_FORMAT]: '$(1)' is not valid. Valid values are: {$(_VALID)}),)

	# Perform assignment (Translating ' into spaces)
	$(eval I2S_FMT := $(subst ',$(SPACE),$(_F)))
endef

#################
# IMG2SP_SET_EXTRAPAR: Sets extra parameters to be used for following IMG2SP CONVERT commands
#
# $(1): Extra parameters
#
# Updates variable I2S_EXTP
#
define IMG2SP_SET_EXTRAPAR
	$(eval I2S_EXTP := $(1))
endef

#################
# IMG2SP_CONVERT_TARGET: Generates the common target line for all CONVERT macros
# Waring: $(1) is a variable name, and not its contents
#
# $(1): Variable that contains the targets 
# $(2): Image file on which targets depend
# $(3): Tile Width
# $(4): Tile Height
# $(5): C-identifier for the array containing pixel data
# $(6): Message string to output
#
# Updates IMGCFILES and OBJS2CLEAN adding new C files that result from the pack generation.
#
define IMG2SP_CONVERT_TARGET
	# Target files are the contents of $(1)
	$(eval I2S_TGT := $($(1)))

	# Check that image file exists
	$(call ENSUREFILEEXISTS,$(2),$(I2S_ERR) CONVERT]: '$(2)' file does not exist or is not readable)	

# Generate target for image conversion
.SECONDARY: $(I2S_TGT)
$(I2S_TGT): $(2) $(I2S_DEPEND)
	@$(call PRINT,$(PROJNAME),$(6))
	$(IMG2TIL) $(I2S_FMT) $(I2S_MASK) $(I2S_OUT) $(I2S_MODE) $(I2S_PAL) $(I2S_GPAL) $(I2S_TILS) -bn "$(5)" -tw "$(3)" -th "$(4)" $(I2S_EXTP) $(2);
endef

#################
# IMG2SP_CONVERT_PALETTE: Converts a firmware palette into .c/.h files with hardware values
# It only converts the palette, without doing anything else
#
# $(1): Palette Values
# $(2): Output C-identifier (it will also be used as filename)
#
# Updates IMGCFILES and OBJS2CLEAN adding new C files that result from the pack generation.
#
define IMG2SP_CONVERT_PALETTE
	# Verify Firmware palette before converting
	$(call VERIFY_FW_PALETTE,$(1),CONVERT_PALETTE)
	# Empty contents before filling it up with new HW values
	$(eval I2SCP_PHW:=)
	# ISCP_PHW fills up with hardware palette values converted from firmware values
	$(call CONVERT_FW2HW_PALETTE,$(1),I2SCP_PHW)

	# Get the C-identifier and palette size
	$(eval I2SCP_CID := $(2))
	$(eval I2SCP_PSZ := $(words $(1)))

	# Check that it is a valid C-identifier
	$(call ENSURE_VALID_C_ID,$(I2SCP_CID),$(I2S_ERR) CONVERT_PALETTE]: '$(I2SCP_CID)' is not a valid C-identifier. \
		As it will be used both for filename and array identifier, it needs to be a valid C-identifier)

	# Generate filenames
	$(call JOINFOLDER2BASENAME, I2SCP_CFILE, $(I2S_FOLD), $(I2SCP_CID).c)
	$(eval I2SCP_HFILE := $(basename $(I2SCP_CFILE)).h)
	$(eval I2SCP_FILES := $(I2SCP_CFILE) $(I2SCP_HFILE))
	$(eval COMMA=,)

# Generate target for palette generation
.SECONDARY: $(I2SCP_FILES)
$(I2SCP_FILES): $(I2S_DEPEND)
	@$(call PRINT,$(PROJNAME),"Generating palette hardware values for '$(I2SCP_CID)'...")
	@echo "// File generated by CONVERT_PALETTE macro from CPCtelera" > $(I2SCP_HFILE)
	@echo "// Converted firmware palette declaration" >> $(I2SCP_HFILE)
	@echo "// " >> $(I2SCP_HFILE)
	@echo "#ifndef _$(basename $(I2SCP_CID))_H_" >> $(I2SCP_HFILE)
	@echo "#define _$(basename $(I2SCP_CID))_H_" >> $(I2SCP_HFILE)
	@echo "" >> $(I2SCP_HFILE)
	@echo "#include <types.h>" >> $(I2SCP_HFILE)
	@echo "" >> $(I2SCP_HFILE)
	@echo "extern const u8 $(I2SCP_CID)[$(I2SCP_PSZ)];" >> $(I2SCP_HFILE)
	@echo "" >> $(I2SCP_HFILE)
	@echo "#endif" >> $(I2SCP_HFILE)
	@echo "// File generated by CONVERT_PALETTE macro from CPCtelera" > $(I2SCP_CFILE)
	@echo "// Firmware Palette Converted: $(1)" >> $(I2SCP_CFILE)
	@echo "// " >> $(I2SCP_CFILE)
	@echo "#include \"$(notdir $(I2SCP_HFILE))\"" >> $(I2SCP_CFILE)
	@echo " " >> $(I2SCP_CFILE)
	@echo "const u8 $(I2SCP_CID)[$(I2SCP_PSZ)] = {" >> $(I2SCP_CFILE)
	@echo "   $(patsubst %$(COMMA)vvv,%,$(patsubst %,0x%$(COMMA),$(I2SCP_PHW))vvv)" >> $(I2SCP_CFILE)
	@echo "};" >> $(I2SCP_CFILE)


# Update OBJS2CLEAN and IMGCFILES
IMGCFILES  := $(I2SCP_CFILE) $(IMGCFILES)
OBJS2CLEAN := $(I2SCP_FILES) $(OBJS2CLEAN)
endef

#################
# IMG2SP_CONVERT_BIN: Converts an image file into data ready to be used in the program.
# 				      This version of the macro converts to binary.
#
# $(1): Image file
# $(2): Horizontal resolution of each converted tile in pixels
# $(3): Vertical resolution of each converted tile in pixels
# $(4): C-identifier for the array containing pixel data
# $(5): C-identifier for the array containing generated palette (if required)
# $(6): C-identifier for the array containing generated tileset (ignored: no tileset produced on binary output)
#
# Updates IMGCFILES, IMGASMFILES and OBJS2CLEAN adding new C files that result from the pack generation.
#
define IMG2SP_CONVERT_BIN
	# Set up C and H files and paths
	$(eval I2S_S := $(basename $(1)).h.s)
	$(eval I2S_H := $(basename $(1)).h)
	$(eval I2S_B := $(basename $(1)).bin)
	$(eval $(call JOINFOLDER2BASENAME, I2S_S2, $(I2S_FOLD), $(I2S_S)))
	$(eval $(call JOINFOLDER2BASENAME, I2S_H2, $(I2S_FOLD), $(I2S_H)))
	$(eval $(call JOINFOLDER2BASENAME, I2S_B2, $(I2S_FOLD), $(I2S_B)))
	$(eval I2S_CSB := $(I2S_S2) $(I2S_H2) $(I2S_B2))
	
	# Check if we have to generate a palette or not
	$(eval I2S_GPAL := $(if $(5),-opn "$(5)",))

	# No tileset to be generated on binary output
	$(eval I2S_TILS := -nt)

	# Generate Target for image conversion
	$(call IMG2SP_CONVERT_TARGET,I2S_CSB,$(1),$(2),$(3),$(4),"Converting $(1) into binary data and .h + .h.s files for declarations...")
	@$(call PRINT,$(PROJNAME),"Moving generated files:")
	@$(call PRINT,$(PROJNAME)," - '$(I2S_S)' > '$(I2S_S2)'")
	@$(call PRINT,$(PROJNAME)," - '$(I2S_H)' > '$(I2S_H2)'")
	@$(call PRINT,$(PROJNAME)," - '$(I2S_B)' > '$(I2S_B2)'")
	@if [ "$(I2S_FOLD)" != "" ]; then \
	   mv "$(I2S_S)" "$(I2S_S2)"; \
	   mv "$(I2S_H)" "$(I2S_H2)"; \
	   mv "$(I2S_B)" "$(I2S_B2)"; \
	fi

# Variables that need to be updated to ensure they are erased on clean
IMGBINFILES := $(I2S_B2) $(IMGBINFILES)
IMGASMFILES := $(I2S_S2) $(IMGASMFILES)
OBJS2CLEAN  := $(I2S_CSB) $(OBJS2CLEAN)
endef

#################
# IMG2SP_CONVERT_C: Converts an image file into data ready to be used in the program
#
# $(1): Image file
# $(2): Horizontal resolution of each converted tile in pixels
# $(3): Vertical resolution of each converted tile in pixels
# $(4): C-identifier for the array containing pixel data
# $(5): C-identifier for the array containing generated palette (if required)
# $(6): C-identifier for the array containing generated tileset (if required)
#
# Updates IMGCFILES and OBJS2CLEAN adding new C files that result from the pack generation.
#
define IMG2SP_CONVERT_C
	# Set up C and H files and paths
	$(eval I2S_C := $(basename $(1)).c)
	$(eval I2S_H := $(basename $(1)).h)
	$(eval $(call JOINFOLDER2BASENAME, I2S_C2, $(I2S_FOLD), $(I2S_C)))
	$(eval $(call JOINFOLDER2BASENAME, I2S_H2, $(I2S_FOLD), $(I2S_H)))
	$(eval I2S_CH := $(I2S_C2) $(I2S_H2))
	
	# Check if we have to generate a palette or not
	$(eval I2S_GPAL := $(if $(5),-opn "$(5)",))

	# Check if we have to generate a tileset or not
	$(eval I2S_TILS := $(if $(6),-ts "$(6)",-nt))

	# Generate target for image conversion
	$(call IMG2SP_CONVERT_TARGET,I2S_CH,$(1),$(2),$(3),$(4),"Converting $(1) into C-arrays...")
	@$(call PRINT,$(PROJNAME),"Moving generated files:")
	@$(call PRINT,$(PROJNAME)," - '$(I2S_C)' > '$(I2S_C2)'")
	@$(call PRINT,$(PROJNAME)," - '$(I2S_H)' > '$(I2S_H2)'")
	@if [ "$(I2S_FOLD)" != "" ]; then \
	   mv "$(I2S_C)" "$(I2S_C2)"; \
	   mv "$(I2S_H)" "$(I2S_H2)"; \
	fi

# Variables that need to be updated to keep up with generated files and erase them on clean
IMGCFILES  := $(I2S_C2) $(IMGCFILES)
OBJS2CLEAN := $(I2S_CH) $(OBJS2CLEAN)
endef


#################
# IMG2SP_CONVERT: Launches the proper version of CONVERT to perform image conversion into data
#
# $(1-6): Parameters
#
define IMG2SP_CONVERT
	$(if $(filter-out $(I2S_OUT),-of c),,$(eval $(call IMG2SP_CONVERT_C,$(1),$(2),$(3),$(4),$(5),$(6))))
	$(if $(filter-out $(I2S_OUT),-of bin),,$(eval $(call IMG2SP_CONVERT_BIN,$(1),$(2),$(3),$(4),$(5),$(6))))	
endef

#################
# IMG2SP: Front-end to access all functionalities of IMG2SP macros about image conversion
# to sprites.
#
# $(1): Command to be performed
# $(2-8): Valid arguments to be passed to the selected command
#
# Valid Commands: SET_PALETTE_FW SET_MODE SET_IMG_FORMAT SET_MASK SET_OUTPUT SET_FOLDER SET_EXTRAPAR CONVERT CONVERT_PALETTE
# Info about each command can be found looking into its correspondent makefile macro IMG2SP_<COMMAND>
#
# When using CONVERT commands, IMGCFILES and OBJS2CLEAN are updated to add new C files that result from the pack generation.
#
define IMG2SP 
	# Check that command parameter ($(1)) is exactly one-word after stripping whitespaces
	$(if $(filter-out $(words $(strip $(1))),1)\
		,$(error $(I2S_ERR)]: '$(1)' is not a valid command. Commands must be exactly one-word in lenght with no whitespaces)\
		,)
	# Set the list of valid commands
	$(eval IMG2SP_F_FUNCTIONS = SET_PALETTE_FW SET_MODE SET_IMG_FORMAT SET_MASK SET_OUTPUT SET_FOLDER SET_EXTRAPAR CONVERT CONVERT_PALETTE)
	# Filter given command as $(1) to see if it is one of the valid commands
	$(eval IMG2SP_F_SF = $(filter $(IMG2SP_F_FUNCTIONS),$(1)))
	# If the given command is valid, it will be non-empty, then we proceed to call the command (up to 8 arguments). Otherwise, raise an error
	$(if $(IMG2SP_F_SF)\
		,$(eval $(call IMG2SP_$(IMG2SP_F_SF),$(strip $(2)),$(strip $(3)),$(strip $(4)),$(strip $(5)),$(strip $(6)),$(strip $(7)),$(strip $(8))))\
		,$(error $(I2S_ERR)]: '$(1)' is not a valid command. Valid commands are: {$(IMG2SP_F_FUNCTIONS)}))
endef

#################################################################################################################################################
### OLD MACROS (Deprecated)
### Maintained here for compatibility
#################################################################################################################################################

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
# $(7): (mask,tileset,zgtiles) 
#		"mask":    generate interlaced mask for all sprites converted
#       "tileset": generate a tileset array including pointers to all sprites
#		"zgtiles": generate tiles in Zig-Zag pixel order, Gray Code row order
# $(8): Output subfolder for generated .C and .H files (inside project folder)
# $(9): (hwpalette, 'palettename') 
#			""				: Do not generate a hardware palette array
#			"hwpalette"		: Generate a palette array as hardware values named 'g_palette'
#			"'palettename'" : Generate a palette array as hardware values with your own 'palettename' as C-identifier
# $(10): Aditional options (you can use this to pass aditional modifiers to cpct_img2tileset)
#
define IMG2SPRITES
	# Set up C and H files and paths
	$(eval I2S_C := $(basename $(1)).c)
	$(eval I2S_H := $(basename $(1)).h)
	$(eval $(call JOINFOLDER2BASENAME, I2S_C2, $(8), $(I2S_C)))
	$(eval $(call JOINFOLDER2BASENAME, I2S_H2, $(8), $(I2S_H)))
	$(eval I2S_CH := $(I2S_C2) $(I2S_H2))
	
	# Configure options for masks, zgtiles, tileset
	$(eval I2S_P := $(shell \
		if   [ "$(7)" = "mask"     ]; then echo "-nt -im"; \
		elif [ "$(7)" = "zgtiles"  ]; then echo "-z -g -nt"; \
		elif [ "$(7)" != "tileset" ]; then echo "-nt"; \
		fi))

	# Configure output of hwpalette
	$(eval I2S_P := $(I2S_P) $(shell \
		if   [ "$(9)"  = "hwpalette" ]; then echo "-oph"; \
		elif [ "$(9)" != ""          ]; then echo "-oph $(9)"; \
		fi))

# Generate target for image conversion
.SECONDARY: $(I2S_CH)
$(I2S_CH): $(1)
	@$(call PRINT,$(PROJNAME),"Converting $(1) into C-arrays...")
	$(IMG2TIL) $(I2S_P) -m "$(2)" -bnp "$(3)" -tw "$(4)" -th "$(5)" -pf $(6) $(10) $(1);
	@$(call PRINT,$(PROJNAME),"Moving generated files:")
	@$(call PRINT,$(PROJNAME)," - '$(I2S_C)' > '$(I2S_C2)'")
	@$(call PRINT,$(PROJNAME)," - '$(I2S_H)' > '$(I2S_H2)'")
	@if [ ! "$(8)" = "" ]; then \
	   mv "$(I2S_C)" "$(I2S_C2)"; \
	   mv "$(I2S_H)" "$(I2S_H2)"; \
	fi

# Variables that need to be updated to keep up with generated files and erase them on clean
IMGCFILES  := $(I2S_C2) $(IMGCFILES)
OBJS2CLEAN := $(I2S_CH) $(OBJS2CLEAN)
endef
