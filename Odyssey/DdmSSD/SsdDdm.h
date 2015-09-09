/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: SsdDdm.h
// 
// Description:
// This file defines the Device Dependent Module class for the
// Solid State Drive. 
// 
// $Log: /Gemini/Odyssey/DdmSSD/SsdDdm.h $
// 
// 15    2/05/00 2:26p Jfrandeen
// 
// 14    12/10/99 1:18p Jfrandeen
// Add Decrement_Requests_Outstanding method.  RowID for status and
// performance table.  
// 
// 13    11/22/99 10:47a Jfrandeen
// Use new FlashStorage in place of FlashFile
// 
// 12    10/26/99 3:37p Hdo
// Add Ssd_Table_Get_ridIopStatus() and Ssd_Get_ridIopStatus_Reply()
// 
// 11    10/21/99 1:07p Hdo
// Change from the SSD_Context to SSDConfig
// Re-write of SSDTable
// 
// 10    10/14/99 4:56p Hdo
// Add code to support PHS Reporter
// 
// 02/25/99 Jim Frandeen: Create file
/*************************************************************************/

#ifndef DdmSSD_H
#define DdmSSD_H

#include "Simple.h"
#include "Message.h"
#include "Ddm.h"

#include "FlashStorage.h"
#include "SsdDevice.h"
#include "SsdContext.h"
#include "SsdRequest.h"
#include "Hw.h"
#include "SsdDescriptor.h"

#include "Table.h"
#include "Rows.h"
#include "Fields.h"

#include "IopStatusTable.h"
#include "StorageRollCallTable.h"

// SSD Config record from the PTS
#include "SsdConfig.h"

// Performance,Health, and Status
#include "SSDStatus.h"
#include "SSDPerformance.h"

// Temporary memory size until we get it from the PTS.
#define SSD_MEMORY_SIZE 16000000 // 16 meg

#define SSD_RESET_MESSAGE (Message *) 0XFFFFFFFF

#include "RqDdmReporter.h"
//#define PHS_REPORTER		// uncomment this to activate the Reporter


/*************************************************************************/
// SSD_Ddm
// This is the class description for the Solid State Disk
// Device Dependent Module.
/*************************************************************************/
class SSD_Ddm: public Ddm {

public: // Methods

	// Virtual methods derived from Ddm.
	SSD_Ddm(DID did);
	virtual STATUS Initialize(Message *pMsg);
	virtual STATUS Enable(Message *pMsg);
	virtual STATUS Quiesce(Message *pMsg);
	static Ddm *Ctor(DID did);

	FF_CONFIG *Get_Config();
	FF_HANDLE Flash_Handle();

public:
	VDN		MyVdn;			// my virtual device number
	DID		MyDid;			// my DID

private: // callback methods

	static void Process_Quiesce			(void *p_context, Status status);
	static void Read_Write_Complete		(void *p_context, Status status);
	static void Process_Open_Flash		(void *p_context, Status status);
	static void Process_Create_Flash	(void *p_context, Status status);

	static void Read_Write_Callback(

		// number of bytes successfully transferred
		U32 transfer_byte_count,

		// If operation did not succeed, logical byte address of failure.
		I64 logical_byte_address,

		// result of operation
		STATUS status,

		// pointer passed in to Flash File method
		void *p_context);

	static void Format_Callback(

		// number of bytes successfully transferred
		U32 transfer_byte_count,

		// If operation did not succeed, logical byte address of failure.
		I64 logical_byte_address,

		// result of operation
		STATUS status,

		// pointer passed in to Flash File method
		void *p_context);

private: // Action callback methods 

	static STATUS Respond_To_Format_Request(void *p_context);
	static STATUS Process_Reformat(void *p_context);

private: // Helper methods

	STATUS Get_BSA_Status(SSD_Request_Context *p_request_context);
	STATUS Process_BSA_Request(Message *pMsg);
	STATUS Process_Format_Request(Message *pMsg);
	STATUS Respond_To_BSA_Request(SSD_Request_Context *p_request_context);
	STATUS Reformat();
	STATUS Respond_To_Enable_Request(SSD_Request_Context *p_request_context);
	STATUS Initialize_Scheduler();
	void   Reply_With_Status(Message *p_message, STATUS status);
	void   Decrement_Requests_Outstanding();

	STATUS Ssd_Table_Modify_Capacity();
	STATUS Ssd_Table_Get_ridIopStatus();
	STATUS Ssd_Get_ridIopStatus_Reply(void *pContext, STATUS status);

	// Descriptor Table initialize methods
	STATUS Ssd_Table_Initialize(Message *pMsg);
	STATUS Ssd_DescTable_InsertRow(void *pClientContext, STATUS status);
	STATUS Ssd_DescTable_ReadNumRow(void *pClientContext, STATUS status);
	STATUS Ssd_DescTable_Read_Reply(void *pClientContext, STATUS status);
	STATUS Ssd_DescTable_Read_Rows(void *pClientContext, STATUS status);
	STATUS Ssd_DescTable_ReadRow_Reply(void *pClientContext, STATUS status);
	STATUS Ssd_DescTable_Reply_Last(void *pClientContext, STATUS status);

	// StorageRollCall Table initialize methods
	STATUS Ssd_SRCTable_Initialize(void *pClientContext, STATUS status);
	STATUS Ssd_SRCTable_InsertRow(void *pClientContext, STATUS status);
	STATUS Ssd_SRCTable_ReadTable(void *pClientContext, STATUS status);
	STATUS Ssd_SRCTable_Read_Reply(void *pClientContext, STATUS status);
	STATUS Ssd_SRCTable_Reply_Last(void *pClientContext, STATUS status);

	// PHS Reporter request handler
	STATUS Process_PHS_Request(Message *pMsg);

private: // ISR helper methods

	static void Reset_ISR_High();
	static void Reset_ISR_Low(INT vector_number);
	Status Open_Reset_ISR();
	Status Close_Reset_ISR();

private: // member data

	// Capacity of flash file system -- number of bytes of user data available.
	I64				m_capacity;

	// Callback context used for initialize, quiesce, enable, and format methods.
	// Only one of these could be happening at any one time.
	// We allocate a static context
	// here so that we are assured of having one for quiesce.
	SSD_Request_Context	m_request_context;

	// Configuration data gets stored here by the Persistent Table Service.
	SSD_CONFIG		m_config;

	// Task structure for context scheduler.
	NU_TASK         m_scheduler_task;

	// Memory allocated for callback contexts.
	void			*m_p_callback_context_memory;

	// Memory allocated for flash file system.
	void			*m_p_flash_file_system_memory;

	// Pointer to quiesce message if we are in the process of getting quiesced
	Message			*m_p_quiesce_message;

	// Pointer to enable message if we are in the process of enabling
	Message			*m_p_enable_message;

	// Pointer to format message if we are in the process of formatting
	Message			*m_p_format_message;

	// Number of BSA requests currently outstanding.
	U32				 m_num_requests_outstanding;

	// Flag is set when flash file system is opened.
	U32				 m_flash_file_system_open;

	// Handle of flash file system
	FF_HANDLE 		 m_flash_handle;

	// Our device
	static SSD_Device	m_device;

	// HISR stack
	void 			*m_p_HISR_stack;

	// HISR object
	NU_HISR			 m_reset_HISR;

	// Pointer to SSD_Ddm, used by reset HISR.
	static SSD_Ddm 	*m_p_ddm;

	// Pointer to Quiesce message, used by reset HISR.
	Message			*m_p_reset_message;

	// m_OK_to_reset is set when enabled, reset when reset is in progress.
	U32				 m_OK_to_reset;
	
	// PTS Table Data
	TSDefineTable	*m_p_define_table;
	TSDefineTable	*m_p_define_SRC_table;

	TSInsertRow		*m_p_insert_row;
	TSInsertRow		*m_p_insert_row_SRC;

	TSEnumTable		*m_p_enum_table;
	TSGetTableDef	*m_p_TSGetTableDef;
	TSReadRow		*m_p_ReadRow;
	TSReadRow		*m_p_IOP_ReadRow;
	TSModifyRow		*m_p_modify_row;
	TSModifyField	*m_p_modify_field;

	rowID			 m_row_ID_SSD_descriptor;
	rowID			 m_row_ID_SRC;
	U32				 m_num_table_rows;
	U32				 m_num_bytes_enumerated;
	U32				 m_nTableRowsRead;
	U32				 m_nIopRowsRead;
	rowID			 m_rid_StringName;

	SSD_Descriptor			*m_p_SSD_descriptor;
	StorageRollCallRecord	*m_p_SRC;
	IOPStatusRecord			*m_p_IopStatus;

	// SSD Status record
	SSD_STATUS		m_Status;
	rowID			m_rid_Status;

	// SSD Performance record
	SSD_PERFORMANCE	m_Performance;
	rowID			m_rid_Performance;

}; // SSD_Ddm

inline FF_CONFIG * SSD_Ddm::Get_Config()
{
	return &m_p_SSD_descriptor->flash_config;
}

inline FF_HANDLE SSD_Ddm::Flash_Handle()
{
	return m_flash_handle;
}


// Reset ISR definitions
#define SSD_RESET_HISR_NAME "SSDHISR"
#define SSD_RESET_HISR_STACK_SIZE 2000
#define SSD_RESET_INTERRUPT_VECTOR_NUMBER 1
#define SSD_RESET_HISR_PRIORITY 0 // 2 is the lowest priority, 0 is the highest
#define SSD_RESET_PHYS_ADDR 0X1C0B0000
#define	SSD_RESET_KSEG1_ADDR			(SSD_RESET_PHYS_ADDR + KSEG1_BASE_ADDR)
#define SSD_RESET_ACK_REGISTER *(char *)(SSD_RESET_KSEG1_ADDR)
#define WARM_RESET_PHYS_ADDR 0X1C0E0000
#define	WARM_RESET_KSEG1_ADDR			(SSD_RESET_PHYS_ADDR + KSEG1_BASE_ADDR)
#define WARM_RESET_REGISTER  *(char *)(WARM_RESET_KSEG1_ADDR)

#endif // DdmSSD_H
