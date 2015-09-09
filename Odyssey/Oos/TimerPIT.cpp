/* TimerPIT -- provides PIT service
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
//  7/07/98 Joe Altmaier: Create file

#include "TimerPIT.h"
#include "Critical.h"
#include "Interrupt.h"
#include "BuildSys.h"
#include "Odyssey_Trace.h"


	DEVICENAME(TimerPIT, TimerPIT::Initialize);	//*** THIS NEEDS TO BE REMOVED ***

	TimerPIT *TimerPIT::pTHead;
	I64 TimerPIT::timeRemaining;
	
	void TimerPIT::Initialize() {
		pTHead = NULL;
		timeRemaining=0;
		Interrupt::SetHandler(Interrupt::tyT0Exp, Interrupt, 0l);
		}

	// Construct Timer diabled
	TimerPIT::TimerPIT() : delta(0),repeat(0),fLinked(FALSE) {
	}
	
	// Deconstruct Timer
	TimerPIT::~TimerPIT() {
		Unlink();
	}
	
	// Link timer into active timer list
	// Timer Must Not be currently linked
	//
	void TimerPIT::Link() {
		// New timer has a certain delta.
		// Link into timer list at appropriate point.
		// If list empty, put at head and start timer.
		// If new timer has a delta smaller than current time remaining,
		//  reprogram PIT for a smaller interval, put this link block on front of list.
		// else link the timer in the list at the appropriate point.
		//
		// Note: each timer hasn't got a delta equal to the desired interval.
		// It has got the interval remaining after the previous timers
		//  have expired.
		// The timer on the head of the list is different.
		// Its delta is useless - the interval is running.
		// Always use TimeRemaining() for that interval.

		I64 deltaRemain;
		Critical section;

		fLinked = true;		

Log('Reli', delta);
		TimerPIT *pTLNext = pTHead;
		if (!pTHead) {	// Empty Timer List
Log('only', 0l);
			pTHead = this;
			this->pTNext = NULL;
			ProgramPIT(pTHead->delta);	// Start the PIT
		}
		else {			// Timer list not empty
			if (!IsRunning())	// PIT not running, must be relinking expired timer
				deltaRemain = pTHead->delta;
			else			// PIT is running
				deltaRemain = TimeRemaining();
			
			if (delta < deltaRemain) { // "this" should be first!
Log('head', deltaRemain);
				// Shorten current head's delta
				pTHead->delta = deltaRemain - this->delta;
				// Make "this" the head of the list
				pTNext = pTHead;
				pTNext->pTPrev = this;
				pTHead = this;
				// Program the PIT for this shorter interval
				ProgramPIT(pTHead->delta);
			}

			else {	// Link into timer list (not first)
Log('list', 0l);
				pTLNext->delta = deltaRemain;
				while (pTLNext && pTLNext->delta < this->delta) {
					this->delta -= pTLNext->delta;
					this->pTPrev = pTLNext;
					pTLNext = pTLNext->pTNext;
				}

				// Insert into list
				this->pTNext = pTLNext;
				this->pTPrev->pTNext = this;
				if (pTLNext) {
					pTLNext->pTPrev = this;
					// After this inserted delta expires, next delta is smaller
					pTLNext->delta -= this->delta;
				}
			}
		}
	}

	// Unlink timer and update PIT/Deltas
	//
	void TimerPIT::Unlink() {
		Critical section;

		if (fLinked) {
			if (pTHead == this) {
				pTHead = pTNext;
				if (pTHead) {
					pTHead->delta += TimeRemaining();
					ProgramPIT(pTHead->delta);
				}
				else
					DisablePIT();
			}
			else {
				pTPrev->pTNext = pTNext;
				if (pTNext) {
					pTNext->pTPrev = pTPrev;
					pTNext->delta += delta;
				}
			}
			fLinked = FALSE;
		}
	}


	// PIT expired, process PIT HISR
	//
	void TimerPIT::Interrupt(long ) {
		// We could have simultaneously had timer interrupt
		// asserted with some other interrupt.
		// If that other interrupt was processed first,
		// and it reprogrammed the timer, then this interrupt
		// may be unnecessary.
		// So first check if the timer is running.
		// This code also works for processing any remaining
		// time that wouldn't fit into the hardware counter,
		// since TimeRemaining() returns the sum of the
		// counter and the remainder.

		I64 timeRem_=TimeRemaining();

		if (timeRem_) {
			ProgramPIT(timeRem_);
			return;
			}

		// Time interval truly expired.
		DisablePIT();

Log('intr', (U32)pTHead);
		TimerPIT *pTLink = pTHead;
		if (pTLink) {
			Critical section;

			// Unlink timer
			pTHead = pTLink->pTNext;
			pTLink->fLinked = FALSE;

			if (pTLink->repeat) {
				pTLink->delta = pTLink->repeat;
				pTLink->Link();
			}
				
			if (pTHead && !IsRunning())
				ProgramPIT(pTHead->delta);

			pTLink->Expire();
		}
	}
