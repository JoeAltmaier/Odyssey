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
// This places keeps the default refresh and sample rates
// 
// Update Log: 
//
// $Log: /Gemini/Odyssey/DdmReporter/DefaultRates.h $
// 
// 1     8/16/99 2:39p Vnguyen
// New check-in.
// 
/*************************************************************************/

#ifndef __DefaultRates_h
#define __DefaultRates_h

// These are defaults Refresh and Sample Rates.
// Use 5 seconds for status refresh rate.
// Use 10 and 1 seconds for performance Refresh and Sample rates.
#define DISK_STATUS_REFRESH_RATE		 5000000
#define DISK_PERFORMANCE_REFRESH_RATE	10000000 	// Microseconds
#define DISK_PERFORMANCE_SAMPLE_RATE	 1000000

#define ARRAY_STATUS_REFRESH_RATE		 5000000 	
#define ARRAY_PERFORMANCE_REFRESH_RATE	10000000 	
#define ARRAY_PERFORMANCE_SAMPLE_RATE	 1000000

#define STS_STATUS_REFRESH_RATE			 5000000 	
#define STS_PERFORMANCE_REFRESH_RATE	10000000 	
#define STS_PERFORMANCE_SAMPLE_RATE	 	 1000000

#define FCPT_STATUS_REFRESH_RATE		 5000000 	
#define FCPT_PERFORMANCE_REFRESH_RATE	10000000 	
#define FCPT_PERFORMANCE_SAMPLE_RATE	 1000000

#define SSD_STATUS_REFRESH_RATE			 5000000 	
#define SSD_PERFORMANCE_REFRESH_RATE	10000000 	
#define SSD_PERFORMANCE_SAMPLE_RATE	 	 1000000

#define EVC_STATUS_REFRESH_RATE			 5000000 	

#endif // __DefaultRates_h
