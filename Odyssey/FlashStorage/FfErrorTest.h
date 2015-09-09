/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FfErrorInjection.h
// 
// Description:
// This file implements error test methods for the Flash Storage. 
// 
// 08/23/99 Jim Frandeen: Create file
/*************************************************************************/

#ifndef Error_Test_H
#define Error_Test_H

#include <stdlib.h>
#include "ErrorLog.h"
#include "FlashStatus.h"

class FF_CONFIG;

/*************************************************************************/
// FF_Error_Test_Name
/*************************************************************************/
typedef enum
{
	// Add error test names here
	READ_ECC_ERROR,
	WRITE_ERROR,
	ERASE_ERROR,
	FF_Error_Test_Last
} FF_Error_Test_Name;

/*************************************************************************/
// FF_Error_Injection
/*************************************************************************/
class FF_Error_Injection
{
public: // methods

	// constructor
	FF_Error_Injection();

	// Allocate memory for static object.
	Status Allocate();

	// Free memory for static object.
	Status Free();

	int Test_Error(int condition, FF_Error_Test_Name test_name);

	// Set all test counters for random.
	void Set_Random(unsigned frequency_value);

	// Set specific test for random.
	void Set_Random(unsigned frequency_value, FF_Error_Test_Name test_name);

	// Set all test counters for static.
	void Set_Static(unsigned frequency_value);

	// Set specific test for static.
	void Set_Static(unsigned frequency_counter, FF_Error_Test_Name test_name);

	// Set up to test acccording to parameters specified in the
	// configuration file.
	Status Set_Config_Options(FF_CONFIG *p_config);

	unsigned Get_Execution_Count(FF_Error_Test_Name test_name);
	unsigned Get_Error_Count(FF_Error_Test_Name test_name);
	unsigned Get_Frequency_Value(FF_Error_Test_Name test_name);
	int Is_Random(FF_Error_Test_Name test_name);

private: // helper methods

	static unsigned Random(void);
	static unsigned Random_Max (unsigned max_value);


private: // member data

	typedef struct _Test_Entry
	{
		unsigned execution_counter;
		unsigned error_injection_counter;
		unsigned frequency_value;
		unsigned frequency_counter;
	} Test_Entry;

	static Test_Entry *m_p_test_entry;
	static unsigned m_next;
	static unsigned m_seed;

}; // FF_Error_Injection


/*************************************************************************/
// FF_ERROR_TEST macro
// If debugging, call FF_Error_Injection; otherwise simply execute the test.
/*************************************************************************/
#ifdef _DEBUG
extern FF_Error_Injection flash_error_test;
#define IF_FF_ERROR(condition, test_name) \
	if (flash_error_test.Test_Error(condition, test_name))	
#else
#define IF_FF_ERROR(condition, test_name) \
	if (condition)
#endif

/*************************************************************************/
// FF_Error_Injection constructor.
/*************************************************************************/
inline FF_Error_Injection::FF_Error_Injection()
{
	// Seed the random number.
	// TODO -- set seed to timer or ??
	m_seed = 1103515245;

	m_p_test_entry = 0;
}

/*************************************************************************/
// Set_Random sets all entries to return true at random times
// between 0 and frequency_value.
// Random entries do not have a frequency_counter.
/*************************************************************************/
inline void FF_Error_Injection::Set_Random(unsigned frequency_value) 
{
	for (unsigned index = 0; index < FF_Error_Test_Last; index++)
		(m_p_test_entry + index)->frequency_value = frequency_value;
}

/*************************************************************************/
// Set_Random sets a specific test to return true at random times
// between 0 and frequency_value.
/*************************************************************************/
inline void FF_Error_Injection::Set_Random(unsigned frequency_value, FF_Error_Test_Name test_name) 
{
	(m_p_test_entry + test_name)->frequency_value = frequency_value;
}

/*************************************************************************/
// Set_Static sets all entries to return true every frequency_counter
// time the test is executed.
/*************************************************************************/
inline void FF_Error_Injection::Set_Static(unsigned frequency_value) 
{
	for (unsigned index = 0; index < FF_Error_Test_Last; index++)
	{
		(m_p_test_entry + index)->frequency_counter = frequency_value;
		(m_p_test_entry + index)->frequency_value = frequency_value;
	}
}

/*************************************************************************/
// Set_Static sets a specific test to return true every frequency_counter
// time the test is executed.
/*************************************************************************/
inline void FF_Error_Injection::Set_Static(unsigned frequency_counter, FF_Error_Test_Name test_name) 
{
	(m_p_test_entry + test_name)->frequency_counter = frequency_counter;
}

/*************************************************************************/
// Get_Execution_Count returns the execution count.
/*************************************************************************/
inline unsigned FF_Error_Injection::Get_Execution_Count(FF_Error_Test_Name test_name) 
{
	return (m_p_test_entry + test_name)->execution_counter;
}

/*************************************************************************/
// Get_Error_Count returns the error injection count.
/*************************************************************************/
inline unsigned FF_Error_Injection::Get_Error_Count(FF_Error_Test_Name test_name) 
{
	return (m_p_test_entry + test_name)->error_injection_counter;
}

/*************************************************************************/
// Is_Random returns true if this entry is a random counter.
/*************************************************************************/
inline int FF_Error_Injection::Is_Random(FF_Error_Test_Name test_name) 
{
	if (m_p_test_entry + test_name)->frequency_counter
	
		// Random entries do not have a frequency counter.
		return 1;
	return 0;
}

/*************************************************************************/
// Get_Frequency_Value returns the frequency value.
/*************************************************************************/
inline unsigned FF_Error_Injection::Get_Frequency_Value(FF_Error_Test_Name test_name) 
{
	return (m_p_test_entry + test_name)->frequency_value;
}

/*************************************************************************/
// Random_Max returns a random number between 0 and max_value inclusive.
/*************************************************************************/
inline unsigned FF_Error_Injection::Random_Max (unsigned max_value) 
{
   return(((2 * Random() * max_value + RAND_MAX) /
      RAND_MAX - 1) / 2);
}

/*************************************************************************/
// Random returns a pseudorandom number.
/*************************************************************************/
inline unsigned FF_Error_Injection::Random(void)
{
	m_next = m_next * 1103515245 + 12345;
	return((m_next >> 16) & 0x7FFF);
}


#endif // Error_Test_H
