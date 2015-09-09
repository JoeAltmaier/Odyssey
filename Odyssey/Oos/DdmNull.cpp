/* DdmNull.cpp -- Do Nothing Ddm.
 *
 * Copyright (C) ConvergeNet Technologies, 1999
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
//  8/17/99 Tom Nelson: Create file
// ** Log at end of file **

// 100 columns
//34567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890

#define _TRACEF
#define	TRACE_INDEX		TRACE_NULL
#include "Odyssey_Trace.h"

// Public Includes
#include <String.h>
#include "BuildSys.h"

// Private Includes
#include "DdmNull.h"

// BuildSys Linkage

CLASSNAME(DdmNull,MULTIPLE);


// .DdmNull -- Constructor -----------------------------------------------------------------DdmNull-
//
DdmNull::DdmNull(DID did) : Ddm(did) {
	TRACE_PROC(DdmNull::DdmNull);

}

// .Initialize -- Process Initialize -------------------------------------------------------DdmNull-
//
ERC DdmNull::Initialize(Message *pArgMsg) {
	TRACE_PROC(DdmNull::Initialize);
	
	Reply(pArgMsg,OK);	// Initialize Complete
			
	return OK;
}
	
// .Enable -- Process Enable ---------------------------------------------------------------DdmNull-
//
ERC DdmNull::Enable(Message *pArgMsg) {
	TRACE_PROC(DdmNull::Enable);

	Reply(pArgMsg,OK);
	return OK;
}


//**************************************************************************************************
// Update Log:
//	$Log: /Gemini/Odyssey/Oos/DdmNull.cpp $
// 
// 3     12/09/99 2:06a Iowa
// 
// 1     9/16/99 3:17p Tnelson
// Support for PTS
