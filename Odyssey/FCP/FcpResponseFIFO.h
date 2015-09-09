/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FcpResponseFIFO.h
// 
// Description:
// This file describes interfaces to the ISP response FIFO. 
// 
// Update Log 
// 5/18/98 Jim Frandeen: Create file
// 6/2/98 Jim Frandeen: Change order to legaleese, description, update log
// 9/2/98 Michael G. Panas: add C++ stuff
// 11/30/98 Michael G. Panas: New memory allocation methods
/*************************************************************************/

#if !defined(FcpResponseFIFO_H)
#define FcpResponseFIFO_H

#include "Nucleus.h"

#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif

STATUS	FCP_Response_FIFO_Create(PINSTANCE_DATA Id);
void	FCP_Response_FIFO_Destroy();

void	FCP_Response_FIFO_Increment_Index();

#ifdef          __cplusplus
}                                           /* End of C declarations     */
#endif


#endif /* FcpResponseFIFO_H  */
