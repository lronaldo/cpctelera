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
##                        CPCTELERA ENGINE                               ##
##               Android Exporter Configuration File                     ##
##-----------------------------------------------------------------------##
## This file holds the configuration of the android APK exporter based   ##
## on RetroVirtualMachine engine embedded emulator. You may change these ##
## values in order to suit your needs when creating an APK for your game ##
## to be played on Android devices. 												 ##
###########################################################################

## APP GLOBAL CONFIGURATION
##
## - UNIQUE ID
##   	Your application must have a __unique ID__ that identifies it on Android. This 
## ID is generated when your CPCTelera project is created. You may change this ID
## but take into account that it identifies your application. If you have already used
## previous ID on any Android device or in the Google Play Store, changing the ID will
## make new compilations of your game to be seen as a different application.
##
##	- NAME
##	  	This is just the name for your application. A text string that will be displayed
## to the user showing what your application is. 
##
## - CERTIFICATE
##		You need a valid personal certificate to sign your application before uploading
## it to any Android device or the Google Play Store. You may create your certificate using
## cpct_android_cert. 
##
CUSTOM_APP_NAME   := My Custom Name
CUSTOM_APP_ID     := org.cpctelera.customid
CUSTOM_APP_CERT   := cert.keystore

## TODO: Set tools appropriately
AND_OBJDIR   := $(OBJDIR)/_android
ANDROID_PATH := $(CPCT_PATH)/tools/android
DEFAULT_APK  := $(ANDROID_PATH)/rvmengine/defaultRVMapp.apk
ZIPALIGN     := $(ANDROID_PATH)/bin/zipalign.linux32
JARSIGNER    := $(ANDROID_PATH)/bin/jarsigner.linux64
JAVA         := java
APKTOOL      := $(JAVA) -jar $(ANDROID_PATH)/bin/apktool/apktool_2.4.0.jar
SET_PKG_ID   := java -jar $(ANDROID_PATH)/bin/setAxmlPkgName/setAxmlPkgName.jar
TMP_APK      := $(APK).tmp

$(APK): $(SNA)
	# DECODE APK
	@$(APKTOOL) decode $(DEFAULT_APK) -f -o $(AND_OBJDIR)
	# REPLACE ASSETS
	@$(CP) $(SNA) $(AND_OBJDIR)/assets/payload.sna
	#@$(CP) -R $(AND_ASSETS)* $(AND_OBJDIR)
	# REPLACE APPLICATION NAME
	@sed -i -e '/<resources>/,/<\/resources>/ s|<string name="app_name">[0-9a-Z.]\{1,\}</string>|<string name="app_name">$(CUSTOM_APP_NAME)</string>|g' $(AND_OBJDIR)res/values/strings.xml
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
