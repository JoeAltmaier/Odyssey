/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: Trace_Index.h
// 
// Description:
// This file defines the module indexes for most system modules used
// for debugging. Modules should always have a unique index for tracing
// and a secondary in case finer tracing is desired.
//
// Index Numbers should be sequential, since these values are used to
// index values in the TraceLevel[] array.
// 
/*************************************************************************/

// Update Log 
//
// $Log: /Gemini/Include/Trace_Index.h $
// 
// 32    2/08/00 10:02p Rbraun
// Added TRACE_TEST_MGR for Neptune Test Manager
// 
// 31    12/16/99 3:24p Iowa
// Heap leak detection support.
// 
// 30    12/09/99 1:20a Iowa
// 
// 29    10/18/99 4:52p Mpanas
// Fix trace level overlap
// Minor cleanup
// 
// 28    10/14/99 3:32a Iowa
// Iowa merge
// 
// 27    10/08/99 11:49a Vnguyen
// Add TRACE_PHS_REPORTER.
// 
// 25    9/16/99 2:58p Cchan
// Added Trace entries for the HSCSI library
// 
// 24    9/10/99 10:16a Joehler
// Added TRACE_UPGRADE for UpgradeMaster
// 
// 23    8/20/99 6:10p Hdo
// DdmLED
// 
// 22    8/20/99 5:16p Tnelson
// Add TRACE_NULL
// 
// 21    8/20/99 11:33a Bbutler
// 
// 20    8/17/99 7:54p Tnelson
// Changed TRACE_DDM1 to TRACE_VIRTUAL_MSTR
// 
// 19    8/08/99 6:44p Bbutler
// 
// 
// 18    8/06/99 8:47p Jaltmaier
// New TRACE_MESSAGE
// 
// 17    8/06/99 9:27a Joehler
// Added TRACE_ALARM for Alarm Manager
//
// 17    8/6/99  9:26a JOehler
// Added TRACE_ALARM for Alarm Manager
// 
// 16    8/03/99 5:16p Jlane
// Added TRACE_VCM for Virtual Circuit Master
// 
// 15    8/02/99 3:15p Hdo
// 
// 14    7/23/99 12:27p Hdo
// 
// 13    7/21/99 6:06p Hdo
// 
// 12    7/06/99 9:33a Dpatel
// Added Raid Master Trace levels.
// 
// 11    7/02/99 12:08p Tnelson
// 
// 10    6/30/99 7:54p Mpanas
// Add DDMPTSDEFAULT trace level index
// 
// 9     6/27/99 3:07p Tnelson
// New OS modules
// 
// 1/24/99 Michael G. Panas: Create file
// 2/17/99 Jim Frandeen: Add end of line to end of file

#if !defined(Trace_Index_H)
#define Trace_Index_H

// BASE CODE
#define	TRACE_BOOT					0		// BOOT Process
#define	TRACE_BOOT1					1		// secondary
#define	TRACE_APP_INIT				2
#define	TRACE_APP_INIT1				3
#define	TRACE_OOS					4
#define	TRACE_DIDMAN				5
#define	TRACE_MESSENGER				6
#define	TRACE_MESSENGER1			7
#define	TRACE_FAILSAFE				8
#define	TRACE_FAILSAFE1				9
#define	TRACE_ODY_DRIVERS			10		// Odyssey System drivers
#define	TRACE_ODY_DRIVERS1			11
#define	TRACE_EVAL_DRIVERS			12		// Eval Bd system drivers
#define	TRACE_EVAL_DRIVERS1			13
#define	TRACE_NUCLEUS				14
#define	TRACE_NUCLEUS1				15
#define	TRACE_DDM_MGR				16		// CHAOS Ddm Manager
#define	TRACE_VIRTUAL_MGR			17		// CHAOS Virtual Manager
#define	TRACE_DDM					18		// CHAOS DDM base class
#define	TRACE_VIRTUAL_MSTR			19		// CHAOS Virtual Master
#define	TRACE_HEAP					20
#define	TRACE_HEAP1					21
#define	TRACE_TIMER					22		// OOS Timer
#define	TRACE_TIMER1				23
#define	TRACE_MS					24		// MS Windows Stuff
#define	TRACE_MS1					25
#define	TRACE_TRANSPORT				26		// OOS Transport
#define TRACE_FAILOVER				27
#define TRACE_MESSAGE				28
#define TRACE_SYSINFO				29		// DdmSysInfo

// LIBRARYS
#define	TRACE_CLIB					30		// C Library
#define	TRACE_CLIB1					31
#define	TRACE_FCP					32		// FCP Library
#define	TRACE_FCP1					33
#define	TRACE_CACHE					34		// Cache Library
#define	TRACE_CACHE1				35
#define	TRACE_CALLBACK				36		// Call Back Lib
#define	TRACE_CALLBACK1				37
#define	TRACE_BUILDSYS				38		// BuildSys Lib
#define	TRACE_PTSLOADER 			39
#define	TRACE_HSCSI					40		// HSCSI Library
#define	TRACE_HSCSI1				41	

// leave hole here
// System DDMS

#define	TRACE_PTS					50		// Persistent Table
#define	TRACE_PTS1					51
#define	TRACE_PTS_LISTEN			52		// Persistent Table Listener
#define	TRACE_PTS_LISTEN1			53
#define	TRACE_DDM_REPORTER			54		// DDM Reporter
#define	TRACE_DDM_REPORTER1			55
#define	TRACE_DDM_STRESS			56
#define	TRACE_DDM_STRESS1			57
#define	TRACE_DDM_MONITOR			58
#define	TRACE_DDM_MONITOR1			59
#define	TRACE_DDM_NULL				60
#define	TRACE_DDM_NULL1				61
#define	TRACE_DDM_PART				62		// Partition
#define	TRACE_DDM_PART1				63
#define	TRACE_SYSTEM_MASTER			64		// System Master
#define	TRACE_SYSTEM_MASTER1		65
#define	TRACE_DDM_CACHE				66
#define	TRACE_DDM_CACHE1			67
#define	TRACE_PTSDEFAULT			68
#define	TRACE_PTSDEFAULT1			69

#define	TRACE_FCP_TARGET			70
#define	TRACE_FCP_TARGET1			71
#define	TRACE_FCP_INITIATOR			72
#define	TRACE_FCP_INITIATOR1		73
#define	TRACE_FCP_LOOP				74
#define	TRACE_FCP_LOOP1				75
#define	TRACE_FC_MASTER				76
#define	TRACE_FC_MASTER1			77
#define	TRACE_DRIVE_MONITOR			78
#define	TRACE_DRIVE_MONITOR1		79
#define	TRACE_ECHO_SCSI				80
#define	TRACE_ECHO_SCSI1			81
#define	TRACE_SCSI_TARGET_SERVER	82
#define	TRACE_SCSI_TARGET_SERVER1	83
#define	TRACE_BSA					84
#define	TRACE_BSA1					85
#define	TRACE_DAISY_MONITOR			86		// Daisy Chain Monitor
#define	TRACE_DAISY_MONITOR1		87
#define	TRACE_RAID					88		// Raid
#define	TRACE_RAID1					89
#define	TRACE_RSM					90		// Reserved Sector Manager
#define	TRACE_RSM1					91
#define	TRACE_SSD					92		// Solid State Disk
#define	TRACE_SSD1					93
#define	TRACE_RAM_DISK				94		// Ram Disk
#define	TRACE_RAM_DISK1				95

#define	TRACE_SSAPI_OBJECTS			96		// SSAPI layer objects
#define	TRACE_SSAPI_MANAGERS		97		// SSAPI layer managers

#define TRACE_RMSTR					98
#define TRACE_RMSTR_1				99
#define TRACE_RMSTR_2				100

#define TRACE_PNP					101		// PnP
#define TRACE_SDDM					102

#define TRACE_FLASH_MONITOR			103		// Flash Monitor
#define TRACE_VCM					104		// Virtual Circuit Master

#define TRACE_ALARM                 105		// Alarm Manager
#define TRACE_SYSTEMLOG				106		// LogMaster

#define TRACE_CHAOSFILE				107		// ChaosFile object
#define TRACE_PARTITIONMGR			108		// DDM for ChaosFile object
#define TRACE_NULL					109		// CHAOS DdmNull
#define TRACE_DDM_LED				110		// DdmLED

#define TRACE_UPGRADE				111	
#define TRACE_FILESYS				112
#define TRACE_PHS_REPORTER			113		// For PHS Reporter

#define TRACE_DDM_NET				114
#define TRACE_NETWORK				115
#define TRACE_DDM_PPP				116

#define TRACE_QUIESCEMASTER			117

#define TRACE_TEST_MGR				118		// Neptune Test Manager

#define TRACE_MAX					200
#endif

