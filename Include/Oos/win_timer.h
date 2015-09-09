//win_timer.h

#ifndef __win_timer_H
#define __win_timer_H

#include "stdlib.h"
#include "time.h"
#include "OsTypes.h"

struct win_timer_struct {
	clock_t m_tLastExpire;
	BOOL m_bEnabled;
	unsigned int m_iTimeWait;
	UNSIGNED m_iID;
	VOID (*expiration_routine)(UNSIGNED);
};

void win_timer_thread(void * pParam);

class win_timer {
public:
	win_timer();
	~win_timer();

	void Run();

	int ResetTimer(int iTimerID, BOOL bEnabled, unsigned int iTimeWait, VOID (*expiration_routine)(UNSIGNED));
	int CreateTimer(BOOL bEnabled, unsigned int iTimeWait, VOID (*expiration_routine)(UNSIGNED), UNSIGNED id);
	int ControlTimer(int iTimerID, BOOL bEnabled);
	int DeleteTimer(int iTimerID);

	void Sleep(UNSIGNED milli);

	clock_t m_tClock;

	CT_Semaphore m_Semaphore;
	BOOL m_bInitialized;
	int m_iTimerCount;
	win_timer_struct** m_apTimers;
};

#endif