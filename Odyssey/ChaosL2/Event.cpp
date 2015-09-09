/*************************************************************************
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// Description:
// This is the Event class source file. 
// 
// Update Log: 
// 11/30/98	JFL	Created.
//			RJB	See Event.h for a list of changes prior to initial checkin
// 08/08/99 RJB Several changes for use in the system log, mostly template related
*************************************************************************/
#include "Event.h"

#include <string.h>
#include <stdio.h>
#include "UnicodeString.h"

const U16 ARRAY_GROW_SIZE=5;

/*
 * Construct with a DID and a VDN.  
 */
Event::Event(STATUS ec_, DID did_, VDN vdn_)
{
	ed.did = did_;
	ed.vdn = vdn_;
	ed.ec = ec_;
	ed.slot = Address::iSlotMe;
	ed.cParms = 0;
	ed.sequenceNum = 0;
	ed.timeStamp = 0;
	ed.significantParms = 0xFFFFFFFF;
	pEvtParms = new EventParm *[ARRAY_GROW_SIZE];
}

/*
 * Construct with NULL DID and VDN 
 */
Event::Event(STATUS _ec)
{
	ed.did = DIDNULL;
	ed.vdn = VDNNULL;
	ed.ec = _ec;
	ed.slot = Address::iSlotMe;
	ed.cParms = 0;
	ed.sequenceNum = 0;
	ed.timeStamp = 0;
	ed.significantParms = 0xFFFFFFFF;
	pEvtParms = new EventParm *[ARRAY_GROW_SIZE];
}


Event::Event(void *epd_) 		// a blob containing everything about the event
{
	Set(epd_);
}

void Event::Set(void *epd_)
{
	U32 cb = 0;
	
	char *p = (char *)epd_;
	memcpy(&cb, p, sizeof(cb));	// total bytes
	p += sizeof(cb);
	memcpy(&ed, p, sizeof(ed));
	p += sizeof(ed);
//	p += sizeof(ed.cParms);
	
	pEvtParms = new EventParm *[ARRAY_GROW_SIZE * (ed.cParms/ARRAY_GROW_SIZE + (ed.cParms % ARRAY_GROW_SIZE ? 1 : 0))];

	for (int i=0; i < ed.cParms; ++i)			
	{
		pEvtParms[i] = new EventParm;
		memcpy(&pEvtParms[i]->cBytes, p, sizeof(pEvtParms[i]->cBytes));
		p += sizeof(pEvtParms[i]->cBytes);
		memcpy(&pEvtParms[i]->parmType, p, sizeof(pEvtParms[i]->parmType));
		p += sizeof(pEvtParms[i]->parmType);
		pEvtParms[i]->pData = new char[pEvtParms[i]->cBytes];
		memcpy(pEvtParms[i]->pData, p, pEvtParms[i]->cBytes);
		p += pEvtParms[i]->cBytes;
	}
}


Event::~Event()
{
	for (int i=0; i < ed.cParms; ++i)
	{
		delete pEvtParms[i]->pData;
		delete pEvtParms[i];
	}
	delete []pEvtParms;
}

U32 Event::GetEventData(void **pData_) const
{
   /*
	* build a blob containing the paramater data.  
	* the blob is formatted as follows:
	* <total bytes><parm count><parm 1 size><parm 1 type><parm 1 data>...<parm n data>...
	* some of the values (such as total size) are not really necessary, 
	* but may be useful for debug.  The caller supplies the pointer and must delete it.
	*/


	U32 cb = 0, i=0;
	cb = sizeof(cb) + sizeof(ed);			// reserve room for the total size of the blob and event data
	for (i=0; i < ed.cParms; ++i)			// calculate the total size of the blob
		cb += pEvtParms[i]->cBytes			// bytes of data
		+ sizeof(pEvtParms[i]->cBytes)		// + the size of the byte count
		+ sizeof(pEvtParms[i]->parmType);	// + the sizeof the parameter type

	*pData_ = new char[cb];  // allocate the blob.  Cast away const is OK, since doing so doesn't change the real object state at all

	char *p = (char *)*pData_;

	memcpy(p, &cb, sizeof(cb));	// total bytes
	
	p += sizeof(cb);	
	memcpy(p, &ed, sizeof(ed));	// Event data

	p += sizeof(ed);
	for (i=0; i < ed.cParms; ++i)
	{
		memcpy(p, &pEvtParms[i]->cBytes, sizeof(pEvtParms[i]->cBytes));
		p += sizeof(pEvtParms[i]->cBytes);
		memcpy(p, &pEvtParms[i]->parmType, sizeof(pEvtParms[i]->parmType));
		p += sizeof(pEvtParms[i]->parmType);
		memcpy(p, pEvtParms[i]->pData, pEvtParms[i]->cBytes);
		p += pEvtParms[i]->cBytes;
	}
	return cb;

}


	// append a parameter to the parameter array
void Event::Append(EventParm *pParm_)
{
	
	if (++ed.cParms % ARRAY_GROW_SIZE == 0)
	{
		EventParm **pTemp = pEvtParms;
		pEvtParms = new EventParm *[ed.cParms + ARRAY_GROW_SIZE];
		for (int i = 0; i < ed.cParms; ++i)
			pEvtParms[i] = pTemp[i];
		delete pTemp;
	}
	pEvtParms[ed.cParms - 1] = pParm_;
}



void Event::AddEventParameter(const HexData *hd)
{
	EventParm *pParm = new EventParm;
	pParm->cBytes = hd->cBytes;
	pParm->parmType = HEX_PARM;
	pParm->pData = new char[hd->cBytes];  
	memcpy(pParm->pData, hd->pData, hd->cBytes);
	Append(pParm);
}

void Event::AddEventParameter(const UnicodeString &us)
{
	EventParm *pParm = new EventParm;
	pParm->cBytes = us.GetSize();
	pParm->parmType = USTR_PARM;
	pParm->pData = new char[pParm->cBytes];
	((UnicodeString &)us).CString(pParm->pData, pParm->cBytes);
	Append(pParm);
}

void Event::AddEventParameter(const wchar_t *pw)
{
	AddEventParameter(UnicodeString(pw));
}

