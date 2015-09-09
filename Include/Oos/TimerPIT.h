/* Timer -- provides PIT service
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
**/
 
// Revision History: 
//  6/12/98 Joe Altmaier: Create file

#ifndef __TimerPIT_h
#define __TimerPIT_h

#include "Kernel.h"
#include "Galileo.h"
#include "Interrupt.h"

#define TICKSPERUSEC 75
// Stupid compiler!!!  Won't do unsigned constants!  Would rather say 0xFF000000
#define MAXINTERVAL (0x7FFFFFFFL)

	class TimerPIT {
		// All timers are linked into a single list
		static
		TimerPIT *pTHead;
		static
		I64 timeRemaining;
		
		// Individual timer fields		
		TimerPIT *pTNext;
		TimerPIT *pTPrev;
		I64 delta;
		I64 repeat;
		BOOL fLinked;

		
		static
		U32 DisablePIT() {
			U32 ctl=*(U32*)(GT_TIMER_CONTROL_REG);
			ctl = GTREG(ctl) & ~(U32)(GT_TCREG_ENTC0 /*| GT_TCREG_SELTC0*/);
			*(U32*)(GT_TIMER_CONTROL_REG) = GTREG(ctl);
			// Clear Galileo interrupt for PIT 0 expire
//			U32 mask = ~(1l << Interrupt::tyT0Exp);
//			*(U32*)(GT_INT_CAUSE_REG) = GTREG(mask);
			return ctl;
		}

		static
		void EnablePIT(U32 ctl) {
			ctl |= (GT_TCREG_ENTC0 /*| GT_TCREG_SELTC0*/);
			*(U32*)(GT_TIMER_CONTROL_REG) = GTREG(ctl);
		}

		static
		void ProgramPIT(I64 _interval) {
//Log('sPIT', _interval);
			// Program the timer for the interval in usec
			U32 ctl = DisablePIT();
			
			// Max hardware count is 32 bits
			if (_interval >= MAXINTERVAL) {
				timeRemaining = _interval - MAXINTERVAL;
				_interval = MAXINTERVAL;
				}
			else
				timeRemaining = 0;

			// Set PIT counter
			*(U32*)(GT_TIMER0_REG) = GTREG(_interval);
			EnablePIT(ctl);
		}

		// Timer management methods		
		static
		BOOL IsRunning() {
			U32 ctl=*(U32*)(GT_TIMER_CONTROL_REG);
			ctl = GTREG(ctl);
			return (ctl & GT_TCREG_ENTC0) != 0;
			}

		static
		U32 TimeRemaining() {
			if (IsRunning()) {
				U32 count=*(U32*)(GT_TIMER0_REG);
				return GTREG(count) + timeRemaining;
				}

			return 0;
			};
		
		static
		void Interrupt(long );
		
	protected:
		// Derived class access to base class
		void Link();
		void Unlink();
		
		TimerPIT();
		~TimerPIT();
		
		// Base class access to derived class
		virtual void Expire()=0;

	public:
		void Start(I64 _interval,I64 _repeat) {
			Unlink();
			delta = _interval * TICKSPERUSEC;
			repeat = _repeat * TICKSPERUSEC;
			Link();
		}

		void Stop() {
			Unlink();
		}

		static
		void Initialize();
	};

#endif