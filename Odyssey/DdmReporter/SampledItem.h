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
// This class is the class for a sampled peice of data.
//
// $Log: /Gemini/Odyssey/DdmReporter/SampledItem.h $
// 
// 5     9/09/99 11:23a Vnguyen
// Fix bug in calculating min and max samples
// 
// 4     8/11/99 8:27a Vnguyen
// Add member function Count() to return the number of samples done so
// far.
// 
// 3     5/04/99 9:35a Jlane
// Miscellaneous updates to get compiuling and to bring in synch with new
// Ddm Model.
// 
// 2     4/22/99 9:30a Jlane
// Add trailing semicolan.
//
// Update Log: 
// 10/26/98	JFL	Created.
/*************************************************************************/

#ifndef __SampledItems_h
#define __SampledItems_h

#include "CTTypes.h"
//#include <Math.h>

class SampledI64 {
public:
	I64		sum_of_samples;
	I64		sum_of_squared_samples;
	I64		num_samples;
	I64		max_sample;
	I64		min_sample;
	I64		avg_sample;
	
	void	Reset() 
	{
		sum_of_samples =
		sum_of_squared_samples =
		num_samples = 
		max_sample = 
		min_sample =
		avg_sample = 0;
	};

	void	Sample(I64 new_sample)
	{ 

		if (num_samples)
		{
			if (new_sample > max_sample)
				max_sample = new_sample;
			if (new_sample < min_sample)
				min_sample = new_sample;
		}
		else
		{	min_sample =
			max_sample = new_sample;
		}

		sum_of_samples += new_sample;
		num_samples++;
	};
	
	I64		Average()
	{
		return sum_of_samples / num_samples;
	};
	
	I64		Variance()
	{
		// The math is compliments of Excel help.  Thanks to Norm for the tip.
		return (num_samples * sum_of_squared_samples - (sum_of_samples * sum_of_samples)) / 
		        num_samples * (num_samples-1);
	};
	
	#if false
	float	StandardDeviation()
	{
		return sqrt(Variance);
	};
	#endif
	
	I64	Total()
	{
		return sum_of_samples;
	}
	
	I64	Max() 
	{
		return max_sample;
	};
	
	I64	Min() 
	{
		return min_sample;
	};
	
	I64 Count()
	{
		return num_samples;
	}
	
};


class SampledU32 {
public:

	U32		sum_of_samples;
	U32		sum_of_squared_samples;
	U32		num_samples;
	U32		max_sample;
	U32		min_sample;
	U32		avg_sample;
	
	void	Reset() 
	{
		sum_of_samples =
		sum_of_squared_samples =
		num_samples = 
		max_sample = 
		min_sample =
		avg_sample = 0;
	};

	void	Sample(U32 new_sample)
	{ 
		if (num_samples)
		{
			if (new_sample > max_sample)
				max_sample = new_sample;
			if (new_sample < min_sample)
				min_sample = new_sample;
		}
		else
		{	min_sample =
			max_sample = new_sample;
		}

		sum_of_samples += new_sample;
		num_samples++;
	};
	
	U32		Average()
	{
		return sum_of_samples / num_samples;
	};
	
	U32		Variance()
	{
		// The math is compliments of Excel help.  Thanks to Norm for the tip.
		return (num_samples * sum_of_squared_samples - (sum_of_samples * sum_of_samples)) / 
		        num_samples * (num_samples-1);
	};
	
	#if false
	float	StandardDeviation()
	{
		return sqrt(Variance);
	};
	#endif
	
	U32	Total()
	{
		return sum_of_samples;
	}
	
	U32	Max() 
	{
		return max_sample;
	};
	
	U32	Min() 
	{
		return min_sample;
	};

	U32 Count()
	{
		return num_samples;
	}

};

#endif	// __SampledItems_h
