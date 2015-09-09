/* RqOsPtsLoader.h -- Request Interface to PtsLoader Ddm.
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
//  7/02/99 Tom Nelson: Create file


#ifndef __RqOsPtsLoader_h
#define __RqOsPtsLoader_h

#include "Ddm.h"
#include "RequestCodes.h"
#include "Message.h"

// Find PTS VDN/DID/ROUTING
//
class RqOsPtsLoaderFindPts : public Message {
public:
	enum { RequestCode = REQ_OS_PTSLOADER_FINDPTS };

	typedef RqOsDdmManagerRouteInstance::Payload Payload;

	Payload payload;
	
	RqOsPtsLoaderFindPts() : Message(RequestCode) {}
};
	

// Find PTS VDN/DID/ROUTING
//
class RqOsPtsProxyLoaderFindPts : public Message {
public:
	enum { RequestCode = REQ_OS_PTSPROXYLOADER_FINDPTS };

	typedef RqOsDdmManagerRouteInstance::Payload Payload;

	Payload payload;
	
	RqOsPtsProxyLoaderFindPts() : Message(RequestCode) {}
};
	

#endif	// __RqOsPtsLoader_h

