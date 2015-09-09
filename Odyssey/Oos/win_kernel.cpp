//win_kernel.cpp

#include "win_kernel.h"
#include "windows.h"
#include "Odyssey_Trace.h"
#include "stdio.h"

win_timer Kernel::m_win_timer;

STATUS Kernel::Create_Thread(
						CT_Task &_task,
						char *_stName,
						CT_Task_Entry _task_entry,
						void *_argv,
						void *_pStack,
						U32 _sStack) {
	_task = _beginthread(_task_entry, _sStack, _argv);
	return _task;
}

// Suspend a thread
void Kernel::Suspend_Thread(CT_Task &_task) { 
	//NU_Suspend_Task(&_task); 
}

// Schedule a thread
void Kernel::Schedule_Thread(CT_Task &_task) { 
	//NU_Resume_Task(&_task); 
}
	
CT_Task* Kernel::Current_Thread_Pointer() {
	return NULL;//NU_Current_Task_Pointer();
}
	
// Reschedule this thread
void Kernel::Reschedule() { 
	//NU_Relinquish(); 
}

void Kernel::Sleep(UNSIGNED milli) {
	m_win_timer.Sleep(milli);
}			   

//** Semaphores
int iSemaphoreUniqueName = 0;

STATUS Kernel::Create_Semaphore(CT_Semaphore *pSemaphore, char *psName, UNSIGNED initial_count) {
	char sName[256];
	sprintf(sName, "%s%d",psName,iSemaphoreUniqueName++);
	*pSemaphore = CreateSemaphore(NULL, initial_count, SEMAPHORE_MAX, sName);
	return 0;
}

STATUS Kernel::Delete_Semaphore(CT_Semaphore *pSemaphore) {
	return CloseHandle(*pSemaphore);
}

// suspend = CT_SUSPEND or CT_NO_SUSPEND
STATUS Kernel::Obtain_Semaphore(CT_Semaphore *pSemaphore, UNSIGNED suspend) {
	STATUS ret;
	if(suspend == CT_SUSPEND) {
		ret = WaitForSingleObject(*pSemaphore,INFINITE);
		switch(ret) {
		case WAIT_ABANDONED:
			Tracef("Wait abandoned on semaphore %u\n", *pSemaphore);
			break;
		case WAIT_OBJECT_0:
			break;
		case WAIT_TIMEOUT:
			Tracef("Wait timed out on semaphore %u\n", *pSemaphore);
			break;
		case WAIT_FAILED:
			Tracef("Wait failed on semaphore %u\n", *pSemaphore);
			break;
		}
	}
	else {
		ret = WaitForSingleObject(*pSemaphore, 0);
	}

	return ret;
}

STATUS Kernel::Release_Semaphore(CT_Semaphore *pSemaphore) {
	long lPrev;

	STATUS ret = ReleaseSemaphore(*pSemaphore, 1, &lPrev);

	if(ret == 0)
		Tracef("Release Semaphore %u failed\n",*pSemaphore);
	return ret;
}

STATUS Kernel::Reset_Semaphore(CT_Semaphore *pSemaphore,UNSIGNED initial_count) {
	return 0;//???????
	//return NU_Reset_Semaphore(pSemaphore,initial_count);
}

//** Timers
STATUS Kernel::Create_Timer(CT_Timer *pTimer, CHAR *pName, 
                        VOID (*expiration_routine)(UNSIGNED), UNSIGNED id,
                        UNSIGNED initial_time, UNSIGNED reschedule_time,
                        OPTION enable) {
	*pTimer = m_win_timer.CreateTimer(enable, reschedule_time, expiration_routine, id);
	return 1;
	//return NU_Create_Timer(pTimer, pName, expiration_routine, id, initial_time, reschedule_time, enable);
}

STATUS Kernel::Control_Timer(CT_Timer *pTimer, OPTION enable) {
	return m_win_timer.ControlTimer(*pTimer, enable);
	//return NU_Control_Timer(pTimer,enable);
}

// Timer must be disabled before it can be deleted.
STATUS Kernel::Delete_Timer(CT_Timer *pTimer) {
	return m_win_timer.DeleteTimer(*pTimer);
	//return NU_Delete_Timer(pTimer);
}

STATUS Kernel::Reset_Timer(CT_Timer *pTimer, 
                        VOID (*expiration_routine)(UNSIGNED),
                        UNSIGNED initial_time, UNSIGNED reschedule_time,
                        OPTION enable) {
	return m_win_timer.ResetTimer(*pTimer, enable, reschedule_time, expiration_routine);
	//return NU_Reset_Timer(pTimer,expiration_routine,initial_time,reschedule_time,enable);
}
I64 Kernel::Time_Stamp() {
	return 0;
	//return NU_Retrieve_Clock() * (I64)CT_USEC_MULT; /* usec per 10ms */
}


