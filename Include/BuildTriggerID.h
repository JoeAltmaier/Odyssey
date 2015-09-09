/* BuildTriggerID.h -- Build EIS Triggerpoint ID Code Macros
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
// 	    4/13/99 Bob Butler: Adapted from BuildRequest.h
//      					Added additional macros to build a string 
//							lookup table


// Make the BUILD define the outermost ifdef, so this file can be included 
// twice in EISINIT.C, once for the ENUM mode, and once for the string mode
#ifndef _BuildTriggerIDTable

#ifndef _BuildTriggerID_H
#define _BuildTriggerID_H

#include "CTTypes.h"

#define EISCLSCALE	0x10000
#define EISCLMASK	0xFFFF0000	// Mask Range word from request code

#define DEFINE_EIS_ERROR_CLASSES	enum {

#define EIS_ERROR_CLASS(eisClName)	__EIS_##eisClName##__,

#define END_EIS_ERROR_CLASSES		EIS_ERROR_CLASS_LAST }; 


#define MASK_EIS_ERROR_CLASS(eisID)	(eisID & EISCLMASK)

#define DEFINE_EIS_ID_CODES(eisClName)	extern struct MapEISCodes aEIS_##eisClName##Codes[];\
										enum { eisClName = __EIS_##eisClName##__ * EISCLSCALE,

#define EIS_ID_CODE(eisCode)	eisCode,		

#define END_EIS_ID_CODES(eisClName)		__EIS_##eisClName##__Last }; 

extern BOOL FindClassID(char *, U32 *);
extern BOOL FindTriggerID(char *, U32 *);
extern BOOL FindTriggerID(U32, char *, U32 *);

#endif	// _BuildRequest_H

#else // _BuildTriggerIDTable

struct MapEISCodes 
{ 
	U32 iIDCode; 
	char *sEISCode; 
};

struct MapEISClasses 
{ 
	U32 iClCode; 
	char *sClName; 
	MapEISCodes *paCodes;
};

#undef DEFINE_EIS_ERROR_CLASSES
#define DEFINE_EIS_ERROR_CLASSES   MapEISClasses aEISClasses[] = { 

#undef EIS_ERROR_CLASS
#define EIS_ERROR_CLASS(eisClName)	{ __EIS_##eisClName##__, #eisClName, aEIS_##eisClName##Codes},

#undef END_EIS_ERROR_CLASSES
#define END_EIS_ERROR_CLASSES		{ EIS_ERROR_CLASS_LAST, 0, 0}}; 

#undef DEFINE_EIS_ID_CODES
#define DEFINE_EIS_ID_CODES(eisClName)	MapEISCodes aEIS_##eisClName##Codes[] = {

#undef EIS_ID_CODE
#define EIS_ID_CODE(eisCode)	{ eisCode, #eisCode },

#undef END_EIS_ID_CODES
#define END_EIS_ID_CODES(eisClName)		{__EIS_##eisClName##__Last, 0}  }; 


#endif