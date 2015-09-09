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
// File: DdmLED.cpp
// 
// Description:
// This file is the DdmLED definition.  This class is used to
// control the LED on the HBC/IOP board.
// 
// $Log: /Gemini/Odyssey/DdmLED/DdmLED.cpp $
// 
// 7     11/06/99 2:14p Jlane
// Multiple changes to clean this whole thing up.
// 
// 6     10/28/99 10:18a Sgavarre
// Changes to support BootMgr.
// 
// 5     10/14/99 4:47p Jlane
// Multiple fixes to get working.
// 
// 4     8/30/99 9:11p Iowa
// Add ToggleAllLEDs so Enable doesn't reply twice.
// 
// 08/30/99 Joe Altmaier: ToggleAllLEDs so Enable doesn't reply twice.
//
// 3     8/30/99 2:47p Hdo
// Add Reply(OK) to message handlers methods
// 
// 2     8/27/99 4:27p Jfrandeen
// Remove the condition check on the HBC
// Return the pDdmLED instead of FALSE
// 
// 1     8/20/99 6:21p Hdo
// First check in
// 
// 07/13/99 Huy Do: Create file
/*************************************************************************/
#include "DdmLED.h"
#include "Address.h"
#include "RequestCodes.h"
#include "hw.h"

#include "Trace_Index.h"
#define TRACE_INDEX TRACE_DDM_LED
#include "Odyssey_Trace.h"

#include "BuildSys.h"
CLASSNAME(DdmLED, SINGLE);

#define NO_COLOR				(U8)0x00
#define GREEN					(U8)0x01
#define RED						(U8)0x02
#define AMBER					(U8)0x03
#define LED_ADDRESS				((volatile U8*)CPU_LEDS_ADDR)	// LED address

#define LED_ZERO_POS			0x00
#define LED_ZERO_BITS			((U8)0x03)
#define LED_ZERO_GREEN			(GREEN)
#define LED_ZERO_RED			(RED)
#define LED_ZERO_AMBER			(LED_ZERO_GREEN | LED_ZERO_RED)
#define SET_LED_ZERO_OFF		(*(LED_ADDRESS) &= ~LED_ZERO_BITS)

#define LED_ONE_POS				0x02
#define	LED_ONE_BITS			((U8)0x0C)
#define LED_ONE_GREEN			(GREEN << LED_ONE_POS)
#define LED_ONE_RED				(RED << LED_ONE_POS)
#define LED_ONE_AMBER			(LED_ONE_GREEN | LED_ONE_RED)
#define SET_LED_ONE_OFF			(*(LED_ADDRESS) &= ~LED_ONE_BITS)

#define LED_TWO_POS				0x04
#define	LED_TWO_BITS			((U8)0x30)
#define LED_TWO_GREEN			(GREEN << LED_TWO_POS)
#define LED_TWO_RED				(RED << LED_TWO_POS)
#define LED_TWO_AMBER			(LED_TWO_GREEN | LED_TWO_RED)
#define SET_LED_TWO_OFF			(*(LED_ADDRESS) &= ~LED_TWO_BITS)

// Turn all LEDs off.
//#define SET_LEDS_OFF			(*(LED_ADDRESS) &= ~(LED_ZERO_BITS | LED_ONE_BITS | LED_ONE_BITS))
#define SET_LEDS_OFF			(*(LED_ADDRESS) = NO_COLOR)

// Set the color bits for a given LED # (not preserving existing color bits).
#define SET_LED_BITS(bits)		(*(LED_ADDRESS) |= (bits))

// Set the color bits for a given LED # (not preserving existing color bits).
#define CLR_LED_BITS(bits)		(*(LED_ADDRESS) &= ~(bits))

// Return the Bits to set color #(color) for LED #(number).
#define LED_BITS(number, color)	((color) << ((number)<<1))

// Turn on color for LED #(number) preserving any other existing colors.
#define LED_ON(number, color)	(SET_LED_BITS(LED_BITS((number), (color))))

// Turn off color for LED #(number) preserving any other existing colors.
#define LED_OFF(number, color)	(CLR_LED_BITS(LED_BITS((number), (color))))

// Turn LED #(number) off not preserving any other existing colors.
#define CLR_LED(number)			(LED_OFF((number), AMBER))

// Set LED #(number) to color not preserving any other existing colors.
#define SET_LED(number, color)	{LED_OFF((number), AMBER); LED_ON((number), (color));}


SERVELOCAL (DdmLED, LED_TURN_RED);
SERVELOCAL (DdmLED, LED_TURN_GREEN);
SERVELOCAL (DdmLED, LED_TURN_AMBER);
SERVELOCAL (DdmLED, LED_TOGGLE);
SERVELOCAL (DdmLED, LED_OFF);

// Global References
DdmLED *pDdmLED = NULL;

/*************************************************************************/
// DdmLED
// Constructor method for the class DriveMonitorIsm
/*************************************************************************/
inline DdmLED::DdmLED(DID did) : Ddm(did)
{
	TRACE_ENTRY(DdmLED::DdmLED);
	pDdmLED = this;
}

/*************************************************************************/
// Ctor
// Create a new instance of the Drive Monitor
/*************************************************************************/
inline Ddm *DdmLED::Ctor(DID did)
{
	TRACE_ENTRY(DdmLED::Ctor);

	// Only Zero instance is allowed
	if( pDdmLED )
		return pDdmLED;	// "Should never return FALSE"
	return new DdmLED(did);
}

/*************************************************************************/
// Initialize
// Start up the hardware belonging to this derived class
/*************************************************************************/
STATUS DdmLED::Initialize(Message *pMsg)
{
	TRACE_ENTRY(DdmLED::Initialize);

	DispatchRequest(LED_TURN_RED,	REQUESTCALLBACK (DdmLED, TurnRed));
	DispatchRequest(LED_TURN_GREEN,	REQUESTCALLBACK (DdmLED, TurnGreen));
	DispatchRequest(LED_TURN_AMBER,	REQUESTCALLBACK (DdmLED, TurnAmber));
	DispatchRequest(LED_TOGGLE,		REQUESTCALLBACK (DdmLED, ToggleLEDs));
	DispatchRequest(LED_OFF,		REQUESTCALLBACK (DdmLED, TurnLEDsOFF));

	SET_LEDS_OFF;
	
	return Ddm::Initialize(pMsg);
}

/*************************************************************************/
// Enable
// Start up the DDM belonging to this derived class
/*************************************************************************/
inline STATUS DdmLED::Enable(Message *pMsg)
{
	TRACE_ENTRY(DdmLED::Enable);

	// Now the OS is running, both LEDs will alternating with GREEN
	// until the next command
	m_LED_Color = GREEN;
	m_LED_On = 1;
	m_LED_Off = 0;
		
	// If this IOP is in the right hand bays, start with opposite led. 
	m_fLEDToggleState = !(Address::iSlotMe > IOP_SSDL3);
		
	// And set the LEDs
	ToggleAllLEDs();
		
	return Ddm::Enable(pMsg);
}

/*************************************************************************/
// Quiesce
// Turn off the DDM belonging to this derived class
/*************************************************************************/
inline STATUS DdmLED::Quiesce(Message *pMsg)
{
	TRACE_ENTRY(DdmLED::Quiesce);

	// Should I alternating both LED with AMBER? (normal rate)?
	return Ddm::Quiesce(pMsg);
}

/*************************************************************************/
// TurnRed
// Turn the LED specified by pContext to red
/*************************************************************************/
STATUS DdmLED::TurnRed(Message *pMsg)
{
	TRACE_ENTRY(DdmLED::TurnRed);
	m_LED_Color = RED;
	Reply(pMsg, OK);
	return OK;
}

/*************************************************************************/
// TurnGreen
// Turn the LEDs to green
/*************************************************************************/
STATUS DdmLED::TurnGreen(Message *pMsg)
{
	TRACE_ENTRY(DdmLED::TurnGreen);
	m_LED_Color = GREEN;
	Reply(pMsg, OK);
	return OK;
}

/*************************************************************************/
// TurnAmber
// Turn the LEDs 1 amber
/*************************************************************************/
STATUS DdmLED::TurnAmber(Message *pMsg)
{
	TRACE_ENTRY(DdmLED::TurnAmber);
	m_LED_Color = AMBER;
	Reply(pMsg, OK);
	return OK;
}

/*************************************************************************/
// ToggleLEDs
// Toggle the LEDs
/*************************************************************************/
STATUS DdmLED::ToggleLEDs(Message *pMsg)
{
	TRACE_ENTRY(DdmLED::ToggleLEDs);
	ToggleAllLEDs();
	Reply(pMsg, OK);
	return OK;
}

/*************************************************************************/
// ToggleLEDs
// Toggle the LEDs
/*************************************************************************/
STATUS DdmLED::ToggleAllLEDs()
{
	TRACE_ENTRY(DdmLED::ToggleLEDs);

	// Based on Toggle Flag decide which LEDs are on and off.
	if (m_fLEDToggleState)
	{
		m_LED_On = 0;
		m_LED_Off = 1; 
	}
	else
	{
		m_LED_On = 1;
		m_LED_Off = 0; 
	}
	
	// Set the LEDs accortdingly
	SET_LED( m_LED_On, m_LED_Color );		// Turn one LED on.
	CLR_LED( m_LED_Off);					// Turn off the other.
	
	// Toggle the flag for next time.
	m_fLEDToggleState = !m_fLEDToggleState;
	
	return OK;
}

/*************************************************************************/
// TurnLEDsOFF
// Turn the LEDs off
/*************************************************************************/
STATUS DdmLED::TurnLEDsOFF(Message *pMsg)
{
	TRACE_ENTRY(DdmLED::TurnLEDsOFF);

	*LED_ADDRESS = 0;

	Reply(pMsg, OK);

	return OK;
}
