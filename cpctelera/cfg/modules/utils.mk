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
##           General Utility functions for other makefiles               ##
##-----------------------------------------------------------------------##
## This file contines general function definitions that other makefiles  ##
## use to simplify calculations. These are to be considered helper funcs.##
###########################################################################

## INCLUDED MACROS
##   * ADD2INTS
##   * ADD2SET
##   * ADD_N_ITEMS
##   * CHECKSYSTEMOSX
##   * CHECKSYSTEMCYGWIN
##   * CHECKVARIABLEISSET
##   * CONTAINED_IN
##   * CONVERTVALUE
##   * CONVERT_FW2HW_PALETTE
##   * CREATETMPFILENAME
##   * DEC2HEX
##   * EQUALS
##   * ENSURE_ADDRESS_VALID
##   * ENSURE_VALID_C_ID
##   * ENSURE_SINGLE_VALUE
##   * ENSUREFOLDERISREADABLE
##   * ENSUREVALID
##   * ENSUREFILEEXISTS
##   * FILEEXISTS
##   * FOLDEREXISTS
##   * FOLDERISREADABLE
##   * FOLDERISWRITABLE
##   * GET_IMG_SIZE
##   * GREATER_THAN
##   * HEX2DEC
##   * INTINRANGE
##   * ISHEX
##   * ISINT
##   * JOINFOLDER2BASENAME
##   * PRINT 
##   * REMOVEFILEIFEXISTS
##   * REPLACETAG_RT
##   * REPLACETAGGEDLINE
##   * REPLACETAGGEDLINE_RT
##   * SET_ONE_OF_MANY_VALID
##   * STRLEN
##   * SYSPATH
##   * VERIFY_FW_PALETTE
##

# ANSI Sequences for terminal colored printing
COLOR_RED    :=\033[1;31;49m
COLOR_GREEN  :=\033[1;32;49m
COLOR_YELLOW :=\033[1;33;49m
COLOR_BLUE   :=\033[1;34;49m
COLOR_MAGENTA:=\033[1;35;49m
COLOR_CYAN   :=\033[1;36;49m
COLOR_WHITE  :=\033[1;37;49m
COLOR_GREY   :=\033[1;38;49m
COLOR_NORMAL :=\033[0;39;49m

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
# JOINFOLDER2BASENAME: Returns a path to a file obtained by joining basename of the file (after
# removing dir path of the file, if it had) with the given path. FILE cannot be empty, PATH can.
# If PATH is empty, FILE is returned as is, without any changes (even if it has its own path).
# If PATH is non-empty, PATH is joined with the name of the FILE without is own PATH, if it had.
# Parameters are stripped previous to being used
#
# $(1): Variable where to store joined PATH + FILE_BASENAME
# $(2): PATH to be used for joining
# $(3): FILE to be used for joining
#
define JOINFOLDER2BASENAME
	# Strip parameters
	$(eval _J2B_1 := $(strip $(1)))
	$(eval _J2B_2 := $(strip $(2)))
	$(eval _J2B_3 := $(strip $(3)))
	# First, check if FILE is empty
	$(if $(_J2B_3),,$(error <<ERROR>> Empty filename when trying to join with its path. {VAR: '$(_J2B_1)', PATH: '$(_J2B_2)', FILE: '$(_J2B_3)'}))
	# Revome FILE's path if it had one
	$(eval _JF2BN_F := $(notdir $(_J2B_3)))
	# Depending on path being empty or not, joining has to be done in a different way
	$(eval $(_J2B_1) := $(shell \
		if [ "$(_J2B_2)" != "" ]; then \
			A="$(_J2B_2)"; A="$${A%%/}"; \
			echo "$${A}/$(_JF2BN_F)"; \
		else \
			echo "$(_J2B_3)"; \
		fi))
endef

########################
## SYSPATH
##   Returns System-Dependent valid path for a unix-style path
## input. This path is only changed (into a Windows-style path)
## when detected system is cygwin. Then, cygpath is used to 
## return the windows-style path.
##   $(1): Unix-style path
##
define SYSPATH
$(strip \
	$(if $(call CHECKSYSTEMCYGWIN)\
		,$(shell cygpath -w $(strip $(1)))\
		,$(strip $(1))\
	)\
)
endef

########################
## CHECKSYSTEMOSX
##   Returns "OSX" if system is OSX, empty otherwise.
## This is designed to be used in makefile if macros
##
define CHECKSYSTEMOSX
$(shell if [[ `uname` =~ "Darwin" ]]; then echo "OSX"; fi)
endef

########################
## CHECKSYSTEMCYGWIN
##   Returns "CYGWIN" if system is Cygwin, empty otherwise.
## This is designed to be used in makefile if macros
##
define CHECKSYSTEMCYGWIN
$(shell if [[ `uname` =~ "CYGWIN" ]]; then echo "CYGWIN"; fi)
endef

########################
## CREATETMPFILENAME
## Creates a temporary filename and returns it
##
define CREATETMPFILENAME
$(if $(call CHECKSYSTEMOSX)\
	,$(shell mktemp -t tmp.XXXXX)\
	,$(shell mktemp)
)
endef

########################
## REPLACETAGGEDLINE
## Replaces a complete line in a file which contains a given tag 
## $1: Tag to be searched
## $2: New line to replace the one that contains the tag
## $3: File to modify
## $4: sed deliminer (optional)
##
define REPLACETAGGEDLINE
	# Strip parameters
	$(eval _RTL_1 :=$(1))
	$(eval _RTL_2 :=$(2))
	$(eval _RTL_3 :=$(strip $(3)))
	# Check file exists
	$(call ENSUREFILEEXISTS,$(_RTL_3),[REPLACETAGGEDLINE]: <<ERROR>> File '$(_RTL_3)' not found when trying to replace tag '$(_RTL_1)' in it)
	# Set up sed delimiter
	$(eval _RTL_4 := $(if $(strip $(4)),$(strip $(4)),/))
	# Create temporary file
	$(eval _RTL_TMP := $(call CREATETMPFILENAME))
	# Replace tagged line with new line
	$(shell \
		if cat "$(_RTL_3)" | sed "s$(_RTL_4).*$(_RTL_1).*$(_RTL_4)$(_RTL_2)$(_RTL_4)g" > $(_RTL_TMP); then \
			mv "$(_RTL_TMP)" "$(_RTL_3)"; \
		fi)
endef

########################
## REPLACETAGGEDLINE_RT
## Replaces a complete line in a file which contains a given tag. This
## macro expands a piece of shell code that needs to be executed inside a rule
## in RealTime (after all preprocessing)
## $1: Tag to be searched
## $2: New line to replace the one that contains the tag
## $3: File to modify
## $4: sed deliminer (optional)
##
define REPLACETAGGEDLINE_RT
	@# Strip parameters
	$(eval _RTL_1 :=$(1))
	@#(eval _RTL_2 :=$(2)) << Doing this eliminates trailing spaces that are required
	$(eval _RTL_3 :=$(strip $(3)))
	@# Set up sed delimiter
	$(eval _RTL_4 := $(if $(strip $(4)),$(strip $(4)),/))
	@# Create temporary file
	$(eval _RTL_TMP := $(call CREATETMPFILENAME))
	@# Replace tagged line with new line
	@if cat "$(_RTL_3)" | sed "s$(_RTL_4).*$(_RTL_1).*$(_RTL_4)$(2)$(_RTL_4)g" > $(_RTL_TMP); then \
		mv "$(_RTL_TMP)" "$(_RTL_3)"; \
	fi
endef

########################
## REPLACETAG_RT
## Replaces all occurrences of tag inside a file with a given string. 
## This macro expands a piece of shell code that needs to be executed
## later on, inside a rule.
## $1: Tag to be searched
## $2: String to replace the tag
## $3: File to modify
## $4: sed delimiter (optional)
##
define REPLACETAG_RT
	@# Strip parameters
	$(eval _RTL_1 :=$(1))
	$(eval _RTL_2 :=$(2))
	$(eval _RTL_3 :=$(strip $(3)))
	@# Set up sed delimiter
	$(eval _RTL_4 := $(if $(strip $(4)),$(strip $(4)),/))
	@# Create temporary file
	$(eval _RTL_TMP := $(call CREATETMPFILENAME))
	@# Replace tagged line with new line
	@if cat "$(_RTL_3)" | sed "s$(_RTL_4)$(_RTL_1)$(_RTL_4)$(_RTL_2)$(_RTL_4)g" > $(_RTL_TMP); then \
		mv "$(_RTL_TMP)" "$(_RTL_3)"; \
	fi
endef

#################
# CONVERTVALUE: Takes two lists of values representing a mapping from list1 to list2 
# (search list to conversion list) and one value from list1 to be converted. It searches
# the value in list one and, if it is found, it assigns the Output Variable with
# the corresponding value from list2 (conversion value). If the value is not found,
# Output variable is assigned the Default value. If list2 is smaller than list1,
# latest elements from list1 are converted to empty.
# Warning: Parameters $(2), $(3) and $(5) are names or actual variables and not their contents.
#
# $(1): Searched value
# $(2): Variable holding search array
# $(3): Variable holding conversion array
# $(4): Default value (for not-found output)
# $(5): Output variable
#
define CONVERTVALUE
	$(eval _CNV := $(strip $($(3))))
	$(eval $(strip $(5)) := $(strip $(4)))
	$(eval $(foreach _F,$(strip $($(2))) \
				, $(if $(filter-out $(_F),$(strip $(1))) \
					, $(eval _CNV := $(filter-out $(firstword $(_CNV)),$(_CNV))) \
					, $(eval $(5) := $(firstword $(_CNV))) \
				)\
			)\
	 	)
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
# ISINT: Checks if a variable contains an integer or not. 
# If the variable contains an integer, string "true" is returned,
# otherwise, empty-string is returned. This is designed to be
# used in makefile if functions.
# Value is striped before being checked
#
# $(1): Value to be checked as integer
#
define ISINT
$(shell if [[ $(strip $(1)) =~ ^[-+]?[0-9]+$$ ]]; then echo "true"; fi)
endef

#################
# ISHEX: Checks if a variable contains an hexadecimal integer or not.
# If the variable contains an integer, string "true" is returned,
# otherwise, empty-string is returned. This is designed to be
# used in makefile if functions.
# Value is striped before being checked
#
# $(1): Value to be checked as an hexadecimal integer
#
define ISHEX
$(shell if [[ $(strip $(1)) =~ ^[-+]?0[xX][0-9A-Fa-f]+$$ ]]; then echo "true"; fi)
endef 


#################
# INTINRANGE: Checks if an integer value is in a given range or not. 
# If the value is in range, it returns the string "true". otherwise, 
# empty-string is returned. This is designed to be used in makefile 
# 'if' functions.
# Values are striped before being checked
#
# $(1): Value to be checked
# $(2): Minimum integer value of the range
# $(3): Maximum integer value of the range
#
define INTINRANGE
$(shell if [[ "$(strip $(1))" -ge "$(strip $(2))" && "$(strip $(3))" -ge "$(strip $(1))" ]]; then echo "true"; fi)
endef

#################
# ADD2INTS: Adds two integer values and echoes the result.
# Integers received are not checked. If a non-integer is passed, it will fail.
#
# $(1): First int to be added
# $(2): Second int to be added
#
define ADD2INTS
$(shell echo $$(( $(1) + $(2) )))
endef

#################
# ENSURE_VALID_C_ID: Checks if a given value is a valid C-identifier, if not, raises an error
#
# $(1): identifier to verify
# $(2): Error Message if it does not exist
#
define ENSURE_VALID_C_ID
	# Remove trailing whitespaces
	$(eval EICID := $(strip $(1)))
	
	# Ensure that given identifier is exactly 1 word in size
	$(eval $(if $(filter-out $(words $(EICID)),1)\
				,$(error $(strip $(2)))\
				,))

	# Check that the word matches C-identifier rules 
	$(eval $(if $(shell REX='^[a-zA-Z_][a-zA-Z0-9_]*$$'; if [[ $(EICID) =~ $$REX ]]; then echo "true"; fi)\
				,\
				,$(error $(strip $(2)))))
endef

#################
# ENSUREFILEEXISTS: Checks if a given file exists and, if not, raises an error.
# As this is a makefile macro, it is expanded and executed previous to rule execution.
#
# $(1): file
# $(2): Error Message if it does not exist
#
define ENSUREFILEEXISTS
	$(eval EFE_THEFILE := $(strip $(1)))
	$(if $(filter-out $(wildcard $(EFE_THEFILE)),$(EFE_THEFILE))\
		,$(error $(strip $(2)))\
		,)
endef

#################
# ENSUREFOLDERISREADABLE: Checks if a given folder exists and is readable and
# if not, raises an error.
#
# $(1): folder
# $(2): Error Message if it does not exist
#
define ENSUREFOLDERISREADABLE
	$(if $(call FOLDERISREADABLE,$(strip $(1)))\
		,\
		,$(error $(strip $(2)))\
	)
endef


#################
# ENSUREVALID: Checks that a value or a given list of values is/are
# contained into a list of valid values. If any value from the seach
# list is not found in the valid list, an error is raised.
#
# $(1): Search list: values to be checked as being valid
# $(2): Valid list: valid values
# $(3): Error message to display when a non-valid value is found
#
define ENSUREVALID
	$(foreach EV_V,$(strip $(1))\
		,$(if $(filter-out $(strip $(2)),$(EV_V))\
			,$(error <<ERROR>> '$(EV_V)' $(strip $(3)). Valid values are: {$(strip $(2))})\
			,))
endef

#################
# SET_ONE_OF_MANY_VALID: Ensures that a given variable is set a value amongst
# those valid ones. If the value is contained in the valid list of values,
# it is assigned. If not, an error message is raised. Values are allways stripped,
# so that leading and trailing whitespaces do not count.
# If the value passed contains more than 1 value (has whitespaces inside)
# an error is raised too.
# Warning: $(1) is a variable name and not its contents
#
# $(1): Variable to be assigned to
# $(2): Value to be assigned
# $(3): List of valid values
# $(4): Error message in case of failure
#
define SET_ONE_OF_MANY_VALID
	# Error message
	$(eval S1OM_ERR := <<ERROR>> $(strip $(4)). Valid values are: {$(strip $(3))})
	
	# Check that the passed value is unique
	$(if $(filter-out $(words $(strip $(2))),1),$(error $(S1OM_ERR)),)

	# Check that the passed value is valid
	$(if $(filter-out $(strip $(3)),$(strip $(2))),$(error $(S1OM_ERR)),)

	# Assign the valid value
	$(eval $(strip $(1)) := $(strip $(2)))
endef

#################
# ADD2SET: Adds a given value to a set of values. The value is
# not added if it is already in the set. If it is not, it is added.
# Values are stripped so that leading and trailing whitespaces do not count.
# Warning: $(1) is a variable name and not its contents
#
# $(1): Variable containing the set (to be modified adding the new value)
# $(2): New value to be added to the set
#
define ADD2SET
	$(eval _S := $(strip $(1))) 
	$(eval _E := $(strip $(2))) 
	$(if $(filter $($(_S)),$(_E))\
		,\
		,$(eval $(_S) := $($(_S)) $(_E)))
endef

#################
# CONVERT_FW2HW_PALETTE: Converts Firmware palette values to Hardware equivalents.
# Given palette must be verified previously to contain integers from 0 to 26 only.
# Warning: $(2) is a variable name and not its contents
#
# $(1): Palette values to be converted
# $(2): Output variable with converted palette
#
_HWPAL := 54 44 55 5C 58 5D 4C 45 4D 56 46 57 5E 40 5F 4E 47 4F 52 42 53 5A 59 5B 4A 43 4B
define CONVERT_FW2HW_PALETTE
	$(foreach _PV,$(1),\
		$(eval _PV1 := $(call ADD2INTS,$(_PV),1))\
		$(eval $(2) := $($(2)) $(word $(_PV1),$(_HWPAL)))\
		)
endef

#################
# VERIFY_FW_PALETTE: Verifies that a given Firmware palette is valid, raising an
# error if there is some inconsistency in the palette.
#
# $(1): Palette values
# $(2): Name of the macro that called (for error msgs)
#
define VERIFY_FW_PALETTE
	# Check Palette for correctness
	$(eval _MSG:=is not a valid firmware value [$(2)]. Values must be integers from 0 to 26)
	$(foreach _I,$(1),\
		$(if $(call ISINT,$(_I)),,$(error <<ERROR>> '$(_I)' $(_MSG))) \
		$(if $(call INTINRANGE,$(_I),0,26),,$(error <<ERROR>> '$(_I)' $(_MSG)))\
		)

	# Check that palette has a valid size
	$(if $(call INTINRANGE,$(words $(1)),1,16),,$(error <<ERROR>> [$(2)] Firmware palette must have at least 1 element and 16 at most. $(ITEM) is not a valid size.))
endef

#################
# ENSURE_SINGLE_VALUE: Ensures that passed parameter is comprised of a single 
# value (be it a word, and integer or whatever) after stripping whitespaces.
# If the value is not single, throw an error (given as second parameter)
#
# $(1): Value to be checked as single
# $(2): Error message in case the value is not single
#
define ENSURE_SINGLE_VALUE
$(if $(call EQUALS,$(words $(strip $(1))),1),,$(error $(strip $(2))))
endef

#################
# GREATER_THAN: Performs a check about an integer being greater than
# other, returning string "true" or empty string (false) depending 
# on the result. It is thought to be used in conjunction with makefile if functions
#
# $(1): Left value 
# $(2): right value
#
define GREATER_THAN
$(shell test "$(strip $(1))" -gt "$(strip $(2))" && echo true)
endef

#################
# CONTAINED_IN: Checks if a value is contained in a set or not
# It returns de value if it is contained or empty (false) if not.
# It is thought to be used in conjunction with makefile if functions.
#
# $(1): Value to be checked in the set
# $(2): Set of values separated by spaces
#
define CONTAINED_IN
$(filter $(strip $(1)),$(strip $(2)))
endef

#################
# ADD_N_ITEMS: Adds $(2) times the item $(3) to at the end of the contents of a 
# given variable $(1), separated by spaces.
# Warning: $(1) is a variable name and not its contents
#
# $(1): Variable where contents will be added at its end
# $(2): Number of elements to be added
# $(3): element
#
define ADD_N_ITEMS
	# Performs only if N is greater than 0
	$(eval _TMP_I := $(shell seq 1 $(2)))
	$(foreach _I,$(_TMP_I),$(eval $(1) := $($(1)) $(strip $(3))))
endef

#################
# STRLEN: Evals to the length of the string passed as parameter $(1)
# after being stripped.
#
# $(1): String to calculate its length
#
define STRLEN
$(shell _SLV='$(strip $(1))' && echo $${#_SLV})
endef

#################
# REMOVEFILEIFEXISTS: Removes a file if it exists in the filesystem,
# doing nothing if the file does not exist. Path to the file must 
# be relative to prevent problems with failed absolute paths. A ./
# sign will be added in front before removing the file. Directories
# are never removed in any case
#
# $(1): File (with relative path)
#
define REMOVEFILEIFEXISTS
$(shell $(RM) "./$(1)")
endef

#################
# FILEEXISTS: Checks if a given file exists or not. Returns "true" 
# when the file exists and empty-string otherwise. It is thought to
# be used in makefile if functions. Parameter is striped before 
# being tested.
#
# $(1): File
#
define FILEEXISTS
$(shell if [ -f "$(strip $(1))" ]; then echo true; fi)
endef

#################
# FOLDEREXISTS: Checks if a given folder exists or not. Returns "true" 
# when the file exists and empty-string otherwise. It is thought to
# be used in makefile if functions. Parameter is striped before 
# being tested.
#
# $(1): folder (must not be a file)
#
define FOLDEREXISTS
$(shell if [ -d "$(strip $(1))" ]; then echo true; fi)
endef

#################
# FOLDERISWRITABLE: Checks if a given folder exists and is writable 
# or not. Returns "true" when the file exists and empty-string 
# otherwise. It is thought to be used in makefile if functions. 
# Parameter is striped before being tested.
#
# $(1): folder (must not be a file)
#
define FOLDERISWRITABLE
$(shell if [[ -d "$(strip $(1))" && -w "$(strip $(1))" && -x "$(strip $(1))" ]]; then echo true; fi)
endef

#################
# FOLDERISREADABLE: Checks if a given folder exists and is readable
# or not. Returns "true" when the file exists and empty-string 
# otherwise. It is thought to be used in makefile if functions. 
# Parameter is striped before being tested.
#
# $(1): folder (must not be a file)
#
define FOLDERISREADABLE
$(shell if [[ -d "$(strip $(1))" && -r "$(strip $(1))" && -x "$(strip $(1))" ]]; then echo true; fi)
endef

#################
# EQUALS: Checks if a given two given values are equal (they are 
# treated as strings). If both are equal, it returns "true". otherwise
# it returns empty-string. It is thought to be used in makefile if functions.
# Parameter is striped before being tested.
#
# $(1): First member to be tested
# $(2): Second member to be tested
#
define EQUALS
$(shell if [ "$(strip $(1))" = "$(strip $(2))" ]; then echo true; fi)
endef

#################
# GET_IMG_SIZE: It evaluates to a string including width, height
# and filename of the given image file. This string can then be 
# easily patched using 'word' makefile function to obtain image
# sizes. If given image file has any problem and does not return 
# its size (because it is not a valid file or valid image file, 
# for instance), the error message passed as $(2) is raised.
#
# $(1): Image file 
# $(2): Error msg
#
define GET_IMG_SIZE
$(eval _SIZES   := $(shell $(IMG2CPC) --img-size "$(strip $(1))")) \
$(eval _SWIDTH  := $(word 1,$(_SIZES))) \
$(if $(call EQUALS,xxx,$(_SWIDTH)),$(error $(strip $(2))),) \
$(_SIZES)
endef

#################
# HEX2DEC: It converts an hexadecimal value to 
# a decimal one.
#
# $(1): hexadecimal value to be converted
#
define HEX2DEC
$(shell printf "%d" "$(strip $(1))")
endef

#################
# DEC2HEX: It converts a decimal value to an hexadecimal one.
#
# $(1): Decimal value to be converted
#
define DEC2HEX
$(shell printf "0x%x" "$(strip $(1))")
endef

#################
# ENSURE_ADDRESS_VALID: Ensures that a given memory 
# address is a valid 16-bits address, checking that it is
# either a decimal or hexadecimal number between 0 and 65535.
# If it is not valid, it raises an error.
#
# $(1): Address to be checked
# $(2): Error Message Header from the routine that called
#
define ENSURE_ADDRESS_VALID
# Error MSG header
$(eval _R := $(2))

# Check that load address is valid
$(eval _A := $(strip $(1)))
$(call ENSURE_SINGLE_VALUE,$(_A),$(_R) '$(1)' is not a valid 16-bits address. It is not a single value)
$(if $(call ISHEX,$(_A))\
	,$(eval _A := $(call HEX2DEC,$(_A)))\
	,$(if $(call ISINT,$(_A)),,$(error $(_R) '$(1)' is not a valid 16-bits address. It is neither a decimal, nor an hexadecimal integer)))
$(if $(call INTINRANGE,$(_A),0,65535),,$(error $(_R) '$(1)' is not a valid 16-bits address. It is not in the range [0 - 65535]/[0x0000-0xFFFF]))
endef
