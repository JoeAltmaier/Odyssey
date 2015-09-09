/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: LoopMultiTarget.cpp
// 
// Description:
// This module is the Loop Monitor Muliple-Target ID Handler
//	Handles:
//		Loop Descriptor Table Create/Read/Write/Update
//		FC Port Descriptor Table Create/Read/Write/Update
//		Export Table Reads
// 
// Update Log:
//	$Log: /Gemini/Odyssey/IsmLoopMonitor/LoopMultiTarget.cpp $
// 
// 2     1/09/00 7:50p Mpanas
// Set the IDsInUse field and the TargetIDs[] array of the
// LoopDescriptor correctly.
// 
// 1     8/14/99 9:24p Mpanas
// New file to support Multiple Target operations
// 
// 
// 08/06/99 Michael G. Panas: Create file
/*************************************************************************/

#include "LmCommon.h"


/*************************************************************************/
// Forward references
/*************************************************************************/

/*************************************************************************/
// Global references
/*************************************************************************/



/*************************************************************************/
// LM_MT_OK
// Check if Multi-Target mode is OK to use, return one when OK
/*************************************************************************/
U32 LoopMonitorIsm::LM_MT_OK(U32 chip)
{
	PINSTANCE_DATA	 Id = &Instance_Data[chip];
	extern	unsigned char firmware_version[];

	TRACE_ENTRY(LM_MT_OK);
	
	// must be a 2200 to even consider
	if (Id->ISP_Type == 0x00002200)
	{
		// check the 2200 firmware major revision
		// 2 is 2200ef, 10 is 2200efm
		if (firmware_version[0] >= 10)
		{
			return(1);
		}
	}
	
	return(0);	// not OK

} // LM_MT_OK

/*************************************************************************/
// LM_Expose_IDs
// Check IDs to see if they need to be enabled
// Expects m_num_IDs to be set to the number of new IDs needed and m_ids[]
// to be set to the actual new ID values.  We can not expose more than
// one ID when the the chip is a 2100 or the 2200 firmware is not 2200efm.
/*************************************************************************/
void LoopMonitorIsm::LM_Expose_IDs(U32 loop)
{
	U32				 i, index;
	U32				 chip = FCLOOPCHIP(loop);
	LM_CONTEXT		*pLmc;
	VP_CONFIG		*pVpConfig, *pVp;
	
	TRACE_ENTRY(LM_Expose_IDs);
	
	if (m_num_IDs[chip] == 0)
	{
		// must have been a misteak?
		return;
	}
	
	if (LM_MT_OK(chip))
	{
		if ((LM_Loop_Desc[chip]->IDsInUse + m_num_IDs[chip]) > 32)
		{
			TRACEF(TRACE_L2, ("\n\rLM_Expose_IDs: Failed - too many IDs: %d", 
								(LM_Loop_Desc[chip]->IDsInUse + m_num_IDs[chip])));
			// TODO: this is an error!
			return;
		}
	
		// get an index to the last ID + 1
		// This also the VP_Index of the next entry
		index = LM_Loop_Desc[chip]->IDsInUse;
		
		// allocate enough space for all the new VP_Configs
		pVpConfig = new VP_CONFIG[m_num_IDs[chip]];
		pVp = pVpConfig;
		
		// loop for all the IDs
		for (i = 0; i < m_num_IDs[chip]; i++)
		{
			// build a new VP_CONFIG for this ID
			LM_Build_VP_Config(pVp, m_ids[chip][i], loop);
			
			// next
			pVp++;
		}
	
		// Get ready to start the statemachine to update the VP_CONFIG entries
		// in the 2200.  When we are done with that, we need to enable the
		// new IDs in our list.
		pLmc = new LM_CONTEXT;
		memset(pLmc, 0, sizeof(LM_CONTEXT));
		pLmc->loop_number = loop;
		
		// save the list we just built
		pLmc->buffer = (void *)pVpConfig;
		// and a pointer to it
		pLmc->ptr = (void *)pVpConfig;
		
		// get an index to the last ID + 1
		index = LM_Loop_Desc[chip]->IDsInUse;
		
		// save the starting index (all will be sequencial from here)
		pLmc->index = index;
		pLmc->start_index = index;
		pLmc->num_indexes = m_num_IDs[chip];
		
		// calulate the last index needed
		pLmc->last = index + m_num_IDs[chip];
	}
	
	if (LM_MT_OK(chip))
	{
		// add in the count of new IDs
		LM_Loop_Desc[chip]->IDsInUse += m_num_IDs[chip];
	}
	else
	{
		// check for > 1 ID configured
		if (LM_Loop_Desc[chip]->IDsInUse != 0)
		{
			// sorry, only one ID allowed!
			TRACEF(TRACE_L2, ("\n\rLM_Expose_IDs: Failed - only one ID allowed!"));
			// TODO: this is an error!
			return;
		}
		else
		{
			LM_Loop_Desc[chip]->IDsInUse++;
		}
	}
	
	// add the actual IDs to our array
	for (i = 0; i < m_num_IDs[chip]; i++, index++)
	{
		// add a new ID to the list
		LM_Loop_Desc[chip]->TargetIDs[index] = m_ids[chip][i];
	}
	
	if (LM_MT_OK(chip))
	{
		// start the statemachine if Multi-Target mode is OK
		LM_Do_Expose_Callback_Modify_VP(pLmc, 0);
	}
	else
	{
		// Otherwise just update the table entry for this loop
		LmLpTableUpdate(NULL, NULL, loop, (pLMCallback_t) NULL);
	}

} // LM_Expose_IDs


/*************************************************************************/
// LM_Build_VP_Config
// Build up a VP_CONFIG entry for the id and loop instance passed
/*************************************************************************/
void LoopMonitorIsm::LM_Build_VP_Config(VP_CONFIG *pVp, U32 id, U32 LoopInstance)
{
	U32				 chip = FCLOOPCHIP(LoopInstance);

	TRACE_ENTRY(LM_Build_VP_Config);
	
	// options
	pVp->options = 0;

	// set the id
	pVp->hard_ID = id;
	
	// TODO:
	// get the port name
	
	// get the node name
	
} // LM_Build_VP_Config



/*************************************************************************/
// LM_Do_Expose_Callback_Modify_VP
// Modify all the VP_CONFIGs requested, recursive callbacks come here
/*************************************************************************/
void LoopMonitorIsm::LM_Do_Expose_Callback_Modify_VP(void *ptr, STATUS status)
{
	LM_CONTEXT		*pLmc = (LM_CONTEXT *) ptr;
	VP_CONFIG		*vp_config;
	
	TRACE_ENTRY(LM_Do_Expose_Callback_Modify_VP);
	
	// TODO:
	// check for errors
	
	// check to see if we are done
	if (pLmc->index == pLmc->last)
	{
		// done - enable all the IDs
		LM_Do_Expose_Callback_Enable(pLmc, 0);
		return;
	}
	
	// need to come back here until all the VP_CONFIG entries are done
	pLmc->Callback = (pLMCallback_t) &LM_Do_Expose_Callback_Modify_VP;
	
	// point to current
	vp_config = (VP_CONFIG *)pLmc->ptr;
	pLmc->idx = pLmc->index;

	// just do one entry each time
	LM_Modify_VP_Config(pLmc, 1, &pLmc->idx, vp_config,
				ISP_MOD_VP_CFG_MODIFY_VPS);
	
	// bump to next entry
	vp_config++;
	pLmc->ptr = (void *)vp_config;
	pLmc->index++;
		
} // LM_Do_Expose_Callback_Modify_VP


/*************************************************************************/
// LM_Do_Expose_Callback_Enable
// Enable the Loop IDs that have been requested
/*************************************************************************/
void LoopMonitorIsm::LM_Do_Expose_Callback_Enable(void *ptr, STATUS status)
{
	LM_CONTEXT		*pLmc = (LM_CONTEXT *) ptr;
	U32				 chip = FCLOOPCHIP(pLmc->loop_number);
	U8				*p;
	U32				 index;
	
	TRACE_ENTRY(LM_Do_Expose_Callback_Enable);

	// TODO:
	// check for errors
	
	// this is where we go when we are done
	pLmc->Callback = (pLMCallback_t) &LM_Do_Expose_Callback_Last;
	
	// use the first part of the vp buffer for the enable vp index list
	p = (U8 *)pLmc->buffer;
	index = pLmc->start_index;
	
	// build the list in sequence
	for (int i = 0; i < pLmc->num_indexes; i++)
	{
		p[i] = index++;
	}
	
	// enable all the new IDs just configured
	LM_Enable_Loop_ID(pLmc, (U16)pLmc->num_indexes, (U8 *)pLmc->buffer);

} // LM_Do_Expose_Callback_Enable


/*************************************************************************/
// LM_Do_Expose_Callback_Last
// finish operation requested, the last thing to do is update the 
// LoopDescriptor with the new IDs.
/*************************************************************************/
void LoopMonitorIsm::LM_Do_Expose_Callback_Last(void *ptr, STATUS status)
{
	LM_CONTEXT		*pLmc = (LM_CONTEXT *) ptr;
	
	TRACE_ENTRY(LM_Do_Expose_Callback_Last);

	// TODO:
	// check for errors
	
	// Update the table entry for this loop
	LmLpTableUpdate(NULL, NULL, pLmc->loop_number, (pLMCallback_t) NULL);
		
	// delete memory allocated for the VP_CONFIG tables
	delete pLmc->buffer;
	
	// delete context - we are done with it
	delete pLmc;
	
} // LM_Do_Expose_Callback_Last



/*************************************************************************/
// LM_Disable_Loop_ID
// Enable the IDs in pMsg
// Needs a Callback to complete since it uses IOCBs down below.
/*************************************************************************/
void LoopMonitorIsm::LM_Disable_Loop_ID(void *pCtx,
				U16 VP_Count, U8 *VP_Indexes)
{
	LM_CONTEXT		*pLmc = (LM_CONTEXT *) pCtx;
	U32				 chip = FCLOOPCHIP(pLmc->loop_number);
	PINSTANCE_DATA	 Id = &Instance_Data[chip];
	STATUS			 status = 0;
	
	TRACE_ENTRY(LM_Disable_Loop_ID);

	status = FCP_VP_Control(Id, VP_Count, VP_Indexes,
									ISP_VP_CTL_DISABLE_VPS_LIP,
									(void *)pCtx);
	
	if (status != MB_STS_GOOD)
	{
		TRACE_HEX(TRACE_L2, "\n\rLM_Disable_Loop_ID: Failed - ", status);
	}
	
} // LM_Disable_Loop_ID


/*************************************************************************/
// LM_Enable_Loop_ID
// Enable the IDs in pMsg
// Needs a Callback to complete since it uses IOCBs down below.
/*************************************************************************/
void LoopMonitorIsm::LM_Enable_Loop_ID(void *pCtx,
				U16 VP_Count, U8 *VP_Indexes)
{
	LM_CONTEXT		*pLmc = (LM_CONTEXT *) pCtx;
	U32				 chip = FCLOOPCHIP(pLmc->loop_number);
	PINSTANCE_DATA	 Id = &Instance_Data[chip];
	STATUS			 status = 0;
	
	TRACE_ENTRY(LM_Enable_Loop_ID);

	status = FCP_VP_Control(Id, VP_Count, VP_Indexes,
									ISP_VP_CTL_ENABLE_VPS,
									(void *)pCtx);
	
	if (status != MB_STS_GOOD)
	{
		TRACE_HEX(TRACE_L2, "\n\rLM_Enable_Loop_ID: Failed - ", status);
	}
	
} // LM_Enable_Loop_ID


/*************************************************************************/
// LM_Get_VP_DB
// Print the VP Database
// loop is the loop instance number
/*************************************************************************/
void LoopMonitorIsm::LM_Get_VP_DB(U32 loop)
{
	LM_CONTEXT		*pLmc;
	U8				*buffer = (U8 *)new char[2048];
	PINSTANCE_DATA	 Id = &Instance_Data[loop];
	STATUS			 status;
	U16				 cnt = 0;
	
	TRACE_ENTRY(LM_Get_VP_DB);

	status = FCP_Get_VP_Database(Id, buffer, &cnt);
	
	if (status != MB_STS_GOOD)
	{
		TRACE_HEX(TRACE_L2, "\n\rLM_Get_VP_DB: Failed - ", status);
	}
	else
	{
		TRACE_HEX16(TRACE_L2, "\n\rVP Count = ", cnt);
		TRACE_DUMP_HEX(TRACE_L2, "\n\rVP Database:", buffer, 1280);
	}
	
} // LM_Get_VP_DB


/*************************************************************************/
// LM_Modify_VP_Config
// Change the VP Database
// Needs a Callback to complete since it uses IOCBs down below.
/*************************************************************************/
void LoopMonitorIsm::LM_Modify_VP_Config(void *pCtx,
				U16 VP_Count, U8 *VP_Indexes, VP_CONFIG *vp_config,
				U8 command)
{
	LM_CONTEXT			*pLmc = (LM_CONTEXT *) pCtx;
	U32					 chip = FCLOOPCHIP(pLmc->loop_number);
	PINSTANCE_DATA		 Id = &Instance_Data[chip];
	STATUS				 status = 0;
	
	TRACE_ENTRY(LM_Modify_VP_Config);

	status = FCP_Modify_VP_Config(Id, VP_Count, VP_Indexes, 
											vp_config, command,
											(void*) pCtx);
	
	if (status != MB_STS_GOOD)
	{
		TRACE_HEX(TRACE_L2, "\n\rLM_Modify_VP_Config: Failed - ", status);
	}
	
} // LM_Modify_VP_Config


