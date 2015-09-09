/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: SSDConfig.h
// 
// Description:
// This file defines the configuration data for the SSD DDM.
// 
// Update Log 
//	$Log: /Gemini/Include/CTTables/ConfigRecords/SSDConfig.h $
// 
// 3     11/22/99 10:44a Jfrandeen
// Use new FlashStorage in place of FlashFile
// 
// 2     10/27/99 10:35a Hdo
// Add more fields: SerialNumber, vdnBSA, vdMonitor
// 
// 1     10/01/99 5:51p Hdo
// Initial check-in
// 
/*************************************************************************/
#if !defined(SsdConfig_H)
#define SsdConfig_H

#pragma	pack(4)

#include "PtsCommon.h"
#include "IopStatusTable.h"

extern const fieldDef	SSD_Config_FieldDefs[];
extern const U32		cbSSD_Config_FieldDefs;

// MEMORY_FOR_SSD includes all memory used.
//#define MEMORY_FOR_SSD						16000000				// 16 megabytes
//#define MEMORY_FOR_CALLBACKS				100000					// 100K

// Page size used by flash file system and page manager
//#define SSD_PAGE_SIZE 4096

// Number of page table entries, assuming 20 bits for virtual page address
//#define SSD_NUM_VIRTUAL_PAGES 1048576

#define SSD_CONFIG_TABLE_NAME	"SSD_Config"
#define SSD_CONFIG_VERSION 1

/*************************************************************************/
//    SSD_CONFIG
//    defines configuration structure
/*************************************************************************/
typedef struct _SSDConfData {

		static const fieldDef *FieldDefs()		{ return SSD_Config_FieldDefs;  }
		static const U32 FieldDefsSize()		{ return cbSSD_Config_FieldDefs;}
 		static const char* const TableName()	{ return SSD_CONFIG_TABLE_NAME; }

		rowID			rid;			// Row ID of this record.
		U32				version;		// Version of this record
		U32				size;			// Size of this record
		I64				capacity;		// Capacity of the SSD
		VDN				vdnBSADdm;		// BSA Virtual Device number for this ID
		String32		SerialNumber;	// Device serial number	
		VDN				vdnMonitor;		// Virtual Device number of the SSD monitor

	// CM_CONFIG
		//U32				CM_version;
		//U32				CM_size;
		U32				page_size;
		U32 			page_table_size;
		U32				hash_table_size;
		U32				num_reserve_pages;
		U32				dirty_page_writeback_threshold;
		U32				dirty_page_error_threshold;
		U32				num_prefetch_forward;
		U32				num_prefetch_backward;

	// FF_CONFIG
		//U32				FF_version;
		//U32				FF_size;
		//U32				memory_size;
		//U32				callback_memory_size;
		U32				verify_write_level;
		U32				verify_erase_level;
		U32				percentage_erase_level;
		U32				percentage_replacement_pages;
		U32				replacement_page_threshold;
		U32				erase_all_pages;
		U32				verify_format_level;
		U32				verify_structures;
	} SSD_CONFIG;

#endif