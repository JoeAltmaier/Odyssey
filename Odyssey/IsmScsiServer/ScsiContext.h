/*************************************************************************/
// 
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: ScsiContext.h
// 
// Description:
// This file contains context information for the Scsi Server.
// 
// Update Log
//	$Log: /Gemini/Odyssey/IsmScsiServer/ScsiContext.h $
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
// 12/10/98 Michael G. Panas: Create file
/*************************************************************************/


#ifndef __ScsiContext_h
#define __ScsiContext_h

// SCSI Target Server ISM internal context
struct SCSI_CONTEXT {
	//void			*buffer;			// pointer to buffer (if used)
	Message			*pMsg;				// pointer to original message
	Message			*p_bsaMsg;			// pointer to BSA message
};


#endif