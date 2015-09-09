/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// Description:
// This class is the BsaIsmStatus Record.   Containing BSA Status
// data used by the Odyssey Performance Health Status System.
// 
// $Log: /Gemini/Include/BsaStatus.h $
// 
// 3     8/26/99 3:50p Vnguyen
// Clean up the number of performance and status counters to be
// calculated and returned to the PHS Reporter.
// 
// 2     5/04/99 9:05p Mpanas
// Remove unused includes, add num_messages_sent
// 
// 1     5/04/99 6:51p Jlane
// Initial Checkin.
//
/*************************************************************************/

#ifndef __BsaStatus_h
#define __BsaStatus_h

#include "CTTypes.h"

typedef struct _BSA_ISMStatus {
	U32	num_error_replies_received;

// Anything below this line is not used at this time.
	U32	num_messages_received;
	U32	num_replies_sent;
	U32	num_messages_sent;
	U32	num_good_replies_received;
	U32	num_recoverable_media_errors_no_delay;	// The # of recoverable disk media errors w/o delay
	U32	num_recoverable_media_errors_delay;		// The # of recoverable disk media errors with delay
	U32	num_recoverable_media_errors_by_retry;	// The # of disk media errors recoverable w/retry.
	U32	num_recoverable_media_errors_by_ecc;	// The # of disk media errors recoverable w/ecc.
	U32	num_recoverable_nonmedia_errors;		// The # of non-disk media errors recoverable..
	U32	num_bytes_processed_total;				// The total # of bytes processed.
	U32	num_unrecoverable_media_errors;			// The # of unrecoverable disk media errors
} BSA_ISMStatus;

#endif  // __BsaStatus_h