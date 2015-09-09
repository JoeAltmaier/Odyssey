/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: UtilCmd.h
//
// Description:
//
// Update Log: 
//	2/99	Jim Taylor:	initial creation
//
//
/*************************************************************************/

#ifndef __UtilCmd_h
#define __UtilCmd_h

#define	RAID_ABORT_UTILITY		1
#define	RAID_SUSPEND_UTILITY	2

class UtilCmd
{

private:

	Utility		*pHead;
	Utility		*pTail;
	U32			Count;

public:

	UtilCmd();

	U32			GetCount();
	BOOL		IsEmpty();
	void		InsertInQueue(Utility *pUtility);
	void		RemoveFromQueue(Utility *pUtility);
	Utility		*FindUtilityInQueue(rowID Handle);
	void		SuspendUtilities();

};


/*************************************************************************/
// GetCount
// Returns number of Utilities in queue
/*************************************************************************/

inline U32	UtilCmd::GetCount()
{
	return (Count);
}

/*************************************************************************/
// IsEmpty
// Returns TRUE if Queue is empty
/*************************************************************************/

inline BOOL	UtilCmd::IsEmpty()
{
	return (pHead == NULL);
}

#endif
