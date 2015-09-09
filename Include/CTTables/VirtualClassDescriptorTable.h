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
// This is the Virtual Class descriptor record and table.  These records
// describe the classes of redundant Ddm's present & available on each IOP.
// 
// $Log: /Gemini/Include/CTTables/VirtualClassDescriptorTable.h $
// 
// 3     3/17/99 10:35p Jlane
// Updated with and for a corresdponding .cpp file that has the fielddefs.
//
// Update Log: 
// 02/08/99 JFL	Created.
// 02/28/99 JFL Compiled.
//
/*************************************************************************/

#ifndef __VirtualClassDescriptorTable_h
#define __VirtualClassDescriptorTable_h

#if !defined(CTtypes_H)
# include  "CtTypes.h"
#endif

#if !defined(PtsCommon_H)
# include  "PtsCommon.h"
#endif

#if !defined(__Address_h)
# include  "Address.h"
#endif

#pragma	pack(4)


typedef struct {	// One row for each class type
	rowID		rid;					// rowID of this record.
	U32 		version;				// Version of VirtualClassRecord.
	U32			size;					// Size of VirtualClassRecord in bytes.
	String32	ClassName;				// Name of the Class
	U32			ClassVersion;			// Version of the Class.
	U32			ClassRevision;			// Revision of the Class.
	rowID		ridPropertySheet;		// What does this look like?  Pointer to 0th item in Class Config Table.?
	U32			sStack;					// Stack size for the Class
	U32			sQueue;					// The size of the msg Queue for the Ddm.
	// These values are needed to initialize the VirtualDeviceTable DIDs when creating instances of this class.
	// Primary Ddm instantiation particulars:
	int 		iPrimaryCabinet;		// The cabinet in which the Ddm class exists.
	TySlot		iPrimarySlot;			// The slot in which this Ddm class exists.
	#if false
	Ddm* 		(*PrimaryCtor)(DID);	// ctor is a pointer to a function taking a DID
	#endif
	// Failover Ddm instantiation particulars:
	int 		iFailoverCabinet;		// The cabinet in which the Ddm class exists.
	TySlot		iFailoverSlot;			// The slot in which this Ddm class exists.
	#if false
	Ddm* 		(*FailoverCtor)(DID);	// ctor is a pointer to a function taking a DID
	#endif
} VirtualClassDescriptorRecord, *pVirtualClassDescriptorTable, VirtualClassDescriptorTable[];

//  compiler-checkable aliases for table / field names

//  table name
#define  CT_VCDT_TABLE_NAME		"Virtual_Class_Descriptor_Table"

//  field defs
#define  CT_VCDT_VERSION			"version"			// Version of IOP_Status record.
#define  CT_VCDT_SIZE				"size"				// # of bytes in record.
#define  CT_VCDT_CLASSNAME			"ClassName"			// Name of the Class.
#define  CT_VCDT_CLASSVERSION		"ClassVersion"		// Version of the Class.
#define  CT_VCDT_CLASSREVISION		"ClassRevision"		// Revision of the Class.
#define  CT_VCDT_RIDPROPERTYSHEET	"ridPropertySheet"	// What does this look like?  Pointer to 0th item in Class Config Table.?
#define  CT_VCDT_SSTACK				"sStack"			// Stack size for the Class.
#define  CT_VCDT_SQUEUE				"sQueue"			// The size of the msg Queue for the Ddm.
#define  CT_VCDT_IPRIMARYCABINET	"iPrimaryCabinet"	// The cabinet in which the Ddm class exists.
#define  CT_VCDT_IPRIMARYSLOT		"iPrimarySlot"		// The slot in which this Ddm class exists.
#define  CT_VCDT_PRIMARYCTOR		"PrimaryCtor"		// ctor is a pointer to a function taking a DID.
#define  CT_VCDT_IFAILOVERCABINET	"iFailoverCabinet"	// The cabinet in which the Ddm class exists.
#define  CT_VCDT_IFAILOVERSLOT		"iFailoverSlot"		// The slot in which this Ddm class exists.
#define  CT_VCDT_FAILOVERCTOR		"FailoverCtor"		// ctor is a pointer to a function taking a DID.


//  here is the standard table which defines IOP Status table fields
extern const fieldDef aVirtualClassDescriptorTable_FieldDefs[];

//  and here is the size, in bytes, of the IOP Status field defs
extern const U32 cbVirtualClassDescriptorTable_FieldDefs;


#endif	// __VirtualClassDescriptorTable_h
