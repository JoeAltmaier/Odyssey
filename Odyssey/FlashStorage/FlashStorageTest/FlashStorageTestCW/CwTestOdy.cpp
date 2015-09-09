/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: CwTestOdy.cpp
//
// Description:
// This file contains the test methods for the flash file system.
// This file is used only for the test driver on Odyssey.
//
//
// Update Log:
// 1/25/99 Jim Frandeen: Create file.
/*************************************************************************/

#include  "Callback.h"
#include  "FlashStorage.h"
#include  "ErrorLog.h"
#include  "FtFlashTest.h"
#include  "FlashStorageBat.h"
#include  "TraceMon.h"

U32 unit_is_open = 0;

void Open_Flash()
{
	// Open test unit.
	unit_is_open = 1;
	
} // Open_Flashvoid Open_Flash()

void Close_Flash()
{
	// Close test unit.
	unit_is_open = 0;
	
} // Open_Flash
