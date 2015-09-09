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
* UnicodeString32Table.h
*
* Description:
*
* This the a string resource table for strings with 15 or less Unicode
* characters. The table is global to the system and may be used by all 
* parties that need to store string resources.
*
*************************************************************************/

#ifndef __UNICODE_STRING_32_TABLE_
#define __UNICODE_STRING_32_TABLE_

#include "CtTypes.h"
#include "TableMsgs.h"

#pragma	pack(4)

extern fieldDef	UnicodeString32Table_FieldDefs[];
extern U32		cbUnicodeString32Table_FieldDefs;


#pragma pack(4)
		
#define	UNICODE_STRING_32TABLE_NAME		"UnicodeString32Table"
#define	UNICODE_STRING_32TABLE_VERSION	1

struct UnicodeString32Record {
	rowID				rid;					// rid of this record
	U32					version;				// version of this record
	U32					size;					// size of this record
	UnicodeString32		string;					// resource field
};

#endif	// __UNICODE_STRING_32_TABLE_