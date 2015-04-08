##-----------------------------LICENSE NOTICE------------------------------------
##  This file is part of CPCtelera: An Amstrad CPC Game Engine 
##  Copyright (C) 2015 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
##
##  This program is free software: you can redistribute it and/or modify
##  it under the terms of the GNU General Public License as published by
##  the Free Software Foundation, either version 3 of the License, or
##  (at your option) any later version.
##
##  This program is distributed in the hope that it will be useful,
##  but WITHOUT ANY WARRANTY; without even the implied warranty of
##  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##  GNU General Public License for more details.
##
##  You should have received a copy of the GNU General Public License
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
# INCLUDES
include cfg/build_config.mk

.PHONY: all clean cleanall 

# MAIN TARGET
all: $(OBJSUBDIRS) $(TARGET)

## COMPILING SOURCEFILES AND SAVE OBJFILES IN THEIR CORRESPONDENT SUBDIRS
$(foreach OF, $(OBJFILES), $(eval $(call COMPILECFILE, $(OF), $(patsubst $(OBJDIR)%,$(SRCDIR)%,$(OF:%.$(OBJEXT)=%.$(SRCEXT))))))

# LINK RELOCATABLE MACHINE CODE FILES (.REL) INTO A INTEL HEX BINARY (.IHX)
$(IHXFILE): $(OBJFILES)
	@$(call PRINT,"Linking binary file")
	$(Z80CC) $(Z80CCLINKARGS) $^ -o "$@"

# GENERATE BINARY FILE (.BIN) FROM INTEL HEX BINARY (.IHX)
## FIX: Binary file gets deleted magically when compilation ends. We make a copy of it for other uses.
%.bin: %.ihx
	@$(call PRINT,"Creating Amsdos binary file $@")
	$(HEX2BIN) -p 00 "$<" | $(TEE) $@.log
	@cp $@ $@.bin

# GENERATE A DISK FILE (.DSK) AND INCLUDE BINARY FILE (.BIN) INTO IT
%.dsk: $(BINFILE)
	@$(call PRINT,"Creating Disk File $@")
	@$(call GETBINADDRESSES,$<)
	@$(call CREATEDSK,$<,$@)
	@$(call PRINT,"Successfully created $@")

# GENERATE A CASSETTE FILE (.CDT) AND INCLUDE BINARY FILE (.BIN) INTO IT
%.cdt: $(BINFILE)
	@$(call PRINT,"Creating Cassette File $@")
	@$(call GETBINADDRESSES,$<)
	@$(call CREATECDT,$<,$(notdir $<),$@)
	@$(call PRINT,"Successfully created $@")

# CREATE OBJDIR & SUBDIRS IF THEY DO NOT EXIST
$(OBJSUBDIRS): 
	@$(MKDIR) $@

# CLEANING TARGETS
cleanall: clean
	@$(call PRINT,"Deleting $(TARGET)")
	$(RM) $(TARGET)

clean: 
	@$(call PRINT,"Deleting folder: $(OBJDIR)/")
	$(RM) -r $(OBJDIR)