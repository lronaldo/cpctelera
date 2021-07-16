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
##                          CPCTELERA ENGINE                             ##
##                  Main Building Makefile for Projects                  ##
##-----------------------------------------------------------------------##
## This file contains the rules for building a CPCTelera project. These  ##
## These rules work generically for every CPCTelera project.             ##
## Usually, this file should be left unchanged:                          ##
##  * Project's build configuration is to be found in build_config.mk    ##
##  * Global paths and tool configuration is located at $(CPCT_PATH)/cfg/##
###########################################################################
# Logfile where load and run addresses for the generated binary will be logged
BINADDRLOG   := $(OBJDIR)/binaryAddresses.log
PREBUILD_OBJ := $(OBJDIR)/prebuildstep.objectfile

.PHONY: all clean cleanall cleancode recode

# OBJSUBDIR folder files
OBJSUBDIRS_FOLDER_FILES := $(foreach F,$(OBJSUBDIRS),$(F)/.folder)

# MAIN TARGET
.DEFAULT_GOAL := all
all: $(TARGET)

## COMPILING SOURCEFILES AND SAVE OBJFILES IN THEIR CORRESPONDENT SUBDIRS
$(foreach OF, $(BIN_OBJFILES), $(eval $(call BINFILE2C, $(OF), $(OF:%.$(C_EXT)=%.$(BIN_EXT)))))
$(foreach OF, $(GENC_OBJFILES), $(eval $(call COMPILECFILE, $(OF), $(patsubst $(OBJDIR)%,$(SRCDIR)%,$(OF:%.$(OBJ_EXT)=%.$(C_EXT))))))
$(foreach OF, $(GENASM_OBJFILES), $(eval $(call COMPILEASMFILE, $(OF), $(patsubst $(OBJDIR)%,$(SRCDIR)%,$(OF:%.$(OBJ_EXT)=%.$(ASM_EXT))))))
$(foreach OF, $(C_OBJFILES), $(eval $(call COMPILECFILE, $(OF), $(patsubst $(OBJDIR)%,$(SRCDIR)%,$(OF:%.$(OBJ_EXT)=%.$(C_EXT))))))
$(foreach OF, $(ASM_OBJFILES), $(eval $(call COMPILEASMFILE, $(OF), $(patsubst $(OBJDIR)%,$(SRCDIR)%,$(OF:%.$(OBJ_EXT)=%.$(ASM_EXT))))))
## Generate an Add-BIN-to-DSK rule for each Binary file in DSKFILESDIR
$(foreach SF, $(DSKINCSRCFILES), $(eval $(call ADDFILETODSK,$(DSK),$(SF),$(patsubst $(DSKFILESDIR)/%, $(OBJDSKINCSDIR)/%, $(SF)).$(DSKINC_EXT))))

# Files to be created if they do not exist (for compatibility)
$(TOUCHIFNOTEXIST):
	@$(TOUCH) $(TOUCHIFNOTEXIST)

# PREVIOUS BUILDING STEP (CONVERSION TOOLS NORMALLY)
$(PREBUILD_OBJ): $(OBJSUBDIRS_FOLDER_FILES) $(IMGCFILES) $(IMGASMFILES) $(IMGBINFILES) $(PREBUILDOBJS)
	$(info preobjs: '$(OBJSUBDIRS_FOLDER_FILES)')
	@$(call PRINT,$(PROJNAME),"")
	@$(call PRINT,$(PROJNAME),"=== PREBUILD PROCCESSING DONE!")
	@$(call PRINT,$(PROJNAME),"============================================================")
	@$(call PRINT,$(PROJNAME),"")
	@touch $(PREBUILD_OBJ)

# LINK RELOCATABLE MACHINE CODE FILES (.REL) INTO A INTEL HEX BINARY (.IHX)
$(IHXFILE): $(PREBUILD_OBJ) $(NONLINKGENFILES) $(GENOBJFILES) $(OBJFILES)
	@$(call PRINT,$(PROJNAME),"Linking binary file")
	$(Z80CC) $(Z80CCLINKARGS) $(GENOBJFILES) $(OBJFILES) -o "$@"

# GENERATE BINARY FILE (.BIN) FROM INTEL HEX BINARY (.IHX)
$(BINFILE): $(IHXFILE)
	@$(call PRINT,$(PROJNAME),"Creating Amsdos binary file $@")
	$(HEX2BIN) -p 00 "$<" | $(TEE) $@.log

# CREATE BINARY AND GET LOAD AND RUN ADDRESS FROM LOGFILE AND MAPFILE
$(BINADDRLOG): $(BINFILE)
	@$(call GETALLADDRESSES,$<)
	@echo "Generated Binary File $(BINFILE):" > $(BINADDRLOG)
	@echo "Load Address = $(LOADADDR)"       >> $(BINADDRLOG)
	@echo "Run  Address = $(RUNADDR)"        >> $(BINADDRLOG)

# GENERATE A DISK FILE (.DSK) AND INCLUDE BINARY FILE (.BIN) INTO IT
$(DSK): $(BINFILE) $(BINADDRLOG)
	@$(call GETALLADDRESSES,$<)
	@$(call PRINT,$(PROJNAME),"Creating Disk File '$@'")
	@$(call CREATEEMPTYDSK,$@)
	@$(RM) $(DSKINCOBJFILES)	
	@$(call ADDCODEFILETODSK,$@,$<,$(LOADADDR),$(RUNADDR),$(<:%=%.$(DSKINC_EXT)))
	@$(call PRINT,$(PROJNAME),"Successfully created '$@'")

# GENERATE A CASSETTE FILE (.CDT) AND INCLUDE BINARY FILE (.BIN) INTO IT
$(CDT): $(BINFILE) $(BINADDRLOG) $(CDTMANOBJS)
	@$(call PRINT,$(PROJNAME),"Creating Cassette file '$@'")
	@$(call CREATECDT,)
	@$(call PRINT,$(PROJNAME),"Successfully created '$@'")

# GENERATE A SNAPSHOP FILE (.SNA) AND INCLUDE BINARY FILE (.BIN) INTO IT
$(SNA): $(BINFILE) $(BINADDRLOG)
	@$(call GETALLADDRESSES,$<)
	@$(call PRINT,$(PROJNAME),"Creating Snapshot File '$@'")
	@$(call CREATESNA,$<,$@,$(LOADADDR),$(RUNADDR))
	@$(call PRINT,$(PROJNAME),"Successfully created '$@'")

## Include files in DSKFILESDIR to DSK, print a message and generate a flag file DSKINC
$(DSKINC): $(DSK) $(DSKINCOBJFILES)
	@$(call PRINT,$(PROJNAME),"All files added to $(DSK). Disc ready.")
	@touch $(DSKINC)

# CREATE OBJDIR & SUBDIRS IF THEY DO NOT EXIST
define MKSUBDIR
$(eval __S:=$(strip $(1)))
$(eval __D:=$(__S:%.folder=%))
$(__S):
	@$(MKDIR) $(__D)
	@touch $(__S)

endef
$(foreach D,$(OBJSUBDIRS_FOLDER_FILES),$(eval $(call MKSUBDIR,$(D))))


## CLEANING TARGETS
#

# Clean Everything, including target DSK, SNA and CDT
cleanall: clean
	@$(call PRINT,$(PROJNAME),"Deleting $(TARGET)")
	$(RM) $(TARGET)

# Clean only code-produced objects (not assets or generated files)
cleancode:
	@$(call PRINT,$(PROJNAME),"Deleting C-OBJ   Relfiles")
	$(foreach file, $(C_OBJFILES), $(RM) ./$(file))
	@$(call PRINT,$(PROJNAME),"Deleting ASM-OBJ Relfiles")
	$(foreach file, $(ASM_OBJFILES), $(RM) ./$(file))
	@$(call PRINT,$(PROJNAME),"Code cleaned")

# Clean only code an remake the project
recode: cleancode
	$(MAKE) 

# Clean all object files, but not targets (DSK, SNA, CDT)
clean: 
	@$(call PRINT,$(PROJNAME),"Deleting folder: $(OBJDIR)/")
	$(RM) -r ./$(OBJDIR)
	@$(call PRINT,$(PROJNAME),"Deleting objects to clean: $(OBJS2CLEAN)")
	$(foreach elem, $(OBJS2CLEAN), $(RM) -r ./$(elem))
