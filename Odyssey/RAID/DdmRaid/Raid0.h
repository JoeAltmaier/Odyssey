/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: Raid0.h
//
// Description:	Header file for Raid0 class
//
// Update Log: 
//	2/99	Jim Taylor:	initial creation
//
//
/*************************************************************************/

#ifndef __Raid0_h
#define __Raid0_h

class Raid0: public Raid
{

	U32	   	DataBlockSize;

public:
	Raid0() : Raid() {}
	~Raid0();
	STATUS	Initialize(DdmServices *pDdmServices,
								RAID_ARRAY_DESCRIPTOR *pArrayDesc, Ioreq *pIoreq);
	void	ReadMemberTableDone(RAID_ROW_RECORD *pRaidRow, STATUS Status);
	STATUS	DoRead(Ioreq *pIoreq);
	STATUS	DoWrite(Ioreq *pIoreq);
	STATUS	DoReassign(Ioreq *pIoreq);
	void	StartIoreq(Ioreq *pIoreq);
	void	StartIO();
	void	ReqIODone(Reqblk *pReq);
	BOOL	IsRaidUsable(U32 Failmask);
	U32		ConvertReqLbaToIoreqLba(Reqblk *pReq);
	void	RequeueRequestblk(Reqblk *pReq);
	void	ClearQueuesWithError();
	BOOL	NormalIO(Reqblk *pReq);
	void	NormalIOCallback(MessageReply *pMsg);
	BOOL	ReassignBlock(Reqblk *pReq);
	void	ReassignBlockCallback(MessageReply *pMsg);
	BOOL	RetryCommand(Member *pMem, MessageReply *pMsg, ReplyCallback rc);
};

struct RAID0_CONTEXT
{
	Reqblk 			*pReq;
	BSA_RW_PAYLOAD	payload;
	U8				RetryCount;
};

#endif

