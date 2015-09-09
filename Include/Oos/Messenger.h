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
// This class implements message primitives and the message transport.
// 
// Update Log: 
// 6/12/98 Joe Altmaier: Create file
// 9/29/98 Jim Frandeen: Use CtTypes.h instead of stddef.h
// 10/12/98 Joe Altmaier: Remove transport to Transport.h/cpp
/*************************************************************************/

#ifndef __Messenger_h
#define __Messenger_h

#include "OsTypes.h"
#include "Message.h"
#include "DeviceId.h"


class Messenger {

public:
	Messenger();

	static
	STATUS Send(Message *pMsg, DID didTarget);

	static
	STATUS Reply(Message *, BOOL fLast=true); // didReply returns message to message Initiator

	static
	STATUS Signal(U16 nSignal,void *pPayload, DID didTarget);
};
#endif


