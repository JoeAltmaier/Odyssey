/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
// 
// File: FcpI2O.h
// 
// Description:
// This file defines interfaces for the QLogic ISP FCP driver
// that use I2O structures
//
// Update Log 
// 5/5/98 Jim Frandeen: Use C++ comment style
// 4/14/98 Jim Frandeen: Create file
// 6/2/98 Jim Frandeen: Change order to legaleese, description, update log
// 9/2/98 Michael G. Panas: add C++ stuff
// 9/18/98 Michael G. Panas: Add a payload struct to hold the payload data
//                           to insert into a Message. New method to add
//                           payload: FCP_I2O_Add_Payload(). General cleanup
//                           of things that are not used.
// 11/30/98 Michael G. Panas: New memory allocation methods
// 02/17/99 Michael G. Panas: convert to new Message format, add new methods
/*************************************************************************/

#if !defined(FcpI2O_H)
#define FcpI2O_H

#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif

#pragma pack(1)


STATUS	FCP_I2O_Create(PINSTANCE_DATA Id);
void	FCP_I2O_Destroy();

void FCP_I2O_Add_SGL_Entry(void *p_msg,
	void *p_buffer, UNSIGNED buffer_size, UNSIGNED SGL_index);
	
void FCP_I2O_Get_SGL_Data_Segment(void* p_msg,
	DATA_SEGMENT_DESCRIPTOR *p_data_segment, UNSIGNED SGL_index);
	
void FCP_I2O_Get_SGL_Entries(FCP_EVENT_CONTEXT *p_context, IOCB_COMMAND_TYPE4 *p_command);

void FCP_Delete_SGL(FCP_EVENT_CONTEXT *p_context);

UNSIGNED FCP_I2O_Get_SGL_Count(void* p_msg);

void FCP_I2O_Add_Payload(void *p_msg,
	void *p_payload, UNSIGNED payload_size);
	
void FCP_I2O_Add_Reply_Payload(void *p_msg,
	void *p_payload, UNSIGNED payload_size,
	STATUS DetailedStatusCode);
	
void *FCP_I2O_Get_Payload(void *p_msg);

void *FCP_I2O_Get_ID_LUN(void *p_msg);

STATUS FCP_I2O_Get_DetailedStatusCode(void *p_msg);

#ifdef          __cplusplus
}                                           /* End of C declarations     */
#endif


#endif /* FcpI2O_H  */
