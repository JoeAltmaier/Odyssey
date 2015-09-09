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
// File: ScsiSense.h
// 
// Description:
// This file contains Sense Data handler interfaces for the Scsi Server.
// 
// Update Log 
// 
// 12/10/98 Michael G. Panas: Create file
/*************************************************************************/


#ifndef __ScsiSense_h
#define __ScsiSense_h

// Struct to save a copy of the Request Sense Status to be returned
typedef struct _SS_Status {
	U16		AscAscq;
	U8		ScsiStatus;			// Scsi command status
	U8		ScsiAdapterStatus;
	U8		SenseKey;			// for check status
} SS_STATUS, *PSS_STATUS;


#endif