#!/bin/bash
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
##                        Main  Setup File                               ##
##-----------------------------------------------------------------------##
## This file is a script intended for setting up the environment for the ##
## first time, before using it.                                          ##
###########################################################################

## Main Paths
SETUP_PATH="${PWD}"
CPCT_MAIN_DIR="${SETUP_PATH}/cpctelera"
CPCT_TOOLS_DIR="${CPCT_MAIN_DIR}/tools"
CPCT_SCRIPTS_DIR="${CPCT_TOOLS_DIR}/scripts"

## Bash Include files
source "${CPCT_SCRIPTS_DIR}/lib/bash_library.sh"

## Describe the use of the setup script and exit
##
function usage {
   echo "${COLOR_LIGHT_YELLOW}USAGE"
   echo "  ${COLOR_LIGHT_BLUE}$(basename $0) ${COLOR_LIGHT_CYAN}[options]"
   echo
   echo "${COLOR_CYAN}  Setups all the environment and installs CPCtelera. It first compiles \
all the tools included with CPCtelera, its Z80 library and code examples. Finally, it modifies \
your .bashrc or .bash_profile to include CPCT_PATH environment variable, which is used by \
CPCtelera projects to use the tools and link with the library."
   echo
   echo "${COLOR_LIGHT_YELLOW}OPTIONS"
   echo 
   echo "${COLOR_LIGHT_BLUE}  -eme | --enable-machine-echo"
   echo "${COLOR_LIGHT_BLUE}  -dme | --disable-machine-echo"
   echo "${COLOR_CYAN}       Enables or disables pauses between character writes that emulate \
classical machine echo to screen or printer. Enabled by default, except under Cygwin."
   echo 
   echo "${COLOR_LIGHT_BLUE}  -cri | --clean-reinstall"
   echo "${COLOR_CYAN}       Cleans all previous builds of CPCtelera before compiling and \
setting up tools, library and examples."
   echo
   echo "${COLOR_LIGHT_BLUE}  -h | --help"
   echo "${COLOR_CYAN}       Shows this help information"
   echo ${COLOR_NORMAL}
   exit 1
}

## More paths defined
CPCT_EXAMPLES_DIR=${SETUP_PATH}/examples
CPCT_SRC_DIR=${CPCT_MAIN_DIR}/src
CPCT_CFG_DIR=${CPCT_MAIN_DIR}/cfg
CPCT_BIN_DIR=${CPCT_MAIN_DIR}/bin
CPCT_DOCS_DIR=${CPCT_MAIN_DIR}/docs
CPCT_LOGS_DIR=${CPCT_MAIN_DIR}/logs
CPCT_TOOLS_2CDT_DIR=${CPCT_TOOLS_DIR}/2cdt
CPCT_TOOLS_CPC2CDT_DIR=${CPCT_TOOLS_DIR}/cpc2cdt
CPCT_TOOLS_HEX2BIN_DIR=${CPCT_TOOLS_DIR}/hex2bin-2.0
CPCT_TOOLS_DSKGEN_DIR=${CPCT_TOOLS_DIR}/dskgen 
CPCT_TOOLS_IDSK_DIR=${CPCT_TOOLS_DIR}/iDSK-0.13
CPCT_TOOLS_SDCC_DIR=${CPCT_TOOLS_DIR}/sdcc-3.6.8-r9946
CPCT_TOOLS_IMG2CPC_DIR=${CPCT_TOOLS_DIR}/img2cpc
CPCT_TOOLS_RGAS_DIR=${CPCT_TOOLS_DIR}/gfx/rgas-1.2.5
CPCT_TOOLS_ARKOS_DIR=${CPCT_TOOLS_DIR}/arkosTracker-1.0
CPCT_TOOLS_ZX7B_DIR=${CPCT_TOOLS_DIR}/zx7b
CPCT_TOOLS_ANDROID_DIR=${CPCT_TOOLS_DIR}/android
CPCT_TOOLS_ANDROID_BIN_DIR=${CPCT_TOOLS_ANDROID_DIR}/bin
CPCT_TOOLS_ANDROID_CERTS_DIR=${CPCT_TOOLS_ANDROID_DIR}/certs
CPCT_TOOLS_ANDROID_RVMENG_DIR=${CPCT_TOOLS_ANDROID_DIR}/rvmengine
CPCT_TEMPLATES_DIR=${CPCT_SCRIPTS_DIR}/templates
CPCT_TEMPLATES_NEWPRJ_DIR=${CPCT_SCRIPTS_DIR}/templates/newprj
CPCT_TEMPLATES_NEWPRJ_SRC_DIR=${CPCT_SCRIPTS_DIR}/templates/newprj/src
CPCT_TEMPLATES_NEWPRJ_CFG_DIR=${CPCT_SCRIPTS_DIR}/templates/newprj/cfg
CPCT_TEMPLATES_NEWPRJ_VCD_DIR=${CPCT_SCRIPTS_DIR}/templates/newprj/vscode
CPCT_TEMPLATES_NEWPRJ_EXP_DIR=${CPCT_TEMPLATES_NEWPRJ_CFG_DIR}/export

## Main Makefiles and other files
CPCT_TOOLS_MAKEFILE=${CPCT_TOOLS_DIR}/Makefile
CPCT_LIB_MAKEFILE=${CPCT_MAIN_DIR}/Makefile
CPCT_EXAMPLES_MAKEFILE=${CPCT_EXAMPLES_DIR}/Makefile
CPCT_TEMPLATES_MAKEFILE=${CPCT_TEMPLATES_NEWPRJ_DIR}/Makefile
CPCT_TEMPLATES_CFG=${CPCT_TEMPLATES_NEWPRJ_CFG_DIR}/build_config.mk
CPCT_TEMPLATES_CDT=${CPCT_TEMPLATES_NEWPRJ_CFG_DIR}/cdt_manager.mk
CPCT_TEMPLATES_PCK=${CPCT_TEMPLATES_NEWPRJ_CFG_DIR}/compression.mk
CPCT_TEMPLATES_MUS=${CPCT_TEMPLATES_NEWPRJ_CFG_DIR}/music_conversion.mk
CPCT_TEMPLATES_TIL=${CPCT_TEMPLATES_NEWPRJ_CFG_DIR}/tilemap_conversion.mk
CPCT_TEMPLATES_IMG=${CPCT_TEMPLATES_NEWPRJ_CFG_DIR}/image_conversion.mk
CPCT_TEMPLATES_EXP_ANDROID=${CPCT_TEMPLATES_NEWPRJ_EXP_DIR}/android.mk
CPCT_TEMPLATES_CFG_TMPL=${CPCT_TEMPLATES_NEWPRJ_CFG_DIR}/build_config.tmpl.mk
CPCT_TEMPLATES_MAIN=${CPCT_TEMPLATES_NEWPRJ_SRC_DIR}/main.c
CPCT_TEMPLATES_BASHRC=${CPCT_TEMPLATES_DIR}/bashrc.tmpl

## Executable files
CPCT_ARKOS_AKS2BIN=${CPCT_TOOLS_ARKOS_DIR}/tools/AKSToBIN.exe
CPCT_ARKOS_STK2AKS=${CPCT_TOOLS_ARKOS_DIR}/tools/STKToAKS.exe
CPCT_ZIPALIGN_LINUX32=${CPCT_TOOLS_ANDROID_BIN_DIR}/zipalign/linux/zipalign32
CPCT_ZIPALIGN_WIN32=${CPCT_TOOLS_ANDROID_BIN_DIR}/zipalign/win/32/zipalign.exe
CPCT_ZIPALIGN_WIN32_DLL=${CPCT_TOOLS_ANDROID_BIN_DIR}/zipalign/win/32/libwinpthread-1.dll
CPCT_JARSIGNER=${CPCT_TOOLS_ANDROID_BIN_DIR}/sun/jarsigner.jar
CPCT_KEYTOOL=${CPCT_TOOLS_ANDROID_BIN_DIR}/sun/keytool.jar
CPCT_APKTOOL=${CPCT_TOOLS_ANDROID_BIN_DIR}/apktool/apktool_2.4.0.jar
CPCT_RVMENG_APK=${CPCT_TOOLS_ANDROID_RVMENG_DIR}/defaultRVMapp.apk

## All directories and files
CPCT_DIRS=("${CPCT_MAIN_DIR}" "${CPCT_LOGS_DIR}" "${CPCT_EXAMPLES_DIR}" "${CPCT_TOOLS_DIR}"
           "${CPCT_SRC_DIR}" "${CPCT_CFG_DIR}" "${CPCT_TOOLS_2CDT_DIR}" "${CPCT_TOOLS_HEX2BIN_DIR}" 
           "${CPCT_TOOLS_IDSK_DIR}" "${CPCT_TOOLS_SDCC_DIR}" "${CPCT_TEMPLATES_DIR}"
           "${CPCT_TEMPLATES_NEWPRJ_DIR}" "${CPCT_TOOLS_IMG2CPC_DIR}" "${CPCT_TOOLS_RGAS_DIR}" 
           "${CPCT_TOOLS_ARKOS_DIR}" "${CPCT_TEMPLATES_NEWPRJ_SRC_DIR}" "${CPCT_BIN_DIR}"
           "${CPCT_TEMPLATES_NEWPRJ_CFG_DIR}" "${CPCT_TEMPLATES_NEWPRJ_VCD_DIR}" "${CPCT_TEMPLATES_NEWPRJ_EXP_DIR}"
           "${CPCT_DOCS_DIR}" "${CPCT_TOOLS_CPC2CDT_DIR}" "${CPCT_TOOLS_ANDROID_DIR}" "${CPCT_TOOLS_ANDROID_BIN_DIR}"
           "${CPCT_TOOLS_ANDROID_CERTS_DIR}" "${CPCT_TOOLS_ANDROID_RVMENG_DIR}" "${CPCT_TOOLS_DSKGEN_DIR}"
           "${CPCT_TOOLS_ZX7B_DIR}")
CPCT_FILES=("${CPCT_TOOLS_MAKEFILE}" "${CPCT_LIB_MAKEFILE}" "${CPCT_EXAMPLES_MAKEFILE}" 
            "${CPCT_TEMPLATES_CFG_TMPL}" "${CPCT_TEMPLATES_MAKEFILE}" "${CPCT_TEMPLATES_MAIN}"
            "${CPCT_TEMPLATES_CDT}" "${CPCT_TEMPLATES_PCK}" "${CPCT_TEMPLATES_MUS}" 
            "${CPCT_TEMPLATES_TIL}" "${CPCT_TEMPLATES_IMG}" "${CPCT_TEMPLATES_BASHRC}"
            "${CPCT_TEMPLATES_EXP_ANDROID}" )
CPCT_EXECUTABLE_FILES=("${CPCT_ARKOS_AKS2BIN}" "${CPCT_ARKOS_STK2AKS}" "${CPCT_ZIPALIGN_LINUX32}"
            "${CPCT_ZIPALIGN_WIN32}" "${CPCT_ZIPALIGN_WIN32_DLL}" "${CPCT_JARSIGNER}" "${CPCT_KEYTOOL}"
            "${CPCT_APKTOOL}" "${CPCT_RVMENG_APK}")

## Generated files
CPCT_EXAMPLES_BUILD_LOG=${CPCT_LOGS_DIR}/examples_building.log
CPCT_TOOLS_BUILD_LOG=${CPCT_LOGS_DIR}/tool_building.log
CPCT_LIB_BUILD_LOG=${CPCT_LOGS_DIR}/library_building.log
CPCT_EXAMPLES_BUILD_LOG_TOTAL_BYTES_CLEAN=28969
CPCT_EXAMPLES_BUILD_LOG_TOTAL_BYTES=168536
CPCT_TOOLS_BUILD_LOG_TOTAL_BYTES_CLEAN=25948
CPCT_TOOLS_BUILD_LOG_TOTAL_BYTES=627034
CPCT_LIB_BUILD_LOG_TOTAL_BYTES_CLEAN=319
CPCT_LIB_BUILD_LOG_TOTAL_BYTES=63048

## Substitution tags
CPCT_TAG_MAINPATH="%%%CPCTELERA_PATH%%%"
CPCT_TAG_SCRIPTSPATH="%%%CPCTELERA_SCRIPTS_PATH%%%"

## Required stuff for running CPCtelera
REQUIRED_COMMANDS=(gcc g++ make bison flex)
COMM_NUM=0
COMMAND_EXPLANATION[$COMM_NUM]="${REQUIRED_COMMANDS[$COMM_NUM]} compiler is required to compile tools. Please \
install it or build-essentials and run setup again."
COMM_NUM=$((COMM_NUM + 1))
COMMAND_EXPLANATION[$COMM_NUM]="${REQUIRED_COMMANDS[$COMM_NUM]} compiler is required to compile tools. Please \
install it or build-essentials and run setup again."
COMM_NUM=$((COMM_NUM + 1))
COMMAND_EXPLANATION[$COMM_NUM]="${REQUIRED_COMMANDS[$COMM_NUM]} is required for all CPCtelera's build systems. \
Please, install it and run setup again."
COMM_NUM=$((COMM_NUM + 1))
COMMAND_EXPLANATION[$COMM_NUM]="${REQUIRED_COMMANDS[$COMM_NUM]} is required to compile SDCC. Please, install it \
and run setup again."
COMM_NUM=$((COMM_NUM + 1))
COMMAND_EXPLANATION[$COMM_NUM]="${REQUIRED_COMMANDS[$COMM_NUM]} is required to compile SDCC. Please, install it \
and run setup again."
COMM_NUM=$((COMM_NUM + 1))
GCC_MINIMUM_VERSION="5.1"

REQUIRED_LIBRARIES=("boost/graph/adjacency_list.hpp")
LIBRARIES_EXPLANATION[0]="${REQUIRED_LIBRARIES[0]} is part of libboost, which is required for building SDCC. Please, install boost / libboost-dev / libboost-devel or similar in your system and run setup again."

## libintl.h is not required in Mac OSX
if ! checkSystem "osx"; then
   REQUIRED_LIBRARIES+=( "libintl.h" )
   LIBRARIES_EXPLANATION+=( "Libintl (development) is required to build SDCC, which makes use of internationalization. Please, install intltool / libintl-dev / libint-devel or similar in your system and run setup again." )
fi
## Freeimage is not required in Cygwin (binaries already included)
if ! checkSystem "cygwin"; then
   REQUIRED_LIBRARIES+=( "FreeImage.h" )
   LIBRARIES_EXPLANATION+=( "Freeimage (development) is required to build Img2CPC. Please, install freeimage / libfreeimage-dev / freeimage-devel or similar in your system and run setup again." )
   REQUIRED_COMMANDS+=( mono )
   COMMAND_EXPLANATION[$COMM_NUM]="${REQUIRED_COMMANDS[$COMM_NUM]} is required to convert arkos audio files to code automatically. Please, install it \
and run setup again." 
   COMM_NUM=$((COMM_NUM + 1))
fi

## On cygwin, machine echo is disabled by default as it is too slow
if checkSystem "cygwin"; then
   disableMachineEchoSleep
fi

##
## Setup Control Variables and 
## Process Command line parameters
##
CLEANREINSTALL=false
while (( $# >= 1 )); do
   case $1 in
      ## Disable Machine Echo
      "-eme" | "--enable-machine-echo")
         enableMachineEchoSleep
      ;;
      ## Disable Machine Echo
      "-dme" | "--disable-machine-echo")
         disableMachineEchoSleep
      ;;
      ## Get number of bytes
      "-cri" | "--clean-reinstall")
         CLEANREINSTALL=true
      ;;
      ## Show Help
      "-h" | "--help")
         usage
      ;;
      ## Unrecognized parameter / command line option
      *)
         paramError "Unrecognized parameter / command line option '$1'" 7
      ;;
   esac
   shift
done

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

## Checking main path has no spaces in its name
EnsureFilenameHasNoSpaces "$CPCT_MAIN_DIR" "CPCtelera installation path cannot have spaces in between. Please ensure CPCtelera is in a path without spaces before relaunching setup.sh. "

## Checking that directories exist
for (( i = 0; i < ${#CPCT_DIRS[@]}; i++ )); do
   EnsureExists directory "${CPCT_DIRS[$i]}"
done
drawOK

# Check file structure
coloredMachineEcho "${COLOR_CYAN}" 0.005 "> Checking important files......."
for (( i = 0; i < ${#CPCT_FILES[@]}; i++ )); do
   EnsureExists file "${CPCT_FILES[$i]}"
done
for (( i = 0; i < ${#CPCT_EXECUTABLE_FILES[@]}; i++ )); do
   ensureExistsAndIsExecutable file "${CPCT_EXECUTABLE_FILES[$i]}"
done
drawOK

# Check installed commands
coloredMachineEcho "${COLOR_CYAN}" 0.005 "> Checking required commands..."$'\n'
for (( i = 0; i < ${#REQUIRED_COMMANDS[@]}; i++ )); do
   coloredMachineEcho "${COLOR_CYAN}" 0.005 ">>> Looking for '${REQUIRED_COMMANDS[$i]}'..."
   EnsureCommandAvailable ${REQUIRED_COMMANDS[$i]} "Command '${REQUIRED_COMMANDS[$i]}' not found installed in the system. ${COMMAND_EXPLANATION[$i]}"
   drawOK
done

# Check Command versions
coloredMachineEcho "${COLOR_CYAN}" 0.005 "> Checking command versions..."$'\n'

## Different check depending if the main compiler is CLang or GCC
if isClangDefaultCompiler; then
   ## Checks for CLang Compiler
   coloredMachineEcho "${COLOR_CYAN}" 0.005 ">>> Clang has C++11 support..."
   ensureClangHasRequiredFeatures
   drawOK
else
   ## Checks for GCC Compiler
   coloredMachineEcho "${COLOR_CYAN}" 0.005 ">>> GNU GCC/G++ Version >= $GCC_MINIMUM_VERSION..."
   checkMinimumGCCVersion "$GCC_MINIMUM_VERSION"
   case "$?" in
      1) Error "CPCtelera requires GCC $GCC_MINIMUM_VERSION or greater. Please, update \
   your GCC version and run setup again." 1
      ;;
      -1) Error "It was impossible to determine your GCC version. Either your GCC version \
   is too old (previous to 1999) or something is wrong with your GCC installation. Please \
   check your GCC installation and update your version to $GCC_MINIMUM_VERSION or greater \
   and run setup again." 2
      ;;
      *) drawOK 
      ;;
   esac
fi

# Check installed libraries
coloredMachineEcho "${COLOR_CYAN}" 0.005 "> Checking required libraries..."$'\n'
for (( i = 0; i < ${#REQUIRED_LIBRARIES[@]}; i++ )); do
   coloredMachineEcho "${COLOR_CYAN}" 0.005 ">>> Looking for '${REQUIRED_LIBRARIES[$i]}'..."
   EnsureCPPHeaderAvailable ${REQUIRED_LIBRARIES[$i]} "Header file '${REQUIRED_LIBRARIES[$i]}' not found in the system. ${LIBRARIES_EXPLANATION[$i]}"
   drawOK
done
coloredMachineEcho ${COLOR_LIGHT_GREEN} 0.002 "Everything seems to be OK."$'\n'

###############################################################
###############################################################
## Build CPCtelera tools, library and examples
##
stageMessage "2" "Building CPCtelera tools, z80 library and examples"
coloredMachineEcho "${COLOR_CYAN}" 0.005 "> Proceeding to build required tools to build and manage CPCtelera and other software for Amstrad CPC (This might take a while, depending on your system)."$'\n'

###------
# Clean previous installations, if the user requested it
###------
if $CLEANREINSTALL; then
   coloredMachineEcho "${COLOR_CYAN}" 0.005 ">> Cleaning previous installation to perform a clean reinstall..."$'\n'
   
   ## Cleaning tools
   coloredMachineEcho "${COLOR_CYAN}" 0.005 ">>> Cleaning previosly built tools:    "
   makeWithProgressSupervision "$CPCT_TOOLS_DIR" "$CPCT_TOOLS_BUILD_LOG" cleanall \
                              "$CPCT_TOOLS_BUILD_LOG_TOTAL_BYTES_CLEAN"  35  0.05  \
                    "There was an error cleaning previous build of CPCtelera's tools" 

   ## Cleaning library
   coloredMachineEcho "${COLOR_CYAN}" 0.005 ">>> Cleaning previosly built z80 lib:  "
   makeWithProgressSupervision "$CPCT_MAIN_DIR" "$CPCT_LIB_BUILD_LOG" cleanall \
                             "$CPCT_LIB_BUILD_LOG_TOTAL_BYTES_CLEAN"  35  0.05  \
               "There was an error cleaning previous build of CPCtelera's Z80 Library" 

   ## Cleaning examples
   coloredMachineEcho "${COLOR_CYAN}" 0.005 ">>> Cleaning previosly built examples: "
   makeWithProgressSupervision "$CPCT_EXAMPLES_DIR" "$CPCT_EXAMPLES_BUILD_LOG" cleanall \
                                 "$CPCT_EXAMPLES_BUILD_LOG_TOTAL_BYTES_CLEAN"  35  0.1  \
               "There was an error cleaning previous build of CPCtelera's examples" 

   coloredMachineEcho "${COLOR_CYAN}" 0.005 ">> Previous install is clean. Proceeding to rebuild..."$'\n'
fi

# Build tools in subshell process, then go monitoring until it finishes
coloredMachineEcho "${COLOR_CYAN}" 0.005 ">>> Building compilation tools:        "
makeWithProgressSupervision "$CPCT_TOOLS_DIR" "$CPCT_TOOLS_BUILD_LOG" "" \
                           "$CPCT_TOOLS_BUILD_LOG_TOTAL_BYTES"  35  0.3  \
                         "There was an error building CPCtelera tools"

# Build library in subshell process, then go monitoring until it finishes
coloredMachineEcho "${COLOR_CYAN}" 0.005 ">>> Building cpctelera z80 lib:        "
make -C "${CPCT_MAIN_DIR}" cleanall &> "${CPCT_LIB_BUILD_LOG}"
makeWithProgressSupervision "$CPCT_MAIN_DIR" "$CPCT_LIB_BUILD_LOG" "" \
                          "$CPCT_LIB_BUILD_LOG_TOTAL_BYTES"  35  0.1  \
                     "There was an error building CPCtelera z80 library"

coloredMachineEcho ${COLOR_LIGHT_GREEN} 0.002 "> Bulding procedure finished. "$'\n'
coloredMachineEcho ${COLOR_LIGHT_GREEN} 0.002 "> CPCtelera's tools and library are now ready to be used on your system."$'\n'

# Build examples in subshell process, then go monitoring until it finishes
coloredMachineEcho $'\n'"${COLOR_CYAN}" 0.005 ">>> Building cpctelera examples:       "
export PATH=$PATH:$CPCT_SCRIPTS_DIR
export CPCT_PATH=$CPCT_MAIN_DIR
makeWithProgressSupervision "$CPCT_EXAMPLES_DIR" "$CPCT_EXAMPLES_BUILD_LOG" "" \
                              "$CPCT_EXAMPLES_BUILD_LOG_TOTAL_BYTES"  35  0.1  \
                               "There was an error building CPCtelera examples."

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
cp "${CPCT_TEMPLATES_CFG_TMPL}" "${CPCT_TEMPLATES_CFG}"
replaceTaggedLine "${CPCT_TAG_MAINPATH}" "CPCT_PATH := ${CPCT_MAIN_DIR}\#${CPCT_TAG_MAINPATH}" "${CPCT_TEMPLATES_CFG}" '#'
drawOK

# Configuring PATH to use CPCTelera scripts in the system
coloredMachineEcho "${COLOR_CYAN}" 0.005 ">>> CPCTelera scripts path: ${COLOR_WHITE}${CPCT_SCRIPTS_DIR}"$'\n'
coloredMachineEcho "${COLOR_CYAN}" 0.005 ">>> Adding scripts path to ${COLOR_WHITE}\$PATH${COLOR_CYAN} variable in ${COLOR_WHITE}${PROFILE}${COLOR_CYAN}..."

# First, eliminate previous instances of CPCTelera into PROFILE, then add new
touch "$PROFILE"
removeLinesBetween "###CPCTELERA_START" "###CPCTELERA_END" "$PROFILE"
removeTrailingBlankLines "$PROFILE"
cat "$CPCT_TEMPLATES_BASHRC" >> "$PROFILE"
replaceTag "$CPCT_TAG_MAINPATH" "$CPCT_MAIN_DIR" "$PROFILE" '#'
replaceTag "$CPCT_TAG_SCRIPTSPATH" "$CPCT_SCRIPTS_DIR" "$PROFILE" '#'
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
