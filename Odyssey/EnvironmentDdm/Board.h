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
// File: Board.h
// 
// Description:
// 	This file is interface for the Board module. 
// 
// $Log: /Gemini/Odyssey/ExceptionMaster/Board.h $
// 
// 2     9/08/99 8:14p Hdo
// Integration with AlarmMaster
// 
// 1     8/25/99 1:55p Hdo
// First check in
// 
// 8/4/99 Huy Do: Create file
/*************************************************************************/

#ifndef __Board_H__
#define __Board_H__

#include "AbstractCmd.h"

class Board : public AbstractCmd {
public:
	Board(U32 boardNumber, U32 sensor) : BoardNumber(boardNumber), SensorNumber(sensor) {};
	~Board();

	virtual STATUS performTask(XM_CONTEXT	*pXMContext);

private:
	U32		BoardNumber;
	U32		SensorNumber;

	void	FailOver();
	void	StartShutdown();
	void	RunDegradedMode();
	void	RunNormal();
} ;

#endif // __Board_H__
