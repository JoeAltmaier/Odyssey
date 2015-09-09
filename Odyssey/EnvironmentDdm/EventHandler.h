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
// File: EventHandler.h
// 
// Description:
// 	This file is implementation for the EventHandler module. 
// 
// $Log: /Gemini/Odyssey/EnvironmentDdm/EventHandler.h $
// 
// 2     12/13/99 1:33p Vnguyen
// Update for Environment Ddm.
// 
// 1     11/17/99 5:52p Hdo
// First check-in
// 
/*************************************************************************/

#ifndef __EventHandler_H__
#define __EventHandler_H__

#include "OsTypes.h"
#include "CtTypes.h"
#include "Address.h"

#include "Event.h"
#include "DdmMaster.h"
#include "AlarmMasterMessages.h"
#include "EVCStatusRecord.h"

#include "EMCommon.h"

#include "EVCPolicyRecord.h"

typedef struct {
  EX_LEVEL	ExitAir[2];			// This is the same as IOP_Temperature[CMB_EVC0/1]
  EX_LEVEL	FanSpeed[4];
  EX_LEVEL	InputOK[3];
  EX_LEVEL	OutputOK[3];
  EX_LEVEL  PrimaryEnable[3];	
  EX_LEVEL	FanFailOrOverTemp[3];
  EX_LEVEL	DCtoDC3Temp[2];
  EX_LEVEL	DCtoDC5Temp[2];
  EX_LEVEL	DCtoDC12ATemp[2];
  EX_LEVEL	DCtoDC12BTemp[2];
  EX_LEVEL	DCtoDC12CTemp[2];
  EX_LEVEL	DCtoDCEnable[2];
  EX_LEVEL	SMP48Voltage;
  EX_LEVEL	DCtoDC3Voltage;
  EX_LEVEL	DCtoDC5Voltage;
  EX_LEVEL	DCtoDC12Voltage;
  EX_LEVEL	BatteryInstalled[2];
  EX_LEVEL	BatteryTemperature[2];
  EX_LEVEL 	BatteryChargeCurrent[2];
  EX_LEVEL  SystemPower;
  EX_LEVEL	IOP_State[CMB_ADDRESS_MAX];	// default is UNKNOWN
  EX_LEVEL	IOP_Temperature[CMB_ADDRESS_MAX];
} SENSOR_STATE_RECORD;

typedef struct {
	XM_CONTEXT	Data;
} AlarmContext;

class EventHandler {
public:
	
	void InitSensorState();
	BOOL CheckEVCStatus(EVCStatusRecord &rEVCStatus, I64 EVCSensorBitmap);

	// Helper methods
	BOOL CheckSystemPower(EX_LEVEL &reSensor, S32 slCurrent);
	BOOL CheckBatteryCurrent(EX_LEVEL &reSensor, S32 Current);
	BOOL CheckDCtoDCTemperature(EX_LEVEL &reSensor, S32 Temperature);
	BOOL CheckSlotTemperature(EX_LEVEL &reSensor,
							  S32 Temperature,
							  S32 TemperatureHiThreshold,
							  S32 TemperatureNormThreshold);
	BOOL CheckBatteryTemperature(EX_LEVEL &reSensor,
								 S32 Temperature,
								 BOOL Discharge);
	BOOL CheckIOP (EX_LEVEL &reSensor, BOOL fPresent);
	BOOL CheckBool(EX_LEVEL &reSensor, BOOL good);
	BOOL CheckVoltage ( EX_LEVEL &reSensor,
						U32 Voltage,
						U32 NTA1,
						U32 NTA2,U32 ATN1,
						U32 ATN2);

protected:

	STATUS	SubmitAlarm(SENSORCODE	SensorCode,
						EX_LEVEL	Level,
						U32			SensorNumber,
						TySlot		Slot);

	STATUS	RemitAlarm(SENSORCODE	SensorCode,
						EX_LEVEL	Level,
						U32			SensorNumber,
						TySlot		Slot);

	VDN					m_ParentVdn;
	DID					m_ParentDid;

	SENSOR_STATE_RECORD	m_SensorState;
	I64					m_SensorBitmap;
	Event				*p_Event;
	AlarmContext		m_AlamrContext;
	XM_CONTEXT			m_XM_Context;
} ;

#endif