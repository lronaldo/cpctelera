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
##     CPCTELERA ENGINE: Example of use of cpct_drawSprite               ##
##                       Main Building Makefile                          ##
##-----------------------------------------------------------------------##
## This file contains the rules for building an example of use of the    ##
## CPCtelera engine.                                                     ##
## Usually, this file should be left unchanged. To configure your build  ##
## you should change the build.conf file.                                ##
###########################################################################
# INCLUDES
include build_config.mk

# UTILITY FUNCTIONS
define PRINT
 echo -ne "\033[1;31;49m"
 echo -ne "[$(PROJNAME)]>"
 echo -ne "\033[1;33;49m $(1)"
 echo -e  "\033[0;39;49m"
endef

define CREATEDSK
 (LOADADDR=$$( sed -n 's/^Lowest address  = 0000\([0-9]*\).*$$/\1/p' <$(1).log ) ; \
  RUNADDR=$$( sed -n 's/^ *0000\([0-9A-F]*\) *cpc_run_address  *.*$$/\1/p' <$(1:.bin=.map) ) ; \
  if [ -z "$$RUNADDR" ] ; then \
  RUNADDR=$$( sed -n 's/^ *0000\([0-9A-F]*\) *init  *.*$$/\1/p' <$(1:.bin=.map) ) ; \
  fi ; \
  if [ -z "$$RUNADDR" ] ; then \
  RUNADDR=$$( sed -n 's/^ *0000\([0-9A-F]*\) *_main  *.*$$/\1/p' <$(1:.bin=.map) ) ; \
  fi ; \
  if [ -z "$$RUNADDR" ] ; then \
  echo "Cannot figure out run address. Aborting." ; exit 1 ; \
  fi ; \
  $(IDSK) $2.tmp -n -i $1 -e $${RUNADDR} -c $${LOADADDR} -t 1 && mv -vf $2.tmp $2 ; \
 )
endef


# TOOLS CONFIG
RM=rm -f
MKDIR=mkdir

# BINARY CONFIG
PROJNAME=putCharacter
VPATH=$(SRCDIR)
TARGET=$(PROJNAME).dsk

# CALCULATED VARIABLES
OBJ=$(foreach FILE,$(OBJFILES),$(OBJDIR)/$(FILE))

.PHONY: all clean cleanall

# MAIN TARGET
all: $(OBJDIR) $(TARGET)

# LINK RELOCATABLE MACHINE CODE FILES (.REL) INTO A INTEL HEX BINARY (.IHX)
$(OBJDIR)/$(PROJNAME).ihx: $(OBJ)
	@$(call PRINT,"Linking binary file")
	@$(call PRINT,$(CSRCFILES))
	$(Z80CC) $(Z80CCLINKARGS) $^ -o "$@"

# COMPILE C FILES => GENERATE RELOCATABLE (.REL) MACHINE CODE FILES
$(OBJDIR)/%.rel: %.c
	$(Z80CC) $(Z80CCINCLUDE) -mz80 $(Z80CCFLAGS) -c $< -o $(OBJDIR)/

# GENERATE AMSDOS BINARY FILE (.BIN) FROM INTEL HEX BINARY (.IHX)
$(OBJDIR)/%.bin.log $(OBJDIR)/%.bin: $(OBJDIR)/%.ihx
	@$(call PRINT,"Creating Amsdos binary file $@")
	$(HEX2BIN) -e "bin" -p 00 "$<" | tee "$(patsubst %.ihx,%.bin,$<).log"

# GENERATE A DISK FILE (.DSK) AND INCLUDE BINARY FILE (.BIN) INTO IT
%.dsk: $(OBJDIR)/%.bin
	@$(call PRINT,"Creating Disk File $@")
	@$(call CREATEDSK,$<,$@)
	@$(call PRINT,"Successfully created $@")

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
