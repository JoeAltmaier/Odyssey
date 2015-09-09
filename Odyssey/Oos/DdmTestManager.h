/* DdmTestManager.h -- Manages keeping track of and launching test DDMs
 *
 * Copyright (C) ConvergeNet Technologies, 1999
 *
 * This material is a confidential trade secret and proprietary 
 * information of ConvergeNet Technologies, Inc. which may not be 
 * reproduced, used, sold or transferred to any third party without the 
 * prior written consent of ConvergeNet Technologies, Inc.  This material 
 * is also copyrighted as an unpublished work under sections 104 and 408 
 * of Title 17 of the United States Code.  Law prohibits unauthorized 
 * use, copying or reproduction.
 *
**/

// Revision History: 
//  12/10/99 Ryan Braun: Create file
// ** Log at end-of-file **

#ifndef __DdmTestManager_h
#define __DdmTestManager_h

#include "Ddm.h"
#include "TestTable.h"

class Rule {

public:
	Rule() {
		pos = 0;
		type = 0;
	}

	int pos;
	int type;
};

class DdmTestManager : public Ddm {

public:
	static Ddm *Ctor(DID did);
	DdmTestManager(DID did);

	STATUS Initialize(Message *pMsg);
	STATUS Enable(Message *pMsg);	
	STATUS Quiesce(Message *pMsg);

	//message handlers
	STATUS ProcessRegistration(Message *pMsgReq);
	STATUS ProcessRun(Message *pMsgReq);
	
	STATUS InputStream(Message *pMsgReq);
	
private:
	int getWord(char *stringfull, int position, char *word);
	int getQuotedString(char *stringfull, int position, char *word);
	int getIPFromString(char *stringfull, int position, char *ipout);
	bool isDigit(char ch);
	STATUS ParseString(char *inStr, char *ruleStr, char *bitpack);
    	
};

int atoi (char *s);

#endif // __DdmTestManager_h

//**************************************************************************************************
// Update Log:
//	$Log: /Gemini/Odyssey/Oos/DdmTestManager.h $
// 
// 1     2/11/00 1:23p Rbraun
// Initial checking of Neptune test subsystem files
// 
