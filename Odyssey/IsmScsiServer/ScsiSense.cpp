/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: ScsiServ.c
// 
// Description:
// This file defines methods to handle SCSI Sense Data
// 
// Update Log
//	$Log: /Gemini/Odyssey/IsmScsiServer/ScsiSense.cpp $
// 
// 4     11/15/99 4:08p Mpanas
// Re-organize sources
// - Add new files: ScsiInquiry.cpp, ScsiReportLUNS.cpp, 
//   ScsiReserveRelease.cpp
// - Remove unused headers: ScsiRdWr.h, ScsiMessage.h, ScsiXfer.h
// - New Listen code
// - Use Callbacks
// - All methods part of SCSI base class
// - Remove DoWork() and replace with RequestDefault() and ReplyDefault()
// 
// 
// 12/10/98 Michael G. Panas: Create file
/*************************************************************************/

#include "OsTypes.h"
#include "Scsi.h"

#include "SCSIServ.h"
#include "ScsiSense.h"

#include <string.h>

/*************************************************************************/
// Forward References
/*************************************************************************/


/*************************************************************************/
// Global references
/*************************************************************************/

// define the data here for testing purposes
//SS_STATUS	ss;

//************************************************************************
//
// ScsiBuildSense
// build a standard SCSI Sense data structure from the fields stored
// in the SS_STATUS structure
//************************************************************************
void ScsiServerIsm::ScsiBuildSense(PREQUEST_SENSE pRQ)
{
	TRACE_ENTRY(ScsiBuildSense);
	
	// load the fields
	pRQ->ResponseCode = RESPONSE_CODE;
	pRQ->AdditionalLength = ADDITIONAL_LENGTH;
	pRQ->SenseKey = (U8) GetSenseKeyStatus();
	pRQ->ASC_ASCQ = (U16) GetASCStatus();

} // ScsiBuildSense

//************************************************************************
// Data Abstraction Methods
// These are defined to insulate the program from how the data is stored
//************************************************************************

//************************************************************************
// SetStatus
// set the global version of the SCSI status for this LUN to the Sense
// code in sense and the ASC/ASCQ value to code.
//************************************************************************
void ScsiServerIsm::SetStatus(U32 sense, U32 code)
{
	m_ss.SenseKey = (U8)sense & 0xff;
	m_ss.AscAscq = (U16)code & 0xffff;
} // SetStatus

//************************************************************************
// GetSenseKeyStatus
// get the global version of the SCSI status for this LUN
//************************************************************************
U32 ScsiServerIsm::GetSenseKeyStatus()
{
	return(m_ss.SenseKey);
	
} // GetSenseKeyStatus

//************************************************************************
// GetASCStatus
// get the global version of the SCSI status for this LUN
//************************************************************************
U32 ScsiServerIsm::GetASCStatus()
{
	return(m_ss.AscAscq);
} // GetASCStatus

