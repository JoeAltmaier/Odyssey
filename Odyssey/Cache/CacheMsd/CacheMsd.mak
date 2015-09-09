# Microsoft Developer Studio Generated NMAKE File, Format Version 4.20
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

!IF "$(CFG)" == ""
CFG=CacheMsd - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to CacheMsd - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "CacheMsd - Win32 Release" && "$(CFG)" !=\
 "CacheMsd - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "CacheMsd.mak" CFG="CacheMsd - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "CacheMsd - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "CacheMsd - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 
################################################################################
# Begin Project
# PROP Target_Last_Scanned "CacheMsd - Win32 Debug"
CPP=cl.exe

!IF  "$(CFG)" == "CacheMsd - Win32 Release"

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
OUTDIR=.\Release
INTDIR=.\Release

ALL : "$(OUTDIR)\CacheMsd.lib"

CLEAN : 
	-@erase "$(INTDIR)\CmAbort.obj"
	-@erase "$(INTDIR)\CmCache.obj"
	-@erase "$(INTDIR)\CmClose.obj"
	-@erase "$(INTDIR)\CmError.obj"
	-@erase "$(INTDIR)\CmFlush.obj"
	-@erase "$(INTDIR)\CmFrame.obj"
	-@erase "$(INTDIR)\CmFrameTable.obj"
	-@erase "$(INTDIR)\CmInitialize.obj"
	-@erase "$(INTDIR)\CmLock.obj"
	-@erase "$(INTDIR)\CmMem.obj"
	-@erase "$(INTDIR)\CmOpen.obj"
	-@erase "$(INTDIR)\CmPageTable.obj"
	-@erase "$(INTDIR)\CmReplace.obj"
	-@erase "$(INTDIR)\CmWrite.obj"
	-@erase "$(OUTDIR)\CacheMsd.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MDd /W3 /GX /O2 /I "..\..\..\include" /I "..\..\..\include\i2o" /I "..\..\..\include\nucleus" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /MDd /W3 /GX /O2 /I "..\..\..\include" /I\
 "..\..\..\include\i2o" /I "..\..\..\include\nucleus" /D "WIN32" /D "NDEBUG" /D\
 "_WINDOWS" /Fp"$(INTDIR)/CacheMsd.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Release/
CPP_SBRS=.\.
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/CacheMsd.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo
LIB32_FLAGS=/nologo /out:"$(OUTDIR)/CacheMsd.lib" 
LIB32_OBJS= \
	"$(INTDIR)\CmAbort.obj" \
	"$(INTDIR)\CmCache.obj" \
	"$(INTDIR)\CmClose.obj" \
	"$(INTDIR)\CmError.obj" \
	"$(INTDIR)\CmFlush.obj" \
	"$(INTDIR)\CmFrame.obj" \
	"$(INTDIR)\CmFrameTable.obj" \
	"$(INTDIR)\CmInitialize.obj" \
	"$(INTDIR)\CmLock.obj" \
	"$(INTDIR)\CmMem.obj" \
	"$(INTDIR)\CmOpen.obj" \
	"$(INTDIR)\CmPageTable.obj" \
	"$(INTDIR)\CmReplace.obj" \
	"$(INTDIR)\CmWrite.obj"

"$(OUTDIR)\CacheMsd.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "CacheMsd - Win32 Debug"

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
OUTDIR=.\Debug
INTDIR=.\Debug

ALL : "$(OUTDIR)\CacheMsd.lib" "$(OUTDIR)\CacheMsd.bsc"

CLEAN : 
	-@erase "$(INTDIR)\CmAbort.obj"
	-@erase "$(INTDIR)\CmAbort.sbr"
	-@erase "$(INTDIR)\CmCache.obj"
	-@erase "$(INTDIR)\CmCache.sbr"
	-@erase "$(INTDIR)\CmClose.obj"
	-@erase "$(INTDIR)\CmClose.sbr"
	-@erase "$(INTDIR)\CmError.obj"
	-@erase "$(INTDIR)\CmError.sbr"
	-@erase "$(INTDIR)\CmFlush.obj"
	-@erase "$(INTDIR)\CmFlush.sbr"
	-@erase "$(INTDIR)\CmFrame.obj"
	-@erase "$(INTDIR)\CmFrame.sbr"
	-@erase "$(INTDIR)\CmFrameTable.obj"
	-@erase "$(INTDIR)\CmFrameTable.sbr"
	-@erase "$(INTDIR)\CmInitialize.obj"
	-@erase "$(INTDIR)\CmInitialize.sbr"
	-@erase "$(INTDIR)\CmLock.obj"
	-@erase "$(INTDIR)\CmLock.sbr"
	-@erase "$(INTDIR)\CmMem.obj"
	-@erase "$(INTDIR)\CmMem.sbr"
	-@erase "$(INTDIR)\CmOpen.obj"
	-@erase "$(INTDIR)\CmOpen.sbr"
	-@erase "$(INTDIR)\CmPageTable.obj"
	-@erase "$(INTDIR)\CmPageTable.sbr"
	-@erase "$(INTDIR)\CmReplace.obj"
	-@erase "$(INTDIR)\CmReplace.sbr"
	-@erase "$(INTDIR)\CmWrite.obj"
	-@erase "$(INTDIR)\CmWrite.sbr"
	-@erase "$(OUTDIR)\CacheMsd.bsc"
	-@erase "$(OUTDIR)\CacheMsd.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MDd /W3 /GX /Z7 /Od /I "..\..\..\include" /I "..\..\..\include\i2o" /I "..\..\..\include\nucleus" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /YX /c
CPP_PROJ=/nologo /MDd /W3 /GX /Z7 /Od /I "..\..\..\include" /I\
 "..\..\..\include\i2o" /I "..\..\..\include\nucleus" /D "WIN32" /D "_DEBUG" /D\
 "_WINDOWS" /FR"$(INTDIR)/" /Fp"$(INTDIR)/CacheMsd.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Debug/
CPP_SBRS=.\Debug/
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/CacheMsd.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\CmAbort.sbr" \
	"$(INTDIR)\CmCache.sbr" \
	"$(INTDIR)\CmClose.sbr" \
	"$(INTDIR)\CmError.sbr" \
	"$(INTDIR)\CmFlush.sbr" \
	"$(INTDIR)\CmFrame.sbr" \
	"$(INTDIR)\CmFrameTable.sbr" \
	"$(INTDIR)\CmInitialize.sbr" \
	"$(INTDIR)\CmLock.sbr" \
	"$(INTDIR)\CmMem.sbr" \
	"$(INTDIR)\CmOpen.sbr" \
	"$(INTDIR)\CmPageTable.sbr" \
	"$(INTDIR)\CmReplace.sbr" \
	"$(INTDIR)\CmWrite.sbr"

"$(OUTDIR)\CacheMsd.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo
LIB32_FLAGS=/nologo /out:"$(OUTDIR)/CacheMsd.lib" 
LIB32_OBJS= \
	"$(INTDIR)\CmAbort.obj" \
	"$(INTDIR)\CmCache.obj" \
	"$(INTDIR)\CmClose.obj" \
	"$(INTDIR)\CmError.obj" \
	"$(INTDIR)\CmFlush.obj" \
	"$(INTDIR)\CmFrame.obj" \
	"$(INTDIR)\CmFrameTable.obj" \
	"$(INTDIR)\CmInitialize.obj" \
	"$(INTDIR)\CmLock.obj" \
	"$(INTDIR)\CmMem.obj" \
	"$(INTDIR)\CmOpen.obj" \
	"$(INTDIR)\CmPageTable.obj" \
	"$(INTDIR)\CmReplace.obj" \
	"$(INTDIR)\CmWrite.obj"

"$(OUTDIR)\CacheMsd.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ENDIF 

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

################################################################################
# Begin Target

# Name "CacheMsd - Win32 Release"
# Name "CacheMsd - Win32 Debug"

!IF  "$(CFG)" == "CacheMsd - Win32 Release"

!ELSEIF  "$(CFG)" == "CacheMsd - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=\TreeRoot\Odyssey\Cache\CmWrite.cpp
DEP_CPP_CMWRI=\
	"..\..\..\include\Cache.h"\
	"..\..\..\include\Callback.h"\
	"..\..\..\include\i2o\i2odep.h"\
	"..\..\..\include\i2o\i2otypes.h"\
	"..\..\..\include\List.h"\
	"..\..\..\include\nucleus\cs_defs.h"\
	"..\..\..\include\nucleus\dm_defs.h"\
	"..\..\..\include\nucleus\ev_defs.h"\
	"..\..\..\include\nucleus\mb_defs.h"\
	"..\..\..\include\nucleus\Nucleus.h"\
	"..\..\..\include\nucleus\pi_defs.h"\
	"..\..\..\include\nucleus\pm_defs.h"\
	"..\..\..\include\nucleus\qu_defs.h"\
	"..\..\..\include\nucleus\sm_defs.h"\
	"..\..\..\include\nucleus\tc_defs.h"\
	"..\..\..\include\nucleus\tm_defs.h"\
	"..\CmCache.h"\
	"..\CmCommon.h"\
	"..\CmError.h"\
	"..\CmFrame.h"\
	"..\CmFrameHandle.h"\
	"..\CmFrameTable.h"\
	"..\CmPageTable.h"\
	

!IF  "$(CFG)" == "CacheMsd - Win32 Release"


"$(INTDIR)\CmWrite.obj" : $(SOURCE) $(DEP_CPP_CMWRI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "CacheMsd - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\CmWrite.obj" : $(SOURCE) $(DEP_CPP_CMWRI) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\CmWrite.sbr" : $(SOURCE) $(DEP_CPP_CMWRI) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\TreeRoot\Odyssey\Cache\CmCache.cpp
DEP_CPP_CMCAC=\
	"..\..\..\include\Cache.h"\
	"..\..\..\include\Callback.h"\
	"..\..\..\include\i2o\i2odep.h"\
	"..\..\..\include\i2o\i2otypes.h"\
	"..\..\..\include\List.h"\
	"..\..\..\include\nucleus\cs_defs.h"\
	"..\..\..\include\nucleus\dm_defs.h"\
	"..\..\..\include\nucleus\ev_defs.h"\
	"..\..\..\include\nucleus\mb_defs.h"\
	"..\..\..\include\nucleus\Nucleus.h"\
	"..\..\..\include\nucleus\pi_defs.h"\
	"..\..\..\include\nucleus\pm_defs.h"\
	"..\..\..\include\nucleus\qu_defs.h"\
	"..\..\..\include\nucleus\sm_defs.h"\
	"..\..\..\include\nucleus\tc_defs.h"\
	"..\..\..\include\nucleus\tm_defs.h"\
	"..\CmCache.h"\
	"..\CmCommon.h"\
	"..\CmError.h"\
	"..\CmFrame.h"\
	"..\CmFrameHandle.h"\
	"..\CmFrameTable.h"\
	"..\CmPageTable.h"\
	

!IF  "$(CFG)" == "CacheMsd - Win32 Release"


"$(INTDIR)\CmCache.obj" : $(SOURCE) $(DEP_CPP_CMCAC) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "CacheMsd - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\CmCache.obj" : $(SOURCE) $(DEP_CPP_CMCAC) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\CmCache.sbr" : $(SOURCE) $(DEP_CPP_CMCAC) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\TreeRoot\Odyssey\Cache\CmClose.cpp
DEP_CPP_CMCLO=\
	"..\..\..\include\Cache.h"\
	"..\..\..\include\Callback.h"\
	"..\..\..\include\i2o\i2odep.h"\
	"..\..\..\include\i2o\i2otypes.h"\
	"..\..\..\include\List.h"\
	"..\..\..\include\nucleus\cs_defs.h"\
	"..\..\..\include\nucleus\dm_defs.h"\
	"..\..\..\include\nucleus\ev_defs.h"\
	"..\..\..\include\nucleus\mb_defs.h"\
	"..\..\..\include\nucleus\Nucleus.h"\
	"..\..\..\include\nucleus\pi_defs.h"\
	"..\..\..\include\nucleus\pm_defs.h"\
	"..\..\..\include\nucleus\qu_defs.h"\
	"..\..\..\include\nucleus\sm_defs.h"\
	"..\..\..\include\nucleus\tc_defs.h"\
	"..\..\..\include\nucleus\tm_defs.h"\
	"..\CmCache.h"\
	"..\CmCommon.h"\
	"..\CmError.h"\
	"..\CmFrame.h"\
	"..\CmFrameHandle.h"\
	"..\CmFrameTable.h"\
	"..\CmPageTable.h"\
	

!IF  "$(CFG)" == "CacheMsd - Win32 Release"


"$(INTDIR)\CmClose.obj" : $(SOURCE) $(DEP_CPP_CMCLO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "CacheMsd - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\CmClose.obj" : $(SOURCE) $(DEP_CPP_CMCLO) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\CmClose.sbr" : $(SOURCE) $(DEP_CPP_CMCLO) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\TreeRoot\Odyssey\Cache\CmError.cpp
DEP_CPP_CMERR=\
	"..\..\..\include\i2o\i2odep.h"\
	"..\..\..\include\nucleus\cs_defs.h"\
	"..\..\..\include\nucleus\dm_defs.h"\
	"..\..\..\include\nucleus\ev_defs.h"\
	"..\..\..\include\nucleus\mb_defs.h"\
	"..\..\..\include\nucleus\Nucleus.h"\
	"..\..\..\include\nucleus\pi_defs.h"\
	"..\..\..\include\nucleus\pm_defs.h"\
	"..\..\..\include\nucleus\qu_defs.h"\
	"..\..\..\include\nucleus\sm_defs.h"\
	"..\..\..\include\nucleus\tc_defs.h"\
	"..\..\..\include\nucleus\tm_defs.h"\
	"..\..\..\include\Serdrv.h"\
	"..\CmError.h"\
	

!IF  "$(CFG)" == "CacheMsd - Win32 Release"


"$(INTDIR)\CmError.obj" : $(SOURCE) $(DEP_CPP_CMERR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "CacheMsd - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\CmError.obj" : $(SOURCE) $(DEP_CPP_CMERR) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\CmError.sbr" : $(SOURCE) $(DEP_CPP_CMERR) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\TreeRoot\Odyssey\Cache\CmFlush.cpp
DEP_CPP_CMFLU=\
	"..\..\..\include\Cache.h"\
	"..\..\..\include\Callback.h"\
	"..\..\..\include\i2o\i2odep.h"\
	"..\..\..\include\i2o\i2otypes.h"\
	"..\..\..\include\List.h"\
	"..\..\..\include\nucleus\cs_defs.h"\
	"..\..\..\include\nucleus\dm_defs.h"\
	"..\..\..\include\nucleus\ev_defs.h"\
	"..\..\..\include\nucleus\mb_defs.h"\
	"..\..\..\include\nucleus\Nucleus.h"\
	"..\..\..\include\nucleus\pi_defs.h"\
	"..\..\..\include\nucleus\pm_defs.h"\
	"..\..\..\include\nucleus\qu_defs.h"\
	"..\..\..\include\nucleus\sm_defs.h"\
	"..\..\..\include\nucleus\tc_defs.h"\
	"..\..\..\include\nucleus\tm_defs.h"\
	"..\CmCache.h"\
	"..\CmCommon.h"\
	"..\CmError.h"\
	"..\CmFrame.h"\
	"..\CmFrameHandle.h"\
	"..\CmFrameTable.h"\
	"..\CmPageTable.h"\
	

!IF  "$(CFG)" == "CacheMsd - Win32 Release"


"$(INTDIR)\CmFlush.obj" : $(SOURCE) $(DEP_CPP_CMFLU) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "CacheMsd - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\CmFlush.obj" : $(SOURCE) $(DEP_CPP_CMFLU) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\CmFlush.sbr" : $(SOURCE) $(DEP_CPP_CMFLU) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\TreeRoot\Odyssey\Cache\CmFrame.cpp
DEP_CPP_CMFRA=\
	"..\..\..\include\Cache.h"\
	"..\..\..\include\Callback.h"\
	"..\..\..\include\i2o\i2odep.h"\
	"..\..\..\include\i2o\i2otypes.h"\
	"..\..\..\include\List.h"\
	"..\..\..\include\nucleus\cs_defs.h"\
	"..\..\..\include\nucleus\dm_defs.h"\
	"..\..\..\include\nucleus\ev_defs.h"\
	"..\..\..\include\nucleus\mb_defs.h"\
	"..\..\..\include\nucleus\Nucleus.h"\
	"..\..\..\include\nucleus\pi_defs.h"\
	"..\..\..\include\nucleus\pm_defs.h"\
	"..\..\..\include\nucleus\qu_defs.h"\
	"..\..\..\include\nucleus\sm_defs.h"\
	"..\..\..\include\nucleus\tc_defs.h"\
	"..\..\..\include\nucleus\tm_defs.h"\
	"..\CmCommon.h"\
	"..\CmError.h"\
	"..\CmFrame.h"\
	"..\CmFrameHandle.h"\
	"..\CmMem.h"\
	

!IF  "$(CFG)" == "CacheMsd - Win32 Release"


"$(INTDIR)\CmFrame.obj" : $(SOURCE) $(DEP_CPP_CMFRA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "CacheMsd - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\CmFrame.obj" : $(SOURCE) $(DEP_CPP_CMFRA) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\CmFrame.sbr" : $(SOURCE) $(DEP_CPP_CMFRA) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\TreeRoot\Odyssey\Cache\CmFrameTable.cpp
DEP_CPP_CMFRAM=\
	"..\..\..\include\Cache.h"\
	"..\..\..\include\Callback.h"\
	"..\..\..\include\i2o\i2odep.h"\
	"..\..\..\include\i2o\i2otypes.h"\
	"..\..\..\include\List.h"\
	"..\..\..\include\nucleus\cs_defs.h"\
	"..\..\..\include\nucleus\dm_defs.h"\
	"..\..\..\include\nucleus\ev_defs.h"\
	"..\..\..\include\nucleus\mb_defs.h"\
	"..\..\..\include\nucleus\Nucleus.h"\
	"..\..\..\include\nucleus\pi_defs.h"\
	"..\..\..\include\nucleus\pm_defs.h"\
	"..\..\..\include\nucleus\qu_defs.h"\
	"..\..\..\include\nucleus\sm_defs.h"\
	"..\..\..\include\nucleus\tc_defs.h"\
	"..\..\..\include\nucleus\tm_defs.h"\
	"..\CmCommon.h"\
	"..\CmError.h"\
	"..\CmFrame.h"\
	"..\CmFrameHandle.h"\
	"..\CmFrameTable.h"\
	"..\CmMem.h"\
	

!IF  "$(CFG)" == "CacheMsd - Win32 Release"


"$(INTDIR)\CmFrameTable.obj" : $(SOURCE) $(DEP_CPP_CMFRAM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "CacheMsd - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\CmFrameTable.obj" : $(SOURCE) $(DEP_CPP_CMFRAM) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\CmFrameTable.sbr" : $(SOURCE) $(DEP_CPP_CMFRAM) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\TreeRoot\Odyssey\Cache\CmInitialize.cpp
DEP_CPP_CMINI=\
	"..\..\..\include\Cache.h"\
	"..\..\..\include\Callback.h"\
	"..\..\..\include\i2o\i2odep.h"\
	"..\..\..\include\i2o\i2otypes.h"\
	"..\..\..\include\List.h"\
	"..\..\..\include\nucleus\cs_defs.h"\
	"..\..\..\include\nucleus\dm_defs.h"\
	"..\..\..\include\nucleus\ev_defs.h"\
	"..\..\..\include\nucleus\mb_defs.h"\
	"..\..\..\include\nucleus\Nucleus.h"\
	"..\..\..\include\nucleus\pi_defs.h"\
	"..\..\..\include\nucleus\pm_defs.h"\
	"..\..\..\include\nucleus\qu_defs.h"\
	"..\..\..\include\nucleus\sm_defs.h"\
	"..\..\..\include\nucleus\tc_defs.h"\
	"..\..\..\include\nucleus\tm_defs.h"\
	"..\CmCache.h"\
	"..\CmCommon.h"\
	"..\CmError.h"\
	"..\CmFrame.h"\
	"..\CmFrameHandle.h"\
	"..\CmFrameTable.h"\
	"..\CmMem.h"\
	"..\CmPageTable.h"\
	

!IF  "$(CFG)" == "CacheMsd - Win32 Release"


"$(INTDIR)\CmInitialize.obj" : $(SOURCE) $(DEP_CPP_CMINI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "CacheMsd - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\CmInitialize.obj" : $(SOURCE) $(DEP_CPP_CMINI) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\CmInitialize.sbr" : $(SOURCE) $(DEP_CPP_CMINI) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\TreeRoot\Odyssey\Cache\CmLock.cpp
DEP_CPP_CMLOC=\
	"..\..\..\include\Cache.h"\
	"..\..\..\include\Callback.h"\
	"..\..\..\include\i2o\i2odep.h"\
	"..\..\..\include\i2o\i2otypes.h"\
	"..\..\..\include\List.h"\
	"..\..\..\include\nucleus\cs_defs.h"\
	"..\..\..\include\nucleus\dm_defs.h"\
	"..\..\..\include\nucleus\ev_defs.h"\
	"..\..\..\include\nucleus\mb_defs.h"\
	"..\..\..\include\nucleus\Nucleus.h"\
	"..\..\..\include\nucleus\pi_defs.h"\
	"..\..\..\include\nucleus\pm_defs.h"\
	"..\..\..\include\nucleus\qu_defs.h"\
	"..\..\..\include\nucleus\sm_defs.h"\
	"..\..\..\include\nucleus\tc_defs.h"\
	"..\..\..\include\nucleus\tm_defs.h"\
	"..\CmCache.h"\
	"..\CmCommon.h"\
	"..\CmError.h"\
	"..\CmFrame.h"\
	"..\CmFrameHandle.h"\
	"..\CmFrameTable.h"\
	"..\CmPageTable.h"\
	

!IF  "$(CFG)" == "CacheMsd - Win32 Release"


"$(INTDIR)\CmLock.obj" : $(SOURCE) $(DEP_CPP_CMLOC) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "CacheMsd - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\CmLock.obj" : $(SOURCE) $(DEP_CPP_CMLOC) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\CmLock.sbr" : $(SOURCE) $(DEP_CPP_CMLOC) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\TreeRoot\Odyssey\Cache\CmMem.cpp
DEP_CPP_CMMEM=\
	"..\..\..\include\nucleus\cs_defs.h"\
	"..\..\..\include\nucleus\dm_defs.h"\
	"..\..\..\include\nucleus\ev_defs.h"\
	"..\..\..\include\nucleus\mb_defs.h"\
	"..\..\..\include\nucleus\Nucleus.h"\
	"..\..\..\include\nucleus\pi_defs.h"\
	"..\..\..\include\nucleus\pm_defs.h"\
	"..\..\..\include\nucleus\qu_defs.h"\
	"..\..\..\include\nucleus\sm_defs.h"\
	"..\..\..\include\nucleus\tc_defs.h"\
	"..\..\..\include\nucleus\tm_defs.h"\
	"..\CmMem.h"\
	

!IF  "$(CFG)" == "CacheMsd - Win32 Release"


"$(INTDIR)\CmMem.obj" : $(SOURCE) $(DEP_CPP_CMMEM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "CacheMsd - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\CmMem.obj" : $(SOURCE) $(DEP_CPP_CMMEM) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\CmMem.sbr" : $(SOURCE) $(DEP_CPP_CMMEM) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\TreeRoot\Odyssey\Cache\CmOpen.cpp
DEP_CPP_CMOPE=\
	"..\..\..\include\Cache.h"\
	"..\..\..\include\Callback.h"\
	"..\..\..\include\i2o\i2odep.h"\
	"..\..\..\include\i2o\i2otypes.h"\
	"..\..\..\include\List.h"\
	"..\..\..\include\nucleus\cs_defs.h"\
	"..\..\..\include\nucleus\dm_defs.h"\
	"..\..\..\include\nucleus\ev_defs.h"\
	"..\..\..\include\nucleus\mb_defs.h"\
	"..\..\..\include\nucleus\Nucleus.h"\
	"..\..\..\include\nucleus\pi_defs.h"\
	"..\..\..\include\nucleus\pm_defs.h"\
	"..\..\..\include\nucleus\qu_defs.h"\
	"..\..\..\include\nucleus\sm_defs.h"\
	"..\..\..\include\nucleus\tc_defs.h"\
	"..\..\..\include\nucleus\tm_defs.h"\
	"..\CmCache.h"\
	"..\CmCommon.h"\
	"..\CmError.h"\
	"..\CmFrame.h"\
	"..\CmFrameHandle.h"\
	"..\CmFrameTable.h"\
	"..\CmPageTable.h"\
	

!IF  "$(CFG)" == "CacheMsd - Win32 Release"


"$(INTDIR)\CmOpen.obj" : $(SOURCE) $(DEP_CPP_CMOPE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "CacheMsd - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\CmOpen.obj" : $(SOURCE) $(DEP_CPP_CMOPE) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\CmOpen.sbr" : $(SOURCE) $(DEP_CPP_CMOPE) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\TreeRoot\Odyssey\Cache\CmPageTable.cpp
DEP_CPP_CMPAG=\
	"..\..\..\include\Cache.h"\
	"..\..\..\include\Callback.h"\
	"..\..\..\include\i2o\i2odep.h"\
	"..\..\..\include\i2o\i2otypes.h"\
	"..\..\..\include\List.h"\
	"..\..\..\include\nucleus\cs_defs.h"\
	"..\..\..\include\nucleus\dm_defs.h"\
	"..\..\..\include\nucleus\ev_defs.h"\
	"..\..\..\include\nucleus\mb_defs.h"\
	"..\..\..\include\nucleus\Nucleus.h"\
	"..\..\..\include\nucleus\pi_defs.h"\
	"..\..\..\include\nucleus\pm_defs.h"\
	"..\..\..\include\nucleus\qu_defs.h"\
	"..\..\..\include\nucleus\sm_defs.h"\
	"..\..\..\include\nucleus\tc_defs.h"\
	"..\..\..\include\nucleus\tm_defs.h"\
	"..\CmCommon.h"\
	"..\CmError.h"\
	"..\CmFrame.h"\
	"..\CmFrameHandle.h"\
	"..\CmMem.h"\
	"..\CmPageTable.h"\
	

!IF  "$(CFG)" == "CacheMsd - Win32 Release"


"$(INTDIR)\CmPageTable.obj" : $(SOURCE) $(DEP_CPP_CMPAG) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "CacheMsd - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\CmPageTable.obj" : $(SOURCE) $(DEP_CPP_CMPAG) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\CmPageTable.sbr" : $(SOURCE) $(DEP_CPP_CMPAG) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\TreeRoot\Odyssey\Cache\CmReplace.cpp
DEP_CPP_CMREP=\
	"..\..\..\include\Cache.h"\
	"..\..\..\include\Callback.h"\
	"..\..\..\include\i2o\i2odep.h"\
	"..\..\..\include\i2o\i2otypes.h"\
	"..\..\..\include\List.h"\
	"..\..\..\include\nucleus\cs_defs.h"\
	"..\..\..\include\nucleus\dm_defs.h"\
	"..\..\..\include\nucleus\ev_defs.h"\
	"..\..\..\include\nucleus\mb_defs.h"\
	"..\..\..\include\nucleus\Nucleus.h"\
	"..\..\..\include\nucleus\pi_defs.h"\
	"..\..\..\include\nucleus\pm_defs.h"\
	"..\..\..\include\nucleus\qu_defs.h"\
	"..\..\..\include\nucleus\sm_defs.h"\
	"..\..\..\include\nucleus\tc_defs.h"\
	"..\..\..\include\nucleus\tm_defs.h"\
	"..\CmCache.h"\
	"..\CmCommon.h"\
	"..\CmError.h"\
	"..\CmFrame.h"\
	"..\CmFrameHandle.h"\
	"..\CmFrameTable.h"\
	"..\CmPageTable.h"\
	

!IF  "$(CFG)" == "CacheMsd - Win32 Release"


"$(INTDIR)\CmReplace.obj" : $(SOURCE) $(DEP_CPP_CMREP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "CacheMsd - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\CmReplace.obj" : $(SOURCE) $(DEP_CPP_CMREP) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\CmReplace.sbr" : $(SOURCE) $(DEP_CPP_CMREP) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\TreeRoot\Odyssey\Cache\CmAbort.cpp
DEP_CPP_CMABO=\
	"..\..\..\include\Cache.h"\
	"..\..\..\include\Callback.h"\
	"..\..\..\include\i2o\i2odep.h"\
	"..\..\..\include\i2o\i2otypes.h"\
	"..\..\..\include\List.h"\
	"..\..\..\include\nucleus\cs_defs.h"\
	"..\..\..\include\nucleus\dm_defs.h"\
	"..\..\..\include\nucleus\ev_defs.h"\
	"..\..\..\include\nucleus\mb_defs.h"\
	"..\..\..\include\nucleus\Nucleus.h"\
	"..\..\..\include\nucleus\pi_defs.h"\
	"..\..\..\include\nucleus\pm_defs.h"\
	"..\..\..\include\nucleus\qu_defs.h"\
	"..\..\..\include\nucleus\sm_defs.h"\
	"..\..\..\include\nucleus\tc_defs.h"\
	"..\..\..\include\nucleus\tm_defs.h"\
	"..\CmCache.h"\
	"..\CmCommon.h"\
	"..\CmError.h"\
	"..\CmFrame.h"\
	"..\CmFrameHandle.h"\
	"..\CmFrameTable.h"\
	"..\CmPageTable.h"\
	

!IF  "$(CFG)" == "CacheMsd - Win32 Release"


"$(INTDIR)\CmAbort.obj" : $(SOURCE) $(DEP_CPP_CMABO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "CacheMsd - Win32 Debug"


BuildCmds= \
	$(CPP) $(CPP_PROJ) $(SOURCE) \
	

"$(INTDIR)\CmAbort.obj" : $(SOURCE) $(DEP_CPP_CMABO) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\CmAbort.sbr" : $(SOURCE) $(DEP_CPP_CMABO) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
# End Target
# End Project
################################################################################
