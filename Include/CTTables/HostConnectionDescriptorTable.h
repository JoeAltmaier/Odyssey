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
// This is the Host Connections Descriptor Table
// 
// Update Log:
// 06/09/99 Andrey Gusev: Created the file
/*************************************************************************/

#ifndef __HOST_CONNECTION_DESCRIPTOR_TABLE_H__
#define __HOST_CONNECTION_DESCRIPTOR_TABLE_H__

#include "CtTypes.h"
#include "PTSCommon.h"

// Field definitions in ExportTable.cpp
extern	fieldDef	HostConnectionDescriptorTable_FieldDefs[];
extern	U32			cbHostConnectionDescriptor_FieldDefs;

#pragma	pack(4)

#define	HOST_CONNECTION_DESCRIPTOR_TABLE_VERSION	1
#define HOST_CONNECTION_DESCRIPTOR_TABLE_NAME		"HostConnDescriptorTable"
#define MAX_HOST_CONNECTIONS						32

// HostConnectionModeEnum - This enum determines the mode in which the 
// indivvidual connection(s) to host(s) will operate.  Each mode is
// defined below. 
enum HostConnectionModeEnum {
	eHCM_NON_REDUNDANT,		// There is only one connection to one host
							// Not officially supported as of 7/23/99
							// for internal testing use only.
	
	eHCM_ACTIVE_PASSIVE,	// Only one connection is active at any time
							// recpetion of traffic on any passive port
							// quiesces any currently active port(s) and
							// makes the receiving port the active port.

	eHCM_MULTIPLE_READERS,	// Makes all ports available for read only
							// activity.  SCSI Writes requests will be 
							// replied to with a SCSI error.
							
	eHCM_MULTIPLE_WRITERS,	// Makes all Ports available for both reading
							// and writing.  Detection of a read on any
							// port flushes all writers to assure coherency.
							
	eHCM_TRUSTED_WRITERS,	// Makes all Ports available for both reading
							// and writing like eHCM_MULTIPLE_WRITERS, but 
							// no coherency support is implemented.  
							// Coherency is trusted to the clients.
							
	eHCM_COHERENT_CACHE,	// Makes all Ports available for both reading
							// and writing.  Detection of a write is shared
							// among all ports to assure cache coherency.

	ePATH_TYPE_REDUNDANT,
	ePATH_TYPE_CLUSTERED
};


//
// External Initiator Port (EIP) Flag Value/Mask defines
//
#define	mskfEipReadEnabled	0x00000001	// This external Initiator may read.
#define mskfEipWriteEnabled	0x00000002	// This external initiator may write.
#define mskfEip
#define mskfUpdateLoopDesc	0x80000000

struct HostConnectionDescriptorRecord {
	rowID					rid;								// rowID of this table row.
	U32 					version;							// Version of the record.
	U32						size;								// Size of the record in bytes.
	HostConnectionModeEnum	eHostConnectionMode;				// Determines the mode in which this
																// Host connection operates.  See above.
	U32						ridEIPCount;						// number of rids in the ridEIPs array
	rowID					ridEIPs[MAX_HOST_CONNECTIONS];		// row ids of the EIP descriptors
	U32						InitiatorID[MAX_HOST_CONNECTIONS];	// InitiatorID to see this Virtual Circuit.  -1 => all Initiators.
	U32						TargetID[MAX_HOST_CONNECTIONS];		// Export TargetID for this Virtual Circuit.
	U32						LUN[MAX_HOST_CONNECTIONS];			// Export LUN for this Virtual Circuit.
	U32						flgEIPs[MAX_HOST_CONNECTIONS];		// Array of flags for each EIP.	
	rowID					ridName;
	rowID					ridDescription;		
	rowID					ridHost;
};


#define HOST_CONNECTION_TABLE_FN_RID_NAME			"ridName"
#define HOST_CONNECTION_TABLE_FN_RID_DESCRIPTION	"ridDescription"

#endif // __HOST_CONNECTION_DESCRIPTOR_TABLE_H__