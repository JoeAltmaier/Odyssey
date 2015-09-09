/* Kernel.h -- Wrapper Class for underlying Kernel (Nucleus)
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
// 4/21/99 Josh Hatwich: Added win32 support

#ifndef __Kernel_h
#define __Kernel_h

#ifdef _WIN32
	#include "win_kernel.h"
	
#else
#include "Nucleus.h"	// This is the only file in Oos that should include this
#include "OsTypes.h"


// Tasks
typedef NU_TASK CT_Task;
typedef VOID (*CT_Task_Entry)(VOID *);

// Semaphones
typedef NU_SEMAPHORE CT_Semaphore;
#define CT_SUSPEND		NU_SUSPEND
#define CT_NO_SUSPEND	NU_NO_SUSPEND

// Timers
typedef NU_TIMER CT_Timer;
typedef VOID (*CT_Timer_Callback)(UNSIGNED);
#define CT_ENABLE_TIMER		NU_ENABLE_TIMER
#define CT_DISABLE_TIMER	NU_DISABLE_TIMER
#define CT_USEC_MULT		10000		// Tick multiplier for usecs
#define TIMESTAMP I64

class Kernel {
	static void Task_Entry_Nucleus(UNSIGNED, VOID*);
	
public:
	//**
	//** Tasks
	//**
	static STATUS Create_Thread(
		CT_Task &_task,
		char *_stName,
		CT_Task_Entry _task_entry,
		void *_argv,
		void *_pStack,
		U32 _sStack) {
		
		union {
			CT_Task_Entry task_entry;
			UNSIGNED u;
			} _te;
		
		_te.task_entry=_task_entry;
		
		return NU_Create_Task(&_task, _stName, Task_Entry_Nucleus, 
					_te.u, _argv,
					_pStack, _sStack,
                    20, 0,// priority, timeslice
                    NU_PREEMPT, NU_NO_START);
	}
	// Suspend a thread
	static void Suspend_Thread(CT_Task &_task) { NU_Suspend_Task(&_task); }

	// Schedule a thread
	static void Schedule_Thread(CT_Task &_task) { NU_Resume_Task(&_task); }
	
	static CT_Task *Current_Thread_Pointer() { return NU_Current_Task_Pointer(); }
	
	// Reschedule this thread
	static void Reschedule() { NU_Relinquish(); }

	//**
	//** Semaphores
	//**
	static STATUS Create_Semaphore(CT_Semaphore *pSemaphore, char *psName,
						UNSIGNED initial_count) {
		return NU_Create_Semaphore(pSemaphore,psName,initial_count,NU_FIFO);
	}
	static STATUS Delete_Semaphore(CT_Semaphore *pSemaphore) {
		return NU_Delete_Semaphore(pSemaphore);
	}
	// suspend = CT_SUSPEND or CT_NO_SUSPEND
	static STATUS Obtain_Semaphore(CT_Semaphore *pSemaphore, UNSIGNED suspend) {
		return NU_Obtain_Semaphore(pSemaphore,suspend);
	}
	static STATUS Release_Semaphore(CT_Semaphore *pSemaphore) {
		return NU_Release_Semaphore(pSemaphore);
	}
	static STATUS Reset_Semaphore(CT_Semaphore *pSemaphore,UNSIGNED initial_count) {
		return NU_Reset_Semaphore(pSemaphore,initial_count);
	}
	//**
	//** Timers
	//**
	static STATUS Create_Timer(CT_Timer *pTimer, CHAR *pName, 
                        VOID (*expiration_routine)(UNSIGNED), UNSIGNED id,
                        UNSIGNED initial_time, UNSIGNED reschedule_time,
                        OPTION enable) {
		
		return NU_Create_Timer(pTimer, pName, 
                        expiration_routine, id,
                        initial_time, reschedule_time,
                        enable);
	}
	static STATUS Control_Timer(CT_Timer *pTimer, OPTION enable) {
		return NU_Control_Timer(pTimer,enable);
	}
	// Timer must be disabled before it can be deleted.
	static STATUS Delete_Timer(CT_Timer *pTimer) {
		return NU_Delete_Timer(pTimer);
	}
	static STATUS Reset_Timer(CT_Timer *pTimer, 
                        VOID (*expiration_routine)(UNSIGNED),
                        UNSIGNED initial_time, UNSIGNED reschedule_time,
                        OPTION enable) {
		return NU_Reset_Timer(pTimer,expiration_routine,initial_time,reschedule_time,enable);
	}
	static TIMESTAMP Time_Stamp() {
		return NU_Retrieve_Clock() * (I64)CT_USEC_MULT; /* usec per 10ms */
	}

	static void Delay(U32 cMs) {
		NU_Sleep(cMs / 10); // 10 ms per tick
		}

	static void Stop_Scheduler();
};

#if !defined(__ghs__)  // Green Hills
// Fine resolution timer macro
#define TIMESTART(start) asm {mfc0 $8, $9; sw $8, start;}
#define TIMEDIFF(start, total) asm {mfc0 $8, $9; lw $9, start; subu $8, $8,$9; lw $9, total; addu $8, $8, $9; sw $8,total;}
#endif

#endif	// WIN32
#endif


