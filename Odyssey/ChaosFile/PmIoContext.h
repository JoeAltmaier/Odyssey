#ifndef PMIOCONTEXT_H
#define PMIOCONTEXT_H

class PmIoContext 
{
	friend class DdmPartitionMgr;
private:

	PmIoContext(DdmPartitionMgr *pDdm_, DdmServices::ActionCallback acHandler_, void *buffer_, U32 offset_, U32 length_,
		Message *pMsg_, DdmServices::ActionCallback acComplete_, bool deleteBuffer_ = false) 
		: pDdm(pDdm_), acHandler(acHandler_), buffer(buffer_), cPages(0), pReplyMsg(pMsg_), aPgNums(NULL),
		  acComplete(acComplete_), deleteBuffer(deleteBuffer_), offset(offset_), 
		  length(length_), next(NULL), workBuffer(NULL), currPage(0), cPending(0)
	{
	}

	~PmIoContext() 
	{ 
		delete []aPgNums; 
		delete workBuffer;
		
		if (!deleteBuffer)  // fix for weird bug (compiler?)
			buffer = NULL;
			
		if (deleteBuffer == true)
			delete buffer;
	}

	void DeleteList()
	{
		PmIoContext *pIOC = next;
		next = NULL;
		if (pIOC->next != NULL)
			pIOC->DeleteList();
		delete pIOC;
	}

	void SetPageLinks(U32 bytesPerPage_, U32 *pPLT_, U32 startLink_)
	{
		U32 startPg = (offset - (offset % bytesPerPage_)) / bytesPerPage_;
		U32 endPg = ((offset + length) / bytesPerPage_ + (((offset + length) % bytesPerPage_) ? 1 : 0)) - 1;
		
		cPages = endPg - startPg + 1;
		aPgNums = new U32[cPages];

		U32 next = startLink_;
		int i = 0;
		while (i < startPg)
		{
			next = *(pPLT_ + next);
			++i;
		}

		while (i <= endPg)
		{
			aPgNums[i - startPg] = next;
			next = *(pPLT_ + next);
			++i;
		}
		
	}


	bool deleteBuffer;  // if true, delete the buffer below when the dtor is called
	void *buffer;

	void *workBuffer; // this one is always deleted
	
	U32 cPages, cPending, currPage;
	U32 *aPgNums;
	U32 offset;
	U32 length;

	PmIoContext *next;  // chain several operations together.  Only the final operation kicks off
						// the acComplete action, unless an error occurs.  If an error occurs,
						// we abort and call acComplete immediately.  Each intermediate operation
						// should have the same acComplete and reply message.

	DdmServices::ActionCallback acHandler;   // Call this action callback when we are ready to 
											 // process this IO operation.  The this pointer is the payload.
	DdmServices::ActionCallback acComplete;  // Call this action callback when IO is complete, passing pMsgReply as the payload
	Message *pReplyMsg;
	DdmPartitionMgr *pDdm;
		
	PmIoContext();	  
	PmIoContext(const PmIoContext &) ;
	PmIoContext &operator =(const PmIoContext &);
};

#endif