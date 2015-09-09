/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: CodingConventions.c
// 
// Description:
// This file describes coding conventions to be used in the Odyssey project.
// In general, creates two files for each module: a .h file and a .c file.
// 
// Update Log: 
// 4/14/98 Jim Frandeen: Create file
// 5/7/98 '*', should be with the variable name 
// 6/2/98 Jim Frandeen: Change order to legaleese, description, update log
// 9/15/98 Jim Frandeen: Nucleus and I2O naming conventions.
// 10/16/98 Jim Frandeen: Base types
/*************************************************************************/

//    Include header files
//    Do not put path names in the include directive 
#include "CTTypes.h"

/*************************************************************************/
//	Base Types
/*************************************************************************/
// Use the following base types.  These types are all available from
// CTTypes.h

S8		s8;		// 8-bit signed integer, range -128 to 127	
U8		u8;		// 8-bit unsigned ingetger, range 0 to 255		
BOOL	b;		// 8-bit boolean value, 0 or 1
S16		s16;	// 16-bit signed integer, range -32,768 to 32,767
U16		u16;	// 16-bit unsigned integer, range 0 to 65,535
S32		s32;	// 32-bit signed integer, range -2,147,483,648 to 2,147,483,647
U32		u32;	// 32-bit unsigned integer, range 0 to 4,294,967,295
I64		i64;	// 64-bit signed integer, 
				// range -9,223,372,036,854,775,808 to 9,223,372,036,854,775,807
/*************************************************************************/
//	Naming Conventions
/*************************************************************************/
//  There are two naming conventions in use.
//  1.  The Nucleus naming convention is preferred.
//      Variables use all lowercase.
//      Words and phrases are separated by the underscore character:
//          new_priority
//      Method names use uppercase for the first letter of each word or phrase.
//          NU_Change_Priority
//
//  2.  The I2O naming convention
//	    In general, the first letter is uppercase.  All other letters are 
//	    lowercase except for first letter of subsequent words.
//
//  Whichever naming convention you choose, be consistent.
//
//	Name objects using nouns and noun phrases.

//	If an object is used as a counter, begin its name with "num"
    int num_apples;
    int num_errors;

//	If a variable is a pointer, begin the name with "p".
//	The "pointer" qualifier, '*', should be with the variable name 
//	rather than with the type. 
//  Consider the following declaration:
    int *p_num_apples, p_num_errors;
//  This declares p_num_apples as a pointer to int, and p_num_errors as an int.
//  The correct declaration would be:
    int *p_num_apples, *p_num_errors;

//	Think of the '*' in front of a variable name as short for 'pointer to'
    int *p_num_errors;

//	Method names should be in the form of ObjectVerb
    void Message_Frame_Free();
    void Message_Frame_Send();

//	If an object stores a boolean value, name it in the form of a question
//	that yields a boolean answer.
    BOOL is_enabled;
    BOOL needs_updating;

//	Names defined by the typedef directive should be all all caps
typedef    char  U8;

//	When you want to define names with constant integer values, 
//	use enum rather than `#define'. 
//	Names defined by emum should be all all caps.
//	If you need explicit values, assign each initiator value explicitly.
typedef enum
    {
    I2O_MEM_ACCESS_PRIVATE			= 0,
    I2O_MEM_ACCESS_SYSTEM			= 1,
    I2O_MEM_ACCESS_LOCAL_ADAPTERS	= 2,
    I2O_MEM_ACCESS_ALL_ADAPTERS		= 3,
    I2O_MEM_ACCESS_BUS_SPECIFIC		= 4
    } I2O_MEM_ACCESS_ATTR;

//	Names defined by the #define directive should be all all caps.
#define    I2O_FUNCTION_SZ        8

//	struct definitions follow the I2O convention.
typedef struct _I2O_MESSAGE_FRAME {
   U8                          VersionOffset;
   U8                          MsgFlags;
   U16                         MessageSize;
   BF                          TargetAddress:I2O_TID_SZ;
   BF                          InitiatorAddress:I2O_TID_SZ;
   BF                          Function:I2O_FUNCTION_SZ;
   I2O_INITIATOR_CONTEXT       InitiatorContext;
} I2O_MESSAGE_FRAME, *PI2O_MESSAGE_FRAME;
//	Names with leading and trailing underscores are reserved for system use.


/*************************************************************************/
//    Forward References
//    Put forward references to methods at the top in alphabetical order.
//    This makes it clear what methods are implemented in this module.
//    This also permits methods to arranged in alphabetical order, which 
//    makes them easy to find. 
/*************************************************************************/
int Method_Name_A();
char *Method_Name_Z();

/*************************************************************************/
//    Comments
/*************************************************************************/

//	A block comment like this describes the code immediately following it.
//	Put one blank line before a block comment.
//	There are no blank lines after the block comment and the code
//	it describes.
    foo();

//	Do not use comments to avoid compilation.  Instead, use #if 0 #endif.
#if 0
#endif // end of commented out section 

/*************************************************************************/
//	Method_Name_A
//	Each method should begin with a description of what it does.
/*************************************************************************/
int Method_Name_A()
{ // opening brace must begin in column 0 

    // The following indentation style is used 

    if (x < foo(y, z))
        haha = bar[4] + 5;
    else {
        while (z) {
            haha = foo(z, z);
            z = z - 1;
        }
    }
} // ending brace must begin in column 0 

/*************************************************************************/
//	Spaces
/*************************************************************************/

//	Follow every keyword with a space, tab, or newline.
    if (x)
    while (z)

//	Do not write a space after the opening parenthesis and before the
//	closing parenthesis of an expression.
    if (a < b)

//	Write the opening parenthesis, "(" of a method immediately to the
//	right of the method's name (i.e., without any intervening spaces).
    Foo();

//	Write a space or a newline after every comma or semicolon.
    for (index = 1; index < 10; index = index + 1)

//	Write a space around every assignment operator.
//	Do not write a space between unary opertors and their operands.
    flags = !message_flags;

//	Do not write any spaces around the "." and "->" operators.
    flags = p_message_frame->message_flags;

//	Write a space around every other operator.
    num_errors = num_errors + 1;

/*************************************************************************/
//	Tabs
/*************************************************************************/

//	Do not store tabs in source files.  Convert them to spaces instead.

//	Indent each tab stop 4 spaces.

