/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: prefix_ev64120_s.h
//
// Description:
// This is a Metrowerks CodeWarrior Prefix File for assembly language
// code modules.  A prefix file is a file that the assembler processes
// before every assembly file in the project. It is as though you put
// the same .include directive at the beginning of every assembly file.
//
//
// Update Log:
// 10/12/98 JSN: Created.
/*************************************************************************/

#ifndef PREFIX_ODYSSEY_H
#define PREFIX_ODYSSEY_H 	1


/*
 * Define one of the following for target system dependancies.
 */
#define	INCLUDE_ODYSSEY			// Flag for running on ODYSSEY systems.
#undef	INCLUDE_EV64120			// Flag for running on EV64120 systems.
#define TRK_TRANSPORT_INT_DRIVEN	
#define CONFIG_INT_PRIORITY		// Flag for prioritized interrupts.
#define CONFIG_E2				// For E2 Odyssey boards
#undef CONFIG_E1				// For E1 Odyssey boards

#endif /* PREFIX_ODYSSEY_H */
