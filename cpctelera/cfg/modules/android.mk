##-----------------------------LICENSE NOTICE------------------------------------
##  This file is part of CPCtelera: An Amstrad CPC Game Engine 
##  Copyright (C) 2019 José Luís Luri
##  Copyright (C) 2019 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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
AND_OBJAPKDIR     := $(AND_OBJDIR)/apkcontents
AND_OBJAPKDIR_ASS := $(AND_OBJAPKDIR)/assets
AND_OBJAPKDIR_RES := $(AND_OBJAPKDIR)/res
AND_OBJAPKMANIFEST:= $(AND_OBJAPKDIR)/AndroidManifest.xml
AND_STRINGRESFILE := $(AND_OBJAPKDIR_RES)/values/strings.xml
AND_OBJPAYLOADSNA := $(AND_OBJAPKDIR_ASS)/payload.sna
AND_APPIDTAG      := ___CPCT.RVMEngine.AppID___
AND_APPNAMETAG    := %%%CPCTRVMEngineAppName%%%
AND_APPSPLASHTAG  := %%%CPCTRVMEngineSplash%%%
AND_DEFAULT_APK   := $(ANDROID_PATH)rvmengine/defaultRVMapp.apk
AND_KEYSTORE_DIR  := $(ANDROID_PATH)certs
AND_TMPAPK        := $(AND_OBJDIR)/$(AND_APK).tmp
ZIPALIGN          := $(ANDROID_PATH)bin/zipalign/linux/zipalign32
KEYTOOL				:= $(JAVA) -jar $(ANDROID_PATH)bin/sun/keytool.jar
JARSIGNER         := $(JAVA) -jar $(ANDROID_PATH)bin/sun/jarsigner.jar
APKTOOL           := $(JAVA) -jar $(ANDROID_PATH)bin/apktool/apktool_2.4.0.jar

## User configurable values
AND_APK     := game.apk
AND_APPNAME := CPCtelera game
AND_SNAFILE := $(if $(SNA),$(SNA),game.sna)
AND_ASSETS  := android/assets
AND_RES     := android/res
AND_APPID   := $(AND_APPID_PREFIX).game
AND_KEYSTORE:= $(AND_KEYSTORE_DIR)/cpctelera.user.keystore.jks
AND_KEYALIAS:= cpctelera.user.cert
AND_KEYVALID:= 10000
AND_KEYSIZE := 2048
AND_KEYALG  := RSA

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
	$(KEYTOOL) -genkeypair -keystore "$(AND_KEYSTORE)" -keyalg $(AND_KEYALG) -keysize $(AND_KEYSIZE) -validity $(AND_KEYVALID) -alias $(AND_KEYALIAS); \
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
 	@if [ ! -f "$(AND_KEYSTORE)" ]; then\
	 	echo "IMPORTANT: No user private certificate configured.";\
	 	echo "   - A user private certificate is required to sign apk files. Only signed files can be installed on Android platforms or uploaded to Google Play Store. Please provide information to generate your own personal Android signing certificate.";\
	 	echo "   - REMEMBER:";\
	 	echo "       * Your INFORMATION must be accurate. Otherwise you may have problems when uploading your APKs to any online store.";\
	 	echo "       * Pick up a PASSWORD you remember. If you forget your password you will need to create a new certificate, losing control of previously signed and uploaded applications.":\
	 	echo "       * SAVE your generated SIGNING KEY. You may found it in '$(AND_KEYSTORE)'.";\
	 	echo "";\
 		$(call AND_GEN_USER_CERT);\
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
.PHONY: $(_AND_RULENAME)
$(_AND_RULENAME): $(AND_SNAFILE)
	@# Anounce generation of APK
	@$(call PRINT,$(PROJNAME),"Started generation of Android Package '$(AND_APK)'")
	
	@# CHECK USER CERTIFICATE BEFORE STARTING
	$(call AND_CHECK_USER_CERT)
	@# DECODE APK
	$(APKTOOL) decode "$(AND_DEFAULT_APK)" -f -o "$(AND_OBJAPKDIR)"
	@# REPLACE SNA
	$(CP) "$(AND_SNAFILE)" "$(AND_OBJPAYLOADSNA)"
	@# REPLACE ASSETS IF THE USER HAS SUPPLIED WITH SOME
	$(CP) -r "$(AND_ASSETS)"/* "$(AND_OBJAPKDIR_ASS)"
	@# REPLACE RESOURCES IF THE USER HAS SUPPLIED WITH SOME
	$(CP) -r "$(AND_RES)"/* "$(AND_OBJAPKDIR_RES)"
	@# REPLACE APPLICATION NAME
	$(call REPLACETAG_RT,$(AND_APPNAMETAG),$(AND_APPNAME),$(AND_STRINGRESFILE))
	@# REPLACE APPLICATION ID
	$(call REPLACETAG_RT,$(AND_APPIDTAG),$(AND_APPID),$(AND_OBJAPKMANIFEST))
	@# BUILD APK
	$(APKTOOL) build "$(AND_OBJAPKDIR)" -o "$(AND_TMPAPK)"
	@# SIGN APK
	$(JARSIGNER) -verbose -sigalg SHA1withRSA -digestalg SHA1 -keystore "$(AND_KEYSTORE)" "$(AND_TMPAPK)" $(AND_KEYALIAS)
	@# ALIGN APK
	$(ZIPALIGN) -f -v 4 "$(AND_TMPAPK)" "$(AND_APK)"
	$(RM) "./$(AND_TMPAPK)"

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
