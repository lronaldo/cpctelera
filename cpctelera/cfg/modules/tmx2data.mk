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
##                 TMX2DATA General functionalities                      ##
##-----------------------------------------------------------------------##
## This file contains all macros related to TMX2DATA general             ##
## functionality used to automate the conversion between TMX tilemaps    ##
## and their array/binary representation. TMX2DATA is a general macro    ##
## that automates this conversion and has several submacros for diferent ##
## tasks. All these macros are included here.                            ##
## TMX2CSV old macro is also maintained for backwards compatibility.     ##
###########################################################################

#
# Default values for all TMX2DATA functions
#
T2D_FILE 	:=no_file
T2D_BITARR	:=
T2D_CIDENT	:= 
T2D_GEN     :=-gc -gh
T2D_GENF	   :=.c .h
T2D_NUMBASE :=
T2D_CMACROS	:=
T2D_ASMPREF	:=-au
T2D_OUTFOLD	:=src/
T2D_EXTRA   :=

# Ensure that tilemap_conversion.mk exists for compatibility with older CPCtelera projects
T2D_DEPEND  := cfg/tilemap_conversion.mk
TOUCHIFNOTEXIST := $(TOUCHIFNOTEXIST) $(T2D_DEPEND)


#################
# TMX2DATA_SET_ASMVARPREFIX: Sets an underscore prefix for all
# assembly variables and labels.
#
# $(1): Asm Var Prefix (yes/no)
#
define TMX2DATA_SET_ASMVARPREFIX
	# Check that the passed value is valid and assign it 
	$(eval T2D_VALID := yes no)
	$(call SET_ONE_OF_MANY_VALID,_S,$(1),$(T2D_VALID),[TMX2DATA: SET_ASMVARPREFIX] '$(1)' is not valid.)
	$(eval T2D_ASMPREF := $(if $(call EQUALS,$(_S),yes),-au,))
endef

#################
# TMX2DATA_SET_USEMACROS: Sets if textual conversion must use CPCtelera
# Macros or not for the values inserted in bitarrays. Valid values: yes / no
#
# $(1): Use Macros (yes/no)
#
define TMX2DATA_SET_USEMACROS
	# Check that the passed value is valid and assign it 
	$(eval T2D_VALID := yes no)
	$(call SET_ONE_OF_MANY_VALID,_S,$(1),$(T2D_VALID),[TMX2DATA: SET_USEMACROS] '$(1)' is not valid.)
	$(eval T2D_CMACROS := $(if $(call EQUALS,$(_S),no),-nm,))
endef


#################
# TMX2DATA_SET_FOLDER: Sets the output folder where all generated
# files will be stored
#
# $(1): Output folder
#
define TMX2DATA_SET_FOLDER
	$(call ENSUREFILEEXISTS,$(1),<<ERROR>> [TMX2DATA: SET_FOLDER] Folder '$(1)' does not exist or is not accessible)
	$(eval T2D_OUTFOLD := $(1))
endef

#################
# TMX2DATA_SET_OUTPUTS: Selects the output formats that will be produced.
# One file will be produced for each selected output format.
# Valid output formats are: c (c-file) h (c-header) hs (asm-header) s (asm-file) bin (binary)
#
# $(1): List of output formats to be generated
#
define TMX2DATA_SET_OUTPUTS
	# Check that the passed value is valid and assign it 
	$(eval T2D_VALID := c h hs s bin)
	$(eval T2D_O := $(1))
	$(call ENSUREVALID,$(T2D_O),$(T2D_VALID),is not a valid output format [TMX2DATA: SET_OUTPUTS].)
	# Convert outputs
	$(eval T2D_CON := .c .h .h.s .s .bin)
	$(eval T2D_GENF :=)
	$(foreach VAL,$(T2D_O),$(call CONVERTVALUE,$(VAL),T2D_VALID,T2D_CON,,VAL2) $(call ADD2SET,T2D_GENF,$(VAL2)))
	$(eval T2D_CON := -gc -gh -ghs -gs -gb)
	$(eval T2D_GEN :=)
	$(foreach VAL,$(T2D_O),$(call CONVERTVALUE,$(VAL),T2D_VALID,T2D_CON,,VAL2) $(call ADD2SET,T2D_GEN,$(VAL2)))
endef


#################
# TMX2DATA_SET_BASE: Sets the numerical base to be used on output values
# Valid numerical bases are: decimal, binary and hexadecimal
#
# $(1): Numerical base
#
define TMX2DATA_SET_BASE
	# Check that the passed value is valid and assign it 
	$(eval T2D_VALID := dec hex bin)
	$(call SET_ONE_OF_MANY_VALID,_S,$(1),$(T2D_VALID),[TMX2DATA: SET_BASE] '$(1)' is not a valid numerical base.)
	$(eval T2D_NUMBASE := -nb $(_S))
endef


#################
# TMX2DATA_SET_BITSPERITEM: Sets the number of bits per item that will be used 
# when converting the TMX into tilemap data. Values lower than 8 will result in
# bitarray-tilemaps to be generated. Valid values: 1, 2, 4, 6, 8
#
# $(1): Bits per item
#
define TMX2DATA_SET_BITSPERITEM
	# Check that the passed value is valid and assign it 
	$(eval T2D_VALID:=1 2 4 6 8)
	$(call SET_ONE_OF_MANY_VALID,T2D_BITARR,$(1),$(T2D_VALID),[TMX2DATA: SET_BITSPERITEM] '$(1)' is not a valid number.)
	$(eval T2D_BITARR := -ba $(T2D_BITARR))
endef

#################
# TMX2DATA_SET_EXTRAPAR: Sets the extra parameters to be passed to CPCTTMX2DT conversion command
#
# $(1): Extra parameters to be passed
#
#
define TMX2DATA_SET_EXTRAPAR
	$(eval T2D_EXTRA := $(1))
endef

#################
# TMX2DATA_CONVERT: General rule to convert TMX tilemaps into C arrays.
# Updates IMGCFILES and OBJS2CLEAN adding new C files that result from 
# tmx conversions
#
# $(1): TMX file to be converted to C array
# $(2): C identifier for the generated C array
#
# Updates IMGCFILES, IMGASMFILES, IMGBINFILES and OBJS2CLEAN according to files to be generated
#
define TMX2DATA_CONVERT
	# Generate all output filenames
	$(eval T2D_GFS:=) 
	$(eval T2D_JGFS:=) 
	$(foreach EXT,$(T2D_GENF),$(eval T2D_GFS := $(T2D_GFS) $(basename $(1))$(EXT)))
	$(foreach   F,$(T2D_GFS),$(call JOINFOLDER2BASENAME, F2, $(T2D_OUTFOLD), $(F)) $(eval T2D_JGFS := $(T2D_JGFS) $(F2)))

	# Get C, ASM and BIN files that are going to be generated
	$(eval T2D_CFILES   := $(filter %.c,$(T2D_JGFS)))
	$(eval T2D_ASMFILES := $(filter %.s %.h.s,$(T2D_JGFS)))
	$(eval T2D_BINFILES := $(filter %.bin,$(T2D_JGFS)))

	# Add C-identifier option if there is a C-identifier provided
	$(eval T2D_CIDENT := $(if $(2),-ci $(2),))

# Generate target for tilemap conversion
.SECONDARY: $(T2D_JGFS)
$(T2D_JGFS): $(1) $(T2D_DEPEND)
	@$(call PRINT,$(PROJNAME),"Converting tilemap in $(1) into data...")
	$(CPCTMX2DT) $(T2D_NUMBASE) $(T2D_CMACROS) $(T2D_ASMPREF) $(T2D_GEN) $(T2D_CIDENT) $(T2D_BITARR) -of $(T2D_OUTFOLD) $(T2D_EXTRA) $(1)

# Variables that need to be updated to keep up with generated files and erase them on clean
IMGCFILES  := $(T2D_CFILES) $(IMGCFILES)
IMGASMFILES:= $(T2D_ASMFILES) $(IMGASMFILES)
IMGBINFILES:= $(T2D_BINFILES) $(IMGBINFILES)
OBJS2CLEAN := $(T2D_JGFS) $(OBJS2CLEAN)
endef

#################
# TMX2DATA: Front-end to access all functionalities of TMX2DATA macros about TMX file 
# conversion into data (arrays and/or binary files)
#
# $(1): Command to be performed
# $(2-8): Valid arguments to be passed to the selected command
#
# Valid Commands: SET_FOLDER SET_ASMVARPREFIX SET_USEMACROS SET_BASE SET_OUTPUTS SET_BITSPERITEM SET_EXTRAPAR CONVERT
# Info about each command can be found looking into its correspondent makefile macro TMX2DATA_<COMMAND>
#
# When using CONVERT commands, IMGCFILES and OBJS2CLEAN are updated to add new C files that result from the pack generation.
#
define TMX2DATA 
	# Set the list of valid commands
	$(eval TMX2DATA_F_FUNCTIONS = SET_FOLDER SET_ASMVARPREFIX SET_USEMACROS SET_BASE SET_OUTPUTS SET_BITSPERITEM SET_EXTRAPAR CONVERT)
	# Check that command parameter ($(1)) is exactly one-word after stripping whitespaces
	$(if $(filter-out $(words $(strip $(1))),1)\
		,$(error <<ERROR>> [TMX2DATA] '$(strip $(1))' is not a valid command. Commands must be exactly one-word in lenght with no whitespaces. Valid commands: {$(TMX2DATA_F_FUNCTIONS)})\
		,)
	# Filter given command as $(1) to see if it is one of the valid commands
	$(eval TMX2DATA_F_SF = $(filter $(TMX2DATA_F_FUNCTIONS),$(1)))
	# If the given command is valid, it will be non-empty, then we proceed to call the command (up to 8 arguments). Otherwise, raise an error
	$(if $(TMX2DATA_F_SF)\
		,$(eval $(call TMX2DATA_$(TMX2DATA_F_SF),$(strip $(2)),$(strip $(3)),$(strip $(4)),$(strip $(5)),$(strip $(6)),$(strip $(7)),$(strip $(8))))\
		,$(error <<ERROR>> [TMX2DATA] '$(strip $(1))' is not a valid command. Valid commands: {$(TMX2DATA_F_FUNCTIONS)}))
endef

#################################################################################################################################################
### OLD MACROS (Deprecated)
### Maintained here for compatibility
#################################################################################################################################################

#################
# TMX2C: General rule to convert TMX tilemaps into C arrays.
# Updates IMGCFILES and OBJS2CLEAN adding new C files that result from 
# tmx conversions
#
# $(1): TMX file to be converted to C array
# $(2): C identifier for the generated C array
# $(3): Output folder for C and H files generated (Default same folder)
# $(4): Bits per item (1,2,4 or 6 to codify tilemap into a bitarray). Blanck for normal integer tilemap array
# $(5): Aditional options (you can use this to pass aditional modifiers to cpct_tmx2csv)
#
define TMX2C
	# Set up C and H files for output
	$(eval T2C_C := $(basename $(1)).c)
	$(eval T2C_H := $(basename $(1)).h)
	$(eval $(call JOINFOLDER2BASENAME, T2C_C2, $(3), $(T2C_C)))
	$(eval $(call JOINFOLDER2BASENAME, T2C_H2, $(3), $(T2C_H)))
	$(eval T2C_CH := $(T2C_C2) $(T2C_H2))

	# Configure options for output folder $(3) and bits per item $(4)
	$(eval T2C_OF := $(shell if [ "$(3)" != "" ]; then echo "-of $(3)"; else echo ""; fi))
	$(eval T2C_BA := $(shell if [ "$(4)" != "" ]; then echo "-ba $(4)"; else echo ""; fi))

# Generate target for tilemap conversion
.SECONDARY: $(T2C_CH)
$(T2C_CH): $(1) 
	@$(call PRINT,$(PROJNAME),"Converting tilemap in $(1) into C-arrays...")
	$(TMX2CSV) -gh -ci $(2) $(T2C_OF) $(T2C_BA) $(5) $(1)

# Variables that need to be updated to keep up with generated files and erase them on clean
IMGCFILES  := $(T2C_C2) $(IMGCFILES)
OBJS2CLEAN := $(T2C_CH) $(OBJS2CLEAN)
endef