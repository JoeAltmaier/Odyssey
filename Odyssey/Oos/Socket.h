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
// This class abstracts network service.
// 
// Update Log: 
// 8/16/99 Joe Altmaier: Create file
/*************************************************************************/

#ifndef __Socket_h
#define __Socket_h

#include "OsTypes.h"
#include "Dma.h"

#define PLUS
#include "Externs.h"

class Socket {
	addr_struct addrMe;
	addr_struct addrHim;
	
	int socketDescriptor; // >= 0 if socket open

public:
	Socket();
	
	void Configure(U32 ipMe, U32 ipHim, U32 port);
	BOOL IsOpen() { return (socketDescriptor >= 0); }
	void Wait();

	// Open socket, as bind/listen or connect
	Status Connect(BOOL fHost);
	Status Close();
	
	// Transmit
	Status Write(void *pData, U32 cbData);
	// Block on receive
	Status Read(void *pData, U32 cbRead, U32 *pCbRead);
};


#endif