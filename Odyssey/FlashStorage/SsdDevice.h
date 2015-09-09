/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: SsdDevice.h
// 
// Description:
// This defines the object inherited from the base class Flash_Device.
// The SSD_Device is implemented for the Solid State Drive.
// 
// 1/13/98 Jim Frandeen: Create file
/*************************************************************************/

#if !defined(SsdDevice_H)
#define SsdDevice_H

#include "FlashDevice.h"

class SSD_Device : public Flash_Device
{
public: // inherited methods

	// Constructor
	SSD_Device();
	
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

	// Get unit status
	virtual Status Get_Device_Status(Flash_Address flash_address, 
		UI64 controller_status, Flash_Device_Status *p_device_status);

	// Abort command
	virtual Status Abort_Command(Flash_Address flash_address);

private: // helper methods
	
	// Verify page against data in buffer.
	// Call callback when transfer has completed.
	Status Verify_Read(
		Callback_Context *p_callback_context, 
		Flash_Address flash_address, 
		Flash_Page_Map flash_page_map,
		FF_SGL *p_sgl);

	Status Get_Device_ID(U32 array, FF_DEVICE_ID *p_device_ID);
	Status Get_Device_ID_Column(U32 array, U32 column, FF_DEVICE_ID *p_device_ID);
	static void Interrupt_Callback(U32 device, UI64 controller_status);
	
private: // callback methods

	static void Read_DMA_Complete				(void *p_context, Status status);
	static void Verify_DMA_Complete				(void *p_context, Status status);
	static void DMA_Complete					(void *p_context, Status status);
	static void Device_Complete					(void *p_context, Status status);
	
private:  // member data

	// Verify buffer is used when hardware does not support verify operation.
	UI64	*m_p_read_verify_buffer;
	
	U32		 m_is_open;


}; // SSD_Device

inline SSD_Device::SSD_Device()
{
	m_is_open = 0;
}

#endif // SsdDevice_H

	
