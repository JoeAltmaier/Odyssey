/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: FormatErrorStats.cpp
// 
// Description:
// This file formats Flash Storage error injection statistics for display.
// 
// 8/31/99 Jim Frandeen: Create file
/*************************************************************************/

#include "FlashStorage.h"
#include <stdio.h>
#include <string.h>
 
union Num64 {
	U64 u64;
	I64 i64;
} ;


#define EOL "\n"
#define HEADER1 "                                           Number"
#define HEADER2 "                         Number            Errors       Injection      Injection"
#define HEADER3 "Error Test Name          Executions       Injected         Type        Frequency"
#define RANDOM  "         Random"
#define STATIC  "         Static"

#define TEST_NAME_SIZE (sizeof("Name                ") - 1)


/*************************************************************************/
// Externals from FormatCachStats
/*************************************************************************/
void Insert_Commas(char *buffer);
void Format(char *buffer, U32 number);
void Format(char *buffer, U64 number);

/*************************************************************************/
// FORMAT_NUMBER
// Macro to format any parameter
/*************************************************************************/
#define FORMAT_NUMBER(number) \
	buffer[0] = 0; \
	Format(buffer, (U32)number); \
	strcat(string, buffer);  

/*************************************************************************/
// Format_Name
/*************************************************************************/
void Format_Name(char *string, char *test_name)
{
	strcat(string, test_name);
	unsigned test_name_length = strlen(test_name);
	for (unsigned count = test_name_length; count < TEST_NAME_SIZE; count++)
		strcat(string, " ");
}

/*************************************************************************/
// Format_Test
/*************************************************************************/
void Format_Test(char *string, FF_Error_Test_Name test_name, 
				 FF_Error_Injection *p_error_test)
{
	char buffer [256];
	char separator[3] = "  "; // 2 spaces

	unsigned number_executions = p_error_test->Get_Execution_Count(test_name);
	FORMAT_NUMBER(number_executions);

	unsigned number_errors = p_error_test->Get_Error_Count(test_name);
	FORMAT_NUMBER(number_errors);

	if (p_error_test->Is_Random(test_name))
		strcat(string, RANDOM);
	else
		strcat(string, STATIC);

	unsigned frequency = p_error_test->Get_Frequency_Value(test_name);
	FORMAT_NUMBER(frequency);

}

/*************************************************************************/
// FORMAT_ERROR
// Macro to format an error
/*************************************************************************/
#define FORMAT_ERROR(test_name) \
	Format_Name(string, #test_name); \
	Format_Test(string, test_name, p_error_test); \
	strcat(string, EOL);


/*************************************************************************/
// Format_Error_Stats
/*************************************************************************/
void Format_Error_Stats(char *string, FF_Error_Injection *p_error_test)
{

	char separator[3] = "  "; // 2 spaces
	*string = 0;

	// Move heading to string.
	strcat(string, HEADER1);
	strcat(string, EOL);
	strcat(string, HEADER2);
	strcat(string, EOL);
	strcat(string, HEADER3);
	strcat(string, EOL);

	FORMAT_ERROR(READ_ECC_ERROR);
	FORMAT_ERROR(WRITE_ERROR);
	FORMAT_ERROR(ERASE_ERROR);


}  // Format_Error_Stats

	

