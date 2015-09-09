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
// File: Fan.h
// 
// Description:
// 	This file is interface for the Fan module. 
// 
// $Log: /Gemini/Odyssey/ExceptionMaster/Fan.h $
// 
// 2     9/08/99 8:14p Hdo
// Integration with AlarmMaster
// 
// 1     8/25/99 1:55p Hdo
// First check in
// 
// 8/9/99 Huy Do: Create file
/*************************************************************************/

#ifndef __Fan_H__
#define __Fan_H__

#include "AbstractCmd.h"

class Fan : public AbstractCmd {
public:
	Fan(U32 number) : FanNumber(number) {};
	~Fan();

	virtual STATUS performTask(XM_CONTEXT	*pXMContext);
	U32		GetSpeed() const { return Speed; }
	U32		GetFan() const { return FanNumber; }

private:
	U32		FanNumber;
	U32		Speed;

	void	SpeedUp();
	void	SlowDown();
} ;

#endif // __Fan_H__
