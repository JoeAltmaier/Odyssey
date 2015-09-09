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
//	$Log: /Gemini/Odyssey/Nac/DdmConsoleTest.h $
// 
// 1     7/21/99 10:12p Mpanas
// First pass at a NAC Image
// 
// 1     7/15/99 11:58p Mpanas
// Changes to support Multiple FC Instances,
// and New DdmConsoleTest
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
