/* Message.h -- Define message structure
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
 * Revision History:
 * 6/12/98 Joe Altmaier: Create file
 * 7/21/98 Joe Altmaier: Use I2O definitions; add TyContext
 * 9/29/98 Jim Frandeen: Use CtTypes.h instead of stddef.h
 * 2/07/99 Tom Nelson:   Use OsTypes.h not CtTypes.h; Begin abandoning I2O
 * 2/12/99 Joe Altmaier: DebugSgl -> Dump.  Don't redefine I2O if already defined
 * 2/12/99 Tom Nelson:	 Removed I2O_ from constant names
 * 3/24/99 Joe Altmaier: Add SGL_FREE_ON_DTOR, SGL_COPY.
 * 3/30/99 Joe Altmaier: Fix merge mess VSS volunteered without asking.
 * 5/07/99 Eric Wedel: added class name parameter to REPLYCALLBACK() def (GH).
 * 8/08/99 Bob Butler: Added CleanAllSgl() to delete client allocated SGLs
 *
**/

#ifndef _Message_h
#define _Message_h

#include "Os.h"
#include "Kernel.h"
#include "WaitQueue_T.h"

// Message Flags
#define MESSAGE_FLAGS_DEAD         			0x0002
#define MESSAGE_FLAGS_AUTO_RETRY 			0x0004
#define MESSAGE_FLAGS_TIMESTAMP             0x0008
#define MESSAGE_FLAGS_NOFAULTIN             0x0010
#define MESSAGE_FLAGS_FAIL                  0x0020
#define MESSAGE_FLAGS_LAST                  0x0040
#define MESSAGE_FLAGS_REPLY                 0x0080
#define MESSAGE_FLAGS_INQUEUE				0x0100
#define MESSAGE_FLAGS_OPEN					0x0200
#define MESSAGE_FLAGS_FILLED				0x0400
#define MESSAGE_FLAGS_FILTER				0x8000
typedef U16 MSGFLAGS;

// To suppress the 'undeleted reply' warning use this macro
// Only do this if the reply is 'kept in your pocket' past 
// the reply handler return or if the message is re-used
#ifdef DEBUG
#define MSGREPLYKEEP(pMsg) if (pMsg->pDeleted) pMsg->pDeleted = true
#else
#define MSGREPLYKEEP(pMsg)
#endif

// SGL Flags
#define    SGL_FLAGS_LAST_ELEMENT              	0x80
#define    SGL_FLAGS_END_OF_BUFFER             	0x40
#define    SGL_FLAGS_IGNORE_ELEMENT            	0x20
#define    SGL_FLAGS_SIMPLE_ADDRESS_ELEMENT    	0x10
#define    SGL_FLAGS_LOCAL_ADDRESS             	0x08
#define    SGL_FLAGS_DIR                       	0x04
#define     SGL_FLAGS_REPLY                    	0x00
#define     SGL_FLAGS_SEND                     	0x04
#define	   SGL_FLAGS_FREE_ON_DTOR				0x02
#define    SGL_DYNAMIC_REPLY_LENGTH				0x01

#define P_SGL(sse) 	( sse.address? ( (sse.flags & SGL_FLAGS_LOCAL_ADDRESS)? (void*)sse.address :P_PA(sse.address) ) : (void*)sse.address )
#define PA_SGL(sse) ( sse.address? ( (sse.flags & SGL_FLAGS_LOCAL_ADDRESS)? PA_P(sse.address) :sse.address ) : sse.address )
#define NSSE (oSgl? (sMessage - oSgl) * sizeof(U32) / sizeof(SGE_SIMPLE_ELEMENT) :0)
#define PSSE PSSE_(this)
#define PSSE_(pMsg) (SGE_SIMPLE_ELEMENT*)(((U32*)pMsg) + oSgl)

typedef struct _SGE_SIMPLE_ELEMENT {
   unsigned 	count:24;
   unsigned		flags:8;
   U32          address;
   BOOL IsEob() { return (flags & SGL_FLAGS_END_OF_BUFFER); }
   BOOL IsLast() { return (flags & SGL_FLAGS_LAST_ELEMENT); }
   BOOL IsClients() { return !(flags & SGL_FLAGS_FREE_ON_DTOR); }
   BOOL IsSend() { return ((flags & SGL_FLAGS_DIR) == SGL_FLAGS_SEND); }
} SGE_SIMPLE_ELEMENT;

typedef struct {
   U32 LowPart;
   U32 HighPart;
} MsgContext;

class Message;

typedef Message MessageReply;	//***OBSOLETE***

class DdmOsServices;
typedef STATUS (DdmOsServices::*ReplyCallback)(Message *);

#ifdef WIN32
#define REPLYCALLBACK(clas,method)	(ReplyCallback) method
#elif defined(__ghs__)  // Green Hills
#define REPLYCALLBACK(clas,method)	(ReplyCallback) &clas::method
#else	// MetroWerks
#define REPLYCALLBACK(clas,method)	(ReplyCallback)&method
#endif

class ReplyCallbackMethod {
public:
	DdmOsServices *pInstance;	// Callback instance
	ReplyCallback pMethod;		// Callback method

	ReplyCallbackMethod() : pInstance(NULL), pMethod(NULL) {}
		
	BOOL IsValid()			  { return pInstance != NULL; 	}
	ERC Invoke(Message *pMsg) { return (pInstance->*pMethod)(pMsg); }
};


class InitiatorContext;

typedef WaitQueue_T<InitiatorContext> IcQueue;

// Initiator Context, private structure to Messenger class
class InitiatorContext {
public:
	// Links in VDN Send chain
	InitiatorContext *pNext;
	InitiatorContext *pPrev;

	InitiatorContext *pIcNext;// link to next tc in reply chain
	DID didInitiator;		// address to Reply
	MsgContext contextTransaction;	// didInitiator expects this
	Message *pMsg;			// back-link to message

	ReplyCallbackMethod	replyCallbackMethod;

	TIMESTAMP timestamp;
	REFNUM refnum;
	
	// Failsafe routing fields
	VDN vdn;
	DID did;
	IcQueue *pQueue;
	
	InitiatorContext(InitiatorContext*pIc, Message *pMsg);

	~InitiatorContext() { pMsg=NULL; if (pQueue) pQueue->Unlink(this); }

	void Link(IcQueue *pQueue_) {
		if (pQueue) 
			pQueue->Unlink(this);

		this->pQueue=pQueue_;
		
		// Add Ic pIc to queue pFs
		pQueue->Put(this);
	}
};


class Message {	// MUST BE MULTIPLE OF 4 BYTES!!!
public:
    REQUESTCODE reqCode;// Message Request Code (see RequestCode.h)
  	U16 sHeader;		// size of message header in bytes, including payload

   	U16 flags;			// bit flags
   	U16 oSgl;			// offset of SGL in message /4

   	U16 sMessage;		// Size of (header + payload + SGL)/4

   	DID didTarget;
   	DID didInitiator;
   	InitiatorContext *pInitiatorContext;
	STATUS DetailedStatusCode;	// Status code on reply

	BOOL *pDeleted; // debugging field

	// put 64 bit fields here
	MsgContext contextTransaction;
	TIMESTAMP timestamp;
	REFNUM refnum;

		
#define MESSAGESIZE 	256

// direction values
#define SGL_REPLY		 	0
#define SGL_SEND 			1
#define SGL_COPY			2
#define SGL_DYNAMIC_REPLY	3

	Message(REQUESTCODE _msgCode);
	Message(REQUESTCODE _msgCode, U32 _sPayload);
	Message(REQUESTCODE _msgCode, U32 _sPayload, MSGFLAGS _flags);
	~Message();

	void Construct(REQUESTCODE _msgCode, U32 _sHeader, MSGFLAGS _flags);
	
	// New message is copy of argument.
	Message(Message *);
	
	// Return a copy of self.
	Message *Clone();

	// Reallocate self in cheaper memory.
	Message *Migrate();
	
	// Delete any sgl items user didn't allocate.	
	Message *CleanSgl();

	// Delete any sgl items user DID allocate.	Should only be called on a last reply
	// before deleting the message.  Also calls CleanSgl().
	Message *CleanAllSgl();

	// Copy reply fields to original message.
	Message *CopyReply(Message *pMsgOriginal);

	// Copy original message fields to reply.
	Message *KeepReply(Message *pMsgOriginal);

	// Clone, copy prototype data fields to reply message.
	Message *MakeReply(Message *pMsgProto);

	InitiatorContext *PopIc();

 	// Returns the low 32 bits of TransactionContext.
	// Returns NULL if signature is not 0, and the high
	// 32 bits of TransactionContext do not match signature.
	void* GetContext(long a_signature = 0);
  
    STATUS Status() const {
    	return DetailedStatusCode;
   	}
    
	// Returns true if the message is internally consistent.
	BOOL IsValid() const {
		return (pInitiatorContext && pInitiatorContext->refnum == refnum);
		}
	
	// Returns true if the message was received as a reply.
	BOOL IsReply() const {
		return (flags & MESSAGE_FLAGS_REPLY) != 0;
	}
		
	// Returns true if the message was received as a LAST reply.
	BOOL IsLast() const {
		return (flags & MESSAGE_FLAGS_LAST) != 0;
	}

	// Returns true if the message was from a dead IOP.
	BOOL IsDead() const {
		return (flags & MESSAGE_FLAGS_DEAD) != 0;
	}

	TIMESTAMP Latency() {
		return Kernel::Time_Stamp() - timestamp;
	}

	Message *POriginalMessage() {
		return (pInitiatorContext? pInitiatorContext->pMsg :this);
	}

	// Payload methods
	
	// Add data at pPayload to the message.
	// sPayload bytes will be copied.
	// Note the message will be rounded up to a multiple of 4 bytes.
	void AddPayload(void *_pPayload, U32 _sPayload);
	
	void AddReplyPayload(void *pPayload, U32 sPayload) {
		AddPayload(pPayload,sPayload);
	}
	
	// Get a pointer to the payload portion of the message.
	// The returned pointer can be cast to a struct* appropriate
	// to the payload.
	void *GetPPayload();
	U32 GetSPayload();
	
	// SGL methods
	
	// Add an SGL fragment entry for the buffer indexed by iSgl 
	// to reference sData bytes of data starting at pData.
	// If the SGL table is not defined or not long enough,
	// it will be created or extended long enough to accomodate.
	// Undefined descriptors will be set to 0 bytes of data.
	void AddSgl(U32 iSgl, void *pData, U32 sData, int direction=SGL_SEND);
	void AddSgl(U32 iSgl, SGE_SIMPLE_ELEMENT &sse);

	// Allocate memory for an SGL element indexed by iSgl
	// to reference sData bytes.
	void AllocateSgl(U32 iSgl, U32 sData);

	// Return the number of SGL buffers.
	// The parameter iSgl in GetSgl varies from 0 to GetCSgl()-1.
	U32 GetCSgl();
	
	// Return the number of fragments in the given SGL buffer.
	U32 GetCFragment(U32 iSgl);

	// Get the first SGL fragment for the buffer indexed by iSgl.
	// If iSgl exceeds the length of the SGL table, NULL is returned.
	SGE_SIMPLE_ELEMENT* GetPSgl(U32 iSgl);

	// Get the size of all SGL fragment for the buffer indexed by iSgl.
	U32 GetSSgl(U32 iSgl);

	// Fill in SGL with no address.
	void FillSgl();
	void FillSgl(U32 &iSse, SGE_SIMPLE_ELEMENT *pSse, U32 &nSse);


	// Get the first SGL fragment for the buffer indexed by iSgl.
	// Return size and address separately, or in Buffer structure.
	// If iSgl exceeds the length of the SGL table, (0,NULL) is returned.
	void GetSgl(U32 iSgl, void **pPData, U32 *pSData);
	void GetSgl(U32 iSgl, SGE_SIMPLE_ELEMENT &sse);

	// Get the given SGL fragment for the buffer indexed by iSgl.
	void GetSgl(U32 iSgl, U32 iFragment, SGE_SIMPLE_ELEMENT &sse, U32 sData = 0);
	void GetSgl(U32 iSgl, U32 iFragment, void **pPData, U32 *pSData);

	// Copy data to the SGL return buffer, starting at the given offset
	// within the return buffer.  Truncates data if it won't fit.
	// Handles multiple return buffer fragments.
	// Returns the number of bytes NOT returned (hopefully 0).
	U32 CopyToSgl(U32 iSgl, U32 offset, void *pData, U32 sData);
	U32 CopyFromSgl(U32 iSgl, U32 offset, void *pData, U32 sData);
	
	// Make PhysicalAddress pci values into pa when local
	void LocalizeSgl();
	// Make pa into PhysicalAddress pci values
	void GlobalizeSgl();

	STATUS BindTarget(DID didTarget);
	STATUS BindResponse(BOOL fLast,Message **ppMsg);
	void BindContext(DdmOsServices *prcThis,ReplyCallback rc, void *pContext, const DID didInitiator);
	void BindContext(long signature, void *pContext, const DID didInitiator);

	REFNUM MakeRefNum()			{ return Os::MakeRefNum(); }
	void Dump(char *pTitle);

	// Helpful to derived messages
	U32  GetSglDataSize(U32 iSgl);
	void * GetSglDataPtr(U32 iSgl, U32 *pcbData=NULL);
	void * GetSglDataCopy(U32 iSgl, U32 *pcbData=NULL);
	void * GetSglDataPtr(U32 iSgl, U32 *pcbData, U32 sType);
	void * GetSglDataCopy(U32 iSgl, U32 *pcbData, U32 sType);

	// Override allocation
	void *operator new(unsigned int);
	Message &operator=(Message &msgSrc);

private:
	int ISse(U32 iSgl, U32 &nSse, U32 &nSgl, SGE_SIMPLE_ELEMENT *&pSse);
	void SglDtor(SGE_SIMPLE_ELEMENT &sse) {
		if ((sse.flags & SGL_FLAGS_FREE_ON_DTOR) && sse.address)
			delete (void*)sse.address;
		sse.address=NULL;
	}
};	// class Message


#endif //_Message_h
