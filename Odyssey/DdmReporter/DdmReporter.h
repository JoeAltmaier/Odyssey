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
// This class is a null device HDM.
// 
// Update Log: 
//
// $Log: /Gemini/Odyssey/DdmReporter/DdmReporter.h $
// 
// 11    8/23/99 4:51p Vnguyen
// Separate quiesce and pause states as it is possible for the two
// states to overlap.  Quiesce is global to all reporters.  Pause is local
// to one reporter.
// 
// 10    8/16/99 12:59p Vnguyen
// Fix various compiler errors.  Mostly typo and mis-match parameters.
// 
// 9     8/05/99 10:52a Vnguyen
// 9	8/04/99	4:05P	Vnguyen
// Add a few defines for default Refresh and Sample rates.
// 8	 8/03/99 1:55p 	Vnguyen 
// Delete unused member variables.  Add methods ProcessStartRequest and
// ProcessRequest.
// 7     5/07/99 11:14a Jlane
// Added VerifySRCTableExists to create the StorageRollCallTable if
// necessary.
// 
// 6     5/05/99 4:16p Jlane
// Supply Listen with sizeof returned record.
// 
// 5     5/05/99 4:08p Jlane
// Miscellaneous integration changes.
//
// 8/17/98 Joe Altmaier: Create file DdmNull.h
// 10/05/98	JFL	Created DdmReporter.cpp/h from DdmNull.cpp/h.  Thanks Joe!
//
/*************************************************************************/

#ifndef __DdmReporter_h
#define __DdmReporter_h

#include "Ddm.h"
#include "SglLinkedList.h"



class DdmReporter: public Ddm {
public:
		DdmReporter(DID did);
virtual	~DdmReporter();

static	Ddm *Ctor(DID did);
		STATUS Initialize(Message *pmsg);
		STATUS Enable(Message *pmsg);
		STATUS Quiesce(Message *pmsg);

static 	DdmReporter*	m_pDdmReporter;		// my this pointer.


		STATUS ProcessStartRequest(Message *pArgMsg);
		STATUS ProcessRequest(Message *pArgMsg);


protected:

//
// Member Variables
//

public:

protected:

SglLinkedList		m_Reporters;		// our list of reporter objects.
};

#endif // __DdmReporter_h
