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
// This class is a test ISM to echo SCSI_SCB messages to the Initiator. 
// 
// Update Log:
//	$Log: /Gemini/Odyssey/IsmEchoScsi/EchoScsiIsm.h $
// 
// 4     7/28/99 7:58p Mpanas
// changes to support new EchoScsi config
// 
// 10/2/98 Michael G. Panas: Create file
// 02/12/99 Michael G. Panas: convert to new Oos model
/*************************************************************************/

#ifndef __EchoScsiIsm_h
#define __EchoScsiIsm_h

#include "Nucleus.h"
#include "Ddm.h"
#include "EsConfig.h"


class EchoScsiIsm: public Ddm {

	ES_CONFIG	config;
	
	VDN			myVd;
public:

	EchoScsiIsm(DID did);

	static
	Ddm *Ctor(DID did);

	STATUS Initialize(Message *pMsg);
	STATUS Enable(Message *pMsg);
	STATUS Quiesce(Message *pMsg);
	
protected:

	STATUS DoWork(Message *pMsg);
	
};
#endif