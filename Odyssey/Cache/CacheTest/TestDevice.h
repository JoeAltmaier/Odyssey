/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
// File: TestDevice.h
// 
// Description:
// This file defines the interface to the Flash Block 
// Device methods. 
// 
// Update Log 
// 
// 9/5/98 Jim Frandeen: Create file
/*************************************************************************/
#if !defined(FbDevice_H)
#define FbDevice_H

#include "Nucleus.h"
#include "Simple.h"

// Flash controller status registers allow the flash controllers to 
// specify the status of an operation. Since an operation accesses 8
// cells, the status register must specify the status for each cell. 
// The status register is a 32-bit register with 4 bits of status for
// each cell.
typedef U32  FB_CONTROLLER_STATUS;

// For a read, we also have error correction code
typedef struct {
	FB_CONTROLLER_STATUS	status;
	U32						error_correction_code;
}FB_READ_STATUS;

 
// Send get status from Write operation
STATUS FB_Device_Send_Get_Write_Status(UNSIGNED controller_number,
	UNSIGNED real_page_number);

// Get status from Write operation when ISR gets interrupt
STATUS FB_Device_Get_Write_Status_ISR(UNSIGNED controller_number,
	FB_CONTROLLER_STATUS *p_controller_status);

// Send get status for erase operation
STATUS FB_Device_Send_Get_Erase_Status(UNSIGNED controller_number,
	UNSIGNED real_page_number);

// Get status for read operation when the ISR gets the interrupt.
STATUS FB_Device_Get_Read_Status_ISR(UNSIGNED controller_number,
	FB_READ_STATUS *p_read_status);

// Read page into flash buffer
STATUS FB_Device_Read_Page(UNSIGNED controller_number,
	UNSIGNED real_page_number,
	U16 m_alternate_address,
	U16 m_bad_cell_map,
	void *p_flash_buffer,
    Callback_Context *p_callback_context,
    UNSIGNED page_size);

// Write page from flash buffer
STATUS FB_Device_Write_Page(UNSIGNED controller_number, 
	UNSIGNED real_page_number,
	U16 alternate_address,
	U16 bad_cell_map,
	void *p_flash_buffer,
    Callback_Context *p_callback_context,
    UNSIGNED page_size);

// Erase page block of 16 pages
STATUS FB_Device_Erase_Page_Block(UNSIGNED controller_number, 
	UNSIGNED real_page_number);


#endif // FbDevice_H

