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

## User configurable values
AND_APPNAME := CPCtelera game
AND_SNAFILE := $(if $(SNA),$(SNA),game.sna)
AND_ASSETS  := android/assets
AND_RES     := android/res
AND_APPID   := com.cpctelera

## TODO: Set tools appropriately
## 
AND_OBJDIR        := $(OBJDIR)/_android
AND_OBJAPKDIR     := $(AND_OBJDIR)/apkcontents
AND_OBJAPKDIR_ASS := $(AND_OBJAPKDIR)/assets
AND_OBJAPKDIR_RES := $(AND_OBJAPKDIR)/res
AND_OBJAPKMANIFEST:= $(AND_OBJAPKDIR)/original/AndroidManifest.xml
AND_STRINGRESFILE := $(AND_OBJAPKDIR_RES)/values/strings.xml
AND_OBJPAYLOADSNA := $(AND_OBJAPKDIR_ASS)/payload.sna
AND_APPNAMETAG    := %%%CPCTRVMEngineAppName%%%
AND_APPSPLASHTAG  := %%%CPCTRVMEngineSplash%%%
AND_DEFAULT_APK   := $(ANDROID_PATH)rvmengine/defaultRVMapp.apk
AND_TMPAPK        := $(AND_OBJDIR)/$(APK).tmp
ZIPALIGN          := $(ANDROID_PATH)bin/zipalign.linux32
JARSIGNER         := $(JAVA) -jar $(ANDROID_PATH)bin/jarsigner.jar
APKTOOL           := $(JAVA) -jar $(ANDROID_PATH)bin/apktool/apktool_2.4.0.jar
SET_PKG_ID        := $(JAVA) -jar $(ANDROID_PATH)bin/setAxmlPkgName/setAxmlPkgName.jar

#################
# AND_SET_SNA: Sets the SNA file to be used as payload in 
# the android application
#
# $(1): SNA File to be added
#
define AND_SET_SNA
 # Set SNA file
 $(eval AND_SNAFILE := $(strip $(1)))
endef

#################
# AND_SET_APPID: Sets the APP ID for the android application
#
# $(1): APP ID 
#
define AND_SET_APPID
 # Set APP ID
 $(eval AND_APPID := com.cpctelera.$(strip $(1)))
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
# AND_GENERATE_APK: Commands to generate the APK 
#
define AND_GENERATE_APK
	@# DECODE APK
	$(APKTOOL) decode "$(AND_DEFAULT_APK)" -f -o "$(AND_OBJAPKDIR)"
	@# REPLACE SNA
	$(CP) "$(AND_SNAFILE)" "$(AND_OBJPAYLOADSNA)"
	@# REPLACE ASSETS IF THE USER HAS SUPPLIED WITH SOME
	$(CP) -r "$(AND_ASSETS)"/* "$(AND_OBJAPKDIR_ASS)"
	@# REPLACE RESOURCES IF THE USER HAS SUPPLIED WITH SOME
	$(CP) -r "$(AND_RES)"/* "$(AND_OBJAPKDIR_RES)"
	@# REPLACE APPLICATION NAME
	@# @sed -i -e '/<resources>/,/<\/resources>/ s|<string name="app_name">[0-9a-Z.]\{1,\}</string>|<string name="app_name">$(CUSTOM_APP_NAME)</string>|g' $(AND_OBJDIR)res/values/strings.xml
	$(call REPLACETAG_RT,$(AND_APPNAMETAG),$(AND_APPNAME),$(AND_STRINGRESFILE))
	@# REPLACE APPLICATION ID
	$(SET_PKG_ID) "$(AND_OBJAPKMANIFEST)" "$(AND_APPID)"
	@# BUILD APK
	$(APKTOOL) build "$(AND_OBJAPKDIR)" -o "$(AND_TMPAPK)"
	@# SIGN APK
	$(JARSIGNER) -verbose -sigalg SHA1withRSA -digestalg SHA1 -keystore cert.keystore -storepass android $(AND_TMPAPK) cert
	@# ALIGN APK
	$(ZIPALIGN) -f -p 4 $(AND_TMPAPK) $(AND_TMPAPK)
	$(RM) $(AND_TMPAPK).tmp

	$(call PRINT,$(PROJNAME),"Successfully created 'game.apk'")
endef
