/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: DeviceDescriptor.h
// 
// Description:
// This is the Device Descriptor Table.  This table includes all non-storage
// FC and SCSI devices 
// 
// Update Log:
// $Log: /Gemini/Include/CTTables/DeviceDescriptor.h $
// 
// 2     1/11/00 5:26p Agusev
// 
// 1     12/10/99 9:45p Mpanas
// New files to support Failover the correct way
// - PathDescriptor.h  .cpp
// - DeviceDescriptor.h  .cpp
// 
// 
// 12/07/99 Michael G. Panas: Create file
/*************************************************************************/

#ifndef _DeviceDescriptor_h
#define _DeviceDescriptor_h

#include "Scsi.h"
#include "CtTypes.h"
#include "Odyssey.h"
#include "PTSCommon.h"

// Field definitions in DeviceDescriptor.cpp
extern	fieldDef	DeviceDescriptorTable_FieldDefs[];
extern	U32			cbDeviceDescriptorTable_FieldDefs;

#pragma	pack(4)

#define DEVICE_DESC_TABLE "Device_Descriptor_Table"
#define	DEVICE_DESC_VERSION	1

typedef struct DeviceDescriptor {

        static const fieldDef *FieldDefs()		{ return DeviceDescriptorTable_FieldDefs; }
        static const U32 FieldDefsSize()	{ return cbDeviceDescriptorTable_FieldDefs;  }
        static const char* const TableName()		{ return DEVICE_DESC_TABLE; }

	rowID			rid;					// rowID of this table row.
	U32 			version;				// Version of Device Descriptor record.
	U32				size;					// Size of Device Descriptor record in bytes.
	U32				fUsed;					// if this device is used for something (bool)
	U32				SlotID;					// physical/scan slot of device
	U32				fSNValid;				// Serial Number valid flag
	String32		SerialNumber;			// Device serial number	
	String16		WWNName;				// World Wide Name (64 or 128-bit IEEE registered)
	U32				CurrentStatus;			// see DriveStatus enum in Odyssey.h
	U32				Type;					// Our Device type
	INQUIRY			InqData;				// read from drive. 56 bytes defined in scsi.h
	rowID			ridVendor;				// rowID of the Vendor Enum for this device
	rowID			ridName;				// rowID of the name string in of the UnicodeString tables
	} DeviceDescriptorRecord, *pDeviceDescriptorRecord;
	
#endif