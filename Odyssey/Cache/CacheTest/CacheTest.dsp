# Microsoft Developer Studio Project File - Name="CacheTest" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=CacheTest - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "CacheTest.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "CacheTest.mak" CFG="CacheTest - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "CacheTest - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe
# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\Debug"
# PROP BASE Intermediate_Dir ".\Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\Debug"
# PROP Intermediate_Dir ".\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /I "..\..\..\include\oos" /I "..\..\Cache" /I "..\..\..\include" /I "..\..\..\include\i2o" /I "..\..\..\include\nucleus" /I ".." /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386
# ADD LINK32  ..\..\Cache\CacheMsd\Debug\Cache.lib ..\..\..\Tools\WinAsyncIO\Debug\WinAsyncIO.lib /nologo /subsystem:windows /debug /machine:I386
# Begin Target

# Name "CacheTest - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Source File

SOURCE=.\CacheConfig.cpp
# End Source File
# Begin Source File

SOURCE=.\CacheFileDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\CacheTest.cpp
# End Source File
# Begin Source File

SOURCE=.\CacheTest.rc
# End Source File
# Begin Source File

SOURCE=.\CacheTestDoc.cpp
# End Source File
# Begin Source File

SOURCE=.\CacheTestView.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Callback\Callback.cpp
# End Source File
# Begin Source File

SOURCE=.\CloseFlash.cpp
# End Source File
# Begin Source File

SOURCE=.\CmCacheTest.cpp
# End Source File
# Begin Source File

SOURCE=.\CmStressTest.cpp
# End Source File
# Begin Source File

SOURCE=.\ConfigStatsDialog.cpp
# End Source File
# Begin Source File

SOURCE=..\..\ErrorLog\ErrorLog.c
# End Source File
# Begin Source File

SOURCE=.\FbTestSemaphore.cpp
# End Source File
# Begin Source File

SOURCE=.\FlushDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\FormatCacheStats.cpp
# End Source File
# Begin Source File

SOURCE=.\MainFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# Begin Source File

SOURCE=.\ReadSequential.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\StopStressDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\StressDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\TestDevice.cpp
# End Source File
# Begin Source File

SOURCE=.\Trace.c
# End Source File
# Begin Source File

SOURCE=..\..\..\Tools\TraceMonitor\TraceString.cpp
# End Source File
# Begin Source File

SOURCE=.\WriteSequential.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=.\CacheConfig.h
# End Source File
# Begin Source File

SOURCE=.\CacheFileDialog.h
# End Source File
# Begin Source File

SOURCE=.\CacheTest.h
# End Source File
# Begin Source File

SOURCE=.\CacheTestData.h
# End Source File
# Begin Source File

SOURCE=.\CacheTestDoc.h
# End Source File
# Begin Source File

SOURCE=.\CacheTestView.h
# End Source File
# Begin Source File

SOURCE=.\CloseFlash.h
# End Source File
# Begin Source File

SOURCE=.\ConfigStatsDialog.h
# End Source File
# Begin Source File

SOURCE=.\FlushDialog.h
# End Source File
# Begin Source File

SOURCE=.\MainFrm.h
# End Source File
# Begin Source File

SOURCE=.\ReadSequential.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\StressDialog.h
# End Source File
# Begin Source File

SOURCE=.\TestDevice.h
# End Source File
# Begin Source File

SOURCE=.\WriteSequential.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\CacheTest.ico
# End Source File
# Begin Source File

SOURCE=.\res\CacheTest.rc2
# End Source File
# Begin Source File

SOURCE=.\res\CacheTestDoc.ico
# End Source File
# Begin Source File

SOURCE=.\res\Toolbar.bmp
# End Source File
# End Group
# Begin Source File

SOURCE=..\CacheMsd\Debug\Cache.lib
# End Source File
# End Target
# End Project
