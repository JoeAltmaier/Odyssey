/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: DdmSSD.h
// 
// Description:
// This file defines the Device Dependent Module class for the
// Solid State Drive. 
// 
// Update Log 
// 
// 02/25/99 Jim Frandeen: Create file
/*************************************************************************/

#ifndef DdmSCC_H
#define DdmSCC_H

#include "Simple.h"
#include "Message.h"
#include "Ddm.h"

/*************************************************************************/
// SSD_Ddm
// This is the class description for the Solid State Disk
// Device Dependent Module.
/*************************************************************************/
class SSD_Ddm: public Ddm {

public: // Methods

	// constructor
	SSD_Ddm(DID did);

	// Virtual methods derived from Ddm.
	virtual STATUS Initialize(Message *pMsg);
	virtual STATUS Enable(Message *pMsg);

	static Ddm *Ctor(DID did);
	
private: // member data

	struct 
	{
		VDN vd;
	} 					m_config;
	
			
}; // SSD_Ddm

#endif // DdmSCC_H
