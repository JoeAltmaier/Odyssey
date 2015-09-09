/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: RaidErr.h
//
// Description:	Raid Error Processing class
//
//
// Update Log: 
//	4/99	Jim Taylor:	initial creation
//
//
/*************************************************************************/

#ifndef __RaidErr_h
#define __RaidErr_h

#define RAID_OUT_OF_RESOURCES				1
#define RAID_BLOCK_REASSIGNMENT_FAILURE		2
#define RAID_REASSIGNED_WRITE_FAILURE		3
#define RAID_REASSIGN_READ_FAILURE			4

class RaidErr
{
	Raid		*pRaid;
	Reqblk		*pErrReq;					// ptr to first Req with error
	ErrReqblk	*pTestReq;					// used for error processing
	ReqQueue	*pRequestQueue;				// Q of subsequent Reqs that ended in error
	U8			*pTestBuf;					// ptr to buffer for testing
	U32			Failmask;					// mask of members that have failed
	U32			Lba;
	U8			Suspect;					// member that caused first error

public:

	~RaidErr();
	STATUS		Initialize(Raid *pRaid);
	void		ProcessError(Reqblk *pReq);
	STATUS		ProcessError(U8 Member);

	void		MediaError();
	void		ReadMDerrCallback(ErrReqblk *pTestReq);
	void		ReassignBlockCallback(ErrReqblk *pTestReq);
	void		WriteVerifyCallback(ErrReqblk *pTestReq);
	BOOL		FillFromRequest();
	U32			GetAddressFromSGL(SGLIST *pSGList, U32 offset);
	void		UnrecoverableMediaError(U8 Reason);
	void		ErrorDoneRestart();
	void		ErrorDoneOffline();
	void		SetMemberDown();
	void		MarkMembersDown();
	void		MarkMembersDownCallback(ErrReqblk *pTestReq);
	void		MarkArrayOffline();
	BOOL		IsReqblkInError(Reqblk *pReq);
	void		EndErrReqblk(Reqblk *pReq);
};

#endif
