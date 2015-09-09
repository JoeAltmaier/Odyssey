# Microsoft Developer Studio Project File - Name="Oos" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=Oos - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Oos.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Oos.mak" CFG="Oos - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Oos - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "Oos - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Oos - Win32 Release"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "Oos - Win32 Debug"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /YX /FD /GZ /c
# ADD CPP /nologo /Zp4 /MTd /W3 /Gm /GX /ZI /Od /I "..\..\include" /I "..\..\localization" /I "..\..\include\oos" /I "..\..\include\cttables" /I "..\..\include\pts" /I "..\..\cmdqueues" /I "..\..\include\fcp" /I "..\..\include\drivers" /I "..\ssapi_server\\" /I "..\ssapi_server\utils" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\bin\win\oos.lib"

!ENDIF 

# Begin Target

# Name "Oos - Win32 Release"
# Name "Oos - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\Address.cpp
# End Source File
# Begin Source File

SOURCE=.\Application_Initialize.cpp
# End Source File
# Begin Source File

SOURCE=.\BootTable.cpp
# End Source File
# Begin Source File

SOURCE=.\ClassTable.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Include\CTTables\ClassTableTable.cpp
# End Source File
# Begin Source File

SOURCE=.\Critical.cpp
# End Source File
# Begin Source File

SOURCE=.\Ddm.cpp
# End Source File
# Begin Source File

SOURCE=.\DdmLeak.cpp
# End Source File
# Begin Source File

SOURCE=.\DdmManager.cpp
# End Source File
# Begin Source File

SOURCE=.\DdmNull.cpp
# End Source File
# Begin Source File

SOURCE=.\DdmOsServices.cpp
# End Source File
# Begin Source File

SOURCE=.\DdmProfile.cpp
# End Source File
# Begin Source File

SOURCE=.\DdmPtsLoader.cpp
# End Source File
# Begin Source File

SOURCE=.\DdmSysInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\DdmTimer.cpp
# End Source File
# Begin Source File

SOURCE=.\DdmVirtualManager.cpp
# End Source File
# Begin Source File

SOURCE=.\DdmVirtualMaster.cpp
# End Source File
# Begin Source File

SOURCE=.\DeviceId.cpp
# End Source File
# Begin Source File

SOURCE=.\DeviceTable.cpp
# End Source File
# Begin Source File

SOURCE=.\DidMan.cpp
# End Source File
# Begin Source File

SOURCE=.\FailSafe.cpp
# End Source File
# Begin Source File

SOURCE=.\FailTable.cpp
# End Source File
# Begin Source File

SOURCE=..\Msl\HeapBlock.cpp
# End Source File
# Begin Source File

SOURCE=..\Msl\HeapNoFrag.cpp
# End Source File
# Begin Source File

SOURCE=.\Message.cpp
# End Source File
# Begin Source File

SOURCE=.\MessageBroker.cpp
# End Source File
# Begin Source File

SOURCE=.\Messenger.cpp
# End Source File
# Begin Source File

SOURCE=.\Os.cpp
# End Source File
# Begin Source File

SOURCE=..\Msl\OsHeap.cpp
# End Source File
# Begin Source File

SOURCE=.\PersistentData.cpp
# End Source File
# Begin Source File

SOURCE=.\PtsTable.cpp
# End Source File
# Begin Source File

SOURCE=.\RequestCodes.c

!IF  "$(CFG)" == "Oos - Win32 Release"

!ELSEIF  "$(CFG)" == "Oos - Win32 Debug"

# SUBTRACT CPP /YX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ServeTable.cpp
# End Source File
# Begin Source File

SOURCE=.\SuspendTable.cpp
# End Source File
# Begin Source File

SOURCE=.\SystemTable.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Include\CTTables\VirtualDeviceTable.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Include\CTTables\VirtualRouteTable.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Include\CTTables\VirtualStateTable.cpp
# End Source File
# Begin Source File

SOURCE=.\VirtualTable.cpp
# End Source File
# Begin Source File

SOURCE=.\win_kernel.cpp
# End Source File
# Begin Source File

SOURCE=.\win_mips.util.cpp
# End Source File
# Begin Source File

SOURCE=.\win_timer.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\Include\Oos\Address.h
# End Source File
# Begin Source File

SOURCE=.\Array_t.h
# End Source File
# Begin Source File

SOURCE=.\BootTable.h
# End Source File
# Begin Source File

SOURCE=.\ClassTable.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\Oos\Critical.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\Oos\Ddm.h
# End Source File
# Begin Source File

SOURCE=.\DdmManager.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\Oos\DdmOsServices.h
# End Source File
# Begin Source File

SOURCE=.\DdmSysInfo.h
# End Source File
# Begin Source File

SOURCE=.\DdmTimer.h
# End Source File
# Begin Source File

SOURCE=.\DeviceTable.h
# End Source File
# Begin Source File

SOURCE=.\DidMan.h
# End Source File
# Begin Source File

SOURCE=.\DispatchTables.h
# End Source File
# Begin Source File

SOURCE=..\Msl\HeapBlock.h
# End Source File
# Begin Source File

SOURCE=..\Msl\HeapNoFrag.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\Oos\Kernel.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\odyssey_trace.h
# End Source File
# Begin Source File

SOURCE=..\Msl\OsHeap.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\Oos\OsTypes.h
# End Source File
# Begin Source File

SOURCE=.\PersistentData.h
# End Source File
# Begin Source File

SOURCE=.\PtsTable.h
# End Source File
# Begin Source File

SOURCE=.\RoutingTables.h
# End Source File
# Begin Source File

SOURCE=.\ServeTable.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\Oos\Simple.h
# End Source File
# Begin Source File

SOURCE=.\SystemTable.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\Oos\Task.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\Trace_Index.h
# End Source File
# Begin Source File

SOURCE=.\VirtualTable.h
# End Source File
# Begin Source File

SOURCE=.\WaitQueue_T.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\Oos\win_Kernel.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\Oos\win_timer.h
# End Source File
# End Group
# End Target
# End Project
