/* DdmRedundancyMgr.h -- Matches Ddm Classes into redundant pairs and
 *						creates VirtualClassDescriptorTable entries.
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
 * The following naming conventions are in effect:
 *
 * DdmCDT 	= DdmClassDescriptorTable.
 * VCDT 	= VirtualClassDescriptorTable.
 * Rec		= Record.
 *
 * Revision History:
 *     02/22/99 Jerry Lane	Created.
 *
**/

#ifndef __DdmRedundancyMgr_H
#define __DdmRedundancyMgr_H

class TSDefineTable;
class TSInsertRow;
class TSReadTable;
class TSReadRow;
class TSListen;

#include "Ddm.h"
#include "DdmClassDescriptorTable.h"
#include "IOPFailoverMapTable.h"
#include "VirtualClassDescriptorTable.h"


class DdmRedundancyMgr: public Ddm {
public:
	// Standard Ddm Method declarations:
					DdmRedundancyMgr(DID did); 
					~DdmRedundancyMgr(); 
	static 	Ddm*	Ctor(DID did);
	virtual STATUS Initialize(Message *pMsg);
	virtual STATUS Enable(Message *pMsg);
	
	// Our own specialized Ddm Methods:
			
			// VerifyIOPFailoverMapExists - Attempts to define the IOPFailoverMapTable.
			STATUS VerifyIOPFailoverMapExists(void *pClientContext, STATUS status);
		
			// InitIOPFailoverMapTable - This method writes the IOP Failover Map.  
			STATUS InitIOPFailoverMapTable(void *pClientContext, STATUS status);

			// VerifyVirtualCDTExists - Attempts to define the VirtualClassDescriptorTable.
			STATUS VerifyVirtualCDTExists(void *pClientContext, STATUS status);
			
			// ReadVirtualCDT - This method reads the VirtualClassDescriptorTable
			STATUS ReadVirtualCDT(void *pClientContext, STATUS status);

			// VerifyDdmCDTExists - Attempts to define the DdmClassDescriptorTable.
			STATUS VerifyDdmCDTExists(void *pClientContext, STATUS status);
			
			// ReadDdmCDT - This method reads the DdmClassDescriptorTable
			STATUS ReadDdmCDT(void *pClientContext, STATUS status);
			
			// ListenOnDdmCDTInserts - Listens for inserts into the DdmClassDescriptorTable.
			STATUS ListenOnDdmCDTInserts(void *pClientContext, STATUS status);
			
			// HandleDdmCDTInsertListenReply -
			// This is the reply handler for the Listen on inserts into the Ddm Class Descriptor Table.  
			STATUS HandleDdmCDTInsertListenReply(void *pClientContext, STATUS status);
									
			// HandleVCDTInsertReply - Handles the reply from the insdert operation.
			STATUS HandleVCDTInsertReply(void *pClientContext, STATUS status);

private:
			// ExtendTable - 
			// Extend our copy of the DdmClassDescriptorTable or the 
			// VirtualClassDescriptorRecords with the new record(s) returned by Listen.
			void ExtendTable(	void*	&pTable,		// pointer to the table to extend.
								U32		NumRecords,		// # of records currently in table.
								U32		cbRecords,		// the size of records  in the table.
								void*	pNewRecords,	// Pointer to new record to append.
								U32		nNewRecords		// # of new records to extend table by.
							);
			
			// IsRedundantPair - Determine if the two DdmClassDescriptorRecords
			// form a redundant pair; return two new VirtualClassDescriptorRecords
			// if they do.
			bool IsRedundantPair( DdmClassDescriptorRecord 		*pDdmCDTRec1,
								  DdmClassDescriptorRecord		*pDdmCDTRec2,
								  VirtualClassDescriptorRecord	VCDTRecs[2]
								);
								
			// SlotsAreRedundant - Checks the IOFailoverMapTable to see
			// if the specified slots are redundant counterparts.
			bool SlotsAreRedundant( TySlot slot1, TySlot slot2);
			
			// FindIOPFailoverMapEntry -
			// This method will return the IOPFailoverMapEntry for the specified IOP slot.
			IOPFailoverMapRecord*	FindIOPFailoverMapEntry(TySlot IOPSlotNum);
			
			// FindMatchingVCDTRecords -
			// This method searches our local copy of the VirtualClassDescriptorTable
			// for records matching the specified records.  
			bool FindMatchingVCDTRecords( VirtualClassDescriptorRecord	VCDTRecs[2] );

protected:

	// Our instance data:
	IOPFailoverMapRecord*			m_pIOPFailoverMap;
	U32								m_nIOPFailoverMapRecords;
	
	VirtualClassDescriptorRecord*	m_pVCDT;
	U32								m_nRowsVCDT;				// # of rows in the VirtualClassdescriptorTable.
	VirtualClassDescriptorRecord	m_VCDTNewRecs[2];
	rowID							m_rgridNewVCDTRecords[2];
	
	DdmClassDescriptorRecord*		m_pDdmCDT;
	U32								m_nRowsDdmCDT;
	DdmClassDescriptorRecord*		m_pDdmCDTInsertedRecord;
	U32								m_cbDdmCDTInsertedRecord;						// U32	cbModifiedrecordRet,

	U32								m_DdmCDTInsertionListenerID;
	
	TSDefineTable*					m_pDefineTable;
	TSReadTable* 					m_pReadTable;
	TSReadRow*						m_pReadRow;
	TSListen*						m_pListen;
	TSInsertRow*					m_pInsertRow;
	
};


#endif	// __DdmRedundancyMgr_H
