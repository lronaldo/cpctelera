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
	$(eval J2B_1 := $(strip $(1)))
	$(eval J2B_2 := $(strip $(2)))
	$(eval J2B_3 := $(strip $(3)))
	# First, check if FILE is empty
	$(if $(J2B_3),,$(error <<ERROR>> Empty filename when trying to join with its path. {VAR: '$(J2B_1)', PATH: '$(J2B_2)', FILE: '$(J2B_3)'}))
	# Revome FILE's path if it had one
	$(eval JF2BN_F := $(notdir $(J2B_3)))
	# Depending on path being empty or not, joining has to be done in a different way
	$(eval $(J2B_1) := $(shell \
		if [ "$(J2B_2)" != "" ]; then \
			A="$(J2B_2)"; A="$${A%%/}"; \
			echo "$${A}/$(JF2BN_F)"; \
		else \
			echo "$(J2B_3)"; \
		fi))
	$(eval undefine JF2BN_F)
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
# $(3): Variable golding conversion array
# $(4): Default value (for not-found output)
# $(5): Output variable
#
define CONVERTVALUE
	$(eval CO_CNV := $($(3)))
	$(eval $(5) := $(4))
	$(eval $(foreach F,$($(2))\
		,$(if $(filter-out $F,$(1))\
			,$(eval CO_CNV  := $(filter-out $(firstword $(CO_CNV)),$(CO_CNV)))\
			,$(eval $(5)    := $(firstword $(CO_CNV))))))
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
# If the variable contains an integer, it remains unchanged.
# If the variable does not contains an integer, it gets emptied.
# Warning: $(1) must be a variable name, not its contents
#
# $(1): Variable name to be checked
#
define ISINT
	$(eval $(1) := $(shell if [[ $($(1)) =~ ^[-+]?[0-9]+$$ ]]; then echo "$($(1))"; else echo ""; fi))
endef

#################
# INTINRANGE: Checks if a variable containing an integer value is in a given range or not.
# If the value is in range, it remains unchanged.
# If the value is not in range, it gets emptied.
# Warning: $(1) must be a variable name, not its contents
#
# $(1): Variable name to be checked
# $(2): Minimum integer value of the range
# $(3): Maximum integer value of the range
#
define INTINRANGE
	$(eval $(1) := $(shell if (( $($(1)) >= $(2) && $($(1)) <= $(3) )); then echo "$($(1))"; else echo ""; fi))
endef

#################
# ADD2INTS: Adds two integer values and places result in a given variable.
# Integers received are not checked. If a non-integer is passed, it will fail.
# Warning: $(3) must be a variable name, not its contents
#
# $(1): First int to be added
# $(2): Second int to be added
# $(3): Variable to store the result of addition
#
define ADD2INTS
	$(eval $(3) := $(shell echo $$(( $(1) + $(2) ))))
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
# ENSUREFILEEXISTS: Checks if a given file exists and, if not, raises an error
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