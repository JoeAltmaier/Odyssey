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
// File: DdmVCM.cpp
//
// Description:
//    Virtual Circuit Manager DDM
//
// $Log: /Gemini/Odyssey/DdmVCM/DdmVCM.cpp $
// 
// 12    1/07/00 5:40p Agusev
// Fixed WIN32 build. 
// Is this the last time or what?!
// 
// 11    12/17/99 5:32p Dpatel
// server vc modify message
// 
// 10    11/12/99 9:49a Dpatel
// changes for win32 port of VCM
// 
// 9     10/02/99 3:48p Agusev
// checked the return status for Read exp table
// 
// 7     9/06/99 8:03p Jlane
// Yet another interim checkin.
// 
// 6     9/05/99 4:40p Jlane
// Compiles and is theoretically ready to try.
// 
// 5     8/08/99 12:20p Jlane
// Interim checkin upon compiling
// 
// 4     8/05/99 11:36a Jlane
// Interim checkin to share the code.
// 
// 3     7/26/99 1:22p Jlane
// INterim checkin before rewrite supporting new HostConnectionDescriptor.
// 
// 2     7/26/99 1:21p Jlane
// INterim checkin before rewrite supporting new HostConnectionDescriptor.
// 
// 1     7/21/99 5:52p Jlane
// Initial Checkin of as yet unfinished work.
// 
//
/*************************************************************************/

#include  <assert.h>          // debug stuff

#include "DdmVCM.h"
#include "CTEvent.h"
#include "STSConfig.h"

#include "Odyssey_Trace.h" 
#define TRACE_INDEX TRACE_VCM

#include "BuildSys.h"
CLASSNAME (DdmVCM, SINGLE);  // Class Link Name used by Buildsys.cpp --
                              // must match CLASSENTRY() name formal (4th arg)
                              // in buildsys.cpp


// STATIS VARIABLE DECLARATIONS
DdmVCM	*DdmVCM::pDdmVCM;


//  statically declare which request codes our DDM will serve:
//  *** These defs must match those in Initialize(), below ***
SERVELOCAL (DdmVCM, REQ_MODIFY_VC);



//  DdmVCM::DdmVCM (did)
//
//  Description:
//    Our constructor.
//
//  Inputs:
//    did - CHAOS "Device ID" of the instance we are constructing.
//
//  Outputs:
//    none
//
DdmVCM::DdmVCM (DID did) : Ddm (did),
                             m_CmdServer (VCM_CONTROL_QUEUE,
                                          VCM_CONTROL_COMMAND_SIZE,
                                          VCM_CONTROL_STATUS_SIZE,
                                          this,
                                          CMDCALLBACK (CDdmVVCM, CmdReady))
{
	TRACE_ENTRY(DdmVCM::DdmVCM);
	return;
}  /* end of DdmVCM::DdmVCM (DID did) */



//  static DdmVCM* DdmVCM::Ctor (did)
//
//  Description:
//    Our static, standard system-defined helper function.
//    This routine is called by CHAOS when it wants to create
//    an instance of DdmVCM.
//
//  Inputs:
//    did - CHAOS "Device ID" of the new instance we are to create.
//
//  Outputs:
//    DdmVCM::Ctor - Returns a pointer to the new instance, or NULL.
//
Ddm *DdmVCM::Ctor (DID did)
{
	if (!pDdmVCM)
		return (pDdmVCM = new DdmVCM (did));
	else
		return NULL;
}  /* end of DdmVCM::Ctor */



//  DdmVCM::Initialize (pMsg)
//
//  Description:
//    Called for a DDM instance when the instance is being created.
//    This routine is called after the DDM's constructor, but before
//    DdmVCM::Enable().
//
//    We create the PTS tables which we use: the Export table
//
//    Finally, we call our base class' Initialize(), after we complete
//    our local functionality.
//
//  Inputs:
//    pMsg - Points to message which triggered our DDM's fault-in.  This is
//          always an "initialize" message.
//
//  Outputs:
//    DdmVCM::Initialize - Returns OK if all is cool, else an error.
//
STATUS DdmVCM::Initialize (Message *pMsg)
{
	TRACE_ENTRY(DdmVCM::Initialize);
	DispatchRequest(REQ_MODIFY_VC, REQUESTCALLBACK (DdmVCM, VirtualCircuitModify));
	// save The Init Message for later reply.
	m_pInitMsg = pMsg;
	
	STATUS	status = DefineSRCTable (NULL, OK);
	
	return (OK);      // (must *always* return success)
}  /* end of DdmVCM::Initialize() */



//  DdmVCM::DefineSRCTable (void *pClientContext, STATUS status)
//
//  Description:
//    Create the Storage Roll Call Table.
//
//  Inputs:
//    pClientContext - unused in this instance.
//    status - status of prior PTS operation.
//
//  Outputs:
//    Returns OK if all is cool, else an error.
//
STATUS DdmVCM::DefineSRCTable (void *pClientContext, STATUS status)
{
#pragma unused(pClientContext)
TSDefineTable*	pDefineTable;

	TRACE_ENTRY(DdmVCM::DefineSRCTable);

	// create the Storage Roll Call Table if it does not exist.  
	pDefineTable = new TSDefineTable;
	if (!pDefineTable)
		status = CTS_OUT_OF_MEMORY;
	else
		pDefineTable->Initialize(
			this,									// Ddm* pDdm,
			STORAGE_ROLL_CALL_TABLE,				// String64 prgbTableName,
			StorageRollCallTable_FieldDefs,			// fieldDef* prgFieldDefsRet,
			cbStorageRollCallTable_FieldDefs,		// U32 cbrgFieldDefs,
			20,										// U32 cEntriesRsv,
			true,									// bool* pfPersistant,
			TSCALLBACK(DdmVCM,DefineSTSConfig),	// pTSCallback_t pCallback,
			NULL									// void* pContext
		);
	
	if (status == OK)
		pDefineTable->Send();

	return (CTS_SUCCESS);      // (must *always* return success)

}  /* end of DdmVCM::DefineSRCTable () */



//  DdmVCM::DefineSTSConfig (void *pClientContext, STATUS status)
//
//  Description:
//    Create the SCSI Target Server Config Table.
//
//  Inputs:
//    pClientContext - unused in this instance.
//    status - status of prior PTS operation.
//
//  Outputs:
//    Returns OK if all is cool, else an error.
//
STATUS DdmVCM::DefineSTSConfig (void *pClientContext, STATUS status)
{
#pragma unused(pClientContext)
TSDefineTable*	pDefineTable;

	TRACE_ENTRY(DdmVCM::DefineSTSConfig);
	TRACEF( TRACE_L8, ("DdmVCM: Status from DefinTable(STORAGE_ROLL_CALL_TABLE) = %x.\n", status ));

	// create the Export Table if it does not exist.  
	pDefineTable = new TSDefineTable;
	if (!pDefineTable)
		status = CTS_OUT_OF_MEMORY;
	else
		status = pDefineTable->Initialize(
			this,									// Ddm* pDdm,
			STS_CONFIG_TABLE_NAME,					// String64 prgbTableName,
			StsConfigTable_FieldDefs,				// fieldDef* prgFieldDefsRet,
			cbStsConfigTable_FieldDefs,				// U32 cbrgFieldDefs,
			8,										// U32 cEntriesRsv,
			true,									// bool* pfPersistant,
			TSCALLBACK(DdmVCM,DefineExportTable),	// pTSCallback_t pCallback,
			NULL									// void* pContext
		);
	
	if (status == OK)
		pDefineTable->Send();

	return (OK);      // (must *always* return success)

}  /* end of DdmVCM::DefineSTSConfig () */



//  DdmVCM::DefineExportTable (void *pClientContext, STATUS status)
//
//  Description:
//    Create the Storage Roll Call Table.
//
//  Inputs:
//    pClientContext - unused in this instance.
//    status - status of prior PTS operation.
//
//  Outputs:
//    Returns OK if all is cool, else an error.
//
STATUS DdmVCM::DefineExportTable (void *pClientContext, STATUS status)
{
#pragma unused(pClientContext)
TSDefineTable*	pDefineTable;

	TRACE_ENTRY(DdmVCM::DefineExportTable);
	TRACEF( TRACE_L8, ("DdmVCM: Status from DefinTable(STS_CONFIG_TABLE_NAME) = %x.\n", status ));

	// create the Storage Roll Call Table if it does not exist.  
	pDefineTable = new TSDefineTable;
	if (!pDefineTable)
		status = CTS_OUT_OF_MEMORY;
	else
		status = pDefineTable->Initialize(
			this,									// Ddm* pDdm,
			EXPORT_TABLE,							// String64 prgbTableName,
			ExportTable_FieldDefs,					// fieldDef* prgFieldDefsRet,
			cbExportTable_FieldDefs,				// U32 cbrgFieldDefs,
			8,										// U32 cEntriesRsv,
			true,									// bool* pfPersistant,
			TSCALLBACK(DdmVCM,InitializeCmdSvr),	// pTSCallback_t pCallback,
			NULL									// void* pContext
		);
	
	if (status == OK)
		pDefineTable->Send();

	return (CTS_SUCCESS);      // (must *always* return success)

}  /* end of DdmVCM::DefineExportTable () */



// void  DdmVCM::InitializeCmdSvr (void *pClientContext, STATUS status)
//
//  Description:
//  We are called back from m_CmdServer.csrvInitialize().
//
//  Inputs:
//    ulStatus - Result of initialize operation.
//
STATUS  DdmVCM::InitializeCmdSvr (void *pClientContext, STATUS status)
{
#pragma unused(pClientContext)

	TRACE_ENTRY(DdmVCM::InitializeCmdSvr);
	TRACEF( TRACE_L8, ("DdmVCM: Status from DefinTable(EXPORT_TABLE) = %x.\n", status ));

	//  initialize our control interface..
	status = m_CmdServer.csrvInitialize (INITIALIZECALLBACK (DdmVCM,
	                                                   InitializeLast));
	if (status != CTS_SUCCESS)
	{
		assert (status == CTS_SUCCESS);
		
		//  we're toast, stop right here
		Reply (m_pInitMsg, status);
	}
	return status;
}



// InitializeLast
//  We are called back from m_CmdServer.csrvInitialize(), in Initialize().
//  Note that since there is no cookie parameter to csrvInitialize, we
//  treat this callback as a "stub" rather than part of our main flow
//  of initialization.
//
//  Inputs:
//    ulStatus - Result of initialize operation.
//
void  DdmVCM::InitializeLast (STATUS status)
{
	TRACE_ENTRY(DdmVCM::InitializeLast);
	TRACEF( TRACE_L8, ("DdmVCM: Status from csrvInitialize() = %x.\n", status ));

   //  as a stub, all we can do is flag if init went bad, no init message
   //  available to reply to here.
   assert (status == CTS_SUCCESS);
   
	// Reply to our init message
	Reply (m_pInitMsg, CTS_SUCCESS);
		
   return;

}  /* end of DdmVCM::InitializeLast */



//  DdmVCM::Quiesce (pMsg)
//
//  Description:
//    Called when our DDM is supposed to enter the quiescent state.
//    We do any necessary tidying up, and then reply to acknowledge
//    the quiesce request.
//
//    ** Per the DDM model, we cannot receive any request messages
//       once we have entered our quiesce processing.  All new requests
//       are deferred until quiesce is "undone" by an enable message.
//
//    ** Note that replies are never blocked (or "deferred") -- they
//       will always come into the DDM as soon as they work their way
//       through the DDM instance's message queue.
//
//  Inputs:
//    pMsg - Points to quiesce message which triggered this call.
//             Must be replied to in order to signal quiesce completion.
//
//  Outputs:
//    DdmVCM::Quiesce - Returns OK, or a highly descriptive error code.
//
STATUS DdmVCM::Quiesce (Message *pMsg)
{ 
	TRACE_ENTRY(DdmVCM::Quiesce);
   
   //  * * *  do local Quiesce stuff here.  * * *

   //  signal CHAOS that our DDM instance is finished quiescing.
   Ddm::Quiesce(pMsg);

   return (CTS_SUCCESS);      // (must *always* return success)

}  /* end of DdmVCM::Quiesce */



//  DdmVCM::Enable (pMsg)
//
//  Description:
//    Called during our DDM instance's initial fault-in [after Initialize()
//    has been called], and also called when our DDM is being reenabled
//    after a Quiesce() call had been made.
//
//    We read any configuration data which we ourselves need from PTS, since
//    it might have changed since we last ran.
//
//    We do whatever else we need to to render ourselves ready for business.
//    Then we signal CHAOS that we're done enable()ing, and ready to process
//    new request messages.
//
//    ** Per the DDM model, our instance will not be able to receive any
//       new request messages until after signalling completion of our
//       enable processing (by replying to our input message).
//
//    ** Note that we may receive reply messages at any time, so that our
//       initialize and/or enable processing may involve submitting requests
//       to other DDMs and waiting for their replies.
//
//  Inputs:
//    pMsg - Points to "enable" message which triggered this call.
//          Must be replied to in order to signal completion of Enable()
//          processing, and our willingness to accept new request messages.
//
//  Outputs:
//    DdmVCM::Ctor - Returns a pointer to the new instance, or NULL.
//
STATUS DdmVCM::Enable(Message *pMsg)
{
	TRACE_ENTRY(DdmVCM::Enable);
   
   //  let base class know that we're done with enable phase
   //  (allows new requests to be processed by our DDM instance --
   //   until this call, CHAOS was holding them off from us to let
   //   us finish initialization / reply processing)
   Reply (pMsg, CTS_SUCCESS);

   return (CTS_SUCCESS);      // (must *always* return success)

}  /* end of DdmVCM::Enable */



//
//  DdmVCM::CmdReady (hRequest, pRequest)
//
//  Description:
//    Called when CmdSender::csndrExecute() is used to send us a request.
//    We dispatch the request for appropriate processing.  After processing
//    completes (may span multiple DDM messages) we issue a suitable reply.
//
//  Inputs:
//    hRequest - Handle of request we're call for.  We must return this
//                when we report request status.
//    pRequest - Request which we're to process.
//
//  Outputs:
//    DdmVCM::CmdReady - Returns CTS_SUCCESS (will become void later on).
//

void  DdmVCM::CmdReady (HANDLE hRequest, void *_pRequest)
{
STATUS status;
VCRequest *pRequest = (VCRequest *)_pRequest;

	TRACE_ENTRY(DdmVCM::CmdReady);

   assert (pRequest != NULL);

//  what we do to report status is call
//   m_CmdServer.csrvReportStatus (hRequest, SQ_COMMAND_STATUS, CTS_SUCCESS,
//                                 NULL,    // (our result data is in req pkt)
//                                 pRequest);  // pRequest can have short life


   switch (pRequest->eCommand)
      {
      case k_eCreateVC:
         // params are VCCreateParms
         VirtualCircuitCreate(hRequest, pRequest);
         break;

      case k_eDeleteVC:
         // params are VCDeleteParms
         VirtualCircuitDelete(hRequest, pRequest);
         break;

      case k_eVCExportCtl:
         // params are VCExportCtlParms
         VirtualCircuitExportCtl(hRequest, pRequest);
         break;

      case k_eVCQuiesceCtl:
         // params are VCQuiesceCtlParms
         VirtualCircuitQuiesceCtl(hRequest, pRequest);
         break;

      default:
         Tracef ("DdmVCM::CmdReady: unknown command code %d\n",
                 pRequest->eCommand);

		status = CTS_VCM_INVALID_COMMAND;
         //  report unknown command code to sender
         m_CmdServer.csrvReportCmdStatus( hRequest,
         								  SQ_COMMAND_STATUS,
                                          &status,
                                          pRequest);
      }
}  /* end of DdmVCM::CmdReady */



//  DdmVCM::VirtualCircuitQuiesceCtl(VCQuiesceCtlCommand &VCQuiesceCtlParms)
//
//  Description:
//    Called when our DDM is supposed to Qiesce Virtual Circuit(s).
//
//  Inputs:
//    hRequest - Handle of request we're call for.  We must return this
//                when we report request status.
//    pRequest - Request which we're to process.
//
//  Outputs:
//    Returns OK, or a highly descriptive error code.
//
STATUS DdmVCM::VirtualCircuitQuiesceCtl(HANDLE hRequest, VCRequest *pRequest)
{
#pragma unused(hRequest)
#pragma unused(pRequest)

	return OK;
}


#ifdef WIN32
STATUS DdmVCM::VirtualCircuitModify(Message *){
	return OK;
}
#endif

