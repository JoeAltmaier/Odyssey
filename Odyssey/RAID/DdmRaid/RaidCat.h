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
// Description:	Header file for Raid Concatenation class
//
// Update Log: 
//	8/99	Jim Taylor:	initial creation
//
//
/*************************************************************************/

#ifndef __RaidCat_h
#define __RaidCat_h

class RaidCat: public Raid0
{

public:
	RaidCat() : Raid0() {}
	STATUS	DoRead(Ioreq *pIoreq);
	STATUS	DoWrite(Ioreq *pIoreq);
	STATUS	DoReassign(Ioreq *pIoreq);
	U32		ConvertReqLbaToIoreqLba(Reqblk *pReq);
};

#endif

