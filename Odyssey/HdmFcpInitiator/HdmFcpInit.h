/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// Description:
// This class is the Fcp Initiator device HDM.
// 
// Update Log:
//	$Log: /Gemini/Odyssey/HdmFcpInitiator/HdmFcpInit.h $
// 
// 2     2/03/00 7:18p Jlane
// Add FinishInitialize in support of Drive Removal logic to use DDHs only
// if present.
// 
// 1     7/15/99 11:49p Mpanas
// Changes to support Multiple FC Instances
// and support for NAC
// -New Initiator DDM project
//
// 07/02/99 Michael G. Panas: Create file from HdmFcpRac.h
/*************************************************************************/

#ifndef __HdmFcpInit_h
#define __HdmFcpInit_h

#include "Ddm.h"
#include "FcpConfig.h"


class HdmFcpInit: public Ddm {
public:

	FCP_CONFIG	config;
	
	// array index of instance
	UNSIGNED	instance;
	
	// saved enable message cookie
	Message		*pEnableMsg;
	
	HdmFcpInit(DID did);

	static
	Ddm *Ctor(DID did);

	STATUS Initialize(Message *pMsg);
	STATUS FinishInitialize(Message *pMsg);
	STATUS Enable(Message *pMsg);
	STATUS Quiesce(Message *pMsg);
	
protected:

	STATUS DoWork(Message *pMsg);
	
};
#endif