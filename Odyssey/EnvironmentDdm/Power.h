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
// File: Power.h
// 
// Description:
// 	This file is interface for the Power module. 
// 
// $Log: /Gemini/Odyssey/ExceptionMaster/Power.h $
// 
// 2     9/08/99 8:14p Hdo
// Integration with AlarmMaster
// 
// 1     8/25/99 1:55p Hdo
// First check in
// 
// 8/9/99 Huy Do: Create file
/*************************************************************************/

#ifndef __Power_H__
#define __Power_H__

#include "AbstractCmd.h"

class Power : public AbstractCmd {
public:
	Power(U32 voltage) : Voltage(voltage) {};
	~Power();

	virtual STATUS performTask(XM_CONTEXT	*pXMContext);
	U32		GetVoltage() const { return Voltage; }

private:
	U32		Voltage;
} ;

#endif // __Power_H__
