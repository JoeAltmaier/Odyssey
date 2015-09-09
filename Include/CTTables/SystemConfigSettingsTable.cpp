/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: SystemConfigSettingsTable.cpp
// 
// Description:
// This module is the Table definition of the SystemConfigSettingsTable
/*************************************************************************/

#include "OsTypes.h"
#include "PTSCommon.h"
#include "SystemConfigSettingsTable.h"



	fieldDef	SystemConfigSettingsTableFieldDefs[] = {
		// Field Definitions follow one per row.
		// FieldName				Size						Type
		"version",					4,						U32_FT,		Persistant_PT,
		"size",						4,						U32_FT,		Persistant_PT,
		"ipAddress",				sizeof(IP_ADDRESS),		BINARY_FT,	Persistant_PT,
		"subnetMask",				sizeof(IP_ADDRESS),		BINARY_FT,	Persistant_PT,
		"hostName",					sizeof(UnicodeString32),BINARY_FT,	Persistant_PT,
		"location",					sizeof(UnicodeString64),BINARY_FT,	Persistant_PT,
		"gmtTimeBase",				sizeof(GMT_BASE_TIME),	BINARY_FT,	Persistant_PT,
		"trapIpAddressCount",		4,						U32_FT,		Persistant_PT,
		"trapIpAddress",			sizeof(IP_ADDRESS)*SCS_SNMP_TRAP_ADDRESS_MAX_COUNT,		BINARY_FT,	Persistant_PT,
		"sysLogIpAddress",			sizeof(IP_ADDRESS),		BINARY_FT,	Persistant_PT,
		"gateway",					sizeof(IP_ADDRESS),		BINARY_FT,	Persistant_PT,
	};

// defined here so other folks can get to it	
U32 cbSystemConfigSettingsTableFieldDefs = 
				sizeof(SystemConfigSettingsTableFieldDefs);
				

