/* DdmCmbProxy.h -- Build IOPStatusTable then do nothing.
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
 * Description:
 *		Build IOPStatusTable then do nothing.  Allows me to work on
 *		VirtualManager on the Eval without having to load DdmBootMstr
 *		or DdmCmb.
 *
**/

// Revision History: 
//  8/20/99 Tom Nelson: Create file


#ifndef __DdmCmbProxy_h
#define __DdmCmbProxy_h

#include "Ddm.h"

//
// DdmCmbProxy Class
//
class DdmCmbProxy : public Ddm {

public:
	static Ddm *Ctor(DID did) 	{ return new DdmCmbProxy(did); }

	DdmCmbProxy(DID did);

	ERC Initialize(Message *_pMsg);
	ERC Enable(Message *_pMsg);

	ERC ProcessIstDefineTableReply(Message *_pReply);
	ERC ProcessIstInsertReply(Message *_pReply);
};

#endif // __DdmCmbProxy_h

