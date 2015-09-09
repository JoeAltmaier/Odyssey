/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: RdConfig.h
// 
// Description:
// This file defines the configuration data for the Ram Disk DDM. 
// 
// Update Log 
//	$Log: /Gemini/Include/CTTables/ConfigRecords/RDConfig.h $
// 
// 1     9/15/99 4:57p Jlane
// Initial Checkin.
// 
/*************************************************************************/
#if !defined(RdConfig_H)
#define RdConfig_H

#pragma	pack(4)

#include "PtsCommon.h"

extern const fieldDef	RDConfig_FieldDefs[];
extern const U32		cbRDConfig_FieldDefs;

#define RD_CONFIG_TABLE_NAME	"RAMDisk_Config"
#define RD_CONFIG_VERSION 1

/*************************************************************************/
//    RD_CONFIG
//    defines configuration structure
/*************************************************************************/
typedef struct _RdConfData {

    	static const fieldDef *FieldDefs()		{ return RDConfig_FieldDefs;   }
    	static const U32 FieldDefsSize()		{ return cbRDConfig_FieldDefs;  }
    	static const char* const TableName()	{ return RD_CONFIG_TABLE_NAME; }

		rowID	rid;			// Row ID of this record.
		U32		version;		// Version of this record
		U32		size;			// Size of this record
		U32		capacity;		// Capacity of the RamDisk
	} RD_CONFIG;

#endif