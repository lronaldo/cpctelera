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
## to be played on Android devices.                                      ##
###########################################################################

## ANDROID APK EXPORTER CONFIGURATION

# Android Application ID. This ID must be unique for every application uploaded to Google Play
# or to any mobile phone. Please, configure your ID once and do not change it afterwards. Otherwise,
# newer versions of your game will be considered a different APP. You may use here any 
# combination of letters, numbers, '.' and '_'
GAME_ID := $(basename $(SNA))
$(eval $(call APKMAN, SET_APPID         , $(GAME_ID) ))

# This is the name that your mobile phone will display for your game in the installed APPs. You may
# use any string you wanted, and you may change it whenever you want. This does not affect to how
# Google Play or your mobile phone recognize your APP. It is only the string displayed as APP Name.
$(eval $(call APKMAN, SET_APPNAME       , $(GAME_ID) ))

# This is the actual version number of your application. It must be an integer value and you MUST
# increment it whenever you wanted to upload a new version to the market store. Applications on devices
# will not let you overwrite their installed version with previous versions, according to this value.
$(eval $(call APKMAN, SET_VERSIONCODE   , 1 ))

# This is the name of your current version that will be shown in the properties of your application
# on your mobile platform. It is a string that can be anything. The actual version is controlled by 
# your VersionCode, not by this string. You may use this value as information for your users.
$(eval $(call APKMAN, SET_VERSIONNAME   , 1.0-beta ))

# These are the folders were you store the assets that will be used to produce your APP. 
#  - assets folder must contain the file 'conf.json' with the configuration of the emulation and keymap
#  - res folder must contain all the resources (icons, splashes, etc) in their different resolutions
# These two folder are created along with your CPCtelera project and contain the standard required files.
# You may modify those files to suit your needs, or move them in your folder structure, then point
# here to them both.
$(eval $(call APKMAN, SET_ASSET_FOLDERS , exp/android/assets, exp/android/res))

# Finally, this call generates the makefile rule you will use to produce your APK file whenever you
# wanted. You give here the name of the rule. You will use this name as a parameter to 'make' to 
# produce your APK. For instance, calling it 'apk', the command 'make apk' produces your APK.
$(eval $(call APKMAN, GENERATE_APK_RULE , apk ))


## OTHER IMPORTANT DETAILS
##
## - CERTIFICATE
##    You need a valid personal certificate to sign your application before uploading
## it to any Android device or the Google Play Store. The first time you use 'make apk',
## CPCtelera will ask you to create your personal certificate. Important things about
## your certificate to take into account
##    1) If you lose it or forget the password, you may have problems with your published
##       apps and games on Google Play Store. Your certificate identifies you, and creating
##       a new one is like becoming a new person. You may not be able to update your previously
##       published apps and games.
##
##    2) Your certificate information must be real and accurate if you want to publish 
##       your apps or games on any Store or Market. You may get banned or incur in serious legal 
##       troubles if your information is not real or accurate. Take this seriously. 
##
##    3) You are encouraged to make backups of your certificate. Your certificate is stored
##       in your CPCtelera folder path, inside cpctelera/tools/android/certs/cpctelera.user.keystore.jks.
##       This is a Java Keystore containing your certificate. Please back it up and restore it
##       whenever you need. Just by replacing it with any previous personal keystore of yours CPCtelera
##       will recognize and use it. In fact, if you want to use your personal keystore, just make 
##       a copy and replace this file (or use a symbolic link).
##
