/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: DdmConsoleTest_H.h
// 
// Description:
//	This is the class definition of the Console / Test DDM
// 
// Update Log:
//	$Log: /Gemini/Odyssey/HscsiEval/DdmConsoleTest.h $
// 
// 1     10/11/99 5:32p Cchan
// Project for HSCSI library (QL1040B) to run on Eval Board
// 
// 1     7/15/99 11:33p Mpanas
// Changes to support Multiple FC Instances
// Update to latest Key handler
// Also include new DdmConsoleTest
// Change all DriveMonitor external refs to Message
// based commands
//
// 07/07/99 Michael G. Panas: Create file
/*************************************************************************/

#if !defined(DdmConsoleTest_H)
#define DdmConsoleTest_H

#include "Nucleus.h"
#include "Ddm.h"
#include "RqOsDdmManager.h"

//
//  DdmConsoleTest Class
//
class DdmConsoleTest : public Ddm {

public:
	static Ddm *Ctor(DID did) 	{ return new DdmConsoleTest(did); }

	DdmConsoleTest(DID did);

	ERC Initialize(Message *pArgMsg);
	ERC Enable(Message *pArgMsg);

protected:
	STATUS DoWork(Message *pMsg);

};


#endif /* DdmConsoleTest_H  */
