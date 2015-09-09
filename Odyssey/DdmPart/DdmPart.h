/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: DdmPart.h
//
// Description:	Header file for Partition DDM class
//
// Update Log: 
//	5/99	Jim Taylor:	initial creation
//
//
/*************************************************************************/

#ifndef __DdmPart_h
#define __DdmPart_h


class DdmPart: public Ddm
{
	PARTITION_DESCRIPTOR	config;

public:

	DdmPart(DID did);
	~DdmPart(){};

	static
	Ddm		*Ctor(DID did);

	STATUS	Initialize(Message *pMsg);
	STATUS	Quiesce(Message *pMsg);
	STATUS	Enable(Message *pMsg);
	STATUS	DoIO(Message *pMsg);
	STATUS	DispatchDefault(Message *pMsg);
	void	IODone(MessageReply *pMsg);
};

union	CNVTR
{
	U8	charval[4];
	U32	ulngval;
};

#endif
