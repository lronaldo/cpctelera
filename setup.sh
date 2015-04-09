#!/bin/bash
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
##                        CPCTELERA ENGINE                               ##
##                        Main  Setup File                               ##
##-----------------------------------------------------------------------##
## This file is a script intended for setting up the environment for the ##
## first time, before using it.                                          ##
###########################################################################

## Colors for ANSI terminal
COLOR_LIGHT_RED="\033[1;31;49m"
COLOR_LIGHT_GREEN="\033[1;32;49m"
COLOR_LIGHT_YELLOW="\033[1;33;49m"
COLOR_LIGHT_BLUE="\033[1;34;49m"
COLOR_LIGHT_MAGENTA="\033[1;35;49m"
COLOR_MAGENTA="\033[0;35;49m"
COLOR_NORMAL="\033[0;39;49m"
## $1: Error Message
## $2: Number of error that will be returned to shell
function Error {
   echo -e ${COLOR_LIGHT_RED}
   echo "#########################"
   echo "## UNRECOVERABLE ERROR ##"
   echo "#########################"
   echo -e "##> ${COLOR_LIGHT_YELLOW}${1}${COLOR_LIGHT_NORMAL}"
   echo
   exit $2
}

## $1: Stage
## $2: Message
function stageMessage {
   echo -e ${COLOR_LIGHT_BLUE}
   echo "==========================================="
   echo -e "== ${COLOR_LIGHT_MAGENTA}${1}: ${COLOR_MAGENTA}${2}${COLOR_LIGHT_BLUE}"
   echo -e "==========================================="${COLOR_NORMAL}
}

## Clears the screen
function clearScreen {
   echo -e "\033[H\033[J"
}

## $1: Seconds between characters
## $2: Message to write
function machineEcho {
   local i
   for i in $(seq 0 ${#2}); do
      echo -ne "${2:${i}:1}"
      sleep "$1"
   done
   echo
}

function welcomeMessage {
   echo -ne "${COLOR_LIGHT_BLUE}"
   machineEcho 0.04 "Welcome to..."
   echo -ne "${COLOR_LIGHT_YELLOW}"
   machineEcho 0.005 " ____     ____     ____     __            ___"
   machineEcho 0.005 "/\\  _\`\\  /\\  _\`\\  /\\  _\`\\  /\\ \\__        /\\_ \\"
   machineEcho 0.005 "\\ \\ \\/\\_\\\\ \\ \\L\\ \\\\ \\ \\/\\_\\\\ \\ ,_\\     __\\//\\ \\       __   _ __    __"
   machineEcho 0.005 " \\ \\ \\/_/_\\ \\ ,__/ \\ \\ \\/_/_\\ \\ \\/   /'__\`\\\\ \\ \\    /'__\`\\/\\\`'__\\/'__\`\\"
   machineEcho 0.005 "  \\ \\ \\L\\ \\\\ \\ \\/   \\ \\ \\L\\ \\\\ \\ \\_ /\\  __/ \\_\\ \\_ /\\  __/\\ \\ \\//\\ \\L\\.\\_"
   machineEcho 0.005 "   \\ \\____/ \\ \\_\\    \\ \\____/ \\ \\__\\\\ \\____\\/\\____\\\\ \\____\\\\ \\_\\\\ \\__/.\\_\\"
   machineEcho 0.005 "    \\/___/   \\/_/     \\/___/   \\/__/ \\/____/\\/____/ \\/____/ \\/_/ \\/__/\\/_/"
   echo -e ${COLOR_LIGHT_BLUE}
   machineEcho 0.02 "This setup script will help you configure CPCtelera in your system."
   echo
}

## $1: ANSI color string
## $2: Message
function colorMessage {
   echo -e ${1}${2}${COLOR_NORMAL}
}

## $1: Type of file (file, directory)
## $2: File/Directory to check for existance
function EnsureExists {
   local MOD
   case "$1" in
      file)  
         MOD="-f";;
      directory) 
         MOD="-d" ;;
      *)
         Error "Filetype '$1' unrecognized while testing file '$2'."
   esac
   if [ ! $MOD "$2" ]; then
      Error "$1 '$2' does not exist, and it is required for CPCTelera framework to work propperly."
   elif [ ! -r "$2" ]; then
      Error "$1 '$2' is not readable (check for permissions)."
   fi
}

## Main Paths
SETUP_PATH="${PWD}"
CPCT_MAIN_DIR=${SETUP_PATH}/cpctelera
CPCT_TOOLS_DIR=${CPCT_MAIN_DIR}/tools
CPCT_SRC_DIR=${CPCT_MAIN_DIR}/src
CPCT_CFG_DIR=${CPCT_MAIN_DIR}/cfg
CPCT_TOOLS_2CDT_DIR=${CPCT_TOOLS_DIR}/2cdt
CPCT_TOOLS_HEX2BIN_DIR=${CPCT_TOOLS_DIR}/hex2bin-2.0
CPCT_TOOLS_IDSK_DIR=${CPCT_TOOLS_DIR}/iDSK-0.13
CPCT_TOOLS_SDCC_DIR=${CPCT_TOOLS_DIR}/sdcc-3.4.3
CPCT_DIRS=${CPCT_MAIN_DIR}\ ${CPCT_TOOLS_DIR}\ ${CPCT_SRC_DIR}\ ${CPCT_CFG_DIR}\ ${CPCT_TOOLS_2CDT_DIR}\ ${CPCT_TOOLS_HEX2BIN_DIR}\ ${CPCT_TOOLS_IDSK_DIR}\ ${CPCT_TOOLS_SDCC_DIR}
CPCT_TOOLS_MAKEFILE=${CPCT_TOOLS_DIR}/Makefile
CPCT_LIB_MAKEFILE=${CPCT_MAIN_DIR}/Makefile
CPCT_FILES=${CPCT_TOOLS_MAKEFILE}\ ${CPCT_LIB_MAKEFILE}

# Check CPCTelera directory structure
clearScreen
welcomeMessage
stageMessage "1" "Checking Directory Structure..."
for DIR in ${CPCT_DIRS}; do
   EnsureExists directory "$DIR"
done
for FILE in ${CPCT_FILES}; do
   EnsureExists file "$FILE"
done

