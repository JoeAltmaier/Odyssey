/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FfCommon.h
// 
// Description:
// This file contains macros and definitions used in all 
// Flash File modules.
// 
// 7/20/98 Jim Frandeen: Create file
/*************************************************************************/

#if !defined(FfCommon_H)
#define FfCommon_H

#ifdef _WINDOWS
#define SIM // When compiled under Windows, we always run under the simulator.
#pragma warning( disable : 4103)
#endif

// Some boards were created that use block 0.  Parts of block 0 keep getting erased.
// Until we find the cause, don't use block 0 except for Bat blocks.
// If we find a bad block table in block 0, move it.
// 1/29/00 Jim Frandeen: May have found the problem with bad block table getting
// stepped on.  Put Enable/Disable around sending addresses to FPGA.
// Leave bad block table at zero for now.
//#define MOVE_BAD_BLOCK_0 1

// Define CREATE_BAD_BLOCK_0 to create the bad block table at 0
#define CREATE_BAD_BLOCK_0 1

// Reserve one block in every cell block for basic assurance test.
#define RESERVE_BAT_BLOCK 1

#define EOL "\n"


// If config.erase_all_pages is set to this secret value, we erase all
// pages, even the bad block table.  This is only used when the bad block
// table has been destroyed, and we need to start from scratch. 
// This is used for the Format command, and also for the Create command,
// when we need to test every page.
#define ERASE_ALL_PAGES 0XAAAAAAAA

#include "Callback.h"
#include "CtEvent.h"
#include "CtMessages.h"
#include "ErrorLog.h"
#include "Event.h"
#include "FlashAddress.h"
#include "FlashStorage.h"
#include "PciDev.h"
#include "Simple.h"
#include "StringClass.h"
#include "TraceMon.h"

#define ALIGN64 64 // align on 64 byte boundary

/*************************************************************************/
// Define ZERO macro to clear memory.
/*************************************************************************/
#ifdef _WIN32
#define ZERO(dest, count) \
	memset(dest, 0, count);
#else
extern "C" void bzero(void *pMem, U32 nBytes);
#define ZERO(dest, count) \
	bzero(dest, count);
#endif


typedef enum {
	FF_STATE_UNINITIALIZED,
	FF_STATE_UNFORMATTED,
	FF_STATE_OPENING,
	FF_STATE_OPEN,
	FF_STATE_CLOSING
} FF_STATE;


/*************************************************************************/
// Debugging methods defined in FfDebug.cpp.
/*************************************************************************/
void FF_Break();
void FF_Break_Waiting_Context(Callback_Context *p_callback_context);
void FF_Check_Break_Address(Flash_Address flash_address);
void FF_Check_Break_Block(Flash_Address flash_address);
void FF_Set_Break_Address();
extern Flash_Address FF_break_address;

#define FF_ERROR_CODE(code) CTS_FLASH_##code

#ifdef _DEBUG
// When we return an error code, call a method so we can set a breakpoint.
// We want to be able to call it from a C module.
extern "C" 
{
Status FF_Error(Status status);
}
#define FF_ERROR(code) FF_Error(CTS_FLASH_##code)
#else

// NOT _DEBUG
#define FF_ERROR(code) CTS_FLASH_##code
#endif

// Work in progress to convert each CT_LOG_ERROR
void FF_Log_Error(char * message, char * testfilename, int lineno);
#define LOG_ERROR(message) \
	FF_Log_Error(message, __FILE__, __LINE__);
	
// Work in progress to convert each FF_ERROR

#endif /* FfCommon_H  */
