#include "RequestCodes.h"
#include "message.h"
#include "string.h"
#include "ChaosFile.h"

const U32 CF_INVALIDHANDLE = (U32) -1;

// base payload structure for all ChaosFile Requests.  
// the handle is an input value for all but Create and Open, other values are reply values.
class ReqChaosFile : public Message
{
public:
	ReqChaosFile(REQUESTCODE reqCode) : Message(reqCode), handle(CF_INVALIDHANDLE), bytesLastIO(0), resultCode(cfSuccess)
	{
	}

	ReqChaosFile(REQUESTCODE reqCode, U32 handle_) : Message(reqCode), handle(handle_), bytesLastIO(0), resultCode(cfSuccess)
	{
	}

	U32 handle;      
	U32 bytesLastIO;
	U32 resultCode;
	U32 size;
	U32 maxSize;
};



class ReqChaosFileCreate : public ReqChaosFile
{
public:
	ReqChaosFileCreate(char *name_, U32 maxBytes_) 
		: ReqChaosFile(REQ_CHAOSFILE_CREATE), maxBytes(maxBytes_)  
	{ strncpy (name, name_, ChaosFile::cfNameLen); }

	U32 maxBytes;
	char name[ChaosFile::cfNameLen];
};

class ReqChaosFileOpen : public ReqChaosFile
{
public:
	ReqChaosFileOpen(char *name_) : ReqChaosFile(REQ_CHAOSFILE_OPEN)
	{ strncpy (name, name_, ChaosFile::cfNameLen); }

	char name[ChaosFile::cfNameLen];
};


class ReqChaosFileRename : public ReqChaosFile
{
public:
	ReqChaosFileRename(U32 handle_, char *newName_) : ReqChaosFile(REQ_CHAOSFILE_RENAME, handle_)
	{
		strncpy(newName, newName_, ChaosFile::cfNameLen);
	}

	char newName[ChaosFile::cfNameLen];
};


class ReqChaosFileResizeMax : public ReqChaosFile
{
public:
	ReqChaosFileResizeMax(U32 handle_, U32 maxBytes_) : ReqChaosFile(REQ_CHAOSFILE_RESIZE_MAX, handle_), maxBytes(maxBytes_)	{}

	U32 maxBytes;
};


class ReqChaosFileSetSize : public ReqChaosFile
{
public:
	ReqChaosFileSetSize(U32 handle_, U32 size_) : ReqChaosFile(REQ_CHAOSFILE_SET_SIZE, handle_), size(size_)  {}

	U32 size;
};

class ReqChaosFileDelete : public ReqChaosFile
{
public:
	ReqChaosFileDelete(U32 handle_) : ReqChaosFile(REQ_CHAOSFILE_DELETE, handle_)  {}
};

class ReqChaosFileClose : public ReqChaosFile
{
public:
	ReqChaosFileClose(U32 handle_) : ReqChaosFile(REQ_CHAOSFILE_CLOSE, handle_)  {}
};


class ReqChaosFileRead : public ReqChaosFile
{
public:
	ReqChaosFileRead(U32 handle_, void *buffer_, U32 offset_, U32 length_) 
		: ReqChaosFile(REQ_CHAOSFILE_READ, handle_), offset(offset_), length(length_)
	{
		AddSgl(0, buffer_, length_, SGL_REPLY);
	}

	U32 offset;
	U32 length;
};

class ReqChaosFileWrite : public ReqChaosFile
{
public:
	ReqChaosFileWrite(U32 handle_, void *buffer_, U32 offset_, U32 length_, BOOL markEOF_ = false) 
		: ReqChaosFile(REQ_CHAOSFILE_WRITE, handle_), offset(offset_), length(length_), markEOF(markEOF_)
	{
		AddSgl(0, buffer_, length_);
	}

	U32 offset;
	U32 length;
	BOOL markEOF;
};


