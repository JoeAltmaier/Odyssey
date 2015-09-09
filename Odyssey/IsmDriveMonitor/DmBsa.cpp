/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: DmBsa.cpp
// 
// Description:
// This module contains code to support BSA type devices for DriveMonitor
// 

// Update Log:
//	$Log: /Gemini/Odyssey/IsmDriveMonitor/DmBsa.cpp $
// 
// 13    1/11/00 7:27p Mpanas
// New PathTable changes
// 
// 12    1/03/00 5:46p Jlane
// Don't autostart BSA VDs.
// 
// 11    12/23/99 6:09p Jlane
// Roll back in new VD Creation code previously backed out.
// 
// 10    12/22/99 4:34p Jlane
// Use IOP_NONE instead of SLOTNULL for non redundant device
// instantiation.
// 
// 9     12/21/99 4:16p Jlane
// Add support for fRedundant flag.
// 
// 8     12/21/99 2:00p Mpanas
// Add support for IOP Failover
// - make several modules IOP_LOCAL
// 
// 7     11/10/99 3:55p Jlane
// Roll back in previous VD create code cause it works.
// 
// 5     11/04/99 4:40p Jlane
// Use new VD definition code.
// 
// 4     10/11/99 7:20p Mpanas
// Second part of BSA VD Create, check for prior
// BSA device entries (like in BuildSys) 
// Note: this change needs the fix in VirtualDeviceTable.h
// Initialize to zero pad the class string.
// 
// 3     10/08/99 1:04p Mpanas
// minor fix for wrong value
// 
// 2     10/07/99 8:09p Mpanas
// First cut of BSA VD Create
// 
// 1     9/14/99 8:40p Mpanas
// Complete re-write of DriveMonitor
// - Scan in sequence
// - Start Motors in sequence
// - LUN Scan support
// - Better table update
// - Re-organize sources
// 
// 
// 09/14/99 Michael G. Panas: Create file
/*************************************************************************/

#include <stdio.h>
#include <string.h>
#include "OsTypes.h"
#include "Odyssey.h"
#include "Scsi.h"

#include "ReadTable.h"
#include "Table.h"
#include "Rows.h"
#include "Listen.h"

#include "RqOsVirtualMaster.h"
#include "DriveMonitorIsm.h"
#include "DmCommon.h"

#include "Message.h"
#include "FcpMessageFormats.h"
#include "FcpMessageStatus.h"

#include "CTEvent.h"
#include "CtUtils.h"

// Table method context
typedef struct _DM_CR_CONTEXT {
	U32								 flags;			// execution flags
	void							*p;				// saved context pointer
	DM_DEVICE_STATE					*pDMState;		// device state
	DriveMonitorIsm::pDMCallback_t	 Callback;		// saved Callback address
} DM_CR_CONTEXT, *PDM_CR_CONTEXT;


/*************************************************************************/
// Forward references
/*************************************************************************/

/*************************************************************************/
// Global references
/*************************************************************************/


/*************************************************************************/
// DM_Create_Bsa
// Create a BSA virtual device for a specific PathDescriptor.  The descriptor
// passed in the DEVICE_STATE structure.
/*************************************************************************/
void DriveMonitorIsm::DM_Create_Bsa_Device(void *p,
									DM_DEVICE_STATE	*pDMState,
									pDMCallback_t Callback)
{
	DM_CR_CONTEXT	*pCC = new DM_CR_CONTEXT;
	STATUS 			 status;

	TRACE_ENTRY(DM_Create_Bsa_Device);

	pCC->p = p;
	pCC->pDMState = pDMState;
	pCC->Callback = Callback;
	pCC->flags = 0;
	
	if (DM_Find_Bsa_Device(pDMState))
	{
		TRACEF(TRACE_L3, ("\n\rDM: Bsa Vdn %d found", pDMState->pPD->vdnDdm));
	
		// found a pre-configured device for this device, call end
		DM_Create_Bsa_End(pCC, 0);
		
		return;
	}
	
	TRACEF(TRACE_L3, ("\n\rDM: Bsa Create Started"));
	
	// create a virtual device
	status = DM_Create_InsertBSACfg(pCC, OK);
	
} // DM_Create_Bsa_Device


/*************************************************************************/
// VCCreate_InsertSTSCfg
//
//  Description:
//    Insert a BSA configuration record into the BSA config table 
//
//  Inputs:
//    pClientContext - Our create context
//
//    status - The returned status of the previous PTS operation.
//
//  Outputs:
//    Returns OK, or a highly descriptive error code.
//
/*************************************************************************/
STATUS DriveMonitorIsm::DM_Create_InsertBSACfg(void *pClientContext, STATUS status)
{
	DM_CR_CONTEXT*	pCC = (DM_CR_CONTEXT *)pClientContext;

	TRACE_ENTRY(DM_Create_InsertBSACfg);

	// Initialize our BSA configuration record
	m_BSAConfigRec.version = BSA_CONFIG_VERSION;
	m_BSAConfigRec.size = sizeof(BSA_CONFIG);

	// Fill in the BSA config values that match this DiskDescriptor record
	m_BSAConfigRec.initVd = config.vd;
	m_BSAConfigRec.LUN = pCC->pDMState->pPD->FCTargetLUN;
	m_BSAConfigRec.ID = pCC->pDMState->pPD->FCTargetID;
	m_BSAConfigRec.EnableSMART = 0;
	
	// Insert the new BSA Config record in the table of same.
	m_pInsertRow = new TSInsertRow;
	if (!m_pInsertRow)
		status = CTS_OUT_OF_MEMORY;
	else		
		status = m_pInsertRow->Initialize(
			this,									// DdmServices *pDdmServices,
			BSA_CONFIG_TABLE_NAME,					// String64 rgbTableName,
			&m_BSAConfigRec,						// void *prgbRowData,
			sizeof(m_BSAConfigRec),					// U32 cbRowData,
			&m_BSAConfigRec.rid,					// rowID *prowIDRet,
			TSCALLBACK(DriveMonitorIsm,DM_Create_InstBSAVD),// pTSCallback_t pCallback,
			pCC										// void* pContext
		);

	if (status == OK)
		m_pInsertRow->Send();
	else
		status = DM_Create_Bsa_End(pCC, status);			
		
	return status;
	
}	// DM_Create_InsertBSACfg

/*************************************************************************/
// DM_Create_InstBSAVD
//
//  Description:
//    Instantiate the BSA DDM Virtual Device by Inserting a BSA VD Record
//    into the Virtual Device table.
//
//  Inputs:
//    pClientContext - our create context pointer
//
//    status - The returned status of the previous PTS operation.
//
//  Outputs:
//    Returns OK, or a highly descriptive error code.
//
/*************************************************************************/
STATUS DriveMonitorIsm::DM_Create_InstBSAVD(void *pClientContext, STATUS status)
{
	DM_CR_CONTEXT*	pCC = (DM_CR_CONTEXT *)pClientContext;

	TRACE_ENTRY(DM_Create_InstBSAVD);

	if ((status != ercKeyNotFound) && (status != ercEOF) && (status != OK))
		return DM_Create_Bsa_End(pCC, status);
			
	TRACE_STRING(TRACE_L8, "\n\rDM_Create_InstBSAVD: Creating VDT entry");

	RqOsVirtualMasterLoadVirtualDevice *pCreateBsaVDMsg;
	
	// Our failover partner's slot number
	TySlot MyFOP;
	if(config.flags & DM_FLAGS_REDUNDANT)
		MyFOP =  Address::GetFopForIop(Address::iSlotMe);	// DID of secondary DDM.
	else
		MyFOP =  SLOTNULL;									// There is no DID of secondary DDM.
	

	// Alloocate and construct a VirtualMasterLoadVirtualDevice message.
	// Mark the VD with the rowID of the DiskDescriptor that is creating it.
	// TODO: try to find our failover partner's slot number
	pCreateBsaVDMsg = new RqOsVirtualMasterLoadVirtualDevice(
		"HDM_BSA", 									// Class Name of VD.
		Address::iSlotMe,							// Primary Slot.
		MyFOP,										// Secondary Slot
		false,										// fAutoStart
		RowId(m_BSAConfigRec.rid),					// rid of VD's Config Record
		RowId(pCC->pDMState->ridDD)					// Owner unique ID rid
	);

	// Check the pointer and...
	if (!pCreateBsaVDMsg)
		// Set an error if null.
		status = CTS_OUT_OF_MEMORY;
	else
		// Send the message off to the Virtual Master.
		status = Send(
			pCreateBsaVDMsg,
			pCC,										// void* pContext
			REPLYCALLBACK(DriveMonitorIsm, DM_Create_InstBSAVDReply)
		);
	
	// Cleanup in the event of any error.
	if (status != OK)
	{
		CheckFreeAndClear(pCreateBsaVDMsg);
		status = DM_Create_Bsa_End(pCC, status);			
	}
	
	return status;
} // DM_Create_InstBSAVD



/*************************************************************************/
// DM_Create_InstBSAVDReply
//
//  Description:
//    The virtual device entry has been created, the Virtual Device Number
//	  is now available for use.  Copy the VDN into the DiskDescriptor
//    record.
//
//  Inputs:
//    pMsg - The reply to our RqOsVirtualMasterLoadVirtualDevice MSG.
//
//  Outputs:
//    Returns OK, or a highly descriptive error code.
//
/*************************************************************************/
STATUS DriveMonitorIsm::DM_Create_InstBSAVDReply(Message* pMsg)
{
RqOsVirtualMasterLoadVirtualDevice*	pCreateBsaVDMsg;
STATUS								status;
DM_CR_CONTEXT*						pCC;

	TRACE_ENTRY(DM_Create_InstBSAVDReply);
	
	pCreateBsaVDMsg = (RqOsVirtualMasterLoadVirtualDevice *)pMsg;
	status = pCreateBsaVDMsg->Status();
	pCC = (DM_CR_CONTEXT *)pCreateBsaVDMsg->GetContext();

	if (status != OK)
	{
		TRACE_HEX(TRACE_L8, "\n\rDM_Create_Bsa_End: status = ", status);
		return DM_Create_Bsa_End(pCC, status);
	}
	
	// create(insert) was successful, add the BSA VDN to the DiskDescriptor record
	pCC->pDMState->pPD->vdnDdm = pCreateBsaVDMsg->GetVdn();

	// delete the msg.
	delete pCreateBsaVDMsg;

	// done with create BSA DDM
	return DM_Create_Bsa_End(pCC, ercOK);
	
} // DM_Create_InstBSAVDReply
		
		
/*************************************************************************/
// DM_Create_Bsa_End
// Complete the table row entry update. Call the Finish routine
//  Description:
//    The virtual device entry has been created, the Virtual Device Number
//	  is now available for use.  Unless there was an error, the VDN is in
//    the DiskDescriptor record for use now.
//    To complete this operation, we must call the Callback and delete our
//    context.
//
//  Inputs:
//    pClientContext - our create context
//
//    status - The returned status of the PTS operation.
//
//  Outputs:
//    Returns OK, or a highly descriptive error code.
//
/*************************************************************************/
STATUS	DriveMonitorIsm::DM_Create_Bsa_End(void *pClientContext, STATUS status)
{
	DM_CR_CONTEXT 		*pCC = (DM_CR_CONTEXT *)pClientContext;
	DM_DEVICE_STATE		*pDMState = pCC->pDMState;
	pDMCallback_t 		 Callback = pCC->Callback;

	TRACE_ENTRY(DM_Create_Bsa_End);
	
	if (status != ercOK)
	{
		TRACE_HEX(TRACE_L8, "\n\rDM_Create_Bsa_End: status = ", status);
	}
	else
	{
		TRACEF(TRACE_L3, ("\n\rDM: Created BSA Vdn %d ", pDMState->pPD->vdnDdm));
	}
	
	// Do a callback if there is one
	if (Callback)
		(this->*Callback)((void *)pCC->p, status);
	
	delete pCC;
	
	return ercOK;
	
} // DM_Create_Bsa_End


/*************************************************************************/
// DM_Find_Bsa_Device
// Check for the BSA VirtualDevice number for this disk, return 0 if not found.
// Return 1 if device was found, state will be updated with the VDN.
/*************************************************************************/
U32 DriveMonitorIsm::DM_Find_Bsa_Device(DM_DEVICE_STATE	*pDMState)
{
	TRACE_ENTRY(DM_Find_Bsa_Device);

	// check to see if we found a BSA for this device already
	if (pDMState->state & DEVICE_STATE_VDN_FOUND)
	{
		// clear the state flag, since we only need to do this once
		pDMState->state &= ~DEVICE_STATE_VDN_FOUND;
		
		// add the BSA VDN to the PathDescriptor record
		pDMState->pPD->vdnDdm = pDMState->Vdn;

		return(1);
	}
	
	// device not found
	return(0);
	
} // DM_Find_Bsa_Device




