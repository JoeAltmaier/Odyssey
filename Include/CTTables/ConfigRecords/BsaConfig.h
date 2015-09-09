/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: BsaConfig.h
// 
// Description:
// This file defines the configuration data for the BsaIsm DDM. 
// 
// Update Log 
//	$Log: /Gemini/Include/CTTables/ConfigRecords/BsaConfig.h $
// 
// 3     8/25/99 1:26p Egroff
// Updated to have rid and fielddef accessor methods.
// 
// 1     3/12/99 9:07p Mpanas
// Initial checkin
// Bsa ISM config data
// 
// 03/12/99 Michael G. Panas: Create file
/*************************************************************************/
#if !defined(BsaConfig_H)
#define BsaConfig_H

#pragma	pack(4)

#include "PtsCommon.h"

extern const fieldDef	BSAConfig_FieldDefs[];
extern const U32		cbBSAConfig_FieldDefs;

#define BSA_CONFIG_TABLE_NAME	"BSA_Config"
#define BSA_CONFIG_VERSION 1

/*************************************************************************/
//    BSA_CONFIG
//    defines configuration structure
/*************************************************************************/
typedef struct _BsaIsmConfData {

    	static const fieldDef *FieldDefs()		{ return BSAConfig_FieldDefs;   }
    	static const U32 FieldDefsSize()		{ return cbBSAConfig_FieldDefs;  }
    	static const char* const TableName()	{ return BSA_CONFIG_TABLE_NAME; }

		rowID	rid;
		U32		version;
		U32		size;
		U32		LUN;			// drive LUN number
		U32		ID;				// drive target ID
		VDN		initVd;			// VD assigned to the Initiator DDM
		U32		EnableSMART;	// TODO:
	} BSA_CONFIG;

#endif