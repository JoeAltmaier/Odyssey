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
// This class is the HSCSI initiator device HDM.
// 
// Update Log:
//	$Log: /Gemini/Odyssey/HdmHscsi/HdmHscsi.h $ 
// 
// 1     9/17/99 11:57a Cchan
// Files for HSCSI DDM to support the HSCSI library (QL1040B)
//
/*************************************************************************/

#ifndef __HdmHscsi_h
#define __HdmHscsi_h

#include "Ddm.h"
#include "HscsiConfig.h"


class HdmHscsi: public Ddm {
public:

	HSCSI_CONFIG	config;
	
	// array index of instance
	UNSIGNED	instance;
	
	HdmHscsi(DID did);

	static
	Ddm *Ctor(DID did);

	STATUS Initialize(Message *pMsg);
	STATUS Enable(Message *pMsg);
	STATUS Quiesce(Message *pMsg);
	
protected:

	STATUS DoWork(Message *pMsg);
	
};
#endif