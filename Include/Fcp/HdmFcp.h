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
// This class is the base class for the Fcp Target and Target/Init
// device HDM.
// 
// Update Log:
//	$Log: /Gemini/Include/Fcp/HdmFcp.h $
// 
// 2     8/20/99 7:53p Mpanas
// Changes to support Export Table states
// 
// 1     7/15/99 4:17p Mpanas
// New Fcp Base class
//
// 07/06/99 Michael G. Panas: Create file
/*************************************************************************/

#ifndef __HdmFcp_h
#define __HdmFcp_h

#include "Ddm.h"


class HdmFcp: public Ddm {
public:

	virtual VDN FCP_Find_Next_Virtual_Device(void *pCtx, U32 key) = 0;
	
	HdmFcp(DID did) : Ddm( did ){}

};
#endif