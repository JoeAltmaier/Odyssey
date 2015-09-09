/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// (c) Copyright 1999 ConvergeNet Technologies, Inc.
//     All Rights Reserved.
//
// File: DdmPnPTest.h
// 
// Description:
//    Defines a test DDM for exercising the PnPDDM.
// 
//     3/19/99 7:37p HDo: Create.
//
/*************************************************************************/

#ifndef _DdmPnPTest_h_
#define _DdmPnPTest_h_

#ifndef __Ddm_h
# include "Ddm.h"
#endif

#ifndef __Table_h__
# include  "Table.h"
#endif

typedef struct {
	rowID	rid;									// rowID of this record.
	U32 	version;								// Version of DISK_EVENT_DATA record
	U32		size;									// Size of DISK_EVENT_DATA record in bytes.
	U32		key;									// ala Rick Currently unused, could hold Circuit Key.
	U32		RefreshRate;							// 0 => refresh once >0 implies refresh every x seconds?    
	rowID	ridSRCRecord;							// Row ID of  Storage Roll Call  entry 4 this device.
	U32		num_recoverable_media_errors_no_delay;	// The number of recoverable disk media errors w/o delay
	U32		num_recoverable_media_errors_delay;		// The number of recoverable disk media errors with delay
	U32		num_recoverable_media_errors_by_retry;	// The number of disk media errors recoverable w/retry.
	U32		num_recoverable_media_errors_by_ecc;	// The number of disk media errors recoverable w/ecc.
	U32		num_recoverable_nonmedia_errors;		// The number of non-disk media errors recoverable..
	U32		num_bytes_processed_total;				// The total number of bytes processed.
	U32		num_unrecoverable_media_errors;			// The number of unrecoverable disk media errors.
}  DiskStatusRecord, DiskStatusTable[];

class DdmPnPTest: public Ddm
{
public:

	static Ddm *Ctor(DID did);
	DdmPnPTest(DID did);
	virtual STATUS  Initialize (Message *pMsg);
	virtual STATUS  Enable (Message *pMsg);

private:
	struct {
		VDN vd;
		U16 id;
	} config;

	//STATUS TestCreateTable(void);
	//STATUS TestInsertRow(void);
	//STATUS TestGetTableDef(void);
	//STATUS TestEnumTable(void);
};  /* end of class DdmPnPTest */

#endif   // #ifndef _DdmPnPTest_h_