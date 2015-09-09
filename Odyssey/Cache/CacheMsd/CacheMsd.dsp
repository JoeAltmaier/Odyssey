# Microsoft Developer Studio Project File - Name="CacheMsd" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=CacheMsd - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "CacheMsd.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "CacheMsd.mak" CFG="CacheMsd - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "CacheMsd - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "CacheMsd - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe

!IF  "$(CFG)" == "CacheMsd - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\Release"
# PROP BASE Intermediate_Dir ".\Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Release"
# PROP Intermediate_Dir ".\Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MDd /W3 /GX /O2 /I "..\..\..\include" /I "..\..\..\include\i2o" /I "..\..\..\include\nucleus" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "CacheMsd - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\Debug"
# PROP BASE Intermediate_Dir ".\Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\Debug"
# PROP Intermediate_Dir ".\Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MDd /W3 /GX /Z7 /Od /I "..\..\..\include\oos" /I "..\..\..\include" /I "..\..\..\include\i2o" /I "..\..\..\include\nucleus" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /YX /FD /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:".\Debug\Cache.lib"

!ENDIF 

# Begin Target

# Name "CacheMsd - Win32 Release"
# Name "CacheMsd - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Source File

SOURCE=..\CmCache.cpp
# End Source File
# Begin Source File

SOURCE=..\CmClose.cpp
# End Source File
# Begin Source File

SOURCE=..\CmDebug.cpp
# End Source File
# Begin Source File

SOURCE=..\CmFlush.cpp
# End Source File
# Begin Source File

SOURCE=..\CmFrameTable.cpp
# End Source File
# Begin Source File

SOURCE=..\CmInitialize.cpp
# End Source File
# Begin Source File

SOURCE=..\CmLock.cpp
# End Source File
# Begin Source File

SOURCE=..\CmMem.cpp
# End Source File
# Begin Source File

SOURCE=..\CmOpen.cpp
# End Source File
# Begin Source File

SOURCE=..\CmPageTable.cpp
# End Source File
# Begin Source File

SOURCE=..\CmReplace.cpp
# End Source File
# Begin Source File

SOURCE=..\CmSecondary.cpp
# End Source File
# Begin Source File

SOURCE=..\CmStats.cpp
# End Source File
# Begin Source File

SOURCE=..\CmValidate.cpp
# End Source File
# Begin Source File

SOURCE=..\CmWrite.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=..\CmCache.h
# End Source File
# Begin Source File

SOURCE=..\CmError.h
# End Source File
# Begin Source File

SOURCE=..\CmFrame.h
# End Source File
# Begin Source File

SOURCE=..\CmFrameTable.h
# End Source File
# Begin Source File

SOURCE=..\CmMem.h
# End Source File
# Begin Source File

SOURCE=..\CmPageTable.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
