/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
// File: FlashDevice.h
// 
// Description:
// This file defines the interface to the Flash Storage 
// Device methods. 
// 
// $Log: /Gemini/Include/FlashDevice.h $
// 
// 13    2/05/00 2:18p Jfrandeen
// 
// 12    1/12/00 10:07a Jfrandeen
// Add SGL interface.
// 
// 11    12/01/99 6:19p Hdo
// Remove unused parameter names.
// 
// 10    11/22/99 10:34a Jfrandeen
// Implement interleaved flash
// 
// 8     8/12/99 3:42p Jfrandeen
// Add 256 MBit device
// 
// 7     6/24/99 2:32p Jfrandeen
// 128 mbit part
// 
// 6     6/15/99 8:49p Jfrandeen
// Support Samsung 128 MB
// 
// 5     5/07/99 8:54p Jfrandeen
// 
// 4     5/06/99 5:40p Jfrandeen
// 
// 3     5/06/99 3:41p Jfrandeen
// Increment_SSD_Address returns value incremented
// 
// 2     5/05/99 2:19p Jfrandeen
// 
// 1     4/29/99 11:18a Jfrandeen
// 
// 1     4/01/99 7:32p Jfrandeen
// 
// 9/5/98 Jim Frandeen: Create file
/*************************************************************************/
#if !defined(FlashDevice_H)
#define FlashDevice_H

#include "FlashStorage.h"
#include "FlashAddress.h"
#include "PciDev.h"

class FF_SGL;

/*************************************************************************/
// Flash_Device_Status has one doubleword of status for each bank.
/*************************************************************************/
typedef struct {
	union
	{
		UI64	bank_status[4];
		U8		device_status[32];
	};
	U32			device_is_ready;
} Flash_Device_Status;

#define DWORDS_PER_DEVICE_PAGE 64

// Maximum number of units that may be addressed by the software --
// 8 Columns * 2 Arrays
#define FF_NUM_UNITS_MAX 16

// FF_SECTORS_PER_BLOCK_MAX is hardware dependent.
// For the Samsung 128T (16 megabyte unit), the max is 32.
// For the Toshiba 64FT (8 megabyte unit), the max is 16.
// FF_DELETED_MAP_ENTRY has a bit map with one bit per sector.
// This must change whenever the number of sectors per block changes.
#define FF_SECTORS_PER_BLOCK_MAX 32

#define FF_DEVICES_PER_PAGE_MAX 32

/*************************************************************************/
// Flash_Capacity
// Flash capacity is returned from Flash_Device::Initialize method.
// This is used to determine the various capacities of the unit:
// bytes per page, number of units, etc.
// With interleaving,
// The page offset is used to map the bank number and device number as follows:
//
// 1 1 1 1 0 0 0 0 0 0 0 0 0 0
// 3 2 1 0 9 8 7 6 5 4 3 2 1 0
// O O O O O O O O O b b D D D

// D	Device number 
// b	Bank number 
//
// Bits allocated for the flash device are within the
// bits allocated for the offset when interleaving is available for the device.
// The number of address bits for bank is included in the number of bits
// for offset.  This is only used when interleaving is available for the device.
/*************************************************************************/
class Flash_Capacity
{
public:

	U32 num_address_bits_for_block;
	U32 num_address_bits_for_sector;
	U32 num_address_bits_for_array;
	U32 num_address_bits_for_bank;
	U32 num_address_bits_for_column;
	U32 num_address_bits_for_device;
	U32 num_address_bits_for_offset;
}; // Flash_Capacity;

// Define unit IDs for flash units
typedef unsigned long  FF_DEVICE_ID;

// Currently, there are only two known unit IDs.
// Maker code in low byte, Unit code in high byte.
#define FF_DEVICE_ID_TOSHIBA_256 0x7598
#define FF_DEVICE_ID_SAMSUNG_128 0x73EC

// Flags used by Verify_Page
#define FF_VERIFY_READ					0x008
#define FF_VERIFY_ERASE					0x010

class FF_Mem;

/*************************************************************************/
// Flash_Page_Map contains an array of page addresses, one for each
// device page.
/*************************************************************************/
typedef U32 Flash_Page_Map[FF_DEVICES_PER_PAGE_MAX];

/*************************************************************************/
// Flash_Device class describes the flash device object.
/*************************************************************************/
class Flash_Device
{
public: // virtual methods

	// These are the methods that must be implemented by the derived class.

	// Open the device.
	// Use FF_Mem object to allocate memory as needed.
	virtual Status Open(FF_Mem *p_mem);

	// Close the device
	virtual Status Close();

	// Read flash page into buffer.
	// Call callback when transfer has completed.
	virtual Status Read_Page(
		Callback_Context *p_callback_context, 
		Flash_Address flash_address, 
		Flash_Page_Map flash_page_map,
		FF_SGL *p_sgl);

	// Verify page against data in buffer.
	// Call callback when transfer has completed.
	virtual Status Verify_Page(
		Callback_Context *p_callback_context, 
		Flash_Address flash_address, 
		Flash_Page_Map flash_page_map,
		FF_SGL *p_sgl,
		U32 flags);

	// Write flash page from buffer.
	// Call callback when DMA transfer has completed.
	virtual Status Write_Page(
		Callback_Context *p_callback_context, 
		Flash_Address flash_address, 
		Flash_Page_Map flash_page_map,
		FF_SGL *p_sgl);

	// Erase block.
	// Call callback when erase has completed.
	virtual Status Erase_Page_Block(
		Callback_Context *p_callback_context, 
		Flash_Address flash_address, 
		Flash_Page_Map flash_page_map);

	// Get_Capacity is called first so that the addressing
	// methods described below may be used.
	virtual Status Get_Capacity(Flash_Capacity *p_flash_capactiy);

	// Get device status
	virtual Status Get_Device_Status(Flash_Address flash_address, 
		UI64 controller_status, Flash_Device_Status *p_device_status);

	// Abort command
	virtual Status Abort_Command(Flash_Address flash_address);

	// Return memory size required by object.
	virtual U32 Memory_Size(FF_CONFIG *p_config) ;

}; // Flash_Device

// Base class methods are implemented here

	inline Status Flash_Device::Get_Capacity(Flash_Capacity *)
		{return OK;}

	inline Status Flash_Device::Open(
		FF_Mem *) {return OK;}

	inline Status Flash_Device::Close() {return OK;}

	inline Status Flash_Device::Read_Page(
			Callback_Context*, 
			Flash_Address, 
			Flash_Page_Map,
			FF_SGL *) {return OK;}

	inline Status Flash_Device::Verify_Page(
			Callback_Context *, 
			Flash_Address, 
			Flash_Page_Map,
			FF_SGL *,
			U32) {return OK;}

	inline Status Flash_Device::Write_Page(
			Callback_Context *, 
			Flash_Address, 
			Flash_Page_Map,
			FF_SGL *) {return OK;}

	inline Status Flash_Device::Get_Device_Status(Flash_Address, 
		UI64, Flash_Device_Status *) {return OK;}

	inline Status Flash_Device::Erase_Page_Block(
		Callback_Context *, 
		Flash_Address, 
		Flash_Page_Map) {return OK;}

	inline Status Flash_Device::Abort_Command(Flash_Address) {return OK;}

	inline U32 Flash_Device::Memory_Size(FF_CONFIG *) 
	{ return 0; }

	// End of base method implementation


typedef struct _FF_INTERRUPT_STATUS
{
	// Interrupt source has one bit for each unit in each flash array.
	// A "1" bit indicates that an interrupt is pending.
	// Array 1, interleaved columns [7:0] interrupt pending is reg bits [15:08]
	// Array 0, interleaved columns [7:0] interrupt pending is reg bits [07:00]
	UI64 source;
	
	// controller_status depends on the operation, e.g., ECC error on read.
	// No controller_status is pertinent for erase completion.
	UI64 controller_status;
} FF_INTERRUPT_STATUS;


/*************************************************************************/
// FF_Mem class describes a memory object.
// This permits the flash storage system to control its memory use.
/*************************************************************************/
class FF_Mem
{
public:
	// Initialize object
	void Initialize(U32 memory_size, void *p_memory);
	
	// Allocate memory
	void *Allocate(U32 size, U32 alignment = 8);
	
	// How much memory is left?
	U32 Memory_Available();

	// How much memory was allocated?
	U32 Memory_Size();

private:
	void 		*m_p_memory_initial;
	void 		*m_p_memory;
	U32 		 m_memory_size;
	U32 		 m_memory_available;
	
};

inline void FF_Mem::Initialize(U32 memory_size, void *p_memory)
{
	m_p_memory_initial = p_memory;

	// Align memory on dword boundary.
	m_p_memory = (void*)ALIGN(p_memory, 8);
		 
	// Calculate pointer to next available memory location
	char *p_next_memory = (char*)p_memory + memory_size;
	
	// Align memory on dword boundary.
	p_next_memory = (char *)ALIGN(p_next_memory, 8);

	m_memory_size = memory_size;

	// Update amount of memory left
	m_memory_available = ((char*)p_next_memory - (char*)m_p_memory);
}

inline U32 FF_Mem::Memory_Available()
{
	return m_memory_available;
}

inline U32 FF_Mem::Memory_Size()
{
	return m_memory_size;
}


#endif // FlashDevice_H

