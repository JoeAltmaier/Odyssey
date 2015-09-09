/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
// File: Align.h
// 
// Description:
// This file contains macros to check for alignment.
// 
// Update Log 
// 
// 2/3/99 Jim Frandeen: Create file
/*************************************************************************/
#if !defined(Align_H)
#define Align_H

#define IS_ALIGNED_4(variable) \
	(((U32)(variable) & 3) == 0)

#define IS_ALIGNED_8(variable) \
	(((U32)(variable) & 7) == 0)

#endif // Align_H

