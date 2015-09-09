/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: ExportTable.cpp
// 
// Description:
// This module is the Table definition of the Export Table
//
//	Note:  There should only be one of these for each
//         loaded image!
// 
// Update Log:
//	$Log: /Gemini/Include/CTTables/ExportTable.cpp $
// 
// 10    9/01/99 12:09p Jlane
// Renamed ridVCMCommand to ridVcId.
// 
// 9     8/28/99 8:38p Agusev
// 
// 8     8/06/99 9:16a Vnguyen
// 
// 8	 8/06/99 9:10a Vnguyen
// Rename rowID from ridSTSStatusRec, ridSTSPerformanceRec to
// ridStatusRec and ridPerformanceRec for consistency and future extension.
//
// 7     7/26/99 10:38a Jlane
// Added row IDs for Status and Performance records.
// 
// 6     7/22/99 5:44p Jlane
// New declaration to support multiple export recs per Virtual Circuit and
// also the new Host Connection table structure.
// 
// 5     7/06/99 5:35p Agusev
// Added a field to point to a HostConnectionCollection descriptor
// Modified a field name to be a #define it coul be modified by using the
// PTS'  ModifyField()
// 
// 4     7/02/99 6:53p Mpanas
// Add the Failover loop number, remove
// "name" since it is not used, and add
// the ridUserInfo for Andre to point to
// the user string info
// 
// 3     6/07/99 10:37p Mpanas
// Add fields to the Export and DiskDescriptor
// tables to support the future ISP2200 and
// Multiple FCInstances
// 
// 2     3/30/99 9:31p Mpanas
// Missed the size field
// 
// 1     3/09/99 4:33p Mpanas
// Initial Checkin
// This module is the Table definition of the Export Table
// 
// 
// 03/09/99 Michael G. Panas: Create file
/*************************************************************************/

#include "OsTypes.h"
#include "PTSCommon.h"
#include "ExportTable.h"




	fieldDef	ExportTable_FieldDefs[] = {
		// Field Definitions follow one per row.
		// FieldName							  Size   Type
//		"rowID",									8,	ROWID_FT, Persistant_PT,
		"version",									4,	U32_FT, Persistant_PT,
		"size",										4,	U32_FT, Persistant_PT,
		"ridVcId",									8,	ROWID_FT, Persistant_PT,
		"ProtocolType",								4,	U32_FT, Persistant_PT,
		"vdNext",									4,	U32_FT, Persistant_PT,
		"vdLegacyBsa",								4,	U32_FT, Persistant_PT,
		"vdLegacyScsi",								4,	U32_FT, Persistant_PT,
		"ExportedLUN",								4,	U32_FT, Persistant_PT,
		"InitiatorId",								4,	U32_FT, Persistant_PT,
		"TargetId",									4,	U32_FT, Persistant_PT,
		"FCInstance",								4,	U32_FT, Persistant_PT,
		"SerialNumber",								32,	STRING32_FT, Persistant_PT,
		"Capacity",									8,	U64_FT, Persistant_PT,
		"FailState",								4,	U32_FT, Persistant_PT,
		"ReadyState",								4,	U32_FT, Persistant_PT,
		"DesiredReadyState",						4,	U32_FT, Persistant_PT,
		"WWNName",					  				16,	STRING16_FT, Persistant_PT,
		RID_USER_INFO_NAME,							8,	ROWID_FT, Persistant_PT,
		"ridHC",									8,	ROWID_FT, Persistant_PT,
		"ridStatusRec",								8,	ROWID_FT, Persistant_PT,
		"ridPerformanceRec",						8,	ROWID_FT, Persistant_PT,
		"ridAltExportRec",							8,	ROWID_FT, Persistant_PT,
		"ridSRC",									8,	ROWID_FT, Persistant_PT,
		"attribMask",								4,	U32_FT,	  Persistant_PT	
	};

// defined here so other folks can get to it	
U32			cbExportTable_FieldDefs = 
				sizeof(ExportTable_FieldDefs);