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
// File: Battery.h
// 
// Description:
// 	This file is interface for the Battery module. 
// 
// $Log: /Gemini/Odyssey/ExceptionMaster/Battery.h $
// 
// 2     9/08/99 8:14p Hdo
// Integration with AlarmMaster
// 
// 1     8/25/99 1:55p Hdo
// First check in
// 
// 8/9/99 Huy Do: Create file
/*************************************************************************/

#ifndef __Battery_H__
#define __Battery_H__

#include "AbstractCmd.h"

class Battery : public AbstractCmd {
public:
	Battery(U8 number, SENSORCODE sensorError) : BatteryNumber(number), Code(sensorError) {}
	~Battery();

	virtual STATUS performTask(XM_CONTEXT	*pXMContext);
	SENSORCODE GetSensorCode() { return Code; }
	//FailureCode GetFailureCode() { return failCode;}

private:
	U32			BatteryNumber;
	U32			Voltage;
	SENSORCODE	Code;
	//FailureCode	failCode;

	void	Charge();
} ;

#endif // __Battery_H__
