/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FC_Loop.h
// 
// Description:
// This file defines interfaces to the loop instance numbers.
// The instance number, chip number and board number can be derived
// using this basic formula:
//		instance = (MAX_FC_CHIPS * slot) * chip_number
//
// This is a global file that anyone can use.
// 
// Update Log:
//	$Log: /Gemini/Include/Fcp/FC_Loop.h $
// 
// 1     7/15/99 4:17p Mpanas
// New Loop Instance Macros
//
// 07/03/99 Michael G. Panas: Create file
/*************************************************************************/

#if !defined(FC_Loop_H)
#define FC_Loop_H

#define	MAX_FC_CHIPS		3		// max number of chips on each board

// calculate the instance number given the slot and chip numbers
#define	FCLOOPINSTANCE(slot, chip) ((MAX_FC_CHIPS * slot) + chip)

// calculate the chip number given the instance number
#define	FCLOOPCHIP(instance) (instance % MAX_FC_CHIPS)

// calculate the slot number given the instance number
#define	FCLOOPSLOT(instance) (instance / MAX_FC_CHIPS)

#endif /* FC_Loop_H  */
