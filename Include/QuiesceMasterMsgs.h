/*************************************************************************
*
* This material is a confidential trade secret and proprietary 
* information of ConvergeNet Technologies, Inc. which may not be 
* reproduced, used, sold or transferred to any third party without the 
* prior written consent of ConvergeNet Technologies, Inc.  This material 
* is also copyrighted as an unpublished work under sections 104 and 408 
* of Title 17 of the United States Code.  Law prohibits unauthorized 
* use, copying or reproduction.
*
* Description:
*	This file contains the definition of the Quiesce Master messages.
* 
* Update Log: 
* 06/27/99 Bob Butler: Create file
*
* 10/12/99 Bob Butler: 
*			Added routed Quiesce for IOP local DDMs
*************************************************************************/

#ifndef QuiesceMasterMessages_h
#define QuiesceMasterMessages_h

#include "Trace_Index.h"		// include the trace module numbers

#ifdef TRACE_INDEX	// GAI
#undef TRACE_INDEX
#endif	// TRACE_INDEX

#define	TRACE_INDEX		TRACE_QUIESCEMASTER	// set this modules index to your index	
#include "Odyssey_Trace.h"			// include trace macros

#include "CTTypes.h"
#include "message.h"

class RqQuiesceBus : public Message
{
public:
	RqQuiesceBus(U32 bus_) : Message(REQ_QUIESCE_BUS), bus(bus_)
	{
	}

	U32 GetBus() const { return bus; }
	
		
private:
	U32 bus;
		
};

class RqQuiesceIop : public Message
{
public:
	RqQuiesceIop(TySlot slot_) : Message(REQ_QUIESCE_IOP), slot(slot_)
	{
	}

	U32 GetSlot() const { return slot; }
	
		
private:
	U32 slot;
		
};


class RqQuiesceVirtualCircuit : public Message
{
public:
	RqQuiesceVirtualCircuit(VDN vdn_) : Message(REQ_QUIESCE_VCIRCUIT), vdn(vdn_)
	{
	}

	U32 GetVdn() const { return vdn; }
	
		
private:
	VDN vdn;
		
};

class RqRoutedQuiesceIopLocal : public Message
{
public:
	RqRoutedQuiesceIopLocal(VDN vdn_) : Message(REQ_QUIESCE_ROUTED_IOPLOCAL), vdn(vdn_)
	{
	}

	U32 GetVdn() const { return vdn; }
	
		
private:
	VDN vdn;
		
};


#endif