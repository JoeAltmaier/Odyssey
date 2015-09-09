/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: HscsiCheck.h
// 
// Description:
// This file defines interfaces to keep track of check information
// for each LUN and each initiator. 
// 
// Update Log
//	$Log: /Gemini/Odyssey/Hscsi/HscsiCheck.h $ 
// 
// 1     9/14/99 7:23p Cchan
// Files for the HSCSI library, needed to support HBC-embedded QLogic
// ISP1040B SCSI chip.
//
/*************************************************************************/

#if !defined(HscsiCheck_H)
#define HscsiCheck_H

#include "HscsiIOCB.h"
#include "Nucleus.h"
#include "HscsiConfig.h"
#include "HscsiISP.h"
#include "Scsi.h"

#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif


/*************************************************************************/
// Check conditions
/*************************************************************************/
typedef enum
{
	HSCSI_CHECK_NULL = 0,
	HSCSI_CHECK_NO_BUFFER,
	HSCSI_CHECK_UNRECOGNIZED_COMMAND,
	HSCSI_CHECK_DRIVER_QUIESCENT,
	HSCSI_CHECK_I2O,
	HSCSI_CHECK_TIMEOUT, 
	HSCSI_CHECK_BAD_LUN
} HSCSI_CHECK_CONDITION;

STATUS	HSCSI_Check_Create(PHSCSI_INSTANCE_DATA Id);
void	HSCSI_Check_Destroy();

void HSCSI_Check_Save(U16 LUN, U8 initiator, 
	HSCSI_CHECK_CONDITION check_condition);

U8 HSCSI_Check_Get(U16 LUN, U8 initiator);
	
void HSCSI_Check_Create_Sense(U16 LUN, U8 initiator, 
	REQUEST_SENSE *p_sense_data);

#ifdef          __cplusplus
}                                           /* End of C declarations     */
#endif


#endif /* HscsiCheck_H  */
