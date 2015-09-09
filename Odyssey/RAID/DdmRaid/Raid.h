/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: Raid.h
//
// Description:	Raid Base class
//
//
// Update Log: 
//	2/99	Jim Taylor:	initial creation
//
//
/*************************************************************************/

#ifndef __RaidBase_h
#define __RaidBase_h

#include "RaidPerf.h"
#include "RaidStat.h"
#include "UtilQ.h"
#include "ArrayDescriptor.h"
#include "RAIDMemberTable.h"
#include "RAIDUtilTable.h"
#include "PtsCommon.h"
#include "Rows.h"
#include "Fields.h"
#include "CmdServer.h"
#include "RaidCommands.h"
#include "String.h"
#include "FcpMessageFormats.h"
#include "FcpMessageStatus.h"

#define	TRACE_INDEX		TRACE_RAID
#include "Odyssey_Trace.h"




#define	RAID_PROCESSING_ERROR	0x00000001


// context for Status Check and Power Management

struct RAID_MSTR_STATUS
{
	Ioreq		*pIoreq;
	U32			Failmask;
	U8			State;
};

struct RAID_STATUS_CONTEXT
{
	RAID_MSTR_STATUS	*pMaster;
	U32					MemberMask;
};

// context for commands called by Error Processing

struct RAID_CONTEXT
{
	ErrReqblk 	  			*pErrReq;
	BSA_RW_PAYLOAD			payload;
};

// used to read table rows

struct	RAID_ROW_RECORD
{
	Ioreq		*pInitIor;
	U8			Index;
	union
	{
		RAID_ARRAY_MEMBER		MemberData;
		RAID_ARRAY_UTILITY		UtilityData;
		RAID_ARRAY_DESCRIPTOR	ArrayData;
	} RowData;
};

class Raid: public DdmServices
{

	friend class RaidErr;

protected:

	U8						Health;
	U8			  		 	NumberMembers;
	VDN						RaidVDN;
	rowID					RaidRowId;
	Member 					*pMember[MAX_ARRAY_MEMBERS];

	UtilCmd					UtilCmd;
	IoreqQueue				IoreqQueue;
	UtilQueue				*pUtilQueue;
	RAID_STATUS				RaidStatus;
	RAID_PERFORMANCE		RaidPerformance;
	RAID_ARRAY_DESCRIPTOR	*pArrayDescriptor;
	RaidErr					*pRaidErr;
	Ioreq					*pQuiesceIor;
	CmdServer				*pCmdServer;

public:
	U32						Flags;

	Raid();
	
	// All derived classes must implement these pure virtual methods
	virtual STATUS	Initialize(DdmServices *pDdmServices,
								RAID_ARRAY_DESCRIPTOR *pArrayDesc, Ioreq *pIoreq) = 0;
	virtual STATUS	DoRead(Ioreq *pIoreq) = 0;
	virtual STATUS	DoWrite(Ioreq *pIoreq) = 0;
	virtual STATUS	DoReassign(Ioreq *pIoreq) = 0;

	// used by Raid Err class
	virtual BOOL	IsRaidUsable(U32 Failmask) = 0;
	virtual U32		ConvertReqLbaToIoreqLba(Reqblk *pReq) = 0;
	virtual void	RequeueRequestblk(Reqblk *pReq) = 0;
	virtual void	ClearQueuesWithError() = 0;

	// All derived classes that support these methods need
	// to implement them. Defaults return error
	virtual STATUS	DoVerify(Utility *pUtility); 	
	virtual STATUS	DoRegenerate(Utility *pUtility);
	virtual STATUS	DoBkgdInit(Utility *pUtility);
	virtual STATUS	DoExpand(Utility *pUtility);
	virtual STATUS	DoReplaceMember(RaidRequest *pRaidRequest);
	virtual STATUS	DoAddMember(RaidRequest *pRaidRequest);


	// These methods implemented in base raid class. May be
	// re-implemented in derived class if desired.
	virtual STATUS	AbortUtility(rowID Handle);
	virtual STATUS	SetUtilityPriority(rowID Handle, RAID_UTIL_PRIORITY Priority);
	virtual void	SuspendAllUtilities();

	STATUS			RaidDownMember(RaidRequest *pRaidRequest);
	void			ReadMemberTableRow(RAID_ROW_RECORD *pRaidRow, pTSCallback_t pCallback);
	void			ReadUtilityTableRow(RAID_ROW_RECORD *pRaidRow, pTSCallback_t pCallback);
	void			ReadArrayTableRow(RAID_ROW_RECORD *pRaidRow, pTSCallback_t pCallback);
	void			UpdateUtilityStatusInPTS(Utility *pUtility, pTSCallback_t pCallback);
	void			UpdateUtilityProgressInPTS(Utility *pUtility, pTSCallback_t pCallback);
	void			UpdateUtilityDone(Utility *pUtility, STATUS Status);
	STATUS			SetMemberHealthDownInPTS(ErrReqblk *pErrReq, U32 Mask);
	void			SetMemberHealthDownCallback(ErrReqblk *pErrReq, STATUS Status);
	STATUS			StartUtility(RAID_ARRAY_UTILITY *pArrayUtility);
	STATUS			DoStatusCheck(Ioreq *pIoreq);
	STATUS			DoPowerManagement(Ioreq *pIoreq);
	void			StatusCallback(MessageReply *pMsg);

	STATUS			InitCmdServer();
	void			CmdServerInited(STATUS Status);
	void			RaidCommand(HANDLE Handle, RaidRequest *pRaidRequest);

	// Methods for Performance and Status Records
	void			ResetStatus();
	void			ReturnStatus(U8 *ptr);
	void			ResetPerformance();
	void			ReturnPerformance(U8 *ptr);
	void			UpdateSGCombined(U8 Type, U8 Num);
	void 			UpdatePerfReads(U32 NumBlocks);
	void 			UpdatePerfWrites(U32 NumBlocks);
	void		 	UpdateRaidReassignments(STATUS Status);
	void		 	UpdateMemberReassignments(STATUS Status, U8 Mem);
	void	 		UpdateNumRecoveredErrors(U8 Mem);

	void			Quiesce(Ioreq *pIoreq);
	void			CheckForQuiesced();
	U32				GetWaitMask();
	BOOL			IsRaidIdle();
	BOOL			IsRaidOffline();
	void			SetMemberHealth(U32 Mask, RAID_MEMBER_STATUS Health);

protected:

	STATUS			GetReqblkSGList(Ioreq *pIoreq, Reqblk *pReq);
	void			FreeUtilReqblkChain(Utility *pUtility);
	BOOL			RemoveFromUtilityChain(UtilReqblk *pUtilReq);
	void			FreeReqblkChain(Ioreq *pIoreq);
	void			FreeReqblk(Reqblk *pReq);
	BOOL			RemoveFromIorChain(Reqblk *pReq);
	void			ReqblkDone(Reqblk *pReq);
	void			CommandComplete(Reqblk *pReq);
	void			SetIoreqStatus(Reqblk *pReq);
	void			SetReqStatus(Reqblk *pReq, MessageReply *pMsg, U8 Mem);
	BOOL			ReadMDerrBlock(ErrReqblk *pErrReq);
	void			ReadMDerrCallback(MessageReply *pMsg);
	BOOL			ErrReassignBlock(ErrReqblk *pErrReq);
	void			ErrReassignBlockCallback(MessageReply *pMsg);
	BOOL			WriteVerify(ErrReqblk *pErrReq);
	void			WriteVerifyCallback(MessageReply *pMsg);
	BOOL			RetryCommand(Member *pMem, MessageReply *pMsg, ReplyCallback rc);
};

typedef BOOL (Raid::*pReqStartMethod)( Reqblk* );
typedef void (Raid::*pReqCallbackMethod)( Reqblk* );

class DdmRaid;

typedef void (DdmRaid::*pUtilityCallbackMethod)( Utility* );

inline BOOL	Raid::IsRaidOffline()
{
	return (Health == RAID_OFFLINE);
}

inline void	Raid::ResetStatus()
{
	memset(&RaidStatus, 0, sizeof(RAID_STATUS));
}

inline void	Raid::ReturnStatus(U8 *ptr)
{
	memcpy(ptr, &RaidStatus, sizeof(RAID_STATUS));
}

inline void	Raid::ResetPerformance()
{
	memset(&RaidPerformance, 0, sizeof(RAID_PERFORMANCE));
}

inline void	Raid::ReturnPerformance(U8 *ptr)
{
	memcpy(ptr, &RaidPerformance, sizeof(RAID_PERFORMANCE));
}


#endif
