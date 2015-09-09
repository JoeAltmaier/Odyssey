/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FMConfig.h
// 
// Description:
// This file defines the configuration data for the Flash Monitor DDM. 
// 
// Update Log 
//	$Log: /Gemini/Include/CTTables/ConfigRecords/FMConfig.h $
// 
// 1     10/22/99 11:28a Hdo
// Initial check in
// 
/*************************************************************************/
#if !defined(FMConfig_H)
#define FMConfig_H

#pragma	pack(4)

#include "PtsCommon.h"

extern const fieldDef	FMConfig_FieldDefs[];
extern const U32		cbFMConfig_FieldDefs;

#define FM_CONFIG_TABLE_NAME	"FlashMonitor_Config"
#define FM_CONFIG_VERSION		1

/*************************************************************************/
//    FM_CONFIG
//    defines configuration structure
/*************************************************************************/
typedef struct _FMConfData {

    	static const fieldDef *FieldDefs()		{ return FMConfig_FieldDefs;   }
    	static const U32 FieldDefsSize()		{ return cbFMConfig_FieldDefs;  }
    	static const char* const TableName()	{ return FM_CONFIG_TABLE_NAME; }

		rowID		rid;			// Row ID of this record.
		U32			version;		// Version of this record
		U32			size;			// Size of this record
		String32	SerialNumber;	// Device serial number	
		VDN			vdnSSD;			// VDN of the SSD Ddm
	} FM_CONFIG;

#endif