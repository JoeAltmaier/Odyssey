//////////////////////////////////////////////////////////////////////
// IopManProxy.cpp -- Responsible for the management of a single Iop.
//
// Copyright (C) ConvergeNet Technologies, 1998 
//
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// $Log: /Gemini/Odyssey/Oos/IopProxy.cpp $
// 
// 1     2/15/00 6:14p Tnelson
// Proxy services for testing
// 
// 35    11/22/99 5:31p Jlane
// Use Internal version of IopOutOfService Message
// 
// 34    11/21/99 4:59p Jlane
// Changes to IopOutOfService and Take ImageRecord.
// 
// 33    11/17/99 4:59p Joehler
// Added Quiesce System Entries call
// 
// 32    11/06/99 2:18p Jlane
// Use callback for LED messages.
// 
// 31    11/04/99 8:30a Joehler
// added iop out of service functionality
// 
// 30    10/30/99 4:13p Sgavarre
// turn watch off.
// 
// 29    10/28/99 7:22p Sgavarre
// Fix csndrExecute callbacks to have correct prototypes.
// 
// 28    10/28/99 9:21a Sgavarre
// Split boot up for IntBridgeFTree() call in BootMgr.  Use TSTimedListen
// instead of TSListen and remove no longer needed timer handling methods.
// 
// 27    10/19/99 8:16p Jlane
// Dont printf message on each toggle of the LEDs.
// 
// 26    10/14/99 4:48p Jlane
// Multiple fixes to get LED Blinking working.
// 
// 23    9/07/99 9:57p Jlane
// Re add  code to enable IOP quickswitches before giving them their PCI
// window.
// 
// 22    9/02/99 5:31p Jlane
// Skip Craigs new call.
// 
// 21    9/01/99 8:06p Iowa
// 
// 20    9/01/99 6:46p Cmcdermott
// Added a command to issue a PCI ENABLE command to the IOP before sending
// the PCI window.  This was to let the IOP BOOT manager control when the
// PCI quick switches get enable vs. having that policy in the AVR.
// 
// 19    8/24/99 10:02a Jlane
// Multiple changes to support blinking LEDs and confirmation of Iops
// coming active.
// 
// 18    8/20/99 10:16a Jlane
// Listen interface changes.
// 
// 17    8/19/99 5:03p Jlane
// Various Cleanup and bug fixes.  Also don't terminate CmdSender object
// for now.
// 
// 16    8/17/99 2:29p Cmcdermott
// Removed line 116 " FREEANDCLEAR(m_pListenReplyType); " because it was
// causing MIPS exceptions and Jerry Lane said it is no longer needed.
// 
// 15    8/15/99 1:13p Jlane
// Added error handling and CleanUp() utility.
// 
// 14    8/12/99 6:13p Jlane
// Remove sleep operations.
// 
// 13    8/11/99 3:18p Jlane
// Remove NU_Sleep calls and add listening on IOP state.  Interim checkin.
// Work on MUltiple IOP boot ongoing.
// 
// 12    8/10/99 11:37a Jlane
// Added code to listen on IOPs going to the AWAITING_BOOT state and to
// timeout after 30 seconds if they never get there.
// 
// 11    8/02/99 6:44p Mpanas
// Add Black Magic Sleeps before CMB Commands so it will work even if no
// one knows why.
// 
// 10    7/23/99 1:31p Rkondapalli
// Hack downloaded IOP images also added NAC type.  (JFL)
// 
// 9     7/20/99 6:46p Rkondapalli
// E2 CMB Boot changes: Move Init_Iop into IopManProxy.
// 
// 8     6/25/99 1:53p Jlane
// Rolled in new Cmd Sender stuff.
// 
// 7     6/23/99 8:35a Rkondapalli
// Call InitHardware after powering on an IOP and set the PCI Window base
// address correctly.
// 
// 6     6/21/99 7:08p Ewedel
// Modified to pass image / param offset values explicitly among download
// and boot routines.
// 
// 5     6/21/99 6:06p Ewedel
// Added 16-bit scaling needed by PCI window / boot image params to CMB.
// 
// 4     6/21/99 1:27p Rkondapalli
// Enabled image download logic in DownloadIopImage().  [jl?]
// 
// 3     6/18/99 2:00p Rkondapalli
// Use 4 as status data size in csndrinitialize().
// 
// 2     6/17/99 3:25p Jlane
// Got Running as far as I can w/o the real hardware.
// 
// 1     6/16/99 5:47p Jlane
// Initial checkin.
// 
//////////////////////////////////////////////////////////////////////
#include "IopProxy.h"
#include "Address.h"
//#include "pcimap.h"
#include "ImgMgr.h"
#include "Message.h"
#include "SystemStatusTable.h"
#include "IopImageTable.h"
//#include "Watch.h"
#include "RqOsTransport.h"
#include "QuiesceMasterMsgs.h"
#include "RqOsVirtualMaster.h"

#include "Odyssey_Trace.h" 
#ifdef TRACE_INDEX
#undef TRACE_INDEX
#endif
#define TRACE_INDEX TRACE_BOOT

extern "C" STATUS init_iop(U32 slotnum);	// defined in Drivers/inithd.c.  Need a header.

// IopManProxy - Our Constructor.
IopManProxy::IopManProxy(	DdmServices*		pParentDdm, 
						IOPStatusRecord*	pIopStatusRecord,
						IOPImageRecord* 	pIopImageRecord)
{
	SetParentDdm( pParentDdm );
	m_CurrentStatus = OK;
	m_pIopStatusRecord = pIopStatusRecord;
	m_pIopImageRecord = pIopImageRecord;
	m_ridIopStatusRecord = pIopStatusRecord->rid;
	m_pListenReplyType = NULL;
	m_pListen4Iop = NULL;
	m_fIopAwaitingBoot = false;

}


// ~IopManProxy - Our Destructor.
IopManProxy::~IopManProxy()
{
	CleanUp(m_CurrentStatus);
}


// CleanUp() - Our resource freer called upon error and completion.
STATUS IopManProxy::CleanUp(STATUS status)
{
	#if false
	if (m_pCmbCmdSender)
	{
		m_pCmbCmdSender->csndrTerminate();
		m_pCmbCmdSender = NULL;
	}
	#endif
	
	m_CurrentStatus = status;
	return m_CurrentStatus = OK;
}

// StartIop() - Prepare for the IOP Startup state sequence. 
STATUS IopManProxy::StartIop()
{
STATUS status = OK;

	TRACEF(TRACE_L8, ("\n<<---------Initializing IOP at slot #%d-------->>\n", 
				m_pIopStatusRecord->Slot));

	m_pCmbCmdSender	= new CmdSender(
		CMB_CONTROL_QUEUE,			// String64 cmdQueueName
		CMB_CONTROL_COMMAND_SIZE,	// U32 sizeofCQParams
		CMB_CONTROL_STATUS_SIZE,	// U32 sizeofSQParams
		this						// DdmServices *pParentDdm
	);
	
	if (status == OK)
		status = m_pCmbCmdSender->csndrInitialize( INITIALIZECALLBACK( IopManProxy, TurnIopPowerOn));
	
	return status;
}


// TurnIopPowerOn - Turn on an IOP's power to initiate it's booting..
void IopManProxy::TurnIopPowerOn(STATUS status)
{
	// Set up a CMB Cmd structure to turn on the IOP's power.
	m_CmbCtlRequest.eCommand = k_eCmbCtlPower;
	m_CmbCtlRequest.eSlot = m_pIopStatusRecord->Slot;
	m_CmbCtlRequest.u.Power.bPowerOn = true;

	TRACEF(TRACE_L8, ("\n***IopManProxy: Powering on IOP in slot #%d.\n",
			m_pIopStatusRecord->Slot ));
	// Send the command in the CMB Ddm's Cmd Queue for execution. 
	status = m_pCmbCmdSender->csndrExecute(
		&m_CmbCtlRequest,									 	// void *pCQRecord,
		CMD_COMPLETION_CALLBACK(IopManProxy,Listen4IopAwaitingBoot),	// Completion callback
		NULL													// void	*pContext,
	);

	if (status)
		status = CleanUp(status);
		
	return;
}


void IopManProxy::Listen4IopAwaitingBoot(
										STATUS	status,
										void	*pResultData,
										void	*pCmdData,
										void	*pCmdContext)
{
#pragma unused(pResultData)
#pragma unused(pCmdData)
#pragma unused(pCmdContext)
	TRACEF(TRACE_L8, ("\n***IopManProxy: Powered on IOP in slot #%d.  Status = 0x%x.\n.",
			m_pIopStatusRecord->Slot, status));
	m_pListen4Iop = new TSTimedListen;
	if (!m_pListen4Iop)
		status = CTS_OUT_OF_MEMORY;
	else			
		// We are going to listen once only for the new Virtual Device 
		// Record's fIopHasVDR field to match m_EnclosureStatusRec.IOPsMask
		// which will have a one in the bits corresponding to all present
		// IOPs.
		m_DesiredIOPState = IOPS_AWAITING_BOOT;
		status = m_pListen4Iop->Initialize( 
			this,										// DdmServices* pDdmServices
			ListenOnModifyOneRowOneField,				// U32 ListenType
			CT_IOPST_TABLE_NAME,						// String64 prgbTableName
			CT_PTS_RID_FIELD_NAME,						// String64 prgbRowKeyFieldName
			(void*)&m_pIopStatusRecord->rid,					// void* prgbRowKeyFieldValue
			sizeof(rowID),								// U32 cbRowKeyFieldValue
			CT_IOPST_IOPCURRENTSTATE,					// String64 prgbFieldName
			&m_DesiredIOPState,							// void* prgbFieldValue
			sizeof(m_DesiredIOPState),					// U32 cbFieldValue
			ReplyOnceOnly | ReplyWithRow,				// U32 ReplyMode
			NULL,										// void** ppTableDataRet,
			NULL,										// U32* pcbTableDataRet,
			&m_ListenerID,								// U32* pListenerIDRet,
			&m_pListenReplyType,						// U32** ppListenTypeRet,
			&m_pNewIopStatusRecord,						// void** ppModifiedRecordRet,
			&m_cbNewIopStatusRecord,					// U32* pcbModifiedRecordRet,
			TSCALLBACK(IopManProxy, IopAwaitingBootListenReply),	// pTSCallback_t pCallback,
			45000000,									// timeout in microseconds. (45 sec.)
			NULL										// void* pContext
		);
	
	if (status == OK)
			m_pListen4Iop->Send();
	else
		status = CleanUp( status );
	
	return;
}


//  IopManProxy::IopAwaitingBootListenReply(void *pClientContext, STATUS status)
//
//  Description:
//    This method is called back by Listen when the IOPState changes to
//    AWAITING_BOOT.
//
//  Inputs:
//    pClientContext - 
//
//    status - The returned status of the PTS operation.
//
//  Outputs:
//    Returns OK, or a highly descriptive error code.
//
STATUS IopManProxy::IopAwaitingBootListenReply(void *pClientContext, STATUS status)
{
#pragma unused(pClientContext)

	// If this is the initial reply ignore it.
	if (m_pListenReplyType && (*m_pListenReplyType == ListenInitialReply))
	{
		TRACEF(TRACE_L8, ("\n***IopManProxy: Received Initial Listen(IOPS_AWAITING_BOOT) Reply for IOP in slot #%d.  Status = 0x%x.\n",
				m_pIopStatusRecord->Slot, status ));
			
		return OK;
	}
	
	// Did we time out?
	if (status == CTS_PTS_LISTEN_TIMED_OUT)
	{
		TRACEF(TRACE_L3, ("\n***IopManProxy: Listen(IOPS_AWAITING_BOOT) TIMED OUT for IOP in slot #%d.  Status = 0x%x.\n",
				m_pIopStatusRecord->Slot, status ));
			
		m_CurrentStatus = CTS_IOP_POWER_TIMEOUT;
		return CleanUp( CTS_IOP_POWER_TIMEOUT );
	}
	
	// Some other error?
	if (status != OK)
	{
		TRACEF(TRACE_L3, ("\n***IopManProxy: Received BAD Listen(IOPS_AWAITING_BOOT) Reply for IOP in slot #%d.  Status = 0x%x.\n",
				m_pIopStatusRecord->Slot, status ));
			
		return CleanUp(status);
	}
	
	// This is the reply we're waiting for the IOP's state changed to IOPS_AWAITING_BOOT.
	TRACEF(TRACE_L3, ("\n***IopManProxy: Received Listen(IOPS_AWAITING_BOOT) Reply for IOP in slot #%d.  Status = 0x%x.\n",
			m_pIopStatusRecord->Slot, status ));
	
	// We need to replace our IopStatusRecord with the updated one returned by the Listen.
	*m_pIopStatusRecord = *m_pNewIopStatusRecord;
			
	// If all is well, on with the show..
	status = IopPciBoot(status);
	
	return status;
}


// IopPciBoot - Enable the PCI QuickSwitches for the IOP.
STATUS IopManProxy::IopPciBoot(STATUS status)
{
	
	TRACEF(TRACE_L8, ("\n***IopManProxy: Enabling PCI Quick Switches for IOP in slot #%d.\n",
			m_pIopStatusRecord->Slot ));
			
	// Set up a CMB Cmd structure to enable the PCI quick switches.
	m_CmbCtlRequest.eCommand = k_eCmbCtlPciBusAccess;
	m_CmbCtlRequest.eSlot = m_pIopStatusRecord->Slot;
	m_CmbCtlRequest.u.PciBus.eAction = CCmbCtlPciBusEnable::k_eCtlPciBusEnable;

	// Send the command in the CMB Ddm's Cmd Queue for execution. 
	status = m_pCmbCmdSender->csndrExecute(
		&m_CmbCtlRequest,						 				// void *pCQRecord,
		CMD_COMPLETION_CALLBACK(IopManProxy,SetIopOnPciMask),	// completionCallback
		NULL													// void	*pContext,
	);

	return status;
}


// SetIopOnPciMask - Set the IOP's bit in the IOPsOnPCI Mask in the SystemStatusTable.
void IopManProxy::SetIopOnPciMask(
									STATUS	status,
									void	*pResultData,
									void	*pCmdData,
									void	*pCmdContext)
{
#pragma unused(pResultData)
#pragma unused(pCmdData)
#pragma unused(pCmdContext)

	TRACEF(TRACE_L8, ("\n***IopManProxy: Enabled PCI Quick Switches for IOP in slot #%d.\n",
			m_pIopStatusRecord->Slot ));
			
	// Set our bit in the IOPsOnPCIMask system status table.
	// First compute our mask bit.
	U32 myfIOPOnPCIMaskBit = (1 << m_pIopStatusRecord->Slot);

	RqPtsModifyBits	*pSetMyBit = new RqPtsModifyBits(
		SystemStatusRecord::TableName(),	// const char *_psTableName,
		OpOrBits,							// fieldOpType _opFlag,
		CT_PTS_ALL_ROWS_KEY_FIELD_NAME,		// const char *_psKeyFieldName,
		NULL,								// const void *_pKeyFieldValue,
		0,									// U32 _cbKeyFieldValue,
		CT_SYSST_IOPSONPCIMASK,				// const char *_psFieldName,
		(void*)&myfIOPOnPCIMaskBit,			// const void *_pbFieldMask,
		sizeof(U32),						// U32 _cbFieldMask,
		(U32)0								// U32 _cRowsToModify=0
	);
	
	Send(pSetMyBit, REPLYCALLBACK(IopManProxy,FinishIopStart));
	
	if (status)
		status = CleanUp(status);

	return;
}


// FinishIopStart() - end of the StartIop process.  Handle status and return
STATUS IopManProxy::FinishIopStart(Message *pMsg)
{
STATUS status = pMsg->DetailedStatusCode;

	TRACEF(TRACE_L3, ("\n***IopManProxy: Started IOP in slot #%d.  Status = 0x%x.\n",
			m_pIopStatusRecord->Slot, status ));
	return OK;
}


// BootIop - Prepare Set the IOPs PCI WIndow download it's image and boot the IOP.
STATUS IopManProxy::BootIop()
{
STATUS status;

	status = SetIopPciWindow(OK);
	return status;
}


// SetIopPciWindow - Set IOP's PCI Window in preparation for a via PCI boot.
STATUS IopManProxy::SetIopPciWindow(STATUS status)
{	
	TRACEF(TRACE_L8, ("\n***IopManProxy: Setting PCI Window for IOP in slot #%d.\n",
			m_pIopStatusRecord->Slot ));
			
	// Initialize the PCI bridges.  IOPs need this to talk to us.
	init_iop(m_pIopStatusRecord->Slot);
			
	// Set up a CMB Cmd structure to set the IOP's PCI Window.
	m_CmbCtlRequest.eCommand = k_eCmbCtlPciWindow;
	m_CmbCtlRequest.eSlot = m_pIopStatusRecord->Slot;

	// IOP's PCI window size (lsb = 64kB)
   m_CmbCtlRequest.u.PciWindow.ulPciWinSize = (64*1024*1024) >> 16;

	// IOP's PCI window base addr on PCI bus (lsb = 64kB)
	// These two lines should be identical.
   m_CmbCtlRequest.u.PciWindow.ulPciWinPciBase =
            memmaps.aPaPci[m_pIopStatusRecord->Slot] >> 16;
   m_CmbCtlRequest.u.PciWindow.ulPciWinPciBase =
            Address::aSlot[m_pIopStatusRecord->Slot].pciBase >> 16;
	
	// IOP's PCI window base addr on IOP (lsb = 64kB)
   m_CmbCtlRequest.u.PciWindow.ulPciWinIopBase = 0;
	
	// Send the command in the CMB Ddm's Cmd Queue for execution. 
	status = m_pCmbCmdSender->csndrExecute(
		&m_CmbCtlRequest,						 				// void *pCQRecord,
		CMD_COMPLETION_CALLBACK(IopManProxy,DownloadIopImage),	// completionCallback
		NULL													// void	*pContext,
	);

	return status;
}


// DownloadIopImage - Download an IOP's image over the PCI Buss for boot.
void IopManProxy::DownloadIopImage(
									STATUS	status,
									void	*pResultData,
									void	*pCmdData,
									void	*pCmdContext)
{
#pragma unused(pResultData)
#pragma unused(pCmdData)
#pragma unused(pCmdContext)
U32		block;
U32      ulImageOffset;
U32      ulParamOffset;

	TRACEF(TRACE_L8, ("\n***IopManProxy: Set IOP in Slot# %d's PCI window.  Status = 0x%x.\n",
			 m_pIopStatusRecord->Slot, status));

	if (status != OK)
	{
		status = CleanUp(status);
		return;
	}

	switch (m_pIopStatusRecord->IOP_Type)
	{
	case IOPTY_HBC: 	// host bridge controller
      assert (FALSE);   // we can't support HBC in this way.. [ewx]
		block = 0;
		break;

	case IOPTY_NAC:		// Network Array Controller
	case IOPTY_NIC:		// network interface card (fibre channel)
		block = 1;		// HACK ALERT:  SHOULD BE BLOCK # 1.
		break;

	case IOPTY_RAC:		// RAID array controller (fibre channel)
		block = 2;
		break;

	case IOPTY_SSD:		// solid state drive
		block = 3;		// 
		break;

	default:
		TRACEF(TRACE_L3, ("\n***IopManProxy: Invalid Board type (%d) for IOP in slot #%d.  Status = 0x%x.\n", m_pIopStatusRecord->IOP_Type, status));
		status = CleanUp(CTS_IOP_TYPE_UNKNOWN);
		return;
	}  // end switch
	
	// int image_hdr(U32 block, U32* pImgHdrRet. U32*pImgSizRet); 
	
	// int image_dnl(void* pImgHdr, U32 cbImgSiz ,U32 slot) */
	// Let Raghava do the actual work.  Thanks Raghava!
	image_dnl( block, m_pIopStatusRecord->Slot, &ulImageOffset, &ulParamOffset);
//	bcopy64((U8 *)src, (U8 *)dst, size); 

	status = SetIopBootType (ulImageOffset, ulParamOffset);
	if (status)
		status = CleanUp(status);
}


// SetIopBootType - Set IOP's boot type.  Currently alweays to viaPCI.
STATUS IopManProxy::SetIopBootType(U32 ulImageOffset,
                                  U32 ulParamOffset)
{

STATUS   status;

	// Set up a CMB Cmd structure to set the IOP's BootType.
	m_CmbCtlRequest.eCommand = k_eCmbCtlCmbCtlBoot;
	m_CmbCtlRequest.eSlot = m_pIopStatusRecord->Slot;

	// what sort of "boot" to do.
	m_CmbCtlRequest.u.Boot.eAction = CCmbCtlBoot::k_ePCI;

	// offset of boot image from start of IOP's PCI window - lsb == 64kB
	m_CmbCtlRequest.u.Boot.ulImageOffset = ulImageOffset >> 16;
	
	// offset of boot image param area from start of IOP's PCI win - lsb == 64kB
	m_CmbCtlRequest.u.Boot.ulParamOffset = ulParamOffset >> 16;

	TRACEF(TRACE_L8, ("\n***IopManProxy: Booting CHAOS on IOP in Slot# %d.\n",
			m_pIopStatusRecord->Slot));

	// Send the command in the CMB Ddm's Cmd Queue for execution. 
	status = m_pCmbCmdSender->csndrExecute(
		&m_CmbCtlRequest,									// void *pCQRecord,
		CMD_COMPLETION_CALLBACK(IopManProxy,FinishIopBoot),	// completionCallback
		NULL												// void	*pContext,
	);

	return status;
}


// Listen4IopActive - Wait for IOPs to become active.
void IopManProxy::Listen4IopActive(
									STATUS	status,
									void	*pResultData,
									void	*pCmdData,
									void	*pCmdContext)
{
#pragma unused(pResultData)
#pragma unused(pCmdData)
#pragma unused(pCmdContext)
	TRACEF(TRACE_L8, ("\n***IopManProxy: Booted CHAOS on IOP in Slot# %d.  Status = 0x%x.\n",
			m_pIopStatusRecord->Slot, status));

	m_pListen4Iop = new TSTimedListen;
	if (!m_pListen4Iop)
		status = CTS_OUT_OF_MEMORY;
	else			
		// We are going to listen once only for the new Virtual Device 
		// Record's fIopHasVDR field to match m_EnclosureStatusRec.IOPsMask
		// which will have a one in the bits corresponding to all present
		// IOPs.
		m_DesiredIOPState = IOPS_OPERATING;
		status = m_pListen4Iop->Initialize( 
			this,										// DdmServices* pDdmServices
			ListenOnModifyOneRowOneField,				// U32 ListenType
			CT_IOPST_TABLE_NAME,						// String64 prgbTableName
			CT_PTS_RID_FIELD_NAME,						// String64 prgbRowKeyFieldName
			(void*)&m_pIopStatusRecord->rid,					// void* prgbRowKeyFieldValue
			sizeof(rowID),								// U32 cbRowKeyFieldValue
			CT_IOPST_IOPCURRENTSTATE,					// String64 prgbFieldName
			&m_DesiredIOPState,							// void* prgbFieldValue
			sizeof(m_DesiredIOPState),					// U32 cbFieldValue
			ReplyOnceOnly | ReplyWithRow,				// U32 ReplyMode
			NULL,										// void** ppTableDataRet,
			NULL,										// U32* pcbTableDataRet,
			&m_ListenerID,								// U32* pListenerIDRet,
			&m_pListenReplyType,						// U32** ppListenTypeRet,
			&m_pNewIopStatusRecord,						// void** ppModifiedRecordRet,
			&m_cbNewIopStatusRecord,					// U32* pcbModifiedRecordRet,
			TSCALLBACK(IopManProxy, IopActiveListenReply),	// pTSCallback_t pCallback,
			120000000,									// timeout in microseconds (120 sec.)
			NULL										// void* pContext
		);
	
	if (status == OK)
			m_pListen4Iop->Send();
	else
		status = CleanUp( status );
}


//  IopManProxy::IopActiveListenReply(void *pClientContext, STATUS status)
//
//  Description:
//    This method is called back by Listen when the IOPState changes to
//    IOPS_BOOTING.
//
//  Inputs:
//    pClientContext - 
//
//    status - The returned status of the PTS operation.
//
//  Outputs:
//    Returns OK, or a highly descriptive error code.
//
STATUS IopManProxy::IopActiveListenReply(void *pClientContext, STATUS status)
{
#pragma unused(pClientContext)

	// If this is the initial reply ignore it.
	if (m_pListenReplyType && (*m_pListenReplyType == ListenInitialReply))
	{
		TRACEF(TRACE_L8, ("\n***IopManProxy: Received Initial Listen(IOPS_BOOTING) Reply for IOP in slot #%d.  Status = 0x%x.\n",
				m_pIopStatusRecord->Slot, status ));
			
		return OK;
	}
	
	// Did we time out?
	if (status == CTS_PTS_LISTEN_TIMED_OUT)
	{
		TRACEF(TRACE_L3, ("\n***IopManProxy: Listen(IOPS_BOOTING) TIMED OUT for IOP in slot #%d.  Status = 0x%x.\n",
				m_pIopStatusRecord->Slot, status ));
			
		m_CurrentStatus = CTS_IOP_POWER_TIMEOUT;
		return CleanUp( CTS_IOP_POWER_TIMEOUT );
	}

	// Some other error?
	if (status != OK)
	{
		TRACEF(TRACE_L3, ("\n***IopManProxy: Received BAD Listen(IOPS_BOOTING) Reply for IOP in slot #%d.  Status = 0x%x.\n",
				m_pIopStatusRecord->Slot, status ));
			
		return CleanUp(status);
	}
	
	// This is the reply we're waiting for the IOP's state changed to IOPS_OPERATING.
	TRACEF(TRACE_L8, ("\n***IopManProxy: Received Listen(IOPS_BOOTING) Reply for IOP in slot #%d.  Status = 0x%x.\n",
			m_pIopStatusRecord->Slot, status ));
	
	// We need to replace our IopStatusRecord with the updated one returned by the Listen.
	*m_pIopStatusRecord = *m_pNewIopStatusRecord;
			
	FinishIopBoot(status, NULL, NULL, NULL);
	return status;
}
		
		
// FinishIopBoot - Finish IOP's boot.  
// There's really nothing to do here but I needed a cllaback to pass
// to csndrexecute()  in SetIopBootType.
void IopManProxy::FinishIopBoot(
									STATUS	status,
									void	*pResultData,
									void	*pCmdData,
									void	*pCmdContext)

{
#pragma unused(pResultData)
#pragma unused(pCmdData)
#pragma unused(pCmdContext)
	TRACEF(TRACE_L3, ("\n***IopManProxy: Finished booting IOP in Slot# %d.  Status = 0x%x.\n",
			m_pIopStatusRecord->Slot, status));

	status = CleanUp(status);
}


// PingIop - Toggle IOP's LEDs.  
STATUS IopManProxy::PingIop()
{
STATUS	status;

    //  build ToggleLED message
    Message*	pMsg = new Message (LED_TOGGLE);

	if (m_pIopStatusRecord->Slot != Address::iSlotMe)
		status = Send(m_pIopStatusRecord->Slot, pMsg, REPLYCALLBACK(IopManProxy,PingIop1));
	else
		status = Send(pMsg, REPLYCALLBACK(IopManProxy,PingIop1));

	return status;
}


// PingIop1 - Reply handler for return from Toggling IOP's LEDs.  
STATUS IopManProxy::PingIop1(Message* pMsg)
{
STATUS	status = pMsg->DetailedStatusCode;

	if (status != OK)
		TRACEF(TRACE_L3, ("\n***IopManProxy: Received Bad Reply for LEDs in slot #%d.  Status = 0x%x.\n",
			m_pIopStatusRecord->Slot, status ));

	delete pMsg;
	
	return status;
}

		
// Take IOP out of service
STATUS IopManProxy::HandleReqIopOutOfService(MsgIopOutOfServiceInt* pMsg)
{
	TRACEF(TRACE_L8, ("\n***IopManProxy: Take IOP in slot #%d out of service.\n.",
			m_pIopStatusRecord->Slot));

	m_pIopOutOfServiceMsg = pMsg;

	#if false
	//stop transport
     RqOsTransportIopStop* prqStopTransport = 
     	new RqOsTransportIopStop(m_pIopOutOfServiceMsg->GetSlot());
     Send(prqStopTransport, REPLYCALLBACK(IopManProxy, QuiesceIop));
	#endif
	
	// Update our IopStatusRecord in IOPStatusTable to state IOPS_QUIESCING.
	#if false
	// Eric's CMB set state logic only supports Operating and Suspended.
	// Since I'm not sure how these relate to IOP states I'll update the
	// IOPStatusTable directly below but for consistency I'll revisit this.
	MsgCmbSetMipsState *pSet = new MsgCmbSetMipsState(MsgCmbSetMipsState::SetOperating);
	Send(pSet, REPLYCALLBACK(DdmVirtualManager,DiscardOkReply));
	#else
	m_pIopStatusRecord->eIOPCurrentState = IOPS_QUIESCING;
	
	IOPStatusRecord::RqModifyField* pModifyField;
	pModifyField = new IOPStatusRecord::RqModifyField(
		m_pIopStatusRecord->rid,
		CT_IOPST_IOPCURRENTSTATE,
		&m_pIopStatusRecord->eIOPCurrentState,
		sizeof(m_pIopStatusRecord->eIOPCurrentState)
	);
		
	//  (our reply callback will do the quiesce)
	Send (pModifyField, REPLYCALLBACK (IopManProxy, QuiesceIop));
	#endif
	
	return OK;
}


STATUS IopManProxy::QuiesceIop(STATUS status)
{
	if (status!=OK)
	{
		TRACEF(TRACE_L3, ("\n***IopManProxy: Received Bad Reply stopping transport in slot #%d.  Status = 0x%x.\n",
			m_pIopStatusRecord->Slot, status ));
			
		Reply(m_pIopOutOfServiceMsg, status);
		return status;
	}
	
	TRACEF(TRACE_L8, ("\n***IopManProxy: Quiesce IOP in slot #%d.\n.",
			m_pIopStatusRecord->Slot));

	//quiesce iop
     RqQuiesceIop* prqQuiesceIop = 
     	new RqQuiesceIop(m_pIopOutOfServiceMsg->GetSlot());
     Send(prqQuiesceIop, REPLYCALLBACK(IopManProxy, FailoverVirtualDevices));	
	
	return OK;
}

STATUS IopManProxy::FailoverVirtualDevices(STATUS status)
{
	if (status!=OK)
	{
		TRACEF(TRACE_L3, ("\n***IopManProxy: Received Bad Reply quiescing iop in slot #%d.  Status = 0x%x.\n",
			m_pIopStatusRecord->Slot, status ));
		Reply(m_pIopOutOfServiceMsg, status);
		return status;
	}
	
	TRACEF(TRACE_L8, ("\n***IopManProxy: Failover VDs in slot #%d.\n.",
			m_pIopStatusRecord->Slot));

	//failover virtual devices
     RqOsVirtualMasterFailSlot* prqFailSlot = 
     	new RqOsVirtualMasterFailSlot(m_pIopOutOfServiceMsg->GetSlot());
     Send(prqFailSlot, REPLYCALLBACK(IopManProxy, RestartTransport));	

	return OK;
}


// Shouldn't we quiesce, disconnect quick switches before we do this???
STATUS IopManProxy::RestartTransport(STATUS status)
{
	if (status!=OK)
	{
		TRACEF(TRACE_L3, ("\n***IopManProxy: Received Bad Reply failing VDs in slot #%d.  Status = 0x%x.\n",
			m_pIopStatusRecord->Slot, status ));
		Reply(m_pIopOutOfServiceMsg, status);
		return status;
	}
	
	TRACEF(TRACE_L8, ("\n***IopManProxy: Restart transport for slot #%d.\n.",
			m_pIopStatusRecord->Slot));

	//restart tranport
     RqOsTransportIopStart* prqTransportIopStart = 
     	new RqOsTransportIopStart(m_pIopOutOfServiceMsg->GetSlot());
     Send(prqTransportIopStart, 
     	REPLYCALLBACK(IopManProxy, QuieseIopSystemEntries));	

	return OK;
}


// Won't the quiesce master have done this voa Quiesce IOP above?
STATUS IopManProxy::QuieseIopSystemEntries(STATUS status)
{
	if (status!=OK)
	{
		TRACEF(TRACE_L3, ("\n***IopManProxy: Received Bad Reply starting transport for slot #%d.  Status = 0x%x.\n",
			m_pIopStatusRecord->Slot, status ));
		Reply(m_pIopOutOfServiceMsg, status);
		return status;
	}

	TRACEF(TRACE_L8, ("\n***IopManProxy: Quiesce system entries for slot #%d.\n.",
			m_pIopStatusRecord->Slot));

	RqOsDdmManagerQuiesceOs* prqQuiesceOS = new RqOsDdmManagerQuiesceOs();
	Send(m_pIopOutOfServiceMsg->GetSlot(), prqQuiesceOS, 
		REPLYCALLBACK(IopManProxy, DisconnectIopQuickSwitches));	

	return OK;
}

STATUS IopManProxy::DisconnectIopQuickSwitches(STATUS status)
{

	if (status!=OK)
	{
		TRACEF(TRACE_L3, ("\n***IopManProxy: Received Bad Reply quiescing system entries for slot #%d.  Status = 0x%x.\n",
			m_pIopStatusRecord->Slot, status ));
		Reply(m_pIopOutOfServiceMsg, status);
		return status;
	}
	
	TRACEF(TRACE_L8, ("\n***IopManProxy:  Disconnect quick switches for slot #%d.\n.",
			m_pIopStatusRecord->Slot));

	//disconnect iop quick switches
	MsgCmbIopControl* pmsgIopControl = 
		new MsgCmbIopControl (m_pIopOutOfServiceMsg->GetSlot(), 
			MsgCmbIopControl::DisablePCI); 
    Send (pmsgIopControl, REPLYCALLBACK (IopManProxy, TurnIopPowerOff)); 

	return OK;
}

STATUS IopManProxy::TurnIopPowerOff(STATUS status)
{
	if (status!=OK)
	{
		TRACEF(TRACE_L3, ("\n***IopManProxy: Received Bad Reply disconnecting quick switches for slot #%d.  Status = 0x%x.\n",
			m_pIopStatusRecord->Slot, status ));
		Reply(m_pIopOutOfServiceMsg, status);
		return status;
	}
	
	TRACEF(TRACE_L8, ("\n***IopManProxy:  Turn power off for slot #%d.\n.",
			m_pIopStatusRecord->Slot));

	//power off iop
	MsgCmbIopControl* pmsgIopControl = 
		new MsgCmbIopControl (m_pIopOutOfServiceMsg->GetSlot(), 
			MsgCmbIopControl::PowerOff); 
    Send (pmsgIopControl, REPLYCALLBACK (IopManProxy, ReplyIopOutOfService)); 

	return OK;
}

void IopManProxy:: ReplyIopOutOfService(STATUS status)
{
	if (status!=OK)
		TRACEF(TRACE_L3, ("\n***IopManProxy: Received Bad Reply powering off iop in slot #%d.  Status = 0x%x.\n",
			m_pIopStatusRecord->Slot, status ));
		
	Reply(m_pIopOutOfServiceMsg, status);
}