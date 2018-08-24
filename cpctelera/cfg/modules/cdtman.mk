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
##                 CDTMAN General functionalities                        ##
##-----------------------------------------------------------------------##
## This file contains all macros related to CDTMAN general functionality ##
## used to automate the creation of CDT files with binaries and datafiles##
## CDTMAN is a general macro that automates this conversion and has      ##
## several submacros for diferent tasks. All these macros are included   ##
## here.                                                                 ##
###########################################################################

#
# Default values for all CDTMAN functions
#
CDTM_MODE     := 0
CDTM_W        := 160
CDTM_PALFW    := 11 15 3 24 13 20 6 26 0 2 1 18 8 5 16 9 
CDTM_BORDERFW := 0
CDTM_FILENAME := Game
# 5 Vectors for files to be added to the CDT
CDTM_CDTFILES := 
CDTM_CDTNAMES := 
CDTM_CDTTYPES := 
CDTM_CDTLOAD  := 
CDTM_CDTRUN   := 

# Load and Run Addresses of Loader and ShowSCR binaries
# Default LoadADDR leaves 68 bytes for the stack, which is enough 
# for not conflicting with BASIC use of the stack. This value has
# been obtained from tests with different BASICS and CPC models. 
# It may occasionally fail to load (it usually will fail after
# loading the screen, because latest bytes can be lost if they
# are overwritten by the stack). It does cases, just use a 
# different address for the binary of the loader.
CDTM_SHOWSCR_LOADADDR :=0x4000
CDTM_LOADER_LOADADDR  :=0xBF2A
# Sizes of both the Loader and the ShowSCR binaries (alone)
CDTM_SHOWSCR_SIZE     :=157
CDTM_LOADER_SIZE      :=146

# Addresses used to patch calls to MINILOAD. There are 3 different 
# Calls to Miniload:
#   * MINILOAD_RUN: The main entrance point, which is called from outside
#   * MINILOAD_FULL: Internal routine used to wait for next full pulse
#   * MINILOAD_HALF: Same internal routine, but with different entry point, for a half pulse
# These calls must be patched every time the LOADER is placed at a different point in Memory
#
# 0xBF62 = 0xBF46 + 0x1C (0x1C = OFFSET MINILOAD_RUN)
CDTM_MINILOAD_RUN     :=0xBF46
# 0xBF8C = 0xBF46 + 0x46 (0x46 = OFFSET MINILOAD_FULL)
CDTM_MINILOAD_FULL    :=0xBF70
# 0xBF8E = 0xBF46 + 0x48 (0x48 = OFFSET MINILOAD_HALF)
CDTM_MINILOAD_HALF    :=0xBF72
# Offsets relative to LOADER_LOADDADDR of the same 3 calls. These
# Are used to recalculate previous 3 calls in case of binary relocation
CDTM_OFFMINILOAD_RUN  :=0x1C
CDTM_OFFMINILOAD_FULL :=0x46
CDTM_OFFMINILOAD_HALF :=0x48

## OFFSETS FOR POINTS IN BINARY WERE PATCHES MUST BE INSERTED

# OFFSETS TO PATCH SHOWSCR BINARY
CDTM_OFFVIDEOMODE     :=0x04
CDTM_OFFPALSIZE       :=0x0C
CDTM_OFFSCREND        :=0x12
CDTM_OFFPAL           :=0x8C

# OFFSETS TO PATCH LOADER BINARY
CDTM_OFFSCRLOAD       :=0x02
CDTM_OFFSCRSIZE       :=0x05
CDTM_OFFSCRRUN        :=0x0C
CDTM_OFFGAMELOAD      :=0x10
CDTM_OFFGAMESIZE      :=0x13
CDTM_OFFGAMERUN       :=0x1A
# Calls to miniload
CDTM_OFFMINILOAD_CALL1:=0x08
CDTM_OFFMINILOAD_CALL2:=0x16
CDTM_OFFMINFULL_CALL1 :=0x2D
CDTM_OFFMINFULL_CALL2 :=0x3A
CDTM_OFFMINHALF_CALL1 :=0x72
CDTM_OFFMINHALF_CALL2 :=0x75

#################
# ADD_FILE_TO_CDT_LIST: Adds a file to the list of files
# that will be included in the final CDT to be produced
# at the end of the building process. 
#
# $(1): File
# $(2): Type {firmware, miniload}
# $(3): Load Address (for firmware calls)
# $(4): Run Address (for firmware calls)
# $(5): Filename (for firmware calls)
#
define ADD_FILE_TO_CDT_LIST
	# Parameters cannot be empty, so transform them if they are
	$(eval _L := $(if $(3),$(3),noload))
	$(eval _R := $(if $(4),$(4),norun))
	$(eval _N := $(if $(5),$(5),noname))
	# And now add the file to the list
	$(eval CDTM_CDTFILES := $(CDTM_CDTFILES) $(strip $(1)))
	$(eval CDTM_CDTTYPES := $(CDTM_CDTTYPES) $(strip $(2)))
	$(eval CDTM_CDTLOAD  := $(CDTM_CDTLOAD)  $(strip $(_L)))
	$(eval CDTM_CDTRUN   := $(CDTM_CDTRUN)   $(strip $(_R)))
	$(eval CDTM_CDTNAMES := $(CDTM_CDTNAMES) $(subst $(SPACE),',$(strip $(_N))))
endef

#################
# INSERT_NEXT_FILE_INTO_CDT: Adds a new file to the CDT file using
# CPC2CDT and taking into account file codification (firmware or miniload)
# to insert it RAW in raw1full format or adding AMSDOS header
#
# $(1): File
# $(2): Type {firmware, miniload}
# $(3): Load Address (for firmware calls)
# $(4): Run Address (for firmware calls)
# $(5): Filename (for firmware calls)
#
define INSERT_NEXT_FILE_INTO_CDT
	$(info Inserting file: '$(1)' '$(2)' '$(3)' '$(4)' '$(5)')
	$(if $(filter firmware,$(2))\
		,$(CPC2CDT) -x "$(4)" -l "$(3)" -t -b 2000 -r "$(5)" "$(1)" "$(CDT)" \
		,$(CPC2CDT) -m raw1full -rl 740 "$(1)" "$(CDT)" \
		)  
endef

#################
# CREATECDT: Creates the final CDT file with the files 
# that have been added by the configuration macros.
#
define CREATECDT
	# If CDT file already exists, remove it first to create a new one
	$(call REMOVEFILEIFEXISTS,$(CDT))

	# If no file is to be added to the CDT, just add the generated as a firmware file
	$(if $(CDTM_CDTFILES)\
		,\
		,$(call GETALLADDRESSES,$(BINFILE)); \
		 $(call INSERT_NEXT_FILE_INTO_CDT,$(BINFILE),firmware,0x$(strip $(LOADADDR)),0x$(strip $(RUNADDR)),$(CDTM_FILENAME))
		)
	# Add all the files to the CDT one by one
	$(eval _I:=1)
	$(foreach _F,$(CDTM_CDTFILES),\
		$(eval _T := $(word $(_I), $(CDTM_CDTTYPES))) \
		$(eval _N := $(subst ',$(SPACE),$(word $(_I), $(CDTM_CDTNAMES)))) \
		$(eval _L := $(word $(_I), $(CDTM_CDTLOAD))) \
		$(eval _R := $(word $(_I), $(CDTM_CDTRUN))) \
		$(call INSERT_NEXT_FILE_INTO_CDT,$(_F),$(_T),$(_L),$(_R),$(_N))
		$(call ADD2INTS,$(_I),1,_I) \
		)
	$(info CreateCDT ended!)
endef

#################
# CDTMAN_SET_MINILOAD_MODE: Sets the CPC video mode that will be used
# for the image that will be loaded by the miniloader. Only standard
# video modes are valid: 0 1 2
#
# $(1): Amstrad CPC Video Mode
#
# Updates variable CDTM_MODE, CDTM_W
#
define CDTMAN_SET_MINILOAD_MODE
	# Check that the passed value is valid and assign it 
	$(eval CDTM_VALID:=0 1 2)
	$(eval CDTM_WIDTHS:=160 320 640)
	$(call SET_ONE_OF_MANY_VALID,CDTM_MODE,$(1),$(CDTM_VALID),[CDTMAN - SET_MINILOAD_MODE]: '$(1)' is not a valid video mode)
	$(call CONVERTVALUE,$(CDTM_MODE),CDTM_VALID,CDTM_WIDTHS,160,CDTM_W)
endef

#################
# CDTMAN_SET_MINILOAD_FILENAME: Sets the filename that will be used
# for the loader when loading it from BASIC. This filename will act
# as your game's filename. Maximum: 16 characters
#
# $(1): Filename
#
# Updates variable CDTM_FILENAME
#
define CDTMAN_SET_MINILOAD_FILENAME
	# Remove any single quotes from filename
	$(eval CDTM_FILENAME := $(subst ',,$(1)))

	# Ensure Length of Filename is not greater than 16
	$(eval CDTM_FLEN := $(call STRLEN,$(CDTM_FILENAME)))
	$(if $(call GREATER_THAN,$(CDTM_FLEN),16)\
		,$(error <<ERROR>> [CDTMAN - SET_MINILOAD_FILENAME]: Filename '$(CDTM_FILENAME)' is longer than the maximum cassette filename length: '$(CDTM_FLEN)'>16)\
		,)
endef

#################
# CDTMAN_GEN_MINILOADER: 
#
# $(1): Screen file
#
# Updates PREBUILDOBJS, CDTMANOBJS
#
define CDTMAN_GEN_MINILOADER
	# Check file exists

	# Prepare palette firmware for IMG2TILESET
	$(if $(filter 0,$(CDTM_MODE))\
		,$(eval CDTM_PALFWMODE := $(wordlist 1,16,$(CDTM_PALFW)))\
		,\
		)
	$(if $(filter 1,$(CDTM_MODE))\
		,$(eval CDTM_PALFWMODE := $(wordlist 1,4,$(CDTM_PALFW)))\
		,\
		)
	$(if $(filter 2,$(CDTM_MODE))\
		,$(eval CDTM_PALFWMODE := $(wordlist 1,2,$(CDTM_PALFW)))\
		,\
		)

	# Prepare palette hardware for patcher
	$(call CONVERT_FW2HW_PALETTE,$(CDTM_PALFW) $(CDTM_BORDERFW),_TMP)
	$(eval CDTM_PALHW := )
	$(foreach _C,$(_TMP),$(eval CDTM_PALHW := $(CDTM_PALHW) 0x$(_C)))

	# Prepare file names and paths
	$(eval CDTM_SUFIX:=$(suffix $(1)))
	$(eval CDTM_OBJDIR:=$(OBJDIR)/$(dir $(1)))
	$(call JOINFOLDER2BASENAME,CDTM_IMGF,$(CDTM_OBJDIR),$(1))
	$(call JOINFOLDER2BASENAME,CDTM_IMGBIN,$(CDTM_OBJDIR),$(1:$(CDTM_SUFIX)=.bin))
	$(call JOINFOLDER2BASENAME,CDTM_IMGZX7B,$(CDTM_OBJDIR),$(1:$(CDTM_SUFIX)=.zx7b.bin))
	$(call JOINFOLDER2BASENAME,CDTM_SHOWSCR,$(CDTM_OBJDIR),$(1:$(CDTM_SUFIX)=.showscr.bin))
	$(call JOINFOLDER2BASENAME,CDTM_SHOWSCRPCH,$(CDTM_OBJDIR),$(1:$(CDTM_SUFIX)=.patched.showscr.bin))
	$(call JOINFOLDER2BASENAME,CDTM_LOADERBIN,$(CDTM_OBJDIR),$(ML_LOADER_BIN))
	$(call JOINFOLDER2BASENAME,CDTM_LOADERPCH1,$(CDTM_OBJDIR),$(ML_LOADER_BIN:.bin=.patch1.bin))
	$(call JOINFOLDER2BASENAME,CDTM_LOADERPCH2,$(CDTM_OBJDIR),$(ML_LOADER_BIN:.bin=.patch2.bin))

	## Add files to be generated to the list of CDT Files
	$(call ADD_FILE_TO_CDT_LIST,$(CDTM_LOADERPCH2),firmware,$(CDTM_LOADER_LOADADDR),$(CDTM_LOADER_LOADADDR),$(CDTM_FILENAME))
	$(call ADD_FILE_TO_CDT_LIST,$(CDTM_SHOWSCRPCH),miniload)
	$(call ADD_FILE_TO_CDT_LIST,$(BINFILE),miniload)

## Generate pre-build event rules to convert loading screen

## Rule to generate and patch Showscreen binary
$(CDTM_SHOWSCRPCH): $(1)
	## Prepare object dir and move file
	$(MKDIR) $(CDTM_OBJDIR)
	$(CP) "$(1)" "$(CDTM_IMGF)"
	## Convert Screen
	$(IMG2TIL) -nt -of bin -pf { $(CDTM_PALFWMODE) } -m $(CDTM_MODE) -scr -tw $(CDTM_W) -th 200 "$(CDTM_IMGF)"
	## Pack it into zx7b
	$(ZX7B) $(CDTM_IMGBIN) $(CDTM_IMGZX7B)
	## Add it to showscr
	$(CAT) $(ML_SHOWSCR_BIN) $(CDTM_IMGZX7B) > $(CDTM_SHOWSCR)
	# Calculate Screen End and patch file
	IMGSIZE=`wc -c < $(CDTM_SHOWSCR)` && \
	SCREND=$$$$(( $(CDTM_SHOWSCR_LOADADDR) + IMGSIZE - 1 )) && \
	$(BINPATCH) $(CDTM_SHOWSCR) \
	   -pb "$(CDTM_OFFVIDEOMODE)" "$(CDTM_MODE)" \
	   -pw "$(CDTM_OFFSCREND)" "$$$${SCREND}" \
	   -ps "$(CDTM_OFFPAL)" $(CDTM_PALHW)
	# Now copy it to generate the showscr patched file
	$(CP) $(CDTM_SHOWSCR) $(CDTM_SHOWSCRPCH)

## Rule to generate and partially patch loader binary (It cannot be completely patched
## because we still don't know Game Size, Load and Run Addresses)
$(CDTM_LOADERPCH1): $(CDTM_SHOWSCRPCH)
	## Copy Default loader
	$(CP) $(ML_LOADER_BIN) $(CDTM_LOADERBIN)
	## Patch it with ShowSCR Size, Load and Run Addresses, and loader insider calls to miniload
	SCRSIZE=`wc -c < $(CDTM_SHOWSCR)` && \
	$(BINPATCH) "$(CDTM_LOADERBIN)" \
	   -pw "$(CDTM_OFFSCRLOAD)" "$(CDTM_SHOWSCR_LOADADDR)" \
	   -pw "$(CDTM_OFFSCRRUN)" "$(CDTM_SHOWSCR_LOADADDR)" \
		-pw "$(CDTM_OFFSCRSIZE)" "$$$${SCRSIZE}" \
		-pw "$(CDTM_OFFMINILOAD_CALL1)" "$(CDTM_MINILOAD_RUN)" \
		-pw "$(CDTM_OFFMINILOAD_CALL2)" "$(CDTM_MINILOAD_RUN)" \
		-pw "$(CDTM_OFFMINHALF_CALL1)" "$(CDTM_MINILOAD_HALF)" \
		-pw "$(CDTM_OFFMINHALF_CALL2)" "$(CDTM_MINILOAD_HALF)" \
		-pw "$(CDTM_OFFMINFULL_CALL1)" "$(CDTM_MINILOAD_FULL)" \
		-pw "$(CDTM_OFFMINFULL_CALL2)" "$(CDTM_MINILOAD_FULL)"
	## Once successfully patched, create the new patched file
	$(CP) $(CDTM_LOADERBIN) $(CDTM_LOADERPCH1)

## Rule to do the final patching of Loader binary, once GAME SIZE, LOADADDR and RUNADDR are known
$(CDTM_LOADERPCH2): $(CDTM_LOADERPCH1) $(BINFILE)
	## Get LOADADDR and RUNADDR from generated binfile
	@$(call GETALLADDRESSES,$(BINFILE))
	## Final patch of loader binary
	GAMESIZE=`wc -c < $(BINFILE)` && \
	$(BINPATCH) "$(CDTM_LOADERPCH1)" \
		-pw "$(CDTM_OFFGAMELOAD)" "0x$(strip $(LOADADDR))" \
		-pw "$(CDTM_OFFGAMERUN)" "0x$(strip $(RUNADDR))" \
		-pw "$(CDTM_OFFGAMESIZE)" "$$$${GAMESIZE}"
	## Once successfully patched, create the new patched file
	$(CP) $(CDTM_LOADERPCH1) $(CDTM_LOADERPCH2)


# Update PREBUILDOBJS and CDTMANOBJS
PREBUILDOBJS := $(PREBUILDOBJS) $(CDTM_LOADERPCH1)
CDTMANOBJS   := $(CCDTMANOBJS) $(CDTM_LOADERPCH2)
endef

#################
# CDTMAN_SET_MINILOAD_PALETTE_FW: Sets the firmware palette to be used in following CDTMAN CONVERT commands
#
# $(1): Border colour as firmware value (single value, from 0 to 16)
# $(2): Firmware palette array (Array of integers from 0 to 26, max size 16)
#
# Updates variable CDTM_PALFW
#
define CDTMAN_SET_MINILOAD_PALETTE_FW
	# Ensure Border Colour is a single value and firmware palette valid
	$(call ENSURE_SINGLE_VALUE,$(1),<<ERROR>> [CDTMAN - SET_MINILOAD_PALETTE_FW]: '$(1)' is not valid as border colour. It must be a SINGLE integer between 0 and 26)
	$(call VERIFY_FW_PALETTE,$(1),CDTMAN - SET_MINILOAD_PALETTE_FW)
	$(eval CDTM_BORDERFW:=$(1))
	
	# Verify passed firmware palette
	$(call VERIFY_FW_PALETTE,$(2),CDTMAN - SET_MINILOAD_PALETTE_FW)

	# All checks passed, set the palette
	$(eval CDTM_PALFW:=$(2))

	# Complete palette up to 16 values if it conatins less than that
	$(call ADD2INTS,16,-$(words $(CDTM_PALFW)),CDTM_PALSIZE)
	$(call ADD_N_ITEMS,CDTM_PALFW,$(CDTM_PALSIZE),0)

	# Add the border colour at the end of the palette
	#(eval CDTM_PALFW:=$(CDTM_PALFW) $(CDTM_BORDERFW))
endef


#################
# CDTMAN: Front-end to access all functionalities of CDTMAN macros about TMX file 
# conversion into data (arrays and/or binary files)
#
# $(1): Command to be performed
# $(2-8): Valid arguments to be passed to the selected command
#
# Valid Commands: SET_MINILOAD_MODE SET_MINILOAD_PALETTE_FW SET_MINILOAD_FILENAME GEN_MINILOADER SET_FORMAT ADDFILE_RAW
# Info about each command can be found looking into its correspondent makefile macro CDTMAN_<COMMAND>
#
define CDTMAN
	# Set the list of valid commands
	$(eval CDTMAN_F_FUNCTIONS := SET_MINILOAD_MODE SET_MINILOAD_PALETTE_FW SET_MINILOAD_FILENAME GEN_MINILOADER SET_FORMAT ADDFILE_RAW)

	# Check that command parameter ($(1)) is exactly one-word after stripping whitespaces
	$(call ENSURE_SINGLE_VALUE,$(1),<<ERROR>> [CDTMAN] '$(strip $(1))' is not a valid command. Commands must be exactly one-word in lenght with no whitespaces. Valid commands: {$(CDTMAN_F_FUNCTIONS)})

	# Filter given command as $(1) to see if it is one of the valid commands
	$(eval CDTMAN_F_SF = $(filter $(CDTMAN_F_FUNCTIONS),$(1)))

	# If the given command is valid, it will be non-empty, then we proceed to call the command (up to 8 arguments). Otherwise, raise an error
	$(if $(CDTMAN_F_SF)\
		,$(eval $(call CDTMAN_$(CDTMAN_F_SF),$(strip $(2)),$(strip $(3)),$(strip $(4)),$(strip $(5)),$(strip $(6)),$(strip $(7)),$(strip $(8))))\
		,$(error <<ERROR>> [CDTMAN] '$(strip $(1))' is not a valid command. Valid commands: {$(CDTMAN_F_FUNCTIONS)}))
endef
