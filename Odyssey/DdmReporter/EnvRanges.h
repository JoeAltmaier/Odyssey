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
// This places keeps the default low/hi ranges for Environmental Values
// 
// Update Log: 
//
// $Log: /Gemini/Odyssey/DdmReporter/EnvRanges.h $
// 
// 3     9/09/99 11:21a Vnguyen
// Complete checking environmental values for Exception Master.
// 
// 2     9/03/99 5:13p Vnguyen
// Add more defines for EVC variables.  Clarify the IOP Temperature rules.
// 
// 1     8/31/99 12:37p Vnguyen
// Initial check-in.
// 
// 
/*************************************************************************/

#ifndef __EnvRanges_h
#define __EnvRanges_h

// NTW means change state from NORMAL to WARNING
// WTN                         WARNING to NORMAL
// WTA						   WARNING to ALARM
// ATW						   ALARM to WARNING


// IMPORTANT:  For IOP Temperature (EVC, IOP, DDH, etc) to change state between
// NORMAL, WARNING, and ALARM, we use the threshold values returned from the
// CMB Master: TemperatureHiThreshold and TemperatureNormThreshold.  However,
// the Temperature is subject to the following system wide limits.

// SLOT (degree C) starts at normal state
#define SLOT_TEMPERATURE_NTW	70	// >
#define SLOT_TEMPERATURE_WTN	63	// <
#define SLOT_TEMPERATURE_WTA	80	// >
#define SLOT_TEMPERATURE_ATW 	72	// <

// Here is the state transition rule for Temperature:
// NTW:  Temperature >= TemperatureHiThreshold or > SLOT_TEMPERATURE_NTW
// WTN:  Temperature <= TemperatureNormThreshold and < SLOT_TEMPERATURE_WTN
// WTA:  Temperature > SLOT_TEMPERATURE_WTA
// ATW:  Temperature < SLOT_TEMPERATURE_ATW

// The Exception Master is expected to figure out the fan speed for the entire
// system given that each IOP may be in different temperature state.
// EM should wire-or the higher fan speed.

// FANSPEED (RPM) starts at normal
#define FANSPEED_NTA	10	// <					
#define FANSPEED_ATN 	100	// >					

// fInputOK											

// fOutputOK										

// fFanFailOrOverTemp

// fPrimaryEnable									

// fDCtoDCEnable									

// KeyPosition

// (degree C) starts at normal
#define DC_DC_TEMPERATURE_NTW	70	// >		
#define DC_DC_TEMPERATURE_WTN	63	// <		
#define DC_DC_TEMPERATURE_WTA	100	// >		
#define DC_DC_TEMPERATURE_ATW 	90	// <		

// (0.1 Volt) starts at normal state
// This range is wide enough to cover float and charge
// 13.5 * 4 = 54, 13.8 * 4 = 55.2 (float)
// 14.4 * 4 = 57.6, 15 * 4 = 60 (cycle)
#define SMP48V_NORM_NTA1	530	// < ||			
#define SMP48V_NORM_NTA2	630	// >			
#define SMP48V_NORM_ATN1	540	// > &&			
#define SMP48V_NORM_ATN2 	600 // <			

// (0.1 Volt) starts at normal state
// 1.75 * 6 * 4 = 42 (10% left)
#define SMP48V_DISCHARGE_NTA1	420	// < ||	
#define SMP48V_DISCHARGE_NTA2	630	// >	
#define SMP48V_DISCHARGE_ATN1	540	// > &&	
#define SMP48V_DISCHARGE_ATN2 	600 // <	

// (0.001 Volt) starts at normal
#define DC3_NTA1	3000	// < ||				
#define DC3_NTA2	3600	// >				
#define DC3_ATN1	3100	// > &&				
#define DC3_ATN2	3500	// <				

// (0.01 Volt) starts at normal
#define DC5_NTA1	475		// < ||				
#define DC5_NTA2	540		// >				
#define DC5_ATN1	485		// > &&				
#define DC5_ATN2	530		// <				

// (0.01 Volt) starts at normal
#define DC12_NTA1	1140	// < ||				
#define DC12_NTA2	1280	// >				
#define DC12_ATN1	1160	// > &&				
#define DC12_ATN2	1260	// <				

// (0.001 A) starts at normal
#define BATT_CURRENT_CHARGE_NTA	5000 // >				
#define BATT_CURRENT_CHARGE_ATN	4500 // <				

#define BATT_CURRENT_DISCHARGE_NTW	-100 // <			
#define BATT_CURRENT_DISCHARGE_WTN	0	 // >=			

// (degree C) starts at normal
#define BATT_TEMPERATURE_NORM_NTW	45	// >
#define BATT_TEMPERATURE_NORM_WTN	42	// <
#define BATT_TEMPERATURE_NORM_WTA	50	// >
#define BATT_TEMPERATURE_NORM_ATW	47	// <		

// (degree C) starts at normal
#define BATT_TEMPERATURE_DISCHARGE_NTW	55	// >
#define BATT_TEMPERATURE_DISCHARGE_WTN	51	// <
#define BATT_TEMPERATURE_DISCHARGE_WTA	60	// >
#define BATT_TEMPERATURE_DISCHARGE_ATW	56	// <		

// 
#define BATT_PRESENT_TEMPERATURE	0	// battery presents if temperature is hotter than this

#endif // __EnvRanges_h
