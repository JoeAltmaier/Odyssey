/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: Utilq.h
//
// Description:	Header file for the queueing class for utility Reqblks
//
// Update Log: 
//	2/99	Jim Taylor:	initial creation
//
//
/*************************************************************************/

#ifndef __Utilq_h
#define __Utilq_h

#define	MAX_UTIL_PRIORITY	10

class UtilQueue : public ReqQueue
{

public:

	UtilQueue(U8 QMethod) : ReqQueue(NULL, QMethod) {}

	void	UpdateUtilPriority();
	void	CheckUtilPriorityToRun();
	void	CheckUtilToRun(Raid *pRaid);
	BOOL	StartUtility(UtilReqblk *pUtilReq);

};

#endif
