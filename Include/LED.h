/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: LED.h
// 
// Description:
// This file defines macros to turn LEDs on and off on the Odyssey.
// Must have the INCLUDE_ODYSSEY defined.
// #define INCLUDE_ODYSSEY
//
// Update Log 
// 
// 05/14/99 Jim Frandeen: Create file
/*************************************************************************/
#ifndef LED_H
#define LED_H

#include "hw.h"

#define LED_1_GREEN		0
#define LED_1_RED		0X10000

#define LED_2_GREEN		0X20000
#define LED_2_RED		0X30000

#define LED_3_GREEN		0X40000
#define LED_3_RED		0X50000

#define CPU_LED_REGISTER (char *)(CPU_LEDS_KSEG1_ADDR)

#define CPU_LED_1_GREEN		*(CPU_LED_REGISTER + LED_1_GREEN) = 1; \
								*(CPU_LED_REGISTER + LED_1_RED) = 0
#define CPU_LED_1_RED		*(CPU_LED_REGISTER + LED_1_RED) = 1; \
								*(CPU_LED_REGISTER + LED_1_GREEN) = 0
#define CPU_LED_1_YELLOW	*(CPU_LED_REGISTER + LED_1_RED) = 1; \
								*(CPU_LED_REGISTER + LED_1_GREEN) = 1
#define CPU_LED_1_OFF		*(CPU_LED_REGISTER + LED_1_GREEN) = 0; \
								*(CPU_LED_REGISTER + LED_1_RED) = 0
#define CPU_LED_2_GREEN		*(CPU_LED_REGISTER + LED_2_GREEN) = 1; \
								*(CPU_LED_REGISTER + LED_2_RED) = 0
#define CPU_LED_2_RED		*(CPU_LED_REGISTER + LED_2_RED) = 1; \
								*(CPU_LED_REGISTER + LED_2_GREEN) = 0
#define CPU_LED_2_YELLOW	*(CPU_LED_REGISTER + LED_2_RED) = 1; \
								*(CPU_LED_REGISTER + LED_2_GREEN) = 1
#define CPU_LED_2_OFF		*(CPU_LED_REGISTER + LED_2_GREEN) = 0; \
								*(CPU_LED_REGISTER + LED_2_RED) = 0
#define CPU_LED_3_GREEN		*(CPU_LED_REGISTER + LED_3_GREEN) = 1; \
								*(CPU_LED_REGISTER + LED_3_RED) = 0
#define CPU_LED_3_RED		*(CPU_LED_REGISTER + LED_3_RED) = 1; \
								*(CPU_LED_REGISTER + LED_3_GREEN) = 0
#define CPU_LED_3_YELLOW	*(CPU_LED_REGISTER + LED_3_RED) = 1; \
								*(CPU_LED_REGISTER + LED_3_GREEN) = 1
#define CPU_LED_3_OFF		*(CPU_LED_REGISTER + LED_3_GREEN) = 0; \
								*(CPU_LED_REGISTER + LED_3_RED) = 0

#endif // LED_H