# Microsoft Developer Studio Project File - Name="dynlib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=dynlib - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "dynlib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "dynlib.mak" CFG="dynlib - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "dynlib - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "dynlib - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "dynlib - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "Lib/Win32" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "WIN32DLL" /D "LITT_ENDIAN" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x40c /d "NDEBUG"
# ADD RSC /l 0x40c /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib shell32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"Bin/Win32/Release/adflibd.dll"

!ELSEIF  "$(CFG)" == "dynlib - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "Lib/Win32" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "WIN32DLL" /D "LITT_ENDIAN" /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x40c /d "_DEBUG"
# ADD RSC /l 0x40c /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"Bin/Win32/Debug/adflibd.dll" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "dynlib - Win32 Release"
# Name "dynlib - Win32 Debug"
# Begin Source File

SOURCE=.\Lib\adf_bitm.c
# End Source File
# Begin Source File

SOURCE=.\Lib\adf_bitm.h
# End Source File
# Begin Source File

SOURCE=.\Lib\adf_blk.h
# End Source File
# Begin Source File

SOURCE=.\Lib\adf_cache.c
# End Source File
# Begin Source File

SOURCE=.\Lib\adf_cache.h
# End Source File
# Begin Source File

SOURCE=.\Lib\adf_defs.h
# End Source File
# Begin Source File

SOURCE=.\Lib\adf_dir.c
# End Source File
# Begin Source File

SOURCE=.\Lib\adf_dir.h
# End Source File
# Begin Source File

SOURCE=.\Lib\adf_disk.c
# End Source File
# Begin Source File

SOURCE=.\Lib\adf_disk.h
# End Source File
# Begin Source File

SOURCE=.\Lib\adf_dump.c
# End Source File
# Begin Source File

SOURCE=.\Lib\adf_dump.h
# End Source File
# Begin Source File

SOURCE=.\Lib\adf_env.c
# End Source File
# Begin Source File

SOURCE=.\Lib\adf_env.h
# End Source File
# Begin Source File

SOURCE=.\Lib\adf_err.h
# End Source File
# Begin Source File

SOURCE=.\Lib\adf_file.c
# End Source File
# Begin Source File

SOURCE=.\Lib\adf_file.h
# End Source File
# Begin Source File

SOURCE=.\Lib\adf_hd.c
# End Source File
# Begin Source File

SOURCE=.\Lib\adf_hd.h
# End Source File
# Begin Source File

SOURCE=.\Lib\adf_link.c
# End Source File
# Begin Source File

SOURCE=.\Lib\adf_link.h
# End Source File
# Begin Source File

SOURCE=.\Lib\Win32\adf_nativ.c
# End Source File
# Begin Source File

SOURCE=.\Lib\Win32\adf_nativ.h
# End Source File
# Begin Source File

SOURCE=.\Lib\adf_raw.c
# End Source File
# Begin Source File

SOURCE=.\Lib\adf_raw.h
# End Source File
# Begin Source File

SOURCE=.\Lib\adf_salv.c
# End Source File
# Begin Source File

SOURCE=.\Lib\adf_salv.h
# End Source File
# Begin Source File

SOURCE=.\Lib\adf_str.h
# End Source File
# Begin Source File

SOURCE=.\Lib\adf_util.c
# End Source File
# Begin Source File

SOURCE=.\Lib\adf_util.h
# End Source File
# Begin Source File

SOURCE=.\Lib\Win32\defendian.h
# End Source File
# Begin Source File

SOURCE=.\Lib\hd_blk.h
# End Source File
# Begin Source File

SOURCE=.\Lib\Win32\nt4_dev.c
# End Source File
# Begin Source File

SOURCE=.\Lib\Win32\nt4_dev.h
# End Source File
# Begin Source File

SOURCE=.\Lib\prefix.h
# End Source File
# End Target
# End Project
