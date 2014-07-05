###########################################################################
##                          CPCTELERA ENGINE                             ##
##                     Linux Main Building Makefile                      ##
##-----------------------------------------------------------------------##
## This file describes the way in which Cpctelera library should be      ##
## built. In general, there is no need to make changes to this file.     ##
## Instead, you should modify build.conf to set up your building config. ##
##-----------------------------------------------------------------------##
## Author:  ronaldo / @FranGallegoBR                                     ##
## (C) 2014 Fremos / Cheesetea / ByteRealms                              ##
###########################################################################
# INCLUDES
include build.conf

# UTILITY FUNCTIONS
define PRINT
 echo -en "\033[1;31;49m"
 echo -n  "[$(LIBRARYNAME)]>"
 echo -en "\033[1;33;49m $(1)"
 echo -e  "\033[0;39;49m"
endef

# TOOLS CONFIG
RM=rm -f
MKDIR=mkdir

# LIBRARY CONFIG
LIBRARYNAME=cpctelera
VPATH=$(SRCDIR)
TARGET=$(LIBRARYNAME).lib

# CALCULATED VARIABLES
OBJ=$(foreach FILE,$(OBJFILES),$(OBJDIR)/$(FILE))

.PHONY: all clean cleanall

# MAIN TARGET
all: $(OBJDIR) $(TARGET)
	
$(TARGET): $(OBJ)
	@$(call PRINT,"Linking library file")
	$(Z80LNK) rc $@ $^
	@$(call PRINT,"Successfully created $(TARGET)")

# SUBTARGETS
$(OBJDIR)/%.rel: %.s
	$(Z80ASM) $(Z80ASMFLAGS) $@ $<

	
# CLEANING TARGETS
cleanall: clean
	@$(call PRINT,"Deleting $(TARGET)")
	$(RM) $(TARGET)

clean: 
	@$(call PRINT,"Deleting folder: $(OBJDIR)/")
	$(RM) -r $(OBJDIR)
	
# CREATE OBJDIR IF IT DOES NOT EXIST
$(OBJDIR):
	@$(call PRINT,"Creating object folder: $@/")
	$(MKDIR) $(OBJDIR)
