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

## Main Paths
SETUP_PATH="${PWD}"
CPCT_MAIN_DIR=${SETUP_PATH}/cpctelera
CPCT_TOOLS_DIR=${CPCT_MAIN_DIR}/tools
CPCT_SCRIPTS_DIR=${CPCT_TOOLS_DIR}/scripts

## Bash Include files
source ${CPCT_SCRIPTS_DIR}/bash_library.sh

## More paths defined
CPCT_EXAMPLES_DIR=${SETUP_PATH}/examples
CPCT_SRC_DIR=${CPCT_MAIN_DIR}/src
CPCT_CFG_DIR=${CPCT_MAIN_DIR}/cfg
CPCT_LOGS_DIR=${CPCT_MAIN_DIR}/logs
CPCT_TOOLS_2CDT_DIR=${CPCT_TOOLS_DIR}/2cdt
CPCT_TOOLS_HEX2BIN_DIR=${CPCT_TOOLS_DIR}/hex2bin-2.0
CPCT_TOOLS_IDSK_DIR=${CPCT_TOOLS_DIR}/iDSK-0.13
CPCT_TOOLS_SDCC_DIR=${CPCT_TOOLS_DIR}/sdcc-3.4.3

## Main Makefiles
CPCT_TOOLS_MAKEFILE=${CPCT_TOOLS_DIR}/Makefile
CPCT_LIB_MAKEFILE=${CPCT_MAIN_DIR}/Makefile
CPCT_EXAMPLES_MAKEFILE=${CPCT_EXAMPLES_DIR}/Makefile

## All directories and files
CPCT_DIRS=("${CPCT_MAIN_DIR}" "${CPCT_LOGS_DIR}" "${CPCT_EXAMPLES_DIR}" "${CPCT_TOOLS_DIR}"
           "${CPCT_SRC_DIR}" "${CPCT_CFG_DIR}" "${CPCT_TOOLS_2CDT_DIR}" "${CPCT_TOOLS_HEX2BIN_DIR}" 
           "${CPCT_TOOLS_IDSK_DIR}" "${CPCT_TOOLS_SDCC_DIR}")
CPCT_FILES=("${CPCT_TOOLS_MAKEFILE}" "${CPCT_LIB_MAKEFILE}" "${CPCT_EXAMPLES_MAKEFILE}")

## Generated files
CPCT_TOOLS_BUILD_LOG=${CPCT_LOGS_DIR}/tool_building.log
CPCT_LIB_BUILD_LOG=${CPCT_LOGS_DIR}/library_building.log
CPCT_TOOLS_BUILD_LOG_TOTAL_BYTES=369073
CPCT_LIB_BUILD_LOG_TOTAL_BYTES=8133

## Required stuff for running CPCtelera
REQUIRED_COMMANDS=(gcc g++ bison flex)
COMMAND_EXPLANATION[0]="gcc compiler is required to compile tools. Please install it or build-essentials and run setup again."
COMMAND_EXPLANATION[1]="g++ compiler is required to compile tools. Please install it or build-essentials and run setup again."
COMMAND_EXPLANATION[2]="bison is required to compile SDCC. Please, install it an run setup again."
COMMAND_EXPLANATION[3]="flex is required to compile SDCC. Please, install it an run setup again."
REQUIRED_LIBRARIES=("boost/graph/adjacency_list.hpp")
LIBRARIES_EXPLANATION[0]="libboost is required for building SDCC. Please, install libboost-dev / libboost-devel or similar in your system and run setup again."


###############################################################
###############################################################
## Perform CPCtelera requirements tests
##   - Check directory structure
##   - Check required commands installed
##   - Check required libraries installed
##
clearScreen
welcomeMessage
stageMessage "1" "CPCtelera initial tests"

# Check directory structure
coloredMachineEcho "${COLOR_CYAN}" 0.005 "> Checking directory structure..."
for DIR in ${CPCT_DIRS[*]}; do
   EnsureExists directory "$DIR"
done
coloredMachineEcho ${COLOR_LIGHT_GREEN} 0.05 "[ OK ]"$'\n'

# Check file structure
coloredMachineEcho "${COLOR_CYAN}" 0.005 "> Checking important files......."
for FILE in ${CPCT_FILES[*]}; do
   EnsureExists file "$FILE"
done
coloredMachineEcho ${COLOR_LIGHT_GREEN} 0.05 "[ OK ]"$'\n'

# Check installed commands
coloredMachineEcho "${COLOR_CYAN}" 0.005 "> Checking required commands..."$'\n'
for C in $(seq 0 $((${#REQUIRED_COMMANDS[@]}-1))); do
   coloredMachineEcho "${COLOR_CYAN}" 0.005 ">>> Looking for '${REQUIRED_COMMANDS[$C]}'..."
   EnsureCommandAvailable ${REQUIRED_COMMANDS[$C]} "Command '${REQUIRED_COMMANDS[$C]}' not found installed in the system. ${COMMAND_EXPLANATION[$C]}"
   coloredMachineEcho ${COLOR_LIGHT_GREEN} 0.05 "[ OK ]"$'\n'
done

# Check installed libraries
#      Error ""

coloredMachineEcho "${COLOR_CYAN}" 0.005 "> Checking required libraries..."$'\n'
for C in $(seq 0 $((${#REQUIRED_LIBRARIES[@]}-1))); do
   coloredMachineEcho "${COLOR_CYAN}" 0.005 ">>> Looking for '${REQUIRED_LIBRARIES[$C]}'..."
   EnsureCPPHeaderAvailable ${REQUIRED_LIBRARIES[$C]} "Header file '${REQUIRED_LIBRARIES[$C]}' not found in the system. ${LIBRARIES_EXPLANATION[$C]}"
   coloredMachineEcho ${COLOR_LIGHT_GREEN} 0.05 "[ OK ]"$'\n'
done
coloredMachineEcho ${COLOR_LIGHT_GREEN} 0.002 "Everything seems to be OK."$'\n'

###############################################################
###############################################################
## Build CPCtelera tools and library
##
stageMessage "2" "Building CPCtelera tools and z80 library"
coloredMachineEcho "${COLOR_CYAN}" 0.005 "> Proceeding to build required tools to build and manage CPCtelera and other software for Amstrad CPC (This might take a while, depending on your system):"$'\n'

# Build tools in subshell process, then go monitoring until it finishes
coloredMachineEcho "${COLOR_CYAN}" 0.005 ">>> Building compilation tools: "
( make -C "${CPCT_TOOLS_DIR}" &> "${CPCT_TOOLS_BUILD_LOG}" ; exit $? ) &
if ! superviseBackgroundProcess "$!" "${CPCT_TOOLS_BUILD_LOG}" "${CPCT_TOOLS_BUILD_LOG_TOTAL_BYTES}" 35 0.3; then
   Error "There was an error building CPCtelera tools. Please, check '${CPCT_TOOLS_BUILD_LOG}' for details. Aborting. "
fi
coloredMachineEcho ${COLOR_LIGHT_GREEN} 0.05 " [ OK ]"$'\n'

# Build library in subshell process, then go monitoring until it finishes
coloredMachineEcho "${COLOR_CYAN}" 0.005 ">>> Building cpctelera z80 lib: "
( make -C "${CPCT_MAIN_DIR}" &> "${CPCT_LIB_BUILD_LOG}" ; exit $? ) &
if ! superviseBackgroundProcess "$!" "${CPCT_LIB_BUILD_LOG}" "${CPCT_LIB_BUILD_LOG_TOTAL_BYTES}" 35 0.05; then
   Error "There was an error building CPCtelera tools. Please, check '${CPCT_LIB_BUILD_LOG}' for details. Aborting. "
fi
coloredMachineEcho ${COLOR_LIGHT_GREEN} 0.05 " [ OK ]"$'\n'

coloredMachineEcho ${COLOR_LIGHT_GREEN} 0.002 "> Bulding procedure finished. "$'\n'
coloredMachineEcho ${COLOR_LIGHT_GREEN} 0.002 "> CPCtelera's tools and library are now ready to be used on your system."$'\n'

###############################################################
###############################################################
## Build code samples if required
##
#askSimpleQuestion y n "Reply y/n" retval
#echo "RETORNO=$retval"
#exit
