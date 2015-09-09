/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: DM_Messages.h
// 
// Description:
//	This file has the message definitions for the DriveMonitor
// 
// Update Log:
//	$Log: /Gemini/Include/Fcp/DM_Messages.h $
// 
// 1     7/15/99 11:51p Mpanas
// Changes to support Multiple FC Instances
// and support for NAC
// -New Message front end for the DriveMonitor
//
// 07/07/99 Michael G. Panas: Create file
/*************************************************************************/

#if !defined(DM_Messages_h)
#define DM_Messages_h

#include "Ddm.h"
#include "RequestCodes.h"
#include "Message.h"


// Note: The DM_SCAN and DM_STOP_ALL do not have a payload
//       so no special message is needed.
//

// Drive Scan Single Drive message
class DmScanSingle : public Message {
public:
	enum { RequestCode = DM_SCAN_SINGLE };

	typedef struct _payload {
		U32		drive;		// Drive Number to scan
	} Payload;
	
	Payload payload;
	
	DmScanSingle() : Message(RequestCode) {}
};

// Drive Startup message
class DmStart : public Message {
public:
	enum { RequestCode = DM_START };

	typedef struct _payload {
		U32		drive;		// Drive Number to start
	} Payload;
	
	Payload payload;
	
	DmStart() : Message(RequestCode) {}
};

// Drive Spin Down message
class DmStop : public Message {
public:
	enum { RequestCode = DM_STOP };

	typedef struct _payload {
		U32		drive;		// Drive Number to stop
	} Payload;
	
	Payload payload;
	
	DmStop() : Message(RequestCode) {}
};


//  Test Types
#define	DM_TYP_STS		0		// run test through the Scsi Target Server
#define	DM_TYP_DRIVE	1		// run test direct to drive


// Drive Monitor Read Test Message
//	Two types are supported:
//		Read Drive
//		Read Through STS
//
class DmReadTest : public Message {
public:
	enum { RequestCode = DM_READ_TEST };

	typedef struct _payload {
		U32		drive;		// Drive Number to test
		VDN		vdnSTS;		// SCSI Target Server VDN
		U32		type;		// Test type
	} Payload;
	
	Payload payload;
	
	DmReadTest() : Message(RequestCode) {}
};

// Drive Monitor Read Verify Test Message
//
class DmReadVerifyTest : public Message {
public:
	enum { RequestCode = DM_READ_VERIFY };

	typedef struct _payload {
		U32		drive;		// Drive Number to test
		VDN		vdnSTS;		// SCSI Target Server VDN
		U32		block;		// starting block number
		U32		count;		// number of blocks
		U32		type;		// Test type
	} Payload;
	
	Payload payload;
	
	DmReadVerifyTest() : Message(RequestCode) {}
};

// Drive Monitor Write Verify Test Message
//
class DmWriteVerifyTest : public Message {
public:
	enum { RequestCode = DM_WRITE_VERIFY };

	typedef struct _payload {
		U32		drive;		// Drive Number to test
		VDN		vdnSTS;		// SCSI Target Server VDN
		U32		block;		// starting block number
		U32		count;		// number of blocks
		U32		type;		// Test type
	} Payload;
	
	Payload payload;
	
	DmWriteVerifyTest() : Message(RequestCode) {}
};

#endif /* DM_Messages_h  */
