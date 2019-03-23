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

## TODO: Set tools appropriately
## 
AND_OBJDIR        := $(OBJDIR)/_android
AND_OBJAPKDIR     := $(AND_OBJDIR)/apkcontents
AND_OBJAPKDIR_ASS := $(AND_OBJDIR)/apkcontents/assets
AND_OBJAPKDIR_RES := $(AND_OBJDIR)/apkcontents/res
AND_STRINGRESFILE := $(AND_OBJDIR)/res/values/strings.xml
AND_OBJPAYLOADSNA := $(AND_OBJAPKDIR)/assets/payload.sna
AND_APPNAMETAG    := %%%CPCTRVMEngineAppName%%%
AND_APPSPLASHTAG  := %%%CPCTRVMEngineSplash%%%
AND_DEFAULT_APK   := $(ANDROID_PATH)/rvmengine/defaultRVMapp.apk
ANDROID_PATH      := $(CPCT_PATH)/tools/android
ZIPALIGN          := $(ANDROID_PATH)/bin/zipalign.linux32
JARSIGNER         := $(ANDROID_PATH)/bin/jarsigner.linux64
APKTOOL           := $(JAVA) -jar $(ANDROID_PATH)/bin/apktool/apktool_2.4.0.jar
SET_PKG_ID        := $(JAVA) -jar $(ANDROID_PATH)/bin/setAxmlPkgName/setAxmlPkgName.jar
TMP_APK           := $(AND_OBJDIR)/$(APK).tmp

#################
# ADD_SNA_AND_ASSETS2APK: Decodes the default APK to a folder
# in the AND_OBJDIR folder of the project and replaces strings, SNA and
# assets before packing it again into a new APK ready to be used.
# (It will require to be signed and ID-replaced, but for the rest, it
# is ready to be used)
#
# $(1): APK File to generate with new assets and SNA
# $(2): SNA File to be added
# $(3): New application name to be used
# $(4): Assets folder to be copied to the APK
# $(5): Resources folder to be copied to the APK
#
define ADD_SNA_AND_ASSETS2APK
# Strip parameters
	$(eval _ASA_APK   := $(strip $(1)))
	$(eval _ASA_SNA   := $(strip $(2)))
	$(eval _ASA_NAME  := $(strip $(3)))
	$(eval _ASA_ASSETS:= $(strip $(4)))
	$(eval _ASA_RES   := $(strip $(5)))
# Ensure SNA and Default APK Exist
	$(call ENSUREFILEEXISTS,$(AND_DEFAULT_APK),[ ANDROID - ADD_SNA_AND_ASSETS2APK ]: <<CRITICAL-ERROR>> File '$(AND_DEFAULT_APK)' does not exist or cannot be read when trying to add it to '$(_ASA_APK)'. '$(AND_DEFAULT_APK)' is installed along with CPCtelera, so this means that your installation folder/files have been modified. Please check for this file or reinstall CPCtelera.)
	$(call ENSUREFILEEXISTS,$(_ASA_SNA),[ ANDROID - ADD_SNA_AND_ASSETS2APK ]: <<ERROR>> File '$(_ASA_SNA)' does not exist or cannot be read when trying to add it to '$(_ASA_APK)'. '$(_ASA_SNA)' should have been generated when compiling your CPCtelera project. Please review your project configuration and generate it again.)
# DECODE APK
	@$(APKTOOL) decode $(AND_DEFAULT_APK) -f -o $(AND_OBJAPKDIR)
# REPLACE SNA
	$(call ENSUREFILEEXISTS,$(AND_OBJPAYLOADSNA),[ ANDROID - ADD_SNA_AND_ASSETS2APK ]: <<ERROR>> File '$(AND_OBJPAYLOADSNA)' was not found after decompressing '$(AND_DEFAULT_APK)'. Please review that '$(AND_DEFAULT_APK)' was completely decompressed into '$(AND_OBJAPKDIR)', there are no filesystem permission problems and enough space for decompression.)
	@$(CP) "$(_ASA_SNA)" "$(AND_OBJPAYLOADSNA)"
# REPLACE ASSETS IF THE USER HAS SUPPLIED WITH SOME
	$(call ENSUREFILEEXISTS,$(AND_OBJAPKDIR_ASS),[ ANDROID - ADD_SNA_AND_ASSETS2APK ]: <<ERROR>> Folder '$(AND_OBJAPKDIR_ASS)' was not found after decompressing '$(AND_DEFAULT_APK)'. Please review that '$(AND_DEFAULT_APK)' was completely decompressed into '$(AND_OBJAPKDIR)', there are no filesystem permission problems and enough space for decompression.)
	$(if $(call FOLDERISREADABLE,$(_ASA_ASSETS))\
		,@$(CP) -R $(_ASA_ASSETS)/* $(AND_OBJAPKDIR_ASS)\
		,$(call WARNING, ADD_SNA_AND_ASSETS2APK did not found your assets folder '$(_ASA_ASSETS)'. It will not be copied to the APK file '$(_ASA_APK)')\
	)
# REPLACE RESOURCES IF THE USER HAS SUPPLIED WITH SOME
	$(call ENSUREFILEEXISTS,$(AND_OBJAPKDIR_RES),[ ANDROID - ADD_SNA_AND_ASSETS2APK ]: <<ERROR>> Folder '$(AND_OBJAPKDIR_RES)' was not found after decompressing '$(AND_DEFAULT_APK)'. Please review that '$(AND_DEFAULT_APK)' was completely decompressed into '$(AND_OBJAPKDIR)', there are no filesystem permission problems and enough space for decompression.)
	$(if $(call FOLDERISREADABLE,$(_ASA_RES))\
		,@$(CP) -R $(_ASA_RES)/* $(AND_OBJAPKDIR_RES)\
		,$(call WARNING, ADD_SNA_AND_ASSETS2APK did not found your resources folder '$(_ASA_RES)'. It will not be copied to the APK file '$(_ASA_APK)')\
	)
# REPLACE APPLICATION NAME
# @sed -i -e '/<resources>/,/<\/resources>/ s|<string name="app_name">[0-9a-Z.]\{1,\}</string>|<string name="app_name">$(CUSTOM_APP_NAME)</string>|g' $(AND_OBJDIR)res/values/strings.xml
	$(call REPLACETAG,$(AND_APPNAMETAG),$(_ASA_NAME),$(AND_STRINGRESFILE))
endef

#################
# ADD_FILE_TO_CDT_LIST: Adds a file to the list of files
# that will be included in the final CDT to be produced
# at the end of the building process. 
#
# $(1): Fil
# $(2): Type {firmware, basic, miniload}
# $(3): Load Address (for firmware calls)
# $(4): Run Address (for firmware calls)
# $(5): Filename (for firmware calls)
#
define GENERATE_APK
$(APK): $(SNA)
	# BUILD APK
	@$(APKTOOL) build $(AND_OBJDIR) -o $(TMP_APK)
	# REPLACE APPLICATION ID
	@$(APKRENAME) $(TMP_APK) $(CUSTOM_APP_ID)
	@$(RM) -rf tmpForApkRename
	# SIGN APK
	@$(JARSIGNER) -verbose -sigalg SHA1withRSA -digestalg SHA1 -keystore cert.keystore -storepass android $(TMP_APK) cert
	# ALIGN APK
	@mv $(TMP_APK) $(TMP_APK).tmp
	@$(ZIPALIGN) -f -p 4 $(TMP_APK).tmp $(TMP_APK)
	@$(RM) $(TMP_APK).tmp

	@$(call PRINT,$(PROJNAME),"Successfully created 'game.apk'")
endef
