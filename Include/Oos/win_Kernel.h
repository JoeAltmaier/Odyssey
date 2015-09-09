/* win_Kernel.h -- Wrapper Class for underlying Kernel (Windows)
 *
 * Copyright (C) ConvergeNet Technologies, 1998,99
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
 * 		This class encapsulates the features of the underlying Kernel.
 *
**/

// Revision History: 
// 2/11/99 Joe Altmaier: Create file
// 3/04/99 Tom Nelson:   Added Create_Timer()
// 3/15/99 Tom Nelson:	 Added Current_Thread_Pointer().
// 4/20/99 Josh Hatwich:  Made it windowsy

#ifndef __win_Kernel_h
#define __win_Kernel_h

#include "process.h"
#include "OsTypes.h"
#include "win_timer.h"

#define TIMESTAMP I64

class Kernel {
public:

	//** Tasks
	static STATUS Create_Thread(
		CT_Task &_task,
		char *_stName,
		CT_Task_Entry _task_entry,
		void *_argv,
		void *_pStack,
		U32 _sStack);

	static void Sleep(UNSIGNED milli);

	// Suspend a thread
	static void Suspend_Thread(CT_Task &_task);

	// Schedule a thread
	static void Schedule_Thread(CT_Task &_task);
	
	static CT_Task *Current_Thread_Pointer();
	
	// Reschedule this thread
	static void Reschedule();

	//**
	//** Semaphores
	//**
	static STATUS Create_Semaphore(CT_Semaphore *pSemaphore, char *psName,
						UNSIGNED initial_count);

	static STATUS Delete_Semaphore(CT_Semaphore *pSemaphore);

	// suspend = CT_SUSPEND or CT_NO_SUSPEND
	static STATUS Obtain_Semaphore(CT_Semaphore *pSemaphore, UNSIGNED suspend);

	static STATUS Release_Semaphore(CT_Semaphore *pSemaphore);

	static STATUS Reset_Semaphore(CT_Semaphore *pSemaphore,UNSIGNED initial_count);

	//** Timers
	static STATUS Create_Timer(CT_Timer *pTimer, CHAR *pName, 
                        VOID (*expiration_routine)(UNSIGNED), UNSIGNED id,
                        UNSIGNED initial_time, UNSIGNED reschedule_time,
                        OPTION enable);

	static STATUS Control_Timer(CT_Timer *pTimer, OPTION enable);

	// Timer must be disabled before it can be deleted.
	static STATUS Delete_Timer(CT_Timer *pTimer);

	static STATUS Reset_Timer(CT_Timer *pTimer, 
                        VOID (*expiration_routine)(UNSIGNED),
                        UNSIGNED initial_time, UNSIGNED reschedule_time,
                        OPTION enable);
	static I64 Time_Stamp();

	static win_timer m_win_timer;

};

#endif