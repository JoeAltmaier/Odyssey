/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: SSDPerformance.h
//
//
// Description:
// This class is the SSD Performance Record.   Containing
// performance data used by the Odyssey Perfoirmance Health Status System.
// 
// $Log: /Gemini/Include/SSDPerformance.h $
// 
// 3     10/08/99 1:52p Vnguyen
// Add comments for NumErasePage.
// 
// 2     10/04/99 3:47p Hdo
// Add NumReadBytesTotal, and NumWriteBytesTotal
// 
// 1     8/16/99 4:06p Vnguyen
// Initial check-in.
// 
// Update Log: 
// 
/*************************************************************************/

#ifndef __SSDPerformance_h
#define __SSDPerformance_h


typedef struct
{
	U32		NumPagesRead;
	U32		NumPagesReadCacheHit;
	U32		NumPagesReadCacheMiss;

	U32		NumPagesWrite;
	U32		NumPagesWriteCacheHit;
	U32		NumPagesWriteCacheMiss;

	U32		NumErasePagesAvailable;	// NumErasePagesAvailable is special.  
									// It is a status type counter.  But we collect 
									// it here to get more frequent data.

	I64		NumReadBytesTotal;
	I64		NumWriteBytesTotal;
} SSD_PERFORMANCE;


#endif // __SSDPerformance_h
