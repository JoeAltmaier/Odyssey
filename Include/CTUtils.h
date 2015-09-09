/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: CTUtils.h
// 
// Description:
// This module defines some helpful macros and other utilities.
// 
// $Log: /Gemini/Include/CTUtils.h $
// 
// 1     9/07/99 9:12a Jlane
// Initial Checkin.
// 
/*************************************************************************/

#if !defined(CTUtils_H)
#define CTUtils_H

#ifndef CTtypes_H
#include "CTTypes.h"
#endif


// Some useful pointer handling macros:

// FreeAndClear(p) - Deletes the pointer p and sets it to NULL.
#define FreeAndClear(p)			\
{								\
	delete (p);					\
	(p) = NULL;					\
}								\

// CheckFreeAndClear(p) - Invokes FreeAndClear(p) only if p is not NULL.
#define CheckFreeAndClear(p) 	\
{								\
if (p) 							\
	FreeAndClear(p)				\
}								\

#endif // CTUtils_H

