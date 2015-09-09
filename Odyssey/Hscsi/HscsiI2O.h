/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
// 
// File: HscsiI2O.h
// 
// Description:
// This file defines interfaces for the QLogic ISP HSCSI driver
// that use I2O structures
//
// Update Log
//	$Log: /Gemini/Odyssey/Hscsi/HscsiI2O.h $ 
// 
// 1     9/14/99 7:24p Cchan
// Files for the HSCSI library, needed to support HBC-embedded QLogic
// ISP1040B SCSI chip.
//
/*************************************************************************/

#if !defined(HscsiI2O_H)
#define HscsiI2O_H

#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif

#pragma pack(1)


STATUS	HSCSI_I2O_Create(PHSCSI_INSTANCE_DATA Id);
void	HSCSI_I2O_Destroy();

void HSCSI_I2O_Add_SGL_Entry(void *p_msg,
	void *p_buffer, UNSIGNED buffer_size, UNSIGNED SGL_index);
	
void HSCSI_I2O_Get_SGL_Data_Segment(void* p_msg,
	HSCSI_DATA_SEGMENT_DESCRIPTOR *p_data_segment, UNSIGNED SGL_index);
	
UNSIGNED HSCSI_I2O_Get_SGL_Count(void* p_msg);

void HSCSI_I2O_Add_Payload(void *p_msg,
	void *p_payload, UNSIGNED payload_size);
	
void HSCSI_I2O_Add_Reply_Payload(void *p_msg,
	void *p_payload, UNSIGNED payload_size,
	STATUS DetailedStatusCode);
	
void *HSCSI_I2O_Get_Payload(void *p_msg);

void *HSCSI_I2O_Get_ID_LUN(void *p_msg);

STATUS HSCSI_I2O_Get_DetailedStatusCode(void *p_msg);

#ifdef          __cplusplus
}                                           /* End of C declarations     */
#endif


#endif /* HscsiI2O_H  */
