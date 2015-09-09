/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FfErrorTest.cpp
// 
// Description:
// This file implements error test methods for the Flash Storage. 
// 
// 08/23/99 Jim Frandeen: Create file
/*************************************************************************/

#include <string.h>
#include "FfMem.h"
#include "FlashStorage.h"


/*************************************************************************/
// FF_Error_Test globals.
/*************************************************************************/
FF_Error_Test::Test_Entry *FF_Error_Test::m_p_test_entry;
unsigned FF_Error_Test::m_next;
unsigned FF_Error_Test::m_seed;

/*************************************************************************/
// Allocate memory for object.
/*************************************************************************/
Status FF_Error_Test::Allocate()
{
	m_p_test_entry = (Test_Entry *)new char[sizeof(Test_Entry) * FF_Error_Test_Last];
	if (m_p_test_entry == 0)
		return FF_ERROR_NO_MEMORY;

	ZERO(m_p_test_entry, sizeof(Test_Entry) * FF_Error_Test_Last);
	return OK;
}

/*************************************************************************/
// Test_Error 
// condition specifies the value of a condition that has been tested.
// If condition is already true, return true.
// If conditon is false, return true some of the time, depending on
// the number of times the test has been executed, and the frequency
// of error conditions desired.
/*************************************************************************/
int FF_Error_Test::Test_Error(int condition, FF_Error_Test_Name test_name)
{
	// Has table been initialized?
	if (m_p_test_entry == 0)
		return condition;

	// Point to test entry in table of test entries.
	Test_Entry *p_test_entry = m_p_test_entry + test_name;

	// Increment the execution counter.
	p_test_entry->execution_counter++;

	// Is the condition already true?
	if (condition)
		return true;

	// The condition is not true, so look to see if it's time to
	// simulate a true value.

	// Is this a static counter?
	if (p_test_entry->frequency_counter)
	{
		// Yes, it is a frequency counter.
		// Decrement the counter by one
		if (--p_test_entry->frequency_counter == 0)
		{
			// The counter has gone to zero.
			// Reload the counter from the frequency value.
			p_test_entry->frequency_counter = p_test_entry->frequency_value;

			// Increment the number of times we returned true.
			p_test_entry->true_counter++;

			// Return true;
			return 1;
		}

		// The counter has not gone to zero.  Return false.
		return 0;
	}

	// Is this a random counter?
	if (p_test_entry->frequency_value)
	{
		// This is a random test.
		// Generate a random number between 0 and the frequency value.
		unsigned random = Random_Max(p_test_entry->frequency_value);

		// If the value is zero, we return true.
		if (random == 0)
		{
			// Increment the number of times we returned true.
			p_test_entry->true_counter++;

			// Return true;
			return 1;
		}
	}

	return 0;

} // FF_Error_Test


/*************************************************************************/
// Set_Config_Options 
// Set up to test acccording to parameters specified in the
// configuration file.
/*************************************************************************/
Status FF_Error_Test::Set_Config_Options(FF_CONFIG *p_config)
{
	if (p_config->test_all_random)
		Set_Random(p_config->test_all_random);

	else if (p_config->test_all_static)
		Set_Static(p_config->test_all_static);

	// Test write errors?
	if (p_config->write_error_frequency_value)
	{
		if (p_config->test_all_random)
			Set_Random(p_config->write_error_frequency_value, WRITE_ERROR);
		else
			Set_Static(p_config->write_error_frequency_value, WRITE_ERROR);
	}

	// Test erase errors?
	if (p_config->erase_error_frequency_value)
	{
		if (p_config->test_all_random)
			Set_Random(p_config->erase_error_frequency_value, ERASE_ERROR);
		else
			Set_Static(p_config->erase_error_frequency_value, ERASE_ERROR);
	}

	// Test read errors?
	if (p_config->read_error_frequency_value)
	{
		if (p_config->test_all_random)
			Set_Random(p_config->read_error_frequency_value, READ_ECC_ERROR);
		else
			Set_Static(p_config->read_error_frequency_value, READ_ECC_ERROR);
	}

	return OK;

} // Set_Config_Options

