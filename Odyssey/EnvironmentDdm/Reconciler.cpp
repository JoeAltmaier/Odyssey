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
// File: Reconciler.cpp
// 
// Description:
// 	This file is implementation for the Reconciler module. 
// 
// $Log: /Gemini/Odyssey/EnvironmentDdm/Reconciler.cpp $
// 
// 3     12/13/99 3:32p Vnguyen
// Remove eEVCMasterSlot per change in EvcRawParameters record.
// 
// 2     12/13/99 1:33p Vnguyen
// Update for Environment Ddm.
// 
// 1     11/19/99 2:52p Hdo
// Initial check-in
// 
/*************************************************************************/

#include "Trace_Index.h"
//#define TRACE_INDEX TRACE_ENVIRONMENT
#include "Odyssey_Trace.h"

#include "Reconciler.h"


//
//
//
Reconciler::Reconciler()
{
	SMP48V = 0;
	memset(&m_EVCStatusRecord, 0, sizeof(EVCStatusRecord));
}


//
//
//
Reconciler::~Reconciler()
{
}


//
//
//
void	Reconciler::SetRawData(CtEVCRawParameterRecord &rEVCRawParam1, CtEVCRawParameterRecord &rEVCRawParam2)
{
	m_aEVCRaw[0] = rEVCRawParam1;
	m_aEVCRaw[1] = rEVCRawParam2;
}


//
//
//
BOOL	Reconciler::GetDistilledData(EVCStatusRecord *EVCRecord, I64 *EVC_Bitmap)
{
	Voting();
	memcpy(EVCRecord, &m_EVCStatusRecord, sizeof(EVCStatusRecord));
	*EVC_Bitmap = m_EVC_BitMap;

	return TRUE;
}


//
//
//
void	Reconciler::Voting()
{
 	// Until the voting algorithm is implemented, we will just accept data from
 	// the last available EVC.

 	// Create the merged EVCStatusRecord
 	for (int i = 0; i < 2; i++)
 	{
 		CtEVCRawParameterRecord *pRawParameterRecord;
 		pRawParameterRecord = &m_aEVCRaw[i];
 		if (pRawParameterRecord->fEvcReachable)
 		{
 			// This EVC is available, let's get its data.
 			m_EVCStatusRecord.afEvcReachable[i] = pRawParameterRecord->fEvcReachable;
 			m_EVCStatusRecord.ExitAirTemp[i] = pRawParameterRecord->ExitAirTemp;
 			m_EVCStatusRecord.ExitTempFanUpThresh = pRawParameterRecord->ExitTempFanUpThresh;
 			m_EVCStatusRecord.ExitTempFanNormThresh = pRawParameterRecord->ExitTempFanNormThresh;
			for (int j = 0; j<4; j++)
 				m_EVCStatusRecord.FanSpeed[j] = pRawParameterRecord->FanSpeed[j];
			for (int j = 0; j<2; j++)
	 			m_EVCStatusRecord.FanSpeedSet[j] = pRawParameterRecord->FanSpeedSet[j];
			for (int j = 0; j<3; j++)
			{
	 			m_EVCStatusRecord.fInputOK[j] = pRawParameterRecord->fInputOK[j];
 				m_EVCStatusRecord.fOutputOK[j] = pRawParameterRecord->fOutputOK[j];
 				m_EVCStatusRecord.fPrimaryEnable[j] = pRawParameterRecord->fPrimaryEnable[j];
 				m_EVCStatusRecord.fFanFailOrOverTemp[j] = pRawParameterRecord->fFanFailOrOverTemp[j];
 			}
 			m_EVCStatusRecord.DCtoDC33Current[i] = pRawParameterRecord->DCtoDC33Current;
 			m_EVCStatusRecord.DCtoDC5Current[i] = pRawParameterRecord->DCtoDC5Current;
 			m_EVCStatusRecord.DCtoDC12ACurrent[i] = pRawParameterRecord->DCtoDC12ACurrent;
 			m_EVCStatusRecord.DCtoDC12BCurrent[i] = pRawParameterRecord->DCtoDC12BCurrent;
 			m_EVCStatusRecord.DCtoDC12CCurrent[i] = pRawParameterRecord->DCtoDC12CCurrent;
 			m_EVCStatusRecord.DCtoDC33Temp[i] = pRawParameterRecord->DCtoDC33Temp;
 			m_EVCStatusRecord.DCtoDC5Temp[i] = pRawParameterRecord->DCtoDC5Temp;
 			m_EVCStatusRecord.DCtoDC12ATemp[i] = pRawParameterRecord->DCtoDC12ATemp;
 			m_EVCStatusRecord.DCtoDC12BTemp[i] = pRawParameterRecord->DCtoDC12BTemp;
 			m_EVCStatusRecord.DCtoDC12CTemp[i] = pRawParameterRecord->DCtoDC12CTemp;
			for (int j = 0; j<2; j++)
	 			m_EVCStatusRecord.fDCtoDCEnable[j] = pRawParameterRecord->fDCtoDCEnable[j];
 			m_EVCStatusRecord.SMP48Voltage = pRawParameterRecord->SMP48Voltage;
 			m_EVCStatusRecord.DCtoDC33Voltage = pRawParameterRecord->DCtoDC33Voltage;
 			m_EVCStatusRecord.DCtoDC5Voltage = pRawParameterRecord->DCtoDC5Voltage;
 			m_EVCStatusRecord.DCtoDC12Voltage = pRawParameterRecord->DCtoDC12Voltage;
 			m_EVCStatusRecord.KeyPosition = pRawParameterRecord->KeyPosition;
			for (int j = 0; j<2; j++)
	 		{
				m_EVCStatusRecord.fBatteryInstalled[j] = 
					(pRawParameterRecord->BatteryTemperature[j] > BATT_PRESENT_TEMPERATURE) ? true : false;
	 			m_EVCStatusRecord.BatteryTemperature[j] = pRawParameterRecord->BatteryTemperature[j];
				m_EVCStatusRecord.BatteryCurrent[j] = pRawParameterRecord->BatteryCurrent[j];
			}
			// m_EVCStatusRecord.eEvcMasterSlot = pRawParameterRecord->eEvcMasterSlot;
			// We don't need eEvcMasterSlot for now.
 		} /* if */
 	} /* for */
}

