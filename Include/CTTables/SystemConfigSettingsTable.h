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
// This is the SystemConfigSettingsTable
// 
/*************************************************************************/

#ifndef __SYSTEM_SETTINGS_TABLE_H__
#define __SYSTEM_SETTINGS_TABLE_H__

#include "CtTypes.h"
#include "Odyssey.h"
#include "PTSCommon.h"
#include "Simple.h"
#include "RqPts_T.h"
#include "StringClass.h"
#include "UnicodeString.h"


// Field definitions in LoopDescriptor.cpp
extern	fieldDef	SystemConfigSettingsTableFieldDefs[];
extern	U32			cbSystemConfigSettingsTableFieldDefs;

#pragma	pack(4)


#define SYSTEM_CONFIG_SETTINGS_TABLE_NAME		"SystemConfigSettings"
#define	SYSTEM_CONFIG_SETTINGS_VERSION			1
#define SCS_SNMP_TRAP_ADDRESS_MAX_COUNT			10

typedef U32			IP_ADDRESS;
typedef	I64			GMT_BASE_TIME;

struct SystemConfigSettingsRecord 
{
	rowID  				rid;   				// rowID of this record.
	U32   				version;   			// record version
	U32  				size;   			// record size in bytes.
	IP_ADDRESS			ipAddress;			// IP for the box when using SSAPI
	IP_ADDRESS			subnetMask;			// subnet mask for the above IP
	UnicodeString32		hostName;			// arbitrary for now, DNS-ressolved later
	UnicodeString64		location;			// for the User to set
	GMT_BASE_TIME		gmtBaseTime;		// time base for timestamps
	U32					snmpTrapAddressCount;// number of IPs in the array below
	IP_ADDRESS			snmpTrapAddress[SCS_SNMP_TRAP_ADDRESS_MAX_COUNT];	// for SNMT traps
	IP_ADDRESS			sysLogIpAddress;	// for the 2nd release
	IP_ADDRESS			gateway;			// system gateway address
	
	SystemConfigSettingsRecord()
	{
		UnicodeString us(StringClass("\n"));
		rid = RowId(0);
		version = SYSTEM_CONFIG_SETTINGS_VERSION;
		size = sizeof(SystemConfigSettingsRecord);
		ipAddress = 0;
		subnetMask = 0;
		us.CString(&hostName, sizeof(hostName));
		us.CString(&location, sizeof(location));
		gmtBaseTime = 0;
		snmpTrapAddressCount = 0;
		for (U16 i = 0; i < SCS_SNMP_TRAP_ADDRESS_MAX_COUNT; i++)
			snmpTrapAddress[i] = 0;
		sysLogIpAddress = 0;
		gateway = 0;
	}
	
	static const char* TableName()
	{
		return(SYSTEM_CONFIG_SETTINGS_TABLE_NAME);
	}
	
	static const U32 FieldDefsSize()
	{
		return cbSystemConfigSettingsTableFieldDefs;
	}
	
	static const fieldDef* FieldDefs()
	{
		return SystemConfigSettingsTableFieldDefs;
	}
	
    //  some PTS interface message typedefs
    typedef RqPtsDefineTable_T <SystemConfigSettingsRecord>   RqDefineTable;
    typedef RqPtsInsertRow_T   <SystemConfigSettingsRecord>   RqInsertRow;
	typedef RqPtsModifyField_T <SystemConfigSettingsRecord>	   RqModifyField;
	typedef RqPtsEnumerateTable_T <SystemConfigSettingsRecord> RqEnumTable;
	typedef RqPtsListen_T <SystemConfigSettingsRecord> RqListen;
	
};


#endif	// __SYSTEM_SETTINGS_TABLE_H__