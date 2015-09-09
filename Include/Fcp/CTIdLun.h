/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
//  CTIdLun.h
//
// Description:
// This file contains the definition of the structure used to access the
// LUN and Target Id stored in SCSI_SCB_EXEC messages.
// Ids and LUNs are either passed out from the target HDM or passed in
// to the Initiator HDM
// 
// Update Log: 
// 10/2/98 Michael G. Panas: Create file
// 02/21/99 Michael G. Panas: remove I2O stuff
// 04/23/99 Michael G. Panas: need seperate target ID for ISP2200,
//                            make fields 8 bit
/*************************************************************************/

#ifndef __CTIdLun_h
#define __CTIdLun_h

typedef struct _Id_Fields {
	U8			id;				// target/intiator Id
	U8			HostId;			// Host Id
	U16			LUN;			// Host LUN or Initiator LUN
} IDLUN, *PIDLUN;

#endif