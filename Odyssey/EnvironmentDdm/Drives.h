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
// File: Drives.h
// 
// Description:
// 	This file is interface for the Drives module. 
// 
// $Log: /Gemini/Odyssey/ExceptionMaster/Drives.h $
// 
// 2     9/08/99 8:14p Hdo
// Integration with AlarmMaster
// 
// 1     8/25/99 1:55p Hdo
// First check in
// 
// 8/4/99 Huy Do: Create file
/*************************************************************************/

#ifndef __Drives_H__
#define __Drives_H__

#include "AbstractCmd.h"

class Drives : public AbstractCmd {
public:
	Drives(U32 number) : DriveNumber(number) {};
	~Drives();

	virtual STATUS performTask(XM_CONTEXT	*pXMContext);

private:
	U32		DriveNumber;

	void	SpinDown();
	void	SpinUp();
	void	FlushData();
} ;

#endif // __Drives_H__
