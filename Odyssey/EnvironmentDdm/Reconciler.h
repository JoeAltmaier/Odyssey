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
// File: Reconciler.h
// 
// Description:
// 	This file is implementation for the Reconciler module. 
// 
// $Log: /Gemini/Odyssey/EnvironmentDdm/Reconciler.h $
// 
// 2     12/13/99 1:33p Vnguyen
// Update for Environment Ddm.
// 
// 1     11/19/99 2:52p Hdo
// Initial check-in
// 
/*************************************************************************/

#ifndef __Reconciler_H__
#define __Reconciler_H__

#include "EVCRawParameters.h"
#include "EventHandler.h"
#include "EVCStatusRecord.h"

class Reconciler {
public:
	Reconciler();
	~Reconciler();
	void	Set48V(U32 Voltage) { SMP48V = Voltage; }
	void	SetRawData(CtEVCRawParameterRecord &rEVCRawParam1, CtEVCRawParameterRecord &rEVCRawParam2);
	BOOL	GetDistilledData(EVCStatusRecord *EVCRecord, I64 *EVC_Bitmap);

protected:
	// Helper method
	void	Voting();

	U32		SMP48V;
	I64		m_EVC_BitMap;

	// The two raw set of data
	CtEVCRawParameterRecord	m_aEVCRaw[2];

	// Our local copy of EVCStatusRecord. It serves as a previous record
	// of EVC status during the reconcile phase
	EVCStatusRecord			m_EVCStatusRecord;
};

#endif