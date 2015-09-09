/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// Consol.h
//
// Description:
// This is DdmConsoleHBC file.  

//	Include this file in your project settings under "C/C++ Language" in
//  the "Prefix File" box.
// 
// Update Log: 
//	$Log: /Gemini/Odyssey/Hbc/DdmConsoleHbc.h $
// 
// 5     2/11/00 2:05p Rbraun
// Removed IPx manipulators
// 
// 4     1/26/00 2:52p Joehler
// Debugged HBC Console.
// 
// 3     12/09/99 3:57p Joehler
// Completion on HbcConsole
// 
// 2     11/30/99 2:43p Jlane
// Prelimiminary changes to get working.
// 
// 1     11/22/99 4:38p Egroff
// Initial checkin.
//
/*************************************************************************/
#ifndef __DdmConsoleHbc_h

#define __DdmConsoleHbc_h
#include "Ddm.h"
#include "SystemConfigSettingsTable.h"
#include "Network.h"

#define COLUMN_INIT 5
#define DELAY 100

void HbcConsoleHandleKeys(void* pParam);

class DdmConsoleHbc : public Ddm {

public:
	static Ddm *Ctor(DID did) 	{ return new DdmConsoleHbc(did); }

	DdmConsoleHbc(DID did);

	STATUS Initialize(Message *pArgMsg);
	STATUS Enable(Message *pArgMsg);

	STATUS DisplayMenu();
	void SetIPAddress();
	void SetSubnetMask();
	void SetDefaultGateway();
	
private:

	STATUS SystemConfigSettingsTableInitialized(Message* pReply_);
	STATUS SystemConfigSettingsTableEnumed(Message* pReply_);
	STATUS SystemConfigSettingsRowInserted(Message* pReply_);
	void ListenOnSysConfigTable();
	STATUS SystemConfigSettingsListenReply(Message* pReply_);
	STATUS SystemConfigSettingTableModified(Message* pReply_);
#ifdef false
	STATUS PPPModeDisabled(Message* pReply);
#endif

private:

	CT_Task* handleKeysTask;
	void	*handleKeysStack;

	Message* pInitMsg;	
	SystemConfigSettingsRecord *m_pSystemConfigRecord;
	U8	IP_address[4];
	U8	subnet_mask[4];	
	U8	default_gateway[4];
	U32 line;
	U32 column;
};

#endif	// __DdmConsoleHbc_h