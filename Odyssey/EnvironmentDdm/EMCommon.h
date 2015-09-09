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
// File: EMCommon.h
// 
// Description:
// 	This file is common data structure for the ExceptionMaster.
// 
// $Log: /Gemini/Odyssey/ExceptionMaster/EMCommon.h $
// 
// 9     9/07/99 5:58p Hdo
// Add XM_LEVEL back in the XM_CONTEXT
// 
// 8     9/07/99 4:10p Vnguyen
// Add sensor code PRIMARY_SUPPLY_ENABLE
// 
// 7     9/07/99 1:59p Vnguyen
// Add SYSTEM_POWER and BATTERY_CHARGE_CURRENT sensor codes.
// 
// 6     9/03/99 1:59p Vnguyen
// Add SLOT_INSTALLED and SLOT_REMOVE sensor code.
// 
// 5     9/02/99 11:50a Vnguyen
// Add Sensor code BATTERY_REMOVED, clean up other sensor codes, also
// clean up XM_CONTEXT structure and removed unused variables.
// 
// 4     8/31/99 2:34p Vnguyen
// Add EX_LEVEL UNKNOWN
// 
// 3     8/31/99 11:56a Vnguyen
// Add XM_CMD for BATTERY_DISCHARGE,  rename BATTERY_CURRENT_ERROR to
// BATTERY_CHARGE
// 
// 2     8/25/99 4:32p Vnguyen
// Add BATTERY_TEMPERATURE, XM_CMD_VOLTAGE_ERROR
// 
// 1     8/25/99 1:55p Hdo
// First check in
// 
// 8/14/99 Huy Do: Create file
// 8/23/99 Huy Do: Add IOP_TEMP and TySlot
/*************************************************************************/

#ifndef __EMCommon_H__
#define __EMCommon_H__

#include "CtTypes.h"
#include "Address.h"

typedef enum {
	NORMAL = 0,  // need to be 0 for init via memset
	WARNING,
	ALARM,
	UNKNOWN	// use for special case, no call to EM if goes to NORMAL
} EX_LEVEL;

typedef enum {
	EXIT_AIR_TEMP,				// (part of CMB_SLOT, indexed by CMB_EVC0, CMD_EVC1)
	FAN_SPEED,					// 4 fans
	PRIMARY_SUPPY_INPUT,		// 3 supplies
	PRIMARY_SUPPLY_OUTPUT,		// 3 supplies
	PRIMARY_SUPPLY_ENABLE,		// 3 supplies
	SYSTEM_POWER,				// WARNING: on battery, ALARM: about to run down
	FANFAIL_OR_OVERTEMPERATURE,	// 3 supplies
	AUX_SUPPLY_3_TEMPERATURE,	// 2 supplies
	AUX_SUPPLY_5_TEMPERATURE,	// 2 supplies
	AUX_SUPPLY_12A_TEMPERATURE,	// 2 supplies
	AUX_SUPPLY_12B_TEMPERATURE,	// 2 supplies	
	AUX_SUPPLY_12C_TEMPERATURE,	// 2 supplies	
	AUX_SUPPLY_ENABLE,			// 2 supplies
	SMP_VOLTAGE_48,
	DC_VOLTAGE_3,				// 3.3 V
	DC_VOLTAGE_5,				
	DC_VOLTAGE_12,
	BATTERY_INSTALLED,			// 2 batteries, XM_NORMAL
	BATTERY_REMOVED,			// 2 batteries, XM_ALARM
	BATTERY_TEMPERATURE,		// 2 batteries
	BATTERY_CHARGE_CURRENT,		// 2 batteries, ALARM means current > limit
	SLOT_INSTALLED,				// Could be IOP or DDH, XM_NORMAL
	SLOT_REMOVED,				// Could be IOP or DDH, XM_ALARM
   	SLOT_TEMPERATURE		 	// Could be IOP or DDH
} SENSORCODE;


typedef struct {
	SENSORCODE	SensorCode;
	EX_LEVEL	Level;
	U32			SensorNumber;  	// This is an index into Sensor Array if
								// there are more than one sensors.
	TySlot		Slot;
	union {
		S32		Temperature;	// The actual units for these values are defined
		U32		Speed;			// in EVCStatusRecord.h
		U32		Voltage;
		S32		Current;		
	} Value;
} XM_CONTEXT;

#endif	// __EMCommon_H__
