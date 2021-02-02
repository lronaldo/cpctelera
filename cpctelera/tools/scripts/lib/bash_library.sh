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
##               Bash function library for scripting                     ##
##-----------------------------------------------------------------------##
## This file is intended to be used as a repository of bash functions to ##
## be used in CPCtelera's bash scripts.                                  ##
###########################################################################

## Colors for ANSI terminal
COLOR_LIGHT_RED=$'\033[1;31;49m'
COLOR_LIGHT_GREEN=$'\033[1;32;49m'
COLOR_LIGHT_YELLOW=$'\033[1;33;49m'
COLOR_LIGHT_BLUE=$'\033[1;34;49m'
COLOR_LIGHT_MAGENTA=$'\033[1;35;49m'
COLOR_LIGHT_CYAN=$'\033[1;36;49m'
COLOR_LIGHT_WHITE=$'\033[1;37;49m'
COLOR_INVERTED_LIGHT_RED=$'\033[1;39;41m'
COLOR_INVERTED_LIGHT_GREEN=$'\033[1;39;42m'
COLOR_INVERTED_GREEN=$'\033[0;39;42m'
COLOR_INVERTED_CYAN=$'\033[0;39;46m'
COLOR_INVERTED_WHITE=$'\033[0;39;47m'
COLOR_RED=$'\033[0;31;49m'
COLOR_GREEN=$'\033[0;32;49m'
COLOR_MAGENTA=$'\033[0;35;49m'
COLOR_CYAN=$'\033[0;36;49m'
COLOR_YELLOW=$'\033[0;33;49m'
COLOR_WHITE=$'\033[0;37;49m'
COLOR_NORMAL=$'\033[0;39;49m'

## Control variables
ENABLED_MACHINE_ECHO_SLEEP=true

## Show a big error message for an unrecoverable error and exit
## $1: Error Message
## $2: Number of error that will be returned to shell
function Error {
   echo
   echo ${COLOR_LIGHT_RED}
   echo "#########################"
   echo "## UNRECOVERABLE ERROR ##"
   echo "#########################"
   echo "##> ${COLOR_LIGHT_YELLOW}${1}${COLOR_NORMAL}"
   echo
   exit $2
}

## Show a normal error message for a badly given command line parameter and exit
## $1: Error Message
## $2: Number of error that will be returned to shell
function paramError {
   echo ${COLOR_LIGHT_RED}[ ERROR ]: ${COLOR_RED}${1}${COLOR_NORMAL}
   echo
   exit $2
}

## $1: Stage
## $2: Message
function stageMessage {
   echo ${COLOR_LIGHT_BLUE}
   echo "==============================================================="
   echo "== ${COLOR_LIGHT_MAGENTA}${1}: ${COLOR_MAGENTA}${2}${COLOR_LIGHT_BLUE}"
   echo "==============================================================="${COLOR_NORMAL}
}

## Clears the screen
function clearScreen {
   printf "\033[H\033[J"
}


## $1: Seconds between characters
## $2: Message to write
function machineEcho_sleep {
   local i
   for (( i=0; i <= ${#2}; i++ )); do
      echo -n "${2:${i}:1}"
      sleep "$1"
   done
}

## $1: Seconds between characters
## $2: Message to write
function machineEcho_no_sleep {
   local i
   for (( i=0; i <= ${#2}; i++ )); do
      echo -n "${2:${i}:1}"
   done
}

## Select specific machineEcho mode depending
## on the system we are being executed
function machineEcho {
  if $ENABLED_MACHINE_ECHO_SLEEP; then
    machineEcho_sleep "$1" "$2"
  else
    machineEcho_no_sleep "$1" "$2" 
  fi
}

## $1: Color ANSI sequence
## $2: Seconds between characters
## $3: Message to write
function coloredMachineEcho {
   echo -n "$1"
   machineEcho $2 "$3"
}

## Disables sleeps between character output when doing Machine Echo
## 
function disableMachineEchoSleep {
   ENABLED_MACHINE_ECHO_SLEEP=false
}

## Enable sleeps between character output when doing Machine Echo
## 
function enableMachineEchoSleep {
   ENABLED_MACHINE_ECHO_SLEEP=true
}

## Welcome to CPCtelera
##
function welcomeMessage {
   local DLY_T1=0.04
   local DLY_3D=0.002
   local DLY_T2=0.01

   echo -n "${COLOR_LIGHT_BLUE}"
   machineEcho $DLY_T1 "Welcome to..."$'\n'
   sleep 0.4
   echo -n "${COLOR_LIGHT_YELLOW}"
   machineEcho $DLY_3D " ____     ____     ____     __            ___"$'\n'
   machineEcho $DLY_3D "/\\  _\`\\  /\\  _\`\\  /\\  _\`\\  /\\ \\__        /\\_ \\"$'\n'
   machineEcho $DLY_3D "\\ \\ \\/\\_\\\\ \\ \\L\\ \\\\ \\ \\/\\_\\\\ \\ ,_\\     __\\//\\ \\       __   _ __    __"$'\n'
   machineEcho $DLY_3D " \\ \\ \\/_/_\\ \\ ,__/ \\ \\ \\/_/_\\ \\ \\/   /'__\`\\\\ \\ \\    /'__\`\\/\\\`'__\\/'__\`\\"$'\n'
   machineEcho $DLY_3D "  \\ \\ \\L\\ \\\\ \\ \\/   \\ \\ \\L\\ \\\\ \\ \\_ /\\  __/ \\_\\ \\_ /\\  __/\\ \\ \\//\\ \\L\\.\\_"$'\n'
   machineEcho $DLY_3D "   \\ \\____/ \\ \\_\\    \\ \\____/ \\ \\__\\\\ \\____\\/\\____\\\\ \\____\\\\ \\_\\\\ \\__/.\\_\\"$'\n'
   machineEcho $DLY_3D "    \\/___/   \\/_/     \\/___/   \\/__/ \\/____/\\/____/ \\/____/ \\/_/ \\/__/\\/_/"$'\n'
   echo ${COLOR_LIGHT_BLUE}
   machineEcho $DLY_T2 "This setup script will help you configure CPCtelera in your system."
   echo
}

## Ensures that a file of a given type exists and is readable by the user. Otherwise, it throws an unrecoverable error
## $1: Type of file (file, directory)
## $2: File/Directory to check for existance
## $3: Aditional error message, used when file does not exist or is not readable
##
function EnsureExists {
   local MOD
   case "$1" in
      file)
         MOD="-f";;
      directory)
         MOD="-d" ;;
      *)
         Error "Filetype '$1' unrecognized while testing file '$2'." 121
   esac
   if [ ! $MOD "$2" ]; then
      Error "$1 '$2' does not exist, and it is required for CPCTelera framework to work propperly. $3" 121
   elif [ ! -r "$2" ]; then
      Error "$1 '$2' exists, but it is not readable (check for permissions). $3" 121
   fi
}

## $1: X
## $2: Y
##
function setCursorXY {
   printf "\033[%s;%sH" $2 $1
}

## Saves Cursor Position
function saveCursorPos {
   printf "\033[s"
}

## Restores Cursor Position
function restoreCursorPos {
   printf "\033[u"
}

## $1: Number of characters to move cursor left
##
function cursorLeft {
   printf "\033[%sD" $1
}

## $1: PID of the process
## Returns 0 if running, error otherwise
function processRunning {
   kill -s 0 "$1" &> /dev/null
   return $?
}

## Repeats a character N times
## $1: Character to repeat
## $2: times to be repeated
##
function repeatCharacter {
   printf "$1"'%.0s' $(eval "echo {1.."$(($2))"}")
}

## $1: Size (in characters, without brackets)
## $2: Percentage
## $3: ANSI color sequence for BARS
## $4: ANSI color sequence for Spaces
##
function drawProgressBar {
   local PCT="$2"
   local BARS
   if (( ${#PCT} == 0 )); then
      PCT = 0
   fi
   if (( PCT > 100 )); then
      PCT=100
   elif (( PCT < 0 )); then
      PCT=0
   fi
   local NUMBARS=$(($1 * PCT / 100))
   if (( NUMBARS > $1 )); then
      NUMBARS=$1
   fi
   local NUMSPACES=$(($1 - NUMBARS))
   echo -n ${3}
   if ((NUMBARS > 0)); then
      repeatCharacter ' ' $NUMBARS
   fi
   if ((NUMSPACES > 0)); then
      echo -n ${4}
      repeatCharacter ' ' $NUMSPACES
   fi
   echo -n ${COLOR_NORMAL} ${PCT}%
}

#### Checks if a given command is available on the system. 
## Returns 0 if the command is available, 1 otherwise
##  $1: Command to check for in the system
##
function isCommandAvailable {
   if command -v "$1" >/dev/null 2>&1; then
      return 0
   fi
   return 1
}

#### Checks if a given command is available on the system. 
## If the command is not available, aborts execution with an error message.
##  $1: Command to check for in the system
##  $2: Error Message
##
function EnsureCommandAvailable {
   isCommandAvailable "$1"
   if [[ "$?" != "0" ]]; then
      Error "$2" 127
   fi
}


## $1: Header file name
## $2: Error message
##
function EnsureCPPHeaderAvailable {
   local ERRTMP=$(createTempFile).err
   local OUTTMP=$(createTempFile).out
   local SRCTMP=$(createTempFile).cpp
   echo "#include <${1}>" > "$SRCTMP"
   echo "int main(void) { return 0; }" >> "$SRCTMP"
   g++ -o "$OUTTMP" -c "$SRCTMP" 2> "$ERRTMP"
   if [ -s "$ERRTMP" ]; then
      # File has warnings or errors. We ignore warnings.
      if grep "error" "$ERRTMP"; then
         Error "$2" 126
      fi
   fi
}

## Check if default compiler is clang
##   Returns: 0 if Clang is installed
##            1 otherwise
##
function isClangDefaultCompiler {
   local ERRTMP=$(createTempFile).err
   local OUTTMP=$(createTempFile).out
   local SRCTMP=$(createTempFile).cpp
   ##
   ## Here document with the test CPP program to know if
   ## Clang is installed or not.
   ## Returns 0 if compiler is clang, 1 otherwise
   ##
   cat <<-oooooooooooooooooooooooooo > "${SRCTMP}"
	int main(void) {
	#ifdef __clang__
	   return 0;
	#else
	   return 1;
	#endif
	}
	oooooooooooooooooooooooooo
   gcc "$SRCTMP" -o "$OUTTMP" 2> "$ERRTMP"
   if [ -s "$ERRTMP" ]; then
      return 1
   else
      return $(${OUTTMP})
   fi
}

## Ensures that Clang has required features for compilation or not. 
## Otherwise, it throws an unrecoverable error
##
function ensureClangHasRequiredFeatures {
   local ERRTMP=$(createTempFile).err
   local OUTTMP=$(createTempFile).out
   local SRCTMP=$(createTempFile).cpp
   local MSG_CLANGINSTALLED="You have CLang installed as main compiler and \
your CLang installation has no support for required C++11 features. "
   local MSG_UPGRADE="Please, upgrade your CLang installation or install GCC \
>= 4.6 as main compiler. "
   local MSG_OSXUPGRADE=" Under OSX, you may install GCC using brew (http://brew.sh). "
   local ERROR_MSG=""

   ## Here document with the test C program to know if CLang has
   ## all required features or not. The program returns
   ##    0: Has all the required features
   ##    1: No Generalized initializers suport
   ##    2: No Ranged Fors suport
   ##    3: No support for 1 & 2
	cat <<-oooooooooooooooooooooooooo > "$SRCTMP"
	int main(void) {
	   unsigned char val = 0b00000011;
	   // Checking if __has_feature clang macro is present
	   #ifdef __has_feature
	   
	      // Checking if C++11 Generalized Identifiers is available
	      #if __has_feature(cxx_generalized_initializers)
	         val &= 0b11111110;   // It is available, set identifier bit to 0        
	      #endif
	   
	      // Checkinf if C++11 Ranged fors is available
	      #if __has_feature(cxx_range_for)
	         val &= 0b11111101;   // It is available, set identifier bit to 0        
	      #endif
	         
	   #endif
	   return val;
	}
	oooooooooooooooooooooooooo

   ## Compile the test program
   g++ -std=c++0x -O3 -Wall -fsigned-char "$SRCTMP" -o "$OUTTMP" 2> "$ERRTMP"

   ## Check if there was an error
   if [ -s "$ERRTMP" ]; then
      ERROR_MSG="${MSG_CLANGINSTALLED}${MSG_UPGRADE}"
    else
        ${OUTTMP}
        case $? in
         0) ERROR_MSG=""
         ;;
         2) ERROR_MSG="${MSG_CLANGINSTALLED} This installation does not support C++11 \
Ranged for loops. ${MSG_UPGRADE}"
         ;;
         *) ERROR_MSG="${MSG_CLANGINSTALLED} This installation does not support C++11 \
Generalized initializers. ${MSG_UPGRADE}"
         ;;
      esac
   fi

   ##
   ## If there was an error, print the error message and abort
   ##
   if [[ "$ERROR_MSG" != "" ]]; then
      if checkSystem osx; then 
         ERROR_MSG="${ERROR_MSG}${MSG_OSXUPGRADE}";
      fi
      Error "$ERROR_MSG" 125
   fi
}

## $1: PID of the process to monitor
## $2: Logfile generated by the process
## $3: Total Size for the logfile at the end of the process
## $4: Size of the progressBar
## $5: Sleep interval (between checks)
function superviseBackgroundProcess {
   # Go checking the build process until it finishes
   local PROCPID=$1
   local LOGFILE=$2
   local MAXBYTES=$3
   local BARSIZE=$4
   local SLEEPTIME=$5
   local BYTES
   local PCT
   local LEFT
   local EXIT_STATUS
   while processRunning "$PROCPID"; do
      if isFileReadable "$LOGFILE"; then
         BYTES=$(wc -c "${LOGFILE}" | grep -Eo '[0-9]+ ')
         PCT=$((BYTES * 100 / MAXBYTES))
         LEFT=$((BARSIZE + 2 + ${#PCT}))
         #saveCursorPos
         drawProgressBar "${BARSIZE}" "${PCT}" ${COLOR_INVERTED_GREEN} ${COLOR_INVERTED_WHITE}
         sleep ${SLEEPTIME}
         #restoreCursorPos
         cursorLeft $LEFT
      else
         sleep ${SLEEPTIME}
      fi
   done
   wait "$PROCPID" 
   EXIT_STATUS=$?
   if [[ "$EXIT_STATUS" == 0 ]]; then
      drawProgressBar "${BARSIZE}" 100 ${COLOR_INVERTED_LIGHT_GREEN} ${COLOR_INVERTED_WHITE}
   else
      drawProgressBar "${BARSIZE}" "${PCT}" ${COLOR_INVERTED_LIGHT_RED} ${COLOR_INVERTED_WHITE}
   fi
   return $EXIT_STATUS
}

## $1..n-2: Array containing acceptable keys as reply
## $n-1:    Message Question
## $n:      Return variable
##
function askSimpleQuestion {
   local REPLY
   local n=$#
   local RETURNVAL=${!n}
   n=$((n-1))
   local QUESTION=${!n}
   local BADREPLY=1
   n=$((n-1))
   declare -a VALIDREPLIES=("${@:1:n}")

   ## Check that stdin is a terminal
   if [ -t 0 ]; then
      stty -echo -icanon time 0 min 0
   fi;

   echo -n "$QUESTION"
   while (( BADREPLY )); do
      read -s -n 1 REPLY
      if contains ${VALIDREPLIES[@]} $REPLY; then
         BADREPLY=0
      fi
   done

   ## Check that stdin is a terminal
   if [ -t 0 ]; then
      stty sane
   fi;

   eval $RETURNVAL=$REPLY
}

## Check if an array contains a given value
## $1..$n-1: Array values
## $n: value to find in the array
##
function contains {
    local n=$#
    local value=${!n}
    for ((i=1;i < $#;i++)) {
        if [ "${!i}" == "${value}" ]; then
            return 0
        fi
    }
    return 1
}

## Check if a given value is an integer or not
## $1: value to be tested
##
function isInt {
   if [[ $1 =~ ^[-+]?[0-9]+$ ]]; then
      return 0
   fi
   return 1
}

## Check if a given value is an hexadecimal value or not
## $1: value to be tested
##
function isHex {
   if [[ $1 =~ ^[0-9A-Fa-f]+$ ]]; then
      return 0
   fi
   return 1
}

## Check if a given value is a binary value or not
## $1: value to be tested
##
function isBin {
   if [[ $1 =~ ^[0-1]+$ ]]; then
      return 0
   fi
   return 1
}

## Check if a given value is a C-hexadecimal value or not
## $1: Value to be tested
##
function isCHex {
   if [[ $1 =~ ^0[x|X][0-9A-Fa-f]+$ ]]; then
      return 0
   fi
   return 1  
}

## Check if a given value is a C-binary value or not
## $1: Value to be tested
##
function isCBin {
   if [[ $1 =~ ^0[b|B][0-1]+$ ]]; then
      return 0
   fi
   return 1  
}

## Converts a binary number to a decimal one. The
## number may have 0b/0B prefix or not.
## $1: binary number
##
function bin2Dec {
   local B="$1"
   if [ ${B:0:2} = "0b" ] || [ ${B:0:2} = "0B" ]; then
     B="${B:2}"
   fi
   echo $((2#$B))
}

## Converts a hexadecimal number to a decimal one. The number
## may have 0x/0X prefix or no prefix at all
## $1: hexadecimal number
##
function hex2Dec {
   local H="$1"
   if [ "${H:0:2}" != "0x" ] && [ "${H:0:2}" != "0X" ]; then
     H="0x${H}"
   fi
   printf "%d" "$H"
}

## Converts and integer decimal number to binary base.
## $1: integer decimal number
##
function dec2Bin {
   echo "obase=2;$1" | bc
}

## Converts and integer decimal number to hexadecimal base
## $1: integer decimal number
##
function dec2Hex {
   printf "%x" "$1"
}

## Converts a number to decimal, be it a Binary or an hexadecimal one
## $1: number to convert to decimal
##
function any2Dec {
   local N="$1"
   if isInt "$N"; then
      echo "$N"
   elif isBin "$N" || isCBin "$N"; then 
      echo $(bin2Dec "$N")
   elif isHex "$N" || isCHex "$N"; then
      echo $(hex2Dec "$N")
   fi
}

## Check if a given value is a valid C integer value 
## either in decimal, hexadecimal or binary
## $1: Value to be tested
##
function isCIntValue {
   if isInt "$1"; then 
      return 0
   elif isCHex "$1"; then
      return 0
   elif isCBin "$1"; then
      return 0
   fi
   return 1
}


## Check if a given value is empty
## $1: Value to check
##
function isEmpty {
   if [ "$1" == "" ]; then
      return 0
   fi
   return 1
}

## Check if a given value is a command line option (starts with -)
## $1: Value to check
##
function isCommandLineOption {
   if [ "${1:0:1}" == "-" ]; then 
      return 0
   fi
   return 1
}

## Check if a file exists and is readable (only files, not directories)
## $1: File to check 
##
function isFileReadable {
   if [ -f "$1" ] && [ -r "$1" ]; then
      return 0
   fi
   return 1
}

## Checks if a given value is a valid C idenfitier or not
## $1: Value to check as identifier
##
function isValidCIdentifier { 
   local REX='^[a-zA-Z_][a-zA-Z0-9_]*$'
   if [[ $1 =~ $REX ]]; then
      return 0
   fi
   return 1
}

## Check if a folder exists and is readable (only folders, not files)
## $1: Folder to check 
##
function isFolderReadable {
   if [ -d "$1" ] && [ -r "$1" ]; then
      return 0
   fi
   return 1
}

## Gets the full path of a given file, from its relative path
## $1: File
## $2: Return variable where to store the real path
##
function getFullPath {
   pushd $(dirname $1) &> /dev/null
   eval $2=${PWD}
   popd &> /dev/null
}

## Draws an OK checkmark
function drawOK {
   coloredMachineEcho ${COLOR_LIGHT_GREEN} 0.05 " [ OK ]"$'\n'
}

## Replaces a complete line in a file which contains a given tag 
## $1: Tag to be searched
## $2: New line to replace the one that contains the tag
## $3: File to modify
## $4: sed deliminer (optional)
##
function replaceTaggedLine {
  local TMP=$(createTempFile)
  local D='/'
  if [ "$4" != "" ]; then
    D=$4
  fi
  cat "$3" |sed "s${D}.*${1}.*${D}${2}${D}g" > $TMP
  mv "$TMP" "$3"
}

## Removes a group of lines inside a file identified by start and end tags
## $1: Start tag
## $2: End tag
## $3: File
## $4: sed deliminer (optional)
##
function removeLinesBetween {
  local TMP=$(createTempFile)
  local D='/'
  if [ "$4" != "" ]; then
    D=$4
  fi
  cat "$3" |sed "${D}${1}${D},${D}${2}${D}d" > $TMP
  mv "$TMP" "$3"
}

## Replaces all ocurrences of a given tag in a file for another string
## $1: Tag to be searched
## $2: New string that replaces the tag
## $3: File to modify
## $4: sed deliminer (optional)
##
function replaceTag {
  local TMP=$(createTempFile)
  local D='/'
  if [ "$4" != "" ]; then
    D=$4
  fi
  cat "$3" |sed "s${D}${1}${D}${2}${D}g" > $TMP
  mv "$TMP" "$3"
}

## Creates a temporary file reliably, on Linux or Mac OSX, and echoes the file name 
## 
function createTempFile {
  if checkSystem "osx"; then
    mktemp -t tmp.XXXXX
  else
    mktemp 
  fi
}

## Removes trailing blank lines from a file
## $1: filename
##
function removeTrailingBlankLines {
  if checkSystem "osx"; then
    echo "$(echo "$(tail -r "$1")" | tail -r)" > "$1"
  else
    echo "$(echo "$(tac "$1")" | tac)" > "$1"
  fi
}

## Checks if the present system is one of the given in the parameters
## Valid system strings are: "osx", "linux", "cygwin"
## $n: systems to check
##
function checkSystem {
   local SYS=$(uname)
   while (( $# >= 1 )); do
      case "$1" in
         "osx")
            if [[ "$SYS" =~ "Darwin" ]]; then
               return 0
            fi
         ;;
         "win10linux")
            if [[ "$SYS" =~ "Linux" ]]; then
               # Use /proc/version for better compatibilty 
               if grep -qE "(icrosoft|WSL)" /proc/version &> /dev/null; then
                  return 0;
               fi
            fi
         ;;
         "linux") 
            if [[ "$SYS" =~ "Linux" ]] || [[ "$SYS" =~ "GNU" ]]; then
               return 0
            fi
         ;;
         "linux64")
            if [[ "$SYS" =~ "Linux" ]] || [[ "$SYS" =~ "GNU" ]]; then
               SYS=$(uname -m)
               if [[ "$SYS" =~ "64" ]]; then
                  return 0
               fi
            fi
         ;;
         "linux32")
            if [[ "$SYS" =~ "Linux" ]] || [[ "$SYS" =~ "GNU" ]]; then
               SYS=$(uname -m)
               if [[ ! "$SYS" =~ "64" ]]; then
                  return 0
               fi
            fi
         ;;
         "cygwin") 
            if [[ "$SYS" =~ "CYGWIN" ]]; then
               return 0
            fi
         ;;
         "cygwin32") 
            if [[ "$SYS" =~ "CYGWIN" ]]; then
               SYS=$(uname -a)
               if [[ ! "$SYS" =~ "64" ]]; then
                  return 0
               fi
            fi
         ;;
         "cygwin64") 
            if [[ "$SYS" =~ "CYGWIN" ]]; then
               SYS=$(uname -a)
               if [[ "$SYS" =~ "64" ]]; then
                  return 0
               fi
            fi
         ;;
      esac
      shift
   done
   return 1
}

## Echoes a file name without its extension
##  $1: filename
##
function filenameWithNoExt() {
   echo "${1%.*}"
}

## Echoes the bash profile initialization script file name
##
function bashProfileFilename {
   FILES=("$HOME/.bashrc" "$HOME/.bash_profile" "$HOME/.profile")
   for (( i = 0; i < ${#FILES[@]}; i++ )); do
      F="${FILES[$i]}"
	  if isFileReadable "$F"; then
         echo "$F"
         return 0
      fi
   done
   ## Creates bash_profile, by default
   touch "$HOME/.bash_profile"
   echo "$HOME/.bash_profile"
   return 0
}

## Ensures that a Filename has no spaces in it, outputing an
## unrecoverable error message otherwise
##  $1 Filename to check
##  $2 Error message to output
##
function EnsureFilenameHasNoSpaces {
   case "$1" in
      *" "*) Error "$2 ('$1')" 124;; 
   esac
}

## Checks whether a string contains a given substring or character
## Returns 0 if character is contained inside the string, 1 otherwise
##  $1: String to check
##  $2: Substring to be looked for inside $1
##
function containsSubstring {
   local STR="$1"
   local CH="$2"
   if [[ "$STR" == *"$CH"* ]]; then
      return 0
   fi
   return 1   
}

## Checks if a string contains any given character from other string. 
## It echoes the character found if any. Be careful with characters ']' and '\',
## they should be escaped.
##  $1 String to check
##  $2 String with characters to look for
##
function containsChars {
   local STR="$1"
   local CHARS="$2"
   local POS
   ## Look for any character and get its position in STR
   POS=${STR%%[${CHARS}]*};
   POS=${#POS}

   if (( POS < ${#STR} )); then
      echo "${STR:POS:1}"
   fi
}

## Echoes the size of a given file in bytes. 
##  Does not check that file exists or is readable
##  $1 File
##
function fileSize {
   echo $(wc -c < "${1}" | grep -Eo '[0-9]+')
}

## Takes a list of values and outputs them separated by commas
##  $@: Values
##
function valuesToCommaList() {
   local i
   local VALUES=( $@ )
   for (( i=0; i < ${#VALUES[@]}-1; i++ )); do
      printf "%s," ${VALUES[$i]}
   done
   echo "${VALUES[$i]}"
}

## Check if a given value is a valid C identifier or not
##   $1: value to check as being valid C identifier
##
function checkValidCIdentifier() {
   if [[ $1 =~ ^[A-Za-z_]{1}[A-Za-z0-9_]*$ ]] && [[ $(containsChars "$1" "ñ") == "" ]]; then
         return 0
   fi
   return 1
}

## Gets the index of a substring inside a string. Indexes go from 1 onwards.
##   $1: String
##   $2: Search substring
##
## Echoes the index where substring starts, 0 when substring has not been 
## found in the string.
##
function getIndexOf() {
  local STRING="$1"
  local SEARCH="$2"
  local PREV=${STRING%%${SEARCH}*}

  if [ "$PREV" = "$STRING" ]; then
    echo 0
  else
    echo $(( ${#PREV} + 1 ))
  fi
}

## Gets the first token from a string. Token is a group of characters, 
## starting from string character 0, and ending before the first appearance
## of a given SPACE character or string (whitespace, $2).
##   $1: String to extract the token from
##   $2: Whitespace character.
##
## Echoes the token if Whitespace is found, or the complete string otherwise
## 
function getStringToken() {
   local STRING="$1"
   local SPACE="$2"
   local IDX
   
#   IDX=$(expr index "$STRING" "$SPACE"); 
   IDX=$(getIndexOf "$STRING" "$SPACE")
   if (( $IDX != 0 )); then
      echo ${STRING:0:${IDX}-1}
   else
      echo $STRING
   fi   
}

## Checks that a program version is greater than other using version strings
## in the format "M.m.r", being M (Mayor), m (minor) and r (revision) integers
##   $1: Version number string to check. This one has to be greater than $2
##   $2: Version number string used as threshold.
##  Returns TRUE(0) if $1 >= $2,
##          FALSE(1) if $1 < $2,
##          ERROR(-1) if version format string is not correct
## 
function versionGreaterOREqualThan() {
   local V="$1"
   local VTH="$2"
   local V_DIGIT
   local VTH_DIGIT

   ## Check string validity
   if [[ ! $V =~ ^[0-9\.]+$ ]] || [[ ! $VTH =~ ^[0-9\.]+$ ]]; then
      return -1
   fi

   ## Loop checking versions
   while (( ${#V} && ${#VTH} )); do
      ## Extract next Digit from V
      V_DIGIT=$(getStringToken "$V" ".")
      V=${V:${#V_DIGIT}+1}

      ## Extract next Digit from VTH
      VTH_DIGIT=$(getStringToken "$VTH" ".")
      VTH=${VTH:${#VTH_DIGIT}+1}

      ## Check numbers
      if (( "$VTH_DIGIT" > "$V_DIGIT" )); then
         return 1
      elif (( "$VTH_DIGIT" < "$V_DIGIT" )); then
         return 0
      fi
   done

   return 0
}

## Check if GCC Version is grater or equal than a given version value
## Version value is given as a string in the format "M.m.r"
##   $1: Version threshold
## Returns TRUE(0) if GCC Version >= $1, 
##         FALSE(1) if GCC Version < $1, 
##         ERROR(-1) if GCC Version cannot be identified
##
function checkMinimumGCCVersion() {
   local VTH="$1"
   local V=$(gcc -dumpversion)
   if (( $? != 0 )); then
      return -1  ## ERROR: GCC Version undetected
   fi
   if versionGreaterOREqualThan "$V" "$VTH"; then
      return 0   ## TRUE: GCC Version equal or over minimum
   fi
   return 1      ## FALSE: GCC Version below minimum
}

## Launches a Makefile using GNU Make and supervises progress, 
## outputting a Progress Bar. If the building fails, it aborts
## the process launching an error.
##    $1: Directory where Makefile is
##    $2: Log file that will be created about the compilation
##    $3: Parameters for GNU Make (for cleaning, mainly)
##    $4: Expected total number of bytes the log should have at the end (for ProgressBar percentage)
##    $5: Size of the ProgressBar in characters
##    $6: Delay between checks of the status of the process (for supervision)
##    $7: Error message to ouput when compilation fails.
##
function makeWithProgressSupervision {
   local MAKEDIR="$1"
   local MAKELOG="$2"
   local MAKEPARAMS="$3"
   local LOGTOTALBYTES="$4"
   local PBARSIZE="$5"
   local CHECKDELAY="$6"
   local ERRORMSG="$7"

   ## Take into account if we have parameters for passing to GNU Make or not
   if [[ "$MAKEPARAMS" != "" ]]; then
      ( make -C "$MAKEDIR" "$MAKEPARAMS" > "$MAKELOG" 2>&1 ; exit $? ) &
   else 
      ( make -C "$MAKEDIR" > "$MAKELOG" 2>&1 ; exit $? ) &
   fi

   ## Supervise the Making Process
   if ! superviseBackgroundProcess "$!" "$MAKELOG" "$LOGTOTALBYTES" "$PBARSIZE" "$CHECKDELAY"; then
      Error "${ERRORMSG}. Please, check '${MAKELOG}' for details. Aborting. " 123
   fi
   drawOK
}

## Extracts the value of a variable from a given config file. The config file should
## have variables in the format VAR=value, being each variable in a single line.
## If variable is not found in the config file, it issues a non-recoverable error.
##    $1: Name of the variable
##    $2: Config file
##
## Returns value of the variable via echo
##
function extractVarValueFromConfigFile {
   local VARNAME="$1"
   local CFILE="$2"
   local VALUE

   ## Get the value of the variable and check it is not empty
   VALUE=$(cat $CFILE | grep -m 1 "^ *${VARNAME}.*=" | grep -o "=.*$")
   if (( ${#VALUE} == 0 )); then
      Error "Variable '$VARNAME' not found in config file '$CFILE'. This may be due to config \
file not existing or not being readable, or to some kind of data corruption inside this config \
file. Please, check the file ('$CFILE') and verify that variable '$VARNAME' is correctly defined." 11  
   fi

   ## Return the value of the variable by printing it
   echo "${VALUE:1}"
}

## Updates the value of a variable in a config file, by changing the line of the
## file that defines the value of the variable. Variables in the config file should
## be in the format VAR=value, one single variable per line. If it does not find 
## the file or the variable, it issues a non-recoverable error.
##    $1: Name of the variable
##    $2: New value 
##    $3: Config file
##
function updateVarValueInConfigFile() {
   local VARNAME="$1"
   local NEWVALUE="$2"
   local CFILE="$3"
   local LINE
   local TMP

   ## Get the line where variable is located
   LINE=$(cat $CFILE | grep -m 1 -n "^ *${VARNAME}.*=" | grep -o "^[0-9]*")
   if ! isInt $LINE; then
      Error "Variable '$VARNAME' not found in config file '$CFILE'. This may be due to config \
file not existing or not being readable, or to some kind of data corruption inside this config \
file. Please, check the file ('$CFILE') and verify that variable '$VARNAME' is correctly defined." 11  
   fi

   ## Update the line with its new contents
   TMP=$(createTempFile)
   (
      head -$((LINE-1)) "$CFILE"
      echo "${VARNAME}=${NEWVALUE}"
      tail -n +$((LINE+1)) "$CFILE"
   ) > $TMP && mv "$TMP" "$CFILE"
}

## Checks that a given Executable exists and has execution permissions
## If it does not have execution permission, asks the user for setting them.
##  $1: Executable file
##
function checkExecutableExists {
  ERRCPSTR="This file is required for this script to work properly. Please, \
check CPCtelera installation is okay and this file is in its place and has \
required user permissions."
  if   [ ! -e "$1" ]; then
     Error "'$1' does not exist. $ERRCPSTR" 122
  elif [ ! -f "$1" ]; then
     Error "'$1' is not a regular file and it should be. $ERRCPSTR" 122
  elif [ ! -r "$1" ]; then 
     Error "'$1' is not readable. $ERRCPSTR" 122
  elif [ ! -x "$1" ]; then
     echo "${COLOR_LIGHT_YELLOW}WARNING:${COLOR_CYAN}"
     echo "   '${COLOR_WHITE}$1${COLOR_CYAN}' is not executable. Execution \
permission is required for this script to work.${COLOR_LIGHT_CYAN}"
     echo
     askSimpleQuestion y n "Do you want this script to try to make it \
executable? (y/n)" ANSWER
     echo "${COLOR_NORMAL}"
     echo
     if [[ "$ANSWER" == "n" ]]; then
        paramError "'$1' has not been modified. This script cannot continue. Aborting. " 121
     fi
     echo "${COLOR_CYAN}Changing '${COLOR_WHITE}$1${COLOR_CYAN}' execution permission... "
     if ! chmod +x "$1"; then
        Error "Your user has not got enough privileges to change '$1' execution permission. \
Please, change it manually and run this script again." 122
     fi
     echo "${COLOR_LIGHT_GREEN}Success!${COLOR_NORMAL}"
  fi
}

## Ensures that a file of a given type exists and is readable and executable by the user. 
## Otherwise, it throws an unrecoverable error. If the file is readable but not executable,
## the script tries to make it executable with chmod.
##
## $1: Type of file (file, directory)
## $2: File/Directory to check for existance
## $3: Aditional error message, used when file does not exist or is not readable
##
function ensureExistsAndIsExecutable {
   EnsureExists "$1" "$2" "$3"
   ## Check file is executable and, if not, try to make it so
   if [ ! -x "$2" ]; then
     if ! chmod +x "$2"; then
        Error "File '$2' is required to be executable and it is not. This script tried to change \
it to executable, but your user has not got enough privileges to do so. Please, change it manually \
and run this script again." 120
     fi      
   fi
}

## Converts a given string to lowercase
##
## $1: String to convert to lowercase
##
function toLower {
   echo "$1" | tr '[:upper:]' '[:lower:]'
}

## Converts a given string to uppercase
##
## $1: String to convert to uppercase
##
function toUpper {
   echo "$1" | tr '[:lower:]' '[:upper:]'
}

## Gets a number of bytes from a given file, reading the
## file as binary data and outputting the read bytes as an
## ascii-hexadecimal lowercase string
##
## $1: File 
## $2: Number of binary bytes to get
## 
function getFileInitialBytes {
   od -An -t x1 -N "$2" -v "$1" | tr -d " "
}

## Checks if a given file does start by a given binary string data.
## Binary string data must be given in ascii-hexadecimal lowercase form.
## It returns 0 (true) if the file starts by the given binary data, or
## 1 (false) otherwise.
## Warning: Length of $2 string must be even. Otherwise, check will always
## return 1 (false).
##
## $1: File to check
## $2: Starting hexadecimal bytes (text string)
## 
function binaryFileStartsWith {
   local D=$(( ${#2} / 2 ))
   local B=$(getFileInitialBytes "$1" "$D")
   if [ "$2" = "$B" ]; then
      return 0
   fi
   return 1
}