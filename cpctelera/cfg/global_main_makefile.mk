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
.PHONY: all clean cleanall

# Logfile where load and run addresses for the generated binary will be logged
BINADDRLOG=$(OBJDIR)/binaryAddresses.log

# MAIN TARGET
.DEFAULT_GOAL := all
all: $(OBJSUBDIRS) $(TARGET)

## COMPILING SOURCEFILES AND SAVE OBJFILES IN THEIR CORRESPONDENT SUBDIRS
$(foreach OF, $(BIN_OBJFILES), $(eval $(call BINFILE2C, $(OF), $(OF:%.$(C_EXT)=%.$(BIN_EXT)))))
$(foreach OF, $(C_OBJFILES), $(eval $(call COMPILECFILE, $(OF), $(patsubst $(OBJDIR)%,$(SRCDIR)%,$(OF:%.$(OBJ_EXT)=%.$(C_EXT))))))
$(foreach OF, $(ASM_OBJFILES), $(eval $(call COMPILEASMFILE, $(OF), $(patsubst $(OBJDIR)%,$(SRCDIR)%,$(OF:%.$(OBJ_EXT)=%.$(ASM_EXT))))))
## Generate an Add-BIN-to-DSK rule for each Binary file in DSKFILESDIR
$(foreach SF, $(DSKINCSRCFILES), $(eval $(call ADDBINFILETODSK, $(DSK), $(SF), $(patsubst $(DSKFILESDIR)/%, $(OBJDSKINCSDIR)/%, $(SF)).$(DSKINC_EXT))))

# LINK RELOCATABLE MACHINE CODE FILES (.REL) INTO A INTEL HEX BINARY (.IHX)
$(IHXFILE): $(OBJFILES) 
	@$(call PRINT,$(PROJNAME),"Linking binary file")
	$(Z80CC) $(Z80CCLINKARGS) $^ -o "$@"

# GENERATE BINARY FILE (.BIN) FROM INTEL HEX BINARY (.IHX)
%.bin: %.ihx
	@$(call PRINT,$(PROJNAME),"Creating Amsdos binary file $@")
	$(HEX2BIN) -p 00 "$<" | $(TEE) $@.log

# CREATE BINARY AND GET LOAD AND RUN ADDRESS FROM LOGFILE AND MAPFILE
$(BINADDRLOG): $(BINFILE)
	@$(call GETLOADADDRESS,LOADADDR,$(<).log)
	@$(call GETRUNADDRESS,RUNADDR,$(<:.bin=.map))
	@$(call CHECKVARIABLEISSET,LOADADDR)
	@$(call CHECKVARIABLEISSET,RUNADDR)
	@echo "Generated Binary File $(BINFILE):" > $(BINADDRLOG)
	@echo "Load Address = $(LOADADDR)"       >> $(BINADDRLOG)
	@echo "Run  Address = $(RUNADDR)"        >> $(BINADDRLOG)

# GENERATE A DISK FILE (.DSK) AND INCLUDE BINARY FILE (.BIN) INTO IT
%.dsk: $(BINFILE) $(BINADDRLOG)
	@$(call PRINT,$(PROJNAME),"Creating Disk File $@")
	@$(call CREATEEMPTYDSK,$@)
	@$(call ADDCODEFILETODSK,$@,$<,$(LOADADDR),$(RUNADDR),$(<:%=%.$(DSKINC_EXT)))
	@$(call PRINT,$(PROJNAME),"Successfully created $@")

# GENERATE A CASSETTE FILE (.CDT) AND INCLUDE BINARY FILE (.BIN) INTO IT
%.cdt: $(BINFILE) $(BINADDRLOG)
	@$(call PRINT,$(PROJNAME),"Creating Cassette File $@")
	@$(call CREATECDT,$<,$(notdir $<),$@,$(LOADADDR),$(RUNADDR))
	@$(call PRINT,$(PROJNAME),"Successfully created $@")

## Include files in DSKFILESDIR to DSK, print a message and generate a flag file DSKINC
$(DSKINC): $(DSK) $(DSKINCOBJFILES)
	@$(call PRINT,$(PROJNAME),"All files added to $(DSK). Disc ready.")
	@touch $(DSKINC)

# CREATE OBJDIR & SUBDIRS IF THEY DO NOT EXIST
$(OBJSUBDIRS): 
	@$(MKDIR) $@

# CLEANING TARGETS
cleanall: clean
	@$(call PRINT,$(PROJNAME),"Deleting $(TARGET)")
	$(RM) $(TARGET)

clean: 
	@$(call PRINT,$(PROJNAME),"Deleting folder: $(OBJDIR)/")
	$(RM) -r ./$(OBJDIR)
	@$(call PRINT,$(PROJNAME),"Deleting objects to clean: $(OBJS2CLEAN)")
	$(foreach elem, $(OBJS2CLEAN), $(RM) -r ./$(elem))
