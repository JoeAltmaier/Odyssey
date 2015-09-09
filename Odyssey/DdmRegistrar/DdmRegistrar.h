/* DdmRegistrar.h -- Registers Ddms on an IOP with the HBC / PTS.
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
 * Revision History:
 *     02/17/99 Jerry Lane	Created.
 *
**/

#ifndef __DdmRegistrar_H
#define __DdmRegistrar_H

//#include "Nucleus.h"
//#include "Message.h"
#include "Ddm.h"
#include "RqOsSysInfo.h"

class DdmRegistrar: public Ddm {
public:
	// Standard Ddm Method declarations:
					DdmRegistrar(DID did); 
	static 	Ddm*	Ctor(DID did);
	virtual STATUS Initialize(Message *pMsg);
	virtual STATUS Enable(Message *pMsg);
	
	// Our own specialized Ddm Methods:
			// These are standard Message Handlers:
			STATUS VerifyDdmCDTExists(Message *pMsg);
			STATUS RegisterDdmClasses(void *pClientContext, STATUS status);
			STATUS RegisterClassEntry(RqOsSiGetClassTable *pMsg);
			
			// These are PTS callback handlers:
			STATUS HandleInsertRowReply(void *pClientContext, STATUS status);
};


#endif	// __DdmRegistrar_H
