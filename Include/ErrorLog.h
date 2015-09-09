/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: ErrorLog.h
// 
// Description:
// This file defines an error logging procedure that can be called
// by any module.  
// 
// Update Log 
// 9/10/98 Jim Frandeen: Create file
// 10/14/98 Jim Frandeen: New base types
/*************************************************************************/
#if !defined(ErrorLog_H)
#define ErrorLog_H

#include "Simple.h"

#ifdef          __cplusplus
extern  "C" {                               /* C declarations in C++     */
#endif

#define EOL "\n"

/*************************************************************************/
// CT_ERROR_TYPE used in CT_Log_Error
/*************************************************************************/
typedef enum 
{
	CT_ERROR_TYPE_FATAL,
	CT_ERROR_TYPE_INFORMATION
} CT_ERROR_TYPE;

void CT_Log_Error(CT_ERROR_TYPE error_type,
	char* p_module_name, 
	char* p_error_description, 
	S32 status, 
	U32 data);

/*************************************************************************/
//    CT_ASSERT Macro Definition
//    TODO include trace here
/*************************************************************************/
#if defined(_DEBUG)
#define CT_ASSERT(condition, module) \
	if (!(condition)) \
		CT_Log_Error(CT_ERROR_TYPE_FATAL, \
			#module, \
			"Assert (" #condition ") Failed", \
			0, \
			0);
#else

// If debugging is not turned on, no code is generated
#define CT_ASSERT(condition, module)
#endif // #ifdef (_DEBUG)
		

#ifdef          __cplusplus
}                                           /* End of C declarations     */
#endif

#endif /* ErrorLog_H  */
