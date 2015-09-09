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
// This is the Row Id Collection Table. It will may be used by whoever
// wants to store 1 to many relationships in the PTS.
// 
// Update Log:
// 06/29/99 Andrey Gusev: Created the file
/*************************************************************************/

#ifndef __ROWID_COLLECTION_TABLE__
#define __ROWID_COLLECTION_TABLE__

#include "CtTypes.h"
#include "PTSCommon.h"

extern	fieldDef	RowIdCollection_FieldDefs[];
extern	U32			cbRowIdCollection_FieldDefs;

#pragma	pack(4)

#define	ROWID_COLLECTION_TABLE_VERSION	1


struct RowIdCollectionRecord {
	rowID				rid;				// rowID of this table row.
	U32 				version;			// Version of the record.
	U32					size;				// Size of the record in bytes.
	rowID				ridPointer;			// rowID of the "1" in your 1 to many relationship
};

#endif // __ROWID_COLLECTION_TABLE__