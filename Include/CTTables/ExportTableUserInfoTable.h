/*************************************************************************
* This material is a confidential trade secret and proprietary 
* information of ConvergeNet Technologies, Inc. which may not be 
* reproduced, used, sold or transferred to any third party without the 
* prior written consent of ConvergeNet Technologies, Inc.  This material 
* is also copyrighted as an unpublished work under sections 104 and 408 
* of Title 17 of the United States Code.  Law prohibits unauthorized 
* use, copying or reproduction.
*
* File Name:
* ExportTableUserInfo.h
*
* Description:
*
* This the table used to store row ids of various string resources
* supplied by the user when mapping a storage element to a LUN for further
* exporting.
*************************************************************************/

#ifndef __EXPORT_TABLE_USER_INFO_H__
#define __EXPORT_TABLE_USER_INFO_H__

#include "CtTypes.h"
#include "TableMsgs.h"

#pragma	pack(4)

extern fieldDef	ExportTableUserInfoTable_FieldDefs[];
extern U32		cbExportTableUserInfoTable_FieldDefs;


#pragma pack(4)
		
#define	EXPORT_TABLE_USER_INFO_TABLE_NAME		"ExportTableUserInfoTable"
#define	EXPORT_TABLE_USER_INFO_TABLE_VERSION	1

struct ExportTableUserInfoRecord {
	rowID				rid;					// rid of this record
	U32					version;				// version of this record
	U32					size;					// size of this record
	rowID				ridName;				// LUN map name (in a string resource tbl)
	rowID				ridDescription;			// LUN map description  (in a string resource tbl)
};

#define	TFN_ETUI_RIDNAME		"ridName"
#define	TFN_ETUI_RIDDESCRIPTION	"ridDescription"

#endif	// __EXPORT_TABLE_USER_INFO_H__