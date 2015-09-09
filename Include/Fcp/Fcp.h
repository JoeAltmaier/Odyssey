/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: Fcp.h
// 
// Description:
// This file defines the external interfaces to the FCP driver. 
// 
// Update Log: 
// 5/19/98 Jim Frandeen: Create file
// 6/2/98 Jim Frandeen: Change order to legaleese, description, update log
// 9/2/98 Michael G. Panas: add C++ stuff
// 11/30/98 Michael G. Panas: New memory allocation methods
// 01/24/99 Michael G. Panas: Use new TraceLevel[] array, move FCP_STATE here
/*************************************************************************/

#if !defined(Fcp_H)
#define Fcp_H

#include "Nucleus.h"
#include "FCPData.h"

#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif

typedef enum {
	FCP_STATE_RESET,
	FCP_STATE_INITIALIZED,
	FCP_STATE_ACTIVE,
	FCP_STATE_QUIET
} FCP_STATE;


STATUS	FCP_Start(PINSTANCE_DATA Id);
STATUS	FCP_Quiet(PINSTANCE_DATA Id);
STATUS	FCP_Restart(PINSTANCE_DATA Id);
STATUS	FCP_Reset(PINSTANCE_DATA Id);

#ifdef          __cplusplus
}                                           /* End of C declarations     */
#endif

#endif /* Fcp_H  */
