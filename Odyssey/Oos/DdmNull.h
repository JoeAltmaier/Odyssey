/* DdmNull.h -- Do nothing Ddm.
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
//  8/20/99 Tom Nelson: Create file


#ifndef __DdmNull_h
#define __DdmNull_h

#include "Ddm.h"

//
// DdmNull Class
//
class DdmNull : public Ddm {

public:
	static Ddm *Ctor(DID did) 	{ return new DdmNull(did); }

	DdmNull(DID did);

	ERC Initialize(Message *pArgMsg);

	ERC Enable(Message *pArgMsg);
};

#endif // __DdmNull_h

