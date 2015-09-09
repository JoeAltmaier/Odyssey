 /*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// Description:
// This module declares the standard Nucleus entry point.
// 
// Update Log: 
// 4/21/99 Joe Altmaier: Make cpp.  Use kernel.h.  Schedule the task.
/************************************************************************/

#include "Kernel.h"

#include "Application_Initialize.h"

CT_Task         task_0;
char *stack[4096];

/* Define the Application_Initialize routine that determines the initial
   Nucleus PLUS application environment.  */

void Application_Initialize(void * /*pArg - first_available_memory*/) {
    Kernel::Create_Thread(task_0, "TASK 0", StartTask, NULL, stack, sizeof(stack));
    Kernel::Schedule_Thread(task_0);
}
