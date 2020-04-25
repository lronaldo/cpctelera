##-----------------------------LICENSE NOTICE------------------------------------
##  This file is part of CPCtelera: An Amstrad CPC Game Engine 
##  Copyright (C) 2019 José Luís Luri
##  Copyright (C) 2020 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
##             Android Exporter General Functionalities                  ##
##-----------------------------------------------------------------------##
## This file contains all macros and global configuration related to the ## 
## Android exporter. This automates the creation of distributable apks   ##
## with an snapshot of the game and the RVMengine embedded for Android.  ##
###########################################################################

## General functionalities configuration
##
AND_APPID_PREFIX  := com.cpctelera.rvmengine
AND_OBJDIR        := $(OBJDIR)/_android
AND_OBJLOG        := $(AND_OBJDIR)/apk-generation.log
AND_OBJAPKDIR     := $(AND_OBJDIR)/apkcontents
AND_OBJAPKDIR_ASS := $(AND_OBJAPKDIR)/assets
AND_OBJAPKDIR_RES := $(AND_OBJAPKDIR)/res
AND_OBJAPKMANIFEST:= $(AND_OBJAPKDIR)/AndroidManifest.xml
AND_STRINGRESFILE := $(AND_OBJAPKDIR_RES)/values/strings.xml
AND_OBJPAYLOADSNA := $(AND_OBJAPKDIR_ASS)/payload.sna
AND_APPIDTAG      := XXXCPCT.RVMEngine.AppIDXXX
AND_APPNAMETAG    := %%%CPCTRVMEngineAppName%%%
AND_APPSPLASHTAG  := %%%CPCTRVMEngineSplash%%%
AND_DEFAULT_APK   := $(call SYSPATH,$(ANDROID_PATH)rvmengine/defaultRVMapp.apk)
AND_KEYSTORE_DIR  := $(ANDROID_PATH)certs
AND_TMPAPK        := $(AND_OBJDIR)/$(AND_APK).tmp
ZIPALIGN_LINUX    := $(ANDROID_PATH)bin/zipalign/linux/zipalign32
ZIPALIGN_WIN      := $(ANDROID_PATH)bin/zipalign/win/32/zipalign.exe
ZIPALIGN          := $(if $(call CHECKSYSTEMCYGWIN),$(ZIPALIGN_WIN),$(ZIPALIGN_LINUX))
KEYTOOL_JAR       := $(call SYSPATH,$(ANDROID_PATH)bin/sun/keytool.jar)
JARSIGNER_JAR     := $(call SYSPATH,$(ANDROID_PATH)bin/sun/jarsigner.jar)
APKTOOL_JAR       := $(call SYSPATH,$(ANDROID_PATH)bin/apktool/apktool_2.4.0.jar)
KEYTOOL           := $(JAVA) -jar "$(KEYTOOL_JAR)"
JARSIGNER         := $(JAVA) -jar "$(JARSIGNER_JAR)"
APKTOOL           := $(JAVA) -jar "$(APKTOOL_JAR)"

## User configurable values
AND_APK      := game.apk
AND_APPNAME  := CPCtelera game
AND_SNAFILE  := $(if $(SNA),$(SNA),)
AND_ASSETS   := android/assets
AND_RES      := android/res
AND_APPID    := $(AND_APPID_PREFIX).game
AND_KEYSTORE := $(call SYSPATH,$(AND_KEYSTORE_DIR)/cpctelera.user.keystore.jks)
AND_KEYALIAS := cpctelera.user.cert
AND_KEYVALID := 10000
AND_KEYSIZE  := 2048
AND_KEYALG   := RSA
AND_STORETYPE:= pkcs12

## JAVA Version
AND_JAVA_VER     := $(shell java -version 2>&1 | sed -n ';s/.* version "\(.*\)\.\(.*\)\.\(.*\)_.*"/\1\2\3/p;')
AND_JAVA_UPD     := $(shell java -version 2>&1 | sed -n ';s/.* version ".*\..*\..*_\(.*\)"/\1/p;')
AND_MIN_JAVA_VER := 180
AND_MIN_JAVA_UPD := 101

#################
# AND_SET_SNA: Sets the SNA file to be used as payload in 
# the android application. It also sets the name of the resulting
# APK file, just by changing snafile extension
#
# $(1): SNA File to be added
#
define AND_SET_SNA
 # Set SNA file
 $(eval AND_SNAFILE := $(strip $(1)))
 $(eval AND_APK     := $(basename $(AND_SNAFILE)).apk)
endef

#################
# AND_SET_APPID: Sets the APP ID for the android application
#
# $(1): APP ID 
#
define AND_SET_APPID
 # Set APP ID
 $(eval AND_APPID := $(AND_APPID_PREFIX).$(strip $(1)))
endef

#################
# AND_SET_ASSET_FOLDERS: Sets the paths to the folders with 
# android assets required
#
# $(1): Assets folder to be copied to the APK
# $(2): Resources folder to be copied to the APK
#
define AND_SET_ASSET_FOLDERS
 # Set folders and check for their existence
 $(eval AND_ASSETS := $(strip $(1)))
 $(eval AND_RES    := $(strip $(2)))
 $(call ENSUREFOLDERISREADABLE,$(AND_ASSETS),[ AND_SET_ASSET_FOLDERS ]: <<ERROR>> Folder '$(AND_ASSETS)' does not exist or is not readable. '$(AND_ASSETS)' will be required to generate the android APK file)
 $(call ENSUREFOLDERISREADABLE,$(AND_RES),[ AND_SET_ASSET_FOLDERS ]: <<ERROR>> Folder '$(AND_RES)' does not exist or is not readable. '$(AND_RES)' will be required to generate the android APK file)
endef

#################
# AND_SET_APPNAME: Sets the name for the Android Application
#
# $(1): Application name
#
define AND_SET_APPNAME
 # Set the name
 $(eval AND_APPNAME := $(strip $(1)))
endef

#################
# AND_GEN_USER_CERT: Generates a new user certificate and configures
# it to be the user default certificate.
#
define AND_GEN_USER_CERT
	$(KEYTOOL) -genkeypair -keystore "$(AND_KEYSTORE)" -storetype "$(AND_STORETYPE)" -keyalg $(AND_KEYALG) -keysize $(AND_KEYSIZE) -validity $(AND_KEYVALID) -alias $(AND_KEYALIAS); \
	if [ ! -f "$(AND_KEYSTORE)" ]; then\
		echo "<<<ERROR>>> Could not generate certificate keystore '$(AND_KEYSTORE)'. Please read error messages and try again.";\
		exit 1;\
	fi
endef

#################
# AND_CHECK_USER_CERT: Checks that there is a valid user certificate file 
# selected. Otherwise, launches AND_GEN_USER_CERT to generate a valid
# user certificate file
#
define AND_CHECK_USER_CERT
 	@if [ ! -f "$(AND_KEYSTORE)" ]; then                                                             \
		printf "$(COLOR_WHITE)"                                                                      ;\
		printf "\n==================================================================================";\
		printf "\n$(COLOR_RED)IMPORTANT:$(COLOR_YELLOW) No user private certificate configured."     ;\
	 	printf "$(COLOR_WHITE)"                                                                      ;\
		printf "\n==================================================================================";\
	 	printf "$(COLOR_CYAN)"                                                                       ;\
	 	printf "\n    A user private certificate is REQUIRED to sign apk files. Only signed files"   ;\
	 	printf " can be installed on Android platforms or uploaded to Google Play Store. Please "    ;\
	 	printf "provide information to generate your own personal Android signing certificate. "     ;\
		printf "Next steps will create a personal self-signed certificate that you will be able "    ;\
		printf "to use to sign you APK files automatically. "                                        ;\
		printf "\n\n    If you prefer, you may use any previous certificate. You need to import it " ;\
		printf "into a JKS key store (you may use keytool for that). Then, just copy your JKS key"   ;\
		printf "store to '$(COLOR_NORMAL)$(AND_KEYSTORE)$(COLOR_CYAN)' and CPCtelera will use it to ";\
	 	printf "sign your APKs."                                                                     ;\
		printf "\n\n$(COLOR_RED)   - REMEMBER:$(COLOR_CYAN)"                                         ;\
	 	printf "\n       1. Your INFORMATION must be accurate. Otherwise you may have problems when ";\
	 	printf "uploading your APKs to any online store."                                            ;\
	 	printf "\n       2. Pick up a PASSWORD you remember. If you forget your password you will "  ;\
	 	printf "need to create a new certificate, losing control of previously signed and uploaded"  ;\
	 	printf " applications."                                                                      ;\
	 	printf "\n       3. $(COLOR_YELLOW)BACKUP$(COLOR_CYAN) your generated $(COLOR_YELLOW)SIGNING";\
		printf " KEY$(COLOR_CYAN). You may found it in '$(COLOR_NORMAL)$(AND_KEYSTORE)$(COLOR_CYAN)'.";\
	 	printf "$(COLOR_WHITE)"                                                                      ;\
		printf "\n==================================================================================";\
	 	printf "\n\n$(COLOR_NORMAL)"                                                                 ;\
 		$(call AND_GEN_USER_CERT)                                                                    ;\
 	fi
endef

#################
# AND_CHECK_SNA: Checks that a valid SNA file has been generated and
# is ready to be used for APK generation.
#
define AND_CHECK_SNA
	@if [ ! -f "$(AND_SNAFILE)" ]; then                                                             \
		printf "\n$(COLOR_RED)ERROR:$(COLOR_YELLOW) SNA file '$(AND_SNAFILE)' not found."            ;\
		printf "$(COLOR_CYAN)"                                                                       ;\
		printf "\n    An SNA file is required to generate Android APK package. This file is included";\
		printf " within the package and launched when the user starts the APK. Check your project "  ;\
		printf "configuration and be sure to generate a SNA file before generating APK."             ;\
		printf "\n\n$(COLOR_NORMAL)"                                                                 ;\
		exit 2                                                                                       ;\
	fi
endef

#################
# AND_CHECK_JAVA_VERSION: Checks that the appropriate java version is installed
# before continuing. If it is not installed, it warns the user.
#
define AND_CHECK_JAVA_VERSION
	@__ERRORMSG="Java Runtime Enviroment (JRE) 1.8.0 update 101 (u101) or higher is required to produce Android\
 APK files. Please, install JRE 1.8.0_101 or higher in your system and the launch APK generation again. You may\
find latest java runtime available for download at https://www.java.com/"                                    ;\
    java &> /dev/null                                                                                        ;\
    if (( $$$$? != 127 )); then                                                                               \
        if (( $(AND_JAVA_VER) < $(AND_MIN_JAVA_VER) ||                                                        \
             ( $(AND_JAVA_VER) == $(AND_MIN_JAVA_VER) && $(AND_JAVA_UPD) < $(AND_MIN_JAVA_UPD) ) )); then     \
            echo   "[[ ERROR ]]: Your java version is too old. "                                             ;\
            echo   "             Installed: [ VERSION=$(AND_MIN_JAVA_VER) UPDATE=$(AND_JAVA_UPD) ] "         ;\
            echo   "$$$${__ERRORMSG}"                                                                        ;\
            exit 1                                                                                           ;\
        fi                                                                                                   ;\
    else                                                                                                      \
        echo   "[[ ERROR ]]: Java is not installed in your system. "                                         ;\
        echo   "$$$${__ERRORMSG}"                                                                            ;\
        exit 1                                                                                               ;\
	fi
endef

#################
# AND_GENERATE_APK_RULE: Generates a rule mean to be launched when wanting to 
# generate the apk file. The rule will depend on the Snapshot file configured,
# launching SNA regeneration when required
#
# $(1): Rule Name
#
define AND_GENERATE_APK_RULE
$(eval _AND_RULENAME := $(strip $(1)))
$(eval AND_APK     := $(basename $(AND_SNAFILE)).apk)
.PHONY: $(_AND_RULENAME)
$(_AND_RULENAME): $(AND_SNAFILE)
	@# Anounce generation of APK
	@$(call PRINT,$(PROJNAME),"Started generation of Android Package '$(AND_APK)'")
	@printf " On errors consult logfile '$(AND_OBJLOG)'.\n"
	
	@# CHECK JAVA INSTALLATION AND USER CERTIFICATE BEFORE STARTING
	@printf "$(COLOR_CYAN) (1/4): Checking SNA, Java Version and User Certificate...$(COLOR_NORMAL)"
	$(call AND_CHECK_SNA)
	$(call AND_CHECK_JAVA_VERSION)
	$(call AND_CHECK_USER_CERT)
	@printf "$(COLOR_GREEN)OK$(COLOR_NORMAL)\n"

	@# DECODE APK
	@printf "$(COLOR_CYAN) (2/4): Generating APK...\n$(COLOR_NORMAL)"
	@printf "  -a: Decoding..."
	@date &> $(AND_OBJLOG)
	@$(MKDIR) $(AND_OBJDIR)
	@printf "\n[[COMMAND]]: $(APKTOOL) decode $(AND_DEFAULT_APK) -f -o $(AND_OBJAPKDIR)\n\n" &>> $(AND_OBJLOG)
	@$(APKTOOL) decode "$(AND_DEFAULT_APK)" -f -o "$(AND_OBJAPKDIR)" &>> $(AND_OBJLOG)
	@printf "$(COLOR_GREEN)OK$(COLOR_NORMAL)\n"
	@# REPLACE SNA
	@printf "  -b: Injecting assets, icons and content..."
	@printf "\n[[COMMAND]]: $(CP) $(AND_SNAFILE) $(AND_OBJPAYLOADSNA)\n\n" &>> $(AND_OBJLOG)
	@$(CP) "$(AND_SNAFILE)" "$(AND_OBJPAYLOADSNA)" &>> $(AND_OBJLOG)
	@# REPLACE ASSETS IF THE USER HAS SUPPLIED WITH SOME
	@printf "\n[[COMMAND]]: $(CP) -r $(AND_ASSETS)/* $(AND_OBJAPKDIR_ASS)\n\n" &>> $(AND_OBJLOG)
	@$(CP) -r "$(AND_ASSETS)"/* "$(AND_OBJAPKDIR_ASS)" &>> $(AND_OBJLOG)
	@# REPLACE RESOURCES IF THE USER HAS SUPPLIED WITH SOME
	@printf "\n[[COMMAND]]: $(CP) -r $(AND_RES)/* $(AND_OBJAPKDIR_RES)\n\n" &>> $(AND_OBJLOG)
	@$(CP) -r "$(AND_RES)"/* "$(AND_OBJAPKDIR_RES)" &>> $(AND_OBJLOG)
	@# REPLACE APPLICATION NAME
	@printf "\n[[COMMAND]]: Replace tags\n\n" &>> $(AND_OBJLOG)
	@$(call REPLACETAG_RT,$(AND_APPNAMETAG),$(AND_APPNAME),$(AND_STRINGRESFILE)) &>> $(AND_OBJLOG)
	@# REPLACE APPLICATION ID
	@$(call REPLACETAG_RT,$(AND_APPIDTAG),$(AND_APPID),$(AND_OBJAPKMANIFEST)) &>> $(AND_OBJLOG)
	@printf "$(COLOR_GREEN)OK$(COLOR_NORMAL)\n"
	@# BUILD APK
	@printf "  -c: Rebuilding..."
	@printf "\n[[COMMAND]]: $(APKTOOL) build $(AND_OBJAPKDIR) -o $(AND_TMPAPK)\n\n" &>> $(AND_OBJLOG)
	@$(APKTOOL) build "$(AND_OBJAPKDIR)" -o "$(AND_TMPAPK)" &>> $(AND_OBJLOG)
	@printf "$(COLOR_GREEN)OK$(COLOR_NORMAL)\n"
	@printf "\n[[COMMAND]]: $(RM) -r \"./$(AND_OBJAPKDIR)\"\n\n" &>> $(AND_OBJLOG)
	@$(RM) -r "./$(AND_OBJAPKDIR)" &>> $(AND_OBJLOG)
	@# SIGN APK
	@printf "$(COLOR_CYAN) (3/4): Signing APK...\n$(COLOR_NORMAL)"
	@printf "\n[[COMMAND]]: $(JARSIGNER) -verbose -sigalg SHA1withRSA -digestalg SHA1 -keystore $(AND_KEYSTORE) $(AND_TMPAPK) $(AND_KEYALIAS)\n\n" &>> $(AND_OBJLOG)
	@$(JARSIGNER) -verbose -sigalg SHA1withRSA -digestalg SHA1 -keystore "$(AND_KEYSTORE)" "$(AND_TMPAPK)" $(AND_KEYALIAS)
	@# ALIGN APK
	@printf "$(COLOR_CYAN) (4/4): Aligning...$(COLOR_NORMAL)"
	@printf "\n[[COMMAND]]: $(ZIPALIGN) -f -v 4 $(AND_TMPAPK) $(AND_APK)\n\n" &>> $(AND_OBJLOG)
	@$(ZIPALIGN) -f -v 4 "$(AND_TMPAPK)" "$(AND_APK)" &>> $(AND_OBJLOG)
	@printf "\n[[COMMAND]]: $(RM) ./$(AND_TMPAPK)\n\n" &>> $(AND_OBJLOG)
	@$(RM) "./$(AND_TMPAPK)" &>> $(AND_OBJLOG)
	@printf "$(COLOR_GREEN)OK$(COLOR_NORMAL)\n"

	@# APK Successfully generated
	$(call PRINT,$(PROJNAME),"Successfully created '$(AND_APK)'")

endef

#################
# APKMAN: Front-end to access all the functionalities on the android apk generator
#
# $(1): Command to be performed
# $(2-3): Valid arguments to be passed to the selected command
#
# Valid Commands: SET_SNA SET_APPID SET_APPNAME SET_ASSET_FOLDERS GENERATE_APK_RULE
# Info about each command can be found looking into its correspondent makefile macro APKGEN_<COMMAND>
#
define APKMAN
 # Set the list of valid commands
 $(eval APKGEN_F_FUNCTIONS := SET_SNA SET_APPID SET_APPNAME SET_ASSET_FOLDERS GENERATE_APK_RULE)

 # Check that command parameter ($(1)) is exactly one-word after stripping whitespaces
 $(call ENSURE_SINGLE_VALUE,$(1),<<ERROR>> [APKGEN] '$(strip $(1))' is not a valid command. Commands must be exactly one-word in lenght with no whitespaces. Valid commands: {$(APKGEN_F_FUNCTIONS)})

 # Filter given command as $(1) to see if it is one of the valid commands
 $(eval APKGEN_F_SF = $(filter $(APKGEN_F_FUNCTIONS),$(1)))

 # If the given command is valid, it will be non-empty, then we proceed to call the command (up to 8 arguments). Otherwise, raise an error
 $(if $(APKGEN_F_SF)\
    ,$(eval $(call AND_$(APKGEN_F_SF),$(strip $(2)),$(strip $(3))))\
    ,$(error <<ERROR>> [APKGEN] '$(strip $(1))' is not a valid command. Valid commands: {$(APKGEN_F_FUNCTIONS)}))
endef
