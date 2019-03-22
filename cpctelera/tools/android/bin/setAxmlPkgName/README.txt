This utility read binary AndroidManifest.xml file and modify some data about package name then write another binary xml.
Combining with other utilities, you can modify package name of an APK without recompile it nor change java byte code,
as a result, you can install the modified APK as a new app yet has same UI, same logic as old one.

Usage:
    java -jar setAxmlPkgName.jar androidManifestBinXml newPackageFullName

Exmaple: java -jar setAxmlPkgName.jar someDir/AndroidManifest.xml com.example.newapp
Exmaple: java -jar setAxmlPkgName.jar someDir/AndroidManifest.xml com.example.newapp!

NOTE:
 if newPackageFullName ends with ! then it will remove conflict settings:
   <original-package>,<provider>,android:protectionLevel,process,sharedUserId.

This utility changes APK's package name (not java package name) and
  prepend java package name to partial class name in AndroidManifest.xml:
  Application,Activity,Receiver,Service... 
  backupAgent,manageSpaceActivity,targetActivity... 
  meta value(only if start with dot)


Sample:   ( $ just means command prompt)

#extract AndroidManifest.xml from app.apk
$ jar xvf app.apk AndroidManifest.xml

$ java -jar setAxmlPkgName.jar AndroidManifest.xml com.example.newapp

    package=com.example.app
=>  package=com.example.newapp
            name=.Activity1
=>          name=com.examples.Activity1

#update AndroidManifest.xml in app.apk
$ jar uvf app.apk AndroidManifest.xml

#remove signature from app.apk
$ zip -d app.apk META-INF/MANIFEST.MF "META-INF/*.SF" "META-INF/*.RSA" "META-INF/*.DSA" "META-INF/SIG-*"

#use jarsigner to sign app.apk
$ echo android | jarsigner -keystore ~/.android/debug.keystore -sigfile CERT -sigalg SHA1withRSA -digestalg SHA app.apk androiddebugkey

#install
$ adb uninstall com.example.newapp
$ adb install app.apk


Note:
   if you try to do this for system APK, then because it does not contains classes.dex
   so you will be not able to install or run the modified APK. 
   To overcome this problem, you can extract Browser.odex from /system/app of the device, 
   then use baksmali tool to convert it to smali then to dex file, finaly add the dex file to the APK as classes.dex.

$ adb pull /system/app/SomeSystemApp.odex
$ adb pull /system/framework      #get some bootstrap odex such as core.odex 
$ baksmali -d . -x SomeSystemApp.odex
$ smali out -o classes.dex        #convert smali out to dex
$ jar uvf app.apk classes.dex     #add classes.dex to APK