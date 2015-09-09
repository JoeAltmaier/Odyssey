/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: EsConfig.h
// 
// Description:
// This file defines the configuration data for each Echo SCSI DDM. 
// 
// Update Log 
//	$Log: /Gemini/Include/CTTables/ConfigRecords/EsConfig.h $
// 
// 2     8/25/99 1:26p Egroff
// Updated to have rid and fielddef accessor methods.
// 
// 1     7/28/99 5:48p Mpanas
// Config file for the Echo SCSI DDM
// 
// 
// 07/28/99 Michael G. Panas: Create file
/*************************************************************************/
#if !defined(EsConfig_H)
#define EsConfig_H

#pragma	pack(4)

#include "PtsCommon.h"

#define ES_CONFIG_VERSION 1

extern const fieldDef	EsConfig_FieldDefs[];
extern const U32		cbEsConfig_FieldDefs;

#define ES_CONFIG_TABLE_NAME "Echo_SCSI_Config_Table"
#define	ES_TABLE_VERSION	1


/*************************************************************************/
//    ES_CONFIG
//    defines configuration structure
/*************************************************************************/
typedef struct _EsIsmConfData {

	static const fieldDef *FieldDefs()		{ return EsConfig_FieldDefs;   }
	static const U32 FieldDefsSize()		{ return cbEsConfig_FieldDefs;  }
	static const char* const TableName()	{ return ES_CONFIG_TABLE_NAME; }

	rowID	rid;
	U32		version;
	U32		size;
	U32		ID;				// new ID
	U32		LUN;			// new LUN
	VDN		vdnNext;		// VD assigned to the next in the VC
} ES_CONFIG;

#endif // EsConfig_H