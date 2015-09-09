/* Message.h -- Implement message accessor methods
 *
 * Copyright (C) ConvergeNet Technologies, 1998,99 
 * Copyright (C) Dell Computer, 2000
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
// 7/21/98 Joe Altmaier: Create file
//  ** Log at end-of-file **

// 100 columns ruler
//34567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890

#define _TRACEF
#include "Odyssey_Trace.h"
#define TRACE_INDEX TRACE_MESSENGER1

#include <StdLib.h>
#include <String.h>

#include "CtEvent.h"
#include "Galileo.h"
#include "Address.h"
#include "Message.h"
#include "DdmOsServices.h"
#include "RequestCodes.h"

extern "C" void bcopy(void* pFrom, void* pTo, U32 cb);
extern "C" char *NameRq(REQUESTCODE rqCode);

struct {MSGFLAGS flags; char *name;} aFlags[]={
	{0x1, " [UNKNOWN FLAGS:0001]"},
	{MESSAGE_FLAGS_DEAD, " DEAD"},
	{MESSAGE_FLAGS_AUTO_RETRY, " FAILSAFE"},
	{MESSAGE_FLAGS_TIMESTAMP, " TIMESTAMP"},
	{MESSAGE_FLAGS_NOFAULTIN, " NOFAULTIN"},
	{MESSAGE_FLAGS_FAIL, " FAIL"},
	{MESSAGE_FLAGS_LAST, " LAST"},
	{MESSAGE_FLAGS_REPLY, " REPLY"},
	{MESSAGE_FLAGS_INQUEUE, " INQUEUE"},
	{MESSAGE_FLAGS_FILTER, " FILTER"},
	{MESSAGE_FLAGS_OPEN, " OPEN"},
	{MESSAGE_FLAGS_FILLED, " FILLED"},
	{0x800, " [UNKNOWN FLAGS:800]"},
	{0x1000, " [UNKNOWN FLAGS:1000]"},
	{0x2000, " [UNKNOWN FLAGS:2000]"},
	{0x4000, " [UNKNOWN FLAGS:4000]"},
	{MESSAGE_FLAGS_FILTER, " FILTER"},
	{0, ""}
	};


void Message::Construct(REQUESTCODE _msgCode, U32 _sHeader, MSGFLAGS _flags) {

TRACEF(TRACE_L8, ("Message %lx created\r\n", this));
	
	flags = _flags | MESSAGE_FLAGS_LAST | MESSAGE_FLAGS_AUTO_RETRY;
	
	sHeader = (unsigned short)((_sHeader + 3) / 4 * 4);
	sMessage = sHeader / 4;
TRACEF(TRACE_L8, ("sMessage: %lu\n",sHeader));

	oSgl = 0;	// Zero indicates no SGL
	reqCode = _msgCode;
	DetailedStatusCode = OK;
	pInitiatorContext = NULL;
	didInitiator = DIDNULL;
	didTarget = DIDNULL;
	refnum=MakeRefNum();
#ifdef DEBUG
	pDeleted=NULL;
#endif
}


// .Message -- Constructor ---------------------------------------------Message-
//
Message::Message(REQUESTCODE _msgCode) {
	Construct(_msgCode, sHeader, 0);
	}	

// .Message -- Constructor ---------------------------------------------Message-
//
Message::Message(REQUESTCODE _msgCode, U32 _sPayload) {
	Construct(_msgCode, _sPayload + sizeof(Message), 0);
	}

// .Message -- Constructor ---------------------------------------------Message-
//
Message::Message(REQUESTCODE _msgCode, U32 _sPayload, MSGFLAGS _flags) {
	Construct(_msgCode, _sPayload + sizeof(Message), _flags);
	}

//Tracef("Message %lx created\r\n", this);
// .~Message -- Destructor ---------------------------------------------Message-
//
Message::~Message() {
//Tracef("Message %lx deleted\r\n", this);
	if (IsLast()) {
		InitiatorContext *pBind = PopIc();
		while (pBind) {
			delete pBind;
			pBind = PopIc();
		}
	}

	CleanSgl();
#ifdef DEBUG
	if (pDeleted)
		*pDeleted=true;
#endif
}

// .Message -- Copy Constructor ----------------------------------------Message-
//
Message::Message(Message *pMsg) {
	*this=*pMsg;
	flags &= ~MESSAGE_FLAGS_LAST;

	// Clone sgl
	// Number of total fragments in message
	U32 nSse = NSSE;
	SGE_SIMPLE_ELEMENT *pSse = PSSE;

	// Temporary buffers get assigned to original or copy message
	for (U32 iSse=0; iSse < nSse; iSse++)
		// SEND temporary buffers belong to the original message, clear FREE flag in copy
		if ((pSse[iSse].flags & SGL_FLAGS_DIR) == SGL_FLAGS_SEND)
			pSse[iSse].flags &= ~SGL_FLAGS_FREE_ON_DTOR;
			
		// REPLY temporary buffers belong to the copy message, free from original
		else if ((pSse[iSse].flags & SGL_FLAGS_DIR) == SGL_FLAGS_REPLY) {
			((SGE_SIMPLE_ELEMENT*)(((long*)pMsg) + oSgl))[iSse].flags &= ~SGL_FLAGS_FREE_ON_DTOR;
			// Allocate new buffer in the original message if any was already allocated
			((SGE_SIMPLE_ELEMENT*)(((long*)pMsg) + oSgl))[iSse].address = NULL;
		}
}

// .Clone -- Copy self --------------------------------------------------Message-
//
Message *Message::Clone() {
	return new Message(this);
}

// .Migrate -- Reallocate in cheaper memory -----------------------------Message-
//
Message *Message::Migrate() {
	if ((flags & MESSAGE_FLAGS_OPEN) && IS_PCI(PA_P(this))) {
		Message *pRet=new Message(this);
		// Don't clean anything up on delete.
		delete (void*)this;
		return pRet;
	}
	return this;
}

// .CleanSgl -- Destroy sgl ---------------------------------------------Message-
//
Message *Message::CleanSgl() {
	// Number of total fragments in message
	U32 nSse = NSSE;
	SGE_SIMPLE_ELEMENT *pSse = PSSE;

	for (U32 iSse=0; iSse < nSse; iSse++)
		SglDtor(pSse[iSse]);
	
	oSgl=0;
	sMessage = sHeader / sizeof(U32);
	flags &= ~MESSAGE_FLAGS_FILLED;
	return this;
}

// .CleanAllSgl -- Delete client sgl -Must be explicitly called-------------Message-
//
Message *Message::CleanAllSgl() {
	// Number of total fragments in message
	U32 nSse = NSSE;
	SGE_SIMPLE_ELEMENT *pSse = PSSE;

	for (U32 iSse=0; iSse < nSse; iSse++)
	{
		if (pSse[iSse].IsClients() && pSse[iSse].address) {
			// Only delete last SEND sgls - they are shared by all copies
			// Delete REPLY sgls on any copy - the client REPLY sgl is actually returned
			// in the 1st reply, regardless of IsLast
			if (!pSse[iSse].IsSend() || IsLast()) {
				delete (void*)pSse[iSse].address;
				pSse[iSse].address=NULL;
			}
		}
	}

	CleanSgl();

	return this;
}

// .CopyReply -- Copy reply fields over original message ----------------Message-
//
Message *Message::CopyReply(Message *pOriginalMessage) {
	pOriginalMessage->DetailedStatusCode = DetailedStatusCode;
	pOriginalMessage->flags=flags;

	pOriginalMessage->AddPayload(GetPPayload(), GetSPayload());
	
	// Number of total fragments in message
	U32 nSse = NSSE;
	SGE_SIMPLE_ELEMENT *pSse = PSSE;
	SGE_SIMPLE_ELEMENT *pSseOrig = PSSE_(pOriginalMessage);

	for (U32 iSse=0; iSse < nSse; iSse++)
		// Preserve SEND sgl
		if ((pSse[iSse].flags & SGL_FLAGS_DIR) == SGL_FLAGS_REPLY)
			pSseOrig[iSse]=pSse[iSse];

	return this;
}


// .KeepReply -- Copy original sgl over reply message --------------------Message-
//
Message *Message::KeepReply(Message *pOriginalMessage) {
	// Number of total fragments in message
	U32 nSse = NSSE;
	SGE_SIMPLE_ELEMENT *pSse = PSSE;
	SGE_SIMPLE_ELEMENT *pSseOrig = PSSE_(pOriginalMessage);

	for (U32 iSse=0; iSse < nSse; iSse++)
		if ((pSse[iSse].flags & SGL_FLAGS_DIR) == SGL_FLAGS_SEND)
			pSse[iSse]=pSseOrig[iSse];
		else
			pSse[iSse].address=pSseOrig[iSse].address;

	return this;
}


// .MakeReply -- Fill out reply from prototype (pMsg) --------------------Message-
//
Message *Message::MakeReply(Message *pMsg) {

	// Put prototype data into reply
	this->AddPayload(pMsg->GetPPayload(), pMsg->GetSPayload());

	U32 nSgl = this->GetCSgl();  // also allocates reply sgl buffers

	// Examine all prototype sse
	U32 nSse = NSSE;
	SGE_SIMPLE_ELEMENT *pSse = PSSE_(pMsg);
	// Track which sgl entry each sse is in
	U32 iSgl=0;
	U32 oData=0;
	
	// Copy all prototype reply fragments to appropriate offset in pReply sgl
	for (U32 iSse=0; iSse < nSse; iSse++) {
		if ((pSse[iSse].flags & SGL_FLAGS_DIR) == SGL_FLAGS_REPLY)
			this->CopyToSgl(iSgl, oData, P_SGL(pSse[iSse]), pSse[iSse].count);

		oData += pSse[iSse].count;
		if (pSse[iSse].flags & SGL_FLAGS_END_OF_BUFFER) {
			iSgl++;
			oData=0;
		}
	}

	return this;
}

// .PopIc -- Pop Initiator Context --------------------------------------Message-
//
InitiatorContext *Message::PopIc() {
	
	InitiatorContext *pIc = pInitiatorContext;
	if (pIc != NULL)
		pInitiatorContext = pIc->pIcNext;

	return pIc;
}
	
// .GetContext -- Return message context -------------------------------Message-
//
void* Message::GetContext(long signature) {
	
	if (!signature || contextTransaction.HighPart == (unsigned)signature)
		return (void*)contextTransaction.LowPart;
			
	return NULL;
}

// .AddPayload -- Add payload to message -------------------------------Message-
//
// NOTICE: sHeader + SGLs <= MESSAGESIZE
//
void Message::AddPayload(void *pPayload, U32 sPayload) {
TRACEF(TRACE_L8, ("Message %lx AddPayload\r\n", this));
	U8 *pPayloadMsg = ((U8 *)this) + sizeof(Message);
	
	if (sPayload > sHeader - sizeof(Message)) {
		Tracef("[WARNING] Message::AddPayload exceeds maximum message size!\n");
		sPayload = sHeader - sizeof(Message);
	}

	if (pPayload)
		bcopy(pPayload, pPayloadMsg, sPayload);
}


// .GetPPayload -- Get Pointer to payload ------------------------------Message-
//
void* Message::GetPPayload() {

	return (void*)( ((U8*)this) + sizeof(Message) );
}
	
// .GetSPayload -- Get size of payload ---------------------------------Message-
//
U32 Message::GetSPayload() {

	return sHeader - sizeof(Message);
}
		
// .GetCSgl -- Return number of elements-------------------------------Message-
//
// The Message SGL looks like:
//		[<SGE_SIMPLE_ELEMENT> [<SGE_SIMPLE_ELEMENT>]]	// 1st Element
//		[<SGE_SIMPLE_ELEMENT> [<SGE_SIMPLE_ELEMENT>]]	// 2nd Element
//				...						...
//		[<SGE_SIMPLE_ELEMENT> [<SGE_SIMPLE_ELEMENT>]]	// Last Element
//
// Each Element in the SGL consists of one or more SGE_SIMPLE_ELEMENT fragments.
//
// Where the last fragment of each element is flaged SGL_FLAGS_END_OF_BUFFER and
// the last fragment of the last element is also flaged SGL_FLAGS_LAST_ELEMENT.
//
// NOTICE: Message + Payload + SGLs <= MESSAGESIZE
//
// NOTICE: sMessage = (sHeader + SGLs) / 4
//		   oSgl     = (sHeader) / 4
//
U32 Message::GetCSgl() {
	FillSgl();
	
	U32 nSse;
	SGE_SIMPLE_ELEMENT *pSse;
	U32 nBuf;
	int iSse = ISse(0, nSse, nBuf, pSse);
	if (iSse < 0)
		return 0;
		
	// Count buffers by counting End-Of-Buffer elements
	// Count Elements
	for (; iSse < nSse; nBuf++) {
		// Count fragments in this element
		while (iSse < nSse && !(pSse[iSse].flags & (SGL_FLAGS_END_OF_BUFFER | SGL_FLAGS_LAST_ELEMENT) ))
			iSse++;

		iSse++;
	}

	return nBuf;
}

// .GetCFragment -- Return number of fragments in an element-----------Message-
//
U32 Message::GetCFragment(U32 iSgl) {
	FillSgl();
	
	U32 nSse;
	U32 nSgl;
	SGE_SIMPLE_ELEMENT *pSse;

	int iSse = ISse(iSgl, nSse, nSgl, pSse);

	if (iSse < 0 || iSse >= nSse)
		return 0;

	// ASSERT iSse points at the 1st fragment of iSgl,
			
	// Find end of fragment list for iSgl
	// iSse points to 1st (maybe only) fragment for iSgl
	U32 nFragment = 1;
	while (iSse < nSse-1 && !(pSse[iSse].flags & SGL_FLAGS_END_OF_BUFFER )) {
		iSse++;
		nFragment++;
	}
	return nFragment;
}

// .GetSSgl -- Return size of all fragments in an element------------------Message-
//
U32 Message::GetSSgl(U32 iSgl) {
	FillSgl();
	
	U32 nSse;
	U32 nSgl;
	SGE_SIMPLE_ELEMENT *pSse;

	int iSse = ISse(iSgl, nSse, nSgl, pSse);

	if (iSse < 0 || iSse >= nSse)
		return 0;

	// ASSERT iSse points at the 1st fragment of iSgl
			
	// Find end of fragment list for iSgl
	// iSse points to 1st (maybe only) fragment for iSgl
	U32 sFragments = 0;
	while (iSse < nSse) {
		sFragments += pSse[iSse].count;
		if (pSse[iSse].flags & SGL_FLAGS_END_OF_BUFFER)
			break;
		iSse++;
	} 
	return sFragments;
}

// .ISse -- Find first fragment of specified element -------------------Message-
//
// Points one past end of SGL (last fragment) if not found.
// Returns -1 if no SGL
//
int Message::ISse(U32 iSgl, U32 &nSse, U32 &nSgl, SGE_SIMPLE_ELEMENT *&pSse) {
	
	if (oSgl == 0) {
		nSse = 0;
		nSgl = 0;
		pSse = (SGE_SIMPLE_ELEMENT*)(((long*)this) + sMessage);
		return -1;
	}
		
	nSse = NSSE;
	pSse = PSSE;
	int iSse = 0;
	
	// Find 1st fragment of iSgl (requested element)
	for (nSgl=0; nSgl < iSgl && iSse < nSse; nSgl++) {
		// Find end of this SGL element
		while (iSse < nSse-1 && !(pSse[iSse].flags & (SGL_FLAGS_END_OF_BUFFER | SGL_FLAGS_LAST_ELEMENT) ))
			iSse++;

		iSse++;
	}
	return iSse;
}

// .AddSgl -- Add a fragment to specified (iSgl) element ---------------Message-
//
void Message::AddSgl(U32 iSgl, SGE_SIMPLE_ELEMENT &sse) {
	
	void *pData;
	
	pData=P_SGL(sse);

	AddSgl(iSgl, pData, sse.count, (sse.flags & SGL_FLAGS_DIR)? SGL_SEND : SGL_REPLY );
}
	
// .AddSgl -- Add a fragment to specified SGL element ------------------Message-
//
void Message::AddSgl(U32 iSgl, void *pData, U32 sData, int direction) {
	U32 nSse;
	U32 nSgl;
	SGE_SIMPLE_ELEMENT *pSse;

	// After a reply, clear sgl and start over
	if (IsReply()) {
		CleanSgl();
		flags &= ~MESSAGE_FLAGS_REPLY;
#ifdef DEBUG
		if (pDeleted)
			*pDeleted=true;
#endif
	}

TRACEF(TRACE_L8, ("AddSgl 0x%08lx sHeader=%u oSgl=%u sMessage=%u rqCode=%08lx\n", this, sHeader, oSgl, sMessage, reqCode));

	int iSse = ISse(iSgl, nSse, nSgl, pSse);	// All args returned.

	if (iSse < 0) {// No SGL 
		iSse=0;
		oSgl = sMessage;
	}

//Tracef("AddSgl iSse=%d nSse=%d\r\n", iSse, nSse);
		
	// ASSERT iSse points at the 1st fragment of iSgl,
	// or one after the end of the list (iSse == nSse).

	// Must extend list?
	if (iSse >= nSse) {
		if (nSse)
			// Clear old last bit
			pSse[nSse-1].flags &= (~SGL_FLAGS_LAST_ELEMENT);

		// Pad SGL with empty elements until we get to iSgl
		for (; nSgl < iSgl; nSgl++) {
			// Add one empty entry with EOB set
			pSse[iSse].address = 0l;
			pSse[iSse].count = 0;
			pSse[iSse].flags = SGL_FLAGS_IGNORE_ELEMENT | SGL_FLAGS_LOCAL_ADDRESS | SGL_FLAGS_END_OF_BUFFER;
			iSse++;
			nSse++;
		}
		nSse++;
	}
	else {// Not end of list.  Insert or overwrite in middle.
		// Find end of fragment list for element (iSgl)
		// iSse points to 1st (maybe only) fragment for element (iSgl)
		while (iSse < nSse-1 && !(pSse[iSse].flags & SGL_FLAGS_END_OF_BUFFER ))
			iSse++;

		// ASSERT iSse points at the last fragment of element (iSgl)

		// Clear EOB
		pSse[iSse].flags &= (~(SGL_FLAGS_END_OF_BUFFER | SGL_FLAGS_LAST_ELEMENT));

		// If last entry is not dummy placeholder, make room for a new entry
		if ((pSse[iSse].flags & SGL_FLAGS_SIMPLE_ADDRESS_ELEMENT)) {
			// Insert new simple element, mark EOB, after last fragment of iSgl
			// Move rest of list down one (maybe move zero bytes if at end of list)
			iSse++;
			memmove(&pSse[iSse+1], &pSse[iSse], (nSse-iSse) * sizeof(SGE_SIMPLE_ELEMENT));

			nSse++;
		}
	}

	if (direction == SGL_COPY) {
		if (sData > 0) {
			char *pNew=new char[sData];
			memmove(pNew, pData, sData);
			pData=pNew;
		}
		else
			pData = NULL;
	}
	
	pSse[iSse].address  = (U32)pData;
	pSse[iSse].count = sData;
	pSse[iSse].flags = 	SGL_FLAGS_SIMPLE_ADDRESS_ELEMENT 
					| SGL_FLAGS_LOCAL_ADDRESS 
					| SGL_FLAGS_END_OF_BUFFER 
					| ((direction == SGL_SEND) ? SGL_FLAGS_SEND : 0) 
					| ((direction == SGL_REPLY) ? SGL_FLAGS_REPLY : 0) 
					| ((direction == SGL_COPY) ? (SGL_FLAGS_SEND | SGL_FLAGS_FREE_ON_DTOR) : 0) 
					| ((direction == SGL_DYNAMIC_REPLY) ? (SGL_FLAGS_REPLY | SGL_DYNAMIC_REPLY_LENGTH) : 0);

	// Count new sse in message size
	sMessage = ((long*)(pSse+nSse) - (long*)this);
		
	// Make sure list ends with LAST set.
	pSse[nSse-1].flags |= SGL_FLAGS_LAST_ELEMENT;
}

// .AllocateSgl -- Allocate memory for a fragment ------------------Message-
//
void Message::AllocateSgl(U32 iSgl, U32 sData) {
	U32 nSse;
	U32 nSgl;
	SGE_SIMPLE_ELEMENT *pSse;

	// After a reply, clear sgl and start over
	if (IsReply()) {
		CleanSgl();
		flags &= ~MESSAGE_FLAGS_REPLY;
#ifdef DEBUG
		if (pDeleted)
			*pDeleted=true;
#endif
	}

	int iSse = ISse(iSgl, nSse, nSgl, pSse);	// All args returned.

	if (iSse < 0) {// No SGL 
		iSse=0;
		oSgl = sMessage;
	}

//Tracef("AllocateSgl iSse=%d nSse=%d\r\n", iSse, nSse);
		
	// ASSERT iSse points at the 1st fragment of iSgl,
	// or one after the end of the list (iSse == nSse).

	// Must extend list?
	if (iSse >= nSse) {
		if (nSse)
			// Clear old last bit
			pSse[nSse-1].flags &= (~SGL_FLAGS_LAST_ELEMENT);

		// Pad SGL with empty elements until we get to iSgl
		for (; nSgl < iSgl; nSgl++) {
			// Add one empty entry with EOB set
			pSse[iSse].address = 0l;
			pSse[iSse].count = 0;
			pSse[iSse].flags = SGL_FLAGS_IGNORE_ELEMENT | SGL_FLAGS_LOCAL_ADDRESS | SGL_FLAGS_END_OF_BUFFER;
			iSse++;
			nSse++;
		}
		nSse++;
	}
	else {// Not end of list.  Reduce element to one fragment.
		// Find end of fragment list for element (iSgl)
		// iSse points to 1st (maybe only) fragment for element (iSgl)
		int iSseStart=iSse;
		while (iSse < nSse-1 && !(pSse[iSse].flags & SGL_FLAGS_END_OF_BUFFER )) {
			SglDtor(pSse[iSse]);
			iSse++;
		}

		// ASSERT iSse points at the last fragment of element (iSgl)

		// Copy rest of list up after first entry.
		// Move rest of list down (maybe move zero bytes if at end of list)
		iSse++;
		memmove(&pSse[iSseStart+1], &pSse[iSse], (nSse-iSse) * sizeof(SGE_SIMPLE_ELEMENT));

		nSse -= (iSse - iSseStart - 1);
		iSse = iSseStart; // Put the index back where it was before the move -- RJB
	}

	int cbData=sData;
	int cbActual=-1;
	// Allocate fragments until sData bytes allocated.
	while (cbData && cbActual) {
		char *pData=new (didInitiator, &cbActual) char[cbData];
		if (pData == NULL || (int)pData == 0xFFFFFFFF) {
			pData = NULL;
			Tracef("[WARNING] Congestion!  Returning data to slot %d\n", DeviceId::ISlot(didInitiator));
			DetailedStatusCode = CTS_CHAOS_TPT_REPLY_CONGESTION;
			cbData = 0;
			cbActual = 0;
			}
		pSse[iSse].address  = (U32)pData;
		pSse[iSse].count = cbActual;
		pSse[iSse].flags = 	SGL_FLAGS_SIMPLE_ADDRESS_ELEMENT 
						| SGL_FLAGS_LOCAL_ADDRESS 
						| SGL_FLAGS_END_OF_BUFFER 
						| SGL_FLAGS_REPLY
						| SGL_FLAGS_FREE_ON_DTOR;

		if (cbActual < cbData) {
			cbData -= cbActual;
			// Insert new simple element, mark EOB, after last fragment of iSgl
			// Move rest of list down one (maybe move zero bytes if at end of list)
			pSse[iSse].flags &= ~SGL_FLAGS_END_OF_BUFFER;
			iSse++;
			memmove(&pSse[iSse+1], &pSse[iSse], (nSse-iSse) * sizeof(SGE_SIMPLE_ELEMENT));
			nSse++;
		}
		else {
			pSse[iSse].count = cbData;
			cbData = 0;
		}
	}

	if (sData == 0) {
		pSse[iSse].address  = 0;
		pSse[iSse].count = sData;
		pSse[iSse].flags = 	SGL_FLAGS_SIMPLE_ADDRESS_ELEMENT 
						| SGL_FLAGS_LOCAL_ADDRESS 
						| SGL_FLAGS_END_OF_BUFFER 
						| SGL_FLAGS_REPLY
						| SGL_FLAGS_FREE_ON_DTOR;
	}
	
	// Count new sse in message size
	sMessage = ((long*)(pSse+nSse) - (long*)this);
		
	// Make sure list ends with LAST set.
	pSse[nSse-1].flags |= SGL_FLAGS_LAST_ELEMENT;
}


// .FillSgl -- Retrieve fragment from SGL element ----------------------Message-
//
void Message::FillSgl() {
	if (!(flags & MESSAGE_FLAGS_FILLED)) {
		// Number of total fragments in message
		U32 nSse = NSSE;
		SGE_SIMPLE_ELEMENT *pSse = PSSE;

		// Temporary buffers get assigned to original or copy message
		for (U32 iSse=0; iSse < nSse; iSse++)
			if ((pSse[iSse].flags & SGL_FLAGS_SIMPLE_ADDRESS_ELEMENT)
			&& ((pSse[iSse].flags & SGL_FLAGS_DIR) == SGL_FLAGS_REPLY)
			&& (pSse[iSse].address == 0)
			&& (pSse[iSse].count != 0) )
				FillSgl(iSse, pSse, nSse);

		flags |= MESSAGE_FLAGS_FILLED;
	}
}

void Message::FillSgl(U32 &iSse, SGE_SIMPLE_ELEMENT *pSse, U32 &nSse) {
	int cbData=pSse[iSse].count;
	int cbActual=-1;
	// Allocate fragments until sData bytes allocated.
	while (cbData && cbActual) {
		char *pData=new (didInitiator, &cbActual) char[cbData];
		if (pData == NULL || (int)pData == 0xFFFFFFFF) {
			pData = NULL;
			Tracef("[WARNING] Congestion!  Returning data to slot %d\n", DeviceId::ISlot(didInitiator));
			DetailedStatusCode = CTS_CHAOS_TPT_REPLY_CONGESTION;
			cbData = 0;
			cbActual = 0;
			}
		pSse[iSse].address  = (U32)pData;
		pSse[iSse].count = cbActual;
		pSse[iSse].flags = 	SGL_FLAGS_SIMPLE_ADDRESS_ELEMENT 
						| SGL_FLAGS_LOCAL_ADDRESS 
						| SGL_FLAGS_END_OF_BUFFER 
						| SGL_FLAGS_REPLY
						| SGL_FLAGS_FREE_ON_DTOR;

		if (cbActual < cbData) {
			cbData -= cbActual;
			// Insert new simple element, mark EOB, after last fragment of iSgl
			// Move rest of list down one (maybe move zero bytes if at end of list)
			pSse[iSse].flags &= ~SGL_FLAGS_END_OF_BUFFER;
			iSse++;
			memmove(&pSse[iSse+1], &pSse[iSse], (nSse-iSse) * sizeof(SGE_SIMPLE_ELEMENT));
			nSse++;
		}
		else {
			pSse[iSse].count = cbData;
			cbData = 0;
		}
	}
}	
	
// .GetSgl -- Retrieve fragment from SGL element -----------------------Message-
//
void Message::GetSgl(U32 iSgl, void **pPData, U32 *pSData) {
	GetSgl(iSgl, 0, pPData, pSData);
}
	
// .GetSgl -- Retrieve fragment from SGL element -----------------------Message-
//
void Message::GetSgl(U32 iSgl, U32 iFragment, void **pPData, U32 *pSData) {
	
	SGE_SIMPLE_ELEMENT sse;

	GetSgl(iSgl, iFragment, sse, *pSData);
	
	*pPData = P_SGL(sse);
	*pSData = sse.count;
}
		
// .GetSgl -- Retrieve fragment from SGL element -----------------------Message-
//
void Message::GetSgl(U32 iSgl, SGE_SIMPLE_ELEMENT &sse) {
	GetSgl(iSgl, 0, sse);
}
		
// .GetSgl -- Retrieve fragment from SGL element -----------------------Message-
//
void Message::GetSgl(U32 iSgl, U32 iFragment, SGE_SIMPLE_ELEMENT &sse, U32 sData) {
	FillSgl();
	
	U32 nSse;
	U32 nSgl;
	SGE_SIMPLE_ELEMENT *pSse;
	
	int iSse = ISse(iSgl, nSse, nSgl, pSse);	// Returns all args

//Tracef("GetSgl iSse=%d nSse=%d\r\n", iSse, nSse);
	if (iSse < 0 || iSse >= nSse) {
		// Off end of SGL table
		sse.address =0;
		sse.count = 0;
		sse.flags = SGL_FLAGS_IGNORE_ELEMENT;
		return;
	}

	// ASSERT iSse points at the 1st fragment of iSgl.

	// Find desired fragment element
	int i;
	for (i=0; i < iFragment && !(pSse[iSse].flags & (SGL_FLAGS_END_OF_BUFFER | SGL_FLAGS_LAST_ELEMENT) ); i++)
		iSse++;

//Tracef("pSse.Flags=%x\r\n", pSse[iSse].FlagsCount.Flags);
	if (i == iFragment && !(pSse[iSse].flags & SGL_FLAGS_IGNORE_ELEMENT)) {
		// Return fields of sse
//Tracef(" returning %lx\r\n", pSse[iSse].PhysicalAddress);

		// If no reply memory supplied, allocate some.
		if (pSse[iSse].address == NULL && 
			(pSse[iSse].flags & SGL_FLAGS_DIR) == SGL_FLAGS_REPLY) {
			// If the SGL is flagged for dynamic reply length, look to the caller for the size it had better
			// be supplied.
			if ((pSse[iSse].flags & SGL_DYNAMIC_REPLY_LENGTH) && pSse[iSse].count == 0 && (flags & MESSAGE_FLAGS_REPLY) == 0)
				pSse[iSse].count = sData;
			if (pSse[iSse].count)
			{
				int cbActual=0;
				char *pData=new (didInitiator, &cbActual) char[pSse[iSse].count];
				if (pData == NULL || (int)pData == 0xFFFFFFFF) {
					pData = NULL;
					Tracef("[WARNING] Congestion!  Returning data to slot %d\n", DeviceId::ISlot(didInitiator));
					DetailedStatusCode = CTS_CHAOS_TPT_REPLY_CONGESTION;
					cbActual = 0;
					}
				pSse[iSse].address = (U32)pData;
				pSse[iSse].flags |= SGL_FLAGS_FREE_ON_DTOR;
				pSse[iSse].flags |= SGL_FLAGS_LOCAL_ADDRESS;
			}
		}

#if false
		if (pSse[iSse].count) {
#endif
			sse.address=(U32)P_SGL(pSse[iSse]);
			sse.count = pSse[iSse].count;
			sse.flags = pSse[iSse].flags | SGL_FLAGS_LOCAL_ADDRESS;
#if false
		}
#endif
	}
	else {
//Tracef(" returning NULL\r\n");
		// Off end of SGL table, or dummy entry
		sse.address = 0;
		sse.count = 0;
		sse.flags = SGL_FLAGS_IGNORE_ELEMENT;
	}
}

// .GetSgl -- Retrieve fragment from SGL element -----------------------Message-
//
SGE_SIMPLE_ELEMENT* Message::GetPSgl(U32 iSgl) {
	
	U32 nSse;
	U32 nSgl;
	SGE_SIMPLE_ELEMENT *pSse;
	
	int iSse = ISse(iSgl, nSse, nSgl, pSse);	// Returns all args

//Tracef("GetSgl iSse=%d nSse=%d\r\n", iSse, nSse);
	if (iSse < 0 || iSse >= nSse)
		// Off end of SGL table
		return NULL;

	// ASSERT iSse points at the 1st fragment of iSgl.

	return &pSse[iSse];
}

// .CopyToSgl -- Copy data into fragment list----------------------------------Message-
//
U32 Message::CopyToSgl(U32 iSgl, U32 offset, void *pData, U32 sData) {

	// If sgl is DYNAMIC, allocate buffer of given size.
	SGE_SIMPLE_ELEMENT sse;// dummy for GetSgl below
	GetSgl(iSgl, 0, sse, sData);
	
	U32 nSse;
	U32 nSgl;
	SGE_SIMPLE_ELEMENT *pSse;
	int iSse = ISse(iSgl, nSse, nSgl, pSse);

	if (iSse < 0 || iSse >= nSse)
		return sData;

	// ASSERT iSse points at the 1st fragment of iSgl.
	while (pSse[iSse].count < offset) {
		offset -= pSse[iSse].count;
		iSse++;
		if (pSse[iSse].flags & SGL_FLAGS_END_OF_BUFFER)
			return 0;
	}

	// ASSERT iSse points to fragment to start in; offset is where to start.
		
	while (sData > 0) {
		// Copy as much data to each fragment as it will hold.
		U32 cbCopy = sData;
		U32 cbFrag = pSse[iSse].count - offset;
		if (cbCopy > cbFrag)
			cbCopy = cbFrag;

		char *pDest=(char*)P_SGL(pSse[iSse]);

		bcopy(pData, (void*)(pDest + offset), cbCopy);

		sData -= cbCopy;
		offset = 0;
		if (pSse[iSse++].flags & SGL_FLAGS_END_OF_BUFFER)
			break;
	}
		
	return sData;
}


// .CopyFromSgl -- Copy data from fragment list -------------------------------Message-
//
U32 Message::CopyFromSgl(U32 iSgl, U32 offset, void *pData, U32 sData) {
	
	U32 nSse;
	U32 nSgl;
	SGE_SIMPLE_ELEMENT *pSse;
	int iSse = ISse(iSgl, nSse, nSgl, pSse);

	if (iSse < 0 || iSse >= nSse)
		return sData;

	// ASSERT iSse points at the 1st fragment of iSgl.
	while (pSse[iSse].count < offset) {
		offset -= pSse[iSse].count;
		iSse++;
		if (pSse[iSse].flags & SGL_FLAGS_END_OF_BUFFER)
			return 0;
	}

	// ASSERT iSse points to fragment to start in; offset is where to start.
		
	while (sData > 0) {
		// Copy as much data from each fragment as it contains.
		U32 cbCopy = sData;
		U32 cbFrag = pSse[iSse].count - offset;
		if (cbCopy > cbFrag)
			cbCopy = cbFrag;

		char *pDest=(char*)P_SGL(pSse[iSse]);

		bcopy((void*)(pDest + offset), pData, cbCopy);

		sData -= cbCopy;
		offset = 0;
		if (pSse[iSse++].flags & SGL_FLAGS_END_OF_BUFFER)
			break;
	}
		
	return sData;
}

// .LocalizeSgl -- Make all SGL addresses into local pointers --------Message-
//
void Message::LocalizeSgl() {

	U32 nSse = NSSE;
	SGE_SIMPLE_ELEMENT *pSse = PSSE;

	// Test all sgl physical addresses for being within our PCI window.
	// Change them to local physical addresses
	for (int iSse=0; iSse < nSse; iSse++) {
		U32 pci = PA_SGL(pSse[iSse]);
		if (ISLOCAL_PCI(pci)) {
			pSse[iSse].address = (U32)PLOCAL_PCI(pci);
			pSse[iSse].flags |= SGL_FLAGS_LOCAL_ADDRESS;
		}
	}
}

// .GlobalizeSgl -- Make all SGL addresses into PCI physical addresses--Message-
//
void Message::GlobalizeSgl() {
		
	U32 nSse = NSSE;
	SGE_SIMPLE_ELEMENT *pSse = PSSE;

	for (U32 iSse=0; iSse < nSse; iSse++) {
		U32 pa = PA_SGL(pSse[iSse]);
		if (IS_PALOCAL(pa))
			pa = PCI_PALOCAL(pa);
		pSse[iSse].address = pa;
		pSse[iSse].flags &= ~SGL_FLAGS_LOCAL_ADDRESS;
	}
}

// .BindTarget -- Bind user target to message ---------------------------Message-
//
STATUS Message::BindTarget(DID _didTarget) {
	
	didTarget = _didTarget;

	if (didTarget == DIDNULL)
		return CTS_CHAOS_INVALID_DID;
		
	if (flags & MESSAGE_FLAGS_INQUEUE) {
		Tracef("[WARNING] Attempt to Send already queued message. (Message::BindTarget)\n");
		Tracef("          Rq=%x didInitiator=%x didTarget=%x pMsg=%x \n",reqCode,didInitiator,didTarget,this);
		return CTS_CHAOS_INVALID_MESSAGE_FLAGS ;
	}
	
	flags &= ~MESSAGE_FLAGS_REPLY;

	if (flags & MESSAGE_FLAGS_TIMESTAMP)
		timestamp=pInitiatorContext->timestamp=Kernel::Time_Stamp();

	// Ddms need to filter OS Requests & Reply
	if (MASK_REQUEST_RANGE(reqCode) == REQ_OS_DDM)
		flags |= MESSAGE_FLAGS_FILTER;
		
	return OK;
}		

// .BindResponse -- Bind reply response to message -----------------------Message-
//
// Returns pMsg to pass to Reply.
//
STATUS Message::BindResponse(BOOL fLast,Message **ppMsg) {

	Message *pMsg;
	
	if (flags & MESSAGE_FLAGS_INQUEUE) {
		Tracef("[WARNING] Attempt to Reply to already queued message. (Message::BindResponse)\n");
		Dump("Reply message");
		return CTS_CHAOS_INVALID_MESSAGE_FLAGS ;
	}		

	// If no context, or doesn't match pMsg or refnum, error
	if (!IsValid())
	{
		Tracef("[ERROR] Reply to invalid/deleted message (probably double Reply). (Message::BindResponse)\n");
		Dump("Reply message");
		return CTS_CHAOS_INVALID_INITIATOR_ADDRESS;
	}

	pMsg = this;

	// If not LAST reply and msg is LAST, make a copy.
	if (!fLast && pMsg->IsLast())
		pMsg = new Message(pMsg);

	pMsg->flags =  (MESSAGE_FLAGS_REPLY | (fLast ? MESSAGE_FLAGS_LAST : 0));

	// Set message fields according to the initiator pIc.
	pMsg->didInitiator = pInitiatorContext->didInitiator;
	pMsg->contextTransaction = pInitiatorContext->contextTransaction;
	// Should be done in Ddm::Dispatcher
	pMsg->timestamp=pInitiatorContext->timestamp;
	
	*ppMsg = pMsg;

	return OK;
}
	
// .BindContext -- Bind user context to message -----------------------------Message-
//
// Bind the user context pContext to pMsg using the signature.
// Mark the context as belonging to didInitiator.
//
void Message::BindContext(DdmOsServices *_prcThis,ReplyCallback _rc, void *_pContext, const DID _didInitiator) {
TRACE(TRACE_MESSAGE, TRACE_L4, ("%s(%08lx)::Send(%s(%08lx))\n",DdmOsServices::OSGetClassName(_didInitiator),_didInitiator,NameRq(reqCode),reqCode));
	if (!IsLast()) {
		// Don't use same reply chain, this is a new message
		pInitiatorContext = NULL;
		flags |= MESSAGE_FLAGS_LAST;
		}

#ifdef DEBUG
	if (pDeleted)
		*pDeleted=true;
#endif

	// Reformat message for resending
	InitiatorContext *pBind = pInitiatorContext;

	// Add a InitiatorContext (PUSH)
	pBind = new InitiatorContext(pBind, this);
	pBind->didInitiator = _didInitiator;

	pBind->contextTransaction.LowPart = (U32) _pContext;
	pBind->replyCallbackMethod.pMethod = _rc;
	pBind->replyCallbackMethod.pInstance = _prcThis;
	
	contextTransaction = pBind->contextTransaction;

	// Set both lo/hi. Reply (NOT LAST) uses lo, Reply(LAST) uses hi.
	pInitiatorContext = pBind;

	didInitiator = _didInitiator;	//DeviceId::ISlot(didInitiator);
}

// .BindContext -- Bind user context to message -----------------------------Message-
//
// Bind the user context pContext to pMsg using the signature.
// Mark the context as belonging to didInitiator.
//
void Message::BindContext(long _signature, void *_pContext, const DID _didInitiator) {
TRACE(TRACE_MESSAGE, TRACE_L4, ("%s(%08lx)::Send(%s(%08lx))\n",DdmOsServices::OSGetClassName(_didInitiator),_didInitiator,NameRq(reqCode),reqCode));
	if (!IsLast()) {
		// Don't use same reply chain, this is a new message
		pInitiatorContext = NULL;
		flags |= MESSAGE_FLAGS_LAST;
		}

#ifdef DEBUG
	if (pDeleted)
		*pDeleted=true;
#endif

	// Reformat message for resending
	InitiatorContext *pTc = pInitiatorContext;

	// Add a InitiatorContext (push)
	pTc = new InitiatorContext(pTc, this);
	pTc->didInitiator=_didInitiator;

	pTc->contextTransaction.HighPart = (U32) _signature;
	pTc->contextTransaction.LowPart = (U32) _pContext;
	contextTransaction = pTc->contextTransaction;

	// Set both lo/hi. Reply (NOT LAST) uses lo, Reply(LAST) uses hi.
	pInitiatorContext = pTc;

	didInitiator = _didInitiator;	//DeviceId::ISlot(didInitiator);
}

// .Dump -- Print a message ---------------------------------------------Message-
//
void Message::Dump(char *pTitle) {
	U16 _sPayload;
	U16 iCol1,iCol2,jj;

	Tracef("\n%s - %08lx\n",pTitle, this);
	Tracef("0x%02lx       sHeader = %u\n",sHeader,sHeader);
	Tracef("0x%02lx       flags=",flags);
	for (int i=0; aFlags[i].flags; i++)
		if (flags & aFlags[i].flags)
			Tracef(aFlags[i].name);
	Tracef("\n");
	Tracef("0x%02lx       oSgl    = %u bytes\n",oSgl,oSgl<<2);
	Tracef("0x%02lx       sMessage= %u bytes\n",sMessage,sMessage<<2);
	Tracef("0x%08lx reqCode\t%s\n",reqCode, NameRq(reqCode));
	Tracef("0x%08lx didTarget\n",didTarget);
	Tracef("0x%08lx didInitiator\n",didInitiator);
	Tracef("0x%08lx refnum\n0x%08lx\n",(U32)(refnum >> 32), (U32)refnum);
	Tracef("0x%08lx timestamp\n0x%08lx\n",(U32)(timestamp >> 32), (U32)timestamp);
	Tracef("0x%08lx pInitiatorContext\n",pInitiatorContext);
	Tracef("0x%08lx contextTransaction= %lu,%lu\n0x%08lx\n",contextTransaction.HighPart,contextTransaction.HighPart,
														 contextTransaction.LowPart,contextTransaction.LowPart);
	Tracef("0x%08lx DetailedStatusCode (%s)\n",DetailedStatusCode,IsReply() ? "Reply" : "Request");

	_sPayload = sHeader - sizeof(Message);
	if (_sPayload == 0)
		Tracef("Payload: none\n");
	else {
		Tracef("Payload:  %u bytes\n",_sPayload);
		int iPayload = sizeof(Message) >> 2;
		for (int ii=0; ii < _sPayload; ) {
			Tracef("    ");
			for (iCol1=0,jj=ii; ii < _sPayload && iCol1 < 16; ii++,iCol1++)
				Tracef("%02x ", ((U8*)this)[iPayload + ii]);

			for (iCol2=iCol1-1; iCol2 < 16; iCol2++)
				Tracef(" ");
				
			Tracef("  \""); 
			for (iCol2=0; iCol2 < iCol1; jj++,iCol2++) {
				char ch = ((U8*)this)[iPayload + jj];
				Tracef("%c", (ch < 32 || ch > 126) ? '.' : ch);
			}
			Tracef("\"\n"); 
		}
	}

	int nSse = NSSE;

	if (nSse == 0)
		Tracef("SGLs: none\n");
	else {
		Tracef("SGLs: nSse=%d\n", nSse);
				
		// Find 1st element of iSgl
		SGE_SIMPLE_ELEMENT *pSse = PSSE;
		int iSse=0;
		int iElement=0;
		int iFrag=0;
		while (iSse < nSse) {
			Tracef(" (%d,%d): %lx[%lx] flags=%x", iElement, iFrag, pSse[iSse].address, pSse[iSse].count, pSse[iSse].flags);

			if (pSse[iSse].flags & SGL_FLAGS_IGNORE_ELEMENT)
				Tracef(" EMPTY");
			if ((pSse[iSse].flags & SGL_FLAGS_DIR) == SGL_FLAGS_SEND)
				Tracef(" SEND");
			if ((pSse[iSse].flags & SGL_FLAGS_DIR) == SGL_FLAGS_REPLY)
				Tracef(" REPLY");
			if (pSse[iSse].flags & SGL_FLAGS_FREE_ON_DTOR)
				Tracef(" COPY");
			if (pSse[iSse].flags & SGL_DYNAMIC_REPLY_LENGTH)
				Tracef(" DYNAMIC");
			Tracef("\n");
			
			iFrag++;
			if (pSse[iSse].flags & SGL_FLAGS_END_OF_BUFFER) {
				iElement++;
				iFrag=0;
			}
			
			iSse++;
		}
	}
	Tracef("\n");
}

// GetSglDataSize -- return size of SGL --------------------------------Message-
//
U32 Message::GetSglDataSize(U32 iSgl) {
	U32   cbRows=0;
	void *pRows;
	GetSgl(iSgl, &pRows, &cbRows);
		
	return cbRows;	// Returns size row data
}
	
// GetSglDataPtr -- returns SGL pointer & optional size ----------------Message-
//
// Warning: May point to framented SGL data
//
void *Message::GetSglDataPtr(U32 iSgl, U32 *pcbData) {
	U32   cbRows=0;
	void *pRows;
	GetSgl(iSgl, &pRows, &cbRows);
	
	if (pcbData != NULL)
		*pcbData = cbRows;
		
	return pRows;
}	

// GetSglDataPtr -- returns SGL pointer & optional size ----------------Message-
//
// Returns *pcbData as count of items of size sType
//
// Warning: May point to framented SGL data
//
void * Message::GetSglDataPtr(U32 iSgl, U32 *pcbData, U32 sType) {
	void *pData = GetSglDataPtr(iSgl,pcbData);
	if (pcbData != NULL && sType != 0)
		(*pcbData) /= sType;
	
	return pData;
}		

// GetSglDataCopy -- return copy of SGL and optional size --------------Message-
//
// Allocates buffer and returns copy of SGL data
//
void *Message::GetSglDataCopy(U32 iSgl, U32 *pcbData) {
	U32 cbData;
	GetSglDataPtr(iSgl,&cbData);

	// Allocate a buffer for the client and copy the data there.
	void *pCopy = new char[cbData];
	CopyFromSgl(iSgl,0,pCopy,cbData);
	
	if (pcbData != NULL)
		*pcbData = cbData;
			
	return pCopy;
}

// GetSglDataCopy -- return copy of SGL and optional size --------------Message-
//
// Allocates buffer and returns copy of SGL
// Returns *pcbData as count of items of size sType
//
void * Message::GetSglDataCopy(U32 iSgl, U32 *pnType, U32 sType) {
	void *pData = GetSglDataCopy(iSgl,pnType);
	if (pnType != NULL && sType != 0)
		(*pnType) /= sType;
	
	return pData;
}
		
// .New -- Over-ride new operator --------------------------------------Message-
//
// All Message-derived classes get this operator new.
// It allocates enough room to build the largest message.
//
void *Message::operator new(unsigned int sMsg) {
//Tracef("Message::new = %u\n",sMsg);	
	if (sMsg > MESSAGESIZE)
		Tracef("[WARNING] Attemping to new Message > %u bytes. (Message::new)\n",MESSAGESIZE);

	Message *pMsg = (Message *) ::operator new(MESSAGESIZE);

	pMsg->sHeader = (sMsg + 3) / 4 * 4;	// Rounded up to U32s

	return (void*) pMsg;
}

// .= -- Over-ride = operator ------------------------------------------Message-
//
Message &Message::operator=(Message &msgSrc) {
	for (int iL=0; iL < msgSrc.sMessage; iL++)
		((U32*)this)[iL]=((U32*)&msgSrc)[iL];

	return *this;
}


InitiatorContext::InitiatorContext(InitiatorContext *pIc_, Message *pMsg_):pIcNext(pIc_), pMsg(pMsg_), refnum(pMsg_->refnum), pNext(NULL), pPrev(NULL), pQueue(NULL), vdn(VDNNULL) {}


//**************************************************************************************************
// Update Log:
//	$Log: /Gemini/Odyssey/Oos/Message.cpp $
// 
// 74    2/15/00 7:31p Jaltmaier
// Leak:    Undeleted Reply detector.  Better leak statistics.
// 13121: CleanAllSgl usage cleanup.
// Unit test: remove GetBootData reference from MSL_Initialize.
// 
// 75    2/14/00 11:18a Jaltmaier
// Termination stuff.
// Message reply leak detector.
// 
// 73    2/10/00 5:12p Jaltmaier
// Fix CleanAllSgl to not erase dynamic data.
// 
// 72    2/08/00 8:52p Tnelson
// Fix Load/Delete VirtualDevice request
// Added SYSTEMMASTER Macro
// Added Termination to Ddm
// Fix PtsLoader bug
// 
// 73    2/08/00 6:10p Tnelson
// Joes fixed Dynamic size return bug
// 
// 8/24/99 Joe Altmaier: Don't use caller's size in GetSgl(DYNAMIC) if message is Reply.
//						Fixes bug where service returns DYNAMIC with size==0
// 8/08/99 Bob Butler: Added CleanAllSgl() to delete client allocated SGLs
// 8/08/99 Bob Butler:	 Fixed bug in AllocateSgl when moving previously allocated fragments
//						 to make room for the new fragment.
// 4/06/99 Joe Altmaier: sHeader now includes payload.
// All this in support of non-LAST reply.
// Added Clone(), AllocateSgl().  Use SglDtor().  GetSgl may allocate memory.
// 3/24/99 Joe Altmaier: Add SGL_FREE_ON_DTOR, SGL_COPY.
// 2/13/99 Tom Nelson: 	 Remove I2O_ from message defines.
// 2/12/99 Joe Altmaier: Continue abandoning I2O.  DebugSgl -> Dump.  
// 2/07/99 Tom Nelson:	 Begin abandoning I2O
// 7/21/98 Joe Altmaier: Create file
