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
source ${CPCT_SCRIPTS_DIR}/lib/bash_library.sh

## More paths defined
CPCT_EXAMPLES_DIR=${SETUP_PATH}/examples
CPCT_SRC_DIR=${CPCT_MAIN_DIR}/src
CPCT_CFG_DIR=${CPCT_MAIN_DIR}/cfg
CPCT_LOGS_DIR=${CPCT_MAIN_DIR}/logs
CPCT_TOOLS_2CDT_DIR=${CPCT_TOOLS_DIR}/2cdt
CPCT_TOOLS_HEX2BIN_DIR=${CPCT_TOOLS_DIR}/hex2bin-2.0
CPCT_TOOLS_IDSK_DIR=${CPCT_TOOLS_DIR}/iDSK-0.13
CPCT_TOOLS_SDCC_DIR=${CPCT_TOOLS_DIR}/sdcc-3.4.3
CPCT_TEMPLATES_DIR=${CPCT_SCRIPTS_DIR}/templates

## Main Makefiles and other files
CPCT_TOOLS_MAKEFILE=${CPCT_TOOLS_DIR}/Makefile
CPCT_LIB_MAKEFILE=${CPCT_MAIN_DIR}/Makefile
CPCT_EXAMPLES_MAKEFILE=${CPCT_EXAMPLES_DIR}/Makefile
CPCT_TEMPLATES_MAKEFILE=${CPCT_TEMPLATES_DIR}/Makefile
CPCT_TEMPLATES_CFG=${CPCT_TEMPLATES_DIR}/build_config.mk
CPCT_TEMPLATES_MAIN=${CPCT_TEMPLATES_DIR}/main.c
CPCT_TEMPLATES_BASHRC=${CPCT_TEMPLATES_DIR}/bashrc.tmpl

## All directories and files
CPCT_DIRS=("${CPCT_MAIN_DIR}" "${CPCT_LOGS_DIR}" "${CPCT_EXAMPLES_DIR}" "${CPCT_TOOLS_DIR}"
           "${CPCT_SRC_DIR}" "${CPCT_CFG_DIR}" "${CPCT_TOOLS_2CDT_DIR}" "${CPCT_TOOLS_HEX2BIN_DIR}" 
           "${CPCT_TOOLS_IDSK_DIR}" "${CPCT_TOOLS_SDCC_DIR}" "${CPCT_TEMPLATES_DIR}")
CPCT_FILES=("${CPCT_TOOLS_MAKEFILE}" "${CPCT_LIB_MAKEFILE}" "${CPCT_EXAMPLES_MAKEFILE}" 
            "${CPCT_TEMPLATES_CFG}" "${CPCT_TEMPLATES_MAKEFILE}" "${CPCT_TEMPLATES_MAIN}")

## Generated files
CPCT_EXAMPLES_BUILD_LOG=${CPCT_LOGS_DIR}/examples_building.log
CPCT_TOOLS_BUILD_LOG=${CPCT_LOGS_DIR}/tool_building.log
CPCT_LIB_BUILD_LOG=${CPCT_LOGS_DIR}/library_building.log
CPCT_EXAMPLES_BUILD_LOG_TOTAL_BYTES=22563
CPCT_TOOLS_BUILD_LOG_TOTAL_BYTES=376244
CPCT_LIB_BUILD_LOG_TOTAL_BYTES=8835

## Substitution tags
CPCT_TAG_MAINPATH="%%%CPCTELERA_PATH%%%"

## Required stuff for running CPCtelera
REQUIRED_COMMANDS=(gcc g++ make bison flex)
COMMAND_EXPLANATION[0]="gcc compiler is required to compile tools. Please install it or build-essentials and run setup again."
COMMAND_EXPLANATION[1]="g++ compiler is required to compile tools. Please install it or build-essentials and run setup again."
COMMAND_EXPLANATION[2]="make is required for all CPCtelera's build systems. Please, install it an run setup again."
COMMAND_EXPLANATION[3]="bison is required to compile SDCC. Please, install it an run setup again."
COMMAND_EXPLANATION[4]="flex is required to compile SDCC. Please, install it an run setup again."

REQUIRED_LIBRARIES=("boost/graph/adjacency_list.hpp")
## libintl.h is not required in Mac OSX
if ! checkSystem "osx"; then
   REQUIRED_LIBRARIES+=( "libintl.h" )
fi
LIBRARIES_EXPLANATION[0]="${REQUIRED_LIBRARIES[0]} is part of libboost, which is required for building SDCC. Please, install boost / libboost-dev / libboost-devel or similar in your system and run setup again."
LIBRARIES_EXPLANATION[1]="${REQUIRED_LIBRARIES[1]} is required to build SDCC, which makes use of internationalization. Please, install intltool / libintl-dev / libint-devel or similar in your system and run setup again."

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
drawOK

# Check file structure
coloredMachineEcho "${COLOR_CYAN}" 0.005 "> Checking important files......."
for FILE in ${CPCT_FILES[*]}; do
   EnsureExists file "$FILE"
done
drawOK

# Check installed commands
coloredMachineEcho "${COLOR_CYAN}" 0.005 "> Checking required commands..."$'\n'
for C in $(seq 0 $((${#REQUIRED_COMMANDS[@]}-1))); do
   coloredMachineEcho "${COLOR_CYAN}" 0.005 ">>> Looking for '${REQUIRED_COMMANDS[$C]}'..."
   EnsureCommandAvailable ${REQUIRED_COMMANDS[$C]} "Command '${REQUIRED_COMMANDS[$C]}' not found installed in the system. ${COMMAND_EXPLANATION[$C]}"
   drawOK
done

# Check installed libraries
coloredMachineEcho "${COLOR_CYAN}" 0.005 "> Checking required libraries..."$'\n'
for C in $(seq 0 $((${#REQUIRED_LIBRARIES[@]}-1))); do
   coloredMachineEcho "${COLOR_CYAN}" 0.005 ">>> Looking for '${REQUIRED_LIBRARIES[$C]}'..."
   EnsureCPPHeaderAvailable ${REQUIRED_LIBRARIES[$C]} "Header file '${REQUIRED_LIBRARIES[$C]}' not found in the system. ${LIBRARIES_EXPLANATION[$C]}"
   drawOK
done
coloredMachineEcho ${COLOR_LIGHT_GREEN} 0.002 "Everything seems to be OK."$'\n'

###############################################################
###############################################################
## Build CPCtelera tools, library and examples
##
stageMessage "2" "Building CPCtelera tools, z80 library and examples"
coloredMachineEcho "${COLOR_CYAN}" 0.005 "> Proceeding to build required tools to build and manage CPCtelera and other software for Amstrad CPC (This might take a while, depending on your system):"$'\n'

# Build tools in subshell process, then go monitoring until it finishes
coloredMachineEcho "${COLOR_CYAN}" 0.005 ">>> Building compilation tools: "
( make -C "${CPCT_TOOLS_DIR}" &> "${CPCT_TOOLS_BUILD_LOG}" ; exit $? ) &
if ! superviseBackgroundProcess "$!" "${CPCT_TOOLS_BUILD_LOG}" "${CPCT_TOOLS_BUILD_LOG_TOTAL_BYTES}" 35 0.3; then
   Error "There was an error building CPCtelera tools. Please, check '${CPCT_TOOLS_BUILD_LOG}' for details. Aborting. "
fi
drawOK

# Build library in subshell process, then go monitoring until it finishes
coloredMachineEcho "${COLOR_CYAN}" 0.005 ">>> Building cpctelera z80 lib: "
( make -C "${CPCT_MAIN_DIR}" &> "${CPCT_LIB_BUILD_LOG}" ; exit $? ) &
if ! superviseBackgroundProcess "$!" "${CPCT_LIB_BUILD_LOG}" "${CPCT_LIB_BUILD_LOG_TOTAL_BYTES}" 35 0.05; then
   Error "There was an error building CPCtelera tools. Please, check '${CPCT_LIB_BUILD_LOG}' for details. Aborting. "
fi
drawOK

coloredMachineEcho ${COLOR_LIGHT_GREEN} 0.002 "> Bulding procedure finished. "$'\n'
coloredMachineEcho ${COLOR_LIGHT_GREEN} 0.002 "> CPCtelera's tools and library are now ready to be used on your system."$'\n'

# Build examples in subshell process, then go monitoring until it finishes
coloredMachineEcho $'\n'"${COLOR_CYAN}" 0.005 ">>> Building cpctelera examples:"
( make -C "${CPCT_EXAMPLES_DIR}" &> "${CPCT_EXAMPLES_BUILD_LOG}" ; exit $? ) &
if ! superviseBackgroundProcess "$!" "${CPCT_EXAMPLES_BUILD_LOG}" "${CPCT_EXAMPLES_BUILD_LOG_TOTAL_BYTES}" 35 0.1; then
   Error "There was an error building CPCtelera examples. Please, check '${CPCT_EXAMPLES_BUILD_LOG}' for details. Aborting. "
fi
drawOK

###############################################################
###############################################################
## Configuring environment and project templates
##
stageMessage "3" "Configuring CPCtelera environment"
coloredMachineEcho "${COLOR_CYAN}" 0.005 "> Setting up present CPCtelera folder as install directory and configuring routes and templates..."$'\n'

## Select System-dependent profile script
PROFILE=$(bashProfileFilename)

# Configuring CPCTelera global path in templates
coloredMachineEcho "${COLOR_CYAN}" 0.005 ">>> CPCTelera full path: ${COLOR_WHITE}${CPCT_MAIN_DIR}"$'\n'
coloredMachineEcho "${COLOR_CYAN}" 0.005 ">>> Inserting full path into build config template..."
replaceTaggedLine "${CPCT_TAG_MAINPATH}" "CPCT_PATH := ${CPCT_MAIN_DIR}\#${CPCT_TAG_MAINPATH}" "${CPCT_TEMPLATES_CFG}" '#'
drawOK

# Configuring PATH to use CPCTelera scripts in the system
coloredMachineEcho "${COLOR_CYAN}" 0.005 ">>> CPCTelera scripts path: ${COLOR_WHITE}${CPCT_SCRIPTS_DIR}"$'\n'
coloredMachineEcho "${COLOR_CYAN}" 0.005 ">>> Adding scripts path to ${COLOR_WHITE}\$PATH${COLOR_CYAN} variable in ${COLOR_WHITE}${PROFILE}${COLOR_CYAN}..."

# First, eliminate previous instances of CPCTelera into PROFILE, then add new
touch $PROFILE
removeLinesBetween "###CPCTELERA_START" "###CPCTELERA_END" "$PROFILE"
removeTrailingBlankLines "$PROFILE"
cat "$CPCT_TEMPLATES_BASHRC" >> "$PROFILE"
replaceTag "$CPCT_TAG_MAINPATH" "$CPCT_SCRIPTS_DIR" $PROFILE '#'
drawOK

###############################################################
###############################################################
## Final message to the user
##
echo
coloredMachineEcho ${COLOR_LIGHT_WHITE} 0.002 "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%"$'\n'"%%%"
coloredMachineEcho ${COLOR_LIGHT_GREEN} 0.002 " CPCtelera is now ready to be used on your system. "
coloredMachineEcho ${COLOR_LIGHT_WHITE} 0.002 "%%%"$'\n'"%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%"$'\n'
echo

coloredMachineEcho ${COLOR_CYAN} 0.001 "You may now go to the examples folder and play around with the included \
example projects. Inside any project's folder, just type make to create CDT and DSK files for Amstrad CPC. In the \
${COLOR_WHITE}src/${COLOR_CYAN} folder you will find C source code for each example. The ${COLOR_WHITE}cfg/\
${COLOR_CYAN} folder contains the building configuration for your project. Change everything as you like."$'\n'
echo
coloredMachineEcho ${COLOR_CYAN} 0.001 "If you wanted to create a new project, you may use \
${COLOR_WHITE}cpct_mkproject <project_folder>${COLOR_CYAN}. This is a script that automates the creation \
of new projects. For convenience, it has been included in your ${COLOR_WHITE}\$PATH${COLOR_CYAN} environment \
variable (you need to open a new shell for this to take effect). You may create projects anywhere, provided \
you do not change CPCtelera's main folder location. "$'\n'
echo
coloredMachineEcho ${COLOR_CYAN} 0.001 "If you have any comments, please go to \
${COLOR_WHITE}https://github.com/lronaldo/cpctelera${COLOR_CYAN} or send an email \
${COLOR_WHITE}cpctelera@cheesetea.com${COLOR_CYAN}. We hope you enjoy the library and expect to see your \
games comming out soon :)."$'\n'
echo ${COLOR_NORMAL}
