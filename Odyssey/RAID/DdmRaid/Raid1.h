/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: Raid1.h
//
// Description:	Header file for Raid1 class
//
// Update Log: 
//	2/99	Jim Taylor:	initial creation
//
//
/*************************************************************************/

#ifndef __Raid1_h
#define __Raid1_h

struct R1_MSTR_UTIL_CONTEXT;

class Raid1: public Raid
{

	U32			DataBlockSize;
	U32			WaitMask;
	ReqQueue	*pRequestQueue;
	RaidLock	*pRaidLock;

public:
	Raid1() : Raid() {}
	~Raid1();
	STATUS	Initialize(DdmServices *pDdmServices,
								RAID_ARRAY_DESCRIPTOR *pArrayDesc, Ioreq *pIoreq);
	void	ReadMemberTableDone(RAID_ROW_RECORD *pRaidRow, STATUS Status);
	STATUS	CheckUtilitiesToStart(Ioreq *pIoreq);
	void	CheckUtilitiesCallback(RAID_ROW_RECORD *pRaidRow, STATUS Status);
	STATUS	DoRead(Ioreq *pIoreq);
	STATUS	DoWrite(Ioreq *pIoreq);
	STATUS	DoReassign(Ioreq *pIoreq);
	STATUS	DoVerify(Utility *pUtility);
	STATUS	DoRegenerate(Utility *pUtility);
	STATUS	DoBkgdInit(Utility *pUtility);
	STATUS	DoReplaceMember(RaidRequest *pRaidRequest);
	STATUS	DoAddMember(RaidRequest *pRaidRequest);
	STATUS	StartUtility(Utility *pUtility, U32 Count, U8 NumConcurrent);
	void	ContinueUtility(UtilReqblk *pUtilReq);
	void	EndUtility(Utility *pUtility);
	void	EndUtilityCallback(Utility *pUtility, STATUS Status);
	void	StartIoreq(Ioreq *pIoreq);
	void	StartIO();
	void	ReqIODone(Reqblk *pReq);
	BOOL	IsRaidUsable(U32 Failmask);
	U32		ConvertReqLbaToIoreqLba(Reqblk *pReq);
	BOOL	CanUtilityRun(UtilReqblk *pUtilReq);
	void	RequeueRequestblk(Reqblk *pReq);
	void	ClearQueuesWithError();
	BOOL	MirroredWrite(Reqblk *pReq);
	void	MirroredWriteCallback(MessageReply *pMsg);
	BOOL	MirroredRead(Reqblk *pReq);
	void	MirroredReadCallback(MessageReply *pMsg);
	BOOL	ReassignBlock(Reqblk *pReq);
	void	ReassignBlockCallback(MessageReply *pMsg);
	BOOL	Verify(UtilReqblk *pUtilReq);
	void	VerifyCallback(MessageReply *pMsg);
	void	VerifyCorrect(R1_MSTR_UTIL_CONTEXT *pMasterContext, UtilReqblk *pUtilReq, U8 Mem);
	void	VerifyCorrectDone(MessageReply *pMsg);
	void	VerifyPrimaryDone(MessageReply *pMsg);
	void	VerifyOtherReadDone(MessageReply *pMsg);
	void	VerifyDone(MessageReply *pMsg);
	BOOL	Regenerate(UtilReqblk *pUtilReq);
	void	RegenerateReadDone(MessageReply *pMsg);
	void	RegenerateWriteDone(MessageReply *pMsg);
	void	UpdateRegeneratedLba(UtilReqblk *pUtilReq);
	BOOL	CompareBuffers(I64 *pBuf1, I64 *pBuf2, U32 Count);
	BOOL	RetryCommand(Member *pMem, MessageReply *pMsg, ReplyCallback rc);
	BOOL	ReadMDerrBlock(ErrReqblk *pErrReq);
};


struct R1_MSTR_UTIL_CONTEXT
{
	U8			State;
	U8			PrimaryDone;
	U8			Primary;
	U32			NeedsCompareMask;
	U8			*pBuffer[MAX_ARRAY_MEMBERS];
};

struct RAID1_CONTEXT
{
	R1_MSTR_UTIL_CONTEXT	*pMstrContext;
	Reqblk 					*pReq;
	BSA_RW_PAYLOAD			payload;
	U32						MemberMask;
	U8						Member;
	U8						RetryCount;
};

#endif
