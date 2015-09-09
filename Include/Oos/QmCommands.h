/*************************************************************************
*
* This material is a confidential trade secret and proprietary 
* information of ConvergeNet Technologies, Inc. which may not be 
* reproduced, used, sold or transferred to any third party without the 
* prior written consent of ConvergeNet Technologies, Inc.  This material 
* is also copyrighted as an unpublished work under sections 104 and 408 
* of Title 17 of the United States Code.  Law prohibits unauthorized 
* use, copying or reproduction.
*
* Description:
*	This file contains the command definitions for the QuiesceMaster
* 
* Update Log: 
* 06/27/99 Bob Butler: Create file
*
*************************************************************************/

#ifndef QmCommands_h
#define QmCommands_h

#include "CtTypes.h"

#pragma	pack(4)

#define QMCmdQueueTable "QMCmdQueueTable"

#define QMCmdStart		0xFFFF9000

/********************************************************************
*
* Commands sent by Failover, Hotswap and SSAPI to the QuiesceMaster.
* The QM also uses these commands in its internal command tree
* as a result of breaking a command into smaller pieces.
*
********************************************************************/

typedef enum
{
	QMCmd_QuiesceBus = (QMCmdStart | 1),
	QMCmd_QuiesceIOP,
	QMCmd_QuiesceVirtualCircuit,
	QMCmd_QuiesceVdnPrimary,
	QMCmd_LastValid				// insert all valid cmnds above this one
}	QMCmd; // QuiesceMaster Command

/********************************************************************
*
* Actions are used by the QuiesceMaster to mainain its state and break 
* complex commands into simpler commands.  Commands created outside 
* of the QM should always use QmAction_DefaultAction.
*
********************************************************************/


struct QMCmd_Info
{
	U32 opcode;
	union {
		U16		bus;
		TySlot	tySlot;
		VDN		vdn;	
	};
};



#endif