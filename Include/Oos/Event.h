//*************************************************************************
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// Description:
//
// This is the definition of the Event class.  Although an event can be 
// created directly by user code, the normal method of logging events is to call
// LogEvent() function.  LogEvent is supplied as both a global function and 
// a member function of DdmServices, to allow both DDMs and non-DDMs to log events.
// 
// Convenience functions for up to three event arguments are provided.  Events with
// additional arguments can be created directly, and any number of arguments may be 
// added by calling Event::AddEventParameter()
// 
// Update Log: 
// 11/30/98	JFL	Created.
//  4/15/99	RJB	Modified LogEvent interface to use templates.  Renamed ThrowEvent 
//				to LogEvent (personal opinion: Throw implies exception handling --
//				I could be talked into changing it back if there are strong contrary opinions)
//  4/22/99	RJB Added Signal code so events can be logged outside of DDMs
//  5/10/99 RJB Removed va_arg stuff and added AddEventParameter() method
//				Made string event data variable size
//				Changed parameter types to match those in OsTypes.h
//  5/18/99 RJB	Created event_data_t to allow easier packing into a message payload
//				Added constructor to construct an Event from packed data
//  7/21/99 RJB	Cleaned up the interface and hid the internal structs.  
//  8/08/99 RJB Changed the way the parameter data blob is allocated an returned
//				Got rid of some of the template functions that didn't work the 
//				same under all the compilers.
// 10/19/99 JSN Checking in RJBs change to stop using RTTI.  The generated
//              information would have clogged up the small data section.
//*************************************************************************

#ifndef _Event_h_
#define _Event_h_

#include <stdio.h>
#include <typeinfo>
#include <string.h>
#include <assert.h>

#include "OSTypes.h"
#include "CtMessages.h"

#include "address.h"

class Event
{
public:

	// Severity Name Codes:
	enum Severity { INFORMATIONAL_EVT, WARNING_EVT, ERROR_EVT, SYSTEM_EVT, ALARM_EVT };
	enum {SEVERITY_COUNT = 5};
	enum ParmType {			// this can probably be reduced to 3-4 types
		INVALID_PARM = 0,
		CHAR_PARM,		// character value
		S8_PARM,		// 8 bit signed value.
		U8_PARM,		// 8 bit unsigned value.
		S16_PARM,		// 16 bit signed value.
		U16_PARM,		// 16 bit unsigned value. 
		S32_PARM,		// 32 bit signed value.
		U32_PARM,		// 32 bit unsigned value.
		S64_PARM,		// 64 bit signed value. 
		U64_PARM,		// 64 bit unsigned value.
		HEX_PARM,		// Hexidecimal data
		STR_PARM,		// Null terminated character array	
		USTR_PARM		// Double Null terminated double-byte character array	
	};


	// This is just a little utility class used to wrap a pointer and size.  
	// It isn't intended to last beyond a call to Event::AddEventParameter(). 
	class HexData {
	public:
		void *pData;
		U16 cBytes;
		HexData(void *_pData, U16 _cBytes) : pData(_pData), cBytes(_cBytes) { }
	};  // to cause data to be displayed as hex in an event, use AddEventParameter(HexData(&data, sizeof(data)));



	// Construct an event in preparation for calling AddEventParameter().  Events can be
	// constructed with or without DID and VDN, depending on whether they are being created
	// from within a DDM or not.
	Event(STATUS eventCode_, DID did_, VDN vdn_);
	Event(STATUS eventCode_);

	// construct an event from persistent binary data. 
	Event(void 	*epd_);	

	Event() : pEvtParms(NULL) 
	{
		// initialize significant parms to check all parms during alarm coalescing 
		// unless specified differently by the user.
		ed.significantParms = 0xFFFFFFFF;
	}

	void Set(void *ed);

	~Event();

	// methods to get static info
	static int GetMinSeverity()  { return (int)INFORMATIONAL_EVT; }
	static int GetMaxSeverity()  { return (int)ALARM_EVT; }
	static int GetMinFacility()  { return 0; }
	static int GetMaxFacility()  { return 0x00000FFF; }
	
	
	// Get/Set Event data
	
	U32 GetSequenceNum()  const { return ed.sequenceNum; } // zero means not yet in the log
	void SetSequenceNum(U32 sn_)  { ed.sequenceNum = sn_; }
	
	Timestamp GetTimestamp()  const { return ed.timeStamp; } // zero means not in the log.
	void SetTimeStamp(Timestamp ts_)  { ed.timeStamp = ts_; }
	
	STATUS GetEventCode()  const { return ed.ec; }  
	void SetEventCode(STATUS ec_)   { ed.ec = ec_; }
	
	U32 GetSeverity() const { return CT_ST_SEVERITY(ed.ec); }
	U32 GetFacility() const { return CT_ST_FACILITY_ID(ed.ec); }
	U32 GetDetailID() const { return CT_ST_DETAIL_ID(ed.ec); }

	
	TySlot GetSlot()  const { return ed.slot; }		// the slot where the event originated
	void SetSlot(TySlot slot_)   { ed.slot = slot_; }
	
	DID GetDID()  const { return ed.did; }			// The DID of the orginating DDM.  yes, DID contains the slot too, but sometimes we log from non-DDMS
	void SetDID(DID did_)   { ed.did = did_; }
	
	VDN GetVDN()  const { return ed.vdn; }			// The VDN of the orginating DDM.
	void SetVDN(VDN vdn_)   { ed.vdn = vdn_; }

	// Methods to access and set significant parms used during alarm coalescing
	U32 GetSignificantParms() const { return ed.significantParms; }
	void SetSignificantParms(U32 sigParms_) { ed.significantParms = sigParms_; }
	
	U16 GetParameterCount() const { return ed.cParms; }
		
	ParmType GetParameterType(U16 i_) const
	{ 
		assert(i_ < ed.cParms);
		return pEvtParms[i_]->parmType; 
	}

	U32 GetParameterSize(U16 i_) const
	{ 
		assert(i_ < ed.cParms);
		return pEvtParms[i_]->cBytes; 
	}
	
	const void *GetPParameter(U16 i_) const
	{ 
		assert(i_ < ed.cParms);
		return pEvtParms[i_]->pData; 
	}
	
		

	//  Build a hex representation of the supplied data.  This allows logging of any type
	void AddEventParameter(const HexData *hd);

	// Extract the unicode character array from the UnicodeString object.  GetPParameter returns a pointer
	// to the unicode character string, not a pointer to a UnicodeString object.
	void AddEventParameter(const class UnicodeString &us);
	void AddEventParameter(const class UnicodeString *us) { AddEventParameter(*us); }  // pointer version

	void AddEventParameter(const wchar_t *pw);

	void AddEventParameter(const char *parm_)
	{
		EventParm *pParm = new EventParm;

		pParm->cBytes = strlen(parm_) + 1;
		pParm->parmType = STR_PARM;
		// now allocate storage for the data
		pParm->pData = new char[pParm->cBytes]; 
		// and copy it in
		memcpy(pParm->pData, parm_, pParm->cBytes);
		Append(pParm);
	}

	void AddEventParameter (const void *)
	{
		// ignore the add, this is (or at least should be) a null parameter/place holder
	}

	void AddEventParameter (U32 parm_)
	{
		EventParm *pParm = new EventParm;

		pParm->cBytes = sizeof(U32);
		pParm->parmType = U32_PARM;	// 32 bit unsigned value.
		// now allocate storage for the data
		pParm->pData = new char[pParm->cBytes]; 
		// and copy it in
		memcpy(pParm->pData, &parm_, pParm->cBytes);
		Append(pParm);
	}

	// pack up the data into a blob to be stuck into a variable field in the log
	U32 GetEventData(void **pData_) const;
	

protected:

private:
	// This structure defines the basic event data, excluding parameter data.
	// it maps to what ends up in the System Log PTS table
	struct 
	{
		U32				sequenceNum;	// filled in when written to the log.
		I64				timeStamp;		// filled in when written to the log.
		STATUS			ec;				// The event code as generated by the message compiler
		TySlot			slot;			// the slot where the event originated
		DID				did;			// The DID of the orginating DDM.  yes, DID contains the slot too, but sometimes we log from non-DDMS
		VDN				vdn;			// The VDN of the orginating DDM.
		U32				cParms;			// a count of parameters
		U32				significantParms; // bitmap which identifies significant parms
										// for alarm comparison
	} ed;


	// don't allow copy ctor or equals operator.  Probably add later.
	Event(const Event &);
	Event &operator=(const Event &);

	struct EventParm 
	{
		ParmType		parmType;	// the parameter type
		U16				cBytes;		// the size of the data	
		void			*pData;		// the data itself
	} **pEvtParms;  // array of pointers to the event parameters
	

	// append a parameter to the parameter array
	void Append(EventParm *pParm_);

};
 
// the interface to the actual logging mechanism
void LogEvent(Event *) ;

// These are convenience functions for logging with zero to 3 event data parameters.  For 
// more, create an Event object and call AddEventParameter as often as necessary and call LogEvent(pEvt). 
// AddEventParameter is a template function which can handle any of the types listed in ParmTypeEnum,
// if any other types are passed, they will be treated as a hex dump

// This function is called both by the below inline functions and the LogEvent methods 
// in DdmOsServices (which create event objects with DID and VDN)
template <class T1, class T2, class T3> inline void LogEvent(Event *evt, T1 arg1, T2 arg2, T3 arg3)
{
	// If type is "void *", no event parameter will be added by AddEventParameter(void *)

	evt->AddEventParameter(arg1);
	evt->AddEventParameter(arg2);
	evt->AddEventParameter(arg3);
	LogEvent(evt);
}

// These are non-DDM versions of logging functions.  DDM versions have the same interface, 
// but add DID and VDN to the Event object.
inline void LogEvent(STATUS eventCode) 
{ LogEvent(new Event(eventCode), (void*)NULL, (void*)NULL, (void*)NULL); }
template<class T1> inline void LogEvent(STATUS eventCode, T1 arg1)
{ LogEvent(new Event(eventCode), arg1, (void*)NULL, (void*)NULL); }
template <class T1, class T2> inline void LogEvent(STATUS eventCode, T1 arg1, T2 arg2)
{	LogEvent(new Event(eventCode), arg1, arg2, (void*)NULL); }
template <class T1, class T2, class T3> inline void LogEvent(STATUS eventCode, T1 arg1, T2 arg2, T3 arg3)
{	LogEvent(new Event(eventCode), arg1, arg2, arg3); }




#endif	// _Event_h_
