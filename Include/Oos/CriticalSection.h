/* CriticalSection.h
 *
 * Copyright (c) ConvergeNet Technologies (c) 1998 
 *
**/

#ifndef _CriticalSection_h
#define _CriticalSection_h

class CriticalSection {

//	static
//	NU_PROTECT protect;
public:
#ifdef _WIN32
	static void Enter() {}
	static void Leave() {}

#else	// MIPS
	static
	void Enter();

	
	static
	void Leave();
#endif	
	};

#endif