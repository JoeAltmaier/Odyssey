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
// This is the Disk Object Descriptor Table
// 
// Update Log:
// $Log: /Gemini/Include/CTTables/DiskDescriptor.h $
// 
// 14    1/11/00 7:21p Mpanas
// Support for new PathDescriptor table
// 
// 13    12/16/99 1:05a Ewedel
// Added RqPts_T support.
// 
// 12    9/13/99 1:32p Mpanas
// Add CTDiskType field to specify int, ext, FC and SCSI
// Hey, some folks may need to know these things...
// 
// 11    9/05/99 6:45p Mpanas
// Add LUN and LockState fields
// 
// 10    6/07/99 10:37p Mpanas
// Add fields to the Export and DiskDescriptor
// tables to support the future ISP2200 and
// Multiple FCInstances
// 
// 9     3/09/99 3:11p Mpanas
// Add refs to the field defs for this table
// 
// 8     3/04/99 9:43p Mpanas
// Added Table name and version
// 
// 8/31/98 Michael G. Panas: Create file
// 9/8/98 Michael G. Panas: Change class to struct for easier access
// 9/29/98 Jim Frandeen: Use CtTypes.h instead of stddef.h
// 10/12/98 J. Lane: Added post-spec review struct renamed to DiskDescriptor.
// 12/12/98 J. Lane: Changed #ifndef and #define to reflect new module name.
// 11/03/98	J. Lane: added rid_status_record and rid_performance_record.
//					 and also ridThisRow.
// 2/21/99 Michael G. Panas: remove I2O dependancy
/*************************************************************************/

#ifndef _DiskDescriptor_h
#define _DiskDescriptor_h

#include "Scsi.h"
#include "CtTypes.h"
#include "Odyssey.h"
#include "PTSCommon.h"

#ifndef __RqPts_T_h
# include  "RqPts_T.h"
#endif

// Field definitions in DiskDescriptor.cpp
extern	fieldDef	DiskDescriptorTable_FieldDefs[];
extern	U32			cbDiskDescriptorTable_FieldDefs;

enum LockState {
	LOCK_STATE_UNUSED = 0,		// LockState only valid for DDH drives
	LOCK_STATE_UNKNOWN = 1,		// LockState value is unknown
	DRIVE_LOCKED = 2,
	DRIVE_UNLOCKED
};

#pragma	pack(4)

#define DISK_DESC_TABLE "Disk_Descriptor_Table"
#define	DISK_DESC_VERSION	4

struct DiskDescriptor {

        static const fieldDef *FieldDefs()		{ return DiskDescriptorTable_FieldDefs; }
        static const U32 FieldDefsSize()	{ return cbDiskDescriptorTable_FieldDefs;  }
        static const char* const TableName()		{ return DISK_DESC_TABLE; }

	rowID			rid;					// rowID of this table row.
	U32 			version;				// Version of Disk Object descriptor record.
	U32				size;					// Size of Disk Object descriptor record in bytes.
	U32				SlotID;					// physical slot of drive
	U32				fSNValid;				// Serial Number valid flag
	String32		SerialNumber;			// Device serial number	
	String16		WWNName;				// World Wide Name (64 or 128-bit IEEE registered)
	U32				CurrentStatus;			// see DriveStatus enum in Odyssey.h
	CTDiskType		DiskType;				// Drive Type (int, ext, FC, and SCSI)
	LockState		LockState;				// Drive Lock state (DDH drives only)
	U64				Capacity;				// Disk formatted capacity in bytes.
	INQUIRY			InqData;				// read from drive. 56 bytes defined in scsi.h
	rowID			ridVendor;				// rowID of the Vendor Enum for this device
	
    //  some PTS interface message typedefs
    typedef RqPtsDefineTable_T<DiskDescriptor> RqDefineTable;
    typedef RqPtsDeleteTable_T<DiskDescriptor> RqDeleteTable;
    typedef RqPtsQuerySetRID_T<DiskDescriptor> RqQuerySetRID;
    typedef RqPtsEnumerateTable_T<DiskDescriptor> RqEnumerateTable;
    typedef RqPtsInsertRow_T<DiskDescriptor> RqInsertRow;
    typedef RqPtsModifyRow_T<DiskDescriptor> RqModifyRow;
    typedef RqPtsModifyField_T<DiskDescriptor> RqModifyField;
    typedef RqPtsModifyBits_T<DiskDescriptor> RqModifyBits;
    typedef RqPtsTestAndSetField_T<DiskDescriptor> RqTestAndSetField;
    typedef RqPtsReadRow_T<DiskDescriptor> RqReadRow;
    typedef RqPtsDeleteRow_T<DiskDescriptor> RqDeleteRow;
    typedef RqPtsListen_T<DiskDescriptor> RqListen;

	};
	
typedef DiskDescriptor DiskDescriptorRecord;
typedef DiskDescriptor *pDiskDescriptorRecord;

#endif
	