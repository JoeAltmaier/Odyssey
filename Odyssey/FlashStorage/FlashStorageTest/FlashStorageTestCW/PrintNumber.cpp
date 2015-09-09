/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: PrintNumber.cpp
//
// Description:
//
//
// Update Log:
// 1/20/99 Jim Frandeen: Create file.
/*************************************************************************/

/* Include necessary Nucleus PLUS files.  */

#include  "nucleus.h"
#include  "tc_defs.h"
/*************************************************************************/
// Define prototypes for function references.
/*************************************************************************/
extern  "C" {                               /* C declarations in C++     */
char*   Convert_Number (char *st, unsigned int number, int base, int length, char fPad);
void Print_String (char *string);
}                                           /* End of C declarations     */
void Print_Number(char *p_string, UNSIGNED number);

/*************************************************************************/
// Print_Number
/*************************************************************************/
void Print_Number(char *p_string, UNSIGNED number)
{
	char	string[20];
	
	Print_String(p_string);
    Convert_Number(string, number, 16, 8, '0');
    Print_String(string);
	
} // Print_Number

