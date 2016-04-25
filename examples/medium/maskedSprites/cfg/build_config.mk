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
##                        CPCTELERA ENGINE                               ##
##                     Build configuration file                          ##
##-----------------------------------------------------------------------##
## This file is intendend for you to be able to config the way in which  ##
## you would like to build this example of use of the CPCtelera Engine.  ##
## Below you will find several configuration sections with the macros    ##
## available to configure the build, along with explanation comments to  ##
## help you understand what they do. Please, change everything you want. ##
###########################################################################

## CPCTELERA MAIN PATH
##   Sets CPCTelera main path for accessing tools and configuration. If you
##   change folder structure, change CPCT_PATH value for its absolute path.
##
THIS_FILE_PATH := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
CPCT_PATH      := $(THIS_FILE_PATH)../../../../cpctelera/

####
## SECTION 1: Project configuration 
##
## This section establishes source and object subfolders and the binary objects to
## be built. Normally, you want to change the OBJ files you want to be built, selecting
## only the ones that contain the actual code that will be used by you in your application.
#####

# Name of the project (without spaces, as it will be used as filename)
#   and Z80 memory location where code will start in the generated binary
PROJNAME   := maskedSprites
Z80CODELOC := 0x0100

# Folders 
SRCDIR  := src
OBJDIR  := obj

# File extensions
C_EXT   := c
ASM_EXT := s
OBJ_EXT := rel

# BINARY CONFIG
BINFILE := $(OBJDIR)/$(PROJNAME).bin
IHXFILE := $(OBJDIR)/$(PROJNAME).ihx
CDT     := $(PROJNAME).cdt
DSK     := $(PROJNAME).dsk

# TARGETs for compilation (if you only want one of them, remove the other)
TARGET  := $(CDT) $(DSK)

####
## SECTION 2: TOOL PATH CONFIGURATION
##
## Paths are configured in the global_paths.mk configuration file included 
## here. You may overwrite the values of path variables after the include 
## if you wanted specific configuration for this project.
####
include $(CPCT_PATH)/cfg/global_paths.mk

####
## SECTION 3: COMPILATION CONFIGURATION
##
##   Flags used to configure the compilation of your code. They are usually 
##   fine for most of the projects, but you may change them for special uses.
#####
Z80CCFLAGS    :=
Z80ASMFLAGS   := -l -o -s
Z80CCINCLUDE  := -I$(CPCT_SRC)
Z80CCLINKARGS := -mz80 --no-std-crt0 -Wl-u \
                 --code-loc $(Z80CODELOC) \
                 --data-loc 0 -l$(CPCT_LIB)
####
## SECTION 4: CALCULATED FOLDERS, SUBFOLDERS AND FILES
##
##  These macros calculate code subfolders, get all the source files and generate
##  the corresponding subfolders and files in the object directory. All subfolders
##  and files with source extension found are added, up to 1 level of depth in
##  folder structure inside the main source directory.
####
include $(CPCT_PATH)/cfg/global_functions.mk

# Calculate all subdirectories
SUBDIRS    := $(filter-out ., $(shell find $(SRCDIR) -type d -print))
OBJSUBDIRS := $(foreach DIR, $(SUBDIRS), $(patsubst $(SRCDIR)%, $(OBJDIR)%, $(DIR)))

# Calculate all source files
CFILES     := $(foreach DIR, $(SUBDIRS), $(wildcard $(DIR)/*.$(C_EXT)))
ASMFILES   := $(foreach DIR, $(SUBDIRS), $(wildcard $(DIR)/*.$(ASM_EXT)))

# Calculate all object files
C_OBJFILES   := $(patsubst $(SRCDIR)%, $(OBJDIR)%, $(patsubst %.$(C_EXT), %.$(OBJ_EXT), $(CFILES)))
ASM_OBJFILES := $(patsubst $(SRCDIR)%, $(OBJDIR)%, $(patsubst %.$(ASM_EXT), %.$(OBJ_EXT), $(ASMFILES)))
OBJFILES		 := $(C_OBJFILES) $(ASM_OBJFILES)
