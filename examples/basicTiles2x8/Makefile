###########################################################################
##                   USE EXAMPLE OF CPCTELERA ENGINE                     ##
##                                                                       ##
##-----------------------------------------------------------------------##
## This example demonstrates how to use the simple function to put tiles ##
## (2x8 byte sprites): cpct_drawSprite2x8_aligned.                       ##
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

# BINARY CONFIG
PROJNAME=example1
VPATH=$(SRCDIR)
TARGET=$(PROJNAME).bin

# CALCULATED VARIABLES
OBJ=$(foreach FILE,$(OBJFILES),$(OBJDIR)/$(FILE))

.PHONY: all clean cleanall

# MAIN TARGET
all: $(OBJDIR) $(TARGET)

$(OBJDIR)/$(PROJNAME).ihx: $(OBJ)
	@$(call PRINT,"Linking binary file")
	$(Z80CC) $(Z80CCLINKARGS) $^ -o "$@"
	@$(call PRINT,"Successfully created $@")

# SUBTARGETS
$(OBJDIR)/%.rel: %.c
	$(Z80CC) $(Z80CCINCLUDE) -mz80 $(Z80CCFLAGS) -c $< -o $(OBJDIR)/

$(OBJDIR)/%.bin.log %.bin: $(OBJDIR)/%.ihx
	$(HEX2BIN) -e "bin" -p 00 "$<" | tee "$<.log"


	
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
