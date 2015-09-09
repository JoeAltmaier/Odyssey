/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FcpLoop.h
// 
// Description:
// This file defines the external FC Loop interfaces to the FCP driver. 
// 
// Update Log: 
//	$Log: /Gemini/Odyssey/FCP/FcpLoop.h $
// 
// 4     8/13/99 4:35p Mpanas
// Remove call pointer (defined elsewhere)
// 
// 3     8/12/99 9:19p Mpanas
// Add user context pointer for callbacks
// 
// 2     6/18/99 6:45p Mpanas
// Initial ISP2200 support
// 
// 1     6/06/99 4:14p Mpanas
// New Loop control files
//
// 06/02/99 Michael G. Panas: Create file
/*************************************************************************/

#if !defined(FcpLoop_H)
#define FcpLoop_H


#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif

STATUS FCP_Initiate_LIP(PINSTANCE_DATA Id);
STATUS FCP_Initiate_LIP_Reset(PINSTANCE_DATA Id, U8 Loop_ID, U16 Delay);
STATUS FCP_Get_Loop_ID(PINSTANCE_DATA Id);
STATUS FCP_Get_FC_AL_Position_Map(PINSTANCE_DATA Id, U8 *Address,U16 *map_type);
STATUS FCP_Get_Port_Data_Base(PINSTANCE_DATA Id, U8 *Address,
					U16 Loop_ID, U8 VP_Index);
STATUS FCP_Get_Firmware_State(PINSTANCE_DATA Id, U16 *state);
STATUS FCP_Get_Port_Name(PINSTANCE_DATA Id, U8 *Address,
					U8 Loop_ID, U8 VP_Index, U8 Options);
STATUS FCP_Get_Link_Status(PINSTANCE_DATA Id, U8 *Address,
					U8 Loop_ID, U8 Loop_Port);
STATUS FCP_Get_VP_Database(PINSTANCE_DATA Id, U8 *Address, U16 *VP_Entry_Count);
STATUS FCP_Get_VP_Database_Entry(PINSTANCE_DATA Id, U8 *Address, U16 VP_Index);

STATUS FCP_VP_Control(PINSTANCE_DATA Id, 
			U16 VP_Count, U8 *VP_Indexes,
			U8 command, void *user);

STATUS FCP_Modify_VP_Config(PINSTANCE_DATA Id, 
			U16 VP_Count, U8 *VP_Indexes,
			VP_CONFIG *vp_config, U8 command,
			void *user);

STATUS FCP_Handle_Loop_IOCB_Completion(FCP_EVENT_CONTEXT *p_context);

STATUS FCP_Handle_Loop_Event(FCP_EVENT_CONTEXT *p_context);

#ifdef          __cplusplus
}                                           /* End of C declarations     */
#endif

#endif /* FcpLoop_H  */
