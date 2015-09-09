/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// Description:
// This file is header file for the Persistant Table Service class of DDM.
// 
// Update Log: 
// 8/17/98 Joe Altmaier: Create file DdmNull.h
// 10/07/98	JFL	Created from DdmNull.cpp/h.  Thanks Joe!
// 12/01/98 sg	added variable for did
// 02/10/99 jl: Change ctor and constructor to take DID as the Parameter.
// 07/30/99 sg: add separate methods for requests
// 09/15/99	sg: add InsertVLRow
//
/*************************************************************************/

#ifndef __DdmPTS_h
#define __DdmPTS_h

#include "Ddm.h"

#ifdef	PERSIST
#include "chaosfile.h"
#endif

//
// DdmPTS - The class definition of the Persistant Table Service DDM.
//

class DdmPTS: public Ddm
{

public:

	DdmPTS(DID did);

	static Ddm *Ctor(DID did);

	virtual	STATUS Initialize(Message *pMsg);
	virtual STATUS Quiesce (Message *pMsg);
	virtual STATUS Enable (Message *pMsg);

	static	VDN				vdPTS;		// our virtual device number
	static 	DdmPTS*			pDdmPTS;	// my this pointer.
	static	DID				didPTS;		// our did
	void	SendListenReplies();
	STATUS	FlushToFlash();
	
private:
#ifdef PERSIST
	ChaosFile *pcfTableHdrs;
	ChaosFile *pcfTableData1;
#endif

	STATUS	CreatePartitions();
	STATUS	CleanupNonPersistData();
	STATUS	LoadTableHeaders();
	STATUS	LoadTableData();

	
protected:
	STATUS PtsDefineTable(Message *pMsg);
	STATUS PtsInsertRow(Message *pMsg);
	STATUS PtsInsertVarLenRow(Message *pMsg);
	STATUS PtsEnumTable(Message *pMsg);
	STATUS PtsQuerySetRID(Message *pMsg);
	STATUS PtsGetTableDef(Message *pMsg);
	STATUS PtsDeleteTable(Message *pMsg);
	STATUS PtsReadRow(Message *pMsg);
	STATUS PtsReadVarLenRow(Message *pMsg);
	STATUS PtsModifyRow(Message *pMsg);
	STATUS PtsModifyField(Message *pMsg);
	STATUS PtsModifyBits(Message *pMsg);
	STATUS PtsTestSetField(Message *pMsg);
	STATUS PtsDeleteRow(Message *pMsg);
	STATUS PtsListen(Message *pMsg);
	STATUS PtsStopListen(Message *pMsg);
};
#endif