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
##               AKS2DATA General functionalities                        ##
##-----------------------------------------------------------------------##
## This file contains all macros related to AKS2DATA general             ##
## functionality, used to automate the conversion of Arkos music files   ##
## to data directly usable by programs.                                  ##
## AKS2DATA is a general macro that automates this conversion and has    ##
## several submacros for diferent tasks. All these macros are included   ##
## here.                                                                 ##
###########################################################################

#
# Default values for all AKS2DATA functions
#
A2D_OUTFOLD := $(SRCDIR)
A2D_ERR     := <<ERROR>> [AKS2DATA -
A2D_GEN     :=-gs -gh
A2D_GENF    :=.s .h
A2D_SFX     :=
A2D_EXTRAPAR:=

# Ensure that music_conversion.mk exists for compatibility with older CPCtelera projects
A2D_DEPEND  := cfg/music_conversion.mk
TOUCHIFNOTEXIST := $(TOUCHIFNOTEXIST) $(A2D_DEPEND)


#################
# AKS2DATA_SET_EXTRAPAR: Sets additinal paramters to be 
# passed to cpct_aks2c when called.
#
# $(1): Extra parameters
#
define AKS2DATA_SET_EXTRAPAR
	$(eval A2D_EXTRAPAR := $(1))
endef

#################
# AKS2DATA_SET_FOLDER: Sets the output folder where all generated
# files will be stored
#
# $(1): Output folder
#
define AKS2DATA_SET_FOLDER
	$(if $(call FOLDERISWRITABLE,$(1)),,$(error $(A2D_ERR) SET_FOLDER] Folder '$(1)' does not exist, is not a folder or is not accessible))
	$(eval A2D_OUTFOLD := $(1))
endef

#################
# AKS2DATA_SET_SFXONLY: Configures the output of the CONVERT command
# to be for SFX-ONLY songs or not
#
# $(1): yes/no
#
define AKS2DATA_SET_SFXONLY
	# Only yes/no are valid 
	$(if $(call EQUALS,$(1),yes),    $(eval A2D_SFX := -sfx)\
		,$(if $(call EQUALS,$(1),no),$(eval A2D_SFX :=)\
			,$(error $(A2D_ERR) SET_SFXONLY]: '$(1)' is not a valid parameter. Valid values are { yes, no })\
		)\
	)
endef


#################
# AKS2DATA_SET_OUTPUTS: Selects the output formats that will be produced.
# One file will be produced for each selected output format.
# Valid output formats are: h (c-header) hs (asm-header) s (asm-file) bin (binary)
#
# $(1): List of output formats to be generated
#
define AKS2DATA_SET_OUTPUTS
	# Check that the passed value is valid and assign it 
	$(eval _VALID := h hs s bin)
	$(call ENSUREVALID,$(1),$(_VALID),is not a valid output format [AKS2DATA - SET_OUTPUTS])
	# Convert outputs
	$(eval _CON := .h .h.s .s .bin)
	$(eval A2D_GENF :=)
	$(foreach _V,$(1),$(call CONVERTVALUE,$(_V),_VALID,_CON,,_V2) $(call ADD2SET,A2D_GENF,$(_V2)))
	$(eval _CON := -gh -ghs -gs -gb)
	$(eval A2D_GEN :=)
	$(foreach _V,$(1),$(call CONVERTVALUE,$(_V),_VALID,_CON,,_V2) $(call ADD2SET,A2D_GEN,$(_V2)))
endef


#################
# AKS2DATA_CONVERT: Generates a Prebuild rule to convert a given
# music file into data.
#
# $(1): AKS file to be converted to data array
# $(2): C identifier for the generated data array (will have underscore in front on ASM)
# $(3): Memory address where music data will be loaded
#
# Updates IMGASMFILES, IMGBINFILES and PREBUILDOBJS as required
#
define AKS2DATA_CONVERT
	# Ensure non-empty parameters
	$(if $(1),,$(error $(A2D_ERR) CONVERT]: An AKS/SKS file is requiered as first parameter for CONVERT command))
	$(if $(2),,$(error $(A2D_ERR) CONVERT]: A C-identifier is requiered as second parameter for CONVERT command))
	$(if $(3),,$(error $(A2D_ERR) CONVERT]: A 16-bits memory address is requiered as third parameter for CONVERT command))

	# Ensure that AKS file exists and C_identifier and Memory address are valid
	$(call ENSUREFILEEXISTS,$(1),$(A2D_ERR) CONVERT]: File '$(1)' does not exist or is not readable)
	$(call ENSURE_VALID_C_ID,$(2),$(A2D_ERR) CONVERT]: '$(2)' is not a valid C-identifier)
	$(call ENSURE_ADDRESS_VALID,$(3),$(A2D_ERR) CONVERT]:)

	# Set up files to be produced
	$(eval _OBJS:=)
	$(foreach _E,$(A2D_GENF)\
		, $(eval _F := $(basename $(1))$(_E))\
		  $(call JOINFOLDER2BASENAME,_P,$(A2D_OUTFOLD),$(_F))\
		  $(eval _OBJS := $(_OBJS) $(_P))\
		)

# Generate target for music converstion
$(_OBJS): $(1) $(A2D_DEPEND)
	@$(call PRINT,$(PROJNAME),"Converting music in $(1) into data...")
	$(CPCTAKS2C) $(A2D_GEN) $(A2D_SFX) $(A2D_EXTRAPAR) -m "$(3)" -od "$(A2D_OUTFOLD)" -id "$(2)" "$(1)"

# Variables that need to be updated to keep up with generated files and erase them on clean
	$(eval _F := $(filter %.s,$(_OBJS)))
	$(if $(_F),$(eval IMGASMFILES := $(_F) $(IMGASMFILES)))
	$(eval _F := $(filter %.bin,$(_OBJS)))
	$(if $(_F),$(eval IMGBINFILES := $(_F) $(IMGBINFILES)))
	$(eval OBJS2CLEAN  := $(_OBJS) $(OBJS2CLEAN))
	$(eval PREBUILDOBJS := $(PREBUILDOBJS) $(_OBJS))
endef



#################
# AKS2DATA: Front-end to access all functionalities of AKS2DATA macros about Arkos 
# music conversion into data for programs.
#
# $(1): Command to be performed
# $(2-8): Valid arguments to be passed to the selected command
#
# Valid Commands: SET_FOLDER SET_OUTPUTS SET_SFXONLY SET_EXTRAPAR CONVERT 
# Info about each command can be found looking into its correspondent makefile macro AKS2DATA_<COMMAND>
#
define AKS2DATA
	# Set the list of valid commands
	$(eval AKS2DATA_F_FUNCTIONS := SET_FOLDER SET_OUTPUTS SET_SFXONLY SET_EXTRAPAR CONVERT)

	# Check that command parameter ($(1)) is exactly one-word after stripping whitespaces
	$(call ENSURE_SINGLE_VALUE,$(1),<<ERROR>> [AKS2DATA] '$(strip $(1))' is not a valid command. Commands must be exactly one-word in lenght with no whitespaces. Valid commands: {$(AKS2DATA_F_FUNCTIONS)})

	# Filter given command as $(1) to see if it is one of the valid commands
	$(eval AKS2DATA_F_SF = $(filter $(AKS2DATA_F_FUNCTIONS),$(1)))

	# If the given command is valid, it will be non-empty, then we proceed to call the command (up to 8 arguments). Otherwise, raise an error
	$(if $(AKS2DATA_F_SF)\
		,$(eval $(call AKS2DATA_$(AKS2DATA_F_SF),$(strip $(2)),$(strip $(3)),$(strip $(4)),$(strip $(5)),$(strip $(6)),$(strip $(7)),$(strip $(8))))\
		,$(error <<ERROR>> [AKS2DATA] '$(strip $(1))' is not a valid command. Valid commands: {$(AKS2DATA_F_FUNCTIONS)}))
endef

#################################################################################################################################################
### OLD MACROS (Deprecated)
### Maintained here for compatibility
#################################################################################################################################################

#################
# AKS2C: General rule to convert AKS music files into data arrays usable from C and ASM.
# Updates IMGASMFILES and OBJS2CLEAN adding new .s/.h files that result from AKS conversions
#
# $(1): AKS file to be converted to data array
# $(2): C identifier for the generated data array (will have underscore in front on ASM)
# $(3): Output folder for .s and .h files generated (Default same folder)
# $(4): Memory address where music data will be loaded
# $(5): Aditional options (you can use this to pass aditional modifiers to cpct_aks2c)
#
define AKS2C
	# Set up C and H files for output
	$(eval A2C_S := $(basename $(1)).s)
	$(eval A2C_H := $(basename $(1)).h)
	$(eval $(call JOINFOLDER2BASENAME, A2C_S2, $(3), $(A2C_S)))
	$(eval $(call JOINFOLDER2BASENAME, A2C_H2, $(3), $(A2C_H)))
	$(eval A2C_SH := $(A2C_S2) $(A2C_H2))

	# Configure options for output folder $(3)
	$(eval A2C_OF := $(shell if [ ! "$(3)" = "" ]; then echo "-od $(3)"; else echo ""; fi))

# Generate target for music converstion
.SECONDARY: $(A2C_SH)
$(A2C_SH): $(1)
	@$(call PRINT,$(PROJNAME),"Converting music in $(1) into data arrays...")
	$(CPCTAKS2C) -m "$(4)" $(A2C_OF) -id $(2) $(5) $(1)

# Variables that need to be updated to keep up with generated files and erase them on clean
IMGASMFILES := $(A2C_S2) $(IMGASMFILES)
OBJS2CLEAN  := $(A2C_SH) $(OBJS2CLEAN)
endef
