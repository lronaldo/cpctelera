# sdcc.nsi - NSIS installer script for SDCC
#
# Copyright (c) 2003-2013 Borut Razem
#
# This file is part of sdcc.
#
#  This software is provided 'as-is', without any express or implied
#  warranty.  In no event will the authors be held liable for any damages
#  arising from the use of this software.
#
#  Permission is granted to anyone to use this software for any purpose,
#  including commercial applications, and to alter it and redistribute it
#  freely, subject to the following restrictions:
#
#  1. The origin of this software must not be misrepresented; you must not
#     claim that you wrote the original software. If you use this software
#     in a product, an acknowledgment in the product documentation would be
#     appreciated but is not required.
#  2. Altered source versions must be plainly marked as such, and must not be
#     misrepresented as being the original software.
#  3. This notice may not be removed or altered from any source distribution.
#
#  Borut Razem
#  borut.razem@siol.net

# How to create WIN32 setup.exe
#
# - unpack WIN32 mingw daily snapshot sdcc-snapshot-i586-mingw32msvc-yyyymmdd-rrrr.zip
#   to a clean directory (the option to create directories should be enabled).
#   A sub directory sdcc is created (referenced as PKGDIR in continuation).
# - copy files sdcc/support/scripts/sdcc.ico and sdcc/support/scripts/sdcc.nsi
#   (this file) from the sdcc Subversion snapshot to the PKGDIR directory
# - copy file COPYING and COPYING3 from the sdcc Subversion snapshot to the PKGDIR directory,
#   rename it to COPYING.txt and COPYING3.txt and convert it to DOS format:
#   unix2dos COPYING.txt
#   unix2dos COPYING3.txt
#   unix2dos doc/ChangeLog_head.txt
#   unix2dos doc/README.TXT
# - run NSIS installer from PKGDIR directory:
#   "c:\Program Files\NSIS\makensis.exe" -DVER_MAJOR=<SDCC_VER_MAJOR> -DVER_MINOR=<SDCC_VER_MINOR> -DVER_REVISION=<SDCC_VER_DEVEL> -DVER_BUILD=<SDCC_REVISION> sdcc.nsi
#   replace <VER_XXX> with the appropriate values, for example for SDCC 2.7.4:
#   <SDCC_VER_MAJOR> = 2
#   <SDCC_VER_MINOR> = 7
#   <SDCC_VER_DEVEL> = 4
#   replace <SDCC_REVISION> with the current svn revision number.
#   Define -DWIN64 if createing a 64bit package.
# - A setup file setup.exe is created in PKGDIR directory.
#   Rename it to sdcc-yyyymmdd-rrrr-setup.exe and upload it
#   to sdcc download repository at sourceforge.net
#
#
# How to create WIN32 release setup.exe package
#
# - unpack WIN32 mingw daily snapshot sdcc-snapshot-i586-mingw32msvc-yyyymmdd-rrrr.zip
#   to a clean directory (the option to create directories should be enabled).
#   A sub directory sdcc is created (referenced as PKGDIR in continuation).
# - remove the PKGDIR/doc/ directory
# - unpack sdcc-doc-yyyymmdd-rrrr.zip to the PKGDIR/doc directory
# - copy files sdcc/support/scripts/sdcc.ico and sdcc/support/scripts/sdcc.nsi
#   (this file) from the sdcc Subversion snapshot to the PKGDIR directory
# - copy file COPYING and COPYING3 from the sdcc Subversion snapshot to the PKGDIR directory,
#   rename it to COPYING.txt and COPYING3.txt and convert it to DOS format:
#   unix2dos COPYING.txt
#   unix2dos COPYING3.txt
#   unix2dos doc/ChangeLog.txt
#   unix2dos doc/README.TXT
# - run NSIS installer from PKGDIR directory:
#   "c:\Program Files\NSIS\makensis.exe" -DFULL_DOC -DVER_MAJOR=<VER_MAJOR> -DVER_MINOR=<VER_MINOR> -DVER_REVISION=<VER_PATCH> -DVER_BUILD=<REVISION> sdcc.nsi
#   replace <VER_XXX> with the appropriate values, for example for SDCC 3.0.0:
#   <SDCC_VER_MAJOR> = 3
#   <SDCC_VER_MINOR> = 0
#   <SDCC_VER_DEVEL> = 0
#   replace <SDCC_REVISION> with the current svn revision number.
#   Define -DWIN64 if createing a 64bit package.
# - A setup file setup.exe is created in PKGDIR directory.
#   Rename it to sdcc-x.x.x-setup.exe and upload it
#   to sdcc download repository at sourceforge.net
#
# For debugging define -DSDCC.DEBUG command line option

;--------------------------------
; Debugging Macros

!ifdef SDCC.DEBUG
  Var SDCC.FunctionName
  Var SDCC.StrStack0
  Var SDCC.StrStack1
  Var SDCC.StrStack2
  Var SDCC.StrStack3
  Var SDCC.StrStack4

!define SDCC.PushStr "!insertmacro MACRO_SDCC_PushStr"
!macro MACRO_SDCC_PushStr NAME
  StrCpy $SDCC.StrStack4 $SDCC.StrStack3
  StrCpy $SDCC.StrStack3 $SDCC.StrStack2
  StrCpy $SDCC.StrStack2 $SDCC.StrStack1
  StrCpy $SDCC.StrStack1 $SDCC.StrStack0
  StrCpy $SDCC.StrStack0 $SDCC.FunctionName
  StrCpy $SDCC.FunctionName "${NAME}"
!macroend

!define SDCC.PopStr "!insertmacro MACRO_SDCC_PopStr"
!macro MACRO_SDCC_PopStr
  StrCpy $SDCC.FunctionName $SDCC.StrStack0
  StrCpy $SDCC.StrStack0 $SDCC.StrStack1
  StrCpy $SDCC.StrStack1 $SDCC.StrStack2
  StrCpy $SDCC.StrStack2 $SDCC.StrStack3
  StrCpy $SDCC.StrStack3 $SDCC.StrStack4
!macroend
!endif

!define DebugMsg "!insertmacro MACRO_SDCC_DebugMsg"
!macro MACRO_SDCC_DebugMsg MSG
  !ifdef SDCC.DEBUG
    MessageBox MB_OK "*** $SDCC.FunctionName: ${MSG} ***"
  !endif
!macroend

!define Function "!insertmacro MACRO_SDCC_Function"
!macro MACRO_SDCC_Function NAME
  Function "${NAME}"
  !ifdef SDCC.DEBUG
    ${SDCC.PushStr} ${NAME}
  !endif
!macroend

!define FunctionEnd "!insertmacro MACRO_SDCC_FunctionEnd"
!macro MACRO_SDCC_FunctionEnd
  !ifdef SDCC.DEBUG
    ${SDCC.PopStr}
  !endif
  FunctionEnd
!macroend

!define Section "!insertmacro MACRO_SDCC_Section"
!macro MACRO_SDCC_Section NAME ID
  Section "${NAME}" "${ID}"
  !ifdef SDCC.DEBUG
    ${SDCC.PushStr} "${NAME}"
  !endif
!macroend

!define UnselectedSection "!insertmacro MACRO_SDCC_UnselectedSection"
!macro MACRO_SDCC_UnselectedSection NAME ID
  Section /o ${NAME} ${ID}
  !ifdef SDCC.DEBUG
    ${SDCC.PushStr} "${NAME}"
  !endif
!macroend

!define SectionEnd "!insertmacro MACRO_SDCC_SectionEnd"
!macro MACRO_SDCC_SectionEnd
  !ifdef SDCC.DEBUG
    ${SDCC.PopStr}
  !endif
  SectionEnd
!macroend


!define PRODUCT_NAME "SDCC"

; Version
!ifdef VER_MAJOR & VER_MINOR & VER_REVISION & VER_BUILD
  !define PRODUCT_VERSION "${VER_MAJOR}.${VER_MINOR}.${VER_REVISION}"
!else
  !define PRODUCT_VERSION "XX.XX"
!endif

SetCompressor /SOLID lzma

!define SDCC_ROOT "."

!define DEV_ROOT "${SDCC_ROOT}"

InstType "Full (Bin, ucSim, SDCDB, Doc, Lib, Src)"
InstType "Medium (Bin, ucSim, SDCDB, Doc, Lib)"
InstType "Compact (Bin, ucSim, SDCDB, Doc)"

;--------------------------------
; Configuration

!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define UNINST_ROOT_KEY HKLM
!define SDCC_ROOT_KEY HKLM

;--------------------------------
; Header Files

!include MUI2.nsh
!include WordFunc.nsh
!include StrFunc.nsh
!include WinVer.nsh
!include x64.nsh
${StrStr}
${UnStrStr}

;--------------------------------
; Functions

!ifdef VER_MAJOR & VER_MINOR & VER_REVISION & VER_BUILD
  !insertmacro VersionCompare
!endif

;--------------------------------
; Variables

Var SDCC.PathToRemove

;--------------------------------
; Configuration

; MUI Settings
!define MUI_ABORTWARNING
!define MUI_ICON ".\sdcc.ico"

; Welcome page
!define MUI_WELCOMEPAGE_TEXT "This wizard will guide you through the installation of $(^NameDA).$\r$\n$\r$\nIt is recommended that you close all other applications before starting Setup. This will make it possible to update relevant system files without having to reboot your computer.$\r$\n$\r$\n$_CLICK"
!insertmacro MUI_PAGE_WELCOME

; License page
!insertmacro MUI_PAGE_LICENSE "${SDCC_ROOT}\COPYING.txt"

; Uninstall/reinstall page
!ifdef VER_MAJOR & VER_MINOR & VER_REVISION & VER_BUILD
Page custom SDCC.PageReinstall SDCC.PageLeaveReinstall
!endif

; StartMenu page
!define MUI_STARTMENUPAGE_DEFAULTFOLDER ${PRODUCT_NAME}
!define MUI_STARTMENUPAGE_REGISTRY_ROOT ${UNINST_ROOT_KEY}
!define MUI_STARTMENUPAGE_REGISTRY_KEY "${PRODUCT_UNINST_KEY}"
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "NSIS:StartMenuDir"
!define MUI_STARTMENUPAGE_NODISABLE
Var MUI_STARTMENUPAGE_VARIABLE
!insertmacro MUI_PAGE_STARTMENU Application $MUI_STARTMENUPAGE_VARIABLE

; Components page
!define MUI_COMPONENTSPAGE_SMALLDESC
!insertmacro MUI_PAGE_COMPONENTS

; Directory page
!insertmacro MUI_PAGE_DIRECTORY

; Instfiles page
!define MUI_PAGE_CUSTOMFUNCTION_LEAVE "SDCC.InstFilesLeave"
!insertmacro MUI_PAGE_INSTFILES

${Function} SDCC.InstFilesLeave
  ; Remove old path if reinstallation
  ${If} $SDCC.PathToRemove != ""
    ${DebugMsg} "removing path $SDCC.PathToRemove"
    Push $SDCC.PathToRemove
    Call SDCC.RemoveFromPath
  ${EndIf}
${FunctionEnd}

; Finish page - add to path
!define MUI_FINISHPAGE_TEXT "Confirm the checkbox if you want to add SDCC binary directory to the PATH environment variable"
!define MUI_FINISHPAGE_SHOWREADME_TEXT "Add $INSTDIR\bin to the PATH"
!define MUI_FINISHPAGE_SHOWREADME_FUNCTION SDCC.AddBinToPath
!define MUI_FINISHPAGE_SHOWREADME
!define MUI_FINISHPAGE_BUTTON "Next"
!insertmacro MUI_PAGE_FINISH

; Finish page - reboot
!insertmacro MUI_PAGE_FINISH

${Function} SDCC.AddBinToPath
  ; Add new path
  ${DebugMsg} "adding path $INSTDIR\bin"
  Push "$INSTDIR\bin"
  Call SDCC.AddToPath
${FunctionEnd}

; Uninstaller pages
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

; Language files
!insertmacro MUI_LANGUAGE "English"

Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
BrandingText ""
OutFile "setup.exe"
RequestExecutionLevel admin ;Require admin rights on NT6+ (When UAC is turned on)
;;;;ShowInstDetails show
;;;;ShowUnInstDetails show


${Function} .onInit
  ${DebugMsg} "Pre INSTDIR = $INSTDIR"

  ${If} ${RunningX64}
  !ifdef WIN64
    StrCpy $INSTDIR "$PROGRAMFILES64\${PRODUCT_NAME}"
    SetRegView 64
  !else
    StrCpy $INSTDIR "$PROGRAMFILES\${PRODUCT_NAME}"
    SetRegView 32
  !endif
  ${Else}
  !ifdef WIN64
    MessageBox MB_OK|MB_ICONSTOP \
      "This installation package is not supported on this platform. Contact your application vendor."
    Abort
  !endif
    StrCpy $INSTDIR "$PROGRAMFILES\${PRODUCT_NAME}"
  ${Endif}

!ifndef VER_MAJOR & VER_MINOR & VER_REVISION & VER_BUILD
  ; Old unistallation method
  ; Uninstall the old version, if present
  ReadRegStr $R0 ${UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString"
  ${If} $R0 != ""
    MessageBox MB_OKCANCEL|MB_ICONEXCLAMATION \
      "$(^Name) is already installed. $\n$\nClick 'OK' to remove the previous version or 'Cancel' to cancel this upgrade." \
      IDOK +2
    Abort

    ; Run the uninstaller
    ClearErrors
    ExecWait '$R0'
  ${Else}
    ; Install the new version
    MessageBox MB_YESNO|MB_ICONQUESTION "This will install $(^Name). Do you wish to continue?" \
      IDYES +2
    Abort
  ${Endif}
!else
  ; If the registry key exists it is an uninstallation or reinstallation:
  ;  take the old installation directory
  Push $R0

  ReadRegStr $R0 ${UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "InstallLocation"
  ${IfNot} ${Errors}
    StrCpy $INSTDIR $R0
    StrCpy $SDCC.PathToRemove "$INSTDIR\bin"
  ${EndIf}

  Pop $R0
!endif
  ${DebugMsg} "Post INSTDIR = $INSTDIR"
${FunctionEnd}

${Function} un.onInit
  ${DebugMsg} "Pre INSTDIR = $INSTDIR"

  ${If} ${RunningX64}
  !ifdef WIN64
    SetRegView 64
  !else
    SetRegView 32
  !endif
  ${Endif}

  Push $R0
  ReadRegStr $R0 ${UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "InstallLocation"
  ${IfNot} ${Errors}
    StrCpy $INSTDIR $R0
  ${EndIf}
  Pop $R0

  ${DebugMsg} "Post INSTDIR = $INSTDIR"

${FunctionEnd}

${Section} -Common SECCOMMON
  SetOutPath "$INSTDIR"
  File ".\sdcc.ico"
  File "${SDCC_ROOT}\COPYING.txt"
  File "${SDCC_ROOT}\COPYING3.txt"
${SectionEnd}

${Section} "SDCC application files" SEC01
  SectionIn 1 2 3 RO
  SetOutPath "$INSTDIR\bin"
  File "${SDCC_ROOT}\bin\sdasgb.exe"
  File "${SDCC_ROOT}\bin\sdas6808.exe"
  File "${SDCC_ROOT}\bin\sdasz80.exe"
  File "${SDCC_ROOT}\bin\sdas8051.exe"
  File "${SDCC_ROOT}\bin\sdas390.exe"
  File "${SDCC_ROOT}\bin\sdasrab.exe"
  File "${SDCC_ROOT}\bin\sdasstm8.exe"
  File "${SDCC_ROOT}\bin\sdaspdk13.exe"
  File "${SDCC_ROOT}\bin\sdaspdk14.exe"
  File "${SDCC_ROOT}\bin\sdaspdk15.exe"
  File "${SDCC_ROOT}\bin\sdastlcs90.exe"
  File "${SDCC_ROOT}\bin\sdld.exe"
  File "${SDCC_ROOT}\bin\sdldgb.exe"
  File "${SDCC_ROOT}\bin\sdld6808.exe"
  File "${SDCC_ROOT}\bin\sdldz80.exe"
  File "${SDCC_ROOT}\bin\sdldstm8.exe"
  File "${SDCC_ROOT}\bin\sdldpdk.exe"
  File "${SDCC_ROOT}\bin\sdar.exe"
  File "${SDCC_ROOT}\bin\sdranlib.exe"
  File "${SDCC_ROOT}\bin\sdnm.exe"
  File "${SDCC_ROOT}\bin\sdobjcopy.exe"
  File "${SDCC_ROOT}\bin\makebin.exe"
  File "${SDCC_ROOT}\bin\packihx.exe"
  File "${SDCC_ROOT}\bin\sdcc.exe"
  File "${SDCC_ROOT}\bin\sdcpp.exe"
  File "${SDCC_ROOT}\bin\as2gbmap.cmd"
  File "${SDCC_ROOT}\bin\readline5.dll"
!ifdef WIN64
  File "${SDCC_ROOT}\bin\libgcc_s_*-1.dll"
  File "${SDCC_ROOT}\bin\libstdc++-6.dll"
  File "${SDCC_ROOT}\bin\libwinpthread-1.dll"
!endif
${SectionEnd}

${Section} "ucSim application files" SEC02
  SectionIn 1 2 3
  SetOutPath "$INSTDIR\bin"
  File "${SDCC_ROOT}\bin\s51.exe"
  File "${SDCC_ROOT}\bin\shc08.exe"
  File "${SDCC_ROOT}\bin\sz80.exe"
  File "${SDCC_ROOT}\bin\sstm8.exe"
${SectionEnd}

${Section} "SDCDB files" SEC03
  SectionIn 1 2 3
  File "${SDCC_ROOT}\bin\sdcdb.exe"
  File "${SDCC_ROOT}\bin\sdcdb.el"
  File "${SDCC_ROOT}\bin\sdcdbsrc.el"
${SectionEnd}

${Section} "SDCC documentation" SEC04
  SectionIn 1 2 3
  SetOutPath "$INSTDIR\doc"
!ifdef FULL_DOC
  File /r "${SDCC_ROOT}\doc\*"
!else
  File "${SDCC_ROOT}\doc\ChangeLog_head.txt"
  File "${SDCC_ROOT}\doc\README.TXT"
!endif
${SectionEnd}

${Section} "SDCC include files" SEC05
  SectionIn 1 2
  SetOutPath "$INSTDIR\include\asm\default"
  File "${DEV_ROOT}\include\asm\default\features.h"
  SetOutPath "$INSTDIR\include\asm\ds390"
  File "${DEV_ROOT}\include\asm\ds390\features.h"
  SetOutPath "$INSTDIR\include\asm\gbz80"
  File "${DEV_ROOT}\include\asm\gbz80\features.h"
  SetOutPath "$INSTDIR\include\asm\mcs51"
  File "${DEV_ROOT}\include\asm\mcs51\features.h"
  SetOutPath "$INSTDIR\include\asm\pic14"
  File "${DEV_ROOT}\include\asm\pic14\features.h"
  SetOutPath "$INSTDIR\include\asm\pic16"
  File "${DEV_ROOT}\include\asm\pic16\features.h"
  SetOutPath "$INSTDIR\include\asm\z80"
  File "${DEV_ROOT}\include\asm\z80\features.h"
  SetOutPath "$INSTDIR\include\asm\r2k"
  File "${DEV_ROOT}\include\asm\r2k\features.h"
  SetOutPath "$INSTDIR\include\asm\r3ka"
  File "${DEV_ROOT}\include\asm\r3ka\features.h"
  SetOutPath "$INSTDIR\include\asm\stm8"
  File "${DEV_ROOT}\include\asm\stm8\features.h"

  SetOutPath "$INSTDIR\include\ds390"
  File "${DEV_ROOT}\include\ds390\*.h"
  SetOutPath "$INSTDIR\include\ds400"
  File "${DEV_ROOT}\include\ds400\*.h"
  SetOutPath "$INSTDIR\include\hc08"
  File "${DEV_ROOT}\include\hc08\*.h"
  SetOutPath "$INSTDIR\include\mcs51"
  File "${DEV_ROOT}\include\mcs51\*.h"
  SetOutPath "$INSTDIR\include\pic14"
  File "${DEV_ROOT}\include\pic14\*.h"
  File "${DEV_ROOT}\include\pic14\*.txt"
  File "${DEV_ROOT}\include\pic14\*.inc"
  SetOutPath "$INSTDIR\include\pic16"
  File "${DEV_ROOT}\include\pic16\*.h"
  File "${DEV_ROOT}\include\pic16\*.txt"
  SetOutPath "$INSTDIR\include\z180"
  File "${DEV_ROOT}\include\z180\*.h"

  SetOutPath "$INSTDIR\include"
  File "${DEV_ROOT}\include\*.h"

  SetOutPath "$INSTDIR\non-free\include\pic14"
  File "${DEV_ROOT}\non-free\include\pic14\*.h"
  SetOutPath "$INSTDIR\non-free\include\pic16"
  File "${DEV_ROOT}\non-free\include\pic16\*.h"
${SectionEnd}

${Section} "SDCC DS390 library" SEC06
  SectionIn 1 2
  SetOutPath "$INSTDIR\lib\ds390"
  File "${DEV_ROOT}\lib\ds390\*.*"
${SectionEnd}

${Section} "SDCC DS400 library" SEC07
  SectionIn 1 2
  SetOutPath "$INSTDIR\lib\ds400"
  File "${DEV_ROOT}\lib\ds400\*.*"
${SectionEnd}

${Section} "SDCC GBZ80 library" SEC08
  SectionIn 1 2
  SetOutPath "$INSTDIR\lib\gbz80"
  File "${DEV_ROOT}\lib\gbz80\*.*"
${SectionEnd}

${Section} "SDCC Z180 library" SEC09
  SectionIn 1 2
  SetOutPath "$INSTDIR\lib\z180"
  File "${DEV_ROOT}\lib\z180\*.*"
${SectionEnd}

${Section} "SDCC Rabbit 2000 library" SEC10
  SectionIn 1 2
  SetOutPath "$INSTDIR\lib\r2k"
  File "${DEV_ROOT}\lib\r2k\*.*"
${SectionEnd}

${Section} "SDCC Rabbit 3000A library" SEC11
  SectionIn 1 2
  SetOutPath "$INSTDIR\lib\r3ka"
  File "${DEV_ROOT}\lib\r3ka\*.*"
${SectionEnd}

${Section} "SDCC Z80 library" SEC12
  SectionIn 1 2
  SetOutPath "$INSTDIR\lib\z80"
  File "${DEV_ROOT}\lib\z80\*.*"
${SectionEnd}

${Section} "SDCC mcs51 small model library" SEC13
  SectionIn 1 2
  SetOutPath "$INSTDIR\lib\small"
  File "${DEV_ROOT}\lib\small\*.*"
${SectionEnd}

${Section} "SDCC mcs51 medium model library" SEC14
  SectionIn 1 2
  SetOutPath "$INSTDIR\lib\medium"
  File "${DEV_ROOT}\lib\medium\*.*"
${SectionEnd}

${Section} "SDCC mcs51 large model library" SEC15
  SectionIn 1 2
  SetOutPath "$INSTDIR\lib\large"
  File "${DEV_ROOT}\lib\large\*.*"
${SectionEnd}

${Section} "SDCC mcs51 huge model library" SEC16
  SectionIn 1 2
  SetOutPath "$INSTDIR\lib\huge"
  File "${DEV_ROOT}\lib\huge\*.*"
${SectionEnd}

${Section} "SDCC mcs51 small-stack-auto model library" SEC17
  SectionIn 1 2
  SetOutPath "$INSTDIR\lib\small-stack-auto"
  File "${DEV_ROOT}\lib\small-stack-auto\*.*"
${SectionEnd}

${Section} "SDCC mcs51 large-stack-auto model library" SEC18
  SectionIn 1 2
  SetOutPath "$INSTDIR\lib\large-stack-auto"
  File "${DEV_ROOT}\lib\large-stack-auto\*.*"
${SectionEnd}

${Section} "SDCC HC08 library" SEC19
  SectionIn 1 2
  SetOutPath "$INSTDIR\lib\hc08"
  File "${DEV_ROOT}\lib\hc08\*.*"
${SectionEnd}

${Section} "SDCC S08 library" SEC20
  SectionIn 1 2
  SetOutPath "$INSTDIR\lib\s08"
  File "${DEV_ROOT}\lib\s08\*.*"
${SectionEnd}

${Section} "SDCC PIC16 library" SEC21
  SectionIn 1 2
  SetOutPath "$INSTDIR\lib\pic16"
  File "${DEV_ROOT}\lib\pic16\*.o"
  File "${DEV_ROOT}\lib\pic16\*.lib"

  SetOutPath "$INSTDIR\non-free\lib\pic16"
  File "${DEV_ROOT}\non-free\lib\pic16\*.lib"
${SectionEnd}

${Section} "SDCC PIC14 library" SEC22
  SectionIn 1 2
  SetOutPath "$INSTDIR\lib\pic14"
  File "${DEV_ROOT}\lib\pic14\*.lib"

  SetOutPath "$INSTDIR\non-free\lib\pic14"
  File "${DEV_ROOT}\non-free\lib\pic14\*.lib"
${SectionEnd}

${Section} "SDCC STM8 small model library" SEC23
  SectionIn 1 2
  SetOutPath "$INSTDIR\lib\stm8"
  File "${DEV_ROOT}\lib\stm8\*.*"
${SectionEnd}

${Section} "SDCC TLCS90 library" SEC24
  SectionIn 1 2
  SetOutPath "$INSTDIR\lib\tlcs90"
  File "${DEV_ROOT}\lib\tlcs90\*.*"
${SectionEnd}

${Section} "SDCC library sources" SEC25
  SectionIn 1
  SetOutPath "$INSTDIR\lib\src\ds390\examples"
  File "${DEV_ROOT}\lib\src\ds390\examples\MOVED"

  SetOutPath "$INSTDIR\lib\src\ds390"
  File "${DEV_ROOT}\lib\src\ds390\*.c"
#  File "${DEV_ROOT}\lib\src\ds390\Makefile"

  SetOutPath "$INSTDIR\lib\src\ds400"
  File "${DEV_ROOT}\lib\src\ds400\*.c"
#  File "${DEV_ROOT}\lib\src\ds400\Makefile"

  SetOutPath "$INSTDIR\lib\src\gbz80"
  File "${DEV_ROOT}\lib\src\gbz80\*.s"
#  File "${DEV_ROOT}\lib\src\gbz80\Makefile"

  SetOutPath "$INSTDIR\lib\src\z80"
  File "${DEV_ROOT}\lib\src\z80\*.s"
#  File "${DEV_ROOT}\lib\src\z80\Makefile"

  SetOutPath "$INSTDIR\lib\src\z180"
  File "${DEV_ROOT}\lib\src\z180\*.s"
#  File "${DEV_ROOT}\lib\src\z180\Makefile"

  SetOutPath "$INSTDIR\lib\src\r2k"
  File "${DEV_ROOT}\lib\src\r2k\*.s"
#  File "${DEV_ROOT}\lib\src\z180\Makefile"

  SetOutPath "$INSTDIR\lib\src\r3ka"
  File "${DEV_ROOT}\lib\src\r3ka\*.s"
#  File "${DEV_ROOT}\lib\src\r3ka\Makefile"

  SetOutPath "$INSTDIR\lib\src\hc08"
  File "${DEV_ROOT}\lib\src\hc08\*.c"
#  File "${DEV_ROOT}\lib\src\hc08\Makefile"

  SetOutPath "$INSTDIR\lib\src\s08"
  File "${DEV_ROOT}\lib\src\s08\*.c"
#  File "${DEV_ROOT}\lib\src\s08\Makefile"

  SetOutPath "$INSTDIR\lib\src\stm8"
#  File "${DEV_ROOT}\lib\src\stm8\Makefile"

  SetOutPath "$INSTDIR\lib\src\tlcs90"
  File "${DEV_ROOT}\lib\src\tlcs90\*.s"
#  File "${DEV_ROOT}\lib\src\tlcs90\Makefile"

  SetOutPath "$INSTDIR\lib\src\mcs51"
  File "${DEV_ROOT}\lib\src\mcs51\*.asm"
#  File "${DEV_ROOT}\lib\src\mcs51\Makefile"

  SetOutPath "$INSTDIR\lib\src\small"
#  File "${DEV_ROOT}\lib\src\small\Makefile"

  SetOutPath "$INSTDIR\lib\src\medium"
#  File "${DEV_ROOT}\lib\src\medium\Makefile"

  SetOutPath "$INSTDIR\lib\src\large"
#  File "${DEV_ROOT}\lib\src\large\Makefile"

  SetOutPath "$INSTDIR\lib\src\huge"
#  File "${DEV_ROOT}\lib\src\huge\Makefile"

  SetOutPath "$INSTDIR\lib\src\pic14"
#  File "${DEV_ROOT}\lib\src\pic14\configure"
#  File "${DEV_ROOT}\lib\src\pic14\configure.in"
#  File "${DEV_ROOT}\lib\src\pic14\GPL"
#  File "${DEV_ROOT}\lib\src\pic14\LGPL"
#  File "${DEV_ROOT}\lib\src\pic14\Makefile"
#  File "${DEV_ROOT}\lib\src\pic14\Makefile.common"
#  File "${DEV_ROOT}\lib\src\pic14\Makefile.common.in"
#  File "${DEV_ROOT}\lib\src\pic14\Makefile.rules"
#  File "${DEV_ROOT}\lib\src\pic14\Makefile.subdir"
#  File "${DEV_ROOT}\lib\src\pic14\NEWS"
#  File "${DEV_ROOT}\lib\src\pic14\README"
  File "${DEV_ROOT}\lib\src\pic14\TEMPLATE.c"
  File "${DEV_ROOT}\lib\src\pic14\TEMPLATE.S"

  SetOutPath "$INSTDIR\lib\src\pic14\libsdcc\regular"
  File "${DEV_ROOT}\lib\src\pic14\libsdcc\regular\*.c"
  File "${DEV_ROOT}\lib\src\pic14\libsdcc\regular\*.S"
  File "${DEV_ROOT}\lib\src\pic14\libsdcc\regular\*.inc"
#  File "${DEV_ROOT}\lib\src\pic14\libsdcc\Makefile"

  SetOutPath "$INSTDIR\lib\src\pic14\libsdcc\enhanced"
  File "${DEV_ROOT}\lib\src\pic14\libsdcc\enhanced\*.S"
  File "${DEV_ROOT}\lib\src\pic14\libsdcc\enhanced\*.inc"
#  File "${DEV_ROOT}\lib\src\pic14\libsdcc\Makefile"

  SetOutPath "$INSTDIR\non-free\lib\src\pic14\libdev"
  File "${DEV_ROOT}\non-free\lib\src\pic14\libdev\*.c"
#  File "${DEV_ROOT}\non-free\lib\src\pic14\libdev\Makefile"

  SetOutPath "$INSTDIR\lib\src\pic14\libm"
#  File "${DEV_ROOT}\lib\src\pic14\libm\*.c"

  SetOutPath "$INSTDIR\lib\src\pic16"
#  File "${DEV_ROOT}\lib\src\pic16\configure"
#  File "${DEV_ROOT}\lib\src\pic16\configure.in"
#  File "${DEV_ROOT}\lib\src\pic16\COPYING"
#  File "${DEV_ROOT}\lib\src\pic16\Makefile"
#  File "${DEV_ROOT}\lib\src\pic16\Makefile.common"
#  File "${DEV_ROOT}\lib\src\pic16\Makefile.common.in"
#  File "${DEV_ROOT}\lib\src\pic16\Makefile.rules"
#  File "${DEV_ROOT}\lib\src\pic16\Makefile.subdir"
#  File "${DEV_ROOT}\lib\src\pic16\pics.all"
#  File "${DEV_ROOT}\lib\src\pic16\pics.build"
#  File "${DEV_ROOT}\lib\src\pic16\README"

  SetOutPath "$INSTDIR\lib\src\pic16\debug"
#  File "${DEV_ROOT}\lib\src\pic16\debug\Makefile"

  SetOutPath "$INSTDIR\lib\src\pic16\debug\gstack"
#  File "${DEV_ROOT}\lib\src\pic16\debug\gstack\Makefile"
  File "${DEV_ROOT}\lib\src\pic16\debug\gstack\*.c"

  SetOutPath "$INSTDIR\lib\src\pic16\libc"
#  File "${DEV_ROOT}\lib\src\pic16\libc\Makefile"

  SetOutPath "$INSTDIR\lib\src\pic16\libc\ctype"
  File "${DEV_ROOT}\lib\src\pic16\libc\ctype\*.c"
#  File "${DEV_ROOT}\lib\src\pic16\libc\ctype\Makefile"

  SetOutPath "$INSTDIR\lib\src\pic16\libc\delay"
  File "${DEV_ROOT}\lib\src\pic16\libc\delay\*.S"
#  File "${DEV_ROOT}\lib\src\pic16\libc\delay\Makefile"

  SetOutPath "$INSTDIR\lib\src\pic16\libc\stdio"
  File "${DEV_ROOT}\lib\src\pic16\libc\stdio\*.c"
#  File "${DEV_ROOT}\lib\src\pic16\libc\stdio\Makefile"

  SetOutPath "$INSTDIR\lib\src\pic16\libc\stdlib"
  File "${DEV_ROOT}\lib\src\pic16\libc\stdlib\*.c"
  File "${DEV_ROOT}\lib\src\pic16\libc\stdlib\*.S"
#  File "${DEV_ROOT}\lib\src\pic16\libc\stdlib\Makefile"

  SetOutPath "$INSTDIR\lib\src\pic16\libc\string"
  File "${DEV_ROOT}\lib\src\pic16\libc\string\*.c"
#  File "${DEV_ROOT}\lib\src\pic16\libc\string\Makefile"

  SetOutPath "$INSTDIR\lib\src\pic16\libc\utils"
  File "${DEV_ROOT}\lib\src\pic16\libc\utils\*.S"
#  File "${DEV_ROOT}\lib\src\pic16\libc\utils\Makefile"

  SetOutPath "$INSTDIR\non-free\lib\src\pic16\libdev"
  File "${DEV_ROOT}\non-free\lib\src\pic16\libdev\*.c"
#  File "${DEV_ROOT}\non-free\lib\src\pic16\libdev\Makefile"

  SetOutPath "$INSTDIR\lib\src\pic16\libio"
  File "${DEV_ROOT}\lib\src\pic16\libio\*.ignore"
#  File "${DEV_ROOT}\lib\src\pic16\libio\Makefile"

  SetOutPath "$INSTDIR\lib\src\pic16\libio\adc"
  File "${DEV_ROOT}\lib\src\pic16\libio\adc\*.c"
#  File "${DEV_ROOT}\lib\src\pic16\libio\adc\Makefile"

  SetOutPath "$INSTDIR\lib\src\pic16\libio\i2c"
  File "${DEV_ROOT}\lib\src\pic16\libio\i2c\*.c"
#  File "${DEV_ROOT}\lib\src\pic16\libio\i2c\Makefile"

  SetOutPath "$INSTDIR\lib\src\pic16\libio\usart"
  File "${DEV_ROOT}\lib\src\pic16\libio\usart\*.c"
#  File "${DEV_ROOT}\lib\src\pic16\libio\usart\Makefile"

  SetOutPath "$INSTDIR\lib\src\pic16\libm"
  File "${DEV_ROOT}\lib\src\pic16\libm\*.c"
#  File "${DEV_ROOT}\lib\src\pic16\libm\Makefile"

  SetOutPath "$INSTDIR\lib\src\pic16\libsdcc"
#  File "${DEV_ROOT}\lib\src\pic16\libsdcc\Makefile"

  SetOutPath "$INSTDIR\lib\src\pic16\libsdcc\char"
  File "${DEV_ROOT}\lib\src\pic16\libsdcc\char\*.c"
#  File "${DEV_ROOT}\lib\src\pic16\libsdcc\char\Makefile"

  SetOutPath "$INSTDIR\lib\src\pic16\libsdcc\fixed16x16"
  File "${DEV_ROOT}\lib\src\pic16\libsdcc\fixed16x16\*.c"
  File "${DEV_ROOT}\lib\src\pic16\libsdcc\fixed16x16\*.S"
#  File "${DEV_ROOT}\lib\src\pic16\libsdcc\fixed16x16\Makefile"

  SetOutPath "$INSTDIR\lib\src\pic16\libsdcc\float"
  File "${DEV_ROOT}\lib\src\pic16\libsdcc\float\*.c"
#  File "${DEV_ROOT}\lib\src\pic16\libsdcc\float\Makefile"

  SetOutPath "$INSTDIR\lib\src\pic16\libsdcc\gptr"
  File "${DEV_ROOT}\lib\src\pic16\libsdcc\gptr\*.c"
#  File "${DEV_ROOT}\lib\src\pic16\libsdcc\gptr\Makefile"

  SetOutPath "$INSTDIR\lib\src\pic16\libsdcc\int"
  File "${DEV_ROOT}\lib\src\pic16\libsdcc\int\*.c"
#  File "${DEV_ROOT}\lib\src\pic16\libsdcc\int\Makefile"

  SetOutPath "$INSTDIR\lib\src\pic16\libsdcc\long"
  File "${DEV_ROOT}\lib\src\pic16\libsdcc\long\*.c"
#  File "${DEV_ROOT}\lib\src\pic16\libsdcc\long\Makefile"

  SetOutPath "$INSTDIR\lib\src\pic16\libsdcc\lregs"
  File "${DEV_ROOT}\lib\src\pic16\libsdcc\lregs\*.c"
#  File "${DEV_ROOT}\lib\src\pic16\libsdcc\lregs\Makefile"

  SetOutPath "$INSTDIR\lib\src\pic16\libsdcc\stack"
  File "${DEV_ROOT}\lib\src\pic16\libsdcc\stack\*.S"

  SetOutPath "$INSTDIR\lib\src\pic16\startup"
  File "${DEV_ROOT}\lib\src\pic16\startup\*.c"
#  File "${DEV_ROOT}\lib\src\pic16\startup\Makefile"
#  File "${DEV_ROOT}\lib\src\pic16\startup\README"

  SetOutPath "$INSTDIR\lib\src"
  File "${DEV_ROOT}\lib\src\*.c"
${SectionEnd}

${Section} "SDCC STM8 large model library" SEC26
  SectionIn 1 2
  SetOutPath "$INSTDIR\lib\stm8-large"
  File "${DEV_ROOT}\lib\stm8-large\*.*"
${SectionEnd}

${Section} "SDCC EZ80_Z80 library" SEC27
  SectionIn 1 2
  SetOutPath "$INSTDIR\lib\ez80_z80"
  File "${DEV_ROOT}\lib\ez80_z80\*.*"
${SectionEnd}

${Section} "SDCC PDK13 library" SEC28
  SectionIn 1 2
  SetOutPath "$INSTDIR\lib\pdk13"
  File "${DEV_ROOT}\lib\pdk13\*.*"
${SectionEnd}

${Section} "SDCC PDK14 library" SEC29
  SectionIn 1 2
  SetOutPath "$INSTDIR\lib\pdk14"
  File "${DEV_ROOT}\lib\pdk14\*.*"
${SectionEnd}

${Section} "SDCC PDK15 library" SEC30
  SectionIn 1 2
  SetOutPath "$INSTDIR\lib\pdk15"
  File "${DEV_ROOT}\lib\pdk15\*.*"
${SectionEnd}

${Section} "SDCC PDK15 stack-auto library" SEC31
  SectionIn 1 2
  SetOutPath "$INSTDIR\lib\pdk15-stack-auto"
  File "${DEV_ROOT}\lib\pdk15-stack-auto\*.*"
${SectionEnd}

${Section} "SDCC Z80N library" SEC32
  SectionIn 1 2
  SetOutPath "$INSTDIR\lib\z80n"
  File "${DEV_ROOT}\lib\z80n\*.*"
${SectionEnd}

${Section} "SDCC Rabbit 2000A library" SEC33
  SectionIn 1 2
  SetOutPath "$INSTDIR\lib\r2ka"
  File "${DEV_ROOT}\lib\r2ka\*.*"
${SectionEnd}

;--------------------------------
;Descriptions

;Language strings
LangString DESC_SEC01 ${LANG_ENGLISH} "SDCC application files"
LangString DESC_SEC02 ${LANG_ENGLISH} "ucSim application files"
LangString DESC_SEC03 ${LANG_ENGLISH} "SDCDB files"
LangString DESC_SEC04 ${LANG_ENGLISH} "SDCC documentation"
LangString DESC_SEC05 ${LANG_ENGLISH} "SDCC include files"
LangString DESC_SEC06 ${LANG_ENGLISH} "SDCC DS390 library"
LangString DESC_SEC07 ${LANG_ENGLISH} "SDCC DS400 library"
LangString DESC_SEC08 ${LANG_ENGLISH} "SDCC GBZ80 library"
LangString DESC_SEC09 ${LANG_ENGLISH} "SDCC Z180 library"
LangString DESC_SEC10 ${LANG_ENGLISH} "SDCC Rabbit 2000 library"
LangString DESC_SEC11 ${LANG_ENGLISH} "SDCC Rabbit 3000A library"
LangString DESC_SEC12 ${LANG_ENGLISH} "SDCC Z80 library"
LangString DESC_SEC13 ${LANG_ENGLISH} "SDCC mcs51 small model library"
LangString DESC_SEC14 ${LANG_ENGLISH} "SDCC mcs51 medium model library"
LangString DESC_SEC15 ${LANG_ENGLISH} "SDCC mcs51 large model library"
LangString DESC_SEC16 ${LANG_ENGLISH} "SDCC mcs51 huge model library"
LangString DESC_SEC17 ${LANG_ENGLISH} "SDCC mcs51 small-stack-auto model library"
LangString DESC_SEC18 ${LANG_ENGLISH} "SDCC mcs51 large-stack-auto model library"
LangString DESC_SEC19 ${LANG_ENGLISH} "SDCC HC08 library"
LangString DESC_SEC20 ${LANG_ENGLISH} "SDCC S08 library"
LangString DESC_SEC21 ${LANG_ENGLISH} "SDCC PIC16 library"
LangString DESC_SEC22 ${LANG_ENGLISH} "SDCC PIC14 library"
LangString DESC_SEC23 ${LANG_ENGLISH} "SDCC STM8 small library"
LangString DESC_SEC24 ${LANG_ENGLISH} "SDCC TLCS90 library"
LangString DESC_SEC25 ${LANG_ENGLISH} "SDCC library sources"
LangString DESC_SEC26 ${LANG_ENGLISH} "SDCC STM8 large model library"
LangString DESC_SEC27 ${LANG_ENGLISH} "SDCC EZ80_Z80 library"
LangString DESC_SEC28 ${LANG_ENGLISH} "SDCC PDK13 library"
LangString DESC_SEC29 ${LANG_ENGLISH} "SDCC PDK14 library"
LangString DESC_SEC30 ${LANG_ENGLISH} "SDCC PDK15 library"
LangString DESC_SEC31 ${LANG_ENGLISH} "SDCC PDK15 stack-auto library"
LangString DESC_SEC32 ${LANG_ENGLISH} "SDCC Z80N library"

;Assign language strings to sections
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC01} $(DESC_SEC01)
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC02} $(DESC_SEC02)
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC03} $(DESC_SEC03)
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC04} $(DESC_SEC04)
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC05} $(DESC_SEC05)
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC06} $(DESC_SEC06)
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC07} $(DESC_SEC07)
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC08} $(DESC_SEC08)
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC09} $(DESC_SEC09)
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC10} $(DESC_SEC10)
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC11} $(DESC_SEC11)
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC12} $(DESC_SEC12)
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC27} $(DESC_SEC27)
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC13} $(DESC_SEC13)
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC14} $(DESC_SEC14)
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC15} $(DESC_SEC15)
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC16} $(DESC_SEC16)
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC17} $(DESC_SEC17)
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC18} $(DESC_SEC18)
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC19} $(DESC_SEC19)
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC20} $(DESC_SEC20)
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC21} $(DESC_SEC21)
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC22} $(DESC_SEC22)
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC23} $(DESC_SEC23)
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC26} $(DESC_SEC26)
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC24} $(DESC_SEC24)
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC25} $(DESC_SEC25)
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC16} $(DESC_SEC26)
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC17} $(DESC_SEC27)
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC18} $(DESC_SEC28)
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC19} $(DESC_SEC29)
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC20} $(DESC_SEC30)
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC31} $(DESC_SEC31)
  !insertmacro MUI_DESCRIPTION_TEXT ${SEC32} $(DESC_SEC32)
!insertmacro MUI_FUNCTION_DESCRIPTION_END
;--------------------------------

${Section} -Icons SECICONS
!insertmacro MUI_STARTMENU_WRITE_BEGIN Application
  CreateDirectory "$SMPROGRAMS\$MUI_STARTMENUPAGE_VARIABLE"
  CreateShortCut "$SMPROGRAMS\$MUI_STARTMENUPAGE_VARIABLE\Uninstall SDCC.lnk" "$INSTDIR\uninstall.exe" 
!ifdef FULL_DOC
  CreateShortCut "$SMPROGRAMS\$MUI_STARTMENUPAGE_VARIABLE\Documentation.lnk" "$INSTDIR\doc\sdccman.pdf" "" "$INSTDIR\sdcc.ico" "" "" "" ""
  CreateShortCut "$SMPROGRAMS\$MUI_STARTMENUPAGE_VARIABLE\README.lnk" "$INSTDIR\doc\README.TXT" "" "$INSTDIR\sdcc.ico" "" "" "" ""
  CreateShortCut "$SMPROGRAMS\$MUI_STARTMENUPAGE_VARIABLE\Change Log.lnk" "$INSTDIR\doc\ChangeLog.txt" "" "$INSTDIR\sdcc.ico" "" "" "" ""
!else
  CreateShortCut "$SMPROGRAMS\$MUI_STARTMENUPAGE_VARIABLE\Documentation.lnk" "$INSTDIR\doc\README.TXT" "" "$INSTDIR\sdcc.ico" "" "" "" ""
  CreateShortCut "$SMPROGRAMS\$MUI_STARTMENUPAGE_VARIABLE\Change Log.lnk" "$INSTDIR\doc\ChangeLog_head.txt" "" "$INSTDIR\sdcc.ico" "" "" "" ""
!endif
  CreateShortCut "$SMPROGRAMS\$MUI_STARTMENUPAGE_VARIABLE\GPL 2 License.lnk" "$INSTDIR\COPYING.txt"
!insertmacro MUI_STARTMENU_WRITE_END
${SectionEnd}

${Section} -INI SECINI
  WriteIniStr "$SMPROGRAMS\$MUI_STARTMENUPAGE_VARIABLE\SDCC on the Web.url" "InternetShortcut" "URL" "http://sdcc.sourceforge.net/"
!ifdef FULL_DOC
  WriteIniStr "$SMPROGRAMS\$MUI_STARTMENUPAGE_VARIABLE\Latest Changes.url" "InternetShortcut" "URL" "http://svn.code.sf.net/p/sdcc/code/trunk/sdcc/ChangeLog"
!endif
${SectionEnd}

${Section} -PostInstall SECPOSTINSTALL
; Add SDCC bin directory to path if silent mode
  ${If} ${Silent}
    Call SDCC.AddBinToPath
  ${EndIf}

  WriteRegStr ${SDCC_ROOT_KEY} "Software\${PRODUCT_NAME}" "" $INSTDIR
!ifdef VER_MAJOR & VER_MINOR & VER_REVISION & VER_BUILD
  WriteRegDword ${SDCC_ROOT_KEY} "Software\${PRODUCT_NAME}" "VersionMajor" "${VER_MAJOR}"
  WriteRegDword ${SDCC_ROOT_KEY} "Software\${PRODUCT_NAME}" "VersionMinor" "${VER_MINOR}"
  WriteRegDword ${SDCC_ROOT_KEY} "Software\${PRODUCT_NAME}" "VersionRevision" "${VER_REVISION}"
  WriteRegDword ${SDCC_ROOT_KEY} "Software\${PRODUCT_NAME}" "VersionBuild" "${VER_BUILD}"
!endif

  WriteRegExpandStr ${UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninstall.exe"
  WriteRegExpandStr ${UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "InstallLocation" "$INSTDIR"
  WriteRegStr ${UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "${PRODUCT_NAME}"
  WriteRegStr ${UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Publisher" "sdcc.sourceforge.net"
  WriteRegStr ${UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "http://sdcc.sourceforge.net/"
  WriteRegStr ${UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "HelpLink" "http://sdcc.sourceforge.net/"
  WriteRegStr ${UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "URLUpdateInfo" "http://sdcc.sourceforge.net/"

  WriteUninstaller "$INSTDIR\uninstall.exe"
${SectionEnd}


;;;; Uninstaller code ;;;;

${Section} Uninstall SECUNINSTALL
  !insertmacro MUI_STARTMENU_GETFOLDER Application $MUI_STARTMENUPAGE_VARIABLE

  ${DebugMsg} "removing path $INSTDIR\bin"
  Push "$INSTDIR\bin"
  Call un.SDCC.RemoveFromPath

; Clean the registry
  DeleteRegKey ${UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"
  DeleteRegKey ${SDCC_ROOT_KEY} "Software\${PRODUCT_NAME}"

  Delete "$SMPROGRAMS\$MUI_STARTMENUPAGE_VARIABLE\GPL 2 License.lnk"
  Delete "$SMPROGRAMS\$MUI_STARTMENUPAGE_VARIABLE\Change Log.lnk"
!ifdef FULL_DOC
  Delete "$SMPROGRAMS\$MUI_STARTMENUPAGE_VARIABLE\Latest Changes.url"
  Delete "$SMPROGRAMS\$MUI_STARTMENUPAGE_VARIABLE\README.lnk"
!endif
  Delete "$SMPROGRAMS\$MUI_STARTMENUPAGE_VARIABLE\Documentation.lnk"
  Delete "$SMPROGRAMS\$MUI_STARTMENUPAGE_VARIABLE\Uninstall SDCC.lnk"
  Delete "$SMPROGRAMS\$MUI_STARTMENUPAGE_VARIABLE\SDCC on the Web.url"

  RMDir "$SMPROGRAMS\$MUI_STARTMENUPAGE_VARIABLE"

  Delete "$INSTDIR\lib\src\large\Makefile"

  Delete "$INSTDIR\lib\src\medium\Makefile"

  Delete "$INSTDIR\lib\src\small\Makefile"

  Delete "$INSTDIR\lib\src\mcs51\*.asm"
  Delete "$INSTDIR\lib\src\mcs51\Makefile"
  Delete "$INSTDIR\lib\src\mcs51\README"

  Delete "$INSTDIR\lib\src\hc08\*.c"
  Delete "$INSTDIR\lib\src\hc08\hc08.lib"
  Delete "$INSTDIR\lib\src\hc08\Makefile"

  Delete "$INSTDIR\lib\src\s08\*.c"
  Delete "$INSTDIR\lib\src\s08\s08.lib"
  Delete "$INSTDIR\lib\src\s08\Makefile"

  Delete "$INSTDIR\lib\src\stm8\stm8.lib"
  Delete "$INSTDIR\lib\src\stm8\Makefile"

  Delete "$INSTDIR\lib\src\stm8-large\stm8.lib"
  Delete "$INSTDIR\lib\src\stm8-large\Makefile"

  Delete "$INSTDIR\lib\src\z80\*.s"
  Delete "$INSTDIR\lib\src\z80\z80.lib"
  Delete "$INSTDIR\lib\src\z80\README"
  Delete "$INSTDIR\lib\src\z80\Makefile"

  Delete "$INSTDIR\lib\src\z180\*.s"
  Delete "$INSTDIR\lib\src\z180\z80.lib"
  Delete "$INSTDIR\lib\src\z180\README"
  Delete "$INSTDIR\lib\src\z180\Makefile"

  Delete "$INSTDIR\lib\src\gbz80\*.s"
  Delete "$INSTDIR\lib\src\gbz80\gbz80.lib"
  Delete "$INSTDIR\lib\src\gbz80\README"
  Delete "$INSTDIR\lib\src\gbz80\Makefile"

  Delete "$INSTDIR\lib\src\r2k\*.s"
  
  Delete "$INSTDIR\lib\src\r2ka\*.s"

  Delete "$INSTDIR\lib\src\r3ka\*.s"

  Delete "$INSTDIR\lib\src\ez80_z80\*.s"
  Delete "$INSTDIR\lib\src\ez80_z80\ez80_z80.lib"
  Delete "$INSTDIR\lib\src\ez80_z80\README"
  Delete "$INSTDIR\lib\src\ez80_z80\Makefile"

  Delete "$INSTDIR\lib\src\ds390\*.c"
  Delete "$INSTDIR\lib\src\ds390\libds390.lib"
  Delete "$INSTDIR\lib\src\ds390\Makefile.dep"
  Delete "$INSTDIR\lib\src\ds390\Makefile"
  Delete "$INSTDIR\lib\src\ds390\examples\MOVED"

  Delete "$INSTDIR\lib\src\ds400\*.c"
  Delete "$INSTDIR\lib\src\ds400\libds400.lib"
  Delete "$INSTDIR\lib\src\ds400\Makefile.dep"
  Delete "$INSTDIR\lib\src\ds400\Makefile"

  Delete "$INSTDIR\lib\src\pdk13\pdk13.lib"
  Delete "$INSTDIR\lib\src\pdk13\Makefile"

  Delete "$INSTDIR\lib\src\pdk14\pdk14.lib"
  Delete "$INSTDIR\lib\src\pdk14\Makefile"

  Delete "$INSTDIR\lib\src\pdk15\pdk15.lib"
  Delete "$INSTDIR\lib\src\pdk15\Makefile"

  Delete "$INSTDIR\lib\src\pdk15-stack-auto\pdk15.lib"
  Delete "$INSTDIR\lib\src\pdk15-stack-auto\Makefile"

  Delete "$INSTDIR\lib\src\tlcs90\*.s"
  Delete "$INSTDIR\lib\src\tlcs90\tlcs90.lib"
  Delete "$INSTDIR\lib\src\tlcs90\README"
  Delete "$INSTDIR\lib\src\tlcs90\Makefile"
  
  Delete "$INSTDIR\lib\src\z80n\*.s"
  Delete "$INSTDIR\lib\src\z80n\z80n.lib"
  Delete "$INSTDIR\lib\src\z80n\README"
  Delete "$INSTDIR\lib\src\z80n\Makefile"

  Delete "$INSTDIR\lib\src\*.c"

  Delete "$INSTDIR\lib\pic14\*.lib"

  Delete "$INSTDIR\non-free\lib\pic14\*.lib"

  Delete "$INSTDIR\lib\pic16\*.o"
  Delete "$INSTDIR\lib\pic16\*.lib"

  Delete "$INSTDIR\non-free\lib\pic16\*.lib"

  Delete "$INSTDIR\lib\hc08\*.lib"

  Delete "$INSTDIR\lib\s08\*.lib"

  Delete "$INSTDIR\lib\stm8\*.lib"

  Delete "$INSTDIR\lib\stm8-large\*.lib"

  Delete "$INSTDIR\lib\z80\*.rel"
  Delete "$INSTDIR\lib\z80\*.lib"

  Delete "$INSTDIR\lib\z180\*.rel"
  Delete "$INSTDIR\lib\z180\*.lib"

  Delete "$INSTDIR\lib\r2k\*.rel"
  Delete "$INSTDIR\lib\r2k\*.lib"

  Delete "$INSTDIR\lib\r2ka\*.rel"
  Delete "$INSTDIR\lib\r2ka\*.lib"
  
  Delete "$INSTDIR\lib\r3ka\*.rel"
  Delete "$INSTDIR\lib\r3ka\*.lib"

  Delete "$INSTDIR\lib\ez80_z80\*.rel"
  Delete "$INSTDIR\lib\ez80_z80\*.lib"

  Delete "$INSTDIR\lib\small\*.lib"

  Delete "$INSTDIR\lib\medium\*.lib"

  Delete "$INSTDIR\lib\large\*.lib"

  Delete "$INSTDIR\lib\small-stack-auto\*.lib"
  Delete "$INSTDIR\lib\large-stack-auto\*.lib"

  Delete "$INSTDIR\lib\gbz80\*.rel"
  Delete "$INSTDIR\lib\gbz80\*.lib"

  Delete "$INSTDIR\lib\ds390\*.lib"

  Delete "$INSTDIR\lib\ds400\*.lib"

  Delete "$INSTDIR\lib\pdk13\*.lib"

  Delete "$INSTDIR\lib\pdk14\*.lib"

  Delete "$INSTDIR\lib\pdk15\*.lib"

  Delete "$INSTDIR\lib\pdk15-stack-auto\*.lib"

  Delete "$INSTDIR\lib\tlcs90\*.rel"
  Delete "$INSTDIR\lib\tlcs90\*.lib"
  
  Delete "$INSTDIR\lib\z80n\*.rel"
  Delete "$INSTDIR\lib\z80n\*.lib"

  Delete "$INSTDIR\include\asm\z80\*.h"
  Delete "$INSTDIR\include\asm\r2k\*.h"
  Delete "$INSTDIR\include\asm\r3ka\*.h"
  Delete "$INSTDIR\include\asm\pic16\*.h"
  Delete "$INSTDIR\include\asm\pic14\*.h"
  Delete "$INSTDIR\include\asm\mcs51\*.h"
  Delete "$INSTDIR\include\asm\gbz80\*.h"
  Delete "$INSTDIR\include\asm\ds390\*.h"
  Delete "$INSTDIR\include\asm\stm8\*.h"
  Delete "$INSTDIR\include\asm\default\*.h"
  Delete "$INSTDIR\include\z180\*.h"
  Delete "$INSTDIR\include\pic14\*.h"
  Delete "$INSTDIR\include\pic14\*.txt"
  Delete "$INSTDIR\include\pic14\*.inc"
  Delete "$INSTDIR\non-free\include\pic14\*.h"
  Delete "$INSTDIR\include\pic16\*.h"
  Delete "$INSTDIR\non-free\include\pic16\*.h"
  Delete "$INSTDIR\include\pic16\*.txt"
  Delete "$INSTDIR\include\mcs51\*.h"
  Delete "$INSTDIR\include\hc08\*.h"
  Delete "$INSTDIR\include\ds400\*.h"
  Delete "$INSTDIR\include\ds390\*.h"
  Delete "$INSTDIR\include\*.h"

!ifndef FULL_DOC
  Delete "$INSTDIR\doc\README.TXT"
  Delete "$INSTDIR\doc\ChangeLog_head.txt"
!endif

  Delete "$INSTDIR\bin\sdasgb.exe"
  Delete "$INSTDIR\bin\sdas6808.exe"
  Delete "$INSTDIR\bin\sdasz80.exe"
  Delete "$INSTDIR\bin\sdas8051.exe"
  Delete "$INSTDIR\bin\sdas390.exe"
  Delete "$INSTDIR\bin\sdasrab.exe"
  Delete "$INSTDIR\bin\sdasstm8.exe"
  Delete "$INSTDIR\bin\sdaspdk13.exe"
  Delete "$INSTDIR\bin\sdaspdk14.exe"
  Delete "$INSTDIR\bin\sdaspdk15.exe"
  Delete "$INSTDIR\bin\sdastlcs90.exe"
  Delete "$INSTDIR\bin\sdld.exe"
  Delete "$INSTDIR\bin\sdldgb.exe"
  Delete "$INSTDIR\bin\sdld6808.exe"
  Delete "$INSTDIR\bin\sdldz80.exe"
  Delete "$INSTDIR\bin\sdldstm8.exe"
  Delete "$INSTDIR\bin\sdldpdk.exe"
  Delete "$INSTDIR\bin\sdar.exe"
  Delete "$INSTDIR\bin\sdranlib.exe"
  Delete "$INSTDIR\bin\sdnm.exe"
  Delete "$INSTDIR\bin\sdobjcopy.exe"
  Delete "$INSTDIR\bin\makebin.exe"
  Delete "$INSTDIR\bin\packihx.exe"
  Delete "$INSTDIR\bin\sdcc.exe"
  Delete "$INSTDIR\bin\sdcpp.exe"
  Delete "$INSTDIR\bin\as2gbmap.cmd"
  Delete "$INSTDIR\bin\readline5.dll"
!ifdef WIN64
  Delete "$INSTDIR\bin\libgcc_s_*-1.dll"
  Delete "$INSTDIR\bin\libstdc++-6.dll"
  Delete "$INSTDIR\bin\libwinpthread-1.dll"
!endif


  Delete "$INSTDIR\bin\s51.exe"
  Delete "$INSTDIR\bin\shc08.exe"
  Delete "$INSTDIR\bin\sz80.exe"
  Delete "$INSTDIR\bin\sstm8.exe"

  Delete "$INSTDIR\bin\sdcdb.exe"
  Delete "$INSTDIR\bin\sdcdb.el"
  Delete "$INSTDIR\bin\sdcdbsrc.el"

  Delete "$INSTDIR\COPYING.txt"
  Delete "$INSTDIR\COPYING3.txt"
  Delete "$INSTDIR\sdcc.ico"
  Delete "$INSTDIR\uninstall.exe"

  RMDir /r "$INSTDIR\lib\src\pic14"
  RMDir /r "$INSTDIR\non-free\lib\src\pic14"
  RMDir /r "$INSTDIR\lib\src\pic16"
  RMDir /r "$INSTDIR\non-free\lib\src\pic16"
  RMDir "$INSTDIR\lib\src\small"
  RMDir "$INSTDIR\lib\src\medium"
  RMDir "$INSTDIR\lib\src\large"
  RMDir "$INSTDIR\lib\src\mcs51"
  RMDir "$INSTDIR\lib\src\z80"
  RMDir "$INSTDIR\lib\src\z180"
  RMDir "$INSTDIR\lib\src\gbz80"
  RMDir "$INSTDIR\lib\src\r2k"
  RMDir "$INSTDIR\lib\src\r2ka"
  RMDir "$INSTDIR\lib\src\r3ka"
  RMDir "$INSTDIR\lib\src\ez80_z80"
  RMDir "$INSTDIR\lib\src\ds390\examples"
  RMDir "$INSTDIR\lib\src\ds390"
  RMDir "$INSTDIR\lib\src\ds400"
  RMDir "$INSTDIR\lib\src\hc08"
  RMDir "$INSTDIR\lib\src\s08"
  RMDir "$INSTDIR\lib\src\stm8"
  RMDir "$INSTDIR\lib\src\stm8-large"
  RMDir "$INSTDIR\lib\src\pdk13"
  RMDir "$INSTDIR\lib\src\pdk14"
  RMDir "$INSTDIR\lib\src\pdk15"
  RMDir "$INSTDIR\lib\src\pdk15-stack-auto"
  RMDir "$INSTDIR\lib\src\tlcs90"
  RMDir "$INSTDIR\lib\src\z80n"
  RMDir "$INSTDIR\lib\src"
  RMDir "$INSTDIR\non-free\lib\src"

  RMDir "$INSTDIR\lib\pic14"
  RMDir "$INSTDIR\non-free\lib\pic14"
  RMDir "$INSTDIR\lib\pic16"
  RMDir "$INSTDIR\non-free\lib\pic16"
  RMDir "$INSTDIR\lib\z80"
  RMDir "$INSTDIR\lib\z180"
  RMDir "$INSTDIR\lib\r2k"
  RMDir "$INSTDIR\lib\r2ka"
  RMDir "$INSTDIR\lib\r3ka"
  RMDir "$INSTDIR\lib\ez80_z80"
  RMDir "$INSTDIR\lib\small"
  RMDir "$INSTDIR\lib\medium"
  RMDir "$INSTDIR\lib\large"
  RMDir "$INSTDIR\lib\small-stack-auto"
  RMDir "$INSTDIR\lib\large-stack-auto"
  RMDir "$INSTDIR\lib\gbz80"
  RMDir "$INSTDIR\lib\ds390"
  RMDir "$INSTDIR\lib\ds400"
  RMDir "$INSTDIR\lib\hc08"
  RMDir "$INSTDIR\lib\s08"
  RMDir "$INSTDIR\lib\stm8"
  RMDir "$INSTDIR\lib\stm8-large"
  RMDir "$INSTDIR\lib\pdk13"
  RMDir "$INSTDIR\lib\pdk14"
  RMDir "$INSTDIR\lib\pdk15"
  RMDir "$INSTDIR\lib\pdk15-stack-auto"
  RMDir "$INSTDIR\lib\tlcs90"
  RMDir "$INSTDIR\lib\z80n"
  RMDir "$INSTDIR\lib"
  RMDir "$INSTDIR\non-free\lib"

  RMDir "$INSTDIR\include\asm\z80"
  RMDir "$INSTDIR\include\asm\r2k"
  RMDir "$INSTDIR\include\asm\r3ka"
  RMDir "$INSTDIR\include\asm\pic16"
  RMDir "$INSTDIR\non-free\include\asm\pic16"
  RMDir "$INSTDIR\include\asm\pic14"
  RMDir "$INSTDIR\non-free\include\asm\pic14"
  RMDir "$INSTDIR\include\asm\mcs51"
  RMDir "$INSTDIR\include\asm\gbz80"
  RMDir "$INSTDIR\include\asm\ds390"
  RMDir "$INSTDIR\include\asm\stm8"
  RMDir "$INSTDIR\include\asm\default"
  RMDir "$INSTDIR\include\asm"
  RMDir "$INSTDIR\include\z180"
  RMDir "$INSTDIR\include\pic14"
  RMDir "$INSTDIR\non-free\include\pic14"
  RMDir "$INSTDIR\include\pic16"
  RMDir "$INSTDIR\non-free\include\pic16"
  RMDir "$INSTDIR\include\mcs51"
  RMDir "$INSTDIR\include\hc08"
  RMDir "$INSTDIR\include\ds400"
  RMDir "$INSTDIR\include\ds390"
  RMDir "$INSTDIR\include"
  RMDir "$INSTDIR\non-free\include"

  RMDir "$INSTDIR\non-free"

!ifdef FULL_DOC
  RMDir /r "$INSTDIR\doc"
!else
  RMDir "$INSTDIR\doc"
!endif

  RMDir "$INSTDIR\bin"

  RMDir "$INSTDIR"
;;;;  SetAutoClose true
${SectionEnd}

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Path Manipulation functions                                                 ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

!verbose 3
!include "WinMessages.nsh"
!verbose 4

; AddToPath - Adds the given dir to the search path.
;        Input - head of the stack
;        Note - Win9x systems requires reboot

${Function} SDCC.AddToPath
  Exch $0
  Push $1
  Push $2
  Push $3

  ; don't add if the path doesn't exist
  ${If} ${FileExists} $0
    ${If} ${IsNT}
      ; On NT: read PATH from registry
      ReadRegStr $1 HKCU "Environment" "PATH"
    ${Else}
      ; Not on NT: read PATH from environment variable
      ReadEnvStr $1 PATH
    ${EndIf}

    ${StrStr} $2 "$1;" "$0;"
    ${If} $2 == ""
      ${StrStr} $2 "$1;" "$0\;"
      ${If} $2 == ""
        GetFullPathName /SHORT $3 $0
        ${StrStr} $2 "$1;" "$3;"
        ${If} $2 == ""
          ${StrStr} $2 "$1;" "$03\;"
          ${If} $2 == ""
            ${If} ${IsNT}
              ;System PATH variable is at:
              ;HKLM "/SYSTEM/CurrentControlSet/Control/Session Manager/Environment" "Path"
              ReadRegStr $1 HKCU "Environment" "PATH"
              StrCpy $2 $1 1 -1  ; copy last char
              ${If} $2 == ";"    ; if last char == ;
                StrCpy $1 $1 -1  ; remove last char
              ${Endif}
              ${If} $1 != ""
                StrCpy $0 "$1;$0"
              ${Endif}
              WriteRegExpandStr HKCU "Environment" "PATH" $0
              SendMessage ${HWND_BROADCAST} ${WM_WININICHANGE} 0 "STR:Environment" /TIMEOUT=5000
            ${Else}
              ; Not on NT
              StrCpy $1 $WINDIR 2
              FileOpen $1 "$1\autoexec.bat" a
              FileSeek $1 -1 END
              FileReadByte $1 $2
              ${If} $2 = 26        ; DOS EOF
                FileSeek $1 -1 END ; write over EOF
              ${Endif}
              ${DebugMsg} "adding line $\r$\nSET PATH=%PATH%;$3$\r$\n"
              FileWrite $1 "$\r$\nSET PATH=%PATH%;$3$\r$\n"
              FileClose $1
              ${DebugMsg} "SetRebootFlag true"
              SetRebootFlag true
            ${Endif}
          ${Endif}
        ${Endif}
      ${Endif}
    ${Endif}
  ${EndIf}

  Pop $3
  Pop $2
  Pop $1
  Pop $0
${FunctionEnd}

; RemoveFromPath - Remove a given dir from the path
;     Input: head of the stack

!macro SDCC.RemoveFromPath un
${Function} ${un}SDCC.RemoveFromPath
  Exch $0
  Push $1
  Push $2
  Push $3
  Push $4
  Push $5
  Push $6

  IntFmt $6 "%c" 26 ; DOS EOF

  ${If} ${IsNT}
    ;System PATH variable is at:
    ;HKLM "/SYSTEM/CurrentControlSet/Control/Session Manager/Environment" "Path"
    ReadRegStr $1 HKCU "Environment" "PATH"
    StrCpy $5 $1 1 -1 ; copy last char
    ${If} $5 != ";"   ; if last char != ;
      StrCpy $1 "$1;" ; append ;
    ${EndIf}
    Push $1
    Push "$0;"
    Call ${un}StrStr  ; Find `$0;` in $1
    Pop $2            ; pos of our dir
    ${If} $2 != ""
      ; it is in path:
      ; $0 - path to add
      ; $1 - path var
      StrLen $3 "$0;"
      StrLen $4 $2
      StrCpy $5 $1 -$4   ; $5 is now the part before the path to remove
      StrCpy $6 $2 "" $3 ; $6 is now the part after the path to remove
      StrCpy $3 $5$6

      StrCpy $5 $3 1 -1  ; copy last char
      ${If} $5 == ";"    ; if last char == ;
        StrCpy $3 $3 -1  ; remove last char
      ${EndIf}
      ${If} $3 != ""
        ; New PATH not empty: update the registry
        WriteRegExpandStr HKCU "Environment" "PATH" $3
      ${Else}
        ; New PATH empty: remove from the registry
        DeleteRegValue HKCU "Environment" "PATH"
      ${EndIf}
      SendMessage ${HWND_BROADCAST} ${WM_WININICHANGE} 0 "STR:Environment" /TIMEOUT=5000
    ${Endif}
  ${Else}
    ; Not on NT
    StrCpy $1 $WINDIR 2
    FileOpen $1 "$1\autoexec.bat" r
    GetTempFileName $4
    FileOpen $2 $4 w
    GetFullPathName /SHORT $0 $0
    StrCpy $0 "SET PATH=%PATH%;$0"

  nextLine:
    ; copy all lines except the line containing "SET PATH=%PATH%;$0"
    ; from autoexec.bat to the temporary file
    ClearErrors
    FileRead $1 $3
    ${IfNot} ${Errors}
      StrCpy $5 $3 1 -1 ; read last char
      ${If} $5 == $6    ; if DOS EOF
        StrCpy $3 $3 -1 ; remove DOS EOF so we can compare
      ${EndIf}
      ${If} $3 != "$0$\r$\n"
        ${AndIf} $3 != "$0$\n"
        ${AndIf} $3 != "$0"
        FileWrite $2 $3
        Goto nextLine
      ${Else}
        ; This is the line I'm looking for:
        ; don't copy it
        ${DebugMsg} "removing line $0"
        ${DebugMsg} "SetRebootFlag true"
        SetRebootFlag true
        Goto nextLine
      ${EndIf}
    ${EndIf}

    FileClose $2
    FileClose $1
    StrCpy $1 $WINDIR 2
    Delete "$1\autoexec.bat"
    CopyFiles /SILENT $4 "$1\autoexec.bat"
    Delete $4
  ${Endif}

  Pop $6
  Pop $5
  Pop $4
  Pop $3
  Pop $2
  Pop $1
  Pop $0
${FunctionEnd}
!macroend
!insertmacro SDCC.RemoveFromPath ""
!insertmacro SDCC.RemoveFromPath "un."

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;  Uninstall/Reinstall page functions                                         ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

!ifdef VER_MAJOR & VER_MINOR & VER_REVISION & VER_BUILD

Var ReinstallPageCheck

${Function} SDCC.PageReinstall

  ReadRegStr $R0 ${SDCC_ROOT_KEY} "Software\${PRODUCT_NAME}" ""

  ${If} $R0 == ""
    ReadRegStr $R0 ${UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString"
    ${If} $R0 == ""
      Abort
    ${EndIf}
  ${EndIf}

  ReadRegDWORD $R0 ${SDCC_ROOT_KEY} "Software\${PRODUCT_NAME}" "VersionMajor"
  ReadRegDWORD $R1 ${SDCC_ROOT_KEY} "Software\${PRODUCT_NAME}" "VersionMinor"
  ReadRegDWORD $R2 ${SDCC_ROOT_KEY} "Software\${PRODUCT_NAME}" "VersionRevision"
  ReadRegDWORD $R3 ${SDCC_ROOT_KEY} "Software\${PRODUCT_NAME}" "VersionBuild"
  StrCpy $R0 $R0.$R1.$R2.$R3

  ${VersionCompare} ${VER_MAJOR}.${VER_MINOR}.${VER_REVISION}.${VER_BUILD} $R0 $R0
  ${If} $R0 == 0
    StrCpy $R1 "${PRODUCT_NAME} ${PRODUCT_VERSION} is already installed. Select the operation you want to perform and click Next to continue."
    StrCpy $R2 "Add/Reinstall components"
    StrCpy $R3 "Uninstall ${PRODUCT_NAME}"
    !insertmacro MUI_HEADER_TEXT "Already Installed" "Choose the maintenance option to perform."
    StrCpy $R0 "2"
  ${ElseIf} $R0 == 1
    StrCpy $R1 "An older version of ${PRODUCT_NAME} is installed on your system. It's recommended that you uninstall the current version before installing. Select the operation you want to perform and click Next to continue."
    StrCpy $R2 "Uninstall before installing"
    StrCpy $R3 "Do not uninstall"
    !insertmacro MUI_HEADER_TEXT "Already Installed" "Choose how you want to install ${PRODUCT_NAME}."
    StrCpy $R0 "1"
  ${ElseIf} $R0 == 2
    StrCpy $R1 "A newer version of ${PRODUCT_NAME} is already installed! It is not recommended that you install an older version. If you really want to install this older version, it's better to uninstall the current version first. Select the operation you want to perform and click Next to continue."
    StrCpy $R2 "Uninstall before installing"
    StrCpy $R3 "Do not uninstall"
    !insertmacro MUI_HEADER_TEXT "Already Installed" "Choose how you want to install ${PRODUCT_NAME}."
    StrCpy $R0 "1"
  ${Else}
    Abort
  ${EndIf}

  nsDialogs::Create /NOUNLOAD 1018

  ${NSD_CreateLabel} 0 0 100% 24u $R1
  Pop $R1

  ${NSD_CreateRadioButton} 30u 50u -30u 8u $R2
  Pop $R2
  ${NSD_OnClick} $R2 SDCC.PageReinstallUpdateSelection

  ${NSD_CreateRadioButton} 30u 70u -30u 8u $R3
  Pop $R3
  ${NSD_OnClick} $R3 SDCC.PageReinstallUpdateSelection

  ${If} $ReinstallPageCheck != 2
    SendMessage $R2 ${BM_SETCHECK} ${BST_CHECKED} 0
  ${Else}
    SendMessage $R3 ${BM_SETCHECK} ${BST_CHECKED} 0
  ${EndIf}

  nsDialogs::Show

${FunctionEnd}

${Function} SDCC.PageReinstallUpdateSelection

  Pop $R1

  ${NSD_GetState} $R2 $R1

  ${If} $R1 == ${BST_CHECKED}
    StrCpy $ReinstallPageCheck 1
  ${Else}
    StrCpy $ReinstallPageCheck 2
  ${EndIf}

${FunctionEnd}

${Function} SDCC.PageLeaveReinstall

  ${NSD_GetState} $R2 $R1

  ${DebugMsg} "R0 = $R0, R1 = $R1, R2 = $R2"

  ${If} $R0 == "1"
    ${AndIf} $R1 != "1"
    Goto reinst_done
  ${EndIf}

  ${If} $R0 == "2"
    ${AndIf} $R1 == 1
    Goto reinst_done
  ${EndIf}

  ReadRegStr $R1 ${UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString"

  ;Run uninstaller
  HideWindow

  ${If} $R0 == "2"
    ; Uninstall only: uninstaller should be removed
    ClearErrors
    ; ExecWait doesn't wait if _?=$INSTDIR is not defined!
    ExecWait '$R1'
    Quit
  ${Else}
    ; Uninstal & Reinstall: uninstaller will be rewritten
    ClearErrors
    ; ExecWait doesn't wait if _?=$INSTDIR is not defined!
    ExecWait '$R1 _?=$INSTDIR'
  ${EndIf}

  BringToFront

reinst_done:

${FunctionEnd}

!endif # VER_MAJOR & VER_MINOR & VER_REVISION & VER_BUILD
