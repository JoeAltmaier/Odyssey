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
// File: FlashOperation.h
// 
// Description:
// This file is the interface for FlashOperation module. 
// 
// $Log: /Gemini/Odyssey/IsmFlashMonitor/FlashOperation.h $
// 
// 1     10/11/99 4:56p Hdo
// Initial check in
// 
// 07/13/99 Huy Do: Create file
/*************************************************************************/

#if !defined(FlashOperation_H)
#define FlashOperation_H

#include "FlashFile.h"

class FlashOperation {
public:
	void Read();
	void Write();
	void Verify();
	void GetCapacity();
	FF_CONFIG *GetFlashConfig();
};

#endif /* FlashOperation_H  */
