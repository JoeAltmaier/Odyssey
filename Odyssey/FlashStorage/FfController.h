/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
// File: FfController.h
// 
// Description:
// This file defines the interface to the Flash File 
// Controller. 
// 
// 8/12/98 Jim Frandeen: Create file
/*************************************************************************/
#if !defined(FbController_H)
#define FbController_H

#include "Callback.h"
#include "FlashAddress.h"
#include "FlashDevice.h"
#include "FfSgl.h"
#include "List.h"

#ifdef THREADX
#include "tx_api.h"
#else
#include "Nucleus.h"
#endif

#pragma pack(1)

class FF_Controller;
class FF_Interface;
class FF_Page_Map;

/*************************************************************************/
// FF_Controller_Context
// This is the callback context used by FF_Controller_Instance methods.
// This context is always a child context of a FF_Request_Context.
/*************************************************************************/
class FF_Controller_Context : public Callback_Context
{
private: // member data

	// FF_Controller is a friend so it can access its context data
	friend class FF_Controller;
	
	FF_Controller			*m_p_controller;
	Flash_Address			 m_flash_address;
	U32						 m_page_count;
	FF_SGL					 m_sgl;
	void					*m_p_data_buffer;
	U32						 m_block_count;
	U32						 m_erase_all_pages;
	U32						 m_flags;
	U32						 m_retry_count;

}; // FF_Controller_Context

// Write and erase flags
#define FF_RETRY_WRITE_IF_ERROR			0x001
#define FF_NO_RETRY_WRITE_IF_ERROR		0x000
#define FF_NOT_WRITING_BAD_BLOCK_TABLE	0x000
#define FF_WRITING_BAD_BLOCK_TABLE		0x002
#define FF_NO_VERIFY					0x004

// Read_Page flags
// FF_VERIFY_READ and FF_VERIFY_ERASE are defined in FfDevice.h
// for use with the Verify_Page method.
#ifndef FF_VERIFY_READ
#define FF_VERIFY_READ					0x008
#define FF_VERIFY_ERASE					0x010
#endif

#define FF_REPORT_ECC_ERROR				0x020


#define FF_MAX_ERASE_RETRY 10

/*************************************************************************/
// FF_Controller
/*************************************************************************/
class FF_Controller
{
public:
	// Initialize object
	Status Initialize(FF_Interface *p_flash);

	// Read page using SGL.
	// If verify, compare contents of buffer to flash data.
	Status Read_Page(
		Callback_Context *p_callback_context, 
		Flash_Address flash_address,
		FF_SGL *p_sgl,
		U32 flags = 0);
		
	// Read page into data buffer
	// If verify, compare contents of buffer to flash data.
	Status Read_Page(
		Callback_Context *p_callback_context, 
		Flash_Address flash_address,
		void *p_buffer,
		U32 flags = 0);
		
	// Write page using SGL.
	Status Write_Page(
		Callback_Context *p_callback_context,
		Flash_Address flash_address,
		FF_SGL *p_sgl,
		U32 flags = 0);

	// Write page from data buffer
	Status Write_Page(
		Callback_Context *p_callback_context,
		Flash_Address flash_address,
		void *p_buffer,
		U32 flags = 0);
		//int retry_write_if_error = FF_RETRY_WRITE_IF_ERROR,
		//int writing_bad_block_table = FF_NOT_WRITING_BAD_BLOCK_TABLE);

	Status Create_Format_Unit_Context(
		Callback_Context *p_callback_context,
		U32 unit, FF_CONFIG *p_config);

	// Erase block
	Status Erase_Page_Block(
		Callback_Context *p_callback_context, 
		Flash_Address flash_address,
		U32 flags = 0);

	void Find_Waiting_Contexts();

private: // helper methods

	int Is_Buss_Busy(FF_Controller_Context *p_controller_context);

	int Is_Unit_Busy(FF_Controller_Context *p_controller_context);

	void Mark_Unit_Not_Busy(Flash_Address flash_address);
	void Mark_Buss_Not_Busy(Flash_Address flash_address);

	Status Erase_Page_Block_If_Good(FF_Controller_Context *p_controller_context);

private: // callback methods

	static void Authenticate_Write				(void *p_context, Status status);
	static void Erase_Complete					(void *p_context, Status status);
	static void Erase_Wait_Controller			(void *p_context, Status status);
	static void Format_Unit						(void *p_context, Status status);
	static void Read_Complete					(void *p_context, Status status);
	static void Read_Wait_Controller			(void *p_context, Status status);
	static void Read_Unit_Ready					(void *p_context, Status status);
	static void Retry_Write						(void *p_context, Status status);
	static void Verify_Next_Erased_Page			(void *p_context, Status status);
	static void Verify_Write					(void *p_context, Status status);
	static void Write_Cache_Callback_Complete	(void *p_context, Status status);
	static void Write_Complete					(void *p_context, Status status);
	static void Write_Wait_Controller			(void *p_context, Status status);

private: // member data

	// Pointer to interface object set up by Initialize.
	FF_Interface			*m_p_flash;

	// The buss is either busy (m_buss_context points to a context), or free.
	FF_Controller_Context	*m_buss_context;

	// A controller can only do one data transfer at any one time.  
	// If the controller is busy, the command context must be queued.  
	// Each time the controller
	// passes from the busy state to the free state, the buss wait
	// list is checked to see if a context is waiting.  If so, the next
	// context is removed from the wait list and placed back on the ready
	// list to be executed. 
	LIST	m_context_list_wait_buss;
	
	// If the controller is not busy, the unit could be busy finishing
	// a write or an erase operation.  If the unit is busy, the
	// context must be queued.  So each unit must have a wait list. 
	// Each time a unit write or erase command completes, a check is
	// made to see if another context is waiting for that unit. If so,
	// the next context is removed from the wait list and placed back on the
	// ready list to be executed.
	LIST	m_context_list_wait_unit[FF_NUM_UNITS_MAX];

	// Each unit has a callback context associated with it.  If the callback 
	// context is not empty, the unit is waiting for a command to
	// complete - either it is waiting for an IO operation, such as read or 
	// write, or it is waiting for a timer to go off in order to check the
	// status.  When a command completes, the context is removed.
	// However, when a write or erase command
	// completes, the context must wait until the
	// timer expires.  When the timer expires, a get status command is
	// sent to the controller to get the status of the operation.
	FF_Controller_Context	*m_p_controller_context[FF_NUM_UNITS_MAX];

	// With the SSD, units can overlap operations.  It is possible to
	// erase unit 0 while unit 1 is performing another operation.
	// With the HBC, only one unit can be active at any one time.
	U32						 m_can_units_overlap;

#if 0
	FF_INTERRUPT_STATUS 	 m_pending_operations;

	// Watchdog timer.
	
#ifdef THREADX

	TX_TIMER			 m_watchdog_timer;

	// ThreadX disables and enables interrupts.
	U32				m_previous_interrupt_posture;
	
#define ENTER_CRITICAL_SECTION \
	m_previous_interrupt_posture = tx_interrupt_control(TX_INT_DISABLE);

#define LEAVE_CRITICAL_SECTION \
	tx_interrupt_control(m_previous_interrupt_posture);

#else

	// Nucleus or windows
	NU_TIMER			 	m_watchdog_timer;

#ifdef _WINDOWS
	CRITICAL_SECTION	m_critical_section;
#else

	// Nucleus
	NU_PROTECT			m_protect_struct;

#define ENTER_CRITICAL_SECTION \
	NU_Protect(&m_protect_struct);

#define LEAVE_CRITICAL_SECTION \
	NU_Unprotect();
#endif // Nucleus
#endif // _WINDOWS
#endif

}; // FF_Controller


#endif // FbController_H

