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
// File: EventHandler.cpp
// 
// Description:
// 	This file is implementation for the EventHandler module. 
// 
// $Log: /Gemini/Odyssey/EnvironmentDdm/EventHandler.cpp $
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

#include "EventHandler.h"


// InitSensorState -  
// 
// Inputs
// 
// Outputs
//
void EventHandler::InitSensorState()
{
	TRACE_ENTRY(EventHandler::InitSensorState);

	int i;

	for (i=0; i < 2; i++)
		m_SensorState.ExitAir[i] = NORMAL;

	for (i=0; i < 4; i++)
		m_SensorState.FanSpeed[i] = NORMAL;

	for (i=0; i < 3; i++)
		m_SensorState.InputOK[i] = NORMAL;

	for (i=0; i < 3; i++)
		m_SensorState.OutputOK[i] = NORMAL;

	for (i=0; i < 3; i++)
		m_SensorState.PrimaryEnable[i] = NORMAL;

	for (i=0; i < 3; i++)
		m_SensorState.FanFailOrOverTemp[i] = NORMAL;

	for (i=0; i<2; i++)
		m_SensorState.DCtoDC3Temp[i] = NORMAL;

	for (i=0; i < 2; i++)
		m_SensorState.DCtoDC5Temp[i] = NORMAL;

	for (i=0; i < 2; i++)
		m_SensorState.DCtoDC12ATemp[i] = NORMAL;

	for (i=0; i < 2; i++)
		m_SensorState.DCtoDC12BTemp[i] = NORMAL;

	for (i=0; i<2; i++)
		m_SensorState.DCtoDC12CTemp[i] = NORMAL;

	for (i=0; i < 2; i++)
		m_SensorState.DCtoDCEnable[i] = NORMAL;

  	m_SensorState.SMP48Voltage = NORMAL;
  	m_SensorState.DCtoDC3Voltage = NORMAL;
  	m_SensorState.DCtoDC5Voltage = NORMAL;
  	m_SensorState.DCtoDC12Voltage = NORMAL;

	for (i=0; i < 2; i++)
		m_SensorState.BatteryInstalled[i] = NORMAL;

	for (i=0; i < 2; i++)
		m_SensorState.BatteryTemperature[i] = NORMAL;

	for (i=0; i < 2; i++)
		m_SensorState.BatteryChargeCurrent[i] = NORMAL;

	m_SensorState.SystemPower = NORMAL;

	for (i=0; i < CMB_ADDRESS_MAX; i++)
		m_SensorState.IOP_State[i] = UNKNOWN;

	for (i=0; i < CMB_ADDRESS_MAX; i++)
		m_SensorState.IOP_Temperature[i] = NORMAL;
}


// SubmitAlarm
// 
// 
// Inputs
//	SensorCode
//	Level
//	SensorNumber: index into SensorArray if there are more than one sensor.
//	Slot
// 
// Outputs
// 
STATUS EventHandler::SubmitAlarm(SENSORCODE	SensorCode,
	EX_LEVEL	Level,
	U32			SensorNumber,
	TySlot		Slot)
{
	TRACE_ENTRY(EventHandler::SubmitAlarm);

	m_XM_Context.SensorCode = SensorCode;
	m_XM_Context.Level = Level;
	m_XM_Context.SensorNumber = SensorNumber;
	m_XM_Context.Slot = Slot;

	p_Event = new Event(Event::ALARM_EVT, m_ParentVdn, m_ParentDid);
	m_AlamrContext.Data = m_XM_Context;

	MsgSubmitAlarm *pMsg = new MsgSubmitAlarm(m_ParentVdn, m_ParentDid,
		sizeof(AlarmContext), &m_AlamrContext, p_Event, FALSE); // userRemittable

	delete p_Event;

	//return Send(pMsg);
	return OK;
}


// RemitAlarm
// 
// 
// Inputs
//	SensorCode
//	Level
//	SensorNumber: index into SensorArray if there are more than one sensor.
//	Slot
// 
// Outputs
// 
STATUS EventHandler::RemitAlarm(SENSORCODE	SensorCode,
	EX_LEVEL	Level,
	U32			SensorNumber,
	TySlot		Slot)
{
	TRACE_ENTRY(EventHandler::RemitAlarm);

	m_XM_Context.SensorCode = SensorCode;
	m_XM_Context.Level = Level;
	m_XM_Context.SensorNumber = SensorNumber;
	m_XM_Context.Slot = Slot;

	p_Event = new Event(Event::ALARM_EVT, m_ParentVdn, m_ParentDid);
	m_AlamrContext.Data = m_XM_Context;

	//MsgRemitAlarm *pMsg = new MsgRemitAlarm(m_ParentVdn, m_ParentDid,
	//	sizeof(AlarmContext), &m_AlamrContext, p_Event, FALSE); // userRemittable

	delete p_Event;

	//return Send(pMsg);

	return OK;
}



// CheckEVCStatus
// 
// 
// Inputs
// 
// Outputs
// 
BOOL EventHandler::CheckEVCStatus(EVCStatusRecord &rEVCStatus, I64 EVCSensorBitmap)
{
	TRACE_ENTRY(EventHandler::CheckEVCStatus);

	// TODO: check the SensorBitmap

	// check the valid range of all sensors
	// Step 2.1 -- First we check values unique to each EVC.  Before we
	// do that, we have to make sure the EVC is available.  Otherwise, the values
	// are undefined.
	// The following sensors are checked later in the IOP loop (step 2.3):
	// afEvcReachable, ExitAir
 	for (int i = 0; i < 2; i++)
 	{
		if ( rEVCStatus.afEvcReachable[i] )
		{
			// Note:  Currents are not checked.  Only battery charging currents are checked.

			TySlot Slot;
			Slot = ( i ? CMB_EVC1 : CMB_EVC0);

			if ( CheckDCtoDCTemperature(m_SensorState.DCtoDC3Temp[i], rEVCStatus.DCtoDC33Temp[i]) )
				SubmitAlarm(AUX_SUPPLY_3_TEMPERATURE, m_SensorState.DCtoDC3Temp[i], i, Slot);

			if (CheckDCtoDCTemperature(m_SensorState.DCtoDC5Temp[i], rEVCStatus.DCtoDC5Temp[i]))
				SubmitAlarm(AUX_SUPPLY_5_TEMPERATURE,	m_SensorState.DCtoDC5Temp[i], i, Slot);

			if (CheckDCtoDCTemperature(m_SensorState.DCtoDC12ATemp[i], rEVCStatus.DCtoDC12ATemp[i]))
				SubmitAlarm(AUX_SUPPLY_12A_TEMPERATURE, m_SensorState.DCtoDC12ATemp[i], i, Slot);

			if (CheckDCtoDCTemperature(m_SensorState.DCtoDC12BTemp[i], rEVCStatus.DCtoDC12BTemp[i]))
				SubmitAlarm(AUX_SUPPLY_12B_TEMPERATURE, m_SensorState.DCtoDC12BTemp[i], i, Slot);

			if (CheckDCtoDCTemperature(m_SensorState.DCtoDC12CTemp[i], rEVCStatus.DCtoDC12CTemp[i]))
				SubmitAlarm(AUX_SUPPLY_12C_TEMPERATURE, m_SensorState.DCtoDC12CTemp[i], i, Slot);
		} /* if */
	} /* for */

	// Step 2.2 -- Now we check for the merged values from the two EVCs.

	for (int j = 0; j < 3; j++)
	{
		if (CheckBool(m_SensorState.InputOK[j], rEVCStatus.fInputOK[j]))
			SubmitAlarm(PRIMARY_SUPPY_INPUT, m_SensorState.InputOK[j], j, (TySlot) 0);

		if (CheckBool(m_SensorState.OutputOK[j], rEVCStatus.fOutputOK[j]))
			SubmitAlarm(PRIMARY_SUPPLY_OUTPUT, m_SensorState.OutputOK[j], j, (TySlot) 0);

		if (CheckBool(m_SensorState.PrimaryEnable[j], rEVCStatus.fPrimaryEnable[j]))
			SubmitAlarm(PRIMARY_SUPPLY_ENABLE, m_SensorState.OutputOK[j], j, (TySlot) 0);

		if (CheckBool(m_SensorState.FanFailOrOverTemp[j], rEVCStatus.fFanFailOrOverTemp[j]))
			SubmitAlarm(FANFAIL_OR_OVERTEMPERATURE, m_SensorState.FanFailOrOverTemp[j], j, (TySlot) 0);
	}

	for (int j = 0; j<2; j++)
		if (CheckBool(m_SensorState.DCtoDCEnable[j], rEVCStatus.fDCtoDCEnable[j]))
			SubmitAlarm(AUX_SUPPLY_ENABLE, m_SensorState.DCtoDCEnable[j], j, (TySlot) 0);

	// The voltage range for SMP48V depends on the power source: AC or battery
	if (rEVCStatus.BatteryCurrent[0] < BATT_CURRENT_DISCHARGE_NTW ||
		rEVCStatus.BatteryCurrent[1] < BATT_CURRENT_DISCHARGE_NTW)
	{
		// This is discharge
		if (CheckVoltage(m_SensorState.SMP48Voltage, rEVCStatus.SMP48Voltage,
			 			SMP48V_DISCHARGE_NTA1, SMP48V_DISCHARGE_NTA2,
			 			SMP48V_DISCHARGE_ATN1, SMP48V_DISCHARGE_ATN2))
			SubmitAlarm(SMP_VOLTAGE_48, m_SensorState.SMP48Voltage, 0, (TySlot) 0);
	}
	else
	{
		// This is float or charge
		if (CheckVoltage(m_SensorState.SMP48Voltage, rEVCStatus.SMP48Voltage,
			 			SMP48V_NORM_NTA1, SMP48V_NORM_NTA2,
			 			SMP48V_NORM_ATN1, SMP48V_NORM_ATN2))
			SubmitAlarm(SMP_VOLTAGE_48, m_SensorState.SMP48Voltage, 0, (TySlot) 0);
	}
			

	if (CheckVoltage(m_SensorState.DCtoDC3Voltage, rEVCStatus.DCtoDC33Voltage,
		 			DC3_NTA1, DC3_NTA2, DC3_ATN1, DC3_ATN2))
		SubmitAlarm(DC_VOLTAGE_3, m_SensorState.DCtoDC3Voltage, 0, (TySlot) 0);

	if (CheckVoltage(m_SensorState.DCtoDC5Voltage, rEVCStatus.DCtoDC5Voltage,
		 			DC5_NTA1, DC5_NTA2, DC5_ATN1, DC5_ATN2))
		SubmitAlarm(DC_VOLTAGE_5, m_SensorState.DCtoDC5Voltage, 0, (TySlot) 0);

	if (CheckVoltage(m_SensorState.DCtoDC12Voltage, rEVCStatus.DCtoDC12Voltage,
		 			DC12_NTA1, DC12_NTA2, DC12_ATN1, DC12_ATN2))
		SubmitAlarm(DC_VOLTAGE_12, m_SensorState.DCtoDC12Voltage, 0, (TySlot) 0);

	for (int j = 0; j<2; j++)
	{
		if (CheckBool(m_SensorState.BatteryInstalled[j], rEVCStatus.fBatteryInstalled[j]))
			if (m_SensorState.BatteryInstalled[j] == NORMAL)
				SubmitAlarm(BATTERY_INSTALLED, m_SensorState.BatteryInstalled[j], j, (TySlot) 0);
	 		else
	 			SubmitAlarm(BATTERY_REMOVED, m_SensorState.BatteryInstalled[j], j, (TySlot) 0);

		if ( CheckBatteryTemperature(m_SensorState.BatteryTemperature[j], 
								rEVCStatus.BatteryTemperature[j],
								rEVCStatus.BatteryCurrent[j] < BATT_CURRENT_DISCHARGE_NTW) )
			SubmitAlarm(BATTERY_TEMPERATURE, m_SensorState.BatteryTemperature[j], j, (TySlot) 0);

		if ( CheckBatteryCurrent(m_SensorState.BatteryChargeCurrent[j],	rEVCStatus.BatteryCurrent[j]) )
			SubmitAlarm(BATTERY_CHARGE_CURRENT, m_SensorState.BatteryChargeCurrent[j], j, (TySlot) 0); 
			
	}


	// Now we check to see if the system is on battery power or not
	if ( CheckSystemPower(m_SensorState.SystemPower, 
			rEVCStatus.BatteryCurrent[0] + rEVCStatus.BatteryCurrent[1]) )
		SubmitAlarm(SYSTEM_POWER, m_SensorState.SystemPower, 0, (TySlot) 0);


#if 0
	// Step 2.3 -- Now we check all IOPs.  This includes the HBC, IOP, EVC, DDH, etc.
	for (int i = 0; i < CMB_ADDRESS_MAX; i++)
	{
		// Step 1: we check for temperature
		if (rEVCStatus.CmbSlotInfo[i].fPresent)
			if (CheckSlotTemperature(m_SensorState.IOP_Temperature[i],
									rEVCStatus.CmbSlotInfo[i].Temperature,
									rEVCStatus.CmbSlotInfo[i].TemperatureHiThreshold,
									rEVCStatus.CmbSlotInfo[i].TemperatureNormThreshold))
			{
				// Now send notice to Exception Master
				if (i == CMB_EVC0 || i == CMB_EVC1)
					m_XM_Context.SensorCode = EXIT_AIR_TEMP;
				else
					m_XM_Context.SensorCode = SLOT_TEMPERATURE;

				SubmitAlarm(SensorCode, m_SensorState.IOP_Temperature[i], i, (TySlot) i);
			} /* if, if */


		// Step 2: we check for new IOP or deactivated IOP by looking at the fPresent flag
		// Important:  If the IOP state changes from UNKNOWN to NORMAL, that means the system
		// is booting up.  For this case, we do not notify the Exception Master.  For all
		// other state changes, e.g: NORMAL to ALARM (IOP being removed) or ALARM to
		// NORMAL (IOP insertion) we will notify the Exception Master.  This policy is
		// implemented in CheckIOP.
		if (CheckIOP(m_SensorState.IOP_State[i], rEVCStatus.CmbSlotInfo[i].fPresent))
		{
			SENSORCODE SensorCode;
			if (m_SensorState.IOP_State[i] == NORMAL)
				m_XM_Context.SensorCode = SLOT_INSTALLED;
			else
				m_XM_Context.SensorCode = SLOT_REMOVED;

			SubmitAlarm(SensorCode, m_SensorState.IOP_State[i], i, (TySlot) i);
		} /* if */
	} /* for */	
#endif

	return TRUE;
}


// **********************************************************************
// CheckSystemPower -- Check if the system is on AC or Battery power.
// Set state to normal if on AC, WARNING if on battery, ALARM if battery
// power is about to exhaust.  Note:  At this time we don't know how
// to tell if the battery is about to run down.
//
// S32 Current - negative means discharge
// **********************************************************************
BOOL EventHandler::CheckSystemPower(EX_LEVEL &reSensor, S32 Current)
{
	TRACE_ENTRY(EventHandler::CheckSystemPower);
	STATUS status;

	switch( reSensor )
	{
		case NORMAL:
			if (Current < BATT_CURRENT_DISCHARGE_NTW)
			{
				reSensor = WARNING;

				return true;
			}
			break;

		case WARNING:
			if (Current >= BATT_CURRENT_DISCHARGE_WTN)
			{
				reSensor = NORMAL;

				return true;
			}
			break;

		default:
			break;
	} /* switch */
	return false; // no state change
}


// **********************************************************************
// CheckBatteryCurrent -- Check battery charging current.  For first
// release, the charging is probably fixed voltage.
// **********************************************************************
BOOL EventHandler::CheckBatteryCurrent(EX_LEVEL &reSensor, S32 Current)
{
	TRACE_ENTRY(EventHandler::CheckBatteryCurrent);
	STATUS status;

	switch( reSensor )
	{
		case NORMAL:
			if (Current > BATT_CURRENT_CHARGE_NTA)
			{
				reSensor = ALARM;

				return true;
			}
			break;

		case ALARM:
			if (Current < BATT_CURRENT_CHARGE_ATN)
			{
				reSensor = NORMAL;

				return true;
			}
			break;

		default:
			break;
	} /* switch */
	return false; // no state change
}



// **********************************************************************
// CheckDCtoDCTemperature -- Check DC temperature, set state.
// Return true if SensorState changes.
// **********************************************************************
BOOL EventHandler::CheckDCtoDCTemperature(EX_LEVEL &reSensor, S32 Temperature)
{
	TRACE_ENTRY(EventHandler::CheckDCtoDCTemperature);
	STATUS status;

	switch( reSensor )
	{
		case NORMAL:  
			if (Temperature > DC_DC_TEMPERATURE_NTW)
			{
				reSensor = WARNING;

				return true;
			}
			break;

		case WARNING:
			if (Temperature < DC_DC_TEMPERATURE_WTN)
			{
				reSensor = NORMAL;

				return true;
			}
			else
			if (Temperature > DC_DC_TEMPERATURE_WTA)
			{
				reSensor = ALARM;

				return true;
			}
			break; 	

		case ALARM:
			if (Temperature < DC_DC_TEMPERATURE_ATW)
			{
				reSensor = WARNING;

				return true;
			}
			break;

		default:
			break;
	} /* switch */
	return false; // no state change
}

// **********************************************************************
// CheckSlotTemperature -- Check Slot temperature, set state.
// Return true if SensorState changes.
// **********************************************************************
BOOL EventHandler::CheckSlotTemperature(
		EX_LEVEL &reSensor,
		S32 Temperature,
		S32	TemperatureHiThreshold,
		S32 TemperatureNormThreshold)
{
	TRACE_ENTRY(EventHandler::CheckSlotTemperature);
	STATUS status;

	switch( reSensor )
	{
		case NORMAL:
			if (Temperature > SLOT_TEMPERATURE_NTW ||
				Temperature > TemperatureHiThreshold)
			{	
				reSensor = WARNING;

				return true;
			}
			break;

		case WARNING:
			if (Temperature < SLOT_TEMPERATURE_WTN &&
				Temperature < TemperatureNormThreshold)
			{
				reSensor = NORMAL;

				return true;
			}
			else
			if (Temperature > SLOT_TEMPERATURE_WTA)
			{
				reSensor = ALARM;

				return true;
			}
			break; 	

		case ALARM:
			if (Temperature < SLOT_TEMPERATURE_ATW)
			{
				reSensor = WARNING;

				return true;
			}
			break;

		default:
			break;
	} /* switch */
	return false; // no state change
}


// **********************************************************************
// CheckBatteryTemperature -- Check battery temperature, set state.
// Return true if SensorState changes.
// We have two set of thresholds depending if the battery is charging
// or discharging.
// **********************************************************************
BOOL EventHandler::CheckBatteryTemperature(
		EX_LEVEL &reSensor,
		S32 Temperature,
		BOOL Discharge)
{
	TRACE_ENTRY(EventHandler::CheckBatteryTemperature);
	STATUS status;

	if ( Discharge )
	{
		// Battery is discharging
		switch( reSensor )
		{
			case NORMAL:
				if (Temperature > BATT_TEMPERATURE_DISCHARGE_NTW)
				{	
					reSensor = WARNING;

					return true;
				}
				break;

			case WARNING:
				if (Temperature < BATT_TEMPERATURE_DISCHARGE_WTN)
				{
					reSensor = NORMAL;

					return true;
				}  
				else
				if (Temperature > BATT_TEMPERATURE_DISCHARGE_WTA)
				{
					reSensor = ALARM;

					return true;
				}
				break; 	

			case ALARM:
				if (Temperature < BATT_TEMPERATURE_DISCHARGE_ATW)
				{	
					reSensor = WARNING;

					return true;
				}
				break;

			default:
				break;
		} /* switch */
	}
	else
	{
		// Battergy is charging
		switch( reSensor )
		{ 
			case NORMAL:
				if (Temperature > BATT_TEMPERATURE_NORM_NTW)
				{
					reSensor = WARNING;

					return true;
				}
				break;

			case WARNING:
				if (Temperature < BATT_TEMPERATURE_NORM_WTN)
				{
					reSensor = NORMAL;

					return true;
				}
				else
				if (Temperature > BATT_TEMPERATURE_NORM_WTA)
				{
					reSensor = ALARM;

					return true;
				}
				break; 	

			case ALARM:
				if (Temperature < BATT_TEMPERATURE_NORM_ATW)
				{
					reSensor = WARNING;

					return true;
				}
				break;

			default:
				break;
		} /* switch */
		
	} /* else */
	return false; // no state change
}



// **********************************************************************
// CheckIOP -  Step 2: we check for new IOP or deactivated IOP by looking 
// at the fPresent flag Important:  If the IOP state changes from UNKNOWN 
// to NORMAL, that means the system is booting up.  For this case, we do 
// not notify the Exception Master.  For all other state changes, 
// e.g: NORMAL to ALARM (IOP being removed) or ALARM to NORMAL 
// (IOP insertion) we will notify the Exception Master.
// Input:  
// **********************************************************************
BOOL EventHandler::CheckIOP(EX_LEVEL &reSensor, BOOL fPresent)
{
	TRACE_ENTRY(EventHandler::CheckIOP);
	STATUS status;

	switch( reSensor )
	{
		case UNKNOWN:
			if (fPresent)
				reSensor = NORMAL;

				return false;
			break;

		case NORMAL:
			if (!fPresent)
			{
				reSensor = ALARM;

				return true;
			}
			break;

		case ALARM:
			if (fPresent)
			{
				reSensor = NORMAL;

				return true;
			}
			break;
			  		
		default:
			reSensor = UNKNOWN;  // This can't happen!  We just reset state for now.

			break;
	} /* switch */
	return false; // no state change
}


// **********************************************************************
//CheckBool -- Change the SensorLevel according to this
// state machine:
// NORMAL to ALARM:  if good is false
// ALARM to NORM: if good is true
// **********************************************************************
BOOL EventHandler::CheckBool(EX_LEVEL &reSensor, BOOL good)
{
	TRACE_ENTRY(EventHandler::CheckBool);
	STATUS status;

	switch( reSensor )
	{
		case NORMAL:  
			if ( !good )
			{
				reSensor = ALARM;

				return true;
			}
			break;

		case ALARM:
			if ( good )
			{
				reSensor = NORMAL;

				return true;
			}
			break;

		default:
			reSensor = NORMAL;  // This can't happen!  We just reset state for now.
			break;
	} /* switch */
	return false; // no state change
}


// **********************************************************************
// CheckVoltage -  Change state according to the following state machine
// Input:  pSensorLevel
//         Voltage
//         NTA1, NTA2
//         ATN1, ATN2
//
// NORMAL to ALARM: Voltage < NTA1 || Voltage > NTA2
// ALARM to NORMAL: Voltage > NTA1 && Voltage < NTA2
//
// **********************************************************************
BOOL EventHandler::CheckVoltage(
		EX_LEVEL &reSensor, 
		U32	Voltage,
		U32 NTA1,
		U32 NTA2,
		U32 ATN1,
		U32 ATN2)
{
	TRACE_ENTRY(EventHandler::CheckVoltage);
	STATUS status;

	switch( reSensor )
	{
		case NORMAL:
			if (Voltage < NTA1 || Voltage > NTA2)
			{
				reSensor = ALARM;

				return true;
			}
			break;

		case ALARM:
			if (Voltage > ATN1 && Voltage < ATN2)
			{
				reSensor = NORMAL;

				return true;
			}
			break;

		default:
			reSensor = NORMAL;  // This can't happen!  We just reset state for now.
			break;
	} /* switch */
	return false; // no state change
}
