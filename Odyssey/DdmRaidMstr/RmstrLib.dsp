# Microsoft Developer Studio Project File - Name="RmstrLib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=RmstrLib - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "RmstrLib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "RmstrLib.mak" CFG="RmstrLib - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "RmstrLib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "RmstrLib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "RmstrLib - Win32 Release"

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

!ELSEIF  "$(CFG)" == "RmstrLib - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "RmstrLib___Win32_Debug"
# PROP BASE Intermediate_Dir "RmstrLib___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "RmstrLib___Win32_Debug"
# PROP Intermediate_Dir "RmstrLib___Win32_Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /Zp4 /MTd /W3 /Gm /GX /ZI /Od /I "..\DdmVCM" /I "..\DdmRaidMstr" /I "..\..\include\raid" /I "..\include" /I "..\..\include\CmdQueues" /I "..\..\include" /I "..\..\localization" /I "..\..\include\oos" /I "..\..\include\cttables" /I "..\..\include\pts" /I "..\..\cmdqueues" /I "..\..\include\fcp" /I "..\..\include\drivers" /I "..\ssapi_server\\" /I "..\ssapi_server\utils" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\bin\win\RmstrLib.lib"

!ENDIF 

# Begin Target

# Name "RmstrLib - Win32 Release"
# Name "RmstrLib - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\DdmRaidMstr.cpp
# End Source File
# Begin Source File

SOURCE=.\HelperServices.cpp
# End Source File
# Begin Source File

SOURCE=.\RaidDdmInterface.cpp
# End Source File
# Begin Source File

SOURCE=.\RmstrAbortUtil.cpp
# End Source File
# Begin Source File

SOURCE=.\RmstrAddMembers.cpp
# End Source File
# Begin Source File

SOURCE=.\RmstrAlarms.cpp
# End Source File
# Begin Source File

SOURCE=.\RmstrCapabilities.cpp
# End Source File
# Begin Source File

SOURCE=.\RmstrCapabilityData.cpp
# End Source File
# Begin Source File

SOURCE=.\RmstrChangeArrayName.cpp
# End Source File
# Begin Source File

SOURCE=.\RmstrChangePreferredMember.cpp
# End Source File
# Begin Source File

SOURCE=.\RmstrChangePriority.cpp
# End Source File
# Begin Source File

SOURCE=.\RmstrChangeSourceMember.cpp
# End Source File
# Begin Source File

SOURCE=.\RmstrCommitSpare.cpp
# End Source File
# Begin Source File

SOURCE=.\RmstrCreateArray.cpp
# End Source File
# Begin Source File

SOURCE=.\RmstrData.cpp
# End Source File
# Begin Source File

SOURCE=.\RmstrDefineTables.cpp
# End Source File
# Begin Source File

SOURCE=.\RmstrDeleteArray.cpp
# End Source File
# Begin Source File

SOURCE=.\RmstrDeleteSpare.cpp
# End Source File
# Begin Source File

SOURCE=.\RmstrDownAMember.cpp
# End Source File
# Begin Source File

SOURCE=.\RmstrFakeRaidDdm.cpp
# End Source File
# Begin Source File

SOURCE=.\RmstrInternalCommands.cpp
# End Source File
# Begin Source File

SOURCE=.\RmstrLogEvents.cpp
# End Source File
# Begin Source File

SOURCE=.\RmstrProcessArrayOfflineEvent.cpp
# End Source File
# Begin Source File

SOURCE=.\RmstrProcessDownMemberEvent.cpp
# End Source File
# Begin Source File

SOURCE=.\RmstrProcessStopUtilEvent.cpp
# End Source File
# Begin Source File

SOURCE=.\RmstrRemoveMember.cpp
# End Source File
# Begin Source File

SOURCE=.\RmstrServices.cpp
# End Source File
# Begin Source File

SOURCE=.\RmstrSpares.cpp
# End Source File
# Begin Source File

SOURCE=.\RmstrStateChecker.cpp
# End Source File
# Begin Source File

SOURCE=.\RmstrUtils.cpp
# End Source File
# Begin Source File

SOURCE=.\TableServices.cpp
# End Source File
# Begin Source File

SOURCE=.\TableTest.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\HelperServices.h
# End Source File
# Begin Source File

SOURCE=.\RmstrInternalCommands.h
# End Source File
# Begin Source File

SOURCE=.\TableServices.h
# End Source File
# End Group
# Begin Group "SSAPIUtils"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\SSAPI_Server\Utils\ShadowTable.cpp
# End Source File
# Begin Source File

SOURCE=..\SSAPI_Server\Utils\ShadowTable.h
# End Source File
# Begin Source File

SOURCE=..\SSAPI_Server\Utils\StringClass.cpp
# End Source File
# Begin Source File

SOURCE=..\SSAPI_Server\Utils\StringClass.h
# End Source File
# Begin Source File

SOURCE=..\SSAPI_Server\Utils\StringResourceManager.cpp
# End Source File
# Begin Source File

SOURCE=..\SSAPI_Server\Utils\StringResourceManager.h
# End Source File
# Begin Source File

SOURCE=..\SSAPI_Server\Utils\UnicodeString.cpp
# End Source File
# Begin Source File

SOURCE=..\SSAPI_Server\Utils\UnicodeString.h
# End Source File
# End Group
# Begin Group "Tables"

# PROP Default_Filter "*.cpp"
# Begin Source File

SOURCE=..\..\Include\CTTables\ArrayDescriptor.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Include\CTTables\RAIDMemberDescriptor.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Include\CTTables\RaidSpareDescriptor.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Include\CTTables\RAIDUtilDescriptor.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Include\CTTables\RmstrCapabilityTable.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Include\CTTables\StorageRollCallTable.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Include\CTTables\UnicodeString128Table.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Include\CTTables\UnicodeString128Table.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\CTTables\UnicodeString16Table.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Include\CTTables\UnicodeString16Table.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\CTTables\UnicodeString256Table.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Include\CTTables\UnicodeString256Table.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\CTTables\UnicodeString32Table.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Include\CTTables\UnicodeString32Table.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\CTTables\UnicodeString64Table.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Include\CTTables\UnicodeString64Table.h
# End Source File
# End Group
# End Target
# End Project
