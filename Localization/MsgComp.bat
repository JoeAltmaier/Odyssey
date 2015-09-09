@echo off
:
:  MsgComp.bat
:
:       Invokes the CT Message Compiler to build the standard OOS
:       event code header file, as well as message text resources.
:
:       Causes the header file to be dropped into the main Include
:       directory.
:
:       This batch file is (hopefully) a temporary kludge until the
:       CodeWarrior IDE plugin monster is tamed.
:
:
:  $Log: /Gemini/Localization/MsgComp.bat $
REM 
REM 4     7/12/99 11:59a Ewedel
REM Changed compiler invocation so that generated database files now go
REM into Odyssey\Message Text\ directory.
REM 
REM 3     6/28/99 7:21p Ewedel
REM Changed to not display batch file internals (vss comments, etc.) when
REM run.
REM 
REM 2     3/05/99 3:17p Jlane
REM Fixed erroeous bin location.

echo on
..\Tools\bin\MsgComp -h ..\include -r "..\Odyssey\Message Text" CTEvent.mc
