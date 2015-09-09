/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// (c) Copyright 1999 ConvergeNet Technologies, Inc.
//     All Rights Reserved.
//
// File: DdmLED.h
// 
// Description:
// This file is the DdmLED module interface. 
// 
// $Log: /Gemini/Odyssey/DdmLED/DdmLED.h $
// 
// 5     11/06/99 2:14p Jlane
// Multiple changes to clean this whole thing up.
// 
// 4     10/28/99 10:18a Sgavarre
// Changes to support BootMgr.
// 
// 3     10/14/99 4:47p Jlane
// Multiple fixes to get working.
// 
// 2     8/30/99 9:11p Iowa
// Add ToggleAllLEDs so Enable doesn't reply twice.
// 
// 1     8/20/99 6:21p Hdo
// First check in
// 
// 07/13/99 Huy Do: Create file
/*************************************************************************/

#if !defined(DdmLED_H)
#define DdmLED_H

#include "Ddm.h"

class DdmLED : public Ddm {
public:
	DdmLED(DID did) ;
	static Ddm *Ctor(DID did);

	STATUS Initialize(Message *pMsg);
	STATUS Enable(Message *pMsg);
	STATUS Quiesce(Message *pMsg);

	STATUS TurnRed(Message *pMsg);
	STATUS TurnGreen(Message *pMsg);
	STATUS TurnAmber(Message *pMsg);

	STATUS ToggleLEDs(Message *pMsg);
	STATUS ToggleAllLEDs();
	STATUS TurnLEDsOFF(Message *pMsg);
	
private:
	// 
	U32		m_fLEDToggleState;
	U32		m_LED_Color;
	U32		m_LED_On;
	U32		m_LED_Off;
} ;

#endif /* DdmLED_H  */
