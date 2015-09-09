/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: SSDStatus.h
//
//
// Description:
// This class is the SSD Status Record.   Containing
// performance data used by the Odyssey Perfoirmance Health Status System.
// 
// $Log: /Gemini/Include/SSDStatus.h $
// 
// 2     10/04/99 3:48p Hdo
// Add PageTableSize, and PercentDirtyPages.
// 
// 1     8/16/99 4:06p Vnguyen
// Initial check-in.
// 
// Update Log: 
// 
/*************************************************************************/

#ifndef __SSDStatus_h
#define __SSDStatus_h


typedef struct
{
	U32		NumReplacementPagesAvailable;
	U32		PageTableSize;
	U32		PercentDirtyPages;
} SSD_STATUS;


#endif // __SSDStatus_h
