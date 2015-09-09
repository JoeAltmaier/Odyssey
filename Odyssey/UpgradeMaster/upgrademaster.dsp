# Microsoft Developer Studio Project File - Name="upgrademaster" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=upgrademaster - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "upgrademaster.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "upgrademaster.mak" CFG="upgrademaster - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "upgrademaster - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "upgrademaster - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "upgrademaster - Win32 Release"

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

!ELSEIF  "$(CFG)" == "upgrademaster - Win32 Debug"

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
# ADD CPP /nologo /Zp4 /MTd /W3 /Gm /GX /ZI /Od /I "..\..\include\UpgradeMaster" /I "..\..\include\FileSystem" /I "..\..\include" /I "..\..\localization" /I "..\..\include\oos" /I "..\..\include\cttables" /I "..\..\include\pts" /I "..\..\cmdqueues" /I "..\..\include\fcp" /I "..\..\include\drivers" /I "..\ssapi_server\\" /I "..\ssapi_server\utils" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\bin\win\upgrademaster.lib"

!ENDIF 

# Begin Target

# Name "upgrademaster - Win32 Release"
# Name "upgrademaster - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "Table Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Include\CTTables\DefaultImageTable.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Include\CTTables\ImageDescriptorTable.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Include\CTTables\IOPImageTable.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Include\CTTables\IOPStatusTable.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\DdmUpgrade.cpp
# End Source File
# Begin Source File

SOURCE=.\ImageIterator.cpp
# End Source File
# Begin Source File

SOURCE=.\UpgradeCmdQueue.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Group "Table Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Include\CTTables\DefaultImageTable.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\CTTables\ImageDescriptorTable.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\CTTables\IOPImageTable.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\Include\UpgradeMaster\DdmUpgrade.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\UpgradeMaster\ImageIterator.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\UpgradeMaster\UpgradeCmdQueue.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\UpgradeMaster\UpgradeEvents.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\UpgradeMaster\UpgradeImageType.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\UpgradeMaster\UpgradeMasterCommands.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\UpgradeMaster\UpgradeMasterMessages.h
# End Source File
# End Group
# End Target
# End Project
