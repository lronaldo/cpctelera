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
CDTM_VALIDFORMATS := firmware basic miniload generated_fw generated_ml 
CDTM_SCRFILE  := 
CDTM_OBJDIR   := $(OBJDIR)/_cdtmanager
CDTM_DEPEND   := cfg/cdt_manager.mk
CDTMANOBJS    := $(CDTMANOBJS) $(CDTM_DEPEND)
TOUCHIFNOTEXIST := $(TOUCHIFNOTEXIST) $(CDTM_DEPEND)
# 5 Vectors for files to be added to the CDT
CDTM_CDTFILES := 
CDTM_CDTNAMES := 
CDTM_CDTTYPES := 
CDTM_CDTLOAD  := 
CDTM_CDTRUN   := 
# Colours for messages
_C1 := $(COLOR_BLUE)
_C2 := $(COLOR_MAGENTA)
_C3 := $(COLOR_YELLOW)
_C  := $(COLOR_NORMAL)

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
# 0xBF46 = 0xBF2A + 0x1C (0x1C = OFFSET MINILOAD_RUN)
CDTM_MINILOAD_RUN     :=0xBF46
# 0xBF70 = 0xBF2A + 0x46 (0x46 = OFFSET MINILOAD_FULL)
CDTM_MINILOAD_FULL    :=0xBF70
# 0xBF72 = 0xBF2A + 0x48 (0x48 = OFFSET MINILOAD_HALF)
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
# $(2): Type {firmware, basic, miniload}
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
# $(2): Type {generated_fw, firmware, basic, miniload}
# $(3): Load Address (for firmware calls)
# $(4): Run Address (for firmware calls)
# $(5): Filename (for firmware calls)
#
define INSERT_NEXT_FILE_INTO_CDT
	$(if $(call CONTAINED_IN,$(2),generated_fw generated_ml)\
		,\
		, $(call ENSUREFILEEXISTS,$(1),<<ERROR>> [ CDTMAN - INSERT FILE ]: File '$(1)' does not exist or cannot be read when trying to add it to '$(CDT)') \
		  $(call ENSUREVALID,$(2),$(CDTM_VALIDFORMATS),<<ERROR>> [ CDTMAN - INSERT FILE ]: Format '$(2)' for file '$(1) is not valid. Valid formats are: { $(CDTM_VALIDFORMATS) }') \
   )

	printf "$(_C1)'$(_C2)$(CDT)$(_C1)' < '$(_C2)$(notdir $(1))$(_C1)' {Format:'$(_C2)$(2)$(_C1)' "

	$(if $(call EQUALS,generated_fw,$(2))\
		, $(call GETALLADDRESSES,$(BINFILE)) \
		  $(eval _LA:=$(if $(call EQUALS,noload,$(3)),0x$(strip $(LOADADDR)),$(3))) \
		  $(eval _RA:=$(if $(call EQUALS,norun,$(4)),0x$(strip $(RUNADDR)),$(4))) \
		  && $(CPC2CDT) -x "$(_RA)" -l "$(_LA)" -p 3000 -t -b 2000 -r "$(CDTM_FILENAME)" "$(1)" "$(CDT)" > /dev/null \
		  && printf "Load:'$(_C2)$(_LA)$(_C1)' Run:'$(_C2)$(_RA)$(_C1)' Name:'$(_C2)$(CDTM_FILENAME)$(_C1)'" \
	)
	$(if $(call EQUALS,firmware,$(2))\
		, $(CPC2CDT) -x "$(4)" -l "$(3)" -p 3000 -t -b 2000 -r "$(5)" "$(1)" "$(CDT)" > /dev/null \
		  && printf "Load:'$(_C2)$(3)$(_C1)' Run:'$(_C2)$(4)$(_C1)' Name:'$(_C2)$(5)$(_C1)'" \
	)
	$(if $(call CONTAINED_IN,$(2),miniload generated_ml)\
		, $(CPC2CDT) -m raw1full -rl 740 "$(1)" "$(CDT)" >> /dev/null \
	)
	$(if $(call EQUALS,basic,$(2))\
		, $(CPC2CDT) -p 3000 -t -b 2000 -r "$(5)" "$(1)" "$(CDT)" > /dev/null \
		  && printf "Name:'$(_C2)$(5)$(_C1)'" \
	)

	printf "}$(_C)\n"
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
		 $(call INSERT_NEXT_FILE_INTO_CDT,$(BINFILE),firmware,0x$(strip $(LOADADDR)),0x$(strip $(RUNADDR)),$(CDTM_FILENAME)) \
		)
	# Add all the files to the CDT one by one
	$(eval _I:=1)
	$(foreach _F,$(CDTM_CDTFILES),\
		$(eval _T := $(word $(_I), $(CDTM_CDTTYPES))) \
		$(eval _N := $(subst ',$(SPACE),$(word $(_I), $(CDTM_CDTNAMES)))) \
		$(eval _L := $(word $(_I), $(CDTM_CDTLOAD))) \
		$(eval _R := $(word $(_I), $(CDTM_CDTRUN))) \
		$(call INSERT_NEXT_FILE_INTO_CDT,$(_F),$(_T),$(_L),$(_R),$(_N))
		$(eval _I := $(call ADD2INTS,$(_I),1)) \
		)
endef


#################
# CDTMAN_ADDFILE: Adds a new file to the list of files 
# that will be inserted into the CDT file. The file needs
# to be a plain binary without header. It will be inserted
# either as firmware or as miniload format. miniload format
# does not require parameters. firmware format requires 
# a Load Address and a Run Address. Also, firmware files
# will use current CDTM_FILENAME as cassette filename.
#
# $(1): Format { firmware, basic, miniload, generated_fw, generated_ml }
# $(2): File to be added
# $(3): Load Address (Only for firmware formats)
# $(4): Run Address (Only for firmware formats)
#
define CDTMAN_ADDFILE
	# Check constraints
	$(call ENSUREVALID,$(1),$(CDTM_VALIDFORMATS),<<ERROR>> [ CDTMAN - ADDFILE ]: Format '$(1)' for file '$(2) is not valid. Valid formats are: { $(CDTM_VALIDFORMATS) }')
	$(if $(call EQUALS,$(1),firmware)\
		, $(call ENSURE_ADDRESS_VALID,$(3),<<ERROR>> [ CDTMAN - ADDFILE ]:) \
		  $(call ENSURE_ADDRESS_VALID,$(4),<<ERROR>> [ CDTMAN - ADDFILE ]:) \
		,)	

	# Add to list of files to be inserted in the CDT
	$(call ADD_FILE_TO_CDT_LIST,$(2),$(1),$(3),$(4),$(CDTM_FILENAME))
endef

#################
# CDTMAN_ADD_GENERATED_BIN: Adds the binary file generated
# by the compilation of the proyect into the CDT file. 
# The file is added as firmware format, and will use
# default Load Address and Run Address got from generated
# .bin.log file. It will also use current CDTM_FILENAME 
# as cassette filename.
# Load & Run Address parameters can be overriden by 
# passing parameters $(1) and $(2) to this macro. If 
# they are not null, they will be used as Load and Run
# addresses respectively, discarding those from .bin.log.
#
# $(1): [Optional] Format { firmware, miniload } ( firmware by default ) 
# $(2): [Optional] Load Address (Overrides default value got from .bin.log)
# $(3): [Optional] Run  Address (Overrides default value got from .bin.log)
#
define CDTMAN_ADD_GENERATED_BIN
	# Check constraints
	$(eval _LA:=)
	$(eval _RA:=)
	$(eval _FMT:=generated_fw)
	$(eval _ERRH:=<<ERROR>> [ CDTMAN - ADD_GENERATED_BIN ]: )
   $(if $(1)\
      , $(call ENSUREVALID,$(strip $(1)),firmware miniload,$(_ERRH)Format '$(1)' for file '$(BINFILE)' is not valid. Valid formats are: { firmware miniload }) \
        $(eval _FMT:=$(if $(call EQUALS,$(_FMT),firmware),generated_fw,generated_ml))
      ,)
	$(if $(2)\
		, $(call ENSURE_ADDRESS_VALID,$(strip $(2)),$(_ERRH)) \
        $(eval _LA := $(strip $(2))) \
		,)	
	$(if $(3)\
		, $(call ENSURE_ADDRESS_VALID,$(strip $(3)),$(_ERRH)) \
        $(eval _RA := $(strip $(3))) \
		,)	

	# Add to list of files to be inserted in the CDT
	$(call ADD_FILE_TO_CDT_LIST,$(BINFILE),$(_FMT),$(_LA),$(_RA),$(CDTM_FILENAME))
endef

#################
# CDTMAN_SET_MINILOAD_LOADER_ADDRESS: Sets the memory address where the 
# miniload loader binary will be loaded. This binary includes code 
# to load and run the compressed image file, then load and run the
# game. It also includes miniloads code. It takes 146 bytes in total
# from this loading address.
#
# $(1): Memory LOAD ADDRESS for Loader
#
define CDTMAN_SET_MINILOAD_LOADER_ADDRESS
	# Ensure that the address is valid
	$(call ENSURE_ADDRESS_VALID,$(1),<<ERROR>> [ CDTMAN - SET_MINILOAD_LOADER_ADDRESS ])

	# Now set the new address and calculate corresponding Offsets
	$(eval CDTM_MINILOAD_RUN    := $(call DEC2HEX,$(call ADD2INTS,$(_A),$(CDTM_OFFMINILOAD_RUN))))
	$(eval CDTM_MINILOAD_FULL   := $(call DEC2HEX,$(call ADD2INTS,$(_A),$(CDTM_OFFMINILOAD_FULL))))
	$(eval CDTM_MINILOAD_HALF   := $(call DEC2HEX,$(call ADD2INTS,$(_A),$(CDTM_OFFMINILOAD_HALF))))
	$(eval CDTM_LOADER_LOADADDR := $(call DEC2HEX,$(_A)))
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
# CDTMAN_SET_FILENAME: Sets the filename that next firmware added file
# will use when loading from BASIC. This filename will act as your game's 
# filename in many cases. Maximum: 16 characters
#
# $(1): Filename
#
# Updates variable CDTM_FILENAME
#
define CDTMAN_SET_FILENAME
	# Remove any single quotes from filename
	$(eval CDTM_FILENAME := $(subst ',,$(1)))

	# Ensure Length of Filename is not greater than 16
	$(eval CDTM_FLEN := $(call STRLEN,$(CDTM_FILENAME)))
	$(if $(call GREATER_THAN,$(CDTM_FLEN),16)\
		,$(error <<ERROR>> [CDTMAN - SET_FILENAME]: Filename '$(CDTM_FILENAME)' is longer than the maximum cassette filename length: '$(CDTM_FLEN)'>16)\
		,)
endef

#################
# GEN_MINILOADER_SHOWSCR_PATCHED_RULE: Generates a makefile rule for generating
# the Patched Show Screen binary. This rule depends on the original SCR file,
# copies it to obj folder, converts it to screen data, packs it into a zx7b binary,
# attaches this binary at the end of showscr.bin and finally patches resulting
# binary to set video mode and palette values and the offset where the image ends
# for the unpacker.
#
define GEN_MINILOADER_SHOWSCR_PATCHED_RULE
## Rule to generate and patch Showscreen binary
$(CDTM_SHOWSCRPCH): $(CDTM_SCRFILE) $(CDTM_DEPEND)
	@$(call PRINT,CDTMAN,"******************")
	@$(call PRINT,CDTMAN,"*** MINILOADER ***: setting up Screen Shower binary")
	@$(call PRINT,CDTMAN,"******************")
	$(CP) "$(CDTM_SCRFILE)" "$(CDTM_IMGF)"
	@$(call PRINT,CDTMAN,"Converting given image file to mode $(CDTM_MODE) screen data...")
	$(IMG2TIL) -nt -of bin -pf { $(CDTM_PALFWMODE) } -m $(CDTM_MODE) -scr -tw $(CDTM_W) -th 200 "$(CDTM_IMGF)"
	@$(call PRINT,CDTMAN,"Packing screen data with ZX7B compressor...")
	$(ZX7B) $(CDTM_IMGBIN) $(CDTM_IMGZX7B)
	@$(call PRINT,CDTMAN,"Adding screen data at the end of Screen Shower binary...")
	$(CAT) $(ML_SHOWSCR_BIN) $(CDTM_IMGZX7B) > $(CDTM_SHOWSCR)
	@$(call PRINT,CDTMAN,"Patching Screen Shower binary into final patched file ($(CDTM_SHOWSCRPCH))...")
	IMGSIZE=`wc -c < $(CDTM_SHOWSCR)` && \
	SCREND=$$$$(( $(CDTM_SHOWSCR_LOADADDR) + IMGSIZE - 1 )) && \
	$(BINPATCH) $(CDTM_SHOWSCR) \
	   -pb "$(CDTM_OFFVIDEOMODE)" "$(CDTM_MODE)" \
	   -pw "$(CDTM_OFFSCREND)" "$$$${SCREND}" \
	   -ps "$(CDTM_OFFPAL)" $(CDTM_PALHW)
	$(CP) $(CDTM_SHOWSCR) $(CDTM_SHOWSCRPCH)
	@$(call PRINT,CDTMAN,"Successfully generated $(CDTM_SHOWSCRPCH).")

endef

#################
# GEN_MINILOADER_LOADER_PATCHED1_RULE: This macro generates a new rule to set 
# up the first part of the Loader binary. This new rule copies the loader 
# binary to the object folder and then patches it inserting showscr.bin 
# load&run addresses and its size, and also patching calls to miniload.
# It cannot patch the last part until BINFILE is generated. That will 
# be done by GEN_MINILOADER_LOADER_PATCHED2_RULE
#
define GEN_MINILOADER_LOADER_PATCHED1_RULE

$(CDTM_LOADERPCH1): $(CDTM_SHOWSCRPCH) $(CDTM_DEPEND)
	@$(call PRINT,CDTMAN,"******************")
	@$(call PRINT,CDTMAN,"*** MINILOADER ***: setting up loader binary")
	@$(call PRINT,CDTMAN,"******************")
	@printf "$(_C1)> Loader binary Load-address:'$(_C2)$(CDTM_LOADER_LOADADDR)$(_C1)'\n"
	@printf "$(_C1)> Files to load and run: 1-'$(_C2)$(CDTM_SHOWSCR)$(_C1)' 2-'$(_C2)$(BINFILE)$(_C1)'\n"
	@printf "$(_C1)> $(_C2)$(notdir $(CDTM_SHOWSCR))$(_C1) Load-address: '$(_C2)$(CDTM_SHOWSCR_LOADADDR)$(_C1)'.\n"
	@SCRSIZE=`wc -c < $(CDTM_SHOWSCR)` && \
	printf  "$(_C1)> $(_C2)$(notdir $(CDTM_SHOWSCR))$(_C1) size: '$(_C2)$$$${SCRSIZE}$(_C1)' bytes.\n"
	@printf "$(_C1)> $(_C2)$(notdir $(CDTM_SHOWSCR))$(_C1) unpacks '$(_C2)$(CDTM_SCRFILE)$(_C1)' to video memory when run.\n"
	@$(call PRINT,CDTMAN,"Now patching generic loader with the loading and run addresses for showscr.bin and miniload calls.")
	@printf "$(_C1)> Copying '$(_C2)$(notdir $(ML_LOADER_BIN))$(_C1)' to object folder '$(_C2)$(dir $(CDTM_LOADERBIN))$(_C1)' \n"
	@$(CP) $(ML_LOADER_BIN) $(CDTM_LOADERBIN)
	@printf "$(_C1)> Patching '$(_C2)$(notdir $(CDTM_LOADERBIN))$(_C1)' into '$(_C2)$(notdir $(CDTM_LOADERPCH1))$(_C1)'...$(_C)\n"
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
	@printf "$(_C1)> Successfully patched. Now writing to '$(_C2)$(notdir $(CDTM_LOADERPCH1))$(_C1)'\n"
	@$(CP) $(CDTM_LOADERBIN) $(CDTM_LOADERPCH1)
	@$(call PRINT,CDTMAN,"Successfully generated '$(_C2)$(notdir $(CDTM_LOADERPCH1))$(_C3)'. It will be patched again later after generating '$(_C2)$(BINFILE)$(_C3)'.")

endef

#################
# GEN_MINILOADER_LOADER_PATCHED2_RULE: This macro generates a new rule
# that will be launched when generating the CDT file, after having
# generated the BINFILE. This rule will patch the binary to generate
# a final loader binary with Load&Run address and size of the BINFILE.
#
define GEN_MINILOADER_LOADER_PATCHED2_RULE

$(CDTM_LOADERPCH2): $(BINFILE) $(CDTM_LOADERPCH1) $(CDTM_DEPEND)
	@$(call PRINT,CDTMAN,"******************")
	@$(call PRINT,CDTMAN,"*** MINILOADER ***: Final patches to loader binary")
	@$(call PRINT,CDTMAN,"******************")
	@$$(call GETALLADDRESSES,$(BINFILE))
	@printf "$(_C1)> Inserting '$(_C2)$(BINFILE)$(_C1)' load&run addresses and size.\n"
	@printf "$(_C1)> '$(_C2)$(BINFILE)$(_C1)' Load Address: '$(_C2)0x$$(strip $$(LOADADDR))$(_C1)'\n"
	@printf "$(_C1)> '$(_C2)$(BINFILE)$(_C1)' Run  Address: '$(_C2)0x$$(strip $$(RUNADDR))$(_C1)'\n"
	@GAMESIZE=`wc -c < $(BINFILE)` \
	&& printf  "$(_C1)> '$(_C2)$(BINFILE)$(_C1)' Binary size : '$(_C2)0x%x$(_C1)' ($(_C2)%d$(_C1) bytes)\n" "$$$${GAMESIZE}" "$$$${GAMESIZE}" \
	&& ENDADDR=$$$$(( 0x$$(LOADADDR) + GAMESIZE - 1 )) \
	&& printf  "$(_C1)> '$(_C2)$(BINFILE)$(_C1)' End Address : '$(_C2)0x%x$(_C1)'\n" "$$$${ENDADDR}"
	@printf "$(_C1)> $(_C2)Loader$(_C1) Load Address: '$(_C2)$(CDTM_LOADER_LOADADDR)$(_C1)'\n"
	@ENDADDR=$$$$(( $(CDTM_LOADER_LOADADDR) + $(CDTM_LOADER_SIZE) - 1 )) \
	&& printf "$(_C1)> $(_C2)Loader$(_C1) End  Address: '$(_C2)0x%x$(_C1)'\n" "$$$${ENDADDR}"
	@printf "$(_C1)> Patching '$(_C2)$(notdir $(CDTM_LOADERPCH1))$(_C1)' into '$(_C2)$(notdir $(CDTM_LOADERPCH2))$(_C1)...$(_C)'\n"
	GAMESIZE=`wc -c < $(BINFILE)` && \
	$(BINPATCH) "$(CDTM_LOADERPCH1)" \
		-pw "$(CDTM_OFFGAMELOAD)" "0x$$(strip $$(LOADADDR))" \
		-pw "$(CDTM_OFFGAMERUN)" "0x$$(strip $$(RUNADDR))" \
		-pw "$(CDTM_OFFGAMESIZE)" "$$$${GAMESIZE}"
	@printf "$(_C1)> Successfully patched. Now writing to '$(_C2)$(notdir $(CDTM_LOADERPCH2))$(_C)'\n"
	@$(CP) $(CDTM_LOADERPCH1) $(CDTM_LOADERPCH2)
	@$(call PRINT,CDTMAN,"Successfully generated '$(_C2)$(notdir $(CDTM_LOADERPCH2))$(_C3)'.")	

endef

#################
# CDTMAN_GEN_MINILOADER: Generates the rules and commands required to 
# automatically generate a loader based on miniload and zx7b. This 
# basic loader is composed of 3 parts: 1) the loader itself that is 
# added as an AMSDOS file, 2) a binary including a zx7b compressed
# image that gets decompressed into video memory directly and 3) the
# binary generated with CPCtelera. Both files 2) and 3) are added
# as miniload files and loaded by 1).
#
# $(1): Screen file with exact resolution for selected mode (PNG, JPG...)
#
# Updates PREBUILDOBJS, CDTMANOBJS
#
define CDTMAN_GEN_MINILOADER
	# Check file exists
	$(if $(call FILEEXISTS,$(1)),,$(error <<ERROR>> [CDTMAN - GEN_MINILOADER] File '$(1)' does not exist and is required to generate the miniloader))
	$(eval CDTM_SCRFILE := $(1))

	# Prepare firmware palette and resolution width for IMG2TILESET
	$(if $(call EQUALS,0,$(CDTM_MODE))\
		,$(eval CDTM_PALFWMODE := $(wordlist 1,16,$(CDTM_PALFW)))\
		 $(eval CDTM_W := 160) \
		,)
	$(if $(call EQUALS,1,$(CDTM_MODE))\
		,$(eval CDTM_PALFWMODE := $(wordlist 1,4,$(CDTM_PALFW)))\
		 $(eval CDTM_W := 320) \
		,)
	$(if $(call EQUALS,2,$(CDTM_MODE))\
		,$(eval CDTM_PALFWMODE := $(wordlist 1,2,$(CDTM_PALFW)))\
		 $(eval CDTM_W := 640) \
		,)

	# Check file is an image and has the appropriate size constraints
	$(eval _S := $(call GET_IMG_SIZE,$(1),<<ERROR>> [CDTMAN - GEN_MINILOADER] '$(1)' is not a valid image file.))
	$(eval _W := $(word 1,$(_S)))
	$(eval _H := $(word 2,$(_S)))	
	$(eval _R := <<ERROR>> [CDTMAN - GEN_MINILOADER] '$(1)' Incorrect image resolution '$(_W)x$(_H)'. Mode '$(CDTM_MODE)' requires a screen of '$(CDTM_W)x200' pixels)
	$(if $(call EQUALS,$(_W),$(CDTM_W)),,$(error $(_R)))
	$(if $(call EQUALS,$(_H),200),,$(error $(_R)))

	# Prepare palette hardware for patcher
	$(call CONVERT_FW2HW_PALETTE,$(CDTM_PALFW) $(CDTM_BORDERFW),_TMP)
	$(eval CDTM_PALHW := )
	$(foreach _C,$(_TMP),$(eval CDTM_PALHW := $(CDTM_PALHW) 0x$(_C)))

	# Prepare file names and paths
	$(eval CDTM_SUFIX:=$(suffix $(1)))
	$(eval CDTM_OBJDIR:=$(CDTM_OBJDIR)/$(dir $(1)))
	$(call JOINFOLDER2BASENAME,CDTM_IMGF,$(CDTM_OBJDIR),$(1))
	$(call JOINFOLDER2BASENAME,CDTM_IMGBIN,$(CDTM_OBJDIR),$(1:$(CDTM_SUFIX)=.bin))
	$(call JOINFOLDER2BASENAME,CDTM_IMGZX7B,$(CDTM_OBJDIR),$(1:$(CDTM_SUFIX)=.zx7b.bin))
	$(call JOINFOLDER2BASENAME,CDTM_SHOWSCR,$(CDTM_OBJDIR),$(1:$(CDTM_SUFIX)=.showscr.bin))
	$(call JOINFOLDER2BASENAME,CDTM_SHOWSCRPCH,$(CDTM_OBJDIR),$(1:$(CDTM_SUFIX)=.patched.showscr.bin))
	$(call JOINFOLDER2BASENAME,CDTM_LOADERBIN,$(CDTM_OBJDIR),$(ML_LOADER_BIN))
	$(call JOINFOLDER2BASENAME,CDTM_LOADERPCH1,$(CDTM_OBJDIR),$(ML_LOADER_BIN:.bin=.patch1.bin))
	$(call JOINFOLDER2BASENAME,CDTM_LOADERPCH2,$(CDTM_OBJDIR),$(ML_LOADER_BIN:.bin=.patch2.bin))

	## Create the object directory
	$(shell $(MKDIR) $(CDTM_OBJDIR))

	## Generate pre-build event rules to convert loading screen
	$(call GEN_MINILOADER_SHOWSCR_PATCHED_RULE)
	$(call GEN_MINILOADER_LOADER_PATCHED1_RULE)
	$(call GEN_MINILOADER_LOADER_PATCHED2_RULE)

	@## Add files to be generated to the list of CDT Files
	@$(call ADD_FILE_TO_CDT_LIST,$(CDTM_LOADERPCH2),firmware,$(CDTM_LOADER_LOADADDR),$(CDTM_LOADER_LOADADDR),$(CDTM_FILENAME))
	@$(call ADD_FILE_TO_CDT_LIST,$(CDTM_SHOWSCRPCH),miniload)
	@$(call ADD_FILE_TO_CDT_LIST,$(BINFILE),miniload)

# Update PREBUILDOBJS and CDTMANOBJS
PREBUILDOBJS := $(PREBUILDOBJS) $(CDTM_LOADERPCH1)
CDTMANOBJS   := $(CDTMANOBJS) $(CDTM_LOADERPCH2)
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
	$(eval CDTM_PALSIZE := $(call ADD2INTS,16,-$(words $(CDTM_PALFW))))
	$(call ADD_N_ITEMS,CDTM_PALFW,$(CDTM_PALSIZE),0)

	# Add the border colour at the end of the palette
	#(eval CDTM_PALFW:=$(CDTM_PALFW) $(CDTM_BORDERFW))
endef


#################
# CDTMAN: Front-end to access all functionalities of CDTMAN macros about CDT 
# generation and file management.
#
# $(1): Command to be performed
# $(2-8): Valid arguments to be passed to the selected command
#
# Valid Commands: ADDFILE ADD_GENERATED_BIN SET_MINILOAD_MODE SET_MINILOAD_PALETTE_FW SET_MINILOAD_LOADER_ADDRESS SET_FILENAME GEN_MINILOADER SET_FORMAT ADDFILE_RAW 
# Info about each command can be found looking into its correspondent makefile macro CDTMAN_<COMMAND>
#
define CDTMAN
	# Set the list of valid commands
	$(eval CDTMAN_F_FUNCTIONS := ADDFILE ADD_GENERATED_BIN SET_MINILOAD_MODE SET_MINILOAD_PALETTE_FW SET_MINILOAD_LOADER_ADDRESS SET_FILENAME GEN_MINILOADER SET_FORMAT ADDFILE_RAW)

	# Check that command parameter ($(1)) is exactly one-word after stripping whitespaces
	$(call ENSURE_SINGLE_VALUE,$(1),<<ERROR>> [CDTMAN] '$(strip $(1))' is not a valid command. Commands must be exactly one-word in lenght with no whitespaces. Valid commands: {$(CDTMAN_F_FUNCTIONS)})

	# Filter given command as $(1) to see if it is one of the valid commands
	$(eval CDTMAN_F_SF = $(filter $(CDTMAN_F_FUNCTIONS),$(1)))

	# If the given command is valid, it will be non-empty, then we proceed to call the command (up to 8 arguments). Otherwise, raise an error
	$(if $(CDTMAN_F_SF)\
		,$(eval $(call CDTMAN_$(CDTMAN_F_SF),$(strip $(2)),$(strip $(3)),$(strip $(4)),$(strip $(5)),$(strip $(6)),$(strip $(7)),$(strip $(8))))\
		,$(error <<ERROR>> [CDTMAN] '$(strip $(1))' is not a valid command. Valid commands: {$(CDTMAN_F_FUNCTIONS)}))
endef

#################################################################################################################################################
### OLD MACROS (Deprecated)
### Maintained here for compatibility
#################################################################################################################################################

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
