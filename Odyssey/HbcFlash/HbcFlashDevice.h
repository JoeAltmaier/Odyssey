/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: HbcFlashDevice.h
// 
// Description:
// This defines the object inherited from the base class Flash_Device.
// The HBC_Flash_Device is implemented for the flash driver
// on the HBC.
// 
// 7/29/98 Jim Frandeen: Create file
/*************************************************************************/

#if !defined(HbcFlashDevice_H)
#define HbcFlashDevice_H

#include "FlashDevice.h"

class HBC_Flash_Device : public Flash_Device
{
public: // inherited methods

	// Constructor
	HBC_Flash_Device();
	
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

	// Verify flash page with buffer.
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
		UI64 controller_status,
		Flash_Device_Status *p_device_status);

	// Abort command
	virtual Status Abort_Command(Flash_Address flash_address);

private: // helper methods
	
	static void Interrupt_Callback(U32 device);
	
private: // callback methods

	static void Read_DMA_Complete				(void *p_context, Status status);
	static void Write_DMA_Complete				(void *p_context, Status status);
	
private:  // member data


}; // HBC_Flash_Device

inline HBC_Flash_Device::HBC_Flash_Device()
{
}

inline Status HBC_Flash_Device::Abort_Command(Flash_Address)
{
	return OK;
}

#endif // HbcFlashDevice_H

	
