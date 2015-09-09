/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FtTestDevice.h
// 
// Description:
// This file describes the interfaces to the test device methods that
// are common to FtTestDeviceFile.cpp and FtTestDeviceMem.cpp. 
// 
// Files common to all flash file system test drivers
// 
// 1/13/98 Jim Frandeen: Create file
/*************************************************************************/

#if !defined(FtTestDevice_H)
#define FtTestDevice_H

#include "FlashDevice.h"
#include "Nucleus.h"

// If MEMORY_FILE, then use memory for a file.
#define MEMORY_FILE

class FF_SGL;

class FF_Sim_Device : public Flash_Device
{
public: // inherited methods

	// Initialize_Capacity is called first so that the addressing
	// methods described below may be used.
	Status Get_Capacity(Flash_Capacity *p_address_capacity);

	// Initialize the device
	Status Open(FF_Mem *p_mem);

	// Read flash page into buffer.
	// Call callback when transfer has completed.
	Status Read_Page(
		Callback_Context *p_callback_context, 
		Flash_Address flash_address,
		Flash_Page_Map flash_page_map,
		FF_SGL *p_sgl);

	// Verify page.
	// Call callback when transfer has completed.
	Status Verify_Page(
		Callback_Context *p_callback_context, 
		Flash_Address flash_address,
		Flash_Page_Map flash_page_map,
		FF_SGL *p_sgl);

	// Write flash page from buffer.
	// Call callback when DMA transfer has completed.
	Status Write_Page(
		Callback_Context *p_callback_context, 
		Flash_Address flash_address,
		Flash_Page_Map flash_page_map,
		FF_SGL *p_sgl);

	// Wait for the device to finish after the DMA transfer has completed.
	// Call callback when device has completed.
	virtual Status Wait_Write_Complete(
		Callback_Context *p_callback_context,
		Flash_Address flash_address);

	// Erase block.
	// Call callback when erase has completed.
	Status Erase_Page_Block(
		Callback_Context *p_callback_context, 
		Flash_Address flash_address,
		Flash_Page_Map flash_page_map);

	// Get device status
	Status Get_Device_Status(Flash_Address flash_address, 
		UI64 controller_status, Flash_Device_Status *p_device_status);

	U32 Memory_Size(FF_CONFIG *p_config);

	// End of inherited methods
	
	static void FF_Device_Read_Complete(void *p_context, STATUS status);
	static void FF_Device_Erase_Page_Complete(void *p_context, STATUS status);
	static void FF_Device_Write_Page_Complete(void *p_context, STATUS status);
	static void FF_Device_Write_Page_Complete_Success(void *p_context, STATUS status);
	static void FF_Device_Write_Page_Complete_Error(void *p_context, STATUS status);

private: // helper methods
	
	static void Interrupt_Callback(U32 device, UI64 controller_status);
	
}; // FF_Test_Device

void FF_ISR_High();
void Device_Timer_Expiration_Routine(U32 id);
int Random_Number(void);
void Seed_Random();
 
typedef enum {
	READ_CELL,
	WRITE_CELL,
	ERASE_CELL,
	VERIFY_CELL
} Device_IO_Type;

STATUS Do_Device_IO(
	U32 device,
	UNSIGNED offset, // offset in file 
	void *p_buffer, // pointer to data 
	UNSIGNED num_bytes, // number of bytes 
	Callback_Context *p_callback_context, // context to run when I/O is complete
	Device_IO_Type device_IO_type
	); 

STATUS Create_Bad_Sector(
	U32 device,
	UNSIGNED offset, // offset in file 
	UNSIGNED num_bytes // number of bytes 
	);

/*************************************************************************/
// Device globals
/*************************************************************************/
extern Flash_Device_Status device_status[FF_NUM_UNITS_MAX];
#ifdef THREADX
extern TX_TIMER timer[FF_NUM_UNITS_MAX];
#else
extern NU_TIMER timer[FF_NUM_UNITS_MAX];
#endif

extern Callback_Context *timer_context[FF_NUM_UNITS_MAX];
extern void *p_erase_buffer;
extern FF_INTERRUPT_STATUS interrupt_status;
extern FF_INTERRUPT_STATUS busy_status;
extern U32					  FF_bit_mask[32];
extern char *p_mem_file;
extern char *p_mem_file_last;

// These globals are set for each test.
extern U32 test_percentage_format_errors;
extern U32 test_percentage_erase_errors;
extern U32 test_percentage_write_errors;

// These globals are set by the error simulation dialog.
extern U32 percentage_format_errors;
extern U32 percentage_erase_errors;
extern U32 percentage_write_errors;

	
#endif // FtTestDevice_H

	
