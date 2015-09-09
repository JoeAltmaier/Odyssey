# Microsoft Developer Studio Project File - Name="filesystem" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=filesystem - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "filesystem.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "filesystem.mak" CFG="filesystem - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "filesystem - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "filesystem - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "filesystem - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "filesystem - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /Zp4 /MTd /W3 /Gm /GX /ZI /Od /I "..\..\include\FileSystem" /I "..\..\include" /I "..\..\localization" /I "..\..\include\oos" /I "..\..\include\cttables" /I "..\..\include\pts" /I "..\..\cmdqueues" /I "..\..\include\fcp" /I "..\..\include\drivers" /I "..\ssapi_server\\" /I "..\ssapi_server\utils" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\bin\win\filesystem.lib"

!ENDIF 

# Begin Target

# Name "filesystem - Win32 Release"
# Name "filesystem - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "Table Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Include\CTTables\FileDescriptorTable.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Include\CTTables\FileTable.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Include\CTTables\PtsRecordBase.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\FileSystem.cpp
# End Source File
# Begin Source File

SOURCE=.\FileSystemInfo.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Group "Table Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Include\CTTables\FileDescriptorTable.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\CTTables\FileTable.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\CTTables\PtsRecordBase.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\Include\FileSystem\FileSystem.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\FileSystem\FileSystemInfo.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\FileSystem\FileSystemMessages.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\Pts\RqPts.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\Pts\RqPts_T.h
# End Source File
# End Group
# End Target
# End Project
