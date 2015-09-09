/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: StsConfig.h
// 
// Description:
// This file defines the configuration data for each SCSI Target Server. 
// 
// Update Log 
//	$Log: /Gemini/Include/CTTables/ConfigRecords/StsConfig.h $
// 
// 7     9/01/99 12:08p Jlane
// Renamed ridVCMCommand to ridVcId.
// 
// 6     8/25/99 1:26p Egroff
// Updated to have rid and fielddef accessor methods.
// 
// 5     8/07/99 1:38p Jlane
// Add missing size and rid fields.  
// 
// 4     8/07/99 1:30p Jlane
// Changed vd to vdNext to match .cpp.
// 
// 3     8/07/99 1:25p Jlane
// Added ridVCMCommand.
// 
// 2     7/15/99 4:18p Jlane
// Added extern declarations for fielddefs.
// 
// 1     3/25/99 1:48p Mpanas
// Initail Checkin
// Scsi Target Server Config data
// 
// 03/12/99 Michael G. Panas: Create file
/*************************************************************************/
#if !defined(StsConfig_H)
#define StsConfig_H

#pragma pack(4)

#include "PtsCommon.h"

#define STS_CONFIG_VERSION 1

// Field definitions in ExportTable.cpp
extern	const fieldDef	StsConfigTable_FieldDefs[];
extern	const U32		cbStsConfigTable_FieldDefs;

#define STS_CONFIG_TABLE_NAME "SCSI_Target_Server_Config_Table"
#define	STS_TABLE_VERSION	3

/*************************************************************************/
//    STS_CONFIG
//    defines configuration structure
/*************************************************************************/
typedef struct _StsIsmConfData {
	
	static const fieldDef *FieldDefs()		{ return StsConfigTable_FieldDefs;   }
    static const U32 FieldDefsSize()		{ return cbStsConfigTable_FieldDefs;  }
    static const char* const TableName()	{ return STS_CONFIG_TABLE_NAME; }

    rowID			rid;			// row ID of the record.
	U32				version;		// version of this record.
	U32				size;			// size of this record.
	rowID			ridVcId;		// Used by Virtual Circuit Master to ID Virtual Circuit.
	VDN				vdNext;			// VD assigned to the next in the VC
} STS_CONFIG;


// Field name defines....for use in PTS  ops requiring Field Names.
#define	RID_VC_ID				"ridVcId"

#endif