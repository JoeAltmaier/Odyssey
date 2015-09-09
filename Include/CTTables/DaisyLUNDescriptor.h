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
// This is the struct definition for the diasy chain LUN Descriptor.
// 
// Update Log: 
// 11/02/98	JFL	Created.  Sunny but cool.
// 02/28/99	JFL	Added fDiscoveryFlags and synched up with Table Spec.  
/*************************************************************************/
#ifndef __DaisyLUNDescriptor_h
#define __DaisyLUNDescriptor_h

#include "CTTypes.h"
#include "Scsi.h"

#pragma	pack(4)

enum HBCID {
	HBC0	= 0,
	HBC1	= 1
	};
	
//
// The DasyChainLUNDescriptor describes and identifies LUNs available
// on the daisy chain.
//
typedef struct ExternalStorageDescriptorRecord {
	rowID			rid;					// rowID of this record.
	U32 			version;				// Version of Daisy Chain Object Descriptor record.
	U32				size;					// Size of Daisy Chain Object Descriptor record in bytes.
	HBCID			HBC;					// Which HBC FC-AL Monitor made the discovery.
	U16				FCTargetID;				// Discovered Target ID
	U16				FCTargetLUN;			// Discovered LUN
	VDN				vdnBSADdm;				// BSA Virtual Device number for this disk should this be a RowID?
	String32		SerialNumber;
	U32				CurrentStatus;			// see DriveStatus enum above.
	I64				Capacity;				// Ask Jim about type.  Is this in the inquiry data?
	INQUIRY			InqData;				// read from drive. 43 bytes defined in scsi.h
	rowID			rid_status_record;		// rowID of the status record for this device.
	rowID			rid_performance_record;	// rowID of the performance record for this device.
	U32				fDiscoveryFlags;		// Not_Discovered, Discovered...??? TBD. 
 } ExternalStorageDescriptorRecord, ExternalStorageDescriptorTable[];

#endif	// __DaisyLUNDescriptor_h