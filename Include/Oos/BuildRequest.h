/* BuildRequest.h -- Build Request Code Macros
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
**/

// Revision History:
// 	    3/05/99 Tom Nelson: Created
//      3/22/99 Tom Nelson: Add Range Masking


#ifdef _REQUEST_NAMES_
// build request code name table

#undef DEFINE_REQUEST_RANGES
#define DEFINE_REQUEST_RANGES	

#undef REQUEST_RANGE
#define REQUEST_RANGE(rqRgName)	

#undef END_REQUEST_RANGES
#define END_REQUEST_RANGES		struct {REQUESTCODE rqCode; char *pName;} aStRqName[]={


#undef DEFINE_REQUEST_CODES
#define DEFINE_REQUEST_CODES(rqRgName)	

#undef REQUEST_CODE
#define REQUEST_CODE(rqCode)	{rqCode, #rqCode},	

#undef END_REQUEST_CODES
#define END_REQUEST_CODES(rqRgName)		


#else // define request code identifiers

#define RQRGSCALE	0x10000
#define RQRGMASK	0xFFFF0000	// Mask Range word from request code

#define DEFINE_REQUEST_RANGES	enum {

#define REQUEST_RANGE(rqRgName)	__##rqRgName##__,

#define END_REQUEST_RANGES		REQUEST_RANGE_LAST }; 


#define MASK_REQUEST_RANGE(rqCode)	(rqCode & RQRGMASK)

#define DEFINE_REQUEST_CODES(rqRgName)	enum { rqRgName = __##rqRgName##__ * RQRGSCALE,

#define REQUEST_CODE(rqCode)	rqCode,		

#define END_REQUEST_CODES(rqRgName)		__##rqRgName##__Last }; 

#endif  // _REQUEST_NAMES_


