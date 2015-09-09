# Microsoft Developer Studio Project File - Name="DdmPTS" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=DdmPTS - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "DdmPTS.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "DdmPTS.mak" CFG="DdmPTS - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "DdmPTS - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "DdmPTS - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "DdmPTS - Win32 Release"

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

!ELSEIF  "$(CFG)" == "DdmPTS - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "DdmPTS___Win32_Debug"
# PROP BASE Intermediate_Dir "DdmPTS___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "DdmPTS___Win32_Debug"
# PROP Intermediate_Dir "DdmPTS___Win32_Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /Zp4 /MTd /W3 /Gm /GX /ZI /Od /I "..\HbcFlash" /I "..\..\include" /I "..\..\localization" /I "..\..\include\oos" /I "..\..\include\cttables" /I "..\..\include\pts" /I "..\..\cmdqueues" /I "..\..\include\fcp" /I "..\..\include\drivers" /I "..\ssapi_server\\" /I "..\ssapi_server\utils" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\bin\win\ddm_pts.lib"

!ENDIF 

# Begin Target

# Name "DdmPTS - Win32 Release"
# Name "DdmPTS - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\DdmPTS.cpp
# End Source File
# Begin Source File

SOURCE=.\Fields.cpp
# End Source File
# Begin Source File

SOURCE=.\Listen.cpp
# End Source File
# Begin Source File

SOURCE=.\PtsProcs.cpp
# End Source File
# Begin Source File

SOURCE=.\ReadTable.cpp
# End Source File
# Begin Source File

SOURCE=.\Rows.cpp
# End Source File
# Begin Source File

SOURCE=.\Table.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\DdmPts.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\Fields.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\Listen.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\PtsCommon.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\ReadTable.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\Rows.h
# End Source File
# Begin Source File

SOURCE=..\..\Include\Table.h
# End Source File
# End Group
# End Target
# End Project
