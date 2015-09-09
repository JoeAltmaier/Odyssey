/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: LoopExtern.cpp
// 
// Description:
// This module has the Loop Monitor external entry points
// 
// Update Log:
//	$Log: /Gemini/Odyssey/IsmLoopMonitor/LoopExtern.cpp $
// 
// 2     8/14/99 9:28p Mpanas
// LoopMonitor version with most functionality
// implemented
// - Creates/Updates LoopDescriptor records
// - Creates/Updates FCPortDatabase records
// - Reads Export Table entries
// Note: This version uses the Linked List Container types
//           in the SSAPI Util code (SList.cpp)
// 
// 1     7/23/99 1:39p Mpanas
// Add latest versions of the Loop code
// 
// 
// 07/17/99 Michael G. Panas: Create file
/*************************************************************************/

#include "LmCommon.h"

#include "Pci.h"
#include "Scsi.h"
#include "CDB.h"
#include "CTIdLun.h"
#include "BuildSys.h"

#include "FcpData.h"
#include "FcpEvent.h"
#include "FcpLoop.h"
#include "FcpISP.h"
#include "FcpError.h"


/*************************************************************************/
// Forward references
/*************************************************************************/
extern "C" {
void Ext_Scan_Loops(Message *pMsg);
void Ext_LIP(U32 loop);
void Ext_Get_VP_DB(U32 loop);
void Ext_Get_AL_PA_Map(U32 loop);
void Ext_Get_Status(U32 loop);
void Ext_LM_Handle_Callback(void *pCtx, U32 status);
}

/*************************************************************************/
// Global references
/*************************************************************************/
extern LoopMonitorIsm			*pLoopMonitorIsm;



//=========================================================================
//
//  External Interfaces
//
//=========================================================================

/*************************************************************************/
// Ext_Scan_Loops
// External entry for scan
/*************************************************************************/
void Ext_Scan_Loops(Message *pMsg)
{
	TRACE_ENTRY(Ext_Scan_Loops);

	pLoopMonitorIsm->LM_Scan_Loops(pMsg);
	
} // Ext_Scan_Loops


/*************************************************************************/
// Ext_LIP
// External entry to send an LIP on the loop
/*************************************************************************/
void Ext_LIP(U32 loop)
{
	TRACE_ENTRY(Ext_LIP);

	pLoopMonitorIsm->LM_LIP(loop);
	
} // Ext_LIP


/*************************************************************************/
// Ext_Get_VP_DB
// External entry to send an LIP on the loop
/*************************************************************************/
void Ext_Get_VP_DB(U32 loop)
{
	TRACE_ENTRY(Ext_Get_VP_DB);

	pLoopMonitorIsm->LM_Get_VP_DB(loop);
	
} // Ext_Get_VP_DB

/*************************************************************************/
// Ext_Get_AL_PA_Map
// External entry to send an LIP on the loop
/*************************************************************************/
void Ext_Get_AL_PA_Map(U32 loop)
{
	TRACE_ENTRY(Ext_Get_AL_PA_Map);

	pLoopMonitorIsm->LM_Get_Pos_Map(loop);
	
} // Ext_Get_AL_PA_Map

/*************************************************************************/
// Ext_Get_Status
// External entry to get the state of a loop
/*************************************************************************/
void Ext_Get_Status(U32 loop)
{
	U16		state;
	U32		status;
	
	TRACE_ENTRY(Ext_Get_Status);

	status = pLoopMonitorIsm->LM_Get_State(loop, &state);
	
	TRACEF(TRACE_L2, ("\n\rLoop %d: state=%d", loop, state));
	
} // Ext_Get_Status


/*************************************************************************/
// Ext_LM_Handle_Callback
// External entry to continue an oeration that needed a callback (maybe)
/*************************************************************************/
void Ext_LM_Handle_Callback(void *pCtx, U32 status)
{
	LM_CONTEXT		*pTC = (LM_CONTEXT *)pCtx;
	LoopMonitorIsm::pLMCallback_t 	 Callback = pTC->Callback;
	U16				 state;
	
	TRACE_ENTRY(Ext_LM_Handle_Callback);

	// Do a callback if there is one
	if (Callback)
		(pLoopMonitorIsm->*Callback)((void *)pTC, status);
	else
		// done with context
		delete pTC;

} // Ext_LM_Handle_Callback


