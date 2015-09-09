/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: EVCStatusReporter.cpp
// 
// Description:
// This file implements the EVCStatusReporter Reporter class. 
// 
// Update Log 
// 
// $Log: /Gemini/Odyssey/DdmReporter/EVCStatusReporter.cpp $
// 
// 9     12/13/99 3:35p Vnguyen
// Remove eEvcMasterSlot per change in EvcRawParameters record.
// 
// 8     12/01/99 5:20p Vnguyen
// Add typecast for strcpy to get rid of compiler warning due to type
// mismatch between const char * and char *.
// 
// 7     12/01/99 4:49p Ewedel
// Changed to work with new CPtsRecordBase-based EVC Status record.
// 
// 6     11/06/99 5:25p Vnguyen
// Set the table name to the official name to start populating the table.
// 
// 5     11/06/99 3:10p Vnguyen
// Add tracing, and disable hooks to exception master for now.  Most of
// the values from CMB are still bogus.
// 
// 4     10/21/99 4:46p Vnguyen
// Add entry for EvcMasterSlot per change in EvcRawParameters.h
// 
// 3     10/08/99 4:45p Vnguyen
// Use FieldDef and FieldDefSize for EVCStatusRecord.  Much easier to
// maintain than the old way.
// 
// 2     9/09/99 11:21a Vnguyen
// Complete checking environmental values for Exception Master.
// 
// 1     8/24/99 8:31a Vnguyen
// Initial check-in.
// 
/*************************************************************************/

#include "EVCStatusReporter.h"
#include "EnvStatusReport.h"
#include <String.h>
#include "Odyssey_Trace.h"
#include "EVCStatusRecord.h"
#include "DefaultRates.h"
#include "Message.h"

/**********************************************************************/
// Initialize - This is the initializor for the EVCStatusReporter Object.
// We've been called because the PHSReporter Ddm has received a call from
// the SSD DDM and now 
// he (the PHS Ddm) wants to instantiate (and initialize) a new reporter
// object.  Our Job, as the is to make sure that our
// status table and record are created and if not, create them.  
// Note:  All the work is done by the lower class object.
// All we need to do here is to supply Ddm specific info, e.g. table names,
// a buffer to hold the row record, etc.  
/**********************************************************************/
STATUS EVCStatusReporter::Initialize(
	DdmReporter		*pPHSReporter,	// Pointer to PHSReporter DDM.
	DID				didDdm,
	VDN				vdnDdm,  // this parameter is not used, yet.
	REPORTERCODE	ReporterCode
)
{
#pragma unused(vdnDdm)
STATUS	status;
	//
	// Initialize our parent Ddm.  Need to do this to send etc.
	SetParentDdm(m_pPHSReporter = pPHSReporter);

	strcpy(m_Data.PHSTableName, (char *) EVC_STATUS_TABLE); 	 	// Name of table to send PHS data to
//strcpy(m_Data.PHSTableName, "TempEVCTable");		 	// Name of table to send PHS data to

//	m_Data.aPHSTableFieldDefs = (fieldDef *)aEvcStatusTable_FieldDefs;	// FieldDefs for PHS table 
	m_Data.aPHSTableFieldDefs = (fieldDef *)EVCStatusRecord::FieldDefs();	// FieldDefs for PHS table 
//	m_Data.cbPHSTableFieldDefs = cbEvcStatusTable_FieldDefs;	// size of PHS table fieldDefs, U32 cbrgFieldDefs
	m_Data.cbPHSTableFieldDefs = EVCStatusRecord::FieldDefsSize();	// size of PHS table fieldDefs, U32 cbrgFieldDefs

	m_Data.pPHSRecord = &m_EVCStatusRecord;				// A pointer to a buffer for a PHS row record
	m_Data.cbPHSRecord = sizeof(m_EVCStatusRecord);		// Size of a PHS row record, cbRowData
	m_Data.cbPHSRecordMax = sizeof(m_EVCStatusRecord); 	// Maximum size of buffer to receive row data, cbRowDataRetMax

	m_Data.pRefreshRate = &m_EVCStatusRecord.RefreshRate;
	m_Data.pSampleRate = &m_EVCStatusRecord.RefreshRate;  // For status reporter, use Refresh rate for Sample rate.
	
	// initialize the sesor states to default values.  Later on, when they change
	// state, we need to call Exception Master.
	InitSensorState(&SensorState);
													
	// Up up and away...
	status = InitOneTable(&m_Data, didDdm, ReporterCode);
	return status;
}
 

/**********************************************************************/
// InitializePHSRecord - This method fills in the PHS record with default
// values.  
/**********************************************************************/
void EVCStatusReporter::InitializePHSRecord(
 		void *pPHSRecord 		
 )
 {
 	EVCStatusRecord	*pEVCStatusRecord;
 	pEVCStatusRecord = (EVCStatusRecord *) pPHSRecord;
 	
 	// Declare/construct an EVC Status Record with default values. 
	EVCStatusRecord	newEVCStatusRecord;
		
	*pEVCStatusRecord = newEVCStatusRecord;
 	return;	
 }
 


/**********************************************************************/
// HandleDdmSampleDataReply -  Handle the Status data returned from the Ddm.
// Also implement the Voting algorithm.  Note:  only notify Exception Master
// in the routine ReturnTableRefreshData, not here.
/**********************************************************************/
STATUS EVCStatusReporter::HandleDdmSampleData( void* pDdmData, U32 cbDdmData )
{
#pragma unused(cbDdmData)

STATUS	status = OK;

	Tracef("EVCStatusReporter::HandleDdmSampleData()\n");

	// Get a pointer to the Ddm's reply and cast it to our structure.

	ENV_STATUS_RECORD *pEnvStatus;
	pEnvStatus = (ENV_STATUS_RECORD*) pDdmData;
	
	// The pEnvStatus is pointing to the data from 2 EVCs and all IOP temperature.
	// We need to merge the EVCs' data into a single record for EVCStatusRecord
	// and look at the IOP temperature info to check for overtemperature condition.
	
	// Temperature = pEnvStatus->CmbSlotInfo[0].Temp;

	// m_EVCStatusRecord

 	// Until the voting algorithm is implemented, we will just accept data from
 	// the last available EVC.
 	
 	// Step one:  Create the merged EVCStatusRecord
 	for (int i = 0; i<2; i++)
 	{
 		CtEVCRawParameterRecord *pRawParameterRecord;
 		pRawParameterRecord = &pEnvStatus->EVCRawParameters[i];
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
 
 // We disable the checking for now
 #if 0
 	
 	// Step two:  Check for exception and call Exception Master once for each change
	
	// Step 2.1 -- First we check values unique to each EVC.  Before we
	// do that, we have to make sure the EVC is available.  Otherwise, the values
	// are undefined.
	// The following sensors are checked later in the IOP loop (step 2.3):
	// afEvcReachable, ExitAir
 	for (int i = 0; i<2; i++)
 	{
		if (m_EVCStatusRecord.afEvcReachable[i])
		{
			// Note:  Currents are not checked.  Only battery charging currents are checked.
 
 			TySlot Slot;
 			Slot = ( i ? CMB_EVC1 : CMB_EVC0);

			if (CheckDCtoDCTemperature(&SensorState.DCtoDC3Temp[i], m_EVCStatusRecord.DCtoDC33Temp[i]))
			{
				m_XMContext.Value.Temperature = m_EVCStatusRecord.DCtoDC33Temp[i];
				SendException(AUX_SUPPLY_3_TEMPERATURE,	SensorState.DCtoDC3Temp[i], i, Slot);
			}

			if (CheckDCtoDCTemperature(&SensorState.DCtoDC5Temp[i], m_EVCStatusRecord.DCtoDC5Temp[i]))
			{
				m_XMContext.Value.Temperature = m_EVCStatusRecord.DCtoDC5Temp[i];
				SendException(AUX_SUPPLY_5_TEMPERATURE,	SensorState.DCtoDC5Temp[i], i, Slot);
			}

			if (CheckDCtoDCTemperature(&SensorState.DCtoDC12ATemp[i], m_EVCStatusRecord.DCtoDC12ATemp[i]))
			{
				m_XMContext.Value.Temperature = m_EVCStatusRecord.DCtoDC12ATemp[i];
				SendException(AUX_SUPPLY_12A_TEMPERATURE, SensorState.DCtoDC12ATemp[i], i, Slot);
			}

			if (CheckDCtoDCTemperature(&SensorState.DCtoDC12BTemp[i], m_EVCStatusRecord.DCtoDC12BTemp[i]))
			{
				m_XMContext.Value.Temperature = m_EVCStatusRecord.DCtoDC12BTemp[i];
				SendException(AUX_SUPPLY_12B_TEMPERATURE, SensorState.DCtoDC12BTemp[i], i, Slot);
			}

			if (CheckDCtoDCTemperature(&SensorState.DCtoDC12CTemp[i], m_EVCStatusRecord.DCtoDC12CTemp[i]))
			{
				m_XMContext.Value.Temperature = m_EVCStatusRecord.DCtoDC12CTemp[i];
				SendException(AUX_SUPPLY_12C_TEMPERATURE, SensorState.DCtoDC12CTemp[i], i, Slot);
			}
						  		
		} /* if */
	} /* for */
	
	// Step 2.2 -- Now we check for the merged values from the two EVCs.

	
	for (int j = 0; j<3; j++)
	{
		if (CheckBool(&SensorState.InputOK[j], m_EVCStatusRecord.fInputOK[j]))
			SendException(PRIMARY_SUPPY_INPUT, SensorState.InputOK[j], j, (TySlot) 0);

		if (CheckBool(&SensorState.OutputOK[j], m_EVCStatusRecord.fOutputOK[j]))
			SendException(PRIMARY_SUPPLY_OUTPUT, SensorState.OutputOK[j], j, (TySlot) 0);

		if (CheckBool(&SensorState.PrimaryEnable[j], m_EVCStatusRecord.fPrimaryEnable[j]))
			SendException(PRIMARY_SUPPLY_ENABLE, SensorState.OutputOK[j], j, (TySlot) 0);

		if (CheckBool(&SensorState.FanFailOrOverTemp[j], m_EVCStatusRecord.fFanFailOrOverTemp[j]))
			SendException(FANFAIL_OR_OVERTEMPERATURE, SensorState.FanFailOrOverTemp[j], j, (TySlot) 0);
	}

	for (int j = 0; j<2; j++)
		 if (CheckBool(&SensorState.DCtoDCEnable[j], m_EVCStatusRecord.fDCtoDCEnable[j]))
			SendException(AUX_SUPPLY_ENABLE, SensorState.DCtoDCEnable[j], j, (TySlot) 0);
			
	// The voltage range for SMP48V depends on the power source: AC or battery
	if (m_EVCStatusRecord.BatteryCurrent[0] < BATT_CURRENT_DISCHARGE_NTW ||
		m_EVCStatusRecord.BatteryCurrent[1] < BATT_CURRENT_DISCHARGE_NTW)
	{
		// This is discharge
		if (CheckVoltage(&SensorState.SMP48Voltage, m_EVCStatusRecord.SMP48Voltage,
			 			SMP48V_DISCHARGE_NTA1, SMP48V_DISCHARGE_NTA2,
			 			SMP48V_DISCHARGE_ATN1, SMP48V_DISCHARGE_ATN2))
		{
			m_XMContext.Value.Voltage = m_EVCStatusRecord.SMP48Voltage;
			SendException(SMP_VOLTAGE_48, SensorState.SMP48Voltage, 0, (TySlot) 0);
		}	
	}
	else
	{
		// This is float or charge
		if (CheckVoltage(&SensorState.SMP48Voltage, m_EVCStatusRecord.SMP48Voltage,
			 			SMP48V_NORM_NTA1, SMP48V_NORM_NTA2,
			 			SMP48V_NORM_ATN1, SMP48V_NORM_ATN2))
		{
			m_XMContext.Value.Voltage = m_EVCStatusRecord.SMP48Voltage;
			SendException(SMP_VOLTAGE_48, SensorState.SMP48Voltage, 0, (TySlot) 0);
		}	
	}
			

	if (CheckVoltage(&SensorState.DCtoDC3Voltage, m_EVCStatusRecord.DCtoDC33Voltage,
		 			DC3_NTA1, DC3_NTA2, DC3_ATN1, DC3_ATN2))
	{
		m_XMContext.Value.Voltage = m_EVCStatusRecord.DCtoDC33Voltage;
		SendException(DC_VOLTAGE_3, SensorState.DCtoDC3Voltage, 0, (TySlot) 0);
	}	

	if (CheckVoltage(&SensorState.DCtoDC5Voltage, m_EVCStatusRecord.DCtoDC5Voltage,
		 			DC5_NTA1, DC5_NTA2, DC5_ATN1, DC5_ATN2))
	{
		m_XMContext.Value.Voltage = m_EVCStatusRecord.DCtoDC5Voltage;
		SendException(DC_VOLTAGE_5, SensorState.DCtoDC5Voltage, 0, (TySlot) 0);
	}	

	if (CheckVoltage(&SensorState.DCtoDC12Voltage, m_EVCStatusRecord.DCtoDC12Voltage,
		 			DC12_NTA1, DC12_NTA2, DC12_ATN1, DC12_ATN2))
	{
		m_XMContext.Value.Voltage = m_EVCStatusRecord.DCtoDC12Voltage;
		SendException(DC_VOLTAGE_12, SensorState.DCtoDC12Voltage, 0, (TySlot) 0);
	}	

	for (int j = 0; j<2; j++)
	{
		if (CheckBool(&SensorState.BatteryInstalled[j], m_EVCStatusRecord.fBatteryInstalled[j]))
			if (SensorState.BatteryInstalled[j] == NORMAL)
				SendException(BATTERY_INSTALLED, SensorState.BatteryInstalled[j], j, (TySlot) 0);
	 		else
	 			SendException(BATTERY_REMOVED, SensorState.BatteryInstalled[j], j, (TySlot) 0);
	 				
		if (CheckBatteryTemperature(&SensorState.BatteryTemperature[j], 
				m_EVCStatusRecord.BatteryTemperature[j],
				m_EVCStatusRecord.BatteryCurrent[j] < BATT_CURRENT_DISCHARGE_NTW))
			SendException(BATTERY_TEMPERATURE, SensorState.BatteryTemperature[j], j, (TySlot) 0);
					
		if (CheckBatteryCurrent(&SensorState.BatteryChargeCurrent[j],
								m_EVCStatusRecord.BatteryCurrent[j]))
			SendException(BATTERY_CHARGE_CURRENT, SensorState.BatteryChargeCurrent[j], j, (TySlot) 0); 
			
	}
	

	// Now we check to see if the system is on battery power or not
	if (CheckSystemPower(&SensorState.SystemPower, 
			m_EVCStatusRecord.BatteryCurrent[0] + m_EVCStatusRecord.BatteryCurrent[1]))
		SendException(SYSTEM_POWER, SensorState.SystemPower, 0, (TySlot) 0);
	
	// Step 2.3 -- Now we check all IOPs.  This includes the HBC, IOP, EVC, DDH, etc.
	for (int i = 0; i < CMB_ADDRESS_MAX; i++)
	{
		// Step 1: we check for temperature
		if (pEnvStatus->CmbSlotInfo[i].fPresent)
			if (CheckSlotTemperature(&SensorState.IOP_Temperature[i],
									pEnvStatus->CmbSlotInfo[i].Temperature,
									pEnvStatus->CmbSlotInfo[i].TemperatureHiThreshold,
									pEnvStatus->CmbSlotInfo[i].TemperatureNormThreshold))												
			{
				// Now send notice to Exception Master
				m_XMContext.Value.Temperature = pEnvStatus->CmbSlotInfo[i].Temperature;
				SENSORCODE SensorCode;
				if (i == CMB_EVC0 || i == CMB_EVC1)
					SensorCode = EXIT_AIR_TEMP;
				else
					SensorCode = SLOT_TEMPERATURE;
				SendException(SensorCode, SensorState.IOP_Temperature[i], i, (TySlot) i);
			} /* if, if */	


		// Step 2: we check for new IOP or deactivated IOP by looking at the fPresent flag
		// Important:  If the IOP state changes from UNKNOWN to NORMAL, that means the system
		// is booting up.  For this case, we do not notify the Exception Master.  For all
		// other state changes, e.g: NORMAL to ALARM (IOP being removed) or ALARM to
		// NORMAL (IOP insertion) we will notify the Exception Master.  This policy is
		// implemented in CheckIOP.
		if (CheckIOP(&SensorState.IOP_State[i], pEnvStatus->CmbSlotInfo[i].fPresent))
		{
			SENSORCODE SensorCode;
			if (SensorState.IOP_State[i] == NORMAL)
				SensorCode = SLOT_INSTALLED;
			else
				SensorCode = SLOT_REMOVED;
			SendException(SensorCode, SensorState.IOP_State[i], i, (TySlot) i);
		
		} /* if */
	} /* for */	

#endif

	return OK;
}
	

/**********************************************************************
// ReturnTableRefreshData -  Returns the data to be refreshed to the 
// EVCStatusTable.  Also notify Exception Master here.
**********************************************************************/
STATUS EVCStatusReporter::ReturnTableRefreshData(void* &rpTableData, U32 &rcbTableData)
{
	if (m_EVCStatusRecord.key < 5)  // stop after a while, don't pollute the display
		Tracef("EVCStatusReporter::ReturnTableRefreshData(), key = %d\n", m_EVCStatusRecord.key);
	m_EVCStatusRecord.key++;
	rpTableData = &m_EVCStatusRecord;
	rcbTableData = sizeof(m_EVCStatusRecord);
	

	return	OK;
}

/**********************************************************************
// GetPHSrid -  Returns the PHS rowid for this record.
**********************************************************************/
rowID EVCStatusReporter::GetPHSrid(void *pEVCStatusRecord)
{
	return ((EVCStatusRecord *) pEVCStatusRecord)->rid;
}


/**********************************************************************
// InitSensorState -  To by pass the boot-up notice to exception master,
// set default values to UNKNOWN.
**********************************************************************/
void EVCStatusReporter::InitSensorState(SENSOR_STATE_RECORD *pSensorStateRecord)
{
	int i;

	for (i=0; i<2; i++)
	pSensorStateRecord->ExitAir[i] = NORMAL;
	
	for (i=0; i<4; i++)
	pSensorStateRecord->FanSpeed[i] = NORMAL;
		
	for (i=0; i<3; i++)
	pSensorStateRecord->InputOK[i] = NORMAL;

	for (i=0; i<3; i++)
	pSensorStateRecord->OutputOK[i] = NORMAL;

	for (i=0; i<3; i++)
	pSensorStateRecord->PrimaryEnable[i] = NORMAL;

	for (i=0; i<3; i++)
	pSensorStateRecord->FanFailOrOverTemp[i] = NORMAL;

	for (i=0; i<2; i++)
	pSensorStateRecord->DCtoDC3Temp[i] = NORMAL;

	for (i=0; i<2; i++)
	pSensorStateRecord->DCtoDC5Temp[i] = NORMAL;

	for (i=0; i<2; i++)
	pSensorStateRecord->DCtoDC12ATemp[i] = NORMAL;

	for (i=0; i<2; i++)
	pSensorStateRecord->DCtoDC12BTemp[i] = NORMAL;

	for (i=0; i<2; i++)
	pSensorStateRecord->DCtoDC12CTemp[i] = NORMAL;

	for (i=0; i<2; i++)
	pSensorStateRecord->DCtoDCEnable[i] = NORMAL;

  	pSensorStateRecord->SMP48Voltage = NORMAL;
  	pSensorStateRecord->DCtoDC3Voltage = NORMAL;
  	pSensorStateRecord->DCtoDC5Voltage = NORMAL;
  	pSensorStateRecord->DCtoDC12Voltage = NORMAL;
  
	for (i=0; i<2; i++)
	pSensorStateRecord->BatteryInstalled[i] = NORMAL;

	for (i=0; i<2; i++)
	pSensorStateRecord->BatteryTemperature[i] = NORMAL;

	for (i=0; i<2; i++)
	pSensorStateRecord->BatteryChargeCurrent[i] = NORMAL;
	
	pSensorStateRecord->SystemPower = NORMAL;

	for (i=0; i<CMB_ADDRESS_MAX; i++)
	pSensorStateRecord->IOP_State[i] = UNKNOWN;

	for (i=0; i<CMB_ADDRESS_MAX; i++)
	pSensorStateRecord->IOP_Temperature[i] = NORMAL;

	return;
}

/**********************************************************************
// CheckSystemPower -- Check if the system is on AC or Battery power.
// Set state to normal if on AC, WARNING if on battery, ALARM if battery
// power is about to exhaust.  Note:  At this time we don't know how
// to tell if the battery is about to run down.
**********************************************************************/
bool EVCStatusReporter::CheckSystemPower(EX_LEVEL *pSensorLevel, 
	S32 Current)  // if negative means discharge
{
	m_XMContext.Value.Current = Current;
	switch(*pSensorLevel)
	{ 
		case NORMAL:  
			if (Current < BATT_CURRENT_DISCHARGE_NTW)
			{	
				*pSensorLevel = WARNING;
				return true;
			}
			break;
				
		case WARNING:
			if (Current >= BATT_CURRENT_DISCHARGE_WTN)
			{	
				*pSensorLevel = NORMAL;
				return true;
			}
			break;
			  		
		default:
			break;
	} /* switch */
	return false; // no state change
}


/**********************************************************************
// CheckBatteryCurrent -- Check battery charging current.  For first
// release, the charging is probably fixed voltage.
**********************************************************************/
bool EVCStatusReporter::CheckBatteryCurrent(EX_LEVEL *pSensorLevel, 
	S32 Current)
{
	m_XMContext.Value.Current = Current;
	switch(*pSensorLevel)
	{ 
		case NORMAL:  
			if (Current > BATT_CURRENT_CHARGE_NTA)
			{	
				*pSensorLevel = ALARM;
				return true;
			}
			break;
				
		case ALARM:
			if (Current < BATT_CURRENT_CHARGE_ATN)
			{	
				*pSensorLevel = NORMAL;
				return true;
			}
			break;
			  		
		default:
			break;
	} /* switch */
	return false; // no state change
}



/**********************************************************************
// CheckDCtoDCTemperature -- Check DC temperature, set state.
// Return true if SensorState changes.
**********************************************************************/
bool EVCStatusReporter::CheckDCtoDCTemperature(EX_LEVEL *pSensorLevel, S32 Temperature)
{
	switch(*pSensorLevel)
	{ 
		case NORMAL:  
			if (Temperature > DC_DC_TEMPERATURE_NTW)
			{	
				*pSensorLevel = WARNING;
				return true;
			}
			break;
				
		case WARNING:
			if (Temperature < DC_DC_TEMPERATURE_WTN)
			{
				*pSensorLevel = NORMAL;
				return true;
			}  
			else
			if (Temperature > DC_DC_TEMPERATURE_WTA)
			{
				*pSensorLevel = ALARM;
				return true;
			}
			break; 	
		  		
		case ALARM:
			if (Temperature < DC_DC_TEMPERATURE_ATW)
			{	
				*pSensorLevel = WARNING;
				return true;
			}
			break;
			  		
		default:
			break;
	} /* switch */
	return false; // no state change
}

/**********************************************************************
// CheckSlotTemperature -- Check Slot temperature, set state.
// Return true if SensorState changes.
**********************************************************************/
bool EVCStatusReporter::CheckSlotTemperature(EX_LEVEL *pSensorLevel,
		S32 Temperature,
		S32	TemperatureHiThreshold,
		S32 TemperatureNormThreshold)
{
	switch(*pSensorLevel)
	{ 
		case NORMAL:  
			if (Temperature > SLOT_TEMPERATURE_NTW ||
				Temperature > TemperatureHiThreshold)
			{	
				*pSensorLevel = WARNING;
				return true;
			}
			break;
				
		case WARNING:
			if (Temperature < SLOT_TEMPERATURE_WTN &&
				Temperature < TemperatureNormThreshold)
			{
				*pSensorLevel = NORMAL;
				return true;
			}  
			else
			if (Temperature > SLOT_TEMPERATURE_WTA)
			{
				*pSensorLevel = ALARM;
				return true;
			}
			break; 	
		  		
		case ALARM:
			if (Temperature < SLOT_TEMPERATURE_ATW)
			{	
				*pSensorLevel = WARNING;
				return true;
			}
			break;
			  		
		default:
			break;
	} /* switch */
	return false; // no state change
}


/**********************************************************************
// CheckBatteryTemperature -- Check battery temperature, set state.
// Return true if SensorState changes.
// We have two set of thresholds depending if the battery is charging
// or discharging.
**********************************************************************/
bool EVCStatusReporter::CheckBatteryTemperature(EX_LEVEL *pSensorLevel,
		S32 Temperature,
		bool Discharge)
{

	m_XMContext.Value.Temperature = Temperature;
	if (Discharge)
	{
		// Battery is discharging
		switch(*pSensorLevel)
		{ 
			case NORMAL:  
				if (Temperature > BATT_TEMPERATURE_DISCHARGE_NTW)
				{	
					*pSensorLevel = WARNING;
					return true;
				}
				break;
				
			case WARNING:
				if (Temperature < BATT_TEMPERATURE_DISCHARGE_WTN)
				{
					*pSensorLevel = NORMAL;
					return true;
				}  
				else
				if (Temperature > BATT_TEMPERATURE_DISCHARGE_WTA)
				{
					*pSensorLevel = ALARM;
					return true;
				}
				break; 	
		  		
			case ALARM:
				if (Temperature < BATT_TEMPERATURE_DISCHARGE_ATW)
				{	
					*pSensorLevel = WARNING;
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
		switch(*pSensorLevel)
		{ 
			case NORMAL:  
				if (Temperature > BATT_TEMPERATURE_NORM_NTW)
				{	
					*pSensorLevel = WARNING;
					return true;
				}
				break;
				
			case WARNING:
				if (Temperature < BATT_TEMPERATURE_NORM_WTN)
				{
					*pSensorLevel = NORMAL;
					return true;
				}  
				else
				if (Temperature > BATT_TEMPERATURE_NORM_WTA)
				{
					*pSensorLevel = ALARM;
					return true;
				}
				break; 	
		  		
			case ALARM:
				if (Temperature < BATT_TEMPERATURE_NORM_ATW)
				{	
					*pSensorLevel = WARNING;
					return true;
				}
				break;
			  		
			default:
				break;
		} /* switch */
		
	} /* else */
	return false; // no state change
}





/**********************************************************************
// CheckIOP -  Step 2: we check for new IOP or deactivated IOP by looking 
// at the fPresent flag Important:  If the IOP state changes from UNKNOWN 
// to NORMAL, that means the system is booting up.  For this case, we do 
// not notify the Exception Master.  For all other state changes, 
// e.g: NORMAL to ALARM (IOP being removed) or ALARM to NORMAL 
// (IOP insertion) we will notify the Exception Master.
// Input:  
**********************************************************************/
bool EVCStatusReporter::CheckIOP(EX_LEVEL *pSensorLevel,
		bool fPresent)
{
	switch(*pSensorLevel)
	{ 
		case UNKNOWN:
			if (fPresent)
				*pSensorLevel = NORMAL;
				return false; // do not notify Exception Master
			break;
				
		case NORMAL:  
			if (!fPresent)
			{	
				*pSensorLevel = ALARM;
				return true;
			}
			break;
				
		case ALARM:
			if (fPresent)
			{	
				*pSensorLevel = NORMAL;
				return true;
			}
			break;
			  		
		default:
			*pSensorLevel = UNKNOWN;  // This can't happen!  We just reset state for now.
			break;
	} /* switch */
	return false; // no state change
}


/**********************************************************
//CheckBool -- Change the SensorLevel according to this
// state machine:
// NORMAL to ALARM:  if good is false
// ALARM to NORM: if good is true
***********************************************************/
bool EVCStatusReporter::CheckBool(EX_LEVEL *pSensorLevel,
		bool good)
{
	switch(*pSensorLevel)
	{ 
		case NORMAL:  
			if (!good)
			{	
				*pSensorLevel = ALARM;
				return true;
			}
			break;
				
		case ALARM:
			if (good)
			{	
				*pSensorLevel = NORMAL;
				return true;
			}
			break;
			  		
		default:
			*pSensorLevel = NORMAL;  // This can't happen!  We just reset state for now.
			break;
	} /* switch */
	return false; // no state change
}


/**********************************************************************
// CheckVoltage -  Change state according to the following state machine
// Input:  pSensorLevel
//         Voltage
//         NTA1, NTA2
//         ATN1, ATN2
//
// NORMAL to ALARM: Voltage < NTA1 || Voltage > NTA2
// ALARM to NORMAL: Voltage > NTA1 && Voltage < NTA2
//
**********************************************************************/
bool EVCStatusReporter::CheckVoltage(EX_LEVEL *pSensorLevel,
		U32	Voltage,
		U32 NTA1,
		U32 NTA2,
		U32	ATN1,
		U32	ATN2)
{
	switch(*pSensorLevel)
	{ 
		case NORMAL:  
			if (Voltage < NTA1 || Voltage > NTA2)
			{	
				*pSensorLevel = ALARM;
				return true;
			}
			break;
				
		case ALARM:
			if (Voltage > ATN1 && Voltage < ATN2)
			{	
				*pSensorLevel = NORMAL;
				return true;
			}
			break;
			  		
		default:
			*pSensorLevel = NORMAL;  // This can't happen!  We just reset state for now.
			break;
	} /* switch */
	return false; // no state change
}


/**********************************************************************
// SendException -  Send an exception to Exception Master
// Input:  
**********************************************************************/
void EVCStatusReporter::SendException(
	SENSORCODE	SensorCode,
	EX_LEVEL	Level,
	U32			SensorNumber,  	// This is an index into Sensor Array if
								// there are more than one sensors.
	TySlot		Slot
)
{
// We disable the notification for now
return;

	REQUESTCODE reqCode;
	Message *pExceptionMasterMsg;
	switch (Level)
	{
		case NORMAL:
			reqCode = XM_NORMAL;
			break;
		
		case WARNING:
			reqCode = XM_WARNING;
			break;
		
		case ALARM:
			reqCode = XM_ALARM;
			break;
		
		default:
			// This can't happen
			Tracef("EVCStatusReporter::SendException.  Level is not NORMAL, WARNING, ALARM\n");
			return;
			break;
	} /* switch */
	
	pExceptionMasterMsg = new Message(reqCode, sizeof(XM_CONTEXT));		
	m_XMContext.SensorCode = SensorCode;
	m_XMContext.SensorNumber = SensorNumber;
	m_XMContext.Level = Level;
	m_XMContext.Slot = Slot;
	// Value is already set by the caller.
	pExceptionMasterMsg->AddPayload(&m_XMContext, sizeof(XM_CONTEXT));
	if (Send(pExceptionMasterMsg, &DiscardReply))
	{
		Tracef("EVCStatusReporter::SendException.  Error when send message to Exception Master.\n");
		delete pExceptionMasterMsg;
	}
	return;
}