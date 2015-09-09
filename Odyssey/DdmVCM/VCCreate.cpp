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
// File: VCCreate.cpp
//
// Description:
//    Virtual Circuit Manager DDM Create Command methods.
//
// $Log: /Gemini/Odyssey/DdmVCM/VCCreate.cpp $
// 
// 19    2/09/00 6:04p Dpatel
// Fix for Jerry's bug of not being able to export LUNs to initiators
// on OUR initiator loop
// 
// 18    11/22/99 10:37a Dpatel
// for Win32, chgd vdNext to 5
// 
// 17    11/12/99 9:49a Dpatel
// changes for win32 port of VCM
// 
// 16    11/06/99 2:16p Jlane
// Cleanup after using new VD creation message.  Remove timer and listen
// stuff.
// 
// 15    11/04/99 4:27p Jlane
// Use VDT_FIOPHASDID_FIELD and in general make compile with Tom's new VDT
// code.
// 
// 14    11/02/99 11:03a Dpatel
// set the next exp rec to null..
// 
// 13    11/01/99 5:07p Dpatel
// Export, unexport event generated after we hear the listen reply on the
// ready
// state field. 
// 
// 12    10/27/99 5:09p Dpatel
// vccreate needed to set fUsed to used in SRC table if create
// successful..
// 
// 11    10/08/99 12:55p Dpatel
// removed DelSTS_Cfg record in CleanupVC
// 
// 10    10/07/99 9:59a Dpatel
// changed didprimary/didsecondary to slot prim/sec
// 
// 9     10/06/99 8:33p Agusev
// checked for owner == 1 in validate..
// 
// 8     10/05/99 11:24a Agusev
// Many fixes made during initial bringup.
// 
// 6     10/02/99 3:46p Agusev
// some bug fixes - d.patel
// 
// 3     9/06/99 8:03p Jlane
// Yet another interim checkin.
// 
// 2     9/05/99 4:40p Jlane
// Compiles and is theoretically ready to try.
// 
// 1     9/05/99 4:38p Jlane
// INitial checkin.
// 
//
/*************************************************************************/

#include  <assert.h>          // debug stuff

#include "DdmVCM.h"
#include "CTEvent.h"

#include "RqOsVirtualMaster.h"
#include "Odyssey_Trace.h" 
#define TRACE_INDEX TRACE_VCM


//  DdmVCM::VirtualCircuitCreate (HANDLE hRequest, VCRequest *pRequest)
//
//  Description:
//    Called when our DDM is supposed to create a virtual Circuit.
//    Begin the process by reading the Specified Storage Roll Call Record.
//
//  Inputs:
//    hRequest - Handle of request we're call for.  We must return this
//                when we report request status.
//    pRequest - Request which we're to process.
//
//  Outputs:
//    Returns OK, or a highly descriptive error code.
//
STATUS DdmVCM::VirtualCircuitCreate(HANDLE hRequest, VCRequest *pRequest)
{
VCCommandContext_t*	pCmdInfo;
STATUS 				status = OK;

	// Fill out a command context structure to keep the command info for all the callbacks.
	pCmdInfo = new(tZERO) VCCommandContext_t;
	memcpy(&pRequest->rid, hRequest, sizeof(rowID));
	pCmdInfo->hRequest = hRequest;
	pCmdInfo->pRequest = pRequest;

	m_vdnSTS = 0;
	status = VCCreate_ReadEVCRecord( pCmdInfo, status );
	
	return status;
}



//  DdmVCM::VCCreate_ReadEVCRecord (void *pClientContext, STATUS status)
//
//  Description:
//    Continue creating a virtual Circuit. Read the Environmental Status Record
//    We'll need the mask of IOPs present later.
//
//  Inputs:
//    pClientContext - Actually a pointer to our command info structure
//    which contains a handle we return to report status and the request
//    we're processing.
//
//    status - The returned status of the PTS operation.
//
//  Outputs:
//    Returns OK, or a highly descriptive error code.
//
STATUS DdmVCM::VCCreate_ReadEVCRecord(void *pClientContext, STATUS status)
{
VCCommandContext_t*	pCmdInfo = (VCCommandContext_t*)pClientContext;

	if (status != OK)
		return CleanUpVCCreate(pCmdInfo, NULL, status);
	
	m_pReadSystemStatusRec = new TSReadTable;
	if (!m_pReadSystemStatusRec)
		status = CTS_OUT_OF_MEMORY;
	else
		status = m_pReadSystemStatusRec->Initialize( 
			this,										// DdmServices *pDdmServices,
			SYSTEM_STATUS_TABLE,						// String64 rgbTableName,
			(void **)&m_pSystemStatusRecord,			// void* *ppTableDataRet
			NULL,										// U32 *pcRowsReadRet,
			TSCALLBACK(DdmVCM,VCCreate_ReadSRCRecord),	// pTSCallback_t pCallback,
			pCmdInfo									// void* pContext
		);
	
	if (status == OK)
		m_pReadSystemStatusRec->Send();
	else
		status = CleanUpVCCreate(pCmdInfo, NULL, status);
	
	return status;	
}



//  DdmVCM::VCCreate_ReadSRCRecord (void *pClientContext, STATUS status)
//
//  Description:
//    Continue creating a virtual Circuit. Read the StorageRollCall Record
//    Specified in the Virtual, Circuit Create Command.
//
//  Inputs:
//    pClientContext - Actually a pointer to our command info structure
//    which contains a handle we return to report status and the request
//    we're processing.
//
//    status - The returned status of the PTS operation.
//
//  Outputs:
//    Returns OK, or a highly descriptive error code.
//
STATUS DdmVCM::VCCreate_ReadSRCRecord(void *pClientContext, STATUS status)
{
VCCommandContext_t*	pCmdInfo = (VCCommandContext_t*)pClientContext;

// ignore error for win32
#ifndef WIN32
	if (status != OK)
		return CleanUpVCCreate(pCmdInfo, &status, CTS_VCM_EVCSR_READ_ERROR);
#endif
	
	m_pReadSRCRow = new TSReadRow;
	if (!m_pReadSRCRow)
		status = CTS_OUT_OF_MEMORY;
	else
		status = m_pReadSRCRow->Initialize( 
			this,										// DdmServices *pDdmServices,
			STORAGE_ROLL_CALL_TABLE,					// String64 rgbTableName,
			CT_PTS_RID_FIELD_NAME,						// String64 rgbKeyFieldName,
			&pCmdInfo->pRequest->u.VCCreateParms.ridSRCElement,	// void *pKeyFieldValue,
			sizeof(rowID),								// U32 cbKeyFieldValue,
			&m_SRCRecord,								// void *prgbRowDataRet,
			sizeof(m_SRCRecord),						// U32 cbRowDataRetMax,
			NULL,										// U32 *pcRowsReadRet,
			TSCALLBACK(DdmVCM,VCCreate_ReadFCPDBT),		// pTSCallback_t pCallback,
			pCmdInfo									// void* pContext
		);
	
	if (status == OK)
		m_pReadSRCRow->Send();
	else
		status = CleanUpVCCreate(pCmdInfo, NULL, status);
		
	return status;
}



//  DdmVCM::VCCreate_ReadFCPDBT (void *pClientContext, STATUS status)
//
//  Description:
//    Continue creating a virtual Circuit. Read the 
//  Fibre Channel Port Database Table.
//
//  Inputs:
//    pClientContext - Actually a pointer to our command info structure
//    which contains a handle we return to report status and the request
//    we're processing.
//
//    status - The returned status of the PTS operation.
//
//  Outputs:
//    Returns OK, or a highly descriptive error code.
//
STATUS DdmVCM::VCCreate_ReadFCPDBT(void *pClientContext, STATUS status)
{
VCCommandContext_t*	pCmdInfo = (VCCommandContext_t*)pClientContext;

	if (status != OK)
		return CleanUpVCCreate(pCmdInfo, &status, CTS_VCM_SRCR_READ_ERROR);
	
	m_pReadFCPDBTable = new TSReadTable;
	if (!m_pReadFCPDBTable)
		status = CTS_OUT_OF_MEMORY;
	else
		status = m_pReadFCPDBTable->Initialize( 
			this,									// DdmServices *pDdmServices,
			FC_PORT_DATABASE_TABLE_NAME,			// String64 rgbTableName,
			(void **)&m_pFCPortDatabaseTable,				// void* *ppTableDataRet
			&m_cRowsFCPortDatabaseTable,			// U32 *pcRowsReadRet,
			TSCALLBACK(DdmVCM,VCCreate_ReadHCDR),	// pTSCallback_t pCallback,
			pCmdInfo								// void* pContext
		);
	
	if (status == OK)
		m_pReadFCPDBTable->Send();
	else
		status = CleanUpVCCreate(pCmdInfo, NULL, status);
	
	return status;	
}	// end of VCCreate_ReadFCPDBT



//  DdmVCM::VCCreate_ReadHCDR (void *pClientContext, STATUS status)
//
//  Description:
//    Continue creating a virtual Circuit. Read the 
//  HostConnectionDescriptorRecord(s).
//
//  Inputs:
//    pClientContext - Actually a pointer to our command info structure
//    which contains a handle we return to report status and the request
//    we're processing.
//
//    status - The returned status of the PTS operation.
//
//  Outputs:
//    Returns OK, or a highly descriptive error code.
//
STATUS DdmVCM::VCCreate_ReadHCDR(void *pClientContext, STATUS status)
{
VCCommandContext_t*	pCmdInfo = (VCCommandContext_t*)pClientContext;

	if (status != OK)
		return CleanUpVCCreate(pCmdInfo, &status, CTS_VCM_FCPDBT_READ_ERROR);

	m_pReadHCDRRow = new TSReadRow;
	if (!m_pReadHCDRRow)
		status = CTS_OUT_OF_MEMORY;
	else
		status = m_pReadHCDRRow->Initialize( 
			this,										// DdmServices *pDdmServices,
			HOST_CONNECTION_DESCRIPTOR_TABLE_NAME,		// String64 rgbTableName,
			CT_PTS_RID_FIELD_NAME,						// String64 rgbKeyFieldName,
			&pCmdInfo->pRequest->u.VCCreateParms.ridHCDR,			// void *pKeyFieldValue,
			sizeof(rowID),								// U32 cbKeyFieldValue,
			&m_HostConnDescRec,							// void *prgbRowDataRet,
			sizeof(HostConnectionDescriptorRecord),		// U32 cbRowDataRetMax,
			NULL,										// U32 *pcRowsReadRet,
			TSCALLBACK(DdmVCM,VCCreate_ReadLDR),		// pTSCallback_t pCallback,
			pCmdInfo									// void* pContext
		);
	
	if (status == OK)
		m_pReadHCDRRow->Send();
	else
		status = CleanUpVCCreate(pCmdInfo, NULL, status);
	
	// Initialize loop control variable for reading LoopDescriptor Records.									
	m_CurrLoopDescRecord = 0;
	
	return status;
}  // end of DdmVCM::VCCreate_ReadHCDR



//  DdmVCM::VCCreate_ReadLDR (void *pClientContext, STATUS status)
//
//  Description:
//    Continue creating a virtual Circuit. Read the LoopDescriptorRecord
//    associated with the most recently read HostConnectionDescriptor
//    record.  In oirder to do this we need to search FCPortDatabase for
//    the rowID specified in the most recently read HostConnectionDescriptor
//    record.  Once we find that FCPortDatabase record it will have the rowID
//    of the LoopDescriptor Record.
//
//  Inputs:
//    pClientContext - Actually a pointer to our command info structure
//    which contains a handle we return to report status and the request
//    we're processing.
//
//    status - The returned status of the PTS operation.
//
//  Outputs:
//    Returns OK, or a highly descriptive error code.
//
STATUS DdmVCM::VCCreate_ReadLDR(void *pClientContext, STATUS status)
{
VCCommandContext_t*	pCmdInfo = (VCCommandContext_t*)pClientContext;
bool				found = false;
U32					n;

	if (status != OK)
		// Note that we might have been called back resulting from a PTS msg sent in
		// VCCreate_ReadHCDR above or from the PTS Send we do here in a loop.  We can
		// know which by checking the value of m_CurrLoopDescRecord and return the
		// correct error code for either case - like it matters.
		if (m_CurrLoopDescRecord == 0)
			return CleanUpVCCreate(pCmdInfo, &status, CTS_VCM_HCDR_READ_ERROR);
		else
			return CleanUpVCCreate(pCmdInfo, &status, CTS_VCM_LDR_READ_ERROR);
	
	// If this is the first time through the Loop to read
	// LoopDescriptor Records then...
	if (m_CurrLoopDescRecord == 0)
	{
		// Allocate space for the LoopDescriptorRecord Records.
		m_pLoopDescriptorRecs = new LoopDescriptorRecord[m_HostConnDescRec.ridEIPCount];

		if (!m_pLoopDescriptorRecs)
			return CleanUpVCCreate(pCmdInfo, NULL, CTS_OUT_OF_MEMORY);
	}

	// Here begins a loop that reads LoopDescriptorRecords.	
	if (m_CurrLoopDescRecord < m_HostConnDescRec.ridEIPCount)
	{
		// Search the FC Port database for the rowID referenced in the current
		// HostConnectionDescriptor Record ridEIP array entry.
		for (n = 0; n < m_cRowsFCPortDatabaseTable; n++)
		{
			if (m_pFCPortDatabaseTable[n].rid == m_HostConnDescRec.ridEIPs[m_CurrLoopDescRecord])
			{
				// Save the Row ID of the LoopDescriptor from the 
				// EIP record in the FC Port Database Table.
				// WHY?  ridLoopDescriptor = m_pFCPortDatabaseTable[n].ridLoopDescriptor;
				found = true;
				break;
			}
		}	// end for
	
		// If we didn't find the FC Port database entry abort.
		if (!found)
			return CleanUpVCCreate(pCmdInfo, NULL, CTS_VCM_NO_FCPDBR_ERROR);
		
		// Otherwise read the Loop Descriptor Record referenced from the 
		// FCPort database record.  Also, bump our Loop Control Variable.
		m_pReadLDRow = new TSReadRow;
		if (!m_pReadLDRow)
			status = CTS_OUT_OF_MEMORY;
		else		
			status = m_pReadLDRow->Initialize( 
				this,											// DdmServices *pDdmServices,
				LOOP_DESCRIPTOR_TABLE,							// String64 rgbTableName,
				CT_PTS_RID_FIELD_NAME,							// String64 rgbKeyFieldName,
				&m_pFCPortDatabaseTable[n].ridLoopDescriptor,	// void *pKeyFieldValue,
				sizeof(rowID),									// U32 cbKeyFieldValue,
				&m_pLoopDescriptorRecs[m_CurrLoopDescRecord++],	// void *prgbRowDataRet,
				sizeof(LoopDescriptorRecord),					// U32 cbRowDataRetMax,
				NULL,											// U32 *pcRowsReadRet,
				TSCALLBACK(DdmVCM,VCCreate_ReadLDR),			// pTSCallback_t pCallback,
				pCmdInfo										// void* pContext
			);
		
		if (status == OK)
			m_pReadLDRow->Send();
		else
			status = CleanUpVCCreate(pCmdInfo, NULL, status);
										
	}
	else
		// At this point we've read all the Loop Descriptors in preparation
		// for Virtual Circuit Creation and can proceed.
		// Start by validating the Specified new Virtual Circuit.
		status = VCCreate_Validate(pCmdInfo, status);
		
	return status;
}

	
//  int	DdmVCM::VCCreate_FCPDBIndex( rowID	rid )
//
//  Description:
//    This is a utility that is used to lookup the record with the 
//  specified rowID in our local copy of the FCPortDatabase.
//
//  Inputs:
//    pClientContext - Actually a pointer to our command info structure
//    which contains a handle we return to report status and the request
//    we're processing.
//
//    status - The returned status of the PTS operation.
//
//  Outputs:
//    Returns -1, or an index into m_pFCPortDatabaseTable.
//
int	DdmVCM::VCCreate_FCPDBIndex( rowID	rid )
{
int	n;

	for (n = m_cRowsFCPortDatabaseTable; n >= 0; n--)
		// If this FCPortDatabase entry is the one then break out of the loop...
		if (m_pFCPortDatabaseTable[n].rid == rid)
			break;
	
	return n;
}	// end of 



//  DdmVCM::VCCreate_Validate (void *pClientContext, STATUS status)
//
//  Description:
//    Continue creating a Virtual Circuit. Validate the specified
//    Virtual Circuit Parameters.  
//
//  Inputs:
//    pClientContext - Actually a pointer to our command info structure
//    which contains a handle we return to report status and the request
//    we're processing.
//
//    status - The returned status of the PTS operation.
//
//  Outputs:
//    Returns OK, or a highly descriptive error code.
//
STATUS DdmVCM::VCCreate_Validate(void *pClientContext, STATUS status)
{
VCCommandContext_t*	pCmdInfo = (VCCommandContext_t*)pClientContext;
U32					iHostConnEIPArrayEntry, n, m;

	// In Preparation for our export operation clear our working
	// ExportRecord and SCSI Target Server Configuration records.
	memset( &m_ExportTableRecs, 0, sizeof(m_ExportTableRecs));
	memset( &m_SCSITargetConfigRec, 0, sizeof(m_SCSITargetConfigRec));
	
	// Loop for each entry in the Host Connection External Initiator Port
	// Array and make sure that this Virtual Circuit's specified 
	// Target ID is available for use.  Two conditions must be met:
	// 1) The TargetID must not be in use by someone else on the loop
	// (we'll search the FCPortDatabase to verify that) and
	// 2) Either, we must already be exporting the targetID, OR
	// we must currently be exporting less than 32 target IDs.
	U32	newTargetID = pCmdInfo->pRequest->u.VCCreateParms.TargetID;
	bool	fFoundTargetID = false;
	for (iHostConnEIPArrayEntry = 0; iHostConnEIPArrayEntry < m_HostConnDescRec.ridEIPCount; iHostConnEIPArrayEntry++)
	{
		fFoundTargetID = false;
		// Start by verifying requirement 1 from the comment above:
		// Loop through the FC Port Database and make sure that there isn't
		// already a target with our new Virtual Circuit's ID.
		for (n = 0; n < m_cRowsFCPortDatabaseTable; n++)
		{
			if (m_pFCPortDatabaseTable[n].rid == m_HostConnDescRec.ridEIPs[iHostConnEIPArrayEntry]){
				if (!(m_pFCPortDatabaseTable[n].attribs & FC_PORT_OWNER_VC_USE_OK)) {				
					// reject if its an initiator loop
					return CleanUpVCCreate(pCmdInfo, &iHostConnEIPArrayEntry, CTS_VCM_ATTEMPT_TO_USE_INITITATOR_PORT);
				}
			}
		
			// If this FCPortDatabase entry is on the current loop...
			if (m_pFCPortDatabaseTable[n].ridLoopDescriptor == m_pLoopDescriptorRecs[iHostConnEIPArrayEntry].rid)
			{
				// and it has the same ID ...
				if (m_pFCPortDatabaseTable[n].id == newTargetID) {
				
					if (!(m_pFCPortDatabaseTable[n].attribs & FC_PORT_OWNER_INTERNAL)) {				
						// Then we have a problem!  Note that we return the index as a detail.
						return CleanUpVCCreate(pCmdInfo, &iHostConnEIPArrayEntry, CTS_VCM_TARGETID_IN_USE);
					}					
				}				
			}
		}	// end for (n...
		
		// Now, verify that either we are already exporting this Target ID, OR
		// that can export this target ID
		for (m = 0; m < m_pLoopDescriptorRecs[iHostConnEIPArrayEntry].IDsInUse; m++)
			if (m_pLoopDescriptorRecs[iHostConnEIPArrayEntry].TargetIDs[n] == newTargetID)
			{
				fFoundTargetID = true;
				break;
			}
			
		// If we didn't find the target ID then ...
		if (!fFoundTargetID)
			// If there's no room to add it then abort.
			if (m_pLoopDescriptorRecs[iHostConnEIPArrayEntry].IDsInUse == 32)
				// Note that we return the index as a detail.
				return CleanUpVCCreate(pCmdInfo, &iHostConnEIPArrayEntry, CTS_VCM_TOO_MANY_TARGETS);
			#if false
			// Turns out that the Loop Monitor wilol take care of all this upon
			// detecting the insertion of the Export Table Record. 
			else
			{
				// Otherwise add it to the Loop Descriptor
				// and flag the loop Descriptor for updating.
				m_HostConnDescRec.flgEIPs[iHostConnEIPArrayEntry] |= mskfUpdateLoopDesc;
				m_pLoopDescriptorRecs[iHostConnEIPArrayEntry].TargetIDs[m_pLoopDescriptorRecs[iHostConnEIPArrayEntry].IDsInUse++] = newTargetID;
			}
			#endif
			
	}  // end for (iHostConnEIPArrayEntry = 0...
	
	// If all was well begin actual circuit creation and export:
	// OK, so here we are ready to start doing the actual export...
	// of the specified storage roll call element ...
	// as the specified TragetID and LUN [optionally only to the specified InitiatorID]...
	// to the specified External Initiator Ports (Note the plural "PORTS")...
	// which exists on one of our Loop Instances (as described by a Loop Descriptor).
	// OK, SO.  What we need to do is:
	// for each of pair of EIP entries...
	// 1) configure and instantiate one SCSI Target server Virtual Device and...
	// 2) Make two export table entries 
	
	// Note that here begins a loop that spans several callbacks.
	// The Loop will configure an d
	m_CurrEIPArrayEntry = 0;
	
	return VCCreate_Check4STSCfg (pCmdInfo, status);
}	// end of VCCreate_Validate



//  DdmVCM::VCCreate_Check4STSCfg (void *pClientContext, STATUS status)
//
//  Description:
//    Continue creating a virtual Circuit. Check if the SCSI Target Server
//    config record we are about to create already exists.  This may be
//    the case if our redundant counterpart has previously failed during
//    the execution of the current Create Virtual Circuit command.  If
//    this is the case, then we may find the SCSI Target Server config
//    record already present flagged with the Row ID of the current command.
//
//  Inputs:
//    pClientContext - Actually a pointer to our command info structure
//    which contains a handle we return to report status and the request
//    we're processing.
//
//    status - The returned status of the previous PTS operation.
//
//  Outputs:
//    Returns OK, or a highly descriptive error code.
//
STATUS DdmVCM::VCCreate_Check4STSCfg(void *pClientContext, STATUS status)
{
VCCommandContext_t*	pCmdInfo = (VCCommandContext_t*)pClientContext;

	if (m_CurrEIPArrayEntry < m_HostConnDescRec.ridEIPCount)
	{
	
		// Try to read a SCSI Target Server flagged by our current 
		// Create Virtual Circuit command's Row ID.
		m_pReadSTSCfgRec = new TSReadRow;
		if (!m_pReadSTSCfgRec)
			status = CTS_OUT_OF_MEMORY;
		else		
			status = m_pReadSTSCfgRec->Initialize( 
				this,										// DdmServices *pDdmServices,
				STS_CONFIG_TABLE_NAME,						// String64 rgbTableName,
				RID_VC_ID,									// String64 rgbKeyFieldName,
				&pCmdInfo->pRequest->rid,					// void *pKeyFieldValue,
				sizeof(rowID),								// U32 cbKeyFieldValue,
				&m_SCSITargetConfigRec,						// void *prgbRowDataRet,
				sizeof(m_SCSITargetConfigRec),				// U32 cbRowDataRetMax,
				NULL,										// U32 *pcRowsReadRet,
				TSCALLBACK(DdmVCM,VCCreate_InsertSTSCfg),	// pTSCallback_t pCallback,
				pCmdInfo									// void* pContext
			);
		
		if (status == OK)
			m_pReadSTSCfgRec->Send();
		else
			status = CleanUpVCCreate(pCmdInfo, NULL, status);
	}
	else
	{
		// end of loop, now what? .. We're done?
		status = CleanUpVCCreate(pCmdInfo, &pCmdInfo->pRequest->rid, status);
	}
	
	return status;
}	// VCCreate_Check4STSCfg



//  DdmVCM::VCCreate_InsertSTSCfg (void *pClientContext, STATUS status)
//
//  Description:
//    Continue creating a virtual Circuit. Insert a SCSI Target
//    Server configuration record into the STS config table if it was
//    not read bythe previous VCCreate_Check4STSCfg method's readrow
//    operation.
//
//  Inputs:
//    pClientContext - Actually a pointer to our command info structure
//    which contains a handle we return to report status and the request
//    we're processing.
//
//    status - The returned status of the previous PTS operation.
//
//  Outputs:
//    Returns OK, or a highly descriptive error code.
//
STATUS DdmVCM::VCCreate_InsertSTSCfg(void *pClientContext, STATUS status)
{
VCCommandContext_t*	pCmdInfo = (VCCommandContext_t*)pClientContext;

	if (status == OK)
		// We were able to read the STS Config record so continue with
		// the VCCreate operation.
		return VCCreate_Check4STSVD( pCmdInfo, status );

	// If there was any error other than the record not being found
	// abort the current command and return the error.
	if (status != OK && status != ercKeyNotFound && status != ercEOF)			// BAD ERROR CODE.
		return CleanUpVCCreate(pCmdInfo, NULL, status);
		
	// Otherwise insert the STS Config Record.	
	{
		// Initialize our STS configuration record config record.  
		m_SCSITargetConfigRec.version = STS_TABLE_VERSION;
		m_SCSITargetConfigRec.size = sizeof(STS_CONFIG);

		// The STS will have the SRC entry's Virtual Device as it's vdNext.
#ifdef WIN32
		m_SCSITargetConfigRec.vdNext = 5;
#else
		m_SCSITargetConfigRec.vdNext = m_SRCRecord.vdnBSADdm;
#endif
		
		// Mark the STS Config Record with the rowID of the command that created it.
		m_SCSITargetConfigRec.ridVcId = pCmdInfo->pRequest->rid;

		// Insert the new STS Config record in the table of same.
		m_pInsertSTSConfigRec = new TSInsertRow;
		if (!m_pInsertSTSConfigRec)
			status = CTS_OUT_OF_MEMORY;
		else		
			status = m_pInsertSTSConfigRec->Initialize(
				this,									// DdmServices *pDdmServices,
				STS_CONFIG_TABLE_NAME,					// String64 rgbTableName,
				&m_SCSITargetConfigRec,					// void *prgbRowData,
				sizeof(m_SCSITargetConfigRec),			// U32 cbRowData,
				&m_SCSITargetConfigRec.rid,				// rowID *prowIDRet,
				TSCALLBACK(DdmVCM,VCCreate_Check4STSVD),// pTSCallback_t pCallback,
				pCmdInfo								// void* pContext
			);
	
		if (status == OK)
			m_pInsertSTSConfigRec->Send();
		else
			status = CleanUpVCCreate(pCmdInfo, NULL, status);			
	}
			
	return status;
}	// VCCreate_InsertSTSCfg



//  DdmVCM::VCCreate_Check4STSVD (void *pClientContext, STATUS status)
//
//  Description:
//    Continue creating a virtual Circuit. Check if the Virtual Device 
//    Table record we are about to create already exists.  This may be
//    the case if our redundant counterpart has previously failed during
//    the execution of the current Create Virtual Circuit command.  If
//    this is the case, then we may find the Virtual Device Table
//    record already present flagged with the Row ID of the current command.
//
//  Inputs:
//    pClientContext - Actually a pointer to our command info structure
//    which contains a handle we return to report status and the request
//    we're processing.
//
//    status - The returned status of the previous PTS operation.
//
//  Outputs:
//    Returns OK, or a highly descriptive error code.
//
STATUS DdmVCM::VCCreate_Check4STSVD(void *pClientContext, STATUS status)
{
VCCommandContext_t*	pCmdInfo = (VCCommandContext_t*)pClientContext;

	if (status != OK)
		return CleanUpVCCreate(pCmdInfo, &status, CTS_VCM_STS_CFG_INSERT_ERROR);
	
	// Try to read a SCSI Target Server flagged by our current 
	// Create Virtual Circuit command's Row ID.
	m_pReadVDTRec = new TSReadRow;
	if (!m_pReadVDTRec)
		status = CTS_OUT_OF_MEMORY;
	else		
		status = m_pReadVDTRec->Initialize( 
			this,									// DdmServices *pDdmServices,
			PTS_VIRTUAL_DEVICE_TABLE,				// String64 rgbTableName,
			VDT_RID_VDOWNERUSE_FIELD,				// String64 rgbKeyFieldName,
			&pCmdInfo->pRequest->rid,				// void *pKeyFieldValue,
			sizeof(rowID),							// U32 cbKeyFieldValue,
			&m_NewVDTRecord,						// void *prgbRowDataRet,
			sizeof(m_NewVDTRecord),					// U32 cbRowDataRetMax,
			NULL,									// U32 *pcRowsReadRet,
			TSCALLBACK(DdmVCM,VCCreate_InstSTSVD),	// pTSCallback_t pCallback,
			pCmdInfo								// void* pContext
		);
	
	if (status == OK)
		m_pReadVDTRec->Send();
	else
		status = CleanUpVCCreate(pCmdInfo, NULL, status);

	return status;
}	// VCCreate_Check4STSVD



//  DdmVCM::VCCreate_InstSTSVD (void *pClientContext, STATUS status)
//
//  Description:
//    Continue creating a virtual Circuit. Instantiate the SCSI Target
//    Server Virtual Device by Inserting a SCSI Target Server VD Record
//    into the Virtual Device table if it was not read by the previous
//    VCCreate_Check4STSVD method's readrow operation.
//
//  Inputs:
//    pClientContext - Actually a pointer to our command info structure
//    which contains a handle we return to report status and the request
//    we're processing.
//
//    status - The returned status of the previous PTS operation.
//
//  Outputs:
//    Returns OK, or a highly descriptive error code.
//
STATUS DdmVCM::VCCreate_InstSTSVD(void *pClientContext, STATUS status)
{
VCCommandContext_t*	pCmdInfo = (VCCommandContext_t*)pClientContext;

	if ((status != ercKeyNotFound) && (status != ercEOF) && (status != OK))
		return CleanUpVCCreate(pCmdInfo, &status, CTS_VCM_STS_VD_CHECK_ERROR);
		
	if (status == OK)
		// We were able to read the Virtual Device Table Record so continue with
		// the VCCreate operation.
		return VCCreate_DelExpRecs( pCmdInfo, status );
#ifndef WIN32
	RqOsVirtualMasterLoadVirtualDevice *pMsg = new RqOsVirtualMasterLoadVirtualDevice(
									"HDM_STS", 
									IOP_LOCAL,
									IOP_LOCAL,
									true,								// fAutoStart
									RowId(m_SCSITargetConfigRec.rid),	// cfg rid
									RowId(pCmdInfo->pRequest->rid));	// owner rid
	status = Send(
		pMsg,
		pCmdInfo,
		REPLYCALLBACK(DdmVCM,VCCreate_ProcessVDCreateReply));
		
	return status;
#else
	status = VCCreate_DelExpRecs( pCmdInfo, OK );
	return status;
#endif
}


STATUS DdmVCM::VCCreate_ProcessVDCreateReply(Message* pMsg)
{
TRACE_ENTRY(DdmVCM::ProcessVirtualDeviceCreateReply());
RqOsVirtualMasterLoadVirtualDevice *pCreateStsVDMsg = (RqOsVirtualMasterLoadVirtualDevice *)pMsg;
STATUS	status = pCreateStsVDMsg->Status();
VCCommandContext_t*	pCmdInfo = (VCCommandContext_t*)pCreateStsVDMsg->GetContext();

	if (status != OK) {
		status = CleanUpVCCreate(pCmdInfo, &status, CTS_VCM_STS_VD_INSERT_ERROR);
	} else {
		m_vdnSTS = pCreateStsVDMsg->GetVdn();
		status = VCCreate_DelExpRecs( pCmdInfo, OK );	
	}
	delete pCreateStsVDMsg;
	return status;
}




//  DdmVCM::VCCreate_DelExpRecs (void *pClientContext, STATUS status)
//
//  Description:
//    Continue creating a virtual Circuit. Attempt to delete the Export
//    record(s) we are about to create in case they already exist.  
//    This may be the case if our redundant counterpart has previously
//    failed during the execution of the current Create Virtual Circuit
//    command.  If this is the case, then we may find the Export Table 
//    record already present flagged with the Row ID of the current 
//    command.  Rather than attempting to read existing records wwe will
//	  Delete them to make sure we wil create npo duplicates. 
//
//  Inputs:
//    pClientContext - Actually a pointer to our command info structure
//    which contains a handle we return to report status and the request
//    we're processing.
//
//    status - The returned status of the previous PTS operation.
//
//  Outputs:
//    Returns OK, or a highly descriptive error code.
//
STATUS DdmVCM::VCCreate_DelExpRecs(void *pClientContext, STATUS status)
{
VCCommandContext_t*	pCmdInfo = (VCCommandContext_t*)pClientContext;

	// THIS SHOULD NEVER BE POSSIBLE
	if (status != OK)
		return CleanUpVCCreate(pCmdInfo, &status, status);
	
	// Try to read a SCSI Target Server flagged by our current 
	// Create Virtual Circuit command's Row ID.
	m_pDeleteExpRecs = new TSDeleteRow;
	if (!m_pDeleteExpRecs)
		status = CTS_OUT_OF_MEMORY;
	else		
		status = m_pDeleteExpRecs->Initialize( 
			this,									// DdmServices *pDdmServices,
			EXPORT_TABLE,							// String64 rgbTableName,
			RID_VC_ID,								// String64 rgbKeyFieldName,
			&pCmdInfo->pRequest->rid,				// void *pKeyFieldValue,
			sizeof(rowID),							// U32 cbKeyFieldValue,
			0,										// U32 cRowsToDelete,		
			NULL,									// U32 *pcRowsDelRet,
			TSCALLBACK(DdmVCM,VCCreate_InsExpRecs),	// pTSCallback_t pCallback,
			pCmdInfo								// void* pContext
		);
	
	if (status == OK)
		m_pDeleteExpRecs->Send();
	else
		status = CleanUpVCCreate(pCmdInfo, NULL, status);

	return status;
}	// VCCreate_DelExpRecs



//  DdmVCM::VCCreate_InsExpRecs(void *pClientContext, STATUS status)
//
//  Description:
//    Continue creating a virtual Circuit. Create the Export Records
//   and insert them into the Export Table.
//
//  Inputs:
//    pClientContext - Actually a pointer to our command info structure
//    which contains a handle we return to report status and the request
//    we're processing.
//
//    status - The returned status of the PTS operation.
//
//  Outputs:
//    Returns OK, or a highly descriptive error code.
//
STATUS DdmVCM::VCCreate_InsExpRecs(void *pClientContext, STATUS status)
{
VCCommandContext_t*	pCmdInfo = (VCCommandContext_t*)pClientContext;

	// If the previous delete operation failed for anything other than
	// lack of finding the specified record(s) then we have an error
	// so abort the VC create command.
	if (status != OK && status != ercKeyNotFound)
		return CleanUpVCCreate(pCmdInfo, NULL, status);

	// Initialize our primary Export Record.
	m_ExportTableRecs[0].version = EXPORT_TABLE_VERSION;		// Version of Export Table record.
	m_ExportTableRecs[0].size = sizeof(ExportTableRecord);		// Size of Export Table record in bytes.
	// Mark the VC's Export Record with the rowID of the command that created it.
	m_ExportTableRecs[0].ridVcId = pCmdInfo->pRequest->rid;
	m_ExportTableRecs[0].ProtocolType = ProtocolFibreChannel;	// FCP, IP, other
	m_ExportTableRecs[0].vdNext = m_vdnSTS;						// First Virtual Device number in the chain
	// The next two will likely disappear soon.
	m_ExportTableRecs[0].vdLegacyBsa = 0;						// Virtual Device number of the legacy BSA
	m_ExportTableRecs[0].vdLegacyScsi = 0;						// Virtual Device number of the legacy SCSI
	m_ExportTableRecs[0].ExportedLUN = pCmdInfo->pRequest->u.VCCreateParms.LUN;	// LUN number exported
	 
	if (pCmdInfo->pRequest->u.VCCreateParms.fExport2AllInitiators)
		m_ExportTableRecs[0].InitiatorId = -1;
	else
	{
		int iFCBPDBIndex = VCCreate_FCPDBIndex( m_HostConnDescRec.ridEIPs[m_CurrEIPArrayEntry] );
		
		if (iFCBPDBIndex == -1)
			// Throw an error;
			return CleanUpVCCreate(pCmdInfo, NULL, CTS_VCM_EIP_RID_INVALID_ERROR);
		else
			m_ExportTableRecs[0].InitiatorId = m_pFCPortDatabaseTable[iFCBPDBIndex].id;
	}
		
	
	m_ExportTableRecs[0].TargetId = pCmdInfo->pRequest->u.VCCreateParms.TargetID;	// Our ID
	m_ExportTableRecs[0].FCInstance = m_pLoopDescriptorRecs[0].LoopNumber;	// FC Loop number
	m_ExportTableRecs[0].SerialNumber[0] = 0;						// Use a string array for Serial Number
	m_ExportTableRecs[0].Capacity = m_SRCRecord.Capacity;		// Capacity of this Virtual Circuit
	m_ExportTableRecs[0].FailState = 0;
	m_ExportTableRecs[0].ReadyState = StateConfiguring;			// Current state
	m_ExportTableRecs[0].DesiredReadyState = StateConfiguring;	// Invalid until rids are set.
	#if 0
	m_ExportTableRecs[0].WWNName;			// World Wide Name (64 or 128-bit IEEE registered)
	#endif
	m_ExportTableRecs[0].ridUserInfo = pCmdInfo->pRequest->u.VCCreateParms.ridUserInfo;		// rowID of user info for this VC
	m_ExportTableRecs[0].ridHC = m_HostConnDescRec.rid;
#if 0	
	m_ExportTableRecs[0].ridStatusRec.Table = 0;
	m_ExportTableRecs[0].ridStatusRec.HiPart = 0;
	m_ExportTableRecs[0].ridStatusRec.LoPart = 0;
	m_ExportTableRecs[0].ridPerformanceRec.Table = 0;
	m_ExportTableRecs[0].ridPerformanceRec.HiPart = 0;
	m_ExportTableRecs[0].ridPerformanceRec.LoPart = 0;
#endif	
	m_ExportTableRecs[0].ridAltExportRec.Table = 0;
	m_ExportTableRecs[0].ridAltExportRec.HiPart = 0;
	m_ExportTableRecs[0].ridAltExportRec.LoPart = 0;
	m_ExportTableRecs[0].ridSRC = pCmdInfo->pRequest->u.VCCreateParms.ridSRCElement;

#if 0	
	// Initialize our secondary Export Record.
	m_ExportTableRecs[1].version = EXPORT_TABLE_VERSION;		// Version of Export Table record.
	m_ExportTableRecs[1].size = sizeof(ExportTableRecord);		// Size of Export Table record in bytes.
	// Mark the VC's Export Record with the rowID of the command that created it.
	m_ExportTableRecs[1].ridVcId = pCmdInfo->pRequest->rid;
	m_ExportTableRecs[1].ProtocolType = ProtocolFibreChannel;	// FCP, IP, other
	m_ExportTableRecs[1].vdNext = m_NewVDTRecord.rid.LoPart;	// First Virtual Device number in the chain
	// The next two will likely disappear soon.
	m_ExportTableRecs[1].vdLegacyBsa =0;						// Virtual Device number of the legacy BSA
	m_ExportTableRecs[1].vdLegacyScsi = 0;						// Virtual Device number of the legacy SCSI
	m_ExportTableRecs[1].ExportedLUN = pCmdInfo->pRequest->u.VCCreateParms.LUN;	// LUN number exported
	// Host ID
	if (pCmdInfo->pRequest->u.VCCreateParms.fExport2AllInitiators)
		m_ExportTableRecs[1].InitiatorId = -1;
	else
	{
		int iFCBPDBIndex = VCCreate_FCPDBIndex( m_HostConnDescRec.ridEIPs[m_CurrEIPArrayEntry+1] );
		
		if (iFCBPDBIndex == -1)
			// Throw an error;
			return CleanUpVCCreate(pCmdInfo, NULL, CTS_VCM_EIP_RID_INVALID_ERROR);
		else
			m_ExportTableRecs[1].InitiatorId = m_pFCPortDatabaseTable[iFCBPDBIndex].id;
	}
	
	m_ExportTableRecs[1].TargetId = pCmdInfo->pRequest->u.VCCreateParms.TargetID;	// Our ID
	m_ExportTableRecs[1].FCInstance = m_pLoopDescriptorRecs[1].LoopNumber;	// FC Loop number
	m_ExportTableRecs[1].SerialNumber[0] = 0;						// Use a string array for Serial Number
	m_ExportTableRecs[1].Capacity = m_SRCRecord.Capacity;		// Capacity of this Virtual Circuit
	m_ExportTableRecs[1].FailState = 0;
	m_ExportTableRecs[1].ReadyState = StateConfiguring;			// Current state
	m_ExportTableRecs[1].DesiredReadyState = StateConfiguring;	// Invalid until rids are set.
	#if false
	m_ExportTableRecs[1].WWNName;			// World Wide Name (64 or 128-bit IEEE registered)
	#endif
	m_ExportTableRecs[1].ridUserInfo = pCmdInfo->pRequest->u.VCCreateParms.ridUserInfo;		// rowID of user info for this VC
	m_ExportTableRecs[1].ridHC = m_HostConnDescRec.rid;
	#if false
	m_ExportTableRecs[1].ridStatusRec.Table = 0;
	m_ExportTableRecs[1].ridStatusRec.HiPart = 0;
	m_ExportTableRecs[1].ridStatusRec.LoPart = 0;
	m_ExportTableRecs[1].ridPerformanceRec.Table = 0;
	m_ExportTableRecs[1].ridPerformanceRec.HiPart = 0;
	m_ExportTableRecs[1].ridPerformanceRec.LoPart = 0;
	m_ExportTableRecs[1].ridAltExportRec.Table = 0;
	m_ExportTableRecs[1].ridAltExportRec.HiPart = 0;
	m_ExportTableRecs[1].ridAltExportRec.LoPart = 0;
	#endif
	m_ExportTableRecs[1].ridSRC = pCmdInfo->pRequest->u.VCCreateParms.ridSRCElement;
#endif

	// Insert the new Export record into the export table.
	m_pInsertExportRecs = new TSInsertRow;
	if (!m_pInsertExportRecs)
		status = CTS_OUT_OF_MEMORY;
	else		
		status = m_pInsertExportRecs->Initialize(
			this,									// DdmServices *pDdmServices,
			EXPORT_TABLE,							// String64 rgbTableName,
			&m_ExportTableRecs,						// void *prgbRowData,
			sizeof(m_ExportTableRecs),				// U32 cbRowData,
			(rowID*)&m_ExportRecsRowIDs,					// rowID *prowIDRet,
			TSCALLBACK(DdmVCM,VCCreate_ExportVC),	// pTSCallback_t pCallback,
			pCmdInfo								// void* pContext
		);

	if (status == OK)
		m_pInsertExportRecs->Send();
	else
		status = CleanUpVCCreate(pCmdInfo, NULL, status);

	// Initialize loop control variable used in VCCreate_ExportVC
	m_iExportRecToUpdate = 0;
	
	return status;
}	// end of VCCreate_InsExpRecs


//  DdmVCM::VCCreate_ExportVC(void *pClientContext, STATUS status)
//
//  Description:
//    Continue creating a virtual Circuit. Export the Virtual Circuit
//    by modifying the newly inserted Export Records to contian each
//    other's row ID.  Also update the dsesired state to "configured"
//    or "ConfiguredNotExported" as specified in the command.
//
//  Inputs:
//    pClientContext - Actually a pointer to our command info structure
//    which contains a handle we return to report status and the request
//    we're processing.
//
//    status - The returned status of the PTS operation.
//
//  Outputs:
//    Returns OK, or a highly descriptive error code.
//
STATUS DdmVCM::VCCreate_ExportVC(void *pClientContext, STATUS status)
{
VCCommandContext_t*	pCmdInfo = (VCCommandContext_t*)pClientContext;

	if (status != OK)
		return CleanUpVCCreate(pCmdInfo, &status, CTS_VCM_EXPORT_REC_INSERT_ERROR);

	if (m_iExportRecToUpdate < 1)
	{
		// Determine who is the "other" export record
		U32 iOtherExportRec = (m_iExportRecToUpdate == 0) ? 1 : 0;
	
		// Update our local copies of the new Export Record with its own and its alternate's row ID.
		m_ExportTableRecs[m_iExportRecToUpdate].rid =  m_ExportRecsRowIDs[m_iExportRecToUpdate];
		// Resolve , put next exp rec's rid
		//m_ExportTableRecs[m_iExportRecToUpdate].ridAltExportRec = m_ExportRecsRowIDs[iOtherExportRec];
		m_ExportTableRecs[m_iExportRecToUpdate].ridAltExportRec.Table = 0;
		m_ExportTableRecs[m_iExportRecToUpdate].ridAltExportRec.LoPart = 0;
		m_ExportTableRecs[m_iExportRecToUpdate].ridAltExportRec.HiPart = 0;
	
		// Update the DesiredState fields
		if (pCmdInfo->pRequest->u.VCCreateParms.fExport){
			m_ExportTableRecs[m_iExportRecToUpdate].ReadyState = StateConfigured;	// Desired Ready state
			RegisterExportTableListener(&pCmdInfo->pRequest->rid, StateConfiguredAndExported);
		} else {
			m_ExportTableRecs[m_iExportRecToUpdate].ReadyState = StateConfiguredNotExported;	// Desired Ready state
		}
			
		// stash a copy of the export record in the reply structure.
		m_VCCreateResult.VCExportRecRet[m_iExportRecToUpdate] = m_ExportTableRecs[m_iExportRecToUpdate];

		m_pModifyExportRec = new TSModifyRow;
		if (!m_pModifyExportRec)
			status = CTS_OUT_OF_MEMORY;
		else		
			status = m_pModifyExportRec->Initialize(
				this,										// DdmServices *pDdmServices,
				EXPORT_TABLE,								// String64 rgbTableName,
				CT_PTS_RID_FIELD_NAME,						// String64 rgbKeyFieldName,
				&m_ExportRecsRowIDs[m_iExportRecToUpdate],	// void* pKeyFieldValue,
				sizeof(rowID),								// U32 cbKeyFieldValue,
				&m_ExportTableRecs[m_iExportRecToUpdate],	// void *prgbRowData,
				sizeof(ExportTableRecord),					// U32 cbRowData,
				0,											// U32 cRowsToModify,
				NULL,										// U32* pcRowsModifiedRet,
				NULL,										// rowID *prowIDRet,
				0,											// U32 cbMaxRowID,
				TSCALLBACK(DdmVCM,VCCreate_ExportVC),		// pTSCallback_t pCallback,
				pCmdInfo									// void* pContext
			);
	
		m_iExportRecToUpdate++;
		if (status == OK)
			m_pModifyExportRec->Send();
		else
			status = CleanUpVCCreate(pCmdInfo, NULL, status);
	}
	else
	{
		// So As I'm seeing it we are now done with the create
		// update the SRC record's fUsed field
		status = VCCreate_ModifySRCTable(pCmdInfo, status);
	}
	
	return status;
}	// end of VCCreate_ExportVC


//  DdmVCM::VCCreate_ModifySRCTable (void* pClientContext, STATUS status)
//
//  Description:
//    Update the SRC tables fUsed field so that this storage becomes used
//
//  Inputs:
//    pClientContext - A pointer to our command info structure which
//    contains a handle we return to report status and the request
//    we're processing.
//
//
//    status - The returned status of the PTS operation.
//
//  Outputs:
//    returns the passed in status.
//
STATUS DdmVCM::VCCreate_ModifySRCTable(void *pClientContext, STATUS status)
{
VCCommandContext_t*	pCmdInfo = (VCCommandContext_t*)pClientContext;

		m_SRCRecord.fUsed = TRUE;
		
		TSModifyField *pModifySRCRecord = new TSModifyField;
		if (!pModifySRCRecord)
			status = CTS_OUT_OF_MEMORY;
		else		
			status = pModifySRCRecord->Initialize(
				this,										// DdmServices *pDdmServices,
				STORAGE_ROLL_CALL_TABLE,					// String64 rgbTableName,
				CT_PTS_RID_FIELD_NAME,						// String64 rgbKeyFieldName,
				&pCmdInfo->pRequest->u.VCCreateParms.ridSRCElement,	// void *pKeyFieldValue,
				sizeof(rowID),
				fdSRC_FUSED,
				&m_SRCRecord.fUsed,
				sizeof(m_SRCRecord.fUsed),
				0,											// U32 cRowsToModify,
				NULL,										// U32* pcRowsModifiedRet,
				NULL,										// rowID *prowIDRet,
				0,											// U32 cbMaxRowID,
				TSCALLBACK(DdmVCM,VCCreate_SRCTableModifiedReply),		// pTSCallback_t pCallback,
				pCmdInfo									// void* pContext
		);

		if (status == OK)
			pModifySRCRecord->Send();
		else
			status = CleanUpVCCreate(pCmdInfo, NULL, status);
		return status;			
}


STATUS DdmVCM::VCCreate_SRCTableModifiedReply(void *pClientContext, STATUS status)
{
VCCommandContext_t*	pCmdInfo = (VCCommandContext_t*)pClientContext;
	status = CleanUpVCCreate(pCmdInfo, &m_VCCreateResult, status);
	return status;
}



//  DdmVCM::CleanUpVCCreate (VCCommandContext_t* pCmdInfo, void* pReplyData, STATUS status)
//
//  Description:
//    Finish processing a Create Virtual Circuit command. Reply to the 
// command with the speciofied status and data and free all resources.  
//
//  Inputs:
//    pClientContext - A pointer to our command info structure which
//    contains a handle we return to report status and the request
//    we're processing.
//
//    pReplyData - Data returned along with the reply to the requestor
//
//    status - The returned status of the PTS operation.
//
//  Outputs:
//    returns the passed in status.
//
STATUS DdmVCM::CleanUpVCCreate (VCCommandContext_t* pCmdInfo, void* pReplyData, STATUS status)
{
	if (pCmdInfo)
	{
		VCStatus	myStatus;
		
		// Finish filling out the reply structure of the create command.
		myStatus.rid.HiPart = 0;
		myStatus.rid.LoPart = 0;
		myStatus.rid.Table = 0;
		myStatus.version = 1;
		myStatus.size = sizeof(VCStatus);
		// The new Virtual Circuit's ID.  The row ID of the command that created it.
		myStatus.ridVcId = pCmdInfo->pRequest->rid;
		myStatus.eVCEvent = VCCreated;
		myStatus.status = status;
		memcpy( &myStatus.u,
				pReplyData,
				(status == OK)
				 ? sizeof(VCCreateResult)
				 : sizeof(STATUS)
			  );
		
		m_CmdServer.csrvReportCmdStatus(pCmdInfo->hRequest,
										status,
										&myStatus,
										pCmdInfo->pRequest);
	
		if (status == OK)
			m_CmdServer.csrvReportEvent( status, &myStatus );

#ifdef WIN32
		// fake the modify field in the export table
		if (pCmdInfo->pRequest->u.VCCreateParms.fExport){
			SimulateExportTableState(
				1, 
				&m_ExportRecsRowIDs[0], 
				StateConfiguredAndExported);
		}
#endif
	}
	
	// Need to make sure it does'nt stick back in a loop
	//if (status != OK)
	//	return VCCreate_Cleanup_DelSTSCfgRec( pCmdInfo, status );
		
	return VCCreate_Cleanup_Last( pCmdInfo, status );




}	// end of DdmVCM::CleanUpVCCreate



//  DdmVCM::VCCreate_Cleanup_DelSTSCfgRec(void *pClientContext, STATUS status)
//
//  Description:
//    Abort creating a virtual Circuit. Delete any SCSI Target Server Config
//    record we've created.
//
//  Inputs:
//    pClientContext - Actually a pointer to our command info structure
//    which contains a handle we return to report status and the request
//    we're processing.
//
//    status - The returned status of the PTS operation.
//
//  Outputs:
//    Returns OK, or a highly descriptive error code.
//
STATUS DdmVCM::VCCreate_Cleanup_DelSTSCfgRec(void *pClientContext, STATUS status)
{
VCCommandContext_t*	pCmdInfo = (VCCommandContext_t*)pClientContext;

	#if false
	if (status != OK)
		return CleanUpVCCreate(pCmdInfo, &status, CTS_VCM_EXPORT_REC_INSERT_ERROR);
	#endif
	
	// Try to read a SCSI Target Server flagged by our current 
	// Create Virtual Circuit command's Row ID.
	m_pDeleteSTSCfgRec = new TSDeleteRow;
	if (!m_pDeleteSTSCfgRec)
		status = CTS_OUT_OF_MEMORY;
	else		
		status = m_pDeleteSTSCfgRec->Initialize( 
			this,										// DdmServices *pDdmServices,
			STS_CONFIG_TABLE_NAME,						// String64 rgbTableName,
			RID_VC_ID,									// String64 rgbKeyFieldName,
			&pCmdInfo->pRequest->rid,					// void *pKeyFieldValue,
			sizeof(rowID),								// U32 cbKeyFieldValue,
			0,											// U32 cRowsToDelete,
			NULL,										// U32 *pcRowsDeletedRet,
			TSCALLBACK(DdmVCM,VCCreate_InsertSTSCfg),	// pTSCallback_t pCallback,
			pCmdInfo									// void* pContext
		);
	
	if (status == OK)
		m_pDeleteSTSCfgRec->Send();
	else
		status = CleanUpVCCreate(pCmdInfo, NULL, status);

	return status;
}	// VCCreate_Cleanup_DelSTSCfgRec
	
//  DdmVCM::VCCreate_Cleanup_DelExpRecs(void *pClientContext, STATUS status)
//
//  Description:
//    Abort creating a virtual Circuit. Delete any Export table Records we've
//    created.
//
//  Inputs:
//    pClientContext - Actually a pointer to our command info structure
//    which contains a handle we return to report status and the request
//    we're processing.
//
//    status - The returned status of the PTS operation.
//
//  Outputs:
//    Returns OK, or a highly descriptive error code.
//
STATUS DdmVCM::VCCreate_Cleanup_DelExpRecs(void *pClientContext, STATUS status)
{
VCCommandContext_t*	pCmdInfo = (VCCommandContext_t*)pClientContext;

	#if false
	// We're  forgiving of errors during abort.
	if (status != OK)
		return CleanUpVCCreate(pCmdInfo, &status, CTS_VCM_EXPORT_REC_INSERT_ERROR);
	#endif
	
	// Try to all export table records flagged by our current 
	// Create Virtual Circuit command's Row ID.
	m_pDeleteExpRecs = new TSDeleteRow;
	if (!m_pDeleteExpRecs)
		status = CTS_OUT_OF_MEMORY;
	else		
		status = m_pDeleteExpRecs->Initialize( 
			this,										// DdmServices *pDdmServices,
			EXPORT_TABLE,								// String64 rgbTableName,
			RID_VC_ID,									// String64 rgbKeyFieldName,
			&pCmdInfo->pRequest->rid,					// void *pKeyFieldValue,
			sizeof(rowID),								// U32 cbKeyFieldValue,
			NULL,										// U32 *pcRowsDelRet,
			0,											// U32 cRowsToDelete,
			TSCALLBACK(DdmVCM,VCCreate_Cleanup_Last),	// pTSCallback_t pCallback,
			pCmdInfo									// void* pContext
		);
	
	if (status == OK)
		m_pDeleteExpRecs->Send();
	else
		status = CleanUpVCCreate(pCmdInfo, NULL, status);

	return status;
}	// VCCreate_Cleanup_DelExpRecs


//  DdmVCM::VCCreate_Cleanup_Last(void *pClientContext, STATUS status)
//
//  Description:
//    Abort creating a virtual Circuit. Finish any cleanup necessary.
//    Delete allocated memory and free all resources.
//
//  Inputs:
//    pClientContext - Actually a pointer to our command info structure
//    which contains a handle we return to report status and the request
//    we're processing.
//
//    status - The returned status of the PTS operation.
//
//  Outputs:
//    Returns OK, or a highly descriptive error code.
//
STATUS DdmVCM::VCCreate_Cleanup_Last(void *pClientContext, STATUS status)
{
#pragma unused(status)

VCCommandContext_t*	pCmdInfo = (VCCommandContext_t*)pClientContext;

	#if false
	// We're  forgiving of errors during abort.
	if (status != OK)
		return CleanUpVCCreate(pCmdInfo, &status, CTS_VCM_EXPORT_REC_INSERT_ERROR);
	#endif
	
	if (m_pFCPortDatabaseTable)
		delete m_pFCPortDatabaseTable;
	m_pFCPortDatabaseTable = NULL;
	m_cRowsFCPortDatabaseTable = 0;
	
	if (m_pLoopDescriptorRecs)
		delete m_pLoopDescriptorRecs;
	m_pLoopDescriptorRecs = NULL;
	m_CurrLoopDescRecord = 0;

	if (m_pSystemStatusRecord)
		delete m_pSystemStatusRecord;
	m_pSystemStatusRecord = NULL;

	//delete m_pListenReplyType;
	//m_pListenReplyType = NULL;
	delete pCmdInfo;
	return OK;
}	// VCCreate_Cleanup_Last


#ifdef WIN32
//  DdmVCM::SimulateExportTableState(rowID *pExportTableRowId, CTReadyState newState)
//
//  Description:
//    For Win32 simulate the export/unexport state in the export table
//
//  Inputs:
//    pExportTableRowId - rowid in the export table to be modified
//
//    newState - The new ready state 
//
//  Outputs:
//    Returns OK, or a highly descriptive error code.
//
STATUS DdmVCM::SimulateExportTableState(
				U32					fExportRid,
				rowID				*pRowId,		// export rid or ridVcId, 
				CTReadyState		newState)
{
	TSModifyField		*pModifyExportRecord = new TSModifyField;
	CTReadyState		readyState;
	STATUS				status;

	readyState = newState;

	// we modify the state in the export table to Configured and exported

	if (!pModifyExportRecord)
		status = CTS_OUT_OF_MEMORY;
	else		
		status = pModifyExportRecord->Initialize(
			this,										// DdmServices *pDdmServices,
			EXPORT_TABLE,								// String64 rgbTableName,
			fExportRid 										// String64 rgbKeyFieldName,
				? CT_PTS_RID_FIELD_NAME 
				: RID_VC_ID,								
			pRowId,
			sizeof(rowID),
			READY_STATE,
			&readyState,
			sizeof(CTReadyState),
			0,											// U32 cRowsToModify,
			NULL,										// U32* pcRowsModifiedRet,
			NULL,										// rowID *prowIDRet,
			0,											// U32 cbMaxRowID,
			NULL,										// pTSCallback_t pCallback,
			NULL										// void* pContext
	);

	if (status == OK)
		pModifyExportRecord->Send();
	return status;
}
#endif
