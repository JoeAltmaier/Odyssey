/* HeapLink.h -- Heap Link/Signature
 *
 * Copyright (C) ConvergeNet Technologies, 1998 
 *
 * This material is a confidential trade secret and proprietary 
 * information of ConvergeNet Technologies, Inc. which may not be 
 * reproduced, used, sold or transferred to any third party without the 
 * prior written consent of ConvergeNet Technologies, Inc.  This material 
 * is also copyrighted as an unpublished work under sections 104 and 408 
 * of Title 17 of the United States Code.  Law prohibits unauthorized 
 * use, copying or reproduction.
 * 
 * Description:
 *		Base class to hook any type heap into system delete/Free().
 *
 * Revision History:
 *     10/28/98 Tom Nelson: Created
 *
**/

#include "Odyssey_Trace.h"

#ifndef _HeapLink_H
#define _HeapLink_H

#define HEAPSIGN	'Heap'

class HeapLink {
public:
	HeapLink()		{ /*Tracef("HeapLink() this=%lx; *this=%lx\n",this,*(U32 *)this);*/ signature = HEAPSIGN; }

	static BOOL IsValidRamPtr(void * /*pMem*/)	{ return TRUE; };
	virtual U32 Free(void *pMem)=0;
	U32 signature;
};

#endif /* _HeapLink_H */


