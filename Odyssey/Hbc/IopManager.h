//////////////////////////////////////////////////////////////////////
// IopManager.h - Responsible for managing a single IOP.
//
// Copyright (C) ConvergeNet Technologies, 1998, 1999 
//
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// Revision History:
//
// $Log: /Gemini/Odyssey/Hbc/IopManager.h $
// 
// 18    1/26/00 2:43p Joehler
// Added IOP Into Service, Power Up, Lock and Unlock commands as well as
// Upgrade Master support.
// 
// 17    11/22/99 5:31p Jlane
// Use Internal version of IopOutOfService Message
// 
// 16    11/21/99 5:00p Jlane
// Changes to IopOutOfService and Take ImageRecord.
// 
// 15    11/04/99 8:30a Joehler
// added iop out of service functionality
// 
// 14    10/28/99 7:22p Sgavarre
// Fix csndrExecute callbacks to have correct prototypes.
// 
// 13    10/28/99 9:21a Sgavarre
// Add New Entry point for split boot.  Remove no longer needed timer
// handling methods.
// 
// 12    9/01/99 8:06p Iowa
// 
// 11    9/01/99 6:49p Cmcdermott
// Updated for the IOP BOOT MANAGER to enable the PCI quick switches
// before send the PCI window command.  This policy was removed from the
// AVR POWER on logic.
// 
// 10    8/24/99 10:02a Jlane
// Multiple changes to support blinking LEDs and confirmation of Iops
// coming active.
// 
// 9     8/20/99 10:16a Jlane
// Listen interface changes.
// 
// 8     8/19/99 5:03p Jlane
// Various Cleanup and bug fixes.  Also don't terminate CmdSender object
// for now.
// 
// 7     8/15/99 1:13p Jlane
// Added error handling and CleanUp() utility.
// 
// 6     8/11/99 3:18p Jlane
// Remove NU_Sleep calls and add listening on IOP state.  Interim checkin.
// Work on MUltiple IOP boot ongoing.
// 
// 5     8/10/99 11:37a Jlane
// Added code to listen on IOPs going to the AWAITING_BOOT state and to
// timeout after 30 seconds if they never get there.
// 
// 4     6/25/99 1:53p Jlane
// Rolled in new Cmd Sender stuff.
// 
// 3     6/21/99 7:07p Ewedel
// Modified SetIopBootType() to take actual image / param offset values.
// 
// 2     6/17/99 3:25p Jlane
// Got Running as far as I can w/o the real hardware.
// 
// 1     6/16/99 5:47p Jlane
// Initial checkin.
//
//////////////////////////////////////////////////////////////////////
#ifndef __IopManager_H
#define __IopManager_H

#include "OsTypes.h"
#include "CtTypes.h"
#include "DdmCmbMsgs.h"
#include "CmbDdmCommands.h"
#include "Message.h"
#include "CmdSender.h"
#include "Ddm.h"
#include "DiskStatusTable.h"
#include "EVCStatusRecord.h"
#include "IOPStatusTable.h"
#include "IOPImageTable.h"
#include "ImageDescriptorTable.h"
#include "ReadTable.h"
#include "Listen.h"
#include "Table.h"
#include "Rows.h"
#include "BootMgrMsgs.h"


class IopManager: public DdmServices
{
public:
		// Standard methods:
		IopManager(	DdmServices*		pParentDdm, 
					IOPStatusRecord*	pIopStatusRecord,
					IOPImageRecord* 	pIopImageRecord);
		virtual	~IopManager();

		// Our Ddm specific methods:

		// Here is the start IOP Sequence...
		STATUS StartIop();
		
		// Here is the Boot IOP Sequence...
		STATUS BootIop();
		
		// Toggle IOPs LEDs.
		void PingIop();

		// Take IOP out of service
		STATUS HandleReqIopOutOfService(MsgIopOutOfServiceInt* pMsg);

		// Put IOP into service
		STATUS HandleReqIopIntoService(MsgIopIntoServiceInt* pMsg);

		// Lock or Unlock an IOP
		STATUS HandleReqIopLock(MsgIopLockUnlockInt* pMsg);
		STATUS HandleReqIopUnlock(MsgIopLockUnlockInt* pMsg);

private:		
		// CleanUp() - Our resource freer called upon error and completion.
		STATUS CleanUp(STATUS status);
		
		// Start IOP Methods...
		STATUS Listen4IopsOnPci(STATUS status);
		void TurnIopPowerOn(STATUS status);
		void Listen4IopAwaitingBoot(	STATUS	status,
										void	*pResultData,
										void	*pCmdData,
										void	*pCmdContext);
		STATUS IopAwaitingBootListenReply(void *pClientContext, STATUS status);
		STATUS IopPciBoot();
		void SetIopOnPciMask(	STATUS	status,
								void	*pResultData,
								void	*pCmdData,
								void	*pCmdContext);
		STATUS FinishIopStart(Message *pMsg);

		// Boot IOP methods...
		STATUS AssociateImage(Message *pMsg);
		STATUS MakePrimary(Message* pMsg);
		STATUS ReadIopImageRecord(Message* pMsg);
		STATUS SetCurrentImage(Message* pMsg);
		STATUS SetIopPciWindow(Message *pMsg);
		// open the image from the image repository
		void OpenImage(	STATUS	status,
								void	*pResultData,
								void	*pCmdData,
								void	*pCmdContext);
		// download MIPS image to IOP's PCI window, & tell IOP to boot it
		STATUS DownloadImage(Message* pReply);
		//  set "set boot params" command to IOP via CMB
		STATUS SetIopBootType(U32 ulImageOffset, U32 ulParamOffset);
		// Listen for IOP to become active and operational...
		void Listen4IopActive(	STATUS	status,
								void	*pResultData,
								void	*pCmdData,
								void	*pCmdContext);
		STATUS IopActiveListenReply(void *pClientContext, STATUS status);
		//  dummy place for final CMB command callback to go
		void FinishIopBoot(	STATUS	status,
							void	*pResultData,
							void	*pCmdData,
							void	*pCmdContext);

		// Toggle IOPs LEDs methods...
		STATUS PingIop1(Message* pMsg);
		
		// Take IOP out of service methods
		STATUS QuiesceIop(Message* pReply);
		STATUS FailoverVirtualDevices(Message* pReply);
		STATUS QuieseIopSystemEntries(Message* pReply);
		STATUS DisconnectIopQuickSwitches(Message* pReply);
		STATUS ClearIopOnPciMask(Message* pReply);
		STATUS TurnIopPowerOff(Message* pReply);
		STATUS ReplyIopOutOfService(Message* pReply);
				
		// Lock or Unlock an IOP method
		STATUS ReplyLockUnlockIop(Message* pReply);

private:

		// Our instance data
		STATUS					m_CurrentStatus;
		U32						m_fInitialized;

		// IOP Status record members
		IOPStatusRecord*		m_pIopStatusRecord;
		IOPStatusRecord*		m_pNewIopStatusRecord;
		U32						m_cbNewIopStatusRecord;
		rowID					m_ridIopStatusRecord;

		// image management members
		rowID					m_defaultImage;
		IOPImageRecord* 		m_pIopImageRecord;	
		
		// command sender for CMB to control IOP
		CmdSender*				m_pCmbCmdSender;
		CmbCtlRequest			m_CmbCtlRequest;
		
		// timed listen members
		IopState				m_DesiredIOPState;
		U32						m_ListenerID;
		U32*					m_pListenReplyType;
		U32						m_fIopAwaitingBoot;
		TSTimedListen*			m_pListen4Iop;
		
		// internal messages used by the boot manager
		MsgIopOutOfServiceInt*	m_pIopOutOfServiceMsg;
		MsgIopIntoServiceInt*	m_pIopIntoServiceMsg;
		MsgIopLockUnlockInt*	m_pIopLockUnlockMsg;
};


#endif	// __IopManager_H

