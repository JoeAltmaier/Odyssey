/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: Command.cpp
//
// Description:
// This file contains the test command interpreter.
//
//
// Update Log:
// 3/25/99 Jim Frandeen: Create file.
/*************************************************************************/

/*************************************************************************/
// Set break here for Break command
/*************************************************************************/
void Break()
{
}

#define	TRACE_INDEX		TRACE_SSD
#include "stdio.h"

#include  "nucleus.h"
#include  "tc_defs.h"
#include  "Callback.h"
#include  "FlashStorage.h"
#include  "ErrorLog.h"
#include  "FfPageMap.h"
#include  "SsdDevice.h"
#include  "FfController.h"
#include  "FtFlashTest.h"
#include  "FlashStorageBat.h"
#include  "TraceMon.h"
#include  "CwFlashTest.h"
#include  <string.h>

#ifdef TESTHBC
#include	"HbcFlashDevice.h"
#endif

#define EOL "\n"
#define MAX_PAGE_SIZE 16384


/*************************************************************************/
// Globals
/*************************************************************************/
U32					*FT_pointer = (U32 *)0X8402DeD0; // for debugger watching
CM_STATISTICS 		cache_statistics;
CM_EVENT_DATA 		cache_event_data;
U32					closing_flash = 0;
FF_BLOCK_TEST_DATA	bat_data_buffer[FF_NUM_CELL_PARTS];
char				*p_string_buffer;
void 				*p_page_buffer;
void 				*p_read_buffer;
void 				*p_write_buffer;
void 				*p_verify_buffer;
I64					align_flash_statistics;
FF_STATISTICS 		flash_statistics;
I64					align_flash_event_data;
FF_CONFIG			flash_config;
CM_CONFIG			cache_config;
FF_EVENT_DATA 		flash_event_data;
U32					FT_array;
U32					FT_column;
U32					FT_page;
U32					FT_sector;
U32					FT_block;
FF_HANDLE			FT_flash_handle = 0;
void				*FT_p_memory;

#ifdef TESTHBC
HBC_Flash_Device	flash_device;
#else
SSD_Device			flash_device;
#endif

FF_Controller 		*FT_p_controller;
FF_Page_Map 		*FT_p_page_map;
Flash_Address		FT_flash_address;
U32					FT_initialized = 0;
U32					FT_size_bad_block_table;

/*************************************************************************/
// Parameters for tests that could be specified by keyboard input.
/*************************************************************************/
U32					num_blocks = 100;
U32					starting_block = 0;
extern U32			verify_erase;
extern U32			verify_write;
extern U32			verify_structures;



/*************************************************************************/
// Define prototypes for function references.
/*************************************************************************/
U32 Get_Yes(char *p_message);
void Run_Test(void *p_context, STATUS status);
void Bad_Block_Table_Complete(void *p_context, STATUS status);
void Flash_Initialized(void *p_context, STATUS status);
void Run_Test_Complete(void *p_context, STATUS status);
void Format_Callback(
	U32 transfer_byte_count,
	I64 logical_byte_address,
	STATUS status,
	void *p_context);
U32 Get_Command();
char Get_Command_Char();
U32 Get_Hex_Number();
UI64 Get_Hex(char *p_message);
void Display_Read_Buffer();
void Display_Write_Buffer();
void Break();
void Show_Commands();
void Surface_Test_Complete(void *p_context, STATUS status);
void Test_Unit(FT_Test_Context *p_test_context);
void Write_Alternate(void *p_context, STATUS status);
void Erase_Blocks(void *p_context, STATUS status);
int Check_Flash_Handle();

/*************************************************************************/
// Run_Test
// Called by Scheduler.
// Read a command from the keyboard, run the specified test.
// Run_Test_Complete gets scheduled to run when the test is complete. 
/*************************************************************************/
void Run_Test(void *p_context, STATUS status)
{
 	TRACE_ENTRY(Run_Test);
	FT_Test_Context *p_test_context = (FT_Test_Context *)p_context;
	
	U32 fill_character;
	U32 IDword_page;
	U32 array_address;
	U32 column_address;
	
	if (closing_flash)
	{
		// Close the simulated flash file system.
		closing_flash = 0;
		Close_Flash();
		FT_flash_handle = 0;
	}

	// Call Run_Test_Complete when bat has completed.
    p_test_context->m_num_pages = 0;
    p_test_context->m_p_buffer = 0;
	p_test_context->Set_Callback(&Run_Test_Complete);
	
	U32 get_command = 1;
	while (get_command)
	{
		// Assume we will get a legal command.
		get_command = 0;
		U32 command_code = Get_Command();
			
		switch (command_code)
		{
			case 'ba': 
			
				Tracef(EOL "Basic Assurance Test...");
				if (Check_Flash_Handle() == 0)
				{
					get_command = 1;
					break;
				}
				Open_Flash();
				status = FF_Run_Bat(
					FT_flash_handle,
					&bat_data_buffer[0],
					sizeof(bat_data_buffer),
					p_test_context);
				break;
				
			case 'br': 
			
				Tracef(EOL "Break (debugger call)");
				Break();
				get_command = 1;
				break;
				
			case 'cb': 
			
				Tracef(EOL "Create Bad block table...");
				
				// Call Bad_Block_Table_Complete when complete
				p_test_context->Set_Callback(&Bad_Block_Table_Complete);
		
				status = FF_Create(
					&flash_config,
				    &cache_config,
					p_test_context);
				break;
				
			case 'cl': 
			
				Tracef(EOL "Close flash file system...");
				if (FT_flash_handle == 0)
				{
					Tracef(EOL "Flash file system is not open");
					get_command = 1;
					break;
				}
				closing_flash = 1;
				status = FF_Close(FT_flash_handle, p_test_context);
				break;
				
			case 'cs':
			
				Tracef(EOL "Cache Statistics...");
				if (Check_Flash_Handle() == 0)
				{
					get_command = 1;
					break;
				}
				status = FF_Display_Cache_Stats_And_Events(FT_flash_handle);
				get_command = 1;
				break;
			
			case 'dr':
			
				Tracef(EOL "Display Read buffer");
				if (Check_Flash_Handle() == 0)
				{
					get_command = 1;
					break;
				}
				Display_Read_Buffer();
				get_command = 1;
				break;
			
			case 'dw':
			
				Tracef(EOL "Display Write buffer");
				if (Check_Flash_Handle() == 0)
				{
					get_command = 1;
					break;
				}
				Display_Write_Buffer();
				get_command = 1;
				break;
			
			case 'eb':
			
				Tracef(EOL "Erase page Block (specified by SB, SC, SP)");
				if (Check_Flash_Handle() == 0)
				{
					get_command = 1;
					break;
				}
				if (FT_sector != 0)
				{
					Tracef(EOL " page %X is not on a sector boundary", FT_page);
					get_command = 1;
					break;
				}
				FT_flash_address.Initialize();
				FT_flash_address.Array(FT_array);
				FT_flash_address.Column(FT_column);
				FT_flash_address.Block(FT_block);
				FT_flash_address.Sector(FT_sector);
				Tracef(EOL "Erasing flash address %X, page %X, array %X, column %X", 
					FT_flash_address.Index(), FT_page, FT_array, FT_column);
				status = FT_p_controller->Erase_Page_Block(p_test_context, FT_flash_address);
					
				break;
			
			case 'em':
			
				Tracef(EOL "Erase multiple page blocks (specified by SB, SC, SP)");
				if (Check_Flash_Handle() == 0)
				{
					get_command = 1;
					break;
				}
				if (FT_sector != 0)
				{
					Tracef(EOL " page %X is not on a sector boundary", FT_page);
					get_command = 1;
					break;
				}
				FT_flash_address.Initialize();
				FT_flash_address.Array(FT_array);
				FT_flash_address.Column(FT_column);
				FT_flash_address.Block(FT_block);
				FT_flash_address.Sector(FT_sector);
				p_test_context->m_page_number = 0;
				p_test_context->m_real_flash_address = FT_flash_address;
				p_test_context->m_num_pages = (U32)Get_Hex(EOL "Enter number of blocks to erase ");
				
				Tracef(EOL "Erasing %d blocks starting at flash address %X, page %X, array %X, column %X", 
					p_test_context->m_num_pages,
					FT_flash_address.Index(), FT_page, FT_array, FT_column);
				Erase_Blocks(p_test_context, OK);
					
				break;
			
			case 'fo':
			
				// Format flash file system.
				// Call callback routine with pointer to context when format has completed.
				if (Check_Flash_Handle() == 0)
				{
					get_command = 1;
					break;
				}
				Tracef(EOL "Format Flash...");
				Open_Flash();
				flash_config.erase_all_pages = 0;
				status = FF_Format(
					FT_flash_handle,
					&flash_config,
					p_test_context,
					&Format_Callback);
				break;
			
			case 'fs':
			
				Tracef(EOL "Flash Statistics");
				if (Check_Flash_Handle() == 0)
				{
					get_command = 1;
					break;
				}
				status = FF_Display_Stats_And_Events(FT_flash_handle);
				get_command = 1;
				break;
			
			case 'op':
			
				Tracef(EOL "Open Flash file system...");
				if (FT_flash_handle)
				{
					Tracef(EOL "Flash file system is already open");
					get_command = 1;
					break;
				}
				// Call Flash_Initialized when complete
				p_test_context->Set_Callback(&Flash_Initialized);
				
				// Initialize the flash file system.
			    status =  FF_Open(
					&flash_config,
				    &cache_config,
					p_test_context,
					&FT_flash_handle);
				break;
			
			case 'rp':
			
				Tracef(EOL "Read Page (specified by SB, SC, SP)");
				if (Check_Flash_Handle() == 0)
				{
					get_command = 1;
					break;
				}
				FT_flash_address.Initialize();
				FT_flash_address.Array(FT_array);
				FT_flash_address.Column(FT_column);
				FT_flash_address.Block(FT_block);
				FT_flash_address.Sector(FT_sector);
				Tracef(EOL "Reading flash address %X, page %X, array %X, column %X", 
					FT_flash_address.Index(), FT_page, FT_array, FT_column);
				status = FT_p_controller->Read_Page(p_test_context, FT_flash_address,
					p_read_buffer);
				break;
			
			case 'rs':
			
				Tracef(EOL "Read Sequential...");
				if (Check_Flash_Handle() == 0)
				{
					get_command = 1;
					break;
				}
				status = Read_Sequential(Flash_Address::Bytes_Per_Page(), num_blocks, starting_block, 
					p_page_buffer, p_test_context);
				break;
			
			case 'sa':
			
				Tracef(EOL "Set Array address ");
				array_address = Get_Hex_Number();
				if (array_address > 1)
					Tracef(EOL "Array address cannot be > 1");
				else
					FT_array = array_address;
				get_command = 1;
				break;
			
			case 'sc':
			
				Tracef(EOL "Set Column address ");
				column_address = Get_Hex_Number();
				if (column_address > 7)
					Tracef(EOL "Column address cannot be > 7");
				else
					FT_column = column_address;
				get_command = 1;
				break;
			
			case 'sf':
			
				Tracef(EOL "Set write buffer -- Fill with character ");
				fill_character = Get_Hex_Number();
				memset(p_write_buffer, fill_character, Flash_Address::Bytes_Per_Page());
				get_command = 1;
				break;
			
			case 'sp':
			
				Tracef(EOL "Set Page address ");
				FT_page = Get_Hex_Number();
				FT_block = FT_page / Flash_Address::Sectors_Per_Block();
				FT_sector = FT_page % Flash_Address::Sectors_Per_Block();
				get_command = 1;
				break;
			
			case 'st':
			
				Tracef(EOL "Surface Test -- erases all, including system structures");
				if (!Get_Yes("\nAre you sure you want to erase the bad block table? (Type Y if yes)"))
				{
					get_command = 1;
					break;
				}
				
				if (Check_Flash_Handle() == 0)
				{
					get_command = 1;
					break;
				}
				
				Open_Flash();
				
				// Secret command to erase all
				flash_config.erase_all_pages = 0XAAAAAAAA;
				
				// Call Surface_Test_Complete when complete
				p_test_context->Set_Callback(&Surface_Test_Complete);
		
				status = FF_Create(
					&flash_config,
				    &cache_config,
					p_test_context);
				break;
			
			case 'sv':
			
				Tracef(EOL "Set Verify buffer -- fill with character ");
				fill_character = Get_Hex_Number();
				memset(p_verify_buffer, fill_character, Flash_Address::Bytes_Per_Page());
				get_command = 1;
				break;
			
			case 'sw':
			
				Tracef(EOL "Set Write buffer -- fill with address pattern ");
				FT_flash_address.Initialize();
				FT_flash_address.Array(FT_array);
				FT_flash_address.Column(FT_column);
				FT_flash_address.Block(FT_block);
				FT_flash_address.Sector(FT_sector);
				{
					// Fill the buffer with a test pattern.
					UI64 *p_test_pattern = (UI64 *)p_write_buffer;
					for (U32 index = 0; index < Flash_Address::DWords_Per_Page(); index++)
					{
						UI64 test_pattern = 0x8000000000000000
							| ((UI64)FT_flash_address.Index() << 32)
							
							// Index should be in low order so we can see it
							// with the logic analyzer.
							| index;
						*(p_test_pattern + index) = test_pattern;
					}
				}
				get_command = 1;
				break;
			
			case 'td':
				Tracef(EOL "Test Unit (erases, write, reads specified units) ... ");
				if (Check_Flash_Handle() == 0)
				{
					get_command = 1;
					break;
				}
				Test_Unit(p_test_context);
				break;
			
			case 'tr':
				Tracef(EOL "Trace On ");
    			TraceLevel[TRACE_SSD] = Get_Hex("Enter trace level.");
				get_command = 1;
				break;
			
			case 'to':
				Tracef(EOL "Trace Off ");
    			TraceLevel[TRACE_SSD] = 0;
				get_command = 1;
				break;
			
			case 've':
			
				printf(EOL "Set Verify Erase level");
				verify_erase = Get_Hex_Number();
				flash_config.verify_erase = verify_erase;
				get_command = 1;
				break;
			
			case 'vp':
			
				Tracef(EOL "Verify Page (compare verify buffer to flash page)");
				if (Check_Flash_Handle() == 0)
				{
					get_command = 1;
					break;
				}
				FT_flash_address.Initialize();
				FT_flash_address.Array(FT_array);
				FT_flash_address.Column(FT_column);
				FT_flash_address.Block(FT_block);
				FT_flash_address.Sector(FT_sector);
				Tracef(EOL "Verifying flash address %X, page %X, array %X, column %X", 
					FT_flash_address.Index(), FT_page, FT_array, FT_column);
				status = FT_p_controller->Read_Page(p_test_context, FT_flash_address,
					p_verify_buffer, FF_VERIFY_READ);
				break;
			
			case 'vs':
			
				printf(EOL "Set Verify Structures level");
				printf(EOL "1 means verify all structures (things run slower)");
				printf(EOL "0 means don't verify structures (things run faster)");
				printf(EOL "Only has effect before open.");
				verify_structures = Get_Hex_Number();
				flash_config.verify_structures = verify_structures;
				get_command = 1;
				break;
			
			case 'vw':
			
				printf(EOL "Set Verify Write level");
				verify_write = Get_Hex_Number();
				flash_config.verify_write = verify_write;
				get_command = 1;
				break;
			
			case 'wa':
			{
			
				printf(EOL "Write Alternate");
				if (Check_Flash_Handle() == 0)
				{
					get_command = 1;
					break;
				}
				p_test_context->m_page_number = 0;
				//p_test_context->m_num_pages = (U32)Get_Hex(EOL "Enter number of times to write ");
				p_test_context->m_num_pages = 1;
				UI64 alternate_pattern_0 = Get_Hex(EOL "Enter 16 hex digit alternate pattern 1 ");
				UI64 alternate_pattern_1 = Get_Hex(EOL "Enter 16 hex digit alternate pattern 2 ");
				
				// Create a pattern in each buffer.
				for (U32 index = 0; index < Flash_Address::Bytes_Per_Page() / sizeof(UI64); index = index + 2)
				{
					*((UI64 *)p_write_buffer + index) = alternate_pattern_0;
					*((UI64 *)p_write_buffer + index + 1) = alternate_pattern_1;
					*((UI64 *)p_verify_buffer + index) = alternate_pattern_1;
					*((UI64 *)p_verify_buffer + index + 1) = alternate_pattern_0;
				}
	
				FT_flash_address.Initialize();
				FT_flash_address.Array(FT_array);
				FT_flash_address.Column(FT_column);
				FT_flash_address.Block(FT_block);
				FT_flash_address.Sector(FT_sector);
				Write_Alternate(p_test_context, OK);
				break;
			}
			
			case 'wp':
			
				Tracef(EOL "Write Page (specified by SB, SC, SP)");
				if (Check_Flash_Handle() == 0)
				{
					get_command = 1;
					break;
				}
				FT_flash_address.Initialize();
				FT_flash_address.Array(FT_array);
				FT_flash_address.Column(FT_column);
				FT_flash_address.Block(FT_block);
				FT_flash_address.Sector(FT_sector);
				if (!FT_p_page_map->Is_Page_Erased(FT_flash_address))
				{
					Tracef(EOL "Page is not erased");
					FT_p_page_map->Set_Page_Erased(FT_flash_address, 1);
				}
				Tracef(EOL "Writing flash address %X, page %X, array %X, column %X", 
					FT_flash_address.Index(), FT_page, FT_array, FT_column);
				status = FT_p_controller->Write_Page(p_test_context, FT_flash_address,
					p_write_buffer);
				break;
			
			case 'wr':
			
				Tracef(EOL "Write Page from read buffer");
				if (Check_Flash_Handle() == 0)
				{
					get_command = 1;
					break;
				}
				FT_flash_address.Initialize();
				FT_flash_address.Array(FT_array);
				FT_flash_address.Column(FT_column);
				FT_flash_address.Block(FT_block);
				FT_flash_address.Sector(FT_sector);
				if (!FT_p_page_map->Is_Page_Erased(FT_flash_address))
				{
					Tracef(EOL "Page is not erased");
					FT_p_page_map->Set_Page_Erased(FT_flash_address, 1);
				}
				Tracef(EOL "Writing flash address %X, page %X, array %X, column %X", 
					FT_flash_address.Index(), FT_page, FT_array, FT_column);
				status = FT_p_controller->Write_Page(p_test_context, FT_flash_address,
					p_read_buffer);
				break;
			
			case 'ws':
			
				Tracef(EOL "Write Sequential...");
				if (Check_Flash_Handle() == 0)
				{
					get_command = 1;
					break;
				}
				status = Write_Sequential(Flash_Address::Bytes_Per_Page(), num_blocks, starting_block, 
					p_page_buffer, p_test_context);
				break;
			
			case 'wv':
			
				Tracef(EOL "Write Verify -- compare to write buffer");
				if (Check_Flash_Handle() == 0)
				{
					get_command = 1;
					break;
				}
				FT_flash_address.Initialize();
				FT_flash_address.Array(FT_array);
				FT_flash_address.Column(FT_column);
				FT_flash_address.Block(FT_block);
				FT_flash_address.Sector(FT_sector);
				Tracef("\nVerifying flash address %X, page %X, array %X, column %X", 
					FT_flash_address.Index(), FT_page, FT_array, FT_column);
				status = FT_p_controller->Read_Page(p_test_context, FT_flash_address,
					p_write_buffer, FF_VERIFY_READ);
				break;
			
			default:
				Tracef("\nUnrecognized Command.\n");
				Show_Commands();
				get_command = 1;
				break;
		} // switch
				
		if (status != OK)
		{
			get_command = 1;
			Tracef(" ... failed, status = %X.\n", status);
			status = OK;
		}
	} // while
	
	// Return to scheduler.
	return;
		
} // Run_Test
	

/*************************************************************************/
// Erase_Blocks
/*************************************************************************/
void Erase_Blocks(void *p_context, STATUS status)
{
	FT_Test_Context *p_test_context = (FT_Test_Context *)p_context;
	
	// Have we finished?
	if (p_test_context->m_num_pages == p_test_context->m_page_number++)
	{
		Run_Test_Complete(p_context, OK);
		return;
	}
	
	p_test_context->Set_Callback(&Erase_Blocks);
	p_test_context->Set_Status(OK);

	status = FT_p_controller->Erase_Page_Block(p_test_context, p_test_context->m_real_flash_address);
	p_test_context->m_real_flash_address.Increment_Block();
		
} // Erase_Blocks
		
/*************************************************************************/
// Write_Alternate
/*************************************************************************/
void Write_Alternate(void *p_context, STATUS status)
{
	FT_Test_Context *p_test_context = (FT_Test_Context *)p_context;
	
	// Have we finished?
	if (p_test_context->m_num_pages == p_test_context->m_page_number++)
	{
		Run_Test_Complete(p_context, OK);
		return;
	}
	
	p_test_context->Set_Callback(&Write_Alternate);
	p_test_context->Set_Status(OK);

	void *p_buffer;
	
	if (p_test_context->m_page_number & 1)
		p_buffer = p_write_buffer;
	else
		p_buffer = p_verify_buffer;
		
	if (!FT_p_page_map->Is_Page_Erased(FT_flash_address))
	{
		FT_p_page_map->Set_Page_Erased(FT_flash_address, 1);
	}
	
	status = FT_p_controller->Write_Page(p_test_context, FT_flash_address,
		p_buffer);
		
} // Write_Alternate
		
/*************************************************************************/
// Bad_Block_Table_Complete
// Gets called by Scheduler when the bad block table has 
// been initialized.
// Schedule Run_Test to run again.
/*************************************************************************/
void Bad_Block_Table_Complete(void *p_context, STATUS status)
{
	FT_Test_Context *p_test_context = (FT_Test_Context *)p_context;
	
	p_test_context->Set_Callback(&Run_Test);
	p_test_context->Set_Status(OK);

	// Start the scheduler
	p_test_context->Make_Ready();
	
	if (status == OK)
	{
	 	TRACE_ENTRY(Bad_Block_Table_Complete);
		Tracef(" ... successful.\n");
		
		// We need to initialize.
		FT_initialized = 0;
 	}
 	else
 	{
 		TRACE_NUMBER(TRACE_L4, "\nTest Failed, status = ", status);
		Tracef(" ... failed, status = %X.\n", status);
		if (status == FF_ERROR_CODE(BAD_BLOCK_TABLE_ALREADY_EXISTS))
		{
			Tracef("Bad block table already exists.\n");
		}
 	}
 	
} // Bad_Block_Table_Complete
	
/*************************************************************************/
// Bad_Block_Table_Complete
// Gets called by Scheduler when the bad block table has 
// been initialized.
// Schedule Run_Test to run again.
/*************************************************************************/
void Surface_Test_Complete(void *p_context, STATUS status)
{
	FT_Test_Context *p_test_context = (FT_Test_Context *)p_context;
	
    // Zero handle because flash is not actually open.
    FT_flash_handle = 0;

	p_test_context->Set_Callback(&Run_Test);
	p_test_context->Set_Status(OK);

	// Start the scheduler
	p_test_context->Make_Ready();
	
	if (status == OK)
	{
	 	Tracef("\nSurface test successful");
		
		// We need to initialize.
		FT_initialized = 0;
 	}
 	else
 	{
		Tracef("\nSurface test ... failed, status = %X.\n", status);
		if (status == FF_ERROR_CODE(BAD_BLOCK_TABLE_ALREADY_EXISTS))
		{
			Tracef("Bad block table already exists.\n");
		}
 	}
} // Bad_Block_Table_Complete
	
/*************************************************************************/
// Flash_Initialized
// Gets called by Scheduler when the initialization has completed.
// Schedule Run_Test to run again.
/*************************************************************************/
void Flash_Initialized(void *p_context, STATUS status)
{
	FT_Test_Context *p_test_context = (FT_Test_Context *)p_context;
	
	p_test_context->Set_Callback(&Run_Test);
	p_test_context->Set_Status(OK);

	// Start the scheduler
	p_test_context->Make_Ready();
	
	// Don't try to initialize again if the bad block table is not initialized.
	FT_initialized = 1;

	if (status == OK)
	{
	 	TRACE_ENTRY(Flash_Initialized);
		Tracef(EOL "Flash initialization successful.\n");
 	}
 	else
 	{
 		TRACE_NUMBER(TRACE_L4, "\nTest Failed, status = ", status);
		Tracef("Flash initialization failed, status = %X.\n", status);
		if (status == FF_ERROR_CODE(BAD_BLOCK_TABLE_DOES_NOT_EXIST))
		{
			Tracef("Bad block table does not exist.\n");
			Tracef("Type CB to create bad block table.\n");
		}
 	}
	// Get pointer to controller object.
	FF_Get_Controller(FT_flash_handle, &FT_p_controller);
	    
	// Get pointer to page map object.
	FF_Get_Page_Map(FT_flash_handle, &FT_p_page_map);
	    
} // Flash_Initialized
	
/*************************************************************************/
// Run_Test_Complete
// Gets called by Scheduler when the test has completed.
// Schedule Run_Test to run again.
/*************************************************************************/
void Run_Test_Complete(void *p_context, STATUS status)
{
	if (status == OK)
	{
	 	TRACE_ENTRY(Run_Test_Complete);
		Tracef(" ... successful.\n");
 	}
 	else
 	{
 		TRACE_NUMBER(TRACE_L4, "\nTest Failed, status = ", status);
		Tracef(" ... failed, status = %X.\n", status);
		if (status == FF_ERROR_CODE(BAD_BLOCK_TABLE_DOES_NOT_EXIST))
		{
			Tracef("Bad block table does not exist.\n");
			Tracef("Type CB to create bad block table.\n");
		}
 	}
	FT_Test_Context *p_test_context = (FT_Test_Context *)p_context;
	
	p_test_context->Set_Callback(&Run_Test);
	p_test_context->Set_Status(OK);

	// Start the scheduler
	p_test_context->Make_Ready();
	
} // Run_Test_Complete
	
/*************************************************************************/
// 	Get_Command
// Get test command code as two lower-case characters.
/*************************************************************************/
U32 Get_Command()
{
	Tracef("\nEnter command ");
#if 1
	char command_code_0 = Get_Command_Char() | 0x20;
	char command_code_1 = Get_Command_Char() | 0x20;
#else
	char command_code_0 = 'b' | 0x20;
	char command_code_1 = 'a' | 0x20;
#endif
	
	return (command_code_1  | (command_code_0 << 8));
}

/*************************************************************************/
// 	Get_Hex_Number
// Get hex number.
/*************************************************************************/
U32 Get_Hex_Number()
{
	return (U32)Get_Hex("\nEnter hex number ");
}

/*************************************************************************/
// 	Get_Yes
// Get hex number.
/*************************************************************************/
U32 Get_Yes(char *p_message)
{
	Tracef(p_message);
	char next_char = Get_Command_Char();
	switch (next_char)
	{
		case 'y':
			return 1;
		case 'Y':
			return 1;
	}
	return 0;
}

/*************************************************************************/
// 	Get_Hex
// Get hex number.
/*************************************************************************/
UI64 Get_Hex(char *p_message)
{
	U32 invalid_digit = 1;
	UI64 number = 0;
	char next_char = 0;
	UI64 next_value;
	while(invalid_digit)
	{
		Tracef(p_message);
		invalid_digit = 0;
		while (invalid_digit == 0)
		{
			next_char = Get_Command_Char();
			switch (next_char)
			{
				case '\n':
					return number;
				case '\r':
					return number;
				case '0': next_value = 0; break;
				case '1': next_value = 1; break;
				case '2': next_value = 2; break;
				case '3': next_value = 3; break;
				case '4': next_value = 4; break;
				case '5': next_value = 5; break;
				case '6': next_value = 6; break;
				case '7': next_value = 7; break;
				case '8': next_value = 8; break;
				case '9': next_value = 9; break;
				case 'a': case 'A': next_value = 10; break;
				case 'b': case 'B': next_value = 11; break;
				case 'c': case 'C': next_value = 12; break;
				case 'd': case 'D': next_value = 13; break;
				case 'e': case 'E': next_value = 14; break;
				case 'f': case 'F': next_value = 15; break;
				default:
					invalid_digit = 1;
					Tracef("\nInvalid digit");
					break;
			}
			number = number * 16 + next_value;
		}
	}
	
	return number;
}

/*************************************************************************/
// 	Get_Command_Char
// Get test command code.
/*************************************************************************/
char Get_Command_Char()
{
	char key = 0;
	while(key == 0)
	{	
		// TEMPORARY -- kbhit is coded backwards!
		//if (!kbhit())
		if (kbhit())
        	key = getchar();
    }
    //	putchar(key); putchar does not flush
    // Echo is no done by getchar
    //Tracef("%c", key);
    return key;			
}

/*************************************************************************/
// 	Display_Read_Buffer
/*************************************************************************/
void Display_Read_Buffer()
{
	I64 next_dword;
	I64 *dword_buffer = (I64 *)p_read_buffer;
	U32 num_lines = Flash_Address::DWords_Per_Page() / 4;
	for (U32 line_index = 0; line_index < num_lines; line_index++)
	{
		printf("\n ");
		for (U32 word_index = 0; word_index < 4; word_index++)
		{
			next_dword = *(dword_buffer + (line_index * 4) + word_index);
			printf("  %LX", next_dword);
		}
	}
}

/*************************************************************************/
// 	Display_Write_Buffer
/*************************************************************************/
void Display_Write_Buffer()
{
	I64 next_dword;
	I64 *dword_buffer = (I64 *)p_write_buffer;
	for (U32 line_index = 0; line_index < Flash_Address::Bytes_Per_Page() / 32; line_index++)
	{
		printf("\n ");
		for (U32 word_index = 0; word_index < 4; word_index++)
		{
			next_dword = *(dword_buffer + (line_index * 4) + word_index);
			printf("  %LX", next_dword);
		}
	}
}

/*************************************************************************/
// 	Show_Commands
/*************************************************************************/
void Show_Commands()
{
	printf(EOL "ba  Basic Assurance Test");
	printf(EOL "br  Break (debugger)");
	printf(EOL "cb  Create bad block table");
	printf(EOL "cl  Close flash file system");
	printf(EOL "cs  Cache Statistics");
	printf(EOL "dr  Display Read buffer");
	printf(EOL "dw  Display Write buffer");
	printf(EOL "eb  Erase page Block (specified by SB, SC, SP)");
	printf(EOL "em  Erase Multiple page Blocks (specified by SB, SC, SP)");
	printf(EOL "fo  Format Flash file system");
	printf(EOL "fs  Flash Statistics");
	printf(EOL "op  Open flash file system");
	printf(EOL "rs  Read Sequential test");
	printf(EOL "rp  Read Page (specified by SB, SC, SP)");
	printf(EOL "sa  Set Array address ");
	printf(EOL "sb  Set Bank address");
	printf(EOL "sc  Set Column address");
	printf(EOL "sf  Set write buffer -- fill with Fill character ");
	printf(EOL "sp  Set Page address ");
	printf(EOL "sw  Set Write buffer -- fill with address pattern ");
	printf(EOL "st  Surface Test -- erases all, including system structures");
	printf(EOL "sv  Set Verify buffer -- fill with character ");
	printf(EOL "td  Test Unit (erases, write, reads specified units) ... ");
	printf(EOL "to  Trace off");
	printf(EOL "tr  Trace on");
	printf(EOL "ve  set Verify Erase level");
	printf(EOL "vp  Verify Page (compare verify buffer to flash page)");
	printf(EOL "vs  set Verify Structures level");
	printf(EOL "vw  set Verify Write level");
	printf(EOL "wa  Write Alternate -- write alternate patterns to same page without erasing");
	printf(EOL "wp  Write Page (specified by SB, SC, SP)");
	printf(EOL "wr  Write Page out of read buffer (allows doctoring pages with debugger)");
	printf(EOL "wv  Write Verify -- compare to write buffer");
	printf(EOL "ws  Write Sequential test...");
}

/*************************************************************************/
// 	Test_Unit
/*************************************************************************/
void Test_Unit(FT_Test_Context *p_test_context)
{
	U32 array_range_low = 0X10000;
	U32 array_range_high = 0X10000;
	U32 column_range_low = 0X10000;
	U32 column_range_high = 0X10000;
	U32 block_range_low = 0X10000;
	U32 block_range_high = 0X10000;

	Open_Flash();
	while(array_range_low > 7)
		array_range_low = Get_Hex(EOL "Enter hex array range low (0..1) ");
		
	while(array_range_high > 7)
		array_range_high = Get_Hex(EOL "Enter hex array range high (0..1) ");
		
	while(column_range_low > 7)
		column_range_low = Get_Hex(EOL "Enter hex column range low (0..7) ");
		
	while(column_range_high > 7)
		column_range_high = Get_Hex(EOL "Enter hex column range high (0..7) ");
		
	while(block_range_low > 0X3ff)
		block_range_low = Get_Hex(EOL "Enter hex block range low (0..3FF)");
		
	while(block_range_high > 0X3ff)
		block_range_high = Get_Hex(EOL "Enter hex block range high (0..3FF)");
	
	Status status = FF_Test_Device(
		FT_flash_handle,
		0,
		0,
		array_range_low,
		array_range_high,
		column_range_low,
		column_range_high,
		block_range_low,
		block_range_high,
		p_test_context);
		
} // Test_Unit

int Check_Flash_Handle()
{
	if (FT_flash_handle == 0)
	{
		printf(EOL "Flash is not Open (Use OP command)");
		return 0;
	}
	return 1;
}

/************************************************************************/
// Init_Config
/*************************************************************************/
void Init_Config(U32 memory_available)
{
	// Initialize flash config.
	memset(&flash_config, 0, sizeof(flash_config));
	flash_config.version = FF_CONFIG_VERSION;
	
	flash_config.verify_write = verify_write;
	flash_config.verify_erase = verify_erase;
	flash_config.verify_page_erased_before_write = 1;
	flash_config.verify_structures = verify_structures;

	flash_config.percentage_erased_pages = 1;
	flash_config.percentage_replacement_pages = 1;
	flash_config.replacement_page_threshold = 25;
	flash_config.erase_all_pages = 0;
	
	// Set pointer to device in flash configuration.
	flash_config.p_device = &flash_device;

	// Set flash config to point to memory allocated for it.
	flash_config.p_memory = FT_p_memory;
	flash_config.memory_size = memory_available;

	// Initialize cache config.
	cache_config.version = CM_CONFIG_VERSION;
	
	// The flash manager will set up the cache config memory from
	// its left over memory.
	cache_config.p_table_memory = 0;
	cache_config.p_page_memory = 0;
	cache_config.num_pages = 0;
	cache_config.num_pages_secondary = 0;

	// page_size is ignored by the Flash Storage;
	// The device page size is always used.
	cache_config.page_size = MAX_PAGE_SIZE;
	
	cache_config.page_table_size = 32768;
	cache_config.hash_table_size = 0;
	cache_config.num_reserve_pages = 64;
	cache_config.dirty_page_writeback_threshold = 60;
	cache_config.dirty_page_error_threshold = 95;
}

