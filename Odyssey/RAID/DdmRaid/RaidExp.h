/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: RaidExp.h
//
// Description:	Header file for Raid Expansion class
//
// Update Log: 
//	2/99	Jim Taylor:	initial creation
//
//
/*************************************************************************/

#ifndef __RaidExp_h
#define __RaidExp_h

class RaidExp: public Raid
{

public:

	U32			ExpansionLba;

	Raid		*pRaidOld;
	Raid		*pRaidNew;
	RaidLock	*pRaidLock;

	RaidExp() : Raid() {};
	~RaidExp();
	STATUS		Initialize(DdmServices *pDdmServices,
								RAID_ARRAY_DESCRIPTOR *pArrayDesc, Ioreq *pIoreq);
	void		ReadArrayTableDone(RAID_ROW_RECORD *pRaidRow, STATUS Status);
	void		RaidInitializeDone(Ioreq *pIoreq);
	STATUS		ReadUtilityTable(Ioreq *pIoreq);
	void		ReadUtilityTableCallback(RAID_ROW_RECORD *pRaidRow, STATUS Status);
	STATUS		DoRead(Ioreq *pIoreq);
	STATUS		DoWrite(Ioreq *pIoreq);
	STATUS		DoReassign(Ioreq *pIoreq);
	void		ReadRangeLocked(Ioreq *pIoreq);
	void		WriteRangeLocked(Ioreq *pIoreq);
	void		ReassignRangeLocked(Ioreq *pIoreq);
	void		RequestDone(Ioreq *pIoreq);
	STATUS		DoExpand(Utility *pUtility);
	BOOL		IsRaidUsable(U32 Failmask);
	void		SuspendAllUtilities();
};

#endif
