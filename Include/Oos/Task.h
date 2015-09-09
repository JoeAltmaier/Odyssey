/* Task.h -- Encapsulates Kernel Tasks (Public)
 *
 * Copyright (C) ConvergeNet Technologies, 1999
 *
 * This material is a confidential trade secret and proprietary 
 * information of ConvergeNet Technologies, Inc. which may not be 
 * reproduced, used, sold or transferred to any third party without the 
 * prior written consent of ConvergeNet Technologies, Inc.  This material 
 * is also copyrighted as an unpublished work under sections 104 and 408 
 * of Title 17 of the United States Code.  Law prohibits unauthorized 
 * use, copying or reproduction.
 *
 * Description:
 * 		This class encapsulates the underlying Kernel Tasks
 *
**/

// Revision History: 
//  7/24/98 Joe Altmaier: Create file
//  9/29/98 Jim Frandeen: Use CtTypes.h instead of stddef.h
//  2/10/99 Joe Altmaier: Static Reschedule.
//  3/23/99 Tom Nelson:   Remove references to private ClassTable.h


#ifndef __Task_h
#define __Task_h

// Public Includes
#include "OsTypes.h"
#include "Kernel.h"

class Task {
	CT_Task ctTask;
public:
	char *stName;

public:
	Task(char *_stName, 				// The name of the task.
		 unsigned int _cbStack, 		// stack size.
		 CT_Task_Entry _task_entry,		// Task entry point.
		 void *_argv					// Parameters passed to entry point. 
		) : stName(_stName) {

		// Allocate the stack for the new task.
		void *_pStack=(void*)(new char[_cbStack]);
		
		// Create the new task.
		Kernel::Create_Thread(ctTask,
						_stName,
						_task_entry, 
						_argv,
						_pStack, _cbStack);
		}

	void Suspend() { 
		Kernel::Suspend_Thread(ctTask);
	}
		
	void Resume() { 
		Kernel::Schedule_Thread(ctTask);
	}

	static void Reschedule() {
		Kernel::Reschedule();
	}
};

#endif // __Task_h
