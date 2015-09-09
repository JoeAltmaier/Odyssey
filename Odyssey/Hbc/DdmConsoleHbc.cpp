/*************************************************************************/
// File: DdmConsoleHbc.cpp
// 
// Description:
// This file is the console code for the HBC Image
//
// 
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
//
// Update Log 
//	$Log: /Gemini/Odyssey/Hbc/DdmConsoleHbc.cpp $
// 
// 5     2/11/00 2:05p Rbraun
// Fixed PTS code, does not act as default table creator, fixed control
// switch code
// 
// 4     1/26/00 2:52p Joehler
// Debugged HBC Console.
// 
// 3     12/09/99 3:56p Joehler
// Completion on HbcConsole
// 
// 2     11/30/99 2:43p Jlane
// Prelimiminary changes to get working.
// 
// 1     11/22/99 4:38p Egroff
// Initial checkin.
// 
/*************************************************************************/


//====================================================================================
//
//	Start of the Console DDM
//	This DDM becomes a system entry to start it early.  We can do test messages
//	and external entry points here now.  To start with, we will use this DDM to
//	send messages and receive replies.
//
//====================================================================================

#include "DdmConsoleHbc.h"

extern "C" {
#include "CtTtyUtils.h"
}

#include "Odyssey_Trace.h"

#include "CtEvent.h"

#define TRACE_INDEX TRACE_BOOT
#define CONSOLE_PORT 3

// BuildSys Linkage
#include "BuildSys.h"

// JLO TEMP
extern "C" {
#include "systty.h"
}
// JLO TEMP

CLASSNAME(DdmConsoleHbc, SINGLE);

/*************************************************************************/
// Global references
/*************************************************************************/
DdmConsoleHbc	*pDdmConsoleHbc = NULL;

/*************************************************************************/
// DdmConsoleHbc -- Constructor
/*************************************************************************/
DdmConsoleHbc::DdmConsoleHbc(DID did) : Ddm(did) {
	pDdmConsoleHbc = this;

}

/*************************************************************************/
// Initialize -- Process Initialize
/*************************************************************************/

STATUS DdmConsoleHbc::Initialize(Message *pInitMsg_) 
{

	TRACE_ENTRY(DdmConsoleHbc::Initialize);
	
	pInitMsg = pInitMsg_;
		
	// JLO TEMP
	// init port
	ttyinit(CONSOLE_PORT, 57600);
		
	ttyioctl(CONSOLE_PORT, FIOCINT, TTYRX);
	// JLO END TEMP

	SystemConfigSettingsRecord::RqDefineTable *preqDefTab = 
		new SystemConfigSettingsRecord::RqDefineTable(Persistant_PT, 1);
	assert (preqDefTab);
		
	// send table definition request
	Send(preqDefTab, REPLYCALLBACK (DdmConsoleHbc, 
		SystemConfigSettingsTableInitialized));

	return OK;
}

STATUS DdmConsoleHbc::SystemConfigSettingsTableInitialized(Message* pReply_)
{

	TRACE_ENTRY(DdmConsoleHbc::SystemConfigSettingsTableInitialized);
	
	// make sure everything went ok
	assert (pReply_->Status()==OK || pReply_->Status()==ercTableExists);
	
	if (pReply_->Status() == ercTableExists)
	{
		SystemConfigSettingsRecord::RqEnumTable *preqEnumTab = 
			new SystemConfigSettingsRecord::RqEnumTable();
		assert (preqEnumTab);
		
		// send table enumeration request
		Send(preqEnumTab, REPLYCALLBACK (DdmConsoleHbc, 
			SystemConfigSettingsTableEnumed));
		
	}
	else
	{
		m_pSystemConfigRecord = new SystemConfigSettingsRecord();
		SystemConfigSettingsRecord::RqInsertRow *preqInsertRow =
			new SystemConfigSettingsRecord::RqInsertRow(*m_pSystemConfigRecord);
		assert(preqInsertRow);
		
		// send table enumeration request
		Send(preqInsertRow, REPLYCALLBACK (DdmConsoleHbc, 
			SystemConfigSettingsRowInserted));		
	}

	return OK;
		
}

STATUS DdmConsoleHbc::SystemConfigSettingsTableEnumed(Message* pReply_)
{

	TRACE_ENTRY(DdmConsoleHbc::SystemConfigSettingsTableEnumed);

	U32 numberOfRows;
	
	// make sure everything went ok
	assert (pReply_->Status()==OK);
	
	SystemConfigSettingsRecord::RqEnumTable* pReply = 
	 	(SystemConfigSettingsRecord::RqEnumTable*) pReply_;
	
	numberOfRows = pReply->GetRowCount();	
	assert(numberOfRows==0 || numberOfRows==1);
	
	if (numberOfRows == 0)
	{
		m_pSystemConfigRecord = new SystemConfigSettingsRecord();
		SystemConfigSettingsRecord::RqInsertRow *preqInsertRow =
			new SystemConfigSettingsRecord::RqInsertRow(*m_pSystemConfigRecord);
		assert(preqInsertRow);
		
		// send table enumeration request
		Send(preqInsertRow, REPLYCALLBACK (DdmConsoleHbc, 
			SystemConfigSettingsRowInserted));
	}
	else
	{
		m_pSystemConfigRecord = pReply->GetRowCopy();
	
		// now init a listen on the table
		ListenOnSysConfigTable();
	}
		
	delete pReply;

	return OK;
	
}


STATUS DdmConsoleHbc::SystemConfigSettingsRowInserted(Message* pReply_)
{

	TRACE_ENTRY(DdmConsoleHbc::SystemConfigSettingsRowInserted);

	SystemConfigSettingsRecord::RqInsertRow* pReply = 
		(SystemConfigSettingsRecord::RqInsertRow*) pReply_;
	
	// make sure everything went ok
	assert (pReply_->Status()==OK);
	
	m_pSystemConfigRecord->rid = *pReply->GetRowIdPtr();
	
	delete pReply;

	// now init a listen on the table
	ListenOnSysConfigTable();
				
	return OK;
	
}

void DdmConsoleHbc::ListenOnSysConfigTable()
{
	TRACE_ENTRY(DdmConsoleHbc::ListenOnSysConfigTable);
	
	SystemConfigSettingsRecord::RqListen* preqListen = 
		new SystemConfigSettingsRecord::RqListen();
		
	// send the listen request
	Send(preqListen, REPLYCALLBACK(DdmConsoleHbc,
		SystemConfigSettingsListenReply));
	
}

STATUS DdmConsoleHbc::SystemConfigSettingsListenReply(Message* pReply_)
{
	TRACE_ENTRY(DdmConsoleHbc::SystemConfigSettingsListenReply);

	SystemConfigSettingsRecord::RqListen* pReply = 
		(SystemConfigSettingsRecord::RqListen*) pReply_;
		
	// normal listen reply
	assert(pReply->GetTableCount() == 1);
	delete m_pSystemConfigRecord;
	m_pSystemConfigRecord = pReply->GetRowCopy();
	IP_address[0] = IPa(m_pSystemConfigRecord->ipAddress);
	IP_address[1] = IPb(m_pSystemConfigRecord->ipAddress);
	IP_address[2] = IPc(m_pSystemConfigRecord->ipAddress);
	IP_address[3] = IPd(m_pSystemConfigRecord->ipAddress);
	subnet_mask[0] = IPa(m_pSystemConfigRecord->subnetMask);
	subnet_mask[1] = IPb(m_pSystemConfigRecord->subnetMask);
	subnet_mask[2] = IPc(m_pSystemConfigRecord->subnetMask);
	subnet_mask[3] = IPd(m_pSystemConfigRecord->subnetMask);
	default_gateway[0] = IPa(m_pSystemConfigRecord->gateway);
	default_gateway[1] = IPb(m_pSystemConfigRecord->gateway);
	default_gateway[2] = IPc(m_pSystemConfigRecord->gateway);
	default_gateway[3] = IPd(m_pSystemConfigRecord->gateway);
	
	if (pReply->IsInitialReply())
		Reply(pInitMsg);
	
	return OK;
}

/*************************************************************************/
// Enable -- Process Enable
/*************************************************************************/

STATUS DdmConsoleHbc::Enable(Message *pArgMsg) {

	TRACE_ENTRY(DdmConsoleHbc::Enable);
	
	//spawn the handlekeys thread
	handleKeysTask = new(tZERO) CT_Task;		
	handleKeysStack = new(tZERO) U8[32768]; // DEF_STK_SIZE
	Kernel::Create_Thread(*handleKeysTask, "handle_key_thread", HbcConsoleHandleKeys, 
		this, handleKeysStack, 32768);
	Kernel::Schedule_Thread(*handleKeysTask);
	
	Reply(pArgMsg);
	
	return OK;
}

/*************************************************************************/
// DisplayMenu -- Display HBC Console Menu
/*************************************************************************/


STATUS DdmConsoleHbc::DisplayMenu()
{
	TRACE_ENTRY(DdmConsoleHbc::DisplayMenu);
	
	column = COLUMN_INIT;
	line = 1;
	
	ttyport_printf(CONSOLE_PORT, "%s", clrscn);
	ttyport_printf_at(CONSOLE_PORT, line++, column, 
	"***************************************************************\n");
	ttyport_printf_at(CONSOLE_PORT, line++, column, 
	"***  ConvergeNet Technologies - Odyssey 2000 Demonstration  ***\n");
	ttyport_printf_at(CONSOLE_PORT, line++, column, 
	"***                        HBC Image                        ***\n");
	ttyport_printf(CONSOLE_PORT,
	"    ***  A - Set IP Addresses (currently %d.%d.%d.%d)",
	IP_address[0], IP_address[1], IP_address[2],
	IP_address[3]);
	ttyport_printf_at(CONSOLE_PORT, line++, column+60, "***\n");
	ttyport_printf(CONSOLE_PORT,
	"    ***  B - Set Sub Net Mask (currently %d.%d.%d.%d)",
	subnet_mask[0], subnet_mask[1], subnet_mask[2],
	subnet_mask[3]);
	ttyport_printf_at(CONSOLE_PORT, line++, column+60, "***\n");
	ttyport_printf(CONSOLE_PORT,
	"    ***  C - Set Default Gateway (currently %d.%d.%d.%d)",
	default_gateway[0], default_gateway[1], default_gateway[2],
	default_gateway[3]);	
	ttyport_printf_at(CONSOLE_PORT, line++, column+60, "***\n");
	ttyport_printf_at(CONSOLE_PORT, line++, column, 
	"***  D - Enter PPP Mode                                     ***\n");
/*	ttyport_printf_at(CONSOLE_PORT, line++, column,
	"***  X - Exit                                               ***\n");*/
	ttyport_printf_at(CONSOLE_PORT, line++, column, 
	"***************************************************************\n");
	ttyport_printf_at(CONSOLE_PORT, line++, column, "\n");
	
	return OK;
}

		
/*************************************************************************/
// HbcConsoleHandleKeys -- Handle HBC Console Keystrokes
/*************************************************************************/

void HbcConsoleHandleKeys(void* pParam) 
{
	char key;

	TRACE_ENTRY(HbcConsoleHandleKeys);
	
	((DdmConsoleHbc*)pParam)->DisplayMenu();
	
	while(1)
	{
		switch (key = ttyport_getch(CONSOLE_PORT))
		{
            //case 'X':   /* X - cause address exception and return to boot code */
            //case 'x':   /* X - cause address exception and return to boot code */
            //   ttyport_printf(CONSOLE_PORT, "Exitting with exception\r\n\r\n");
			//	unsigned long *d = ( unsigned long * ) 0x1;
            //    *d = 0xFFFF0000;
            //    break;

            case ' ':  /* SPACEBAR - redraw the screen */
				((DdmConsoleHbc*)pParam)->DisplayMenu();
                break;

            case 0x08:  /* BACKSPACE */
            case 0x09:  /* TAB */
            case 0x1B:  /* ESC */
        		ttyport_printf(CONSOLE_PORT, " /007");
                break;
                
            case 0x0D:  /* ENTER */
        	case 0x0A:  /*  or the real ENTER */
				ttyport_printf(CONSOLE_PORT, "\n\r");
                break;
	        case 'A':   //Set IP Address
	        case 'a':   //Set IP Address
	        {
	        	((DdmConsoleHbc*)pParam)->SetIPAddress();
				break;
			}
	        case 'B':	// Set subnet mask
	        case 'b':	// Set subnet mask
	        {
	        	((DdmConsoleHbc*)pParam)->SetSubnetMask();
				break;
			}
	        case 'C':	// Set default gateway
	        case 'c':	// Set default gateway
	        {
	        	((DdmConsoleHbc*)pParam)->SetDefaultGateway();
	            break;
	   		}
	        case 'D':
	        case 'd':
	      		#if false
	        	ttyport_printf(CONSOLE_PORT, "Entering PPP Mode...\n");
	        	RqNetEnterPPPMode*	pNewMsg = (RqNetEnterPPPMode*)new RqNetEnterPPPMode;
	        	Send(pNewMsg, REPLYCALLBACK(DdmConsoleHbc, PPPModeDisabled);
	        	Kernel::Suspend_Thread(Kernel::Current_Thread_Pointer());
	        	#else
	        	ttyport_printf(CONSOLE_PORT, "Not yet implemented...\n");
	        	NU_Sleep(DELAY);
	        	((DdmConsoleHbc*)pParam)->DisplayMenu();
	      		#endif
				break;
			
			default:
				ttyport_printf(CONSOLE_PORT, "Invalid choice...\n");
				NU_Sleep(DELAY);
				((DdmConsoleHbc*)pParam)->DisplayMenu();
			    break;
		}	// switch
	}	// while (1)
	
}

void DdmConsoleHbc::SetIPAddress()
{
	SystemConfigSettingsRecord::RqModifyField *preqModifyField;

	IP_address[0] = (unsigned char)ttyport_Get_U32(CONSOLE_PORT,
	        		"Enter IP address, 1st byte :  ");
	IP_address[1] = (unsigned char)ttyport_Get_U32(CONSOLE_PORT,
					"Enter IP address, 2nd byte :  ");
	IP_address[2] = (unsigned char)ttyport_Get_U32(CONSOLE_PORT,
					"Enter IP address, 3rd byte :  ");
	IP_address[3] = (unsigned char)ttyport_Get_U32(CONSOLE_PORT,
					"Enter IP address, 4th byte :  ");
				
	ttyport_printf(CONSOLE_PORT, "Setting IP address...  ");
	
	preqModifyField = new SystemConfigSettingsRecord::RqModifyField(
					m_pSystemConfigRecord->rid,
					"ipAddress", IP_address, sizeof(IP_ADDRESS));	
	assert (preqModifyField);
						
	// send modify field request
	Send(preqModifyField, REPLYCALLBACK (DdmConsoleHbc, 
				SystemConfigSettingTableModified));
				
}

void DdmConsoleHbc::SetSubnetMask()
{
	SystemConfigSettingsRecord::RqModifyField *preqModifyField;

	line++;
	subnet_mask[0] = (unsigned char)ttyport_Get_U32(CONSOLE_PORT,
	        		"Enter Subnet Mask, 1st byte :  ");
	subnet_mask[1] = (unsigned char)ttyport_Get_U32(CONSOLE_PORT,
					"Enter Subnet Mask, 2nd byte :  ");
	subnet_mask[2] = (unsigned char)ttyport_Get_U32(CONSOLE_PORT,
					"Enter Subnet Mask, 3rd byte :  ");
	subnet_mask[3] = (unsigned char)ttyport_Get_U32(CONSOLE_PORT,
					"Enter Subnet Mask, 4th byte :  ");
				
	ttyport_printf(CONSOLE_PORT, "Setting Subnet Mask...  ");

	preqModifyField = new SystemConfigSettingsRecord::RqModifyField(
					m_pSystemConfigRecord->rid,
					"subnetMask", subnet_mask, sizeof(IP_ADDRESS));	
	assert (preqModifyField);
						
	// send modify field request
	Send(preqModifyField, REPLYCALLBACK (DdmConsoleHbc, 
				SystemConfigSettingTableModified));
				
}

void DdmConsoleHbc::SetDefaultGateway()
{
	SystemConfigSettingsRecord::RqModifyField *preqModifyField;

	default_gateway[0] = (unsigned char)ttyport_Get_U32(CONSOLE_PORT,
	        		"Enter Default Gateway, 1st byte :  ");
	default_gateway[1] = (unsigned char)ttyport_Get_U32(CONSOLE_PORT,
					"Enter Default Gateway, 2nd byte :  ");
	default_gateway[2] = (unsigned char)ttyport_Get_U32(CONSOLE_PORT,
					"Enter Default Gateway, 3rd byte :  ");
	default_gateway[3] = (unsigned char)ttyport_Get_U32(CONSOLE_PORT,
					"Enter Default Gateway, 4th byte :  ");
				
	ttyport_printf(CONSOLE_PORT, "Setting Default Gateway...  ");

	preqModifyField = new SystemConfigSettingsRecord::RqModifyField(
					m_pSystemConfigRecord->rid,
					"gateway", default_gateway, sizeof(IP_ADDRESS));	
	assert (preqModifyField);
						
	// send modify field request
	Send(preqModifyField, REPLYCALLBACK (DdmConsoleHbc, 
				SystemConfigSettingTableModified));
				
}

#ifdef false
STATUS DdmConsoleHbc::PPPModeDisabled(Message* pReply)
{
	TRACE_ENTRY(DdmConsoleHbc::PPPModeDisabled);
	
	Kernel::Schedule_Thread(*handleKeysTask);
	
	delete pReply;
	
	return OK;
}
#endif

STATUS DdmConsoleHbc::SystemConfigSettingTableModified(Message* pReply)
{
	TRACE_ENTRY(DdmConsoleHbc::SystemConfigSettingTableModified);
	
	ttyport_printf(CONSOLE_PORT, "done  \n");	
	
	NU_Sleep(DELAY);
	
	DisplayMenu();
		
	delete pReply;
	
	return OK;
}

//==========================================================================
//
//	End of DdmConsoleHbc
//
//==========================================================================
