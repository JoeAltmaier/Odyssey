/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: SsdDevice.cpp
// 
// Description:
// This file implements the unit interface for the Flash File test. 
// 
// 1/25/99 Jim Frandeen: Create file
// 6/30/99 Jim Frandeen: Implement interleaving
/*************************************************************************/

#include "ErrorLog.h"
#include "Simple.h"
#include "Galileo.h"
#include "Callback.h"
#include "Dma.h"
#include "ErrorLog.h"
#include "FfController.h"
#include "FfCommon.h"
#include "SsdDevice.h"
#include "SsdDeviceISR.h"
#include "FfWatchdog.h"
#include "Hw.h"
#include "pcidev.h"
#include "Trace_Index.h"
#define TRACE_INDEX TRACE_SSD
#include "Odyssey_Trace.h"

#define EOL "\n"

// Set this flag if the FPGA supports read verify.
//#define FPGA_VERIFY_READ_IMPLEMENTED 1

// Set this flag if the FPGA supports erase verify.
//#define FPGA_VERIFY_ERASE_IMPLEMENTED 1

// Set this flag if the FPGA supports ECC.
//#define FPGA_ECC_IMPLEMENTED 1

enum {DmaFlags = GT_DMA_CHAN_EN | GT_DMA_XMIT_64 | GT_DMA_INT_MODE | GT_DMA_FET_NEXTREC};


/*************************************************************************/
// Unit globals
/*************************************************************************/
extern U32					FF_bit_mask[32];
SSD_Device					*FF_p_unit; // pointer to our unit
U32							FF_disabled; // FPGA is disabled

// We use one instance of the watchdog object.
FF_Watchdog					FF_watchdog;

// Flags will have FLASH_ARRAY_0_INSTALLED and maybe FLASH_ARRAY_1_INSTALLED
U32 array_installed_flags;

/*************************************************************************/
// Page Pusher FPGA Registers
/*************************************************************************/
UI64	*FPGA_data_register = (UI64*)				(KSEG1_BASE_ADDR + 0x1C800000);
UI64	*FPGA_array_column_register = (UI64*)		(KSEG1_BASE_ADDR + 0x1C801000);
UI64	*FPGA_block_address_register = (UI64*)		(KSEG1_BASE_ADDR + 0x1C802000);
UI64	*FPGA_control_register = (UI64*)			(KSEG1_BASE_ADDR + 0x1C805000);
UI64	*FPGA_status_register = (UI64*)				(KSEG1_BASE_ADDR + 0x1C808000);
UI64	*FPGA_ready_busy_status = (UI64*)			(KSEG1_BASE_ADDR + 0x1C809000);
UI64	*FPGA_read_ECC_register = (UI64*)			(KSEG1_BASE_ADDR + 0x1C80D000);

/*************************************************************************/
// Unit parameters for Galileo BAnk 1
/*************************************************************************/
#define FPGA_parameter_register 0x00000460

#define TurnOff		(0x2 << 0)
#define AccToFirst	(0x9 << 3)
#define AccToNext	(0x2 << 7)
#define ALEtoWr		(0x3 << 11)
#define WrActive	(0x1 << 14)
#define WrHigh		(0x1 << 17)
#define DevWidth	(0x3 << 20)
#define DMAFlyBy	(0x0 << 22)
#define DevLoc		(0x0 << 23)

U32 FPGA_parameters = TurnOff | AccToFirst | AccToNext 
	| ALEtoWr | WrActive | WrHigh | DevWidth | DMAFlyBy | DevLoc;

// TEMPORARY
#define AccToFirst_2	(0xA << 3)
U32 FPGA_parameters_2 = TurnOff | AccToFirst_2 | AccToNext 
	| ALEtoWr | WrActive | WrHigh | DevWidth | DMAFlyBy | DevLoc;

/*************************************************************************/
// FPGA Control register values
/*************************************************************************/
#define LOAD_ADDRESS_FIFO_REGISTER				0x0000001	// bit 0 
#define NON_INTERLEAVED_WRITE_NORMAL_DMA		0x0000002	// bit 1
#define NON_INTERLEAVED_READ_NORMAL_DMA			0x0000004	// bit 2
#define INTERLEAVED_WRITE_NORMAL_DMA			0x0000008	// bit 3
#define INTERLEAVED_READ_NORMAL_DMA				0x0000010	// bit 4
#define INTERLEAVED_WRITE_FLY_BY_DMA			0x0000020	// bit 5
#define INTERLEAVED_READ_FLY_BY_DMA				0x0000040	// bit 6
#define NON_INTERLEAVED_BLOCK_ERASE				0x0000080	// bit 7
#define INTERLEAVED_BLOCK_ERASE					0x0000100	// bit 8
#define NON_INTERLEAVED_STATUS_READ				0x0000200	// bit 9
#define INTERLEAVED_STATUS_READ					0x0000400	// bit 10
#define NON_INTERLEAVED_ID_READ					0x0000800	// bit 11
#define INTERLEAVED_ID_READ						0x0001000	// bit 12
#define INTERLEAVED_VERIFY_NORMAL_DMA			0x0002000	// bit 13
#define ABORT_COMMAND							0x0004000	// bit 14
#define INTERLEAVED_VERIFY_ERASED				0x0008000	// bit 15

#define READY_BUSY_STATUS_ENABLE				0x0010000	// bit 16
#define READY_BUSY_INTERRUPT_ENABLE				0x0020000	// bit 17
#define ECC_ERROR_INTERRUPT_ENABLE				0x0040000	// bit 18
#define WRITE_DATA_PARITY_INTERRUPT_ENABLE		0x0080000	// bit 19
#define WRITE_FIFO_OVERFLOW_INTERRUPT_ENABLE	0x0100000	// bit 20
#define READ_FIFO_UNDERFLOW_INTERRUPT_ENABLE	0x0200000	// bit 21
#define DMA_LENGTH_ERROR_INTERRUPT_ENABLE		0x0400000	// bit 22
#define COMMAND_OVERWRITE_INTERRUPT_ENABLE		0x0800000	// bit 23
#define FLASH_ARRAY_0_INSTALLED					0x1000000	// bit 24
#define FLASH_ARRAY_1_INSTALLED					0x2000000	// bit 25

/*************************************************************************/
// FPGA Status register values
/*************************************************************************/
#define GLOBAL_INTERRUPT_PENDING				0x000001	// bit 0 
#define OPERATION_IN_PROCESS					0x000002	// bit 1
#define WRITE_IN_PROCESS						0x000004	// bit 2
#define READ_IN_PROCESS							0x000008	// bit 3
#define ERASE_IN_PROCESS						0x000010	// bit 4
#define STATUS_READ_IN_PROCESS					0x000020	// bit 5
#define ARRAY_0_READY_BUSY_INTERRUPT_PENDING	0x000080	// bit 7
#define ARRAY_1_READY_BUSY_INTERRUPT_PENDING	0x000100	// bit 8
#define READ_ECC_ERROR_INTERRUPT_PENDING		0x000200	// bit 9
#define WRITE_PARITY_ERROR_INTERRUPT_PENDING	0x000400	// bit 10
#define WRITE_OVERFLOW_ERROR_INTERRUPT_PENDING	0x000800	// bit 11
#define READ_UNDERFLOW_ERROR_PENDING			0x001000	// bit 12
#define TRANSFER_LENGTH_ERROR_PENDING			0x002000	// bit 13
#define COMMAND_OVERWRITE_ERROR_PENDING			0x004000	// bit 14

/*************************************************************************/
// ARRAY_COLUMN_REGISTER
// creates a dword with array, bank and column in FPGA format.
/*************************************************************************/
#define ARRAY_COLUMN_REGISTER(flash_address) \
	((flash_address.Array() << 12) \
	| flash_address.Column())
	
/*************************************************************************/
// Interleaved Flash Address
// We must send the flash address in interleaved form.
/*************************************************************************/
typedef struct {
	union
	{
		U8		address_byte[96];
		UI64	address_dword[12];
	};
} Interleaved_Flash_Address;


/*************************************************************************/
// Disable/Enable FPGA interrupts.  Leave timer (x8000) running.
/*************************************************************************/
#define FPGA_INTERRUPTS 0x80000 // interrupt level 9
#define DISABLE_FPGA \
	INT old_level = NU_Control_Interrupts(0x8000); \
	NU_Control_Interrupts(old_level & ~FPGA_INTERRUPTS); \
	FF_disabled = 1;
#define ENABLE_FPGA \
	FF_disabled = 0; \
	NU_Control_Interrupts(old_level | FPGA_INTERRUPTS); 

/*************************************************************************/
// SSD_Device::Abort_Command
/*************************************************************************/
Status SSD_Device::Abort_Command(Flash_Address flash_address)
{
 	TRACEF( TRACE_L5, 
		(EOL "SSD_Device::Abort_Commnad, array = %d, column = %d", 
		flash_address.Array(), 
		flash_address.Column()));
		
	DISABLE_FPGA;

	// Get array, bank 0, column in a format the FPGA can understand.
	UI64 array_column_register = ARRAY_COLUMN_REGISTER(flash_address);

	// Send array, column to FPGA.
	*FPGA_array_column_register = array_column_register;

	// Set up the FPGA control register
	UI64 control_value;
	control_value = ABORT_COMMAND
		| READY_BUSY_STATUS_ENABLE
		| READY_BUSY_INTERRUPT_ENABLE
		| array_installed_flags;
	
	*FPGA_control_register = control_value;
	
	ENABLE_FPGA;

	return OK;

} // SSD_Device::Get_Device_Status

/*************************************************************************/
// SSD_Device::Device_Complete
// The device has completed the transfer.
// Terminate this child context.  
// When both the DMA child and the device child complete, the parent will run.
/*************************************************************************/
void SSD_Device::Device_Complete(void *p_context, Status status)
{
	Callback_Context *p_callback_context = (Callback_Context *)p_context;
	
	// Get the device-dependent controller status that was
	// passed to the context by the interrupt handler.
	UI64 controller_status = p_callback_context->Get_Return_Value();
	
	// Point to the parent context.
	Callback_Context *p_parent = p_callback_context->Get_Parent();
	
	// Save the controller status in the parent context.
	p_parent->Set_Return_Value(controller_status);

	Callback_Context::Terminate(p_context, status);
	
} // Device_Complete

/*************************************************************************/
// SSD_Device::DMA_Complete
// The DMA of data from buffer to FPGA has completed.
// Terminate this child context.  
// When both the DMA child and the device child complete, the parent will run.
/*************************************************************************/
void SSD_Device::DMA_Complete(void *p_context, Status status)
{
	Callback_Context::Terminate(p_context, status);
	
} // DMA_Complete

/*************************************************************************/
// SSD_Device::Close
/*************************************************************************/
Status SSD_Device::Close()
{
	// Close the interrupt service routine.
	Status status = FF_ISR_Close();
	if (status != OK)
		return status;
		
	m_is_open = false;

	// Close the watchdog timers.
	return FF_watchdog.Close();
	
} // SSD_Device::Close

/*************************************************************************/
// SSD_Device::Erase_Page_Block
/*************************************************************************/
Status SSD_Device::Erase_Page_Block(
	Callback_Context *p_callback_context, 
	Flash_Address flash_address,
	Flash_Page_Map flash_page_map
	)
{
 	TRACEF( TRACE_L5, 
		(EOL "SSD_Device::Erase_Page_Block, array = %d, column = %d, block = %d", 
		flash_address.Array(), 
		flash_address.Column(), flash_address.Block()));
 	
	// Create unit index from bank and column.
	// Get unit index -- concatenation of array and column, e.g.,
	// ACCC if Array is one bit, and Column is 3 bits.
	U32 unit_index = flash_address.Unit_Index();
	
	// Get array, bank 0, column in a format the FPGA can understand.
	UI64 array_column_register = ARRAY_COLUMN_REGISTER(flash_address);

	// Create interleaved flash address.
	// We don't send a low address byte for erase.
	Interleaved_Flash_Address	interleaved_flash_address;
	for (U32 device_index = 0; device_index < 32; device_index++)
	{
		// Get page address for next device.
		U32 page_address = flash_page_map[device_index];

		// Get mid byte of page address.
		U32 mid_byte = page_address & 0xFF;

		// Store mid byte of page address in interleaved address.
		interleaved_flash_address.address_byte[device_index + 32] = (U8)mid_byte;

		// Get high byte of page address.
		U32 hi_byte = (page_address >> 8) & 0xFF;

		// Store hi byte of page address in interleaved address.
		interleaved_flash_address.address_byte[device_index + 64] = (U8)hi_byte;
	}

	DISABLE_FPGA;


	// Read the PP-FPGA Flash status register.
	UI64 flash_status = *FPGA_status_register;
	
	// There must not be any other operation in progress.
	CT_ASSERT(((flash_status & OPERATION_IN_PROCESS) == 0), Erase_Page_Block);

	// Send array, column to FPGA.
	*FPGA_array_column_register = array_column_register;

	// Send command to receive page address (clears the FIFO).
	UI64 control_value = LOAD_ADDRESS_FIFO_REGISTER | array_installed_flags;
	*FPGA_control_register = control_value;

	// Send interleaved flash address to FPGA Flash Array
	// Block address registers.
	// We don't send a low address byte for erase.
	
#if 0
	U32			m_protect_struct;
	ENTER_CRITICAL_SECTION;
	
	// The trace is just to make sure we actually send the data.
	
	*FPGA_block_address_register = interleaved_flash_address.address_dword[4];
	Tracef("\n%LX", interleaved_flash_address.address_dword[4]);
	
	*FPGA_block_address_register = interleaved_flash_address.address_dword[5];
	Tracef("  %LX", interleaved_flash_address.address_dword[5]);

	*FPGA_block_address_register = interleaved_flash_address.address_dword[6];
	Tracef("  %LX", interleaved_flash_address.address_dword[6]);

	*FPGA_block_address_register = interleaved_flash_address.address_dword[7];
	Tracef("  %LX", interleaved_flash_address.address_dword[7]);

	*FPGA_block_address_register = interleaved_flash_address.address_dword[8];
	Tracef("\n%LX", interleaved_flash_address.address_dword[8]);

	*FPGA_block_address_register = interleaved_flash_address.address_dword[9];
	Tracef("  %LX", interleaved_flash_address.address_dword[9]);

	*FPGA_block_address_register = interleaved_flash_address.address_dword[10];
	Tracef("  %LX", interleaved_flash_address.address_dword[10]);

	*FPGA_block_address_register = interleaved_flash_address.address_dword[11];
	Tracef("  %LX", interleaved_flash_address.address_dword[11]);
	
	LEAVE_CRITICAL_SECTION;

#else
	// TEMPORARY bandaid.  
	// If we don't disable interrupts here, the addresses can get corrupted!
	U32			m_protect_struct;
	ENTER_CRITICAL_SECTION;
	
	// Make sure FPGA FIFO is ready.
	while (*FPGA_status_register & OPERATION_IN_PROCESS)
		;
	
	*FPGA_block_address_register = interleaved_flash_address.address_dword[4];
	*FPGA_block_address_register = interleaved_flash_address.address_dword[5] /*+ 0x0101010101010101*/;
	*FPGA_block_address_register = interleaved_flash_address.address_dword[6] /*+ 0x0202020202020202*/;
	*FPGA_block_address_register = interleaved_flash_address.address_dword[7] /*+ 0x0303030303030303*/;
	*FPGA_block_address_register = interleaved_flash_address.address_dword[8];
	*FPGA_block_address_register = interleaved_flash_address.address_dword[9] /*+ 0x0101010101010101*/;
	*FPGA_block_address_register = interleaved_flash_address.address_dword[10] /*+ 0x0202020202020202*/;
	*FPGA_block_address_register = interleaved_flash_address.address_dword[11] /*+ 0x0303030303030303*/;
	
	LEAVE_CRITICAL_SECTION;
#endif

	// Set up the FPGA control register
	control_value = INTERLEAVED_BLOCK_ERASE
		| READY_BUSY_STATUS_ENABLE
		| READY_BUSY_INTERRUPT_ENABLE
		| array_installed_flags;
	
	*FPGA_control_register = control_value;
	
	// Start the watchdog timer to wait for the interrupt.
	// The interrupt routine calls Wake_Up_Waiting_Context to stop the 
	// watchdog timer.
	FF_watchdog.Start_Watchdog(p_callback_context, unit_index, 
		FF_ERASE_TIMER_WATCHDOG_TICKS);
	
	ENABLE_FPGA;

	// Currently, the caller's context is scheduled to run
	// when the interrupt occurs.
	// p_callback_context->Make_Ready();
	return OK;

}  // SSD_Device::Erase_Page_Block

/*************************************************************************/
// SSD_Device::Get_Capacity
// This is the first unit method called.
// Return a Flash_Capacity record to specify the number of units,
// bytes per page, etc.
/*************************************************************************/
Status SSD_Device::Get_Capacity(Flash_Capacity *p_flash_capactiy)
{
	// Save pointer to our unit for the interrupt service routine.
	FF_p_unit = this;
	
	DISABLE_FPGA;

	// Set up the FPGA control register to reset.
	UI64 control_value;
	control_value = ABORT_COMMAND
		| array_installed_flags;
	*FPGA_control_register = control_value;
	
	// Clear ready/busy interrupts.
	UI64 interrupt_status = *FPGA_ready_busy_status;
	
	ENABLE_FPGA;

	// Set up the FPGA parameter register --  
	// the Galileo unit bank 1 register
	gt_write(FPGA_parameter_register, FPGA_parameters);
	
	// Assume that array 0 is installed.
	unsigned long  unit_ID;
	Status status = Get_Device_ID(0, &unit_ID);
	if (status != OK)
		return status;
	if((unit_ID != FF_DEVICE_ID_TOSHIBA_256) && (unit_ID != FF_DEVICE_ID_SAMSUNG_128))
	{
		Tracef("\nInvalid flash device, unit ID = %LX", unit_ID);
		return FF_ERROR(INVALID_FLASH_DEVICE);
	}
		
	array_installed_flags = FLASH_ARRAY_0_INSTALLED;

	// Check to see if array 1 is installed.
	status = Get_Device_ID(1, &unit_ID);
	if (status != OK)
		return status;
	if ((unit_ID == FF_DEVICE_ID_TOSHIBA_256) || (unit_ID == FF_DEVICE_ID_SAMSUNG_128))
		array_installed_flags |= FLASH_ARRAY_1_INSTALLED;
	
	// Return unit address capacity information.
	// Do we have two banks installed?
	if (array_installed_flags & FLASH_ARRAY_1_INSTALLED)
		p_flash_capactiy->num_address_bits_for_array = 1; 
	else	
		// Only one bank installed.
		p_flash_capactiy->num_address_bits_for_array = 0; 
		
	if (unit_ID == FF_DEVICE_ID_TOSHIBA_256)
		p_flash_capactiy->num_address_bits_for_block = 11;
	else
		p_flash_capactiy->num_address_bits_for_block = 10;
	 
	p_flash_capactiy->num_address_bits_for_sector = 5; 
	p_flash_capactiy->num_address_bits_for_bank = 2; 
	p_flash_capactiy->num_address_bits_for_column = 3; 
	p_flash_capactiy->num_address_bits_for_device = 3;
	
	// Offset includes num_address_bits_for_bank and num_address_bits_for_device.
	p_flash_capactiy->num_address_bits_for_offset = 14; 
	
	return OK;
	
} // SSD_Device::Get_Capacity

/*************************************************************************/
// Get_Device_ID
// Get Device ID of specified array.
// Note that we cannot use Flash_Address methods yet because Flash_Address
// has not yet been initialized.
/*************************************************************************/
Status SSD_Device::Get_Device_ID(U32 array, FF_DEVICE_ID *p_device_ID)
{	
	// Get device ID for column 0.
	FF_DEVICE_ID device_ID;
	Status status = Get_Device_ID_Column(array, 0, &device_ID);
	
	// Get the device ID for each of the other columns.
	// This is to assure that all devices are working.
	FF_DEVICE_ID device_ID_others;
	for (U32 column = 1; column < 8; column++)
	{
		if (status == OK)
			status = Get_Device_ID_Column(array, column, &device_ID_others);
		else
			Get_Device_ID_Column(array, column, &device_ID_others);
			
		// Make sure we get the same device IDs for each column.
		if (device_ID != device_ID_others)
		{
			// If no error has yet been reported..
			if (status == OK)
			{
				status = FF_ERROR(INVALID_COLUMN);
				Tracef("\nInvalid column in array %d column %d ", array, column);
			}
		}
	}
	
	*p_device_ID = device_ID;
	return status;

} // Get_Device_ID

/*************************************************************************/
// Get_Device_ID_Column
// Get Device ID of specified array and column.
// Note that we cannot use Flash_Address methods yet because Flash_Address
// has not yet been initialized.
/*************************************************************************/
Status SSD_Device::Get_Device_ID_Column(U32 array, U32 column, FF_DEVICE_ID *p_device_ID)
{	
	DISABLE_FPGA;

	// Set up the PP_FPGA address register with the Array / Bank / Column address.
	// Array address is in bits 15:12
	// Bank address is in bits 11:8
	// Column address is in bits 7:0
	U32 array_column_register = (array << 12) | column;
	*FPGA_array_column_register = array_column_register;

	// Set up the FPGA control register
	UI64 control_value;
	control_value = INTERLEAVED_ID_READ
		| READY_BUSY_STATUS_ENABLE
		| array_installed_flags;
	
	*FPGA_control_register = control_value;

	// Create a union so we can access the doubleword received from the 
	// device one byte at a time.
	union
	{
		UI64	bank_IDs;
		U8		device_ID[8];
	};

	// Read maker code.
	Status status = OK;
	U8 maker_code;
	U32 bank_index;
	UI64 register_value;
	for (bank_index = 0; bank_index < 4; bank_index++)
	{
		// Get maker code for all 8 devices in the bank.  
		bank_IDs = *FPGA_read_ECC_register;
		maker_code = device_ID[0];
		int same = true;
		
		// Each maker_code should be the same.
		for (U32 device_index = 1; device_index < 8; device_index++)
		{
			if (device_ID[device_index] != maker_code)
				same = false;
		}
		
		// Check to see if they are all the same.
		if (same == false)
		{
			maker_code = 0;
			register_value = bank_IDs;
#if 0			
			StringClass		message;
			CCtMsgView		message_view;
			// Construct an event in preparation for calling AddEventParameter().
			
			Event event(CTS_SSD_INVALID_UNIT);
			event.AddEventParameter(array);
			event.AddEventParameter(column);
			event.AddEventParameter(bank_index);
			event.AddEventParameter(bank_IDs);
			status = message_view.GetMessageText(message, event);
			//printf(message);
#endif
			Tracef("\nInvalid maker code in array %d column %d bank %d, register value = %LX", 
				array, column, bank_index, bank_IDs);
			status = FF_ERROR(INVALID_MAKER_CODE);
		}
	}

	// Read unit ID..
	U8 unit_code;
	for (bank_index = 0; bank_index < 4; bank_index++)
	{
		// Get unit ID for all 8 devices in the bank.  
		bank_IDs = *FPGA_read_ECC_register;
		unit_code = device_ID[0];
		int same = true;
		
		// Each unit ID should be the same.
		for (U32 device_index = 1; device_index < 8; device_index++)
		{
			if (device_ID[device_index] != unit_code)
				same = false;
		}
		
		// Check to see if they are all the same.
		if (same == false)
		{
			unit_code = 0;
			Tracef("\nInvalid unit code in array %d column %d bank %d,  register value = %LX", 
				array, column, bank_index, bank_IDs);
			if (status == FF_ERROR(INVALID_MAKER_CODE))
				status = FF_ERROR(INVALID_UNIT_MAKER_CODE);
			else
				status = FF_ERROR(INVALID_UNIT_CODE);
		}
	}

	ENABLE_FPGA;

	// Return Maker code in low byte, Unit code in high byte.
	unsigned long  unit_ID = (maker_code | (unit_code << 8));
	*p_device_ID = unit_ID;
	return status;

} // Get_Device_ID_Column

/*************************************************************************/
// SSD_Device::Get_Device_Status
// Get status of operation
/*************************************************************************/
Status SSD_Device::Get_Device_Status(Flash_Address flash_address, 
		UI64 controller_status, Flash_Device_Status *p_device_status)
{
 	TRACEF( TRACE_L5, 
		(EOL "SSD_Device::Get_Device_Status, array = %d, column = %d", 
		flash_address.Array(), 
		flash_address.Column()));
		
	if (controller_status == 0xCAFECAFE)
	{
		// 0xCAFECAFE is a kludge way of getting the controller status from the device
		// in a device-dependent way so we can display it for debugging.
		// The controller status gets stored in device_status.bank_status[0].
		p_device_status->bank_status[0] = *FPGA_status_register;
		
		// Get the DMA count register.
		p_device_status->bank_status[1] = *((U32 *)GT_DMA_CH0_COUNT_REG);
		return OK;
	}

	DISABLE_FPGA;

	// Read the PP-FPGA Flash status register.
	UI64 flash_status = *FPGA_status_register;
	
	// There must not be any other operation in progress.
	CT_ASSERT(((flash_status & OPERATION_IN_PROCESS) == 0), Get_Device_Status);

	// Get array, bank 0, column in a format the FPGA can understand.
	UI64 array_column_register = ARRAY_COLUMN_REGISTER(flash_address);

	// Send array, column to FPGA.
	*FPGA_array_column_register = array_column_register;

	// Set up the FPGA control register
	UI64 control_value;
	control_value = INTERLEAVED_STATUS_READ
		| READY_BUSY_STATUS_ENABLE
		| READY_BUSY_INTERRUPT_ENABLE
		| array_installed_flags;
	
	*FPGA_control_register = control_value;
	
	// Read status from the flash data register.
	// The status contains the status byte for each of 8 units.
	// Low order bit in each byte is the pass/fail bit.
	p_device_status->bank_status[0] = *FPGA_read_ECC_register;
	p_device_status->bank_status[1] = *FPGA_read_ECC_register;
	p_device_status->bank_status[2] = *FPGA_read_ECC_register;
	p_device_status->bank_status[3] = *FPGA_read_ECC_register;
	
	// Check to see if each device is ready.
	// 0X40 in each byte is the ready bit.
	int ready = 0X40;
	U32 device_index;
	for (device_index = 0; device_index < 32; device_index++)
		ready &= p_device_status->device_status[device_index];
	p_device_status->device_is_ready = ready;
	
	// Low order bit in each byte is the pass/fail bit.
	Status status = OK;
	int error = 0;
	for (device_index = 0; device_index < 32; device_index++)
		error |= p_device_status->device_status[device_index] & 1;
	if (error)
		status = FF_ERROR_CODE(DEVICE_ERROR);

	// If we don't already have a device error, check each
	// of the status bits from the page pusher.
	if (status == OK)
	{
		if (controller_status & READ_ECC_ERROR_INTERRUPT_PENDING)
			status = FF_ERROR(ECC);
		else if (controller_status & WRITE_PARITY_ERROR_INTERRUPT_PENDING)
			status = FF_ERROR(WRITE_DATA_PARITY);
		else if (controller_status & WRITE_OVERFLOW_ERROR_INTERRUPT_PENDING)
			status = FF_ERROR(WRITE_DATA_OVERFLOW);
		else if (controller_status & READ_UNDERFLOW_ERROR_PENDING)
			status = FF_ERROR(READ_DATA_UNDERFLOW);
		else if (controller_status & TRANSFER_LENGTH_ERROR_PENDING)
			status = FF_ERROR(TRANSFER_LENGTH);
		else if (controller_status & COMMAND_OVERWRITE_ERROR_PENDING)
			status = FF_ERROR(COMMAND_OVERWRITE);	}

	ENABLE_FPGA;

	return status;

} // SSD_Device::Get_Device_Status

/*************************************************************************/
// SSD_Device::Interrupt_Callback
// This static procedure is called by the interrupt handler when
// an interrupt occurs for a unit.
// unit index is a concatenation of array and column, e.g.,
// ACCC if Array is one bit, and Column is 3 bits.
/*************************************************************************/
void SSD_Device::Interrupt_Callback(U32 unit_index, UI64 controller_status)
{
	FF_watchdog.Wake_Up_Waiting_Context(unit_index, controller_status);

} // Interrupt_Callback

/*************************************************************************/
// SSD_Device::Open
/*************************************************************************/
Status SSD_Device::Open(FF_Mem *p_mem)
{
	if (m_is_open)
		Close();

	// Open the interrupt service routine.
	Status status = FF_ISR_Open(p_mem, &Interrupt_Callback);
	if (status != OK)
		return status;
		
#ifndef FPGA_VERIFY_READ_IMPLEMENTED
	// Allocate buffer for hardware verify.
	m_p_read_verify_buffer = (UI64 *)p_mem->Allocate(Flash_Address::Bytes_Per_Page(), ALIGN64);
#endif
		
	m_is_open = true;

	// Initialize the watchdog timers.
	return FF_watchdog.Open(p_mem, Flash_Address::Num_Units());
	
} // SSD_Device::Open

/*************************************************************************/
// SSD_Device::Read_Page
// Read data from the flash device into the buffer.
/*************************************************************************/
Status SSD_Device::Read_Page(
		Callback_Context *p_callback_context, 
		Flash_Address flash_address, 
		Flash_Page_Map flash_page_map,
		FF_SGL *p_sgl)
{
 	TRACEF( TRACE_L5, 
		(EOL "SSD_Device::Read_Page, array = %d, column = %d, block = %d, sector = %d", 
		flash_address.Array(), 
		flash_address.Column(), flash_address.Block(), flash_address.Sector()));

	// Create TyDma from SGL to transfer page from page pusher to SDRAM.
	TyDma *p_tydma;
	
#ifdef FPGA_ECC_IMPLEMENTED
	// We must wait for a DMA transfer and a device completion.
	// Allocate child of caller's context to do the DMA transfer.
	Callback_Context *p_DMA_context = 
		(Callback_Context *)p_callback_context->Allocate_Child(sizeof(Callback_Context));
	if (p_DMA_context == 0)
	{
		// Return to calling context.  No child context was created.
		return FF_ERROR(NO_CONTEXT);
	}
		
	// Allocate child of caller's context to wait for device completion.
	Callback_Context *p_device_context = 
		(Callback_Context *)p_callback_context->Allocate_Child(sizeof(Callback_Context));
	if (p_device_context == 0)
	{
		// Return to calling context.  No child context was created.
		return FF_ERROR(NO_CONTEXT);
	}
		
	Status status = p_sgl->Build_TyDma(
		FPGA_data_register, Flash_Address::Bytes_Per_Page(),
		FF_SGL::Transfer_Address_Is_Source,  
 		&SSD_Device::DMA_Complete,
 		p_DMA_context,
 		
 		// The source does not get incremented.
 		DmaFlags | GT_DMA_SDIR_HOLD,
		&p_tydma);	
		
#else
	Status status = p_sgl->Build_TyDma(
		FPGA_data_register, Flash_Address::Bytes_Per_Page(),
		FF_SGL::Transfer_Address_Is_Source,  
 		&SSD_Device::Read_DMA_Complete,
 		p_callback_context,
 		
 		// The source does not get incremented.
 		DmaFlags | GT_DMA_SDIR_HOLD,
		&p_tydma);	
#endif	

	if (status != OK)
		return status;
				
	// Get array, column in a format the FPGA can understand.
	UI64 array_column_register = ARRAY_COLUMN_REGISTER(flash_address);

	// Get unit index -- concatenation of array and column, e.g.,
	// ACCC if Array is one bit, and Column is 3 bits.
	U32 unit_index = flash_address.Unit_Index();
	
	// Save unit_index in callback context for DMA completion.
	p_callback_context->Set_Return_Value((UI64)unit_index);

	// Create interleaved flash address.
	Interleaved_Flash_Address	interleaved_flash_address;

	// Store low byte of page address in interleaved address.
	// The low byte is always zero because this addresses A0 - A7,
	// the hardware column address within the page.
	interleaved_flash_address.address_dword[0] = 0;
	interleaved_flash_address.address_dword[1] = 0;
	interleaved_flash_address.address_dword[2] = 0;
	interleaved_flash_address.address_dword[3] = 0;

	for (U32 device_index = 0; device_index < 32; device_index++)
	{
		// Get page address for next device.
		U32 page_address = flash_page_map[device_index];

		// Get mid byte of page address.
		U32 mid_byte = page_address & 0xFF;

		// Store mid byte of page address in interleaved address.
		interleaved_flash_address.address_byte[device_index + 32] = (U8)mid_byte;

		// Get high byte of page address.
		U32 hi_byte = (page_address >> 8) & 0xFF;

		// Store hi byte of page address in interleaved address.
		interleaved_flash_address.address_byte[device_index + 64] = (U8)hi_byte;
	}

	DISABLE_FPGA;

	// Read the PP-FPGA Flash status register.
	UI64 flash_status = *FPGA_status_register;
	
	// There must not be any other operation in progress.
	CT_ASSERT(((flash_status & OPERATION_IN_PROCESS) == 0), Read_Or_Verify_Page);

	// Send array, column to FPGA.
	*FPGA_array_column_register = array_column_register;

	// Send command to receive page address (clears the FIFO).
	UI64 control_value = LOAD_ADDRESS_FIFO_REGISTER | array_installed_flags;
	*FPGA_control_register = control_value;

	// Send interleaved flash address to FPGA Flash Array
	// Block address registers.
	
	// TEMPORARY bandaid.  
	// If we don't disable interrupts here, the addresses can get corrupted!
	U32			m_protect_struct;
	ENTER_CRITICAL_SECTION;
	
	// Make sure FPGA FIFO is ready.
	while (*FPGA_status_register & OPERATION_IN_PROCESS)
		;
	
	*FPGA_block_address_register = interleaved_flash_address.address_dword[0];
	*FPGA_block_address_register = interleaved_flash_address.address_dword[1];
	*FPGA_block_address_register = interleaved_flash_address.address_dword[2];
	*FPGA_block_address_register = interleaved_flash_address.address_dword[3];
	*FPGA_block_address_register = interleaved_flash_address.address_dword[4];
	*FPGA_block_address_register = interleaved_flash_address.address_dword[5];
	*FPGA_block_address_register = interleaved_flash_address.address_dword[6];
	*FPGA_block_address_register = interleaved_flash_address.address_dword[7];
	*FPGA_block_address_register = interleaved_flash_address.address_dword[8];
	*FPGA_block_address_register = interleaved_flash_address.address_dword[9];
	*FPGA_block_address_register = interleaved_flash_address.address_dword[10];
	*FPGA_block_address_register = interleaved_flash_address.address_dword[11];
	
	LEAVE_CRITICAL_SECTION;

	// Start the watchdog timer to wait for the interrupt.
	// The interrupt routine calls Wake_Up_Waiting_Context to stop the 
	// watchdog timer.
	
#ifdef FPGA_ECC_IMPLEMENTED

	// When device completes, call Device_Complete.
	p_device_context->Set_Callback(&Device_Complete);
	
	// If timeout, call Device_Complete.
	FF_watchdog.Start_Watchdog(p_device_context, unit_index, 
		FF_READ_TIMER_WATCHDOG_TICKS);
		
#else
	// If DMA timeout, wake up parent,
	FF_watchdog.Start_Watchdog(p_callback_context, unit_index, 
		FF_READ_TIMER_WATCHDOG_TICKS);
#endif
		
	// Start DMA operation to transfer page from page pusher to SDRAM.
    Dma::Transfer(p_tydma);
	
	// Set up the FPGA control register.
	// This sets the flash into the sequential read mode and prepares to initiate
	// the transfer once the Flash Devices become ready.
	control_value = INTERLEAVED_READ_NORMAL_DMA
		| READY_BUSY_STATUS_ENABLE
		| array_installed_flags;
	
	*FPGA_control_register = control_value;
		
	ENABLE_FPGA;

	return OK;

}  // SSD_Device::Read_Page
	

/*************************************************************************/
// SSD_Device::Verify_Page
// Verify the data by comparing it to the data in the buffer.
/*************************************************************************/
Status SSD_Device::Verify_Page(
		Callback_Context *p_callback_context, 
		Flash_Address flash_address, 
		Flash_Page_Map flash_page_map,
		FF_SGL *p_sgl,
		U32 flags)
{
 	TRACEF( TRACE_L5, 
		(EOL "SSD_Device::Verify_Page, array = %d, column = %d, block = %d, sector = %d", 
		flash_address.Array(), 
		flash_address.Column(), flash_address.Block(), flash_address.Sector()));

#ifndef FPGA_VERIFY_ERASE_IMPLEMENTED

	// Verify erase is not implemented in the FPGA, so verify read and verify erase
	// are implemented the same.
	return Verify_Read(p_callback_context,
		flash_address,
		flash_page_map,
		p_sgl);
#else
	// The FPGA implements verify erase.
	// Is this a verify erase operation?
	if ((flags & FF_VERIFY_ERASE) == 0)
		return Verify_Read(p_callback_context,
			flash_address,
			flash_page_map,
			p_sgl);
			
	// This is a verify erase operation.
	// Get array, column in a format the FPGA can understand.
	UI64 array_column_register = ARRAY_COLUMN_REGISTER(flash_address);

	// Get unit index -- concatenation of array and column, e.g.,
	// ACCC if Array is one bit, and Column is 3 bits.
	U32 unit_index = flash_address.Unit_Index();
	
	// Save unit_index in callback context for DMA completion.
	p_callback_context->Set_Return_Value((UI64)unit_index);

	// Create interleaved flash address.
	Interleaved_Flash_Address	interleaved_flash_address;

	// Store low byte of page address in interleaved address.
	// The low byte is always zero because this addresses A0 - A7,
	// the hardware column address within the page.
	interleaved_flash_address.address_dword[0] = 0;
	interleaved_flash_address.address_dword[1] = 0;
	interleaved_flash_address.address_dword[2] = 0;
	interleaved_flash_address.address_dword[3] = 0;

	for (U32 device_index = 0; device_index < 32; device_index++)
	{
		// Get page address for next device.
		U32 page_address = flash_page_map[device_index];

		// Get mid byte of page address.
		U32 mid_byte = page_address & 0xFF;

		// Store mid byte of page address in interleaved address.
		interleaved_flash_address.address_byte[device_index + 32] = (U8)mid_byte;

		// Get high byte of page address.
		U32 hi_byte = (page_address >> 8) & 0xFF;

		// Store hi byte of page address in interleaved address.
		interleaved_flash_address.address_byte[device_index + 64] = (U8)hi_byte;
	}

	DISABLE_FPGA;

	// Read the PP-FPGA Flash status register.
	UI64 flash_status = *FPGA_status_register;
	
	// There must not be any other operation in progress.
	CT_ASSERT(((flash_status & OPERATION_IN_PROCESS) == 0), Read_Or_Verify_Page);

	// Send array, column to FPGA.
	*FPGA_array_column_register = array_column_register;

	// Send command to receive page address (clears the FIFO).
	UI64 control_value = LOAD_ADDRESS_FIFO_REGISTER | array_installed_flags;
	*FPGA_control_register = control_value;

	// Send interleaved flash address to FPGA Flash Array
	// Block address registers.
	
	// TEMPORARY bandaid.  
	// If we don't disable interrupts here, the addresses can get corrupted!
	U32			m_protect_struct;
	ENTER_CRITICAL_SECTION;
	
	// Make sure FPGA FIFO is ready.
	while (*FPGA_status_register & OPERATION_IN_PROCESS)
		;
	
	*FPGA_block_address_register = interleaved_flash_address.address_dword[0];
	*FPGA_block_address_register = interleaved_flash_address.address_dword[1];
	*FPGA_block_address_register = interleaved_flash_address.address_dword[2];
	*FPGA_block_address_register = interleaved_flash_address.address_dword[3];
	*FPGA_block_address_register = interleaved_flash_address.address_dword[4];
	*FPGA_block_address_register = interleaved_flash_address.address_dword[5];
	*FPGA_block_address_register = interleaved_flash_address.address_dword[6];
	*FPGA_block_address_register = interleaved_flash_address.address_dword[7];
	*FPGA_block_address_register = interleaved_flash_address.address_dword[8];
	*FPGA_block_address_register = interleaved_flash_address.address_dword[9];
	*FPGA_block_address_register = interleaved_flash_address.address_dword[10];
	*FPGA_block_address_register = interleaved_flash_address.address_dword[11];
	
	LEAVE_CRITICAL_SECTION;

	// Start the watchdog timer to wait for the interrupt.
	// The interrupt routine calls Wake_Up_Waiting_Context to stop the 
	// watchdog timer.
	FF_watchdog.Start_Watchdog(p_callback_context, unit_index, 
		FF_READ_TIMER_WATCHDOG_TICKS);
		
	// Set up the FPGA control register.
	control_value = INTERLEAVED_VERIFY_ERASED
		| READY_BUSY_STATUS_ENABLE
		| array_installed_flags;
	
	*FPGA_control_register = control_value;
	
	ENABLE_FPGA;

	return OK;

#endif // FPGA_VERIFY_ERASE_IMPLEMENTED

} // Verify_Page

#ifdef FPGA_VERIFY_READ_IMPLEMENTED
/*************************************************************************/
// SSD_Device::Verify_Read
// Verify the data by comparing it to the data in the buffer.
/*************************************************************************/
Status SSD_Device::Verify_Read(
		Callback_Context *p_callback_context, 
		Flash_Address flash_address, 
		Flash_Page_Map flash_page_map,
		FF_SGL *p_sgl)
{ 	
	TyDma *p_tydma;
	Status status = OK;
	
	// We must wait for a DMA transfer and a device completion.
	// Allocate child of caller's context to do the DMA transfer.
	Callback_Context *p_DMA_context = 
		(Callback_Context *)p_callback_context->Allocate_Child(sizeof(Callback_Context));
	if (p_DMA_context == 0)
	{
		// Return to calling context.  No child context was created.
		return FF_ERROR(NO_CONTEXT);
	}
		
	// Allocate child of caller's context to wait for device completion.
	Callback_Context *p_device_context = 
		(Callback_Context *)p_callback_context->Allocate_Child(sizeof(Callback_Context));
	if (p_device_context == 0)
	{
		// Return to calling context.  No child context was created.
		return FF_ERROR(NO_CONTEXT);
	}
		
	// Create TyDma from SGL to transfer page from SDRAM to the page pusher.
	status = p_sgl->Build_TyDma(
		FPGA_data_register, Flash_Address::Bytes_Per_Page(),
		FF_SGL::Transfer_Address_Is_Dest,  
 		&SSD_Device::DMA_Complete,
 		p_DMA_context,
 		
 		// The dest does not get incremented
 		DmaFlags | GT_DMA_DDIR_HOLD,
		&p_tydma);
				
	if (status != OK)
		return status;
				
	// Get array, column in a format the FPGA can understand.
	UI64 array_column_register = ARRAY_COLUMN_REGISTER(flash_address);

	// Get unit index -- concatenation of array and column, e.g.,
	// ACCC if Array is one bit, and Column is 3 bits.
	U32 unit_index = flash_address.Unit_Index();
	
	// Save unit_index in callback context for DMA completion.
	p_callback_context->Set_Return_Value((UI64)unit_index);

	// Create interleaved flash address.
	Interleaved_Flash_Address	interleaved_flash_address;

	// Store low byte of page address in interleaved address.
	// The low byte is always zero because this addresses A0 - A7,
	// the hardware column address within the page.
	interleaved_flash_address.address_dword[0] = 0;
	interleaved_flash_address.address_dword[1] = 0;
	interleaved_flash_address.address_dword[2] = 0;
	interleaved_flash_address.address_dword[3] = 0;

	for (U32 device_index = 0; device_index < 32; device_index++)
	{
		// Get page address for next device.
		U32 page_address = flash_page_map[device_index];

		// Get mid byte of page address.
		U32 mid_byte = page_address & 0xFF;

		// Store mid byte of page address in interleaved address.
		interleaved_flash_address.address_byte[device_index + 32] = (U8)mid_byte;

		// Get high byte of page address.
		U32 hi_byte = (page_address >> 8) & 0xFF;

		// Store hi byte of page address in interleaved address.
		interleaved_flash_address.address_byte[device_index + 64] = (U8)hi_byte;
	}

	DISABLE_FPGA;

	// Read the PP-FPGA Flash status register.
	UI64 flash_status = *FPGA_status_register;
	
	// There must not be any other operation in progress.
	CT_ASSERT(((flash_status & OPERATION_IN_PROCESS) == 0), Read_Or_Verify_Page);

	// Start DMA operation to transfer page from SDRAM to page pusher.
    Dma::Transfer(p_tydma);
 		
	// Send array, column to FPGA.
	*FPGA_array_column_register = array_column_register;

	// Send command to receive page address (clears the FIFO).
	UI64 control_value = LOAD_ADDRESS_FIFO_REGISTER | array_installed_flags;
	*FPGA_control_register = control_value;

	// Send interleaved flash address to FPGA Flash Array
	// Block address registers.
	
	// TEMPORARY bandaid.  
	// If we don't disable interrupts here, the addresses can get corrupted!
	U32			m_protect_struct;
	ENTER_CRITICAL_SECTION;
	
	// Make sure FPGA FIFO is ready.
	while (*FPGA_status_register & OPERATION_IN_PROCESS)
		;
	
	*FPGA_block_address_register = interleaved_flash_address.address_dword[0];
	*FPGA_block_address_register = interleaved_flash_address.address_dword[1];
	*FPGA_block_address_register = interleaved_flash_address.address_dword[2];
	*FPGA_block_address_register = interleaved_flash_address.address_dword[3];
	*FPGA_block_address_register = interleaved_flash_address.address_dword[4];
	*FPGA_block_address_register = interleaved_flash_address.address_dword[5];
	*FPGA_block_address_register = interleaved_flash_address.address_dword[6];
	*FPGA_block_address_register = interleaved_flash_address.address_dword[7];
	*FPGA_block_address_register = interleaved_flash_address.address_dword[8];
	*FPGA_block_address_register = interleaved_flash_address.address_dword[9];
	*FPGA_block_address_register = interleaved_flash_address.address_dword[10];
	*FPGA_block_address_register = interleaved_flash_address.address_dword[11];
	
	LEAVE_CRITICAL_SECTION;

	// Set up the FPGA control register.
	// This sets the flash into the sequential read mode and prepares to initiate
	// the transfer once the Flash Devices become ready.
	control_value = INTERLEAVED_VERIFY_NORMAL_DMA
		| READY_BUSY_STATUS_ENABLE
		| READY_BUSY_INTERRUPT_ENABLE
		| array_installed_flags;
	
	*FPGA_control_register = control_value;
		
	// When device completes, call Device_Complete.
	p_device_context->Set_Callback(&Device_Complete);
	
	// Start the watchdog timer to wait for the interrupt.
	// The interrupt routine calls Wake_Up_Waiting_Context to stop the 
	// watchdog timer.
	FF_watchdog.Start_Watchdog(p_device_context, unit_index, 
		FF_READ_TIMER_WATCHDOG_TICKS);
		
	ENABLE_FPGA;

	return OK;

}  // SSD_Device::Verify_Read
	
#else // FPGA_VERIFY_READ_IMPLEMENTED

/*************************************************************************/
// SSD_Device::Verify_Read
// Verify the data by comparing it to the data in the buffer.
// The verify operation is NOT supported by the hardware.
/*************************************************************************/
Status SSD_Device::Verify_Read(
		Callback_Context *p_callback_context, 
		Flash_Address flash_address, 
		Flash_Page_Map flash_page_map,
		FF_SGL *p_sgl)
{ 	
	Status status = OK;
	
	// Get array, column in a format the FPGA can understand.
	UI64 array_column_register = ARRAY_COLUMN_REGISTER(flash_address);

	// Get unit index -- concatenation of array and column, e.g.,
	// ACCC if Array is one bit, and Column is 3 bits.
	U32 unit_index = flash_address.Unit_Index();
	
	// Save unit_index in callback context for DMA completion.
	p_callback_context->Set_Return_Value((UI64)unit_index);

	// Create interleaved flash address.
	Interleaved_Flash_Address	interleaved_flash_address;

	// Store low byte of page address in interleaved address.
	// The low byte is always zero because this addresses A0 - A7,
	// the hardware column address within the page.
	interleaved_flash_address.address_dword[0] = 0;
	interleaved_flash_address.address_dword[1] = 0;
	interleaved_flash_address.address_dword[2] = 0;
	interleaved_flash_address.address_dword[3] = 0;

	for (U32 device_index = 0; device_index < 32; device_index++)
	{
		// Get page address for next device.
		U32 page_address = flash_page_map[device_index];

		// Get mid byte of page address.
		U32 mid_byte = page_address & 0xFF;

		// Store mid byte of page address in interleaved address.
		interleaved_flash_address.address_byte[device_index + 32] = (U8)mid_byte;

		// Get high byte of page address.
		U32 hi_byte = (page_address >> 8) & 0xFF;

		// Store hi byte of page address in interleaved address.
		interleaved_flash_address.address_byte[device_index + 64] = (U8)hi_byte;
	}

	DISABLE_FPGA;

	// Read the PP-FPGA Flash status register.
	UI64 flash_status = *FPGA_status_register;
	
	// There must not be any other operation in progress.
	CT_ASSERT(((flash_status & OPERATION_IN_PROCESS) == 0), Read_Or_Verify_Page);

	// Send array, column to FPGA.
	*FPGA_array_column_register = array_column_register;

	// Send command to receive page address (clears the FIFO).
	UI64 control_value = LOAD_ADDRESS_FIFO_REGISTER | array_installed_flags;
	*FPGA_control_register = control_value;

	// Send interleaved flash address to FPGA Flash Array
	// Block address registers.
	
	// TEMPORARY bandaid.  
	// If we don't disable interrupts here, the addresses can get corrupted!
	U32			m_protect_struct;
	ENTER_CRITICAL_SECTION;
	
	// Make sure FPGA FIFO is ready.
	while (*FPGA_status_register & OPERATION_IN_PROCESS)
		;
	
	*FPGA_block_address_register = interleaved_flash_address.address_dword[0];
	*FPGA_block_address_register = interleaved_flash_address.address_dword[1];
	*FPGA_block_address_register = interleaved_flash_address.address_dword[2];
	*FPGA_block_address_register = interleaved_flash_address.address_dword[3];
	*FPGA_block_address_register = interleaved_flash_address.address_dword[4];
	*FPGA_block_address_register = interleaved_flash_address.address_dword[5];
	*FPGA_block_address_register = interleaved_flash_address.address_dword[6];
	*FPGA_block_address_register = interleaved_flash_address.address_dword[7];
	*FPGA_block_address_register = interleaved_flash_address.address_dword[8];
	*FPGA_block_address_register = interleaved_flash_address.address_dword[9];
	*FPGA_block_address_register = interleaved_flash_address.address_dword[10];
	*FPGA_block_address_register = interleaved_flash_address.address_dword[11];
	
	LEAVE_CRITICAL_SECTION;

	// Start the watchdog timer to wait for the interrupt.
	// The interrupt routine calls Wake_Up_Waiting_Context to stop the 
	// watchdog timer.
	FF_watchdog.Start_Watchdog(p_callback_context, unit_index, 
		FF_READ_TIMER_WATCHDOG_TICKS);
		
	// The verify operation is not supported by the hardware.
	// We must read the data into a temporary buffer.
	// Save pointer to sgl in context in return_value_1.
	// Note that we used return_value to save the unit index.
	p_callback_context->Set_Return_Value((UI64)p_sgl, 1 /* which_value */);
			
	// Start DMA operation to transfer page from page pusher to SDRAM verify buffer.
    Dma::Transfer(FPGA_data_register, // source is page pusher
 		m_p_read_verify_buffer,	// dest is verify buffer
		Flash_Address::Bytes_Per_Page(), // transfer one page
 		&SSD_Device::Verify_DMA_Complete,
 		p_callback_context,
 		
 		// The source does not get incremented.
 		DmaFlags | GT_DMA_SDIR_HOLD );

	// Set up the FPGA control register.
	// This sets the flash into the sequential read mode and prepares to initiate
	// the transfer once the Flash Devices become ready.
	control_value = INTERLEAVED_READ_NORMAL_DMA
		| READY_BUSY_STATUS_ENABLE
		| READY_BUSY_INTERRUPT_ENABLE
		| array_installed_flags;
	
	*FPGA_control_register = control_value;
	
	ENABLE_FPGA;

	return OK;

}  // SSD_Device::Verify_Read

#endif // NOT FPGA_VERIFY_READ_IMPLEMENTED
	
/*************************************************************************/
// SSD_Device::Read_DMA_Complete
// The DMA of data from FPGA to buffer has completed,
// or we got a timeout.
/*************************************************************************/
void SSD_Device::Read_DMA_Complete(void *p_context, Status status)
{
	Callback_Context *p_callback_context = (Callback_Context *)p_context;
	
	// Set status in case we got a timeout.
	p_callback_context->Set_Status(status);

	// Get saved unit index from callback context.
	U32 unit_index = (U32)p_callback_context->Get_Return_Value();
	
	FF_watchdog.Wake_Up_Waiting_Context(unit_index, 0 /* controller_status */ );
	
} // Read_DMA_Complete

/*************************************************************************/
// SSD_Device::Verify_DMA_Complete
// The DMA of data from FPGA to verify buffer has completed
// or we got a timeout.
// This is a temporary method until the hardware verify is working.
/*************************************************************************/
void SSD_Device::Verify_DMA_Complete(void *p_context, Status status)
{
	Callback_Context *p_callback_context = (Callback_Context *)p_context;

	// Get saved unit index from callback context.
	U32 unit_index = (U32)p_callback_context->Get_Return_Value();
	
	// Get pointer to user's sgl from return_value_1 in callback context.
	FF_SGL *p_sgl = (FF_SGL *)p_callback_context->Get_Return_Value(1);
	
	U32 element_index = 0;
	U32 verify_buffer_index = 0;
	
	// Did we get a timeout?
	if (status == OK) 
	{
		// Step through each element in SGL.
		while (true)
		{
			// Get pointer to next element of user's verify buffer from SGL.
			UI64 *p_user_buffer = (UI64 *)p_sgl->Address(element_index);
			
			// Get count of bytes for next element of user's verify buffer from SGL.
			U32 count = p_sgl->Count(element_index);
			
			// Compare each word in the buffer to the user's data.
			U32 dwords = count / 8;
			
			// Verify next element.
			for (U32 user_buffer_index = 0; user_buffer_index < dwords; user_buffer_index++)
			{
				if (*(FF_p_unit->m_p_read_verify_buffer + verify_buffer_index++) != *(p_user_buffer + user_buffer_index))
				{
					// Wake up context first so that, if we have a break set, we won't
					// get a timeout.
					FF_watchdog.Wake_Up_Waiting_Context(unit_index, 0 /* controller_status */ );
					p_callback_context->Set_Status(FF_ERROR(VERIFY));
					return;
				}
			}
			
			// Is there another element?
			if (p_sgl->Is_Last(element_index))
				break;
				
			// Step to next element in SGL.
			element_index++;
			
		} // while
	}
	
	FF_watchdog.Wake_Up_Waiting_Context(unit_index, 0 /* controller_status */ );
		
} // Verify_DMA_Complete

/*************************************************************************/
// SSD_Device::Write_Page
// Write page from flash buffer
/*************************************************************************/
Status SSD_Device::Write_Page(
		Callback_Context *p_callback_context, 
		Flash_Address flash_address, 
		Flash_Page_Map flash_page_map,
		FF_SGL *p_sgl)
{
 	TRACEF( TRACE_L5, 
		(EOL "SSD_Device::Write_Page, array = %d, column = %d, block = %d, sector = %d", 
		flash_address.Array(), 
		flash_address.Column(), flash_address.Block(), flash_address.Sector()));
 	
#if 0
	// TEMPORARY -- change Galileo DMA parameters
	gt_write(FPGA_parameter_register, FPGA_parameters_2);
#endif
	
	// We must wait for a DMA transfer and a device completion.
	// Allocate child of caller's context to do the DMA transfer.
	Callback_Context *p_DMA_context = 
		(Callback_Context *)p_callback_context->Allocate_Child(sizeof(Callback_Context));
	if (p_DMA_context == 0)
	{
		// Return to calling context.  No child context was created.
		return FF_ERROR(NO_CONTEXT);
	}
		
	// Allocate child of caller's context to wait for device completion.
	Callback_Context *p_device_context = 
		(Callback_Context *)p_callback_context->Allocate_Child(sizeof(Callback_Context));
	if (p_device_context == 0)
	{
		// Return to calling context.  No child context was created.
		return FF_ERROR(NO_CONTEXT);
	}
		
	// Create TyDma from SGL.
	TyDma *p_tydma;
	Status status = p_sgl->Build_TyDma(
		FPGA_data_register, Flash_Address::Bytes_Per_Page(),
		FF_SGL::Transfer_Address_Is_Dest,  
 		&SSD_Device::DMA_Complete,
 		p_DMA_context,
 		
 		// The dest does not get incremented
 		DmaFlags | GT_DMA_DDIR_HOLD,
		&p_tydma);
		
	if (status != OK)
		return status;
		
	// Create unit index from bank and column.
	// Get unit index -- concatenation of array and column, e.g.,
	// ACCC if Array is one bit, and Column is 3 bits.
	U32 unit_index = flash_address.Unit_Index();
	
	// Get array, column in a format the FPGA can understand.
	UI64 array_column_register = ARRAY_COLUMN_REGISTER(flash_address);

	// Create interleaved flash address.
	Interleaved_Flash_Address	interleaved_flash_address;

	// Store low byte of page address in interleaved address.
	// The low byte is always zero because this addresses A0 - A7,
	// the hardware column address within the page.
	interleaved_flash_address.address_dword[0] = 0;
	interleaved_flash_address.address_dword[1] = 0;
	interleaved_flash_address.address_dword[2] = 0;
	interleaved_flash_address.address_dword[3] = 0;

	for (U32 device_index = 0; device_index < 32; device_index++)
	{
		// Get page address for next device.
		U32 page_address = flash_page_map[device_index];

		// Get mid byte of page address.
		U32 mid_byte = page_address & 0xFF;

		// Store mid byte of page address in interleaved address.
		interleaved_flash_address.address_byte[device_index + 32] = (U8)mid_byte;

		// Get high byte of page address.
		U32 hi_byte = (page_address >> 8) & 0xFF;

		// Store hi byte of page address in interleaved address.
		interleaved_flash_address.address_byte[device_index + 64] = (U8)hi_byte;
	}

	DISABLE_FPGA;

	// Read the PP-FPGA Flash status register.
	UI64 flash_status = *FPGA_status_register;
	
	// There must not be any other operation in progress.
	CT_ASSERT(((flash_status & OPERATION_IN_PROCESS) == 0), Write_Page);

	// Start DMA operation to transfer page from SDRAM to page pusher.
    Dma::Transfer(p_tydma);
 		
	// Send array, column to FPGA.
	*FPGA_array_column_register = array_column_register;

	// Send command to receive page address (clears the FIFO).
	UI64 control_value = LOAD_ADDRESS_FIFO_REGISTER | array_installed_flags;
	*FPGA_control_register = control_value;

	// Send interleaved flash address to FPGA Flash Array
	// Block address registers.
	
	// TEMPORARY bandaid.  
	// If we don't disable interrupts here, the addresses can get corrupted!
	U32			m_protect_struct;
	ENTER_CRITICAL_SECTION;
	
	// Make sure FPGA FIFO is ready.
	while (*FPGA_status_register & OPERATION_IN_PROCESS)
		;
	
	*FPGA_block_address_register = interleaved_flash_address.address_dword[0];
	*FPGA_block_address_register = interleaved_flash_address.address_dword[1];
	*FPGA_block_address_register = interleaved_flash_address.address_dword[2];
	*FPGA_block_address_register = interleaved_flash_address.address_dword[3];
	*FPGA_block_address_register = interleaved_flash_address.address_dword[4];
	*FPGA_block_address_register = interleaved_flash_address.address_dword[5];
	*FPGA_block_address_register = interleaved_flash_address.address_dword[6];
	*FPGA_block_address_register = interleaved_flash_address.address_dword[7];
	*FPGA_block_address_register = interleaved_flash_address.address_dword[8];
	*FPGA_block_address_register = interleaved_flash_address.address_dword[9];
	*FPGA_block_address_register = interleaved_flash_address.address_dword[10];
	*FPGA_block_address_register = interleaved_flash_address.address_dword[11];
	
	LEAVE_CRITICAL_SECTION;
	
	// Set up the FPGA control register.
	// This signals the FPGA that it may now inititate a transfer.
	control_value = INTERLEAVED_WRITE_NORMAL_DMA
		| READY_BUSY_STATUS_ENABLE
		| READY_BUSY_INTERRUPT_ENABLE
		| array_installed_flags;
	
	*FPGA_control_register = control_value;
	
	// When device completes, call Device_Complete.
	p_device_context->Set_Callback(&Device_Complete);
	
	// Start the watchdog timer to wait for the interrupt.
	// The interrupt routine calls Wake_Up_Waiting_Context to stop the 
	// watchdog timer.
	FF_watchdog.Start_Watchdog(p_device_context, unit_index, FF_WRITE_TIMER_WATCHDOG_TICKS);
	
	ENABLE_FPGA;

#if 0
	// TEMPORARY -- restore Galileo DMA parameters
	// Set up the FPGA parameter register --  
	// the Galileo unit bank 1 register
	gt_write(FPGA_parameter_register, FPGA_parameters);
#endif
	
	return OK;

}  // SSD_Device::Write_Page

/*************************************************************************/
// FF_Get_Interrupt_Status
// Called by Low Level Interrupt Service Routine
// Get status of page pusher.  
// A bit for each unit indicates whether the unit caused an interrupt
// by going from busy to ready.
/*************************************************************************/
void FF_Get_Interrupt_Status(FF_INTERRUPT_STATUS *p_interrupt_status)
{
	// We don't need to disable FPGA interrupts here because we are
	// in the low level interrupt service routine.
	// DISABLE_FPGA;

	// Read the PP-FPGA Flash Interrupt Source ID register.
	// Array 1, interleaved columns [7:0] interrupt pending is reg bits [15:08]
	// Array 0, interleaved columns [7:0] interrupt pending is reg bits [07:00]
	(*p_interrupt_status).source = *FPGA_ready_busy_status & 0xFFFF;

	// Read the PP-FPGA Flash status register.
	(*p_interrupt_status).controller_status = *FPGA_status_register;

#if 0
	// Trace in interrupt handler seems to cause problems --
	// partial lines of trace appear, some lines are intermixed.
 	TRACEF( TRACE_L5, 
		(EOL " FF_Get_Interrupt_Status, source = %X", (*p_interrupt_status).source));
#endif

	// We don't need to disable/enable FPGA interrupts here because we are
	// in the low level interrupt service routine.
	// ENABLE_FPGA;


} // FF_Get_Interrupt_Status


