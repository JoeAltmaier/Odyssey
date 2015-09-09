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
// This class implements interrupt dispatching for the Galileo chip.
// 
// Update Log: 
// 7/14/98 Joe Altmaier: Create file
/*************************************************************************/

#define _TRACEF
#include "Trace_Index.h"
#include "Odyssey_Trace.h"

#include <StdLib.h>
#include "Galileo.h"
#include "Critical.h"
#include "Interrupt.h"
#include "BuildSys.h"

	DEVICENAME(Interrupt, Interrupt::Initialize);
	FAILNAME(Interrupt, Interrupt::Reset);
		
	Interrupt::IntHandler Interrupt::aHandler[MAXHANDLER];
	long Interrupt::aL[MAXHANDLER];
#ifdef DEBUG_INT
	long Interrupt::aN[MAXHANDLER];
	volatile
	long Interrupt::nInt;
#endif	

	NU_HISR Interrupt::hisrInt;
	char Interrupt::stack[8192 * 8];
	void	(*Interrupt::old_lisr)(int);

	long Interrupt::lCauseGal;
	long Interrupt::lCauseI2o;
	
	void Interrupt::Initialize() {
		for (int i=0; i < MAXHANDLER; i++) {
			aHandler[i]=NULL;
			aL[i]=0l;
#ifdef DEBUG_INT
			aN[i]=0l;
#endif
			}
		
		Reset();

		NU_Register_LISR(GALILEO_INTERRUPT, _Int_Handler, &old_lisr);
		NU_Create_HISR(&hisrInt, "Int_Galileo", Int_Handler, 12, stack, sizeof(stack));
		}

	void Interrupt::Reset() {
		// Mask all interrupts
		*(unsigned long*)GT_I2O_INTIN_MASK_REG = GTREG(0x00000037l);
		*(unsigned long*)GT_I2O_INTOUT_MASK_REG = GTREG(0x0000000Fl);
		*(unsigned long*)(GT_INT_CPU_MASK_REG) = GTREG(0x00000000l);
		*(unsigned long*)(GT_INT_PCI_MASK_REG) = GTREG(0x00000000l);

		// Clear all interrupts
		*(unsigned long*)(GT_INT_CAUSE_REG) = GTREG(0xC3E00001l);
		*(unsigned long*)(GT_I2O_INTIN_CAUSE_REG)	= GTREG(0xFFFFFFFCl);
		}

	void Interrupt::_Int_Handler(int /*vector_number*/) {
		// Hardware level interrupt routine
#ifdef DEBUG_INT
		nInt++;
#endif
		// Test Galileo interrupt cause reg.
		lCauseGal |= GTREG(*(unsigned long*)(GT_INT_CAUSE_REG)) & 0x3FFFFFFEl;

		// Clear interrupts
		*(unsigned long*)(GT_INT_CAUSE_REG) = GTREG(~lCauseGal);

		// Test I2O interrupt cause reg.
		long lMask=GTREG(*(unsigned long*)GT_I2O_INTIN_MASK_REG);
		lCauseI2o = ~lMask & GTREG(*(unsigned long*)GT_I2O_INTIN_CAUSE_REG);

		// Clear I2O interrupts
		unsigned long lClear=(lCauseI2o & 0x30) | 0xFFFFFFCFl;
		*(unsigned long*)(GT_I2O_INTIN_CAUSE_REG)	= lClear;
		lClear=~(lCauseI2o & 3);
		*(unsigned long*)(GT_I2O_INTIN_CAUSE_REG)	= lClear;

		// Dispatch interrupt routines from the HISR process.
		NU_Activate_HISR(&hisrInt);
		}

	void Interrupt::Int_Handler() {
		// HISR level interrupt routine
		Critical section;

		// Hold the critical section from cause test to return.
		while (lCauseGal | lCauseI2o) {
			long lCauseG=lCauseGal;
			lCauseGal &= !lCauseG;
			long lCauseI=lCauseI2o;
			lCauseI2o &= ~lCauseI;

			section.Leave();
			// Dispatch any pending galileo interrupts
			int iTy=tyFirst;
			while (lCauseG) {
				if (lCauseG & 1)
					Dispatch((TyInt)iTy);
				iTy++;
				lCauseG >>= 1;
				}
			
			iTy=tyI2oFirst;
			while (lCauseI) {
				if (lCauseI & 1)
					Dispatch((TyInt)iTy);
				iTy++;
				lCauseI >>= 1;
				}
			section.Enter();
			}
		}

	void Interrupt::Dispatch(TyInt ty) {
#ifdef DEBUG_INT
		aN[ty]++;
#endif
		IntHandler intHandler=aHandler[ty];
		if (intHandler != NULL)
			intHandler(aL[ty]);
		}

	void Interrupt::SetHandler(TyInt ty, IntHandler intHandler, long l) {
		Critical section;
		aHandler[ty]=intHandler;
		aL[ty]=l;
		
		// Enable interrupt
		if (ty >= tyFirst && ty < tyLast) {
			long lBit=1;
			for (int i=ty; i > tyFirst; i--)
				lBit <<= 1;
			*(unsigned long*)(GT_INT_CPU_MASK_REG) = GTREG(GTREG(*(unsigned long*)(GT_INT_CPU_MASK_REG)) | lBit);
			}
		if (ty >= tyI2oFirst && ty < tyI2oLast) {
			long lBit=1;
			for (int i=ty; i > tyI2oFirst; i--)
				lBit <<= 1;
			*(unsigned long*)(GT_I2O_INTIN_MASK_REG) = GTREG(GTREG( *(unsigned long*)(GT_I2O_INTIN_MASK_REG)) & ~lBit);
			 }
		}
		
	void Interrupt::ClearHandler(TyInt ty) {
		Critical section;
		aHandler[ty]=0;

		// Disable interrupt
		if (ty >= tyFirst && ty < tyLast) {
			long lBit=1;
			for (int i=ty; i > tyFirst; i--)
				lBit <<= 1;
			*(unsigned long*)(GT_INT_CPU_MASK_REG) = GTREG(GTREG(*(unsigned long*)(GT_INT_CPU_MASK_REG)) & ~lBit);
			}
		if (ty >= tyI2oFirst && ty < tyI2oLast) {
			long lBit=1;
			for (int i=ty; i > tyI2oFirst; i--)
				lBit <<= 1;
			*(unsigned long*)(GT_I2O_INTIN_MASK_REG) = GTREG(GTREG( *(unsigned long*)(GT_I2O_INTIN_MASK_REG)) | lBit);
			 }
		}

	BOOL Interrupt::TestInterrupt(TyInt ty) {
		Critical section;
		if (ty >= tyFirst && ty < tyLast) {
			U32 lCauseGal = GTREG(*(unsigned long*)(GT_INT_CAUSE_REG)) & 0x3FFFFFFEl;
			return ((lCauseGal & ((U32)1 << (int)(ty-tyFirst))) != 0);
			}
		if (ty >= tyI2oFirst && ty < tyI2oLast) {
			U32 lCauseI2o = GTREG(*(unsigned long*)GT_I2O_INTIN_CAUSE_REG);
			return ((lCauseI2o & ((U32)1 << (int)(ty - tyI2oFirst))) != 0);
			}

		return false;
		}
		
	void Interrupt::ClearInterrupt(TyInt ty) {
		if (ty >= tyFirst && ty < tyLast)
			*(unsigned long*)(GT_INT_CAUSE_REG) = GTREG(~((U32)1 << (int)(ty-tyFirst)));

		if (ty >= tyI2oFirst && ty < tyI2oLast) {
			U32 lCause=((U32)1 << (int)(ty - tyI2oFirst));
			U32 lClear=(lCause & 0x30) | 0xFFFFFFCFl;
			*(unsigned long*)(GT_I2O_INTIN_CAUSE_REG)	= lClear;
			lClear=~(lCause & 3);
			*(unsigned long*)(GT_I2O_INTIN_CAUSE_REG)	= lClear;
			}
		}
		