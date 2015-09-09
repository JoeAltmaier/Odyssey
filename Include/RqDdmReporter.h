/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: RqDdmReporter.h
// 
// Description:
// This file specifies the Request Interface to DdmReporter.
// 
// Update Log 
//
// $Log: /Gemini/Include/RqDdmReporter.h $
// 
// 9     11/03/99 3:35p Vnguyen
// Add typedef U32 for REPORTERCODE.
// 
// 8     9/01/99 12:28p Vnguyen 
// Add define DDM_REPLY_DATA_SGI
// 
// 7     8/16/99 8:38a Vnguyen
// Fix up typedef declaration for REPORTERCODE
// 
// 6     8/10/99 12:09p Vnguyen
// Add missing semicolon for enum ReporterCode.
// Clear up a few comments.
// 
// 5     8/09/99 10:47a Vnguyen
// Add second constructor for Ddm without vdn.
// 
// 4     8/05/99 1:28p Vnguyen
//
// 4	8/05/99 1:26p Vnguyen -- Fix typo 
// 3     8/03/99 4:09p Vnguyen
//
// 08/03/99	Vnguyen. Created.
//

/*************************************************************************/
/* RqDdmReporter.h -- Request Interface to DdmReporter.
 *
 * Copyright (C) ConvergeNet Technologies, 1998,99
 *
 * This material is a confidential trade secret and proprietary 
 * information of ConvergeNet Technologies, Inc. which may not be 
 * reproduced, used, sold or transferred to any third party without the 
 * prior written consent of ConvergeNet Technologies, Inc.  This material 
 * is also copyrighted as an unpublished work under sections 104 and 408 
 * of Title 17 of the United States Code.  Law prohibits unauthorized 
 * use, copying or reproduction.
 *
**/


#ifndef __RqDdmReporter_h
#define __RqDdmReporter_h

#include "Ddm.h"
#include "OsTypes.h"
#include "Message.h"


#define DDM_REPLY_DATA_SGI	0


enum {
	PHS_DISK_PERFORMANCE,
	PHS_DISK_STATUS,
	PHS_ARRAY_PERFORMANCE,
	PHS_ARRAY_STATUS,
	PHS_SCSI_TARGET_SERVER_PERFORMANCE,
	PHS_SCSI_TARGET_SERVER_STATUS,
	PHS_FCP_TARGET_PERFORMANCE,
	PHS_FCP_TARGET_STATUS,
	PHS_SSD_PERFORMANCE,
	PHS_SSD_STATUS,
	PHS_EVC_STATUS
};

typedef U32 REPORTERCODE;


// This is the payload structure for the PHS_START message.
// Also for PHS_PAUSE, PHS_CONTINUE, PHS_RESET, PHS_STOP
// 
// PHS_START -- Start the Reporter for the Did, need one PHS_START for perf, one for status
// PHS_PAUSE, PHS_CONTINUE -- Note:  Data collection will
//   pause (timer is stop), but the performance and status counters are not reset.
// PHS_RESET -- Reset counters (only for performance, do nothing for status)
// PHS_STOP -- Stop the reporter and perform cleanup
//
// Usage:  	RqDdmReporter *pMsg;
//		   	pMsg = new RqDdmReporter(PHS_START, PHS_DISK_STATUS, myDid, myVdn);
//			Send(pMsg, (ReplyCallback) &myHandleReporterReply);
//         Or
//		   	pMsg = new RqDdmReporter(PHS_START, PHS_DISK_STATUS, myDid);
//			Send(pMsg, (ReplyCallback) &myHandleReporterReply);


class RqDdmReporter : public Message {

public:
	DID did;
	REPORTERCODE ReporterCode;
	VDN vdn;

	RqDdmReporter(REQUESTCODE _RequestCode,		
				REPORTERCODE _ReporterCode,		// Type of Reporter to initialize
				DID _did,						// _did to poll for data
				VDN _vdn):						// _vdn to look up in SRC/Export table
				
				Message(_RequestCode),
				ReporterCode(_ReporterCode),
				did(_did),
				vdn(_vdn) {	}


	RqDdmReporter(REQUESTCODE _RequestCode,		
				REPORTERCODE _ReporterCode,		// Type of Reporter to initialize
				DID _did):						// _did to poll for data
				
				Message(_RequestCode),
				ReporterCode(_ReporterCode),
				did(_did),
				vdn(VDNNULL) {	}




//protected:


};
	
#endif	// __RqDdmReporter_h
