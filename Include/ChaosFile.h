/*************************************************************************
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
*	This file contains the interface to the ChaosFile object
* 
* Update Log: 
* 06/07/99 Bob Butler: Create file
* 09/22/99 Bob Butler: Added Win32 support.  
*
*************************************************************************/
#ifndef ChaosFile_H
#define ChaosFile_H

#include "ostypes.h"
#include <stdio.h>

// These will eventually be replaced by event codes
enum CfError {cfSuccess, cfFileNotFound, cfFileAlreadyOpen, cfEOF, cfOutOfSpace, cfDuplicateName, 
			  cfInvalidName, cfInvalidHandle, cfResizeTooSmall, cfWritePastEnd, cfReadPastEnd};

class ChaosFile
{
public:

	ChaosFile (char *name_);
	ChaosFile (char *name_, U32 maxBytes_);

	~ChaosFile();

	BOOL IsValid() { return isValid; }
	CfError GetErrorCode() { return lastErr; }
	
	CfError Rename(char *name_);
	CfError Delete();
	CfError Close();

	CfError SetMaxSize(U32 maxBytes_);
	CfError SetSize(U32 cBytes_);

	U32 GetSize() { return cBytes; }
	U32 GetMaxSize() { return cMaxBytes; }

	CfError Read(void *buffer_, U32 offset_, U32 length_);
	CfError Write(void *buffer_, U32 offset_, U32 length_, BOOL markEOF = false);

	enum { cfNameLen = 20 };   // max length of 19 chars + NULL


private:
	CfError lastErr;
	BOOL isValid;


	U32 handle;


	char *name;

	U32 cBytes, cMaxBytes;

	// Don't allow default constructor, copy constructor or equals operator
	ChaosFile();
	ChaosFile(const ChaosFile &);
	ChaosFile &operator =(const ChaosFile &);
	
};



#endif
