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
// This is the struct definition for the Flash Descriptor.
// 
// Update Log: 
// 11/02/98	JFL	Created.  Sunny but cool.  
/*************************************************************************/
#ifndef __FlashDescriptor_h
#define __FlashDescriptor_h

#include "CTTypes.h"

// The Flash Object Descriptor Table describes and identifies SSD IOPs.
//
#define FB_CONFIG_VERSION 2

typedef struct {
	U32 			version;				// Version of SSD descriptor record.
	U32				size;					// Size of FB_ SSD descriptor record in bytes.
	VDN				vdnBSADdm;				// BSA Virtual Device number for this ID
	String32		SerialNumber;
	U32				CurrentStatus;			// see DriveStatus enum above.
	I64				Capacity;
	rowID			rid_status_record;		// rowID of the status record for this device.
	rowID			rid_performance_record;	// rowID of the performance record for this device.
} FlashDescriptor;

#endif	// __FlashDescriptor_h