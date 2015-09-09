/* Semaphore.h -- Encapsulates Kernel Semaphores
 *
 * Copyright (C) ConvergeNet Technologies, 1999
 *
 * This material is a confidential trade secret and proprietary 
 * information of ConvergeNet Technologies, Inc. which may not be 
 * reproduced, used, sold or transferred to any third party without the 
 * prior written consent of ConvergeNet Technologies, Inc.  This material 
 * is also copyrighted as an unpublished work under sections 104 and 408 
 * of Title 17 of the United States Code.  Law prohibits unauthorized 
 * use, copying or reproduction.
 *
 * Description:
 * 		This class encapsulates the underlying Kernel semaphores.
 *
**/

// Revision History: 
// 3/15/99 Tom Nelson: Created
//

#ifndef __Semaphore_h
#define __Semaphore_h

#include "Kernel.h"
#include "Critical.h"

//***
//*** Semaphore (Standard)
//***

class Semaphore {
private:
	CT_Semaphore ctSemaphore;

public:
	Semaphore(UNSIGNED cInitial = 0) { 
		
		Kernel::Create_Semaphore(&ctSemaphore, "Semaphr", cInitial);
	}
	~Semaphore() {
		Kernel::Delete_Semaphore(&ctSemaphore);
	}

	STATUS Wait() { 
		return Kernel::Obtain_Semaphore(&ctSemaphore,CT_SUSPEND);
	}
	STATUS Signal() { 
		return Kernel::Release_Semaphore(&ctSemaphore);
	}	
};

//***
//*** Semaphore (Mutex)
//***

class Mutex {
private:
	Semaphore semaphore;

public:
	Mutex() : semaphore(1) {}
	
	STATUS Enter() { 
		return semaphore.Wait();
	}
	STATUS Leave() { 
		return semaphore.Signal();
	}
};

//***
//*** Semaphore (Pulse)
//***

class Pulse {
private:
	U32 nWait;
	Semaphore semaphore;

public:
	Pulse() : semaphore(0) {}
	
	// Wait for event
	STATUS Wait() { 
		Critical section;
		++nWait;
		semaphore.Wait();
		return OK;
	}
	// Signal only if task is waiting
	STATUS Signal() { 
		Critical section;
		if (nWait > 0) {
			--nWait;
			semaphore.Signal();
		}
		return OK;
	}
};

#endif	// __Semaphore_h
