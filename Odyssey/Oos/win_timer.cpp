//win_timer.cpp

#include "win_timer.h"
#include "Kernel.h"
#include "windows.h"

win_timer::win_timer() {
	m_bInitialized = FALSE;
	m_iTimerCount = 0;
	m_apTimers = NULL;
	m_Semaphore = 0;
}

win_timer::~win_timer() {
	if(m_apTimers) {
		for(int i=0;i<m_iTimerCount;i++) {
			delete m_apTimers[i];
		}
		delete[] m_apTimers;
	}
}

void win_timer::Sleep(UNSIGNED milli) {
	::Sleep(milli);
}

void win_timer::Run() {
	while(1) {

		//take semaphore
		Kernel::Obtain_Semaphore(&m_Semaphore, CT_SUSPEND);

		int iSleepTime = -1;

		for(int i=0;i<m_iTimerCount;i++) {
			if((unsigned)(clock() - m_apTimers[i]->m_tLastExpire) >= m_apTimers[i]->m_iTimeWait) {
				m_apTimers[i]->expiration_routine(m_apTimers[i]->m_iID);
				m_apTimers[i]->m_tLastExpire = clock();
			}
			else {
				if(iSleepTime == -1) {
					iSleepTime = m_apTimers[i]->m_iTimeWait - (clock() - m_apTimers[i]->m_tLastExpire);
				}
				else {
					iSleepTime = min((unsigned)iSleepTime,m_apTimers[i]->m_iTimeWait - (clock() - m_apTimers[i]->m_tLastExpire));
				}
			}
		}

		//release semaphore
		Kernel::Release_Semaphore(&m_Semaphore);

		if(iSleepTime == -1) 
			Sleep(1000);
		else
			Sleep(iSleepTime);
	}
}

int win_timer::ResetTimer(int iTimerID, BOOL bEnabled, unsigned int iTimeWait, VOID (*expiration_routine)(UNSIGNED)) {

	//take semaphore
	Kernel::Obtain_Semaphore(&m_Semaphore, CT_SUSPEND);

	if(m_iTimerCount > iTimerID) {
		if(m_apTimers[iTimerID]) {
			m_apTimers[iTimerID]->m_bEnabled = bEnabled;
			m_apTimers[iTimerID]->m_iTimeWait = iTimeWait/1000;
			m_apTimers[iTimerID]->expiration_routine = expiration_routine;
			m_apTimers[iTimerID]->m_tLastExpire = clock();
		}
	}

	//release semaphore
	Kernel::Release_Semaphore(&m_Semaphore);

	return 0;
}

int win_timer::CreateTimer(BOOL bEnabled, unsigned int iTimeWait, VOID (*expiration_routine)(UNSIGNED), UNSIGNED id) {
	if(!m_bInitialized) {
		Kernel::Create_Semaphore(&m_Semaphore, "timer_semaphore", 1);
		_beginthread(win_timer_thread, 0, this);
	}

	//take semaphore
	Kernel::Obtain_Semaphore(&m_Semaphore, CT_SUSPEND);

	win_timer_struct** apTimers = new win_timer_struct*[m_iTimerCount + 1];
	if(m_apTimers) {
		memcpy(apTimers, m_apTimers, m_iTimerCount);
		delete[] m_apTimers;
	}
	m_apTimers = apTimers;
	m_apTimers[m_iTimerCount] = new win_timer_struct;

	//populate the timer struct
	m_apTimers[m_iTimerCount]->m_bEnabled = TRUE;
	m_apTimers[m_iTimerCount]->m_iTimeWait = iTimeWait/1000;
	m_apTimers[m_iTimerCount]->expiration_routine = expiration_routine;
	m_apTimers[m_iTimerCount]->m_iID = id;
	m_apTimers[m_iTimerCount]->m_tLastExpire = clock();

	//release semaphore
	Kernel::Release_Semaphore(&m_Semaphore);

	return m_iTimerCount++;
}

int win_timer::ControlTimer(int iTimerID, BOOL bEnabled) {
	int iRet = 0;

	//take semaphore
	Kernel::Obtain_Semaphore(&m_Semaphore, CT_SUSPEND);

	if(iTimerID < m_iTimerCount) {
		if(m_apTimers[iTimerID]) {
			m_apTimers[iTimerID]->m_bEnabled = bEnabled;
			iRet = 1;
		}
	}

	//release semaphore
	Kernel::Release_Semaphore(&m_Semaphore);

	return iRet;
}

int win_timer::DeleteTimer(int iTimerID) {

	//take semaphore
	Kernel::Obtain_Semaphore(&m_Semaphore, CT_SUSPEND);

	if(iTimerID < m_iTimerCount) {
		if(m_apTimers[iTimerID])
			delete m_apTimers[iTimerID];
		m_apTimers[iTimerID] = NULL;
	}

	//release semaphore
	Kernel::Release_Semaphore(&m_Semaphore);

	return 0;
}

void win_timer_thread(void * pParam) {
	win_timer* pWT = (win_timer*) pParam;
	pWT->Run();
}