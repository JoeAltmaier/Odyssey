/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: PCI.c
// 
// Description:
// This file includes some basic PCI access methods
// NOTE: The data is NOT Endian Swapped
// 
// Update Log 
// 8/24/98 Michael G. Panas: Create file
/*************************************************************************/
#include "OsTypes.h"
#include "Pci.h"


/*************************************************************************/
// GetPciReg
// Read the PCI register specified
/*************************************************************************/
U32  GetPciReg(U32 Bus, U32 Slot, U32 Func, U32 Reg)
{
	U32		pci;
	
	pci = ((Bus & 0xff) << 16) | ((Slot & 0x1f) << 11) |
					((Func & 0x7) << 8) | (Reg & 0xff) | 0x80000000;

   *(unsigned long*)(GT_PCI_CONFIG_ADDR_REG) = BYTE_SWAP32(pci);
   return (U32)*(unsigned long*)(GT_PCI_CONFIG_DATA_REG);
}

/*************************************************************************/
// SetPciReg
// Set the PCI register specified to the Data passed
/*************************************************************************/
void SetPciReg(U32 Bus, U32 Slot, U32 Func, U32 Reg, U32 Data)
{
	U32		pci;
	
	pci = ((Bus & 0xff) << 16) | ((Slot & 0x1f) << 11) |
					((Func & 0x7) << 8) | (Reg & 0xff) | 0x80000000;
	
   *(unsigned long*)(GT_PCI_CONFIG_ADDR_REG) = BYTE_SWAP32(pci);
   *(unsigned long*)(GT_PCI_CONFIG_DATA_REG) = Data;
}


