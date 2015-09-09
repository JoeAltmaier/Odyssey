/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: SmConfig.h
// 
// Description:
// This file defines the configuration data for the ScsiMonitor DDM. 
// 
// Update Log 
//	$Log: /Gemini/Include/CTTables/ConfigRecords/SmConfig.h $
// 
// 2     10/12/99 5:47p Cchan
// Changes to support PTS/SmConfig.cpp
// 
// 1     9/17/99 12:21p Cchan
// Configuration header for ScsiMonitor ISM.
// 
/*************************************************************************/
#if !defined(SmConfig_H)
#define SmConfig_H

#pragma pack(4)

#include "PtsCommon.h"

#define SM_CONFIG_VERSION 1
#define SM_CONFIG_TABLE_NAME "scsi_monitor_config_table"

extern const U32 cbSmConfig_FieldDefs;
extern const fieldDef SmConfig_FieldDefs[];

/*************************************************************************/
//    SM_CONFIG
//    defines configuration structure
/*************************************************************************/
typedef struct _SM_CONFIG {

	static const fieldDef *FieldDefs()		{ return SmConfig_FieldDefs; }
	static const U32 FieldDefsSize()		{ return cbSmConfig_FieldDefs; }
	static const char* const TableName()	{ return SM_CONFIG_TABLE_NAME; }
	
	rowID			rid;
	unsigned int	version;
	unsigned int	size;
	unsigned int	num_drives;				// number of drives to scan
	unsigned int	flags;					// drive monitor flags
	VDN				vd;						// Initiator to talk to
	char			xlate[16];				// logical to physical ID table
} SM_CONFIG, *PSMCONFIG;

// ScsiMonitor flags
#define	SM_FLAGS_SCAN_ALL		1			// scan all drives on ddm start

// mode flag defines
#define	SM_MODE_INQ				1			// always do Inquiry on this drive
#define	SM_MODE_GET_SERIAL		2			// read the serial number from this drive
#define	SM_MODE_START			4			// start this drive when found
#define	SM_MODE_RD_CAP			8			// read capacity from this drive when found

#endif // SmConfig_H