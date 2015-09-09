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
// This is the Host Descriptor Table
// 
// Update Log:
// 06/09/99 Andrey Gusev: Created the file
/*************************************************************************/

#ifndef __HOST_DESCRIPTOR_TABLE_H__
#define __HOST_DESCRIPTOR_TABLE_H__

#include "CtTypes.h"
#include "PTSCommon.h"
#include "SystemConfigSettingsTable.h"


// Field definitions in ExportTable.cpp
extern	fieldDef	HostDescriptorTable_FieldDefs[];
extern	U32			cbHostDescriptor_FieldDefs;

#pragma	pack(4)

#define HOST_DESCRIPTOR_TABLE_NAME 			"HostDescriptorTable"
#define	HOST_DESCRIPTOR_TABLE_VERSION		1
#define HOST_DESCRIPTOR_TABLE_EIP_MAX_COUNT	20

struct HostDescriptorRecord {
	rowID				rid;					// rowID of this table row.
	U32 				version;				// Version of Host Descriptor Table record.
	U32					size;					// Size of HostDescriptorRecord in bytes.
	UnicodeString16		name;					// host name
	UnicodeString64		description;			// the description as entered by the user
	U32					hostOs;					// OS running on the host: localized string
	IP_ADDRESS			ipAddress;				// host IP address
	U32					eipCount;
	rowID				eip[HOST_DESCRIPTOR_TABLE_EIP_MAX_COUNT];
};

#define	HDT_FN_NAME			"name"
#define	HDT_FN_DESCRIPTION	"description"
#define	HDT_FN_HOST_OS		"hostOs"
#define	HDT_FN_EIP_COUNT	"eipCount"
#define	HDT_FN_EIP			"eip"
#define	HDT_FN_IP_ADDRESS	"ipAddress"


#endif // __HOST_DESCRIPTOR_TABLE_H__