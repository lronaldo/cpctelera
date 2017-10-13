##-----------------------------LICENSE NOTICE------------------------------------
##  This file is part of CPCtelera: An Amstrad CPC Game Engine 
##  Copyright (C) 2014-2015 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
##                      Build configuration file                         ##
##-----------------------------------------------------------------------##
## This file is intendend for you to be able to config the way in which  ##
## you would like to build CPCtelera, the compiler executables and path. ##
## The most important thing you can do is selecting the actual files and ##
## modules you want to be built, thus leaving out anything you are not   ##
## going to use. This lets you save space an enables you to have only    ## 
## the code that will be actually executed.                              ##
###########################################################################

# Get directory path of this file at the momment of including it
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))

# Get the global path configuration of tools
include $(THIS_DIR)/global_paths.mk

####
## SECTION 1: COMPILER CONFIGURATION
##
##  Setting assembler flags for compilation of CPCTelera library
##
#####
INCLUDE_DIRS := -I$(CPCT_PATH)src/
Z80ASMFLAGS=$(INCLUDE_DIRS) -l -o -s 

####
## SECTION 2: LIBRARY CONFIG
##
## This section establishes source and object subfolders and the binary objects to
## be built. Normally, you want to change the OBJ files you want to be built, selecting
## only the ones that contain the actual code that will be used by you in your application.
##
## SRCDIR       >> Directory under which assembly source files are located
## OBJDIR       >> Directory where generated intermediate object files will be written to
## SUBDIRS      >> All first level subdirectories found in $(SRCDIR)/. They are assumed to contain code files.
## OBJSUBDIRS   >> Subdirectory structure for object files that replicates SUBDIRS
## SRCFILES     >> All .s files to be found under $(SRCDIR)/ and first level SUBDIRS, with full relative path
## OBJFILES     >> intermediate .rel files that will be needed for your application (their names
##                 have to correspond to existing source files in the SRCDIR, but with .rel extension)
#####

SRCDIR=src
OBJDIR=obj
SUBDIRS:=$(filter-out ., $(shell find $(SRCDIR) -type d -print))
OBJSUBDIRS:=$(foreach DIR, $(SUBDIRS), $(patsubst $(SRCDIR)%, $(OBJDIR)%, $(DIR)))
SRCFILES:=$(foreach DIR, $(SUBDIRS), $(wildcard $(DIR)/*.s))
OBJFILES:=$(patsubst $(SRCDIR)%, $(OBJDIR)%, $(SRCFILES:%.s=%.rel))
