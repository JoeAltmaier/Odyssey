/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// (c) Copyright 1999 ConvergeNet Technologies, Inc.
//     All Rights Reserved.
//
// File: AbstractCmd.h
// 
// Description:
// 	This file is an abstract interface for all the command modules.
// 
// $Log: /Gemini/Odyssey/ExceptionMaster/AbstractCmd.h $
// 
// 2     9/08/99 8:14p Hdo
// Define data for AlarmContext
// 
// 1     8/25/99 1:55p Hdo
// First check in
// 
// 8/9/99 Huy Do: Create file
/*************************************************************************/

#ifndef __AbstractCmd_H__
#define __AbstractCmd_H__

#include "Trace_Index.h"
//#define TRACE_INDEX TRACE_EXCEPTION_MASTER
#include "Odyssey_Trace.h"

#include "OsTypes.h"
#include "CtTypes.h"

#include "EMCommon.h"

#include "Event.h"
#include "DdmMaster.h"
#include "AlarmMasterMessages.h"

typedef struct {
	XM_CONTEXT	Data;
} AlarmContext;

class AbstractCmd : DdmMaster {
public:
	AbstractCmd();
	//AbstractCmd(U32 Opcode, SENSORCODE SensorCode) : RqOpcode(Opcode), RqSensorCode(SensorCode) {}
	virtual ~AbstractCmd();

	virtual STATUS performTask(XM_CONTEXT	*pXMContext) = 0;
	U32		getRqOpcode() const { return RqOpcode; }
	void	SetVdn(DID _vdn) { m_ParentVdn = _vdn; }
	void	SetDid(DID _did) { m_ParentDid = _did; }

protected:
	VDN			m_ParentVdn;
	DID			m_ParentDid;

	U32			RqOpcode;
	SENSORCODE	RqSensorCode;
	EX_LEVEL	state;
	rowID		Alarm_rid;
	Event		*p_Event;
	AlarmContext	*pAlamrContext;

} ;
#endif // __AbstractCmd_H__
