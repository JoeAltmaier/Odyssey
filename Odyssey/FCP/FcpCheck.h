/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FcpCheck.h
// 
// Description:
// This file defines interfaces to keep track of check information
// for each LUN and each initiator. 
// 
// Update Log 
// 5/19/98 Jim Frandeen: Create file
// 6/2/98 Jim Frandeen: Change order to legaleese, description, update log
// 8/25/98 Michael G. Panas: Update for 16 bit LUNs
// 9/2/98 Michael G. Panas: add C++ stuff
// 11/30/98 Michael G. Panas: New memory allocation methods
/*************************************************************************/

#if !defined(FcpCheck_H)
#define FcpCheck_H

#include "FcpIOCB.h"
#include "Nucleus.h"
#include "FcpConfig.h"
#include "FcpISP.h"
#include "Scsi.h"

#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif


/*************************************************************************/
// Check conditions
/*************************************************************************/
typedef enum
{
	FCP_CHECK_NULL = 0,
	FCP_CHECK_NO_BUFFER,
	FCP_CHECK_UNRECOGNIZED_COMMAND,
	FCP_CHECK_DRIVER_QUIESCENT,
	FCP_CHECK_I2O,
	FCP_CHECK_TIMEOUT, 
	FCP_CHECK_BAD_LUN
} FCP_CHECK_CONDITION;

STATUS	FCP_Check_Create(PINSTANCE_DATA Id);
void	FCP_Check_Destroy();

void FCP_Check_Save(U16 LUN, U8 initiator, 
	FCP_CHECK_CONDITION check_condition);

U8 FCP_Check_Get(U16 LUN, U8 initiator);
	
void FCP_Check_Create_Sense(U16 LUN, U8 initiator, 
	REQUEST_SENSE *p_sense_data);

#ifdef          __cplusplus
}                                           /* End of C declarations     */
#endif


#endif /* FcpCheck_H  */
