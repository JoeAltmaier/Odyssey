/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// Description:
// This is the Export Table
// 
// Update Log:
//	$Log: /Gemini/Include/CTTables/ExportTable.h $
// 
// 17    9/14/99 7:10p Jlane
// Added a define for the ReadyState field name.
// 
// 16    9/01/99 12:06p Jlane
// Renamed ridVCMCommand to ridVcId.
// 
// 15    8/28/99 8:38p Agusev
// 
// 14    8/06/99 9:16a Vnguyen
//
// 14	 8/06/99 9:10a Vnguyen
// Rename rowID from ridSTSStatusRec, ridSTSPerformanceRec to
// ridStatusRec and ridPerformanceRec for consistency and future extension.
// 
// 13    7/26/99 10:38a Jlane
// Added row IDs for Status and Performance records.
// 
// 12    7/22/99 5:44p Jlane
// New declaration to support multiple export recs per Virtual Circuit and
// also the new Host Connection table structure.
// 
// 11    7/14/99 3:39p Jlane
// Added ExportTableRecord declaration for consistency.
// 
// 10    7/12/99 7:53p Jlane
// Added structure name after last curly brace.
// 
// 9     7/12/99 6:05p Jlane
// Added typpedef before struct definition.
// 
// 8     7/06/99 9:31p Agusev
// Added a field to point to a HostConnectionCollection descriptor
// Modified a field name to be a #define it coul be modified by using the
// PTS'  ModifyField()
// 
// 7     7/02/99 6:53p Mpanas
// Add the Failover loop number, remove
// "name" since it is not used, and add
// the ridUserInfo for Andre to point to
// the user string info
// 
// 6     6/24/99 9:04p Agusev
// changed long long to I64 (WIN32 complience!)
// 
// 5     6/07/99 10:37p Mpanas
// Add fields to the Export and DiskDescriptor
// tables to support the future ISP2200 and
// Multiple FCInstances
// 
// 4     3/09/99 4:31p Mpanas
// Add refs to the field defs for this table
//
// 8/31/98 Michael G. Panas: Create file
// 9/8/98 Michael G. Panas: Change class to struct for easier access
// 9/29/98 Jim Frandeen: Use CtTypes.h instead of stddef.h
// 2/21/98 Michael G. Panas: remove I2O dependancy
// 03/09/99 Michael G. Panas: bring up to spec, add TargetId
/*************************************************************************/

#ifndef __ExportTable_h
#define __ExportTable_h

#include "CtTypes.h"
#include "Odyssey.h"
#include "PTSCommon.h"

// Field definitions in ExportTable.cpp
extern	fieldDef	ExportTable_FieldDefs[];
extern	U32			cbExportTable_FieldDefs;

#pragma	pack(4)


// This is a bit mask
enum EXPORT_ENTRY_ATTRIBUTE{	
	EXPORT_ENTRY_ATTRIBUTE_CACHED			= 1,
	EXPORT_ENTRY_ATTRIBUTE_VERIFIED			= 2
};


#define EXPORT_TABLE "Export_Table"
#define	EXPORT_TABLE_VERSION	3

typedef struct ExportTableEntry {
	rowID			rid;					// rowID of this table row.
	U32 			version;				// Version of Export Table record.
	U32				size;					// Size of Export Table record in bytes.
	rowID			ridVcId;				// For VCM use ONLY.  rowID of VCM Command that created this circuit.
	CTProtocolType	ProtocolType;			// FCP, IP, other
	VDN				vdNext;					// First Virtual Device number in the chain
	VDN				vdLegacyBsa;			// Virtual Device number of the legacy BSA
	VDN				vdLegacyScsi;			// Virtual Device number of the legacy SCSI
	U32				ExportedLUN;			// LUN number exported
	U32				InitiatorId;			// Host ID
	U32				TargetId;				// our ID
	U32				FCInstance;				// FC Loop number
	String32		SerialNumber;			// Use a string array for Serial Number
	I64				Capacity;				// Capacity of this Virtual Circuit
	U32				FailState;
	CTReadyState	ReadyState;				// Current state
	CTReadyState	DesiredReadyState;		// Desired Ready state
	String16		WWNName;				// World Wide Name (64 or 128-bit IEEE registered)
	rowID			ridUserInfo;			// rowID of user info for this VC
	rowID			ridHC;					// rowID of the appropriate HostConnection descriptor
	rowID			ridStatusRec;			// rowID of Status Reporter for this record
	rowID			ridPerformanceRec;		// rowID of Performance Reporter for this record
	rowID			ridAltExportRec;		// rowID of our Alternate Path's Export Record
	rowID			ridSRC;					// rowID of the storage element for this record
	U32				attribMask;				// a bit mask of EXPORT_ENTRY_ATTRIBUTE attributes
	} ExportTableEntry, ExportTableRecord;

// Field names....so people could use Field methods

#define	RID_USER_INFO_NAME	"ridUserInfo"
#define	RID_VC_ID			"ridVcId"
#define	READY_STATE			"ReadyState"


#endif