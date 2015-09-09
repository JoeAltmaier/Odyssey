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
// This is the Ddm Class descriptor record and table.  These records
// describe the classes of Ddm present and available on each IOP.
// 
// $Log: /Gemini/Include/CTTables/DdmClassDescriptorTable.h $
// 
// 5     3/18/99 5:23p Jlane
// Modified to support corresponding .cpp file with satatis field
// definitions.
//
// Update Log: 
// 02/08/99 JFL	Created.
// 02/28/99 JFL Compiled.  #ifdef'd out ctor.
//
/*************************************************************************/

#ifndef __DdmClassDescriptorTable_h
#define __DdmClassDescriptorTable_h

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

typedef struct {	// One row for each class type on each IOP.
	rowID		rid;				// rowID of this record.
	U32			version;			// Version of DdmClassDescriptorRecord record.
	U32			size;				// Size of DdmClassDescriptorRecord record in bytes.
	int 		iCabinet;			// The cabinet in which the Ddm class exists.
	TySlot		iSlot;				// The slot in which this Ddm class exists.
	String32	ClassName;			// Name of the Class.
	U32			ClassVersion;		// Version of the Class.
	U32			ClassRevision;		// Revision of the Class.
	rowID		ridPropertySheet;	// 0th item in Class Config Table.?
	U32			sStack;				// Stack size for the Class.
	U32			sQueue;				// The size of the msg Queue for the Ddm.
	#if false
	Ddm* 		(*ctor)(DID);		// ctor is a pointer to a function taking a VirtualDevice
									// and returning a pointer to a DDM.
	#endif 
} DdmClassDescriptorRecord, DdmClassDescriptorTable[];

//  compiler-checkable aliases for table / field names

//  table name
#define  CT_DCDT_TABLE_NAME		"Ddm_Class_Descriptor_Table"

//  field defs
#define  CT_DCDT_VERSION			"version"			// Version of IOP_Status record.
#define  CT_DCDT_SIZE				"size"				// # of bytes in record.
#define  CT_DCDT_ICABINET			"iCabinet"			// The cabinet in which the Ddm class exists.
#define  CT_DCDT_ISLOT				"iSlot"				// The slot in which this Ddm class exists.
#define  CT_DCDT_CLASSNAME			"ClassName"			// Name of the Class.
#define  CT_DCDT_CLASSVERSION		"ClassVersion"		// Version of the Class.
#define  CT_DCDT_CLASSREVISION		"ClassRevision"		// Revision of the Class.
#define  CT_DCDT_RIDPROPERTYSHEET	"ridPropertySheet"	// What does this look like?  Pointer to 0th item in Class Config Table.?
#define  CT_DCDT_SSTACK				"sStack"			// Stack size for the Class.
#define  CT_DCDT_SQUEUE				"sQueue"			// The size of the msg Queue for the Ddm.
#define  CT_DCDT_PRIMARYCTOR		"ctor"				// ctor is a pointer to a function taking a DID.


//  here is the standard table which defines IOP Status table fields
extern const fieldDef aDdmClassDescriptorTable_FieldDefs[];

//  and here is the size, in bytes, of the IOP Status field defs
extern const U32 cbDdmClassDescriptorTable_FieldDefs;



#endif	// __DdmClassDescriptorTable_h