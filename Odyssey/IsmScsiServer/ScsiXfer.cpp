/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: ScsiXfer.c
// 
// Description:
// This file implements Transfer methods for the SCSI Server
// 
// Update Log 
//	$Log: /Gemini/Odyssey/IsmScsiServer/ScsiXfer.cpp $
// 
// 3     11/15/99 4:03p Mpanas
// Re-organize sources
// - Add new files: ScsiInquiry.cpp, ScsiReportLUNS.cpp, 
//   ScsiReserveRelease.cpp
// - Remove unused headers: ScsiRdWr.h, ScsiMessage.h, ScsiXfer.h
// - New Listen code
// - Use Callbacks
// - All methods part of SCSI base class
//
// 
// 12/14/98 Michael G. Panas: Create file
/*************************************************************************/

#include "SCSIServ.h"
#include "ScsiSense.h"
#include "Scsi.h"

#include <string.h>

/*************************************************************************/
// Forward References
/*************************************************************************/


//************************************************************************
//
// ScsiSendData()
// Copy the data at the location pSrc to the address in the SGL.
// The SGL is contained in the original message.
//************************************************************************
U32 ScsiServerIsm::ScsiSendData(SCSI_CONTEXT *p_context, U8 *pSrc, U32 Length)
{
	U32			result;
	Message		*pMsg = p_context->pMsg;
	
	TRACE_ENTRY(ScsiSendData);
	
	// Copy data to the SGL return buffer, starting at the given offset
	// within the return buffer.  Truncates data if it won't fit.
	// Handles multiple return buffer fragments.
	// Returns the number of bytes NOT returned (hopefully 0).
	result = pMsg->CopyToSgl(0, 0, (void *) pSrc, (long) Length);
	
	if (result != 0)
		return (result);
	
	return(0);
} // ScsiSendData


//************************************************************************
//
// ScsiGetData()
// Copy data from the address in the SGL to the location at pDst
//************************************************************************
U32 ScsiServerIsm::ScsiGetData(SCSI_CONTEXT *p_context, U8 *pDst, U32 Length)
{
	// TODO
	Message		*pMsg = p_context->pMsg;
	
	TRACE_ENTRY(ScsiGetData);
	
	return(0);
} // ScsiGetData


