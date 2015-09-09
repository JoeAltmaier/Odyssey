/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// Description:
// This class is the EVCStatusReporter Reporter class. 
// 
// Update Log: 
//
// $Log: /Gemini/Odyssey/DdmReporter/EVCStatusReporter.h $
// 
// 2     9/09/99 11:21a Vnguyen
// Complete checking environmental values for Exception Master.
// 
// 1     8/24/99 8:31a Vnguyen
// Initial check-in.
// 
/*************************************************************************/

#ifndef __EVCStatusReporter_h
#define __EVCStatusReporter_h

#include "CTTypes.h"
#include "EVCStatusRecord.h"
#include "Reporter.h"
#include "OneTableClass.h"

#include "EnvStatusReport.h"

#include "EnvRanges.h"
#include "EMCommon.h"

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


class EVCStatusReporter: public StatusOneTableReporter
{

public:
				EVCStatusReporter() {};

virtual	STATUS	Initialize(	DdmReporter*	pPHSReporter,	// Pointer to the PHSReporter DDM. 
							DID				didDdm,
							VDN				vdnDdm,
							REPORTERCODE	ReporterCode);

//
virtual void InitializePHSRecord(void *m_pPHSRecord);
virtual rowID GetPHSrid(void *m_pPHSRecord);
		void InitSensorState(SENSOR_STATE_RECORD *pSensorStateRecord);
		bool CheckBatteryCurrent(EX_LEVEL *pSensorLevel, S32 Current);
		bool CheckSystemPower(EX_LEVEL *pSensorLevel, S32 Current);
		bool CheckDCtoDCTemperature(EX_LEVEL *pSensorLevel, S32 Temperature);
		bool CheckSlotTemperature(EX_LEVEL *pSensorLevel, S32 Temperature,
			S32	TemperatureHiThreshold,	S32 TemperatureNormThreshold);
		bool CheckBatteryTemperature(EX_LEVEL *pSensorLevel, S32 Temperature, bool charge);
		bool CheckIOP(EX_LEVEL *pSensorLevel, bool fPresent);
		bool CheckBool(EX_LEVEL *pSensorLevel, bool good);
		bool CheckVoltage(EX_LEVEL *pSensorLevel, U32 Voltage, U32 NTA1, U32 NTA2, 
							U32 ATN1, U32 ATN2);
		void SendException(	SENSORCODE	SensorCode,
							EX_LEVEL	Level,
							U32			SensorNumber,  	
							TySlot		Slot);

		
		
virtual STATUS	HandleDdmSampleData(void* pDdmData, U32 cbDdmData);
virtual STATUS	ReturnTableRefreshData(void* &rpTableData, U32 &rcbTableData);

// Instance Data:

private:
// Our local copy 
	EVCStatusRecord		m_EVCStatusRecord;
	SENSOR_STATE_RECORD	SensorState;
	XM_CONTEXT			m_XMContext;
	OneTableData		m_Data;
};

#endif	// __EVCStatusReporter_h
