/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: LmConfig.h
// 
// Description:
// This file defines the configuration data for the LoopMonitor DDM. 
// 
// Update Log 
//	$Log: /Gemini/Include/CTTables/ConfigRecords/LmConfig.h $
// 
// 3     8/25/99 1:26p Egroff
// Updated to have rid and fielddef accessor methods.
// 
// 2     7/15/99 11:22p Mpanas
// Changes to support Multiple FC Instances
// 
// 1     6/18/99 6:47p Mpanas
// Initial version of the LoopMonitor
// for ISP2200 support
// 
// 
// 06/11/99 Michael G. Panas: Create file
/*************************************************************************/
#if !defined(LmConfig_H)
#define LmConfig_H

#pragma	pack(4)

#include "Fc_loop.h"

#define LM_CONFIG_VERSION 1
#define LM_CONFIG_TABLE_NAME "Lun_Monitor_Config_Table"

extern const fieldDef	LmConfig_FieldDefs[];
extern const U32		cbLmConfig_FieldDefs;

/*************************************************************************/
//    LM_CONFIG
//    defines configuration structure
/*************************************************************************/
typedef struct LM_CONFIG {

	static const fieldDef *FieldDefs()		{ return LmConfig_FieldDefs;   }
    static const U32 FieldDefsSize()		{ return cbLmConfig_FieldDefs;  }
    static const char* const TableName()	{ return LM_CONFIG_TABLE_NAME; }

    rowID	rid;
	U32		version;
	U32		size;
	U32		num_loops;				// number of loops to scan
	U32		flags;					// monitor flags
	VDN		vd;						// FC_MASTER to talk to
	U32		FC_instance[MAX_FC_CHIPS];	// array of loop instance numbers
} LM_CONFIG, *PLMCONFIG;

#endif // LmConfig_H