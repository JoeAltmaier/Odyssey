/* DdmVCMMsgs.h -- Define Ddm CMB message structure
 *
 * Copyright (C) ConvergeNet Technologies, 1998,99 
 *
 * This material is a confidential trade secret and proprietary 
 * information of ConvergeNet Technologies, Inc. which may not be 
 * reproduced, used, sold or transferred to any third party without the 
 * prior written consent of ConvergeNet Technologies, Inc.  This material 
 * is also copyrighted as an unpublished work under sections 104 and 408 
 * of Title 17 of the United States Code.  Law prohibits unauthorized 
 * use, copying or reproduction.
 *
//
// $Log: /Gemini/Odyssey/DdmVCM/DdmVCMMsgs.h $
// 
// 8     12/17/99 5:31p Dpatel
// added vc modify message
// 
//
*/

#ifndef _DdmVCMMsgs_h
#define _DdmVCMMsgs_h


#include "Message.h"
#include "RequestCodes.h"
#include "CTEvent.h"

#include "DdmVCMCommands.h"

class MsgVCMModifyVC : public Message
{
public:
	
	// Ctor: add the SGL for the vc request	
	MsgVCMModifyVC(const VCRequest *_pVCRequest)
		: Message(REQ_MODIFY_VC)
	{
		AddSgl(1, (void *)_pVCRequest, sizeof(VCRequest)); 
	}

	void GetVCRequest(void **ppVCRequest)
	{
		U32		sizeofVCRequest;
		GetSgl(1, ppVCRequest, &sizeofVCRequest);
	}
};




#endif //_Message_h
