#include "CriticalSection.h"

extern "C" long long gCCritical;

//	NU_PROTECT CriticalSection::protect;

//		NU_Unprotect();
// Dec nest counter, if 0 Load StatusReg ($12), set IntEnable bit (1), store SR, return.
	asm void CriticalSection::Leave() { 
		ld  t0, gCCritical;
		addiu t0, t0, -1;
		beqz  t0, unmask;
		sd  t0, gCCritical;
		jr ra;
		nop;
unmask:
		mfc0 t0,$12;
		nop;
		ori t0,t0,0x0001;
		mtc0 t0,$12;
		nop;
		jr ra;
		nop;
		}


//		*(long long *)&protect=0l; NU_Protect(&protect);
// Load StatusReg ($12), clear IntEnable bit (1), store SR, inc nest counter, return.
	asm void CriticalSection::Enter() {
		mfc0 t0,$12;
		nop;
		lui t1, 0xffff;
		ori t1, t1, 0xfffe;
		and $8, $8,$9;
		mtc0 t0,$12;
		ld  t0, gCCritical;
		addiu t0, t0, 1;
		jr ra;
		sd  t0, gCCritical;
		}
