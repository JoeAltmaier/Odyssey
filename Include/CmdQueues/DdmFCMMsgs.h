/* DdmFCMMsgs.h -- Define Ddm FCM message structure
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
// $Log: /Gemini/Include/CmdQueues/DdmFCMMsgs.h $
// 
// 1     1/05/00 5:15p Dpatel
// Initial creation.
// 
//
*/

#ifndef _DdmFCMMsgs_h
#define _DdmFCMMsgs_h


#include "Message.h"
#include "RequestCodes.h"
#include "CTEvent.h"

#include "DdmFCMCmnds.h"

class MsgFCMLoopControl : public Message
{
public:
	
	// Ctor: add the SGL for the vc request	
	MsgFCMLoopControl(const FCM_CMND_INFO *_pCmdInfo)
		: Message(REQ_FCM_LOOP_CONTROL)
	{
		AddSgl(1, (void *)_pCmdInfo, sizeof(FCM_CMND_INFO)); 
	}

	void GetFCMCmdInfo(void **ppCmdInfo)
	{
		U32		sizeofCmdInfo;
		GetSgl(1, ppCmdInfo, &sizeofCmdInfo);
	}
};



class MsgFCMNacShutdown : public Message
{
public:
	
	// Ctor: add the SGL for the vc request	
	MsgFCMNacShutdown(const FCM_CMND_INFO *_pCmdInfo)
		: Message(REQ_FCM_NAC_SHUTDOWN)
	{
		AddSgl(1, (void *)_pCmdInfo, sizeof(FCM_CMND_INFO)); 
	}

	void GetFCMCmdInfo(void **ppCmdInfo)
	{
		U32		sizeofCmdInfo;
		GetSgl(1, ppCmdInfo, &sizeofCmdInfo);
	}
};



class MsgFCMGetWWN : public Message
{
public:
	
	// Ctor: add the SGL for the vc request	
	MsgFCMGetWWN(const FCM_CMND_INFO *_pCmdInfo)
		: Message(REQ_FCM_GET_WWN)
	{
		AddSgl(1, (void *)_pCmdInfo, sizeof(FCM_CMND_INFO)); 
	}

	void GetFCMCmdInfo(void **ppCmdInfo)
	{
		U32		sizeofCmdInfo;
		GetSgl(1, ppCmdInfo, &sizeofCmdInfo);
	}
};



class MsgFCMGetChassisWWN : public Message
{
public:
	
	// Ctor: add the SGL for the vc request	
	MsgFCMGetChassisWWN(const FCM_CMND_INFO *_pCmdInfo)
		: Message(REQ_FCM_GET_CHASSIS_WWN)
	{
		AddSgl(1, (void *)_pCmdInfo, sizeof(FCM_CMND_INFO)); 
	}

	void GetFCMCmdInfo(void **ppCmdInfo)
	{
		U32		sizeofCmdInfo;
		GetSgl(1, ppCmdInfo, &sizeofCmdInfo);
	}
};

#endif //_Message_h
