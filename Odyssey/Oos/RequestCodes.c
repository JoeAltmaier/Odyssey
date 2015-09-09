/* RequestCodes.c -- Convert Request code to text name
 *
 * Copyright (C) ConvergeNet Technologies, 1999 
 * Copyright (C) Dell Computer, 2000
 *
 * This material is a confidential trade secret and proprietary 
 * information of ConvergeNet Technologies, Inc. which may not be 
 * reproduced, used, sold or transferred to any third party without the 
 * prior written consent of ConvergeNet Technologies, Inc.  This material 
 * is also copyrighted as an unpublished work under sections 104 and 408 
 * of Title 17 of the United States Code.  Law prohibits unauthorized 
 * use, copying or reproduction.
 *
**/

// Revision History:
//   ?/??/99 Joe Altmaier: Created
// ** Log at end-of-file **

#include "OsTypes.h"

// Define constants for all request codes
#include "RequestCodes.h"
#ifndef WIN32
// Make string tables for all request codes: aStRqName
#undef _RequestCodes_h
#define _REQUEST_NAMES_
#include "RequestCodes.h"

{0,0}};
#endif

extern char *NameRq(REQUESTCODE rqCode);

char *NameRq(REQUESTCODE rqCode) {
#ifndef WIN32
	int i;

	for (i=0; aStRqName[i].rqCode != 0; i++)
		if (aStRqName[i].rqCode == rqCode)
			return aStRqName[i].pName;
#endif
	return "UNKNOWN REQUEST CODE";
	}
	
// $Log: /Gemini/Odyssey/Oos/RequestCodes.c $
// 
// 4     2/06/00 2:26p Tnelson
// Update comments
// 
// 3     9/03/99 2:53p Tnelson
// Add comment header
// 
