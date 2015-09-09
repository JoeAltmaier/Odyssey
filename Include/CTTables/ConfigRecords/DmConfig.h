/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: DmConfig.h
// 
// Description:
// This file defines the configuration data for the DriveMonitor DDM. 
// 
// Update Log 
//	$Log: /Gemini/Include/CTTables/ConfigRecords/DmConfig.h $
// 
// 8     1/11/00 7:22p Mpanas
// PathTable changes
// 
// 7     12/21/99 3:57p Jlane
// Add fRedundant flag.
// 
// 6     12/21/99 2:01p Mpanas
// Add support for IOP Failover
// - make several modules IOP_LOCAL
// 
// 5     8/29/99 2:26p Mpanas
// Add a real set of global flag definitions
// 
// 4     8/25/99 1:26p Egroff
// Updated to have rid and fielddef accessor methods.
// 
// 3     7/15/99 11:23p Mpanas
// Changes to support Multiple FC Instances
// 
// 2     3/25/99 1:49p Mpanas
// Shorten Config data to fit 256
// 
// 1     3/11/99 5:21p Mpanas
// Initial checkin
// Drive Monitor config data
// 
// 03/04/99 Michael G. Panas: Create file
/*************************************************************************/
#if !defined(DmConfig_H)
#define DmConfig_H

#pragma	pack(4)

#include "PtsCommon.h"

#define DM_CONFIG_VERSION 2
#define DM_CONFIG_TABLE_NAME "drive_monitor_config_table"

extern const U32 cbDmConfig_FieldDefs;
extern const fieldDef DmConfig_FieldDefs[];

typedef char xlate_t[MAX_FC_IDS];		// logical to physical ID table

/*************************************************************************/
//    DM_CONFIG
//    defines configuration structure
/*************************************************************************/
typedef struct _DM_CONFIG {

	static const fieldDef *FieldDefs()		{ return DmConfig_FieldDefs;   }
    static const U32 FieldDefsSize()		{ return cbDmConfig_FieldDefs;  }
    static const char* const TableName()	{ return DM_CONFIG_TABLE_NAME; }

	rowID			rid;
	U32				version;
	U32				size;
	U32				num_drives;				// number of drives to scan
	U32				flags;					// drive monitor global flags
	VDN				vd;						// Initiator to talk to
	U32				FC_instance;			// our loop instance number
	//char			mode[MAX_FC_IDS];		// mode flag for each drive
	xlate_t			xlate;					// logical to physical ID table
} DM_CONFIG, *PDMCONFIG;

// drive monitor global flags (these are bit flags)
#define	DM_FLAGS_SCAN_ALL		0x001		// scan all drives on ddm start
#define	DM_FLAGS_LEGACY			0x002		// Legacy scan (future)
#define	DM_FLAGS_SCAN_MASK		0x00C		// mask for device LUN scan type
#define	DM_FLAGS_SCAN_BOTH		0x00C		// use GET_LUNS, if fail, scan 8 LUNS
#define	DM_FLAGS_SCAN_8_LUNS	0x004		// Scan only 8 LUNS
#define	DM_FLAGS_USE_GET_LUNS	0x008		// Use the GET_LUNS SCSI command
#define	DM_FLAGS_SPIN_TYPE_MASK	0x030		// device spinup type
#define	DM_FLAGS_SPIN_ALL		0x000		// Spin all devices in sequence
#define	DM_FLAGS_SPIN_2X		0x010		// Spin 2 devices at a time in sequence
#define	DM_FLAGS_SPIN_4X		0x020		// Spin 4 devices at a time in sequence
#define DM_FLAGS_AUTO_SCAN		0x040		// Use Int DDH or Ext legacy flags and xlate per slot#

#define	DM_FLAGS_SECONDARY		0x080		// 0 = Primary DM, 1 = Secondary DM
#define DM_FLAGS_REDUNDANT		0x100		// This Drive Monitor will have a failover partner HACK.

// mode flag defines (future use)
#define	DM_MODE_INQ				1			// always do Inquiry on this drive
#define	DM_MODE_GET_SERIAL		2			// read the serial number from this drive
#define	DM_MODE_START			4			// start this drive when found
#define	DM_MODE_RD_CAP			8			// read capacity from this drive when found

#endif // DmConfig_H