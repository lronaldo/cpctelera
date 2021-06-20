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
AND_OBJAPKTOOLYML := $(AND_OBJAPKDIR)/apktool.yml
AND_STRINGRESFILE := $(AND_OBJAPKDIR_RES)/values/strings.xml
AND_OBJPAYLOADSNA := $(AND_OBJAPKDIR_ASS)/payload.sna
AND_APPIDTAG      := XXXCPCT.RVMEngine.AppIDXXX
AND_APPNAMETAG    := %%%CPCTRVMEngineAppName%%%
AND_APPSPLASHTAG  := %%%CPCTRVMEngineSplash%%%
AND_APPVERCODETAG := versionCode:
AND_APPVERNAMETAG := versionName:
AND_DEFAULT_APK   := $(call SYSPATH,$(ANDROID_PATH)rvmengine/defaultRVMapp.apk)
AND_KEYSTORE_DIR  := $(ANDROID_PATH)certs
AND_LICENSE_FILE  := $(ANDROID_PATH)LICENSE
AND_LICENSE_ACC   := $(AND_KEYSTORE_DIR)/.CPCTRVMEngineEndUserLicenseAccepted
ZIPALIGN_LINUX    := $(ANDROID_PATH)bin/zipalign/linux/zipalign32
ZIPALIGN_WIN      := $(ANDROID_PATH)bin/zipalign/win/32/zipalign.exe
ZIPALIGN          := $(if $(call CHECKSYSTEMCYGWIN),$(ZIPALIGN_WIN),$(ZIPALIGN_LINUX))
KEYTOOL_JAR       := $(call SYSPATH,$(ANDROID_PATH)bin/sun/keytool.jar)
JARSIGNER_JAR     := $(call SYSPATH,$(ANDROID_PATH)bin/sun/jarsigner.jar)
APKTOOL_JAR       := $(call SYSPATH,$(ANDROID_PATH)bin/apktool/apktool_2.4.0.jar)
APKSIGNER_JAR     := $(call SYSPATH,$(ANDROID_PATH)bin/sun/apksigner.jar)
KEYTOOL           := $(JAVA) -jar "$(KEYTOOL_JAR)"
JARSIGNER         := $(JAVA) -jar "$(JARSIGNER_JAR)"
APKSIGNER			:= $(JAVA) -jar "$(APKSIGNER_JAR)"
APKTOOL           := $(JAVA) -jar "$(APKTOOL_JAR)"

## User configurable values
AND_APK         :=game.apk
AND_APPNAME     :=CPCtelera game
AND_VERSIONCODE :=  versionCode: '1'
AND_VERSIONNAME :=  versionName: 1.0
AND_SNAFILE     :=$(if $(SNA),$(SNA),)
AND_ASSETS      :=android/assets
AND_RES         :=android/res
AND_APPID       :=$(AND_APPID_PREFIX).game
AND_KEYSTORE    :=$(call SYSPATH,$(AND_KEYSTORE_DIR)/cpctelera.user.keystore.jks)
AND_KEYALIAS    :=cpctelera.user.cert
AND_KEYVALID    :=10000
AND_KEYSIZE     :=2048
AND_KEYALG      :=RSA
AND_STORETYPE   :=pkcs12

## Values depending on user config
AND_TMPAPK        := $(AND_OBJDIR)/$(AND_APK).tmp
AND_TMPAPKALIGNED := $(AND_OBJDIR)/$(AND_APK).aligned.tmp

## JAVA Version
#AND_JAVA_VER     := $(shell java -version 2>&1 | sed -n ';s/.* version "\(.*\)\.\(.*\)\.\(.*\)_.*"/\1\2\3/p;')
#AND_JAVA_UPD     := $(shell java -version 2>&1 | sed -n ';s/.* version ".*\..*\..*_\(.*\)"/\1/p;')
## Commented by now. TODO: Differentiate OpenJDK and Java versions
AND_JAVA_VER     := 200
AND_JAVA_UPD     := 200
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
# AND_SET_VESIONCODE: Sets the VersionCode number that identifies
# the current version of this application 
#
# $(1): VERSIONCODE
#
define AND_SET_VERSIONCODE
 # Set Version Code
 $(eval AND_VERSIONCODE :=versionCode: '$(strip $(1))')
endef

#################
# AND_SET_VESIONNAME: Sets the VersionName, which is an string given
# to name the current version of this application
#
# $(1): VERSIONNAME
#
define AND_SET_VERSIONNAME
 # Set Version Name
 $(eval AND_VERSIONNAME :=versionName: $(strip $(1)))
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
# AND_BETA_LICENSE_MESSAGE: Shows the beta license and asks the user
# to accept it before continuing. Only after accepting the beta
# license the user will be authorized to continue
#
define AND_BETA_LICENSE_MESSAGE
 	@if [ ! -f "$(AND_LICENSE_ACC)" ]; then                                                          \
		printf "$(COLOR_WHITE)"                                                                      ;\
		printf "\n==================================================================================";\
		printf "\n$(COLOR_RED)CPCtelera Android Exporter Beta & RVMEngine:$(COLOR_YELLOW) User license Agreement.";\
	 	printf "$(COLOR_WHITE)"                                                                      ;\
		printf "\n==================================================================================";\
		printf "\n$(COLOR_RED)The authors:"                                                          ;\
		printf "\n$(COLOR_WHITE)CPCtelera Android Exporter Beta"                                     ;\
		printf "\n$(COLOR_CYAN)   (C)opyright 2019. José Luís Luri Bolinski (jose.luis.luri@gmail.com)";\
		printf "\n$(COLOR_CYAN)   (C)opyright 2020. Francisco J. Gallego-Durán (ronaldo@cheesetea.com)";\
		printf "\n$(COLOR_WHITE)RVMEngine"                                                           ;\
		printf "\n$(COLOR_CYAN)   (C)opyright 2020. Juan Carlos González Amestoy (rvm@retrovirtualmachine.org)";\
	 	printf "$(COLOR_WHITE)"                                                                      ;\
		printf "\n==================================================================================";\
	 	printf "$(COLOR_NORMAL)"                                                                     ;\
	 	printf "\n    The authors of CPCtelera Android Exporter Beta and RVMEngine grant you "       ;\
	 	printf "\npermission to use this software, provided that you accept the following license "  ;\
	 	printf "\nterms:"                                                                            ;\
	 	printf "$(COLOR_CYAN)"                                                                       ;\
	 	printf "\n\n    (1) You are only allowed to generate and distribute APK packages for Amstrad";\
	 	printf "\nCPC software snapshots (SNA) of your own. You MUST always possess CopyRights of "  ;\
	 	printf "\nthe software you will distribute using these tools."                               ;\
	 	printf "\n    (2) You are NOT allowed to modify or redistribute RVMEngine by any means. You ";\
	 	printf "\nare ONLY permitted to use it together with CPCtelera Android Exporter, and ONLY "  ;\
	 	printf "\nto distribute software whose rights you own."                                      ;\
	 	printf "\n    (3) You are permitted to distribute APK packages generated with CPCtelera "    ;\
	 	printf "\nAndroid Exporter Beta only for NON-COMMERTIAL PURPOSES. In order to sell your "    ;\
	 	printf "\ngenerated APKs you will need special permission from the authors. "                ;\
	 	printf "\n    (4) You are not allowed to remove the CPCtelera and RVMEngine logos from the " ;\
	 	printf "\nsplash screen of your generated APKs, unless you are given special permission from";\
	 	printf "\nthe authors. "                                                                     ;\
	 	printf "\n    (5) THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS ";\
	 	printf "\nOR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,FITNESS";\
	 	printf "\nFOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR "   ;\
		printf "\nCOPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN ";\
		printf "\nAN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION  " ;\
		printf "\nWITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE  "                  ;\
	 	printf "$(COLOR_WHITE)"                                                                      ;\
		printf "\n==================================================================================";\
	 	printf "$(COLOR_YELLOW)"                                                                     ;\
		printf "\nIf you agree with these license terms, then type-in 'I AGREE'\n"                   ;\
	 	printf "$(COLOR_NORMAL)"                                                                     ;\
		read ANSWER                                                                                  ;\
		ANSWER=`echo $$$$ANSWER | tr '[:lower:]' '[:upper:]' | xargs`                                ;\
		if [[ "$$$$ANSWER" = "I AGREE" ]]; then                                                       \
			printf "\n$(COLOR_GREEN)License Accepted. You may use the CPCtelera Android Exporter "    ;\
			printf "Beta and RVMEngine to create and distribute APKs of your own software. \n\n"      ;\
			printf "User accepted license on: `date`" > $(AND_LICENSE_ACC)                            ;\
		else                                                                                          \
			printf "\n$(COLOR_RED)You did not accept the license. You are not permitted to use "      ;\
			printf "CPCtelera Android Exporter Beta nor RVMEngine. "                                  ;\
			printf "\n\n$(COLOR_NORMAL)"                                                              ;\
			exit 3                                                                                    ;\
		fi                                                                                           ;\
		printf "\n\n$(COLOR_NORMAL)"                                                                 ;\
	fi                                                                                              ;\
	cp "$(AND_LICENSE_FILE)" "$(AND_ASSETS)"                                                        ;\
	cp "$(AND_LICENSE_ACC)" "$(AND_ASSETS)"
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
	@# Show License Message and wait acceptance
	$(call AND_BETA_LICENSE_MESSAGE)

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
	@$(MKDIR) $(AND_OBJDIR)
	@date &> $(AND_OBJLOG)
	@printf "\n[[COMMAND]]: $(APKTOOL) decode $(AND_DEFAULT_APK) -f -o $(AND_OBJAPKDIR)\n\n" &>> $(AND_OBJLOG)
	@$(APKTOOL) decode "$(AND_DEFAULT_APK)" -f -o "$(AND_OBJAPKDIR)" &>> $(AND_OBJLOG)
	@printf "$(COLOR_GREEN)OK$(COLOR_NORMAL)\n"
	@# REPLACE SNA
	@printf "  -b: Injecting assets, icons and content..."
	@printf "\n[[COMMAND]]: $(CP) $(AND_SNAFILE) $(AND_OBJPAYLOADSNA)\n\n" &>> $(AND_OBJLOG)
	@$(CP) -v "$(AND_SNAFILE)" "$(AND_OBJPAYLOADSNA)" &>> $(AND_OBJLOG)
	@# REPLACE ASSETS IF THE USER HAS SUPPLIED WITH SOME
	@printf "\n[[COMMAND]]: $(CP) -r $(AND_ASSETS)/* $(AND_OBJAPKDIR_ASS)\n\n" &>> $(AND_OBJLOG)
	@$(CP) -rv "$(AND_ASSETS)"/. "$(AND_OBJAPKDIR_ASS)" &>> $(AND_OBJLOG)
	@# REPLACE RESOURCES IF THE USER HAS SUPPLIED WITH SOME
	@printf "\n[[COMMAND]]: $(CP) -r $(AND_RES)/* $(AND_OBJAPKDIR_RES)\n\n" &>> $(AND_OBJLOG)
	@$(CP) -rv "$(AND_RES)"/. "$(AND_OBJAPKDIR_RES)" &>> $(AND_OBJLOG)
	@# REPLACE APPLICATION NAME
	@printf "\n[[COMMAND]]: Replace tags\n\n" &>> $(AND_OBJLOG)
	@$(call REPLACETAG_RT,$(AND_APPNAMETAG),$(AND_APPNAME),$(AND_STRINGRESFILE)) &>> $(AND_OBJLOG)
	@# REPLACE APPLICATION ID
	@$(call REPLACETAG_RT,$(AND_APPIDTAG),$(AND_APPID),$(AND_OBJAPKMANIFEST)) &>> $(AND_OBJLOG)
	@# REPLACE VERSIONCODE
	@$(call REPLACETAGGEDLINE_RT,$(AND_APPVERCODETAG),  $(AND_VERSIONCODE),$(AND_OBJAPKTOOLYML)) &>> $(AND_OBJLOG)
	@# REPLACE VERSIONNAME
	@$(call REPLACETAGGEDLINE_RT,$(AND_APPVERNAMETAG),  $(AND_VERSIONNAME),$(AND_OBJAPKTOOLYML)) &>> $(AND_OBJLOG)
	@printf "$(COLOR_GREEN)OK$(COLOR_NORMAL)\n"
	@# BUILD APK
	@printf "  -c: Rebuilding..."
	@printf "\n[[COMMAND]]: $(APKTOOL) build $(AND_OBJAPKDIR) -o $(AND_TMPAPK)\n\n" &>> $(AND_OBJLOG)
	@$(APKTOOL) build "$(AND_OBJAPKDIR)" -o "$(AND_TMPAPK)" &>> $(AND_OBJLOG)
	@printf "$(COLOR_GREEN)OK$(COLOR_NORMAL)\n"
	@printf "\n[[COMMAND]]: $(RM) -r \"./$(AND_OBJAPKDIR)\"\n\n" &>> $(AND_OBJLOG)
	@$(RM) -r "./$(AND_OBJAPKDIR)" &>> $(AND_OBJLOG)
	@# ALIGN APK
	@printf "$(COLOR_CYAN) (3/4): Aligning...$(COLOR_NORMAL)"
	@printf "\n[[COMMAND]]: $(ZIPALIGN) -f -v 4 $(AND_TMPAPK) $(AND_TMPAPKALIGNED)\n\n" &>> $(AND_OBJLOG)
	@$(ZIPALIGN) -f -v 4 "$(AND_TMPAPK)" "$(AND_TMPAPKALIGNED)" &>> $(AND_OBJLOG)
	@printf "$(COLOR_GREEN)OK$(COLOR_NORMAL)\n"
	@# SIGN APK
	@printf "$(COLOR_CYAN) (4/4): Signing APK...\n$(COLOR_NORMAL)"
	@printf "\n[[COMMAND]]: $(APKSIGNER) sign --ks $(AND_KEYSTORE) --out $(AND_APK) $(AND_TMPAPKALIGNED)\n\n" &>> $(AND_OBJLOG)
	@$(APKSIGNER) sign --ks "$(AND_KEYSTORE)" --out "$(AND_APK)" "$(AND_TMPAPKALIGNED)"
	@#printf "\n[[COMMAND]]: $(JARSIGNER) -verbose -sigalg SHA1withRSA -digestalg SHA1 -keystore $(AND_KEYSTORE) $(AND_TMPAPK) $(AND_KEYALIAS)\n\n" &>> $(AND_OBJLOG)
	@#$(JARSIGNER) -verbose -sigalg SHA1withRSA -digestalg SHA1 -keystore "$(AND_KEYSTORE)" "$(AND_TMPAPK)" $(AND_KEYALIAS)
	@printf "\n[[COMMAND]]: $(RM) ./$(AND_TMPAPK) ./$(AND_TMPAPKALIGNED)\n\n" &>> $(AND_OBJLOG)
	@$(RM) "./$(AND_TMPAPK)" "./$(AND_TMPAPKALIGNED)" &>> $(AND_OBJLOG)
	@printf "$(COLOR_GREEN)Signed with APK Signature Scheme v2$(COLOR_NORMAL)\n"

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
 $(eval APKGEN_F_FUNCTIONS := SET_SNA SET_APPID SET_APPNAME SET_VERSIONNAME SET_VERSIONCODE SET_ASSET_FOLDERS GENERATE_APK_RULE)

 # Check that command parameter ($(1)) is exactly one-word after stripping whitespaces
 $(call ENSURE_SINGLE_VALUE,$(1),<<ERROR>> [APKGEN] '$(strip $(1))' is not a valid command. Commands must be exactly one-word in lenght with no whitespaces. Valid commands: {$(APKGEN_F_FUNCTIONS)})

 # Filter given command as $(1) to see if it is one of the valid commands
 $(eval APKGEN_F_SF = $(filter $(APKGEN_F_FUNCTIONS),$(1)))

 # If the given command is valid, it will be non-empty, then we proceed to call the command (up to 8 arguments). Otherwise, raise an error
 $(if $(APKGEN_F_SF)\
    ,$(eval $(call AND_$(APKGEN_F_SF),$(strip $(2)),$(strip $(3))))\
    ,$(error <<ERROR>> [APKGEN] '$(strip $(1))' is not a valid command. Valid commands: {$(APKGEN_F_FUNCTIONS)}))
endef
