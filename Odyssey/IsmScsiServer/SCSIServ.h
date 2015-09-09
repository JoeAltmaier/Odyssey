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
// File: SCSIServ.h
// 
// Description:
// This file is the common header file for the Odyssey Scsi Target Server. 
// 
// Update Log
//	$Log: /Gemini/Odyssey/IsmScsiServer/SCSIServ.h $
// 
// 9     11/15/99 4:08p Mpanas
// Re-organize sources
// - Add new files: ScsiInquiry.cpp, ScsiReportLUNS.cpp, 
//   ScsiReserveRelease.cpp
// - Remove unused headers: ScsiRdWr.h, ScsiMessage.h, ScsiXfer.h
// - New Listen code
// - Use Callbacks
// - All methods part of SCSI base class
// - Remove DoWork() and replace with RequestDefault() and ReplyDefault()
// 
// 05/18/98 Michael G. Panas: Create file
// 01/26/99 Michael G. Panas: Use new TraceLevel[] array, normalize all trace
/*************************************************************************/

#if !defined(__SCSISERV_H_)
#define __SCSISERV_H_

#include <stdio.h>
#include "OsTypes.h"

#define	TRACE_INDEX		TRACE_SCSI_TARGET_SERVER
#include "Odyssey_Trace.h"

#include "scsi.h"

#include "Message.h"

#include "ReadTable.h"
#include "Table.h"
#include "Rows.h"

#include "ScsiServerIsm.h"
#include "ScsiContext.h"

#endif /* __SCSISERV_H_  */
