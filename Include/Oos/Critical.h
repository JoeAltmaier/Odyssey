/* Critical.h -- Critical Section (nestable)
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
//  3/26/99 Tom Nelson: Create file


#ifndef _Critical_h
#define _Critical_h

#include "OsTypes.h"

#ifdef WIN32
#include "windows.h"
#else
//extern "C" U32 TCD_R7KInterrupt_Level;
//extern "C" void Critical_Disable();
//extern "C" void Critical_Enable();
#endif

typedef U32 CRITICAL;	// Critical Section Flags

#ifdef  __cplusplus

class Critical {
	
public:

#ifdef _WIN32
	static void initialize_critical();
#endif

	Critical(BOOL fEnter = TRUE) : fSaved(FALSE) { if (fEnter) Enter(); }
	~Critical()	 { Leave(); }
	
	void Enter() { 
		U32 fEnable=0;
		if (!fSaved) { 
			#ifdef WIN32
			fEnabled = IsEnabled();
			InternalDisable();

			#else // Metrowerks
			asm {
				mfc0 t0,$12;
				nop
				andi t0,t0,1
				sw	t0, fEnable;
				}
			fEnabled=(BOOL)fEnable;
			if (fEnabled)
				asm {
					// Disable base interrupts using global interrupt mask bit
					mfc0 t0,$12;
					nop;
					xori t0, t0,1;
					mtc0 t0,$12;
					nop;
					}
			#endif
			fSaved = TRUE;
		}
	}
		
	void Leave() {
		if (fSaved) {
			if (fEnabled)	
				#ifdef WIN32
				InternalEnable();
				#else
				asm {
					mfc0 t0,$12;
					nop;
					ori t0,t0,1;
					mtc0 t0,$12;
					nop;
					}
				#endif

			fSaved = FALSE;
		}
	}

	
	BOOL fSaved;
#ifndef WIN32
	BOOL fEnabled;
#else
	static BOOL fEnabled;
#endif

#ifdef WIN32
	void InternalDisable();
	void InternalEnable();
#endif

	static BOOL IsEnabled();
	static void Enable();
	static void Disable();
};


extern  "C" {                               /* C declarations in C++     */
#endif

	CRITICAL CriticalEnter();
	void CriticalLeave(CRITICAL flags);
	
#ifdef  __cplusplus
}
#endif // cplusplus
	
#endif	// _Critical_h
