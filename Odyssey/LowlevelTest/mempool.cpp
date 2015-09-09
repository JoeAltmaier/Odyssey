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
// Allocates big chunks of mem from Oos
// 
// Update Log: 
// Created : 10/4/99 - Sudhir
/*************************************************************************/


#include "BuildSys.h"

	
extern "C" void *GetSystemPool(int size);
extern "C" void *GetPciPool(int size);

void *GetSystemPool(int size)
{
	void *mPool;
    
	mPool = (void *) new(tBIG|tUNCACHED) U8[size];
	return(mPool);
}

void *GetPciPool(int size)
{
	void *mPool;

	mPool = (void *) new(tBIG|tPCI|tUNCACHED) U8[size];
	return(mPool);
}
