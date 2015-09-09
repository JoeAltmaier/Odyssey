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
// 9/29/98 Jim Frandeen: Use CtTypes.h instead of stddef.h
/*************************************************************************/
#ifndef __Interrupt_h
#define __Interrupt_h

#include "OsTypes.h"

#define DEBUG_INT

#ifdef _ODYSSEY

#define GALILEO_INTERRUPT 0
#else
#define GALILEO_INTERRUPT 3
#endif


class Interrupt {
public:
	enum TyInt {
		tyFirst=0, 
		tyMemOut, tyDmaOutl, tyCpuOut, 
		tyDma0Comp, tyDma1Comp, tyDma2Comp, tyDma3Comp, 
		tyT0Exp, tyT1Exp, tyT2Exp, tyT3Exp, 
		tyMassRdErr0, tySlvWrErr0, tyMasWrErr0, tySlvRdErr0, 
		tyAddrErr0, tyMemErr, tyMasAbort0, tyTarAbort0, 
		tyRetryCtr0, 
		tyCpuInt0, tyCpuInt1, tyCpuInt2, tyCpuInt3, tyCpuInt4, 
		tyPciIntA, tyPciIntB, tyPciIntC, tyPciIntD, 
		tyCpuIntSum, tyPciIntSum, 
		tyLast, 
		
		tyI2oFirst=32, 
		tyI2oInMsg0=32, tyI2oInMsg1, tyI2oInDoor, tyI2oX1, 
		tyI2oInPost, tyI2oOutFreeOvr, 
		tyI2oLast, 
		MAXHANDLER
		};
	
	typedef void (*IntHandler)(long);

#ifdef DEBUG_INT
	static
	long aN[MAXHANDLER];
	static volatile
	long nInt;
#endif	

private:
	static
	IntHandler aHandler[MAXHANDLER];
	static
	long aL[MAXHANDLER];

	static
	NU_HISR hisrInt;
	static
	char stack[8192 * 8];
	static
	void	(*old_lisr)(int);

	static
	long lCauseGal;
	static
	long lCauseI2o;

	
public:
	static
	void Initialize();
	static
	void Reset();

	static
	void SetHandler(TyInt ty, IntHandler intHandler, long l);
	static
	void ClearHandler(TyInt ty);
	static
	void Dispatch(TyInt ty);
	static
	BOOL TestInterrupt(TyInt ty);
	static
	void ClearInterrupt(TyInt ty);

	static
	void Int_Handler();
	static
	void _Int_Handler(int vector_number);
	};
	
#endif


