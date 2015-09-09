/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FCPortDatabase.h
// 
// Update Log 
//	$Log: /Gemini/Include/CTTables/FCPortDatabaseTable.h $
// 
// 9     2/09/00 6:02p Dpatel
// Fix for Jerry's bug of not being able to export LUNs to initiators
// on OUR initiator loop
// 
// 8     2/09/00 2:32p Mpanas
// Split wwName field to Port and Node wwn
// Add Flags for Owner field
// 
// 7     8/14/99 9:55p Mpanas
// Support for new LoopMonitor
// Add enum for FC_PORT_STATUS
// 
// Description:
// This is the Fibre Channel Port Database Table populated with all 
// the ports present on all FC loops connected to our NACs
/*************************************************************************/

#ifndef __FC_PORT_DATABASE_TABLE_H__
#define __FC_PORT_DATABASE_TABLE_H__

#include "CtTypes.h"
#include "PTSCommon.h"

extern	fieldDef	FCPortDatabaseTable_FieldDefs[];
extern	U32			cbFCPortDatabase_FieldDefs;

#pragma	pack(4)

#define FC_PORT_DATABASE_TABLE_NAME		"FCPortDatabaseRecordTable"
#define	FC_PORT_DATABASE_TABLE_VERSION	1

enum FC_PORT_TYPE {	
		FC_PORT_TYPE_TARGET	=	1,
		FC_PORT_TYPE_INITIATOR,
		FC_PORT_TYPE_TARGET_INITIATOR
};

enum FC_PORT_STATUS {
		FC_PORT_STATUS_ACTIVE = 1,
		FC_PORT_STATUS_REMOVED,
		FC_PORT_STATUS_LOOP_DOWN
};

// Flag bits for the Owner field
#define	FC_PORT_OWNER_INTERNAL		0x01	// 1 = we own, 0 = external
#define	FC_PORT_OWNER_VC_USE_OK		0x02	// 1 = valid for a virtual circuit, 0 = vc not OK

struct FCPortDatabaseRecord{
	rowID			rid;					// rowID of this table row.
	U32 			version;				// version of this record
	U32				size;					// size of this record
	rowID			ridLoopDescriptor;		// rid of the Loop descriptor this port is found on
	U32				id;						// target || initiator loop ID
	U8				wwName[8];				// World Wide Name of this Port
	U8				wwnNodeName[8];			// World Wide Name of this Node
	U32				portType;				// see PRESENT_PORT_TYPE
	U32				portStatus;				// port status
	U32				attribs;				// Bit-vector, look above
	rowID			ridName;				// rid of some unicode string table with name
};

#define	FCP_PORT_DTB_TABLE_FN_RID_NAME	"ridName"
	
#endif // __FC_PORT_DATABASE_TABLE_H__