/* DdmRegistrar.cpp -- Registers Ddms on an IOP with the HBC / PTS.
 *
 * Copyright (C) ConvergeNet Technologies, 1998 
 *
 * This material is a confidential trade secret and proprietary 
 * information of ConvergeNet Technologies, Inc. which may not be 
 * reproduced, used, sold or transferred to any third party without the 
 * prior written consent of ConvergeNet Technologies, Inc.  This material 
 * is also copyrighted as an unpublished work under sections 104 and 408 
 * of Title 17 of the United States Code.  Law prohibits unauthorized 
 * use, copying or reproduction.
 *
 // $Log: /Gemini/Odyssey/DdmRegistrar/DdmRegistrar.cpp $
// 
// 8     7/24/99 5:41p Jlane
// reslove PTS changes
// 
// 7     5/13/99 11:37a Cwohlforth
// Edits to support conversion to TRACEF
// 
// 6     4/06/99 11:24a Jlane
// added unused pragmas for unused args.
// 
// 5     3/30/99 9:23p Jlane
// Add new parameter to CLASSNAME Macro, Fix new allocations to use [] not
// (), Final modifications to map the class info records into PTS format.
 //
 * Revision History:
 *     02/17/99 Jerry Lane	Created.
 *
**/

#include "Address.h"
#include "DdmRegistrar.h"
#include "DdmClassDescriptorTable.h"
#include "BuildSys.h"
#include "Table.h"
#include "Rows.h"

#define _DEBUG
#include "Odyssey_Trace.h"

// my ruler	
//345678901234567890123456789012345678901234567890123456789012345678901234567890


CLASSNAME(DdmRegistrar, SINGLE);	// Class Link Name used by Buildsys.cpp


// DdmScc -- Constructor ------------------------------------------DdmRegistrar-
//
DdmRegistrar::DdmRegistrar(DID did): Ddm(did)
{
	Tracef("DdmRegistrar::DdmRegistrar()\n");
	// I have no config area
//	SetConfigAddress(&config,sizeof(config));	// tell Ddm:: where my config area is
}

// Ctor -- Create ourselves ---------------------------------------DdmRegistrar-
//
Ddm *DdmRegistrar::Ctor(DID did)
{
	return new DdmRegistrar(did);
}

// .Initialize -- Just what it says -------------------------------DdmRegistrar-
//
STATUS DdmRegistrar::Initialize(Message *pMsg)	// virutal
{ 
	Tracef("DdmRegistrar::Initialize()\n");

	#if 0
	// If we ever want to be able to register on command we'll need the following lines:
	Serve(DDM_REGISTRAR_REPORT_DDM_CLASSES,FALSE);
	DispatchRequest(DDM_REGISTRAR_REPORT_DDM_CLASSES, pServices, (RequestCallback) &VerifyDdmCDTExists);
	#endif
	
	return Ddm::Initialize(pMsg);
}

// Enable -- Start-it-up ------------------------------------------DdmRegistrar-
//
STATUS DdmRegistrar::Enable(Message *pMsg)	// virtual
{ 
STATUS	status;

	Tracef("DdmRegistrar::Enable()\n");
	
	status = VerifyDdmCDTExists(pMsg);
	
	return Ddm::Enable(pMsg);
}


// VerifyDdmCDTExists - Verify Ddm Class Descriptor Table Exists - DdmRegistrar-
// Called from Initialize, This is the beginning of the actual work of this Ddm.
// This method is the primary entry point for the Calss Registration process.
// This method insures the existance of the ddm Class descriptor Table.  This is 
// done by simply defining the table.  If the table already exists, no harm will
// be done and if the table doesn't already exist then it will be defined.
// When initiating the DefineTable Operation this method specifies the method
// RegisterDdmClasses as the callback function that method will initiate the 
// rest of the class registration process.  
STATUS DdmRegistrar::VerifyDdmCDTExists(Message *pMsg)
{
#pragma unused(pMsg)

STATUS 		status;

	// This is the code to create the DdmClassDescriptorTable.
		  
	TSDefineTable *pDefineTable = new TSDefineTable;

	status = pDefineTable->Initialize
	(
		this,										// DdmServices* pDdmServices,
		CT_DCDT_TABLE_NAME,							// String64 prgbTableName,
		aDdmClassDescriptorTable_FieldDefs,			// fieldDef* prgFieldDefsRet,
		cbDdmClassDescriptorTable_FieldDefs,		// U32 cbrgFieldDefs,
		20,											// U32 cEntriesRsv,
		false,										// bool* pfPersistant,
		(pTSCallback_t)&RegisterDdmClasses,			// pTSCallback_t pCallback,
		NULL										// void* pContext
	);
	
	if (status == OS_DETAIL_STATUS_SUCCESS)
		pDefineTable->Send();

	return status;
}


// RegisterDdmClasses -- Process Message --------------------------DdmRegistrar-
// Called back once the presence of the Ddm Class Descriptor Table is insured
// to register DdmClasses on the local IOP, this routine will do just that by
// sending a message to the DdmManager requesting the DdmManager return the
// classes of Ddms on the local IOP.  The replies from the DdmManager will be
// directed to the RegisterClassEntry method. 
STATUS DdmRegistrar::RegisterDdmClasses(void *pClientContext, STATUS status)
{
#pragma unused(pClientContext)

RqOsSiGetClassTable*	pMsg = new RqOsSiGetClassTable();

	// Send the message requesting the ClassEntries off to the DdmManager
	status = Send(pMsg, NULL, (ReplyCallback)&RegisterClassEntry);
	if (status != OS_DETAIL_STATUS_SUCCESS)
		delete pMsg;
		
	return status;
}


// RegisterClassEntry -- Register one Ddm Class -------------------DdmRegistrar-
// This method will receive replies from the DdmManager each containing one Ddm
// class entry descriptor.  This method will map this record into the format of
// the DdmClassDescriptorRecord and then insert the DdmClassDescriptorRecord in
// the DdmClassDescriptorTable on the HBC.
STATUS DdmRegistrar::RegisterClassEntry(RqOsSiGetClassTable *pMsg)
{
STATUS 						status;
DdmClassDescriptorRecord	myDdmClassDescriptorRecord;
TSInsertRow					*pInsertRow;

	if (pMsg->DetailedStatusCode != OK) {
		Tracef("DdmRegistrar: SysInfo Ddm Replied with erc = %u\n",pMsg->DetailedStatusCode);
		return OK;
	}
	
	#if 0
	if (pMsg->payload.iClass == 0) {
		Tracef("SysInfo GetClass Table Details:\n");
		Tracef("     #/# cbStack sQueue flags    nLocal nVirtual nDid nDdm Version ClassName\n");
	}
	Tracef("    %2u/%-2u  %4u  %4u  %08x  %4u    %4u   %4u %4u  ..%2lu.. \"%s\"\n",
			pMsg->payload.iClass,
			pMsg->payload.cClass,
			pMsg->payload.cbStack,
			pMsg->payload.sQueue,
			pMsg->payload.flags,
			pMsg->payload.nServeLocal,
			pMsg->payload.nServeVirtual,
			pMsg->payload.nDidInstance,
			pMsg->payload.nDdmInstance,
			pMsg->payload.version,
			pMsg->payload.szClassName);
	#endif
	
	// Do mapping here from SI record to DdmClassDescriptorTable record.
	myDdmClassDescriptorRecord.rid.Table = 0;							// rowID of this record.
	myDdmClassDescriptorRecord.rid.HiPart = 0;							// rowID of this record.
	myDdmClassDescriptorRecord.rid.LoPart = 0;							// rowID of this record.
	myDdmClassDescriptorRecord.version = 1;								// Version of IOPClassConstructor record.
	myDdmClassDescriptorRecord.size = sizeof(DdmClassDescriptorRecord);	// Size of IOPClassConstructor record in bytes.
	myDdmClassDescriptorRecord.iCabinet = Address::iCabinet;					// The cabinet in which the Ddm class exists.
	myDdmClassDescriptorRecord.iSlot = Address::iSlotMe;						// The slot in which this Ddm class exists.
	strcpy(myDdmClassDescriptorRecord.ClassName, pMsg->payload.szClassName);	// Name of the Class.
	myDdmClassDescriptorRecord.ridPropertySheet.Table = 0;						// 0th item in Class Config Table.?
	myDdmClassDescriptorRecord.ridPropertySheet.HiPart = 0;						// 0th item in Class Config Table.?
	myDdmClassDescriptorRecord.ridPropertySheet.LoPart = 0;						// 0th item in Class Config Table.?
	myDdmClassDescriptorRecord.sStack = pMsg->payload.cbStack;			// Stack size for the Class.
	myDdmClassDescriptorRecord.sQueue = pMsg->payload.sQueue;				// The size of the msg Queue for the Ddm.
	
	// Allocate an insertRow object, init it and send it off.
	pInsertRow = new TSInsertRow;
	
	status = pInsertRow->Initialize( this,										// DdmServices *pDdmServices,
									 CT_DCDT_TABLE_NAME,			// String64 rgbTableName,
									 &myDdmClassDescriptorRecord,			// void *prgbRowData,
									 sizeof(myDdmClassDescriptorRecord),	// U32 cbRowData,
									 NULL,									// rowID *pRowIDRet,
									 (pTSCallback_t)&HandleInsertRowReply,	// pTSCallback_t pCallback,
									 NULL									// void *pContext
								   );
								   
	if (status == OS_DETAIL_STATUS_SUCCESS)							   
		pInsertRow->Send();

	return status;
}


// HandleInsertRowReply -- Handle reply from Ddm CDT insert -------DdmRegistrar-
// This method will receive replies from the Table Service interface after 
// insertion of recordds into the Ddm Class Descriptor Table. Currently it just
// checks the error.
STATUS DdmRegistrar::HandleInsertRowReply(void *pClientContext, STATUS status)
{
#pragma unused(pClientContext)

	if (status != OS_DETAIL_STATUS_SUCCESS)							   
		// TODO: Add Error Handling here.
		;

	return status;
}



#if 0
struct DdmClassDescriptorRecord {	// One row for each class type on each IOP.
	rowID		rid;				// rowID of this record.
	U32			version;			// Version of IOPClassConstructor record.
	U32			size;				// Size of IOPClassConstructor record in bytes.
	int 		iCabinet;			// The cabinet in which the Ddm class exists.
	TySlot		iSlot;				// The slot in which this Ddm class exists.
	String32	Name;				// Name of the Class.
	rowID		ridPropertySheet;	// 0th item in Class Config Table.?
	U32			sStack;				// Stack size for the Class.
	U32			sQueue;				// The size of the msg Queue for the Ddm.
	Ddm* 		(*ctor)(DID);		// ctor is a pointer to a function taking a VirtualDevice
									// and returning a pointer to a DDM. 
} DdmClassDescriptorRecord, DdmClassDescriptorTable[];
#endif

