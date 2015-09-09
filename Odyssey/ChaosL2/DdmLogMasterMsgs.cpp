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
*	This file contains the source code for the Log Master messages.
* 
* Update Log: 
* 07/02/99 Bob Butler: Create file
*
*************************************************************************/

#include "LogMasterMessages.h"




// Create an array of Event objects from the data supplied in the reply SGL.
// The number of elements in the array is returned.  
// The caller supplies the pointer and is responsible for calling delete[] on it.

U16 MsgQueryLog::GetLogEntries(Event **paEvt_)
{
	*paEvt_ = NULL;

	U32 sSGL;
	sSGL = GetSSgl(iSglLogEntries);

	if (!sSGL || !cEntries)
		return 0;

	U16 offset = 0;

	*paEvt_ = new Event[cEntries];

	for (int i=0; i < cEntries; ++i)
	{
		U32 cbEvent;
		CopyFromSgl(iSglLogEntries, offset, &cbEvent, sizeof(cbEvent));

		void *pData = new char[cbEvent];
		
		CopyFromSgl(iSglLogEntries, offset, pData, cbEvent);
		(*paEvt_)[i].Set(pData);
		offset += cbEvent;
		delete pData;
	}
	return cEntries;
}

void MsgQueryLog::SetLogEntries(U16 cEntries_, Event **paEvt_)
{
	cEntries = cEntries_;

	U16 cb = 0;
	int i;

	void **apData = new void*[cEntries];
	U16 *aCbEvt = new U16[cEntries];

	for (i=0; i < cEntries; ++i)
	{
		aCbEvt[i] = paEvt_[i]->GetEventData(&(apData[i]));
		cb += aCbEvt[i];
	}

	AllocateSgl(iSglLogEntries, cb);

	U16 offset = 0;

	for (i =0; i < cEntries; ++i)
	{
		CopyToSgl(iSglLogEntries, offset, apData[i], aCbEvt[i]);
		delete apData[i];
		offset += aCbEvt[i];
	}
	delete []apData;
	delete []aCbEvt;
}
	
		

Timestamp MsgQueryLogMetaData::GetFirstTimestamp()
{
	Timestamp tsMin = aTs[lmdFirst][0];

	for (int i = 1; i < Event::SEVERITY_COUNT; ++i)
		tsMin = (tsMin > aTs[lmdFirst][i]) ? aTs[lmdFirst][i] : tsMin;
	return tsMin;
}

Timestamp MsgQueryLogMetaData::GetLastTimestamp()
{
	Timestamp tsMax = aTs[lmdLast][0];

	for (int i = 1; i < Event::SEVERITY_COUNT; ++i)
		tsMax = (tsMax < aTs[lmdLast][i]) ? aTs[lmdLast][i] : tsMax;
	return tsMax;
}


U32 MsgQueryLogMetaData::GetFirstSequenceNumber()
{
	U32 seqMin = aSeqNum[lmdFirst][0];

	for (int i = 1; i < Event::SEVERITY_COUNT; ++i)
		seqMin = (seqMin > aSeqNum[lmdFirst][i]) ? aSeqNum[lmdFirst][i] : seqMin;
	return seqMin;
}

U32 MsgQueryLogMetaData::GetLastSequenceNumber()
{
	U32 seqMax = aSeqNum[lmdLast][0];

	for (int i = 1; i < Event::SEVERITY_COUNT; ++i)
		seqMax = (seqMax < aSeqNum[lmdLast][i]) ? aSeqNum[lmdLast][i] : seqMax;
	return seqMax;
}


U32 MsgQueryLogMetaData::GetEntryCount()
{
	U32 c = 0;
	for (int i = 0; i < Event::SEVERITY_COUNT; ++i)
		c += aCount[i];
	return c;
}

