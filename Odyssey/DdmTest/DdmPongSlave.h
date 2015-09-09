/* DdmPongSlave.h -- Test Timer DDM
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
//  3/27/99 Tom Nelson: Created
//


#ifndef __DdmPongSlave_H
#define __DdmPongSlave_H

#include "OsTypes.h"
#include "Message.h"
#include "Ddm.h"

#include "RqOsVirtualMaster.h"

class DdmPongSlave: public Ddm {
public:
	struct {VDN vdn; } config;

	DdmPongSlave(DID did);
	static Ddm *Ctor(DID did) { return new DdmPongSlave(did); }

	ERC Enable(Message *pMsg);
	ERC Quiesce(Message *pMsg);
	
	ERC ProcessPong(Message *pMsg);
};
#endif	// __DdmPongSlave_H

