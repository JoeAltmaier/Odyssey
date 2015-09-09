//////////////////////////////////////////////////////////////////////
// IopManager.cpp -- Responsible for the management of a single Iop.
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
// $Log: /Gemini/Odyssey/Hbc/IopManager.cpp $
// 
// 37    2/10/00 3:34p Joehler
// Change to allow system to boot when an IOP is plugged in but no image
// is available to boot it.  In this case, the system will come up but the
// IOP will be  out of service.
// 
// 36    1/26/00 2:43p Joehler
// Added IOP Into Service, Power Up, Lock and Unlock commands as well as
// Upgrade Master support.
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
// E2 CMB Boot changes: Move Init_Iop into iopManager.
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
#include "IopManager.h"
#include "Address.h"
#include "ImgMgr.h"
#include "Message.h"
#include "SystemStatusTable.h"
#include "IopImageTable.h"
#include "Watch.h"
#include "RqOsTransport.h"
#include "QuiesceMasterMsgs.h"
#include "RqOsVirtualMaster.h"
#include "UpgradeMasterMessages.h"
#include "imghdr.h"
#include "PciSlot.h"
#include "DefaultImageTable.h"

#include "Odyssey_Trace.h" 
#ifdef TRACE_INDEX
#undef TRACE_INDEX
#endif
#define TRACE_INDEX TRACE_BOOT

extern "C" STATUS init_iop(U32 slotnum);	// defined in Drivers/inithd.c.  Need a header.

// IopManager - Our Constructor.
IopManager::IopManager(	DdmServices*		pParentDdm, 
						IOPStatusRecord*	pIopStatusRecord,
						IOPImageRecord* 	pIopImageRecord)
{

	SetParentDdm( pParentDdm );
	
	m_CurrentStatus = OK;
	
	// IOP Status record members
	m_pIopStatusRecord = pIopStatusRecord;
	m_pIopImageRecord = pIopImageRecord;
	m_ridIopStatusRecord = pIopStatusRecord->rid;
	
	// timed listen members
	m_pListenReplyType = NULL;
	m_pListen4Iop = NULL;
	m_fIopAwaitingBoot = false;
	
	// internal messages used by boot manager
	m_pIopOutOfServiceMsg = NULL;
	m_pIopIntoServiceMsg = NULL;
	m_pIopLockUnlockMsg = NULL;
}


// ~IopManager - Our Destructor.
IopManager::~IopManager()
{
	CleanUp(m_CurrentStatus);
}


//  IopManager::CleanUp()
//
//  Description:
//    Our resource freer called upon error and completion.
//
//  Inputs:
//    status - 
//
//  Outputs:
//    Returns OK, or a highly descriptive error code.
//
STATUS IopManager::CleanUp(STATUS status)
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

#pragma mark ### Start Iop ###

//  IopManager::StartIop()
//
//  Description:
//    Entry point for the IOP Startup state sequence.  Method
//	  allocates and initializes a command sender to be used to 
//    communicate with the CMB Ddm.
//
//  Inputs:
//
//  Outputs:
//    Returns command sender initialize status
//
STATUS IopManager::StartIop()
{
	TRACEF(TRACE_L8, ("\n<<---------Initializing IOP at slot #%d-------->>\n", 
				m_pIopStatusRecord->Slot));

	m_pCmbCmdSender	= new CmdSender(
		CMB_CONTROL_QUEUE,			// String64 cmdQueueName
		CMB_CONTROL_COMMAND_SIZE,	// U32 sizeofCQParams
		CMB_CONTROL_STATUS_SIZE,	// U32 sizeofSQParams
		this						// DdmServices *pParentDdm
	);
	
	return m_pCmbCmdSender->csndrInitialize(INITIALIZECALLBACK(IopManager, TurnIopPowerOn));
}


//  IopManager::TurnIopPowerOn()
//
//  Description:
//	  Calls CMB DDM to power on the IOP
//
//  Inputs:
//	  status - returned status from command sender initialize
//
//  Outputs:
//
void IopManager::TurnIopPowerOn(STATUS status)
{

	TRACEF(TRACE_L8, ("\n***IopManager: Powering on IOP in slot #%d.\n",
			m_pIopStatusRecord->Slot ));
			
	// Set up a CMB Cmd structure to turn on the IOP's power.
	m_CmbCtlRequest.eCommand = k_eCmbCtlPower;
	m_CmbCtlRequest.eSlot = m_pIopStatusRecord->Slot;
	m_CmbCtlRequest.u.Power.bPowerOn = true;

	// Send the command in the CMB Ddm's Cmd Queue for execution. 
	status = m_pCmbCmdSender->csndrExecute(
		&m_CmbCtlRequest,									 	// void *pCQRecord,
		CMD_COMPLETION_CALLBACK(IopManager,Listen4IopAwaitingBoot),	// Completion callback
		NULL													// void	*pContext,
	);

	if (status)
		status = CleanUp(status);
		
	return;
}

//  IopManager::Listen4IopAwaitingBoot()
//
//  Description:
//    Issues a timed listen to wait for IOP to be in state IOPS_AWAITING_BOOT
//
//  Inputs:
//	  status - returned status from command sender power on request
//
//  Outputs:
//
void IopManager::Listen4IopAwaitingBoot(
										STATUS	status,
										void	*pResultData,
										void	*pCmdData,
										void	*pCmdContext)
{
#pragma unused(pResultData)
#pragma unused(pCmdData)
#pragma unused(pCmdContext)

	TRACEF(TRACE_L8, ("\n***IopManager: Powered on IOP in slot #%d.  Status = 0x%x.\n.",
			m_pIopStatusRecord->Slot, status));
			
	m_pListen4Iop = new TSTimedListen;
	if (!m_pListen4Iop)
		status = CTS_OUT_OF_MEMORY;
	else			
	{
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
			TSCALLBACK(IopManager, IopAwaitingBootListenReply),	// pTSCallback_t pCallback,
			45000000,									// timeout in microseconds. (45 sec.)
			NULL										// void* pContext
		);
	}
	
	if (status == OK)
		m_pListen4Iop->Send();
	else
		status = CleanUp( status );
	
	return;
}


//  IopManager::IopAwaitingBootListenReply()
//
//  Description:
//    This method is called back by Listen when the IOPState changes to
//    AWAITING_BOOT.
//
//  Inputs:
//    status - The returned status of the PTS operation.
//
//  Outputs:
//    Returns OK, or a highly descriptive error code.
//
STATUS IopManager::IopAwaitingBootListenReply(void *pClientContext, STATUS status)
{
#pragma unused(pClientContext)

	// If this is the initial reply ignore it.
	if (m_pListenReplyType && (*m_pListenReplyType == ListenInitialReply))
	{
		TRACEF(TRACE_L8, ("\n***IopManager: Received Initial Listen(IOPS_AWAITING_BOOT) Reply for IOP in slot #%d.  Status = 0x%x.\n",
				m_pIopStatusRecord->Slot, status ));
			
		return OK;
	}
	
	// Did we time out?
	if (status == CTS_PTS_LISTEN_TIMED_OUT)
	{
		TRACEF(TRACE_L3, ("\n***IopManager: Listen(IOPS_AWAITING_BOOT) TIMED OUT for IOP in slot #%d.  Status = 0x%x.\n",
				m_pIopStatusRecord->Slot, status ));
			
		m_CurrentStatus = CTS_IOP_POWER_TIMEOUT;
		return CleanUp( CTS_IOP_POWER_TIMEOUT );
	}
	
	// Some other error?
	if (status != OK)
	{
		TRACEF(TRACE_L3, ("\n***IopManager: Received BAD Listen(IOPS_AWAITING_BOOT) Reply for IOP in slot #%d.  Status = 0x%x.\n",
				m_pIopStatusRecord->Slot, status ));
			
		return CleanUp(status);
	}
	
	// This is the reply we're waiting for the IOP's state changed to IOPS_AWAITING_BOOT.
	TRACEF(TRACE_L3, ("\n***IopManager: Received Listen(IOPS_AWAITING_BOOT) Reply for IOP in slot #%d.  Status = 0x%x.\n",
			m_pIopStatusRecord->Slot, status ));
	
	// We need to replace our IopStatusRecord with the updated one returned by the Listen.
	*m_pIopStatusRecord = *m_pNewIopStatusRecord;
			
	// If all is well, on with the show..
	status = IopPciBoot();
	
	return status;
}

//  IopManager::IopPciBoot()
//
//  Description:
//    Enable the PCI QuickSwitches for the IOP.
//
//  Inputs:
//
//  Outputs:
//    Returns OK, or a highly descriptive error code.
//
STATUS IopManager::IopPciBoot()
{
	STATUS status;
	
	TRACEF(TRACE_L8, ("\n***IopManager: Enabling PCI Quick Switches for IOP in slot #%d.\n",
			m_pIopStatusRecord->Slot ));
			
	// Set up a CMB Cmd structure to enable the PCI quick switches.
	m_CmbCtlRequest.eCommand = k_eCmbCtlPciBusAccess;
	m_CmbCtlRequest.eSlot = m_pIopStatusRecord->Slot;
	m_CmbCtlRequest.u.PciBus.eAction = CCmbCtlPciBusEnable::k_eCtlPciBusEnable;

	// Send the command in the CMB Ddm's Cmd Queue for execution. 
	status = m_pCmbCmdSender->csndrExecute(
		&m_CmbCtlRequest,						 				// void *pCQRecord,
		CMD_COMPLETION_CALLBACK(IopManager,SetIopOnPciMask),	// completionCallback
		NULL													// void	*pContext,
	);

	return status;
}


//  IopManager::SetIopOnPciMask()
//
//  Description:
//	  Set this IOP's bit in the IOPsOnPCI bitmask in the 
//	  SystemStatusTable
//
//  Inputs:
//	  status - return code from CMB command to enable quick switched
//
//  Outputs:
//
void IopManager::SetIopOnPciMask(
									STATUS	status,
									void	*pResultData,
									void	*pCmdData,
									void	*pCmdContext)
{
#pragma unused(pResultData)
#pragma unused(pCmdData)
#pragma unused(pCmdContext)

	if (status !=OK)
	{	
		TRACEF(TRACE_L8, ("\n***IopManager: Enabled PCI Quick Switches for IOP in slot #%d failed.  Status = 0x%x.\n",
			m_pIopStatusRecord->Slot, status ));
		CleanUp(status);
		return;
	}

	TRACEF(TRACE_L8, ("\n***IopManager: Enabled PCI Quick Switches for IOP in slot #%d.\n",
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
	
	Send(pSetMyBit, REPLYCALLBACK(IopManager,FinishIopStart));
	
	if (status)
		status = CleanUp(status);

	return;
}

//  IopManager::FinishIopStart()
//
//  Description:
//	  call back for modify bits operation and end of StartIop() sequence
//
//  Inputs:
//	  pMsg - reply from PTS modify bits operation 
//
//  Outputs:
// 	  status - returns status from PTS operation
//
STATUS IopManager::FinishIopStart(Message *pMsg)
{
	STATUS status = pMsg->Status();

	TRACEF(TRACE_L3, ("\n***IopManager: Started IOP in slot #%d.  Status = 0x%x.\n",
			m_pIopStatusRecord->Slot, status ));
			
	return status;
}

#pragma mark ### Boot Iop ###

//  IopManager::BootIop()
//
//  Description:
//	  Prepare to set the IOPs PCI window, download it's image and boot the 
//	  IOP.  This is the entry point for the initial boot of an IOP.  There
//	  are two distinct cases.  If this is the first boot, (ie.  the images
//	  were discovered in the boot ROM and added to the image repository),
//	  then we need to boot the default image.  In the other case, the 
//	  IOPImageTable persisted and we boot the primary image.
//
//  Inputs:
//
//  Outputs:
// 	  status - returns OK
//
STATUS IopManager::BootIop()
{	
	if (m_pIopImageRecord->primaryImage == 0)
	{
		// this is a special case for boot when we are initially booting 
		// an IOP for which we discovered an image in the boot ROM and 
		// added it to the image repository.  This image is the default image.  
		// We need to boot this image, associate it with this slot and 
		// make it primary.
		
		// read the default image from the default image table 
		DefaultImageRecord::RqReadRow* preqRead;
		preqRead = new DefaultImageRecord::RqReadRow(CT_DEF_IMAGE_TABLE_TYPE,
													&m_pIopStatusRecord->IOP_Type,
												sizeof(U32));	
		assert(preqRead);
		
		// send read row request
		Send (preqRead, REPLYCALLBACK(IopManager, AssociateImage));
	}
	else
	{
		// Update our Iop image record to reflect the time booted
		// and the current image
		m_pIopImageRecord->timeBooted = Kernel::Time_Stamp();
		m_pIopImageRecord->currentImage = m_pIopImageRecord->primaryImage;
	
		IOPImageRecord::RqModifyRow* pModifyRow;
		pModifyRow = new IOPImageRecord::RqModifyRow(
			m_pIopImageRecord->rid, *m_pIopImageRecord);

		Send (pModifyRow, REPLYCALLBACK (IopManager, SetIopPciWindow));
	}
	
	return OK;
}

//  IopManager::AssociateImage()
//
//  Description:
//	  Calls the Upgrade Master to associate the default image
//	  with this slot.
//
//  Inputs:
//	  pReply - reply from the read default image request
//
//  Outputs:
// 	  status - returns OK
//
STATUS IopManager::AssociateImage(Message *pReply_)
{
	TRACE_ENTRY(DdmBootMgr::AssociateImage);
	
	STATUS status;
	
	// extract the default image from the reply
	DefaultImageRecord::RqReadRow* pReply = 
		(DefaultImageRecord::RqReadRow*) pReply_;
	status = pReply->Status();
	
	// this is the case where no image was found for this type of IOP
	if (status==ercKeyNotFound)
	{
		TRACEF(TRACE_L3, ("\n***IopManager: No image found for IOP in slot #%d.\n",
				m_pIopStatusRecord->Slot));
			
		return CleanUp(status);
	}
	
	assert(status==OK);
	
	m_defaultImage = pReply->GetRowPtr()->imageKey;
	
	// associate the default image with this IOP
	MsgAssociateImage* pMsg = new MsgAssociateImage(m_defaultImage,
		m_pIopStatusRecord->Slot);

	Send (pMsg, REPLYCALLBACK (IopManager, MakePrimary));

	return OK;
}

//  IopManager::MakePrimary()
//
//  Description:
//	  Calls the Upgrade Master to make this image the primary image.
//
//  Inputs:
//	  pReply - reply from the associate image request
//
//  Outputs:
// 	  status - returns OK
//
STATUS IopManager::MakePrimary(Message* pReply)
{
	TRACE_ENTRY(DdmBootMgr::MakePrimary);

	assert(pReply->Status()==OK);
	
	// make this image primary for this IOP
	MsgMakePrimary* pMsg = new MsgMakePrimary(m_defaultImage,
		m_pIopStatusRecord->Slot);

	Send (pMsg, REPLYCALLBACK (IopManager, ReadIopImageRecord));
	
	return OK;

}

//  IopManager::ReadIopImageRecord()
//
//  Description:
//	  Read the new Iop Image Record associated with this slot.
//
//  Inputs:
//	  pReply - reply from the make primary request
//
//  Outputs:
// 	  status - returns OK
//
STATUS IopManager::ReadIopImageRecord(Message* pReply)
{
	TRACE_ENTRY(DdmBootMgr::ReadIopImageRecord);

	assert(pReply->Status()==OK);
	
	IOPImageRecord::RqReadRow* pReadRow;
	pReadRow = new IOPImageRecord::RqReadRow(m_pIopImageRecord->rid);

	Send (pReadRow, REPLYCALLBACK (IopManager, SetCurrentImage));

	return OK;
}

//  IopManager::SetIopCurrentImage()
//
//  Description:
//	  Set the IOP's current image and time booted.
//
//  Inputs:
//	  pReply - reply from the read request
//
//  Outputs:
// 	  status - returns OK
//
STATUS IopManager::SetCurrentImage(Message* pReply_)
{
	TRACE_ENTRY(DdmBootMgr::SetCurrentImage);

	assert(pReply_->Status()==OK);
	
	IOPImageRecord::RqReadRow* pReply = (IOPImageRecord::RqReadRow*) pReply_;
	
	m_pIopImageRecord = pReply->GetRowCopy();
	
	// Update our Iop image record to reflect the time booted.
	m_pIopImageRecord->timeBooted = Kernel::Time_Stamp();
	m_pIopImageRecord->currentImage = m_defaultImage;
	
	IOPImageRecord::RqModifyRow* pModifyRow;
	pModifyRow = new IOPImageRecord::RqModifyRow(
		m_pIopImageRecord->rid, *m_pIopImageRecord);

	Send (pModifyRow, REPLYCALLBACK (IopManager, SetIopPciWindow));

	return OK;
}



//  IopManager::SetIopPciWindow()
//
//  Description:
//	  Set IOP's PCI Window in preparation for a via PCI boot.
//
//  Inputs:
//	  pMsg - reply from PTS Modify Row request
//
//  Outputs:
// 	  status - returns status from command sender execute
//
STATUS IopManager::SetIopPciWindow(Message *pMsg)
{
	STATUS status = pMsg->Status();
	
	if (status!=OK)
	{
		TRACEF(TRACE_L8, ("\n***IopManager: Modify row request failed for slot #%d.\n",
			m_pIopStatusRecord->Slot ));
		return status;
	}
	
	TRACEF(TRACE_L8, ("\n***IopManager: Setting PCI Window for IOP in slot #%d.\n",
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
		CMD_COMPLETION_CALLBACK(IopManager,OpenImage),	// completionCallback
		NULL													// void	*pContext,
	);

	return status;
}


//  IopManager::OpenImage()
//
//  Description:
//	  Call Upgrade Master to retrieve image from the image repository
//
//  Inputs:
//	  status - return status from CMB set IOP PCI window command
//
//  Outputs:
//
void IopManager::OpenImage(
									STATUS	status,
									void	*pResultData,
									void	*pCmdData,
									void	*pCmdContext)
{
#pragma unused(pResultData)
#pragma unused(pCmdData)
#pragma unused(pCmdContext)

	TRACEF(TRACE_L8, ("\n***IopManager: Set IOP in Slot# %d's PCI window.  Status = 0x%x.\n",
			 m_pIopStatusRecord->Slot, status));

	if (status != OK)
	{
		status = CleanUp(status);
		return;
	}

	// call Upgrade Master to open image
	MsgOpenImage* pMsg = new MsgOpenImage(m_pIopImageRecord->currentImage);
	Send(pMsg, REPLYCALLBACK(IopManager, DownloadImage));
}

//  IopManager::DownloadIopImage()
//
//  Description:
//	  Download an IOP's image over the PCI Bus for boot.
//
//  Inputs:
//	  pReply_ - reply from Upgrade Master open image request
//
//  Outputs:
//	  status - returns status from SetIopBootType()
//
STATUS IopManager::DownloadImage(Message* pReply_)
{
	U32	dst;
	U32	offset = M(48);
	U32	param_offset = M(63);
	STATUS status;
	void* src;
	U32	size;
	img_hdr_t	*imgp;
	
	TRACEF(TRACE_L8, ("\n***IopManager: Downloaded image for  IOP in Slot# %d.  Status = 0x%x.\n",
			 m_pIopStatusRecord->Slot, pReply_->Status()));

   	// copy actual firmward image to target IOP slot's PCI window
	// retrieve image from reply here.  save to src
	MsgOpenImage* pReply = (MsgOpenImage*)pReply_;
	pReply->GetImage(&src);
	imgp = (img_hdr_t *)src;
	if (imgp->i_signature != IMG_SIGNATURE) {
		printf("Image not valid in flash at %08lx\n\r", imgp);
		return (1);
	}
	size = imgp->i_imageoffset + imgp->i_zipsize + imgp->i_sym_table_size;
	size = ROUNDUP(size, 64);	
	dst = PCITOV(memmaps.aPaPci[m_pIopStatusRecord->Slot]) + offset;
	// was bcopy64
	bcopy((char *)src, (char *)dst, size); 

	/*
	 * Now copy Boot Block. IOP needs memmaps structure, so fill it
	 * in the bootblock.
	 */
	dst = PCITOV(memmaps.aPaPci[m_pIopStatusRecord->Slot]) + param_offset;
	PciSlot::bcopy((U8 *)&memmaps, (U8 *)&bootblock.b_memmap, sizeof (mmap_t));
	PciSlot::bcopy((U8 *)&bootblock, (U8 *)dst, sizeof (bootblock_t)); 
	
	status = SetIopBootType (offset, param_offset);
	if (status)
		status = CleanUp(status);
		
	return status;
}

//  IopManager::SetIopBootType()
//
//  Description:
//	  Set IOP's boot type.  Currently always to via PCI.
//
//  Inputs:
//	  ulImageOffset - image offset
//	  ulParamOffset - parameter offset
//
//  Outputs:
//	  status - returns status from command sender execute
//
STATUS IopManager::SetIopBootType(U32 ulImageOffset,
                                  U32 ulParamOffset)
{

	// Set up a CMB Cmd structure to set the IOP's BootType.
	m_CmbCtlRequest.eCommand = k_eCmbCtlCmbCtlBoot;
	m_CmbCtlRequest.eSlot = m_pIopStatusRecord->Slot;

	// what sort of "boot" to do.
	m_CmbCtlRequest.u.Boot.eAction = CCmbCtlBoot::k_ePCI;

	// offset of boot image from start of IOP's PCI window - lsb == 64kB
	m_CmbCtlRequest.u.Boot.ulImageOffset = ulImageOffset >> 16;
	
	// offset of boot image param area from start of IOP's PCI win - lsb == 64kB
	m_CmbCtlRequest.u.Boot.ulParamOffset = ulParamOffset >> 16;

	TRACEF(TRACE_L8, ("\n***IopManager: Booting CHAOS on IOP in Slot# %d.\n",
			m_pIopStatusRecord->Slot));

	// Send the command in the CMB Ddm's Cmd Queue for execution. 
	return m_pCmbCmdSender->csndrExecute(
		&m_CmbCtlRequest,									// void *pCQRecord,
		CMD_COMPLETION_CALLBACK(IopManager,FinishIopBoot),	// completionCallback
		NULL												// void	*pContext,
	);
}


//  - 
//  IopManager::Listen4IopActive()
//
//  Description:
//	  Wait for IOPs to become active.
//
//  Inputs:
//	  status - return status from CMB k_eCmbCtlCmbCtlBoot command
//
//  Outputs:
//
void IopManager::Listen4IopActive(
									STATUS	status,
									void	*pResultData,
									void	*pCmdData,
									void	*pCmdContext)
{
#pragma unused(pResultData)
#pragma unused(pCmdData)
#pragma unused(pCmdContext)

	TRACEF(TRACE_L8, ("\n***IopManager: Booted CHAOS on IOP in Slot# %d.  Status = 0x%x.\n",
			m_pIopStatusRecord->Slot, status));

	m_pListen4Iop = new TSTimedListen;
	if (!m_pListen4Iop)
		status = CTS_OUT_OF_MEMORY;
	else			
	{
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
			TSCALLBACK(IopManager, IopActiveListenReply),	// pTSCallback_t pCallback,
			120000000,									// timeout in microseconds (120 sec.)
			NULL										// void* pContext
		);
	}
	
	if (status == OK)
			m_pListen4Iop->Send();
	else
		status = CleanUp( status );
}

//  IopManager::IopActiveListenReply(void *pClientContext, STATUS status)
//
//  Description:
//    This method is called back by Listen when the IOPState changes to
//    IOPS_BOOTING.
//
//  Inputs:
//    status - The returned status of the PTS timed listen operation.
//
//  Outputs:
//    Returns OK, or a highly descriptive error code.
//
STATUS IopManager::IopActiveListenReply(void *pClientContext, STATUS status)
{
#pragma unused(pClientContext)

	// If this is the initial reply ignore it.
	if (m_pListenReplyType && (*m_pListenReplyType == ListenInitialReply))
	{
		TRACEF(TRACE_L8, ("\n***IopManager: Received Initial Listen(IOPS_BOOTING) Reply for IOP in slot #%d.  Status = 0x%x.\n",
				m_pIopStatusRecord->Slot, status ));
			
		return OK;
	}
	
	// Did we time out?
	if (status == CTS_PTS_LISTEN_TIMED_OUT)
	{
		TRACEF(TRACE_L3, ("\n***IopManager: Listen(IOPS_BOOTING) TIMED OUT for IOP in slot #%d.  Status = 0x%x.\n",
				m_pIopStatusRecord->Slot, status ));
			
		m_CurrentStatus = CTS_IOP_POWER_TIMEOUT;
		return CleanUp( CTS_IOP_POWER_TIMEOUT );
	}

	// Some other error?
	if (status != OK)
	{
		TRACEF(TRACE_L3, ("\n***IopManager: Received BAD Listen(IOPS_BOOTING) Reply for IOP in slot #%d.  Status = 0x%x.\n",
				m_pIopStatusRecord->Slot, status ));
			
		return CleanUp(status);
	}
	
	// This is the reply we're waiting for the IOP's state changed to IOPS_OPERATING.
	TRACEF(TRACE_L8, ("\n***IopManager: Received Listen(IOPS_BOOTING) Reply for IOP in slot #%d.  Status = 0x%x.\n",
			m_pIopStatusRecord->Slot, status ));
	
	// We need to replace our IopStatusRecord with the updated one returned by the Listen.
	*m_pIopStatusRecord = *m_pNewIopStatusRecord;
	
	return status;
}
		


//  IopManager::FinishIopBoot(void *pClientContext, STATUS status)
//
//  Description:
//	  Finish IOP boot.  There's really noting to do here, but I needed a 
//	  callback to pass to csndrexecute() in SetIopBootType.
//
//  Inputs:
//    status - The returned status of the CMB command.
//
//  Outputs:
//
void IopManager::FinishIopBoot(
									STATUS	status,
									void	*pResultData,
									void	*pCmdData,
									void	*pCmdContext)

{
#pragma unused(pResultData)
#pragma unused(pCmdData)
#pragma unused(pCmdContext)
	TRACEF(TRACE_L3, ("\n***IopManager: Finished booting IOP in Slot# %d.  Status = 0x%x.\n",
			m_pIopStatusRecord->Slot, status));
	
	// if m_pIopIntoServiceMsg != NULL, then we are not in the process of
	// the initial boot, but are rather booting an image either for chassis
	// management or upgrade purposes.  In this case, we need to respond to 
	// the boot manager with the appropriate internal message
	if (m_pIopIntoServiceMsg)
	{
		Reply(m_pIopIntoServiceMsg, OK);
		m_pIopIntoServiceMsg = NULL;
	}

	status = CleanUp(status);
}

#pragma mark ### Ping Iop ###

//  IopManager::PingIop()
//
//  Description:
//	  Toggle IOP's LEDs.
//
//  Inputs:
//    status - The returned status of the CMB command.
//
//  Outputs:
//
void IopManager::PingIop()
{

    //  build ToggleLED message
    Message*	pMsg = new Message (LED_TOGGLE);

	if (m_pIopStatusRecord->Slot != Address::iSlotMe)
		 Send(m_pIopStatusRecord->Slot, pMsg, REPLYCALLBACK(IopManager,PingIop1));
	else
		 Send(pMsg, REPLYCALLBACK(IopManager,PingIop1));

}

//  IopManager::PingIop1()
//
//  Description:
//	  Reply handler for return from Toggling IOP's LEDs.
//
//  Inputs:
//    pMsg - The reply from the LED_TOGGLE message
//
//  Outputs:
//	  status - returned status from the LED_TOGGLE message
//
STATUS IopManager::PingIop1(Message* pMsg)
{
	STATUS	status = pMsg->Status();

	if (status != OK)
		TRACEF(TRACE_L3, ("\n***IopManager: Received Bad Reply for LEDs in slot #%d.  Status = 0x%x.\n",
			m_pIopStatusRecord->Slot, status ));

	delete pMsg;
	
	return status;
}

#pragma mark ### Take Iop Out Of Service ###
		
//  IopManager::HandleReqIopOutOfService()
//
//  Description:
//	  Entry point for message or command to take an IOP out of service.
//
//  Inputs:
//    pMsg - Internal message from the boot manager which to reply to
//		after the IOP is out of service
//
//  Outputs:
//	  status - returns OK
//
STATUS IopManager::HandleReqIopOutOfService(MsgIopOutOfServiceInt* pMsg)
{
	TRACEF(TRACE_L8, ("\n***IopManager: Take IOP in slot #%d out of service.\n.",
			m_pIopStatusRecord->Slot));

	m_pIopOutOfServiceMsg = pMsg;

	assert(m_pIopOutOfServiceMsg->GetSlot() == m_pIopStatusRecord->Slot);
	
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
		
	// our reply callback will do the quiesce
	Send (pModifyField, REPLYCALLBACK (IopManager, QuiesceIop));
	#endif
	
	return OK;
}

//  IopManager::QuiesceIop()
//
//  Description:
//	  calls Quiesce Manager to quiesce the IOP
//
//  Inputs:
//    pReply - reply from PTS Modify Field
//
//  Outputs:
//	  status - returns OK or highly descriptive return code
//
STATUS IopManager::QuiesceIop(Message* pReply)
{
	STATUS status = pReply->Status();
	
	if (status!=OK)
	{
		TRACEF(TRACE_L3, ("\n***IopManager: Received Bad Reply stopping transport in slot #%d.  Status = 0x%x.\n",
			m_pIopStatusRecord->Slot, status ));	
		Reply(m_pIopOutOfServiceMsg, status);
		m_pIopOutOfServiceMsg = NULL;
		return status;
	}
	
	TRACEF(TRACE_L8, ("\n***IopManager: Quiesce IOP in slot #%d.\n.",
			m_pIopStatusRecord->Slot));

	//quiesce iop
     RqQuiesceIop* prqQuiesceIop = 
     	new RqQuiesceIop(m_pIopOutOfServiceMsg->GetSlot());
     Send(prqQuiesceIop, REPLYCALLBACK(IopManager, FailoverVirtualDevices));	
	
	return status;
}

//  IopManager::FailoverVirtualDevices()
//
//  Description:
//	  calls Virtual Master to failover virtual devices on the IOP
//
//  Inputs:
//    pReply - reply from quiesce IOP message
//
//  Outputs:
//	  status - returns OK or highly descriptive return code
//
STATUS IopManager::FailoverVirtualDevices(Message* pReply)
{
	STATUS status = pReply->Status();
	
	if (status!=OK)
	{
		TRACEF(TRACE_L3, ("\n***IopManager: Received Bad Reply quiescing iop in slot #%d.  Status = 0x%x.\n",
			m_pIopStatusRecord->Slot, status ));
		Reply(m_pIopOutOfServiceMsg, status);
		m_pIopOutOfServiceMsg = NULL;
		return status;
	}
	
	TRACEF(TRACE_L8, ("\n***IopManager: Failover VDs in slot #%d.\n.",
			m_pIopStatusRecord->Slot));

	//failover virtual devices
     RqOsVirtualMasterFailSlot* prqFailSlot = 
     	new RqOsVirtualMasterFailSlot(m_pIopOutOfServiceMsg->GetSlot());
     Send(prqFailSlot, REPLYCALLBACK(IopManager, QuieseIopSystemEntries));	

	return status;
}

//  IopManager::QuieseIopSystemEntries()
//
//  Description:
//	  calls Ddm Manager to quiesce system entries on the IOP
//
//  Inputs:
//    pReply - reply from failover virtual devices message
//
//  Outputs:
//	  status - returns OK or highly descriptive return code
//
STATUS IopManager::QuieseIopSystemEntries(Message* pReply)
{
	STATUS status = pReply->Status();
	
	if (status!=OK)
	{
		TRACEF(TRACE_L3, ("\n***IopManager: Received Bad Reply starting transport for slot #%d.  Status = 0x%x.\n",
			m_pIopStatusRecord->Slot, status ));
		Reply(m_pIopOutOfServiceMsg, status);
		m_pIopOutOfServiceMsg = NULL;
		return status;
	}

	TRACEF(TRACE_L8, ("\n***IopManager: Quiesce system entries for slot #%d.\n.",
			m_pIopStatusRecord->Slot));

	// quiesce system entries
	RqOsDdmManagerQuiesceOs* prqQuiesceOS = new RqOsDdmManagerQuiesceOs();
	Send(m_pIopOutOfServiceMsg->GetSlot(), prqQuiesceOS, 
		REPLYCALLBACK(IopManager, DisconnectIopQuickSwitches));	

	return status;
}

//  IopManager::DisconnectIopQuickSwitches()
//
//  Description:
//	  calls CMB to disable the PCI for this IOP 
//
//  Inputs:
//    pReply - reply from quiesce system entries message
//
//  Outputs:
//	  status - returns OK or highly descriptive return code
//
STATUS IopManager::DisconnectIopQuickSwitches(Message* pReply)
{
	STATUS status = pReply->Status();

	if (status!=OK)
	{
		TRACEF(TRACE_L3, ("\n***IopManager: Received Bad Reply quiescing system entries for slot #%d.  Status = 0x%x.\n",
			m_pIopStatusRecord->Slot, status ));
		Reply(m_pIopOutOfServiceMsg, status);
		m_pIopOutOfServiceMsg = NULL;
		return status;
	}
	
	TRACEF(TRACE_L8, ("\n***IopManager:  Disconnect quick switches for slot #%d.\n.",
			m_pIopStatusRecord->Slot));

	//disconnect iop quick switches
	MsgCmbIopControl* pmsgIopControl = 
		new MsgCmbIopControl (m_pIopOutOfServiceMsg->GetSlot(), 
			MsgCmbIopControl::DisablePCI); 
    Send (pmsgIopControl, REPLYCALLBACK (IopManager, ClearIopOnPciMask)); 

	return status;
}

//  IopManager::ClearIopOnPciMask()
//
//  Description:
//	  Clear the IOP's bit in the IOPsOnPCI Mask in the SystemStatusTable.
//
//  Inputs:
//    pReply - reply from disable PCI message
//
//  Outputs:
//	  status - returns OK or highly descriptive return code
//
STATUS IopManager::ClearIopOnPciMask(Message* pReply)
{

	STATUS status = pReply->Status();
	
	if (status!=OK)
	{
		TRACEF(TRACE_L3, ("\n***IopManager: Received Bad Reply disconnecting quick switches for slot #%d.  Status = 0x%x.\n",
			m_pIopStatusRecord->Slot, status ));
		Reply(m_pIopOutOfServiceMsg, status);
		m_pIopOutOfServiceMsg = NULL;
		return status;
	}
	
	TRACEF(TRACE_L8, ("\n***IopManager: Disabled PCI Quick Switches for IOP in slot #%d.\n",
			m_pIopStatusRecord->Slot ));
			
	// Set our bit in the IOPsOnPCIMask system status table.
	// First compute our mask bit.
	U32 myfIOPOnPCIMaskBit = ~(1 << m_pIopStatusRecord->Slot);

	RqPtsModifyBits	*pClearMyBit = new RqPtsModifyBits(
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
	
	Send(pClearMyBit, REPLYCALLBACK(IopManager,TurnIopPowerOff));
	
	return status;
}

//  IopManager::TurnIopPowerOff()
//
//  Description:
//	  Call the CMB to turn the power off on this IOP.
//
//  Inputs:
//    pReply - reply from PTS modify field request.
//
//  Outputs:
//	  status - returns OK or highly descriptive return code
//
STATUS IopManager::TurnIopPowerOff(Message* pReply)
{
	STATUS status = pReply->Status();
	
	if (status!=OK)
	{
		TRACEF(TRACE_L3, ("\n***IopManager: Received Bad Reply modifying system status table.  Status = 0x%x.\n",status ));
		Reply(m_pIopOutOfServiceMsg, status);	
		m_pIopOutOfServiceMsg = NULL;
		return status;
	}
	
	TRACEF(TRACE_L8, ("\n***IopManager:  Turn power off for slot #%d.\n.",
			m_pIopStatusRecord->Slot));

	//power off iop
	MsgCmbIopControl* pmsgIopControl = 
		new MsgCmbIopControl (m_pIopOutOfServiceMsg->GetSlot(), 
			MsgCmbIopControl::PowerOff); 
    Send (pmsgIopControl, REPLYCALLBACK (IopManager, ReplyIopOutOfService)); 

	return status;
}

//  IopManager::ReplyIopOutOfService()
//
//  Description:
//	  Reply to boot manager internal message
//
//  Inputs:
//    pReply - reply from power off message
//
//  Outputs:
//	  status - returns OK or highly descriptive return code
//
STATUS IopManager:: ReplyIopOutOfService(Message* pReply)
{
	STATUS status = pReply->Status();
	
	if (status!=OK)
		TRACEF(TRACE_L3, ("\n***IopManager: Received Bad Reply powering off iop in slot #%d.  Status = 0x%x.\n",
			m_pIopStatusRecord->Slot, status ));
		
	Reply(m_pIopOutOfServiceMsg, status);
	m_pIopOutOfServiceMsg = NULL;
	
	return status;
}

#pragma mark ### Put Iop Into Service ###

//  IopManager::HandleReqIopIntoService()
//
//  Description:
//	  Entry point for a request to put an IOP into service (i.e. boot and
//	  image).  This reuses the methods defined for an initial boot.  A non-NULL
//	  m_pIopIntoServiceMsg signals that we are dealing with a boot triggered by
//	  the chassis manager or the upgrade master.  If this is the case, we reply
//	  to the boot manager in FinishIopBoot().  If no image is specified to 
//	  boot, then we boot the primary image.
//
//  Inputs:
//    pMsg - Internal message from the Boot Manager
//
//  Outputs:
//	  status - returns OK 
//
STATUS IopManager::HandleReqIopIntoService(MsgIopIntoServiceInt* pMsg)
{
	TRACEF(TRACE_L8, ("\n***IopManager: Put IOP in slot #%d into service.\n.",
			m_pIopStatusRecord->Slot));

	m_pIopIntoServiceMsg = pMsg;

	assert(m_pIopIntoServiceMsg->GetSlot() == m_pIopStatusRecord->Slot);

	// determine image to boot.  either it is supplied in the message,
	// or boot the primary image for the IOP
	RowId imageKey = pMsg->GetImageKey();
	if (imageKey==0)
		imageKey = m_pIopImageRecord->primaryImage;
	
	// Update our Iop image record to reflect the time booted.
	m_pIopImageRecord->timeBooted = Kernel::Time_Stamp();
	m_pIopImageRecord->currentImage = imageKey;
	
	IOPImageRecord::RqModifyRow* pModifyRow;
	pModifyRow = new IOPImageRecord::RqModifyRow(
		m_pIopImageRecord->rid, *m_pIopImageRecord);

	Send (pModifyRow, REPLYCALLBACK (IopManager, SetIopPciWindow));
	
	return OK;
}

#pragma mark ### Lock Iop ###

//  IopManager::HandleReqIopLock()
//
//  Description:
//	  Entry point for a request to lock an IOP.  Sends lock request to the CMB
//
//  Inputs:
//    pMsg - Internal message from the Boot Manager
//
//  Outputs:
//	  status - returns OK 
//
STATUS IopManager::HandleReqIopLock(MsgIopLockUnlockInt* pMsg)
{
	TRACEF(TRACE_L8, ("\n***IopManager: Locked IOP in slot #%d.\n.",
			m_pIopStatusRecord->Slot));

	m_pIopLockUnlockMsg = pMsg;
	
	assert(m_pIopLockUnlockMsg->GetSlot() == m_pIopStatusRecord->Slot);

	// lock IOP
	MsgCmbIopControl* pmsgIopControl = 
		new MsgCmbIopControl (m_pIopStatusRecord->Slot, 
			MsgCmbIopControl::DisableRemoval); 
    Send (pmsgIopControl, REPLYCALLBACK (IopManager, ReplyLockUnlockIop)); 

	return OK;
}

#pragma mark ### Unlock Iop ###

//  IopManager::HandleReqIopUnlock()
//
//  Description:
//	  Entry point for a request to unlock an IOP.  Sends unlock request to 
//    the CMB
//
//  Inputs:
//    pMsg - Internal message from the Boot Manager
//
//  Outputs:
//	  status - returns OK 
//
STATUS IopManager::HandleReqIopUnlock(MsgIopLockUnlockInt* pMsg)
{
	TRACEF(TRACE_L8, ("\n***IopManager: Unlocked IOP in slot #%d.\n.",
			m_pIopStatusRecord->Slot));

	m_pIopLockUnlockMsg = pMsg;
	
	assert(m_pIopLockUnlockMsg->GetSlot() == m_pIopStatusRecord->Slot);

	// unlock IOP
	MsgCmbIopControl* pmsgIopControl = 
		new MsgCmbIopControl (m_pIopStatusRecord->Slot, 
			MsgCmbIopControl::EnableRemoval); 
    Send (pmsgIopControl, REPLYCALLBACK (IopManager, ReplyLockUnlockIop)); 

	return OK;
}

//  IopManager::ReplyLockUnlockIop()
//
//  Description:
//	  Replies to the boot manager internal message
//
//  Inputs:
//    pReply- reply from the CMB lock or unlock message
//
//  Outputs:
//	  status - returns OK or highly descriptive error code. 
//
STATUS IopManager::ReplyLockUnlockIop(Message* pReply)
{
	STATUS status = pReply->Status();
	
	if (m_pIopLockUnlockMsg->Lock())
	{
		TRACEF(TRACE_L8, ("\n***IopManager: IOP locked in slot #%d.\n.",
			m_pIopStatusRecord->Slot));
	}
	else
	{
		TRACEF(TRACE_L8, ("\n***IopManager: IOP unlocked in slot #%d.\n.",
			m_pIopStatusRecord->Slot));
	}

	if (status!=OK)
		TRACEF(TRACE_L3, ("\n***IopManager: Received Bad Reply locking or unlocking iop in slot #%d.  Status = 0x%x.\n",
			m_pIopStatusRecord->Slot, status ));
		
	Reply(m_pIopLockUnlockMsg, status);
	m_pIopLockUnlockMsg = NULL;
	
	return status;
}

