/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: RaidStructs.h
//
// Description:
//
// Update Log: 
//	2/99	Jim Taylor:	initial creation
//
//
/*************************************************************************/

#ifndef __RaidStructs_h
#define __RaidStructs_h


#include "RaidUtilTable.h"


// Flag bits in Reqblk
#define	MASTER_BIT					0x00000001
#define	WRITE_VERIFY_BIT			0x00000002
#define RAID_WRITE_BIT				0x00000004
#define RAID_UTILITY_BIT			0x00000008
#define RAID_DEFERRED_ERROR_BIT		0x00000010

#define	RAID_READ				1
#define	RAID_WRITE				2
#define	RAID_REASSIGN			3
#define	RAID_STATUS_CHECK  		4
#define	RAID_POWER_MANAGEMENT	5
#define	RAID_QUIESCE			6
#define	RAID_REGENERATE_CMD		0x10
#define	RAID_VERIFY_CMD			0x11
#define	RAID_BKGDINIT_CMD		0x12
#define	RAID_EXPAND_CMD			0x13


struct SGLIST
{
	U32			Address;
	U32			Length;
};


class Raid;
class RaidErr;
class DdmRaid;

struct Reqblk;
class	Ioreq;

typedef void (DdmServices::*pIoreqCallbackMethod)( Ioreq* );
class Ioreq
{

public:

	U8			Type;
	STATUS		Status;
	U32			Flags;
	U32			Lba;
	U32			Count;
	U8			SenseKey;
	U32			ErrorLba;
	U8			NumSGLs;
	SGLIST		*pSGList;
	SGLIST		*pCurSGList;
	Message		*pMessage;
	Ioreq		*pForw;
	Ioreq		*pBack;
	Reqblk		*pReq;
	void		*pContext;
	U8			iCall;
	struct
	{
		DdmServices	*pInst;
		void		(DdmServices::*pCallback)(Ioreq *pIor);
	} Call[2];

	Ioreq(Message *pMsg);
	Ioreq() {};
	~Ioreq() {};
};

struct Reqblk
{
	U8			Type;
	U8			RetryCount;
	STATUS		Status;
	U8			State;
	U8			Member;
	U32			Flags;
	U32			Lba;
	U32			Count;
	U8			ErrorMember;
	U8			SenseKey;
	U32			ErrorLba;
	U8			NumSGLs;
	SGLIST		*pSGList;
	Ioreq		*pIoreq;
	Reqblk		*pForw;
	Reqblk		*pBack;
	Reqblk		*pCombForw;
	Reqblk		*pCombBack;
	Raid		*pInst;
	BOOL		(Raid::*pStartRoutine)(Reqblk *pReq);
	void		(Raid::*pCallback)(Reqblk *pReq);
};

struct	UtilReqblk;

struct	Utility
{
	Utility				*pForw;				// links for chain of all running
	Utility				*pBack;				// utilities
	UtilReqblk			*pUtilReq;
	RAID_UTIL_STATUS	Status;
	rowID				Handle;				// unique handle - is row id of table
	U32					Flags;
	U8					Cmd;
	U8					Priority;
	U32					UpdateRate;			// how often to update progress in PTS
	U32					PassNo;				// used with UpdateRate
	U32					PercentUpdateRate;	// how often to post % complete events
	U32					PercentPassNo;		// used with PercentUpdateRate
	U32					SourceMask;
	U32					DestMask;
	U32					CurrentLBA;
	U32					EndLBA;
	U32					ErrorCount;			// for verify - # of miscompares
	RAID_UTIL_POLICIES	Policy;
};

struct	UtilReqblk : public Reqblk
{
	U8		Priority;
	Utility	*pUtility;
	U32		SourceMask;
	U32		DestMask;
};

struct	ErrReqblk : public Reqblk
{
	RaidErr		*pErrInst;
	void		(RaidErr::*pErrCallback)(ErrReqblk *pErrReq);
};

union	CNVTR
{
	U8	charval[4];
	U32	ulngval;
};

#endif