//SimpleQue.h

#ifndef __SimpleQue_H
#define __SimpleQue_H

#include "OsTypes.h"
#include "Message.h"
#include "Ddm.h"
#include "NetMsgs.h"
#include "stdio.h"
#include "string.h"

class SimpleQue {
public:
	SimpleQue();
	~SimpleQue();

	void Put(Message* pMsg);
	Message* Peek();
	Message* Remove();

private:
	Message** m_pMessageArray;
	int m_iMessageArraySize;
	int m_iMessages;
	
	CT_Semaphore m_Semaphore;

};

#endif