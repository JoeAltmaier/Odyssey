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
// This is the SCSI Target Server Data Table
// The Inquiry string and Mode Pages for this Virtual Circuit are saved here
// 
// Update Log:
// $Log: /Gemini/Include/CTTables/STSData.h $
// 
// 1     11/05/99 3:16p Mpanas
// New Table to make the Inquiry and Mode pages 
// persistent.  This table is used by the SCSI Target
// Server
// 
// 11/03/99 Michael G. Panas: Create file
/*************************************************************************/

#ifndef _STSData_h
#define _STSData_h

#include "Scsi.h"
#include "CtTypes.h"
#include "Odyssey.h"
#include "PTSCommon.h"

// Field definitions in DiskDescriptor.cpp
extern	fieldDef	STSDataTable_FieldDefs[];
extern	U32			cbSTSDataTable_FieldDefs;


#pragma	pack(4)

#define STS_DATA_TABLE 		"Sts_Data_Table"
#define	STS_DATA_VERSION	1

typedef struct StsData {
	rowID			rid;					// rowID of this table row.
	U32 			version;				// Version of Disk Object descriptor record.
	U32				size;					// Size of Disk Object descriptor record in bytes.
	VDN				vdSTS;					// STS Virtual Device number
	INQUIRY			InqData;				// read from drive. 56 bytes defined in scsi.h
	String256		ModePages;				// free form mode page data
	} StsData, *pStsData;
	
#endif