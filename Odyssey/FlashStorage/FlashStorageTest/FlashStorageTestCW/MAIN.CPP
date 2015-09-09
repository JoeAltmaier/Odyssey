/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: main.cpp
//
// Description:
// This file contains the Initialize_SSD_Test() routine, which is the
// main application level module.  The demo version starts several tasks
// to demonstrate the capabilities of various synchronization methods.
//
//
// Update Log:
// 1/20/99 Jim Frandeen: Create file.
/*************************************************************************/

#define	TRACE_INDEX		TRACE_SSD
#include "stdio.h"

#include  "nucleus.h"
#include  "tc_defs.h"
#include  "Callback.h"
#include  "FlashStorage.h"
#include  "ErrorLog.h"
#include  "FfBat.h"
#include  "FtFlashTest.h"
#include  "FlashStorageBat.h"
#include  "TraceMon.h"
#include  "CwFlashTest.h"
#include  "FfController.h"
#include  "BuildSys.h" 
#include  "PciDev.h" 
#include  "Watch.h" 

#define EOL "\n"

#define MAX_PAGE_SIZE 16384


#define MEMORY_FOR_CALLBACKS 100000

#define TEST_PROGRAM_MEMORY_SIZE 			16000000				// 16 megabytes 
#define THREAD_STACK_SIZE 8192

/*************************************************************************/
// Globals
/*************************************************************************/
U32 version = 1;
U32 sub_version = 1;
char program_name[] = "FlashStorageTest";
U32 verify_erase = 3;
U32 verify_write = 1;
U32 verify_structures = 1;

        
/*************************************************************************/
// External function references.
/*************************************************************************/
extern "C" {
void	Print_String (char *string);
void	MSL_Initialize(void *first_available_memory, U32
			size_available_memory, U32 size_heap);
void    tx_application_define(void *first_available_memory);
} // extern "C"


/*************************************************************************/
// Define prototypes for function references.
/*************************************************************************/
void Run_Test(void *p_context, STATUS status);
void Init_Config(U32 memory_available);
void Format_Callback(
	U32 transfer_byte_count,
	I64 logical_byte_address,
	STATUS status,
	void *p_context);
U32 Get_Command();
char Get_Command_Char();
void Show_Version();


/*************************************************************************/
// Globals
/*************************************************************************/
extern void 				*p_page_buffer;
extern void 				*p_read_buffer;
extern void 				*p_write_buffer;
extern void 				*p_verify_buffer;
extern char					*p_string_buffer;
extern FF_CONFIG			 flash_config;
extern CM_CONFIG			 cache_config;
extern Flash_Device			 flash_device;
extern FF_HANDLE 			 FT_flash_handle;
extern FF_Controller 		*FT_p_controller;
extern FF_Page_Map 			*FT_p_page_map;
extern void					*FT_p_memory;
extern U32					 FT_size_bad_block_table;


void Initialize_Test(UNSIGNED argc, VOID *argv);

/*************************************************************************/
// Globals
/*************************************************************************/
NU_TASK         	Task_0;
NU_MEMORY_POOL  	system_memory;

/*************************************************************************/
// Initialize_SSD_Test
// Execution begins here
/*************************************************************************/
void    Initialize();
DEVICENAME(Initialize_SSD, Initialize);

void    Initialize()
{
    VOID    *pointer;
    
    // Create a semaphore so that this task will not finish.
	NU_SEMAPHORE		m_wait_forever_semaphore;
	STATUS status = NU_Create_Semaphore(&m_wait_forever_semaphore, "ReadySem",
		0, // initial count
		NU_FIFO);
	if (status != OK)
	{
		CT_Log_Error(CT_ERROR_TYPE_FATAL,
			"Initialize_SSD", 
			"NU_Create_Semaphore failed",
			status,
			0);
	}

    // Turn on tracing.
    //TraceLevel[TRACE_SSD] = TRACE_ALL_LVL;
   
	char *p_next_available_memory = new (tBIG) char[TEST_PROGRAM_MEMORY_SIZE];
	
    /* Create a system memory pool that will be used to allocate task stacks,
       queue areas, etc.  */
    status = NU_Create_Memory_Pool(&system_memory, "SYSMEM", 
	    p_next_available_memory, 
	    TEST_PROGRAM_MEMORY_SIZE, // # bytes in the pool
	    56, // min # bytes in each allocation,
	    NU_FIFO);
                        
	if (status != OK)
	{
		CT_Log_Error(CT_ERROR_TYPE_FATAL,
			"Initialize_SSD_Test", 
			"NU_Create_Memory_Pool failed",
			status,
			0);
	}

    /* Allocate stack space for task.  */
    status = NU_Allocate_Memory(&system_memory, &pointer, THREAD_STACK_SIZE, NU_NO_SUSPEND);
	if (status != OK)
	{
		CT_Log_Error(CT_ERROR_TYPE_FATAL,
			"Initialize_SSD_Test", 
			"Allocate stack space failed",
			status,
			0);
	}
    
    /* Create task  */
    status = NU_Create_Task(&Task_0, "TASK 0", Initialize_Test, 
    	0, NU_NULL, pointer,
                   THREAD_STACK_SIZE, 1, 20, NU_PREEMPT, NU_START);
	if (status != OK)
	{
		CT_Log_Error(CT_ERROR_TYPE_FATAL,
			"Initialize_SSD_Test", 
			"NU_Create_Task failed",
			status,
			0);
	}

	// Turn on tracing.
	//TraceLevel[TRACE_SSD] = 8;
	
	// Don't let this task finish!
	status = NU_Obtain_Semaphore(
		&m_wait_forever_semaphore,
		NU_SUSPEND);
	// Should never continue!
	//if (status != OK)
	{
		CT_Log_Error(CT_ERROR_TYPE_FATAL,
			"Initialize", 
			"ANU_Obtain_Semaphore failed",
			status,
			0);
	}
	
} // Initialize

/*************************************************************************/
// Initialize_Test
// Task execution begins here.
/*************************************************************************/
void   Initialize_Test(UNSIGNED argc, VOID *argv)
{
 	TRACE_ENTRY(Initialize_Test);
 	STATUS status;
 	
    // Allocate memory for callback contexts.
    void *p_memory;
    NU_Allocate_Memory(&system_memory, &p_memory, MEMORY_FOR_CALLBACKS, NU_NO_SUSPEND);
    
    // Initialize callbacks.
	status = Callback_Context::Initialize(p_memory, MEMORY_FOR_CALLBACKS, sizeof(FF_Bat_Context) + 16);
	if (status != OK)
	{
		CT_Log_Error(CT_ERROR_TYPE_FATAL,
			"Initialize_Test", 
			"Callback_Context::Initialize failed",
			status,
			0);
	}
	
    // Allocate memory for buffer for reading and writing pages.
    NU_Allocate_Memory(&system_memory, &p_page_buffer, MAX_PAGE_SIZE + 64, NU_NO_SUSPEND);
    p_page_buffer = (void *)ALIGN(p_page_buffer, 64);
    
    NU_Allocate_Memory(&system_memory, &p_read_buffer, MAX_PAGE_SIZE + 64, NU_NO_SUSPEND);
    p_read_buffer = (void *)ALIGN(p_read_buffer, 64);
    
    NU_Allocate_Memory(&system_memory, &p_write_buffer, MAX_PAGE_SIZE + 64, NU_NO_SUSPEND);
    p_write_buffer = (void *)ALIGN(p_write_buffer, 64);
    
    NU_Allocate_Memory(&system_memory, &p_verify_buffer, MAX_PAGE_SIZE + 64, NU_NO_SUSPEND);
    p_verify_buffer = (void *)ALIGN(p_verify_buffer, 64);
    
    // Allocate memory for buffer for statistics reporting.
    NU_Allocate_Memory(&system_memory, (void **)&p_string_buffer, 8000, NU_NO_SUSPEND);
    
	// Find out how much memory is left.
	CHAR 		pool_name[8];
	VOID		*start_address;
	UNSIGNED	pool_size;
	UNSIGNED	min_allocation;
	UNSIGNED	memory_available;
	OPTION		suspend_type;
	UNSIGNED	tasks_suspended;
	NU_TASK		*first_task;
	NU_Memory_Pool_Information(&system_memory,
		pool_name,
		&start_address,
		&pool_size,
		&min_allocation,
		&memory_available,
		&suspend_type,
		&tasks_suspended,
		&first_task);
	
    // Allocate the rest of memory for the file system.
    status = NU_Allocate_Memory(&system_memory, &FT_p_memory, 
    	memory_available - 100, NU_NO_SUSPEND);
	if (status != OK)
	{
		CT_Log_Error(CT_ERROR_TYPE_FATAL,
			"Initialize_Test", 
			"Allocate memory failed",
			status,
			0);
	}
    
    // How much is left now.
    U32 available_now;
	NU_Memory_Pool_Information(&system_memory,
		pool_name,
		&start_address,
		&pool_size,
		&min_allocation,
		&available_now,
		&suspend_type,
		&tasks_suspended,
		&first_task);
	
	// Initialize config structures.
	Init_Config(memory_available);
	
	// Create a parent context.
    FT_Test_Context *p_test_context = 
		(FT_Test_Context *)Callback_Context::Allocate(sizeof(FT_Test_Context));
	p_test_context->Set_Callback(&Run_Test);
	p_test_context->Make_Ready();
	Show_Version();
	
#if 0

	Tracef("\nSize of FF_SGL = %d", sizeof(FF_SGL));
	Tracef("\nSize of SGE_SIMPLE_ELEMENT = %d", sizeof(SGE_SIMPLE_ELEMENT));
	Tracef("\nSize of FF_Controller_Context = %d", sizeof(FF_Controller_Context));
	Tracef("\nSize of Flash_Device_Status = %d", sizeof(Flash_Device_Status));
	Tracef("\nSize of Flash_Address = %d", sizeof(Flash_Address));
#endif
	
	
} // Initialize_Test

/*************************************************************************/
// Show_Version
/*************************************************************************/
void Show_Version()
{
	printf("\n%s version %d.%d, %s, %s\n", 
		program_name, version, sub_version, __DATE__, __TIME__);
}
  

/*************************************************************************/
// Format_Callback
// Called by flash file system when format has completed.
/*************************************************************************/
void Format_Callback(

	// number of bytes successfully transferred
	U32 transfer_byte_count,
	
	// If operation did not succeed, logical byte address of failure.
	I64 logical_byte_address,

	// result of operation
	STATUS status,

	// pointer passed in to Flash File method
	void *p_context)
{
	FT_Test_Context *p_parent_context = (FT_Test_Context *)p_context;

    p_parent_context->m_num_pages = transfer_byte_count;

	// Schedule the parent context.
	p_parent_context->Make_Ready();

} // Format_Callback

/*************************************************************************/
// StartTask.
/*************************************************************************/
extern "C" void  StartTask(UNSIGNED argc, VOID *argv);
extern "C" Status Init_Hardware();
void  StartTask(UNSIGNED , VOID *)
{
	Status status = Init_Hardware();
	
	TraceLevel[TRACE_SSD] = TRACE_L3;	
#if 0
	// All levels set to 0 by default
	TraceLevel[TRACE_SSD1] = TRACE_L3;	
	//TraceLevel[TRACE_MESSENGER1] = TRACE_L2;	
	//TraceLevel[TRACE_DDM_MGR] = TRACE_L2;	
	//TraceLevel[TRACE_DDM] = TRACE_L2;	
	//TraceLevel[TRACE_HEAP1] = TRACE_L1;	
	//TraceLevel[TRACE_PTS] = TRACE_ALL_LVL;
#endif	
	
	// start the OS
	Oos::Initialize();
} // StartTask


