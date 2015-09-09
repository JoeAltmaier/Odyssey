//SimpleQue.cpp

#include "SimpleQue.h"

int iHack = 0;

SimpleQue::SimpleQue() {
	m_pMessageArray = new Message*[50];
	m_iMessageArraySize = 50;
	m_iMessages = 0;

	char sName[50];
	sprintf(sName, "simple_que_%d",iHack++);
	Kernel::Create_Semaphore(&m_Semaphore, sName, 1);

}

SimpleQue::~SimpleQue() {
	delete[] m_pMessageArray;
}

void SimpleQue::Put(Message* pMsg) {

	Kernel::Obtain_Semaphore(&m_Semaphore, CT_SUSPEND);

	if(m_iMessages >= m_iMessageArraySize) {
		Message** pNew = new Message*[m_iMessageArraySize*2];
		memcpy(pNew, m_pMessageArray, m_iMessageArraySize);
		delete[] m_pMessageArray;
		m_pMessageArray = pNew;
		m_iMessageArraySize *= 2;
	}
	m_pMessageArray[m_iMessages++] = pMsg;

	Kernel::Release_Semaphore(&m_Semaphore);
}

Message* SimpleQue::Peek() {
	Message* pRet = NULL;

	Kernel::Obtain_Semaphore(&m_Semaphore, CT_SUSPEND);

	if(m_iMessages > 0) 
		pRet = m_pMessageArray[0];

	Kernel::Release_Semaphore(&m_Semaphore);

	return pRet;
}

Message* SimpleQue::Remove() {
	Message* pRet = NULL;

	Kernel::Obtain_Semaphore(&m_Semaphore, CT_SUSPEND);

	if(m_iMessages > 0) {
		pRet = m_pMessageArray[0];
		for(int i=1;i<m_iMessages;i++) {
			m_pMessageArray[i-1] = m_pMessageArray[i];
		}
		m_iMessages--;
	}

	Kernel::Release_Semaphore(&m_Semaphore);

	return pRet;
}
