/* SerialDdm.h -- Serial DDM
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
// $Log: /Gemini/Odyssey/DdmPnP/SerialDdm.h $
// 
// 10    9/07/99 12:13p Jlane
// Add DBGPORT define and also use ttyD instead of ttyC.
// 
// 9     8/31/99 6:04p Hdo
// Add #define to support Eval and Gemini

 * Revision History:
 *		05/25/99 Huy Do: change from NU_Queue to QueueLi
 *		05/24/99 Huy Do: change from QueueLi to NU_Queue and remove
 *						 CreateTask
 *		05/07/99 Huy Do: add QueueLi
 *		05/03/99 Huy Do: implement PortReader (State Table Pattern)
**/

#ifndef __SerialDdm_H__
#define __SerialDdm_H__

#define	TTYRX				2
#define	FIOCINT				1

// These functions are interrupt driven
extern "C" int	ttyin(int);
extern "C" int	ttyout(int port, int data);
extern "C" int  ttyhit(int);

extern "C" void ttyioctl(int, int, int);
extern "C" void ttyinit(int, int);

#ifdef INCLUDE_ODYSSEY	/* Odyssey-native UART hardware (16550-compatible) */

extern	"C" STATUS ttyD_out(U32 data);

#define COMPORT			3
#define DBGPORT			1
#define BAUDRATE		115200
#define WRITEPORT(data)	ttyD_out(data)

#else INCLUDE_ODYSSEY	/* Eval Board UART */

extern	"C" STATUS ttyA_out(U32 data);

#define COMPORT			0
#define DBGPORT			1
#define BAUDRATE		57600
#define WRITEPORT(data)	ttyA_out(data)

#endif INCLUDE_ODYSSEY

#include "Ddm.h"
#include "PnPMsg.h"

typedef enum {
	IDLE_STATE,
	IN_FRAME_STATE
} ReceiveState;

class SerialDdm: public Ddm {
protected:
	ReceiveState m_State;
	CT_Task ctTask;
	void	*pStack;

	rowID *m_pReturnedRID;

	STATUS ProcessPacket(char* pDataBlock);
	STATUS SendToPort(void* pData, U32 uSize);
	STATUS DoWork(Message *pMsg);

public:
	SerialDdm(DID did);
	~SerialDdm();

	virtual STATUS Initialize(Message *pMsg);
	virtual STATUS Enable(Message *pMsg);

	static Ddm *Ctor(DID did);
	void PortReader();
};

#endif	// __SerialDdm_H_
