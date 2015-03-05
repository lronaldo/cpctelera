##-----------------------------LICENSE NOTICE------------------------------------
##  This file is part of CPCtelera: An Amstrad CPC Game Engine 
##  Copyright (C) 2014 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
##                     Linux Main Building Makefile                      ##
##-----------------------------------------------------------------------##
## This file describes the way in which CPCtelera library should be      ##
## built.                                                                ##
## In general, there is no need to make changes to this file.            ##
## Instead, you should modify build.conf to set up your building config. ##
###########################################################################
# INCLUDES
include build_config.mk

# TOOLS CONFIG
RM=rm -f
MKDIR=mkdir

# UTILITY FUNCTIONS
define PRINT
 @echo -ne "\033[1;31;49m"
 @echo -ne "[$(LIBRARYNAME)]>"
 @echo -ne "\033[1;33;49m $(1)"
 @echo -e  "\033[0;39;49m"

endef

define CREATEDIR
 @$(call PRINT,"folder: $(1)")
 @$(MKDIR) -p $(1)

endef

define SUBTARGET
$(1)/%.rel: $(patsubst $(SRCDIR)%,$(OBJDIR)%,$(1))/%.s
	$(Z80ASM) $(Z80ASMFLAGS) $@ $<
endef

# LIBRARY CONFIG
LIBRARYNAME=cpctelera
VPATH=$(SRCDIR)
TARGET=$(LIBRARYNAME).lib

# CALCULATED VARIABLES
#OBJ=$(foreach FILE,$(OBJFILES),$(OBJDIR)/$(FILE))
.PHONY: all clean cleanall

# MAIN TARGET
all: $(OBJDIR) $(TARGET)
#all: 
#	@$(call PRINT,"SUBDIRS: \'$(SUBDIRS)\'")
#	@$(call PRINT,"OBJSUBDIRS: \'$(OBJSUBDIRS)\'")
#	@$(call PRINT,"ASMFILES: \'$(ASMFILES)\'")
#	@$(call PRINT,"OBJFILES: \'$(OBJFILES)\'")
	
$(TARGET): $(OBJFILES)
	@$(call PRINT,"Linking library file")
	$(Z80LNK) rc $@ $^
	@$(call PRINT,"Successfully created $(TARGET)")

# SUBTARGETS
$(foreach DIR, $(OBJSUBDIRS), $(eval $(call SUBTARGET $(DIR))))
#$(OBJFILES): %.s
#	$(Z80ASM) $(Z80ASMFLAGS) $@ $<

	
# CLEANING TARGETS
cleanall: clean
	@$(call PRINT,"Deleting $(TARGET)")
	$(RM) $(TARGET)

clean: 
	@$(call PRINT,"Deleting folder: $(OBJDIR)/")
	$(RM) -r $(OBJDIR)
	
# CREATE OBJDIR & SUBDIRS IF THEY DO NOT EXIST
$(OBJDIR):
	@$(call PRINT,"Creating tree structure for object files")
	$(foreach DIR, $(SUBDIRS), $(call CREATEDIR, $(OBJSUBDIRS)))
