/* DdmSysInfo.h -- System Info Ddm
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
 *		Ddm that processes Get System Info requests
 *
**/

// Revision History: 
// 	 3/27/99 Tom Nelson: Created
// ** Log at end of file **

// 100 columns
//34567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890

#ifndef __DdmSysInfo_H
#define __DdmSysInfo_H

#include "RqOsSysInfo.h"
#include "Ddm.h"

class DdmSysInfo: public Ddm {
	typedef RqOsSysInfoGetDidActivity::DidActivity DidActivity;
	
public:
	DdmSysInfo(DID did);
	static Ddm *Ctor(DID did) 	{ return new DdmSysInfo(did); }

	ERC ProcessGetClassTable(Message *pArgMsg);
	ERC ProcessGetDidActivity(Message *_pRequest);
};

#endif	// __DdmSysInfo_H

//**************************************************************************************************
// Update Log:
//	$Log: /Gemini/Odyssey/Oos/DdmSysInfo.h $
// 
// 3     12/16/99 3:40p Iowa
// System status support.
// 

