/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: DdmRaid.h
//
// Description:	Header file for Raid DDM class
//
// Update Log: 
//	2/99	Jim Taylor:	initial creation
//
//
/*************************************************************************/

#ifndef __DdmRaid_h
#define __DdmRaid_h

// temp
typedef struct
{
	VDN						memberVD;		// VD of member -- 
	RAID_MEMBER_STATUS		memberHealth;
	U32						memberIndex;	// relative to this array
	U32						startLBA; 		// where host data starts
	U32						endLBA;			// where host data ends
}	TMP_RAID_ARRAY_MEMBER;

typedef struct
{
	VDN					arrayVDN;
	U32					totalCapacity;		// in 512 byte blks - depends on RAID Level
	U32					memberCapacity;		// in 512 byte blks - smallest member cap.
	U32					dataBlockSize;		// in 512 byte blks (default = 64)
	U32					parityBlockSize;	// in 512 byte blks	(default = DataBlockSize)
											// if parity blk size is different from data blk
											// size, parity size must be a multiple of the 
											// data blk size (matters in RAIDS 3,4,5)
	RAID_LEVEL			raidLevel;			// rsmRAID0, rsmMIRRORED, rsmRAID5
	RAID_ARRAY_STATUS	health;				// OFFLINE,
	RAID_INIT_STATUS	initStatus;			// Initialization Done,
	U32					numberMembers;
	U32					numberUtilities;	// currently running on array
	rowID				utilities[1]; // info for each running util
	rowID		 		members[8]; 
}	TMP_RAID_ARRAY_DESCRIPTOR;

struct TempConfiguration
{
	TMP_RAID_ARRAY_DESCRIPTOR	AD;
	TMP_RAID_ARRAY_MEMBER		MD[32];
};


class DdmRaid: public Ddm
{

	CmdServer	*pCServer;		// temp

	RAID_ARRAY_DESCRIPTOR	ArrayDescriptor;
// temp
TempConfiguration TmpConfig;

	Raid	*pRaid;
	DID		RaidDID;			// save for PHS Reporter

public:

	DdmRaid(DID did);
	~DdmRaid();

	static
	Ddm		*Ctor(DID did);

	STATUS	Initialize(Message *pMsg);
	STATUS	Quiesce(Message *pMsg);
	STATUS	Enable(Message *pMsg);
	void	HandleReporterReply(Message *pMsg);


// temp
STATUS	RealEnable(Message *pMsg);

void	TempCreateConfiguration(Message *pMsg);
void	TempCreateTables(Message *pMsg);
STATUS	DefineMemberTable(Message *pMsg);
void	CreateMemberTableEntries(Message *pMsg);
void	MemberTableDone(RAID_ROW_RECORD *pRaidRow, STATUS Status);
void	CreateArrayTableEntry(Message *pMsg);
void	ArrayTableDone(RAID_ROW_RECORD *pRaidRow, STATUS Status);
void	ReadArrayTableDone(RAID_ROW_RECORD *pRaidRow, STATUS Status);
STATUS	ReadArrayTableRow(RAID_ROW_RECORD *pRaidRow, pTSCallback_t pCallback);



protected:

	void	QuiesceDone(Ioreq *pIoreq);
	void	EnableDone(Ioreq *pIoreq);
	STATUS	BlockRead(Message *pMsg);
	STATUS	BlockWrite(Message *pMsg);
	STATUS	BlockReassign(Message *pMsg);
	STATUS	DispatchDefault(Message *pMsg);
	void	BlockReadDone(Ioreq *pIoreq);
	void	BlockWriteDone(Ioreq *pIoreq);
	void	BlockReassignDone(Ioreq *pIoreq);
	STATUS	StatusCheck(Message *pMsg);
	STATUS	PowerManagement(Message *pMsg);
	void	IODone(Ioreq *pIoreq);
	STATUS	ResetStatus(Message *pMsg);
	STATUS	ReturnStatus(Message *pMsg);
	STATUS	ResetPerformance(Message *pMsg);
	STATUS	ReturnPerformance(Message *pMsg);
	STATUS	ReturnResetPerformance(Message *pMsg);
	void	SetMsgReplyStatus(Message *pMsg, Ioreq *pIoreq);

private:

	STATUS	CheckLBA(U32 Lba, U32 Num);
};


inline STATUS	DdmRaid::CheckLBA(U32 Lba, U32 Num)
{
	if (Lba < ArrayDescriptor.totalCapacity)
	{
		if (Num > (ArrayDescriptor.totalCapacity - Lba))
			return (OS_DETAIL_STATUS_INVALID_PARAMETER);
		return (OS_DETAIL_STATUS_SUCCESS);
	}
	return (OS_DETAIL_STATUS_INVALID_PARAMETER);
}

#endif
