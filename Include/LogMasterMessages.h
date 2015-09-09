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
*	This file contains the definition of the Log Master messages.
* 
* Update Log: 
* 07/02/99 Bob Butler: Create file
*
*************************************************************************/

#ifndef LogMasterMessages_h
#define LogMasterMessages_h

#include "Address.h"
#include "CtTypes.h"
#include "ddm.h"

// temporarily put this here and assign it an arbitrary value.  This is the
// status returned for a cancelled listen.  Data in the message should be ignored.
#define ELOG_CANCEL_LISTEN 7592

enum LmListenMode { LM_LISTEN_NONE, LM_LISTEN_ALL, LM_LISTEN_NEW };

class MsgAddLogEntry : public Message
{
public:
	MsgAddLogEntry(Event *pEvt_) : Message(PRIVATE_REQ_ADD_LOGENTRY)
	{
		void *pData;
		cbEventData = pEvt_->GetEventData(&pData);
		AddSgl(0, pData, cbEventData); 
	}
	// D'tor can't be virtual, so the message must be cast prior to deletion.
	~MsgAddLogEntry() { if (IsLast()) CleanAllSgl(); }  // delete client allocated SGLs

	// Create the Event object with the event and parm data from the
	// SGLs.  The caller supplies the pointer and is responsible for 
	// deleting it.
	void GetEvent(Event **ppEvt_)
	{
		void *pData = new char[cbEventData];
		
		CopyFromSgl(0, 0, pData, cbEventData);
		*ppEvt_ = new Event(pData);
		delete pData;
	}
private:
	U32 cbEventData;
};


class MsgQueryLog : public Message
{
public:

	// Ctor: Specify the starting sequence number and the max entries to return
	MsgQueryLog(U32 seqStart_, U16 cMaxEntries_, bool bDirectionFwd_ = true, LmListenMode listenMode_ = LM_LISTEN_NONE) 
		: Message(REQ_QUERY_LOG), iSglTimestamp(0), iSglSeverity(0), seqStart(seqStart_), cMaxEntries(cMaxEntries_),
		  iSglFacility(0), iSglSlot(0), bDirectionFwd(bDirectionFwd_), listenMode(listenMode_)
	// allocate the SGL for reply
		{ AddSgl(iSglLogEntries, NULL, 0, SGL_DYNAMIC_REPLY); }

	// D'tor can't be virtual, so the message must be cast prior to deletion.
	~MsgQueryLog() { if (IsLast()) CleanAllSgl(); }  // delete client allocated SGLs

	// Query calls on different fields can be combined (ANDed), but only one query 
	// can be made per field type.  

	// Timestamp queries.  
	void QueryTimestamp(Timestamp ts_)							// single timestamp
		{ Query(iSglTimestamp, 1, &ts_); }		
	void QueryTimestamp(Timestamp tsFirst_, Timestamp tsLast_)  // range of timestamps
		{ Query(iSglTimestamp, tsFirst_, tsLast_); }
	
	// Severity queries
	void QuerySeverity(U16 sev_)							// single severity
		{ Query(iSglSeverity, 1, &sev_); }
	void QuerySeverity(U16 cSeverities_, const U16 *aSev_)  // list of severities
		{ Query(iSglSeverity, cSeverities_, aSev_); }
	void QuerySeverity(U16 sevFirst_, U16 sevLast_)			// range of severities
		{ Query(iSglSeverity, sevFirst_, sevLast_); }

	// Facility Queries
	void QueryFacility(U16 fac_)							// single facility
		{ Query(iSglFacility, 1, &fac_); }
	void QueryFacility(U16 cFacilities_, const U16 *aFac_)	// list of facilities
		{ Query(iSglFacility, cFacilities_, aFac_); }
	void QueryFacility(U16 facFirst_, U16 facLast_)			// range of facilities
		{ Query(iSglFacility, facFirst_, facLast_); }

	// Slot Queries
	void QuerySlot(TySlot slot_)							// single slot
		{ Query(iSglSlot, 1, &slot_); }
	void QuerySlot(U16 cSlots_, const TySlot *aSlot_)		// list of slots
		{ Query(iSglSlot, cSlots_, aSlot_); }
	void QuerySlot(TySlot slotFirst_, TySlot slotLast_)		// range of slots
		{ Query(iSglSlot, slotFirst_, slotLast_); }

	// Create an array of Event objects from the data supplied in the reply SGL.
	// The number of elements in the array is returned.  
	// The caller supplies the pointer and is responsible for calling delete[] on it.
	
	U16 GetLogEntries(Event **apEvt_);

	// Put the log entries into the reply SGL.  Called by LogMaster prior to replying.
	void SetLogEntries(U16 cEntries_, Event **paEvt_);

	bool IsListener() { return listenMode != LM_LISTEN_NONE; }
	LmListenMode GetListenMode() { return listenMode; }

	bool IsForwardDirection() { return bDirectionFwd; }
	U32	 GetStartingSeqNum() { return seqStart; }
	U32  GetMaxCount() { return cMaxEntries; }

	bool IsMatchTimestamp (Timestamp ts_) { return IsMatch(iSglTimestamp, ts_); }
	bool IsMatchSeverity(U16 sev_) { return IsMatch(iSglSeverity, sev_); }
	bool IsMatchFacility(U16 fac_) { return IsMatch(iSglFacility, fac_); }
	bool IsMatchSlot(TySlot slot_) { return IsMatch(iSglSlot, slot_); }
		
private:
	// SGL Indices. The return log data always goes in SGL 0, the others go into the next
	// available SGL, depending on the order the queries are called in.
	enum {iSglLogEntries = 0 };  
	U16 iSglTimestamp, iSglSeverity, iSglFacility, iSglSlot; 

	enum MQLQueryType { MQL_List, MQL_Range}; // query types.  Single values are treated as an array of 1
	
	struct QuerySGL {			// header for the query SGLs
		MQLQueryType qt;
		U16			 cValues;
	};

	U32 seqStart;				// sequence number to start with
	U32 cMaxEntries;			// max entries to return
	U16 cEntries;				// return value: number of entries in the result list
	bool bDirectionFwd;  
	LmListenMode listenMode;


	// template functions to minimize redundent code.

	template<class T> bool IsMatch(U16 iSGL, T val_)
	{
		if (iSGL == 0)  // not specified, always true
			return true;

		// This could be made faster by saving the pointer, but the non-virtual destructor
		// problem gets in the way.  If necessary, the sender will have to cast and then
		// delete on reply, but for now let's see how bad it is.  
		
		bool match = false;
		U32 cb =  GetSSgl(iSGL);
		char *pData = new char[cb];
		CopyFromSgl(iSGL, 0, pData, cb);

		QuerySGL *pq = (QuerySGL *)pData;
		T *aData = (T *)(pData + sizeof(QuerySGL));
		if (pq->qt == MQL_Range)
			match = val_ >= aData[0] && val_ <= aData[1];
		else // list
			for (int i=0; i < pq->cValues && !match; ++i)
				match = aData[i] == val_;
		delete pData;
		return match;

	}

	// the list query template -- copy the list of values into the SGL.
	template<class T> void Query(U16 &iSGL_, U16 cVals_, const T *aVal_)
	{
		assert(iSGL_ == 0);  // Can't do multiple queries of the same type in the same msg
		iSGL_ = GetCSgl();    

		U16 sData = sizeof(QuerySGL) + sizeof(T) * cVals_;
		char *pData = new char[sData];
		((QuerySGL *)pData)->qt = MQL_List; 
		((QuerySGL *)pData)->cValues = cVals_;
		for (int i=0; i < cVals_; ++i)
			memcpy(pData + sizeof(QuerySGL) + sizeof(T) * i, &aVal_[i], sizeof(T));

		AddSgl(iSGL_, pData, sData);
	}

	// the range query template
	template<class T> void Query(U16 &iSGL_, T valFirst_, T valLast_)
	{
		assert(iSGL_ == 0);  // Can't do multiple queries of the same type in the same msg
		iSGL_ = GetCSgl();    // index is count - 1;

		U16 sData = sizeof(QuerySGL) + sizeof(T) * 2;
		char *pData = new char[sData];
		((QuerySGL *)pData)->qt = MQL_Range;
		((QuerySGL *)pData)->cValues = 2;
		memcpy(pData + sizeof(QuerySGL), &valFirst_, sizeof(T));
		memcpy(pData + sizeof(QuerySGL) + sizeof(T), &valLast_, sizeof(T));

		AddSgl(iSGL_, pData, sData);
	}



};

class MsgQueryLogMetaData : public Message
{
public:
	MsgQueryLogMetaData(LmListenMode listenMode_ = LM_LISTEN_NONE) : Message(REQ_QUERY_LOG_METADATA), listenMode(listenMode_) 
	{}

	// no severity means get first or last across all severities
	Timestamp GetFirstTimestamp();
	Timestamp GetFirstTimestamp(U16 sev_ = -1) { return aTs[lmdFirst][sev_]; }
	Timestamp GetLastTimestamp();
	Timestamp GetLastTimestamp(U16 sev_) { return aTs[lmdLast][sev_]; }
	U32 GetFirstSequenceNumber();
	U32 GetFirstSequenceNumber(U16 sev_) { return aSeqNum[lmdFirst][sev_]; }
	U32 GetLastSequenceNumber();
	U32 GetLastSequenceNumber(U16 sev_) { return aSeqNum[lmdLast][sev_]; }
	
	U32 GetEntryCount();
	U32 GetEntryCount(U16 sev_) { return aCount[sev_]; }

	bool IsListener() { return listenMode != LM_LISTEN_NONE; }
	LmListenMode GetListenMode() { return listenMode; }

	void SetSequenceNumbers(U16 sev_, U32 first_, U32 last_)
	{ aSeqNum[lmdFirst][sev_] = first_; aSeqNum[lmdLast][sev_] = last_; }
	void SetTimestamps(U16 sev_, Timestamp first_, Timestamp last_)
	{ aTs[lmdFirst][sev_] = first_; aTs[lmdLast][sev_] = last_; }
	void SetCount(U16 sev_, U32 count_)
	{ aCount[sev_] = count_; }

private:
	LmListenMode listenMode;
	enum {lmdFirst=0, lmdLast=1};
	U32 aCount[Event::SEVERITY_COUNT];
	U32 aSeqNum[2][Event::SEVERITY_COUNT];
	Timestamp aTs[2][Event::SEVERITY_COUNT];
};

// Cancel a listen on the System Log.  Used for both query listens and metadata listens.
class MsgCancelLogListen : public Message
{
	public:
		// ctor using refnum from the original query message
		MsgCancelLogListen(REFNUM refnum_) : Message(REQ_CANCEL_LOG_LISTEN), cancelRefnum(refnum_)
		{}
		// ctor using a pointer to the original query message
		MsgCancelLogListen(Message *pMsg_) : Message(REQ_CANCEL_LOG_LISTEN), cancelRefnum(pMsg_->refnum)
		{}
		
		REFNUM GetRefnum() { return cancelRefnum; }
		
	private:
		REFNUM cancelRefnum;
};
#endif