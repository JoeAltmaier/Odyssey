/* RqOsTransport.h -- Request Interface to Transport.
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
**/

// Revision History: 
//  6/18/99 Joe Altmaier: Create file

#ifndef __RqOsTransport_h
#define __RqOsTransport_h

#include "Ddm.h"
#include "RequestCodes.h"
#include "Message.h"

class RqOsTransportVdnStop : public Message {
public:
	enum { requestcode = REQ_OS_TRANSPORT_VDN_STOP };

	VDN vdnStop;

	RqOsTransportVdnStop(VDN _vdnStop) : Message(RqOsTransportVdnStop::requestcode), vdnStop(_vdnStop) {
	}
};
	
class RqOsTransportIopStop : public Message {
public:
	enum { requestcode = REQ_OS_TRANSPORT_IOP_STOP };

	TySlot tySlot;

	RqOsTransportIopStop(TySlot _tySlot) : Message(RqOsTransportIopStop::requestcode), tySlot(_tySlot) {
	}
};

class RqOsTransportIopStart : public Message {
public:
	enum { requestcode = REQ_OS_TRANSPORT_IOP_START };

	TySlot tySlot;

	RqOsTransportIopStart(TySlot _tySlot) : Message(RqOsTransportIopStart::requestcode), tySlot(_tySlot) {
	}
};

class RqOsTransportHbcMaster : public Message {
public:
	enum { requestcode = REQ_OS_TRANSPORT_HBC_MASTER };

	TySlot tySlot;

	RqOsTransportHbcMaster(TySlot _tySlot) : Message(RqOsTransportHbcMaster::requestcode), tySlot(_tySlot) {
	}
};

class RqOsTransportTerminate : public Message {
public:
	enum { requestcode = REQ_OS_TRANSPORT_TERMINATE };

	RqOsTransportTerminate() : Message(RqOsTransportTerminate::requestcode) {
	}
};

#endif	// __RqOsTransport_h
