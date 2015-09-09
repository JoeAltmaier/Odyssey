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
// This is the Host Collection Descriptor Table
// 
// Update Log:
// 06/20/99 Andrey Gusev: Created the file
/*************************************************************************/

#ifndef __HOST_COLLECTION_DESCRIPTOR_TABLE__
#define __HOST_COLLECTION_DESCRIPTOR_TABLE__

#include "CtTypes.h"
#include "PTSCommon.h"

// Field definitions in ExportTable.cpp
extern	fieldDef	HostCollectionDescriptorTable_FieldDefs[];
extern	U32			cbHostCollectionDescriptor_FieldDefs;

#pragma	pack(4)

#define	HOST_COLLECTION_DESCRIPTOR_TABLE_VERSION	1
#define HOST_COLLECTION_DESCRIPTOR_TABLE_NAME		"HostCollectionDescriptorTable"

enum HOST_COLLECTION_TYPE{ REDUNDANT = 1, CLUSTER };

struct HostCollectionDescriptorRecord {
	rowID					rid;				// rowID of this table row.
	U32 					version;			// Version of the record.
	U32						size;				// Size of the record in bytes.
	U32						isActive;			// bool
	UnicodeString64			description;		// user-provided description?
	String32				collectionTableName;// table with pointers to agregated elements
	HOST_COLLECTION_TYPE	collectionType;		// see above
	rowID					ridLUN;				// rid of the LUN which is exporter thru this collection
	UnicodeString32			name;
};

#endif // __HOST_COLLECTION_DESCRIPTOR_TABLE__