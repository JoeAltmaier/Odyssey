/* RqOsStatus.h -- Request Interface to DdmStatus.
 *
 * Copyright (C) ConvergeNet Technologies, 1999
 *
 * This material is a confidential trade secret and proprietary 
 * information of ConvergeNet Technologies, Inc. which may not be 
 * reproduced, used, sold or transferred to any third party without the 
 * prior written consent of ConvergeNet Technologies, Inc.  This material 
 * is also copyrighted as an unpublished work under sections 104 and 408 
 * of Title 17 of the United States Code.  Law prohibits unauthorized 
 * use, copying or reproduction.
 *
**/

// Revision History: 
//  12/08/99 Tom Nelson: Create file


#ifndef __RqOsStatus_h
#define __RqOsStatus_h

#include "RequestCodes.h"
#include "Message.h"

// Dump PTS IOPStatusTable
//
class RqOsStatusDumpIst : public Message {
public:
	enum { RequestCode = REQ_OS_STATUS_DUMPIST };

	RqOsStatusDumpIst() 
	: Message(RequestCode) {}

	// Proceedural Interface
	static ERC Invoke();
};

// Dump PTS VirtualDeviceTable
//
class RqOsStatusDumpVdt : public Message {
public:
	enum { RequestCode = REQ_OS_STATUS_DUMPVDT };

	RqOsStatusDumpVdt() 
	: Message(RequestCode) {}

	// Proceedural Interface
	static ERC Invoke();
};

// Dump Did Activity
//
class RqOsStatusDumpDidActivity : public Message {
public:
	enum { RequestCode = REQ_OS_STATUS_DUMPDIDACTIVITY };

	RqOsStatusDumpDidActivity() 
	: Message(RequestCode) {}

	// Proceedural Interface
	static ERC Invoke();
};

// Dump All Did Activity
//
class RqOsStatusDumpAllDidActivity : public Message {
public:
	enum { RequestCode = REQ_OS_STATUS_DUMPALLDIDACTIVITY };

	RqOsStatusDumpAllDidActivity() 
	: Message(RequestCode) {}

	// Proceedural Interface
	static ERC Invoke();
};


#endif	// __RqOsStatus_h

