/*************************************************************************/
/*                                                                       */
/*        Copyright (c) 1998      Accelerated Technology, Inc.           */
/*                                                                       */
/* PROPRIETARY RIGHTS of Accelerated Technology are involved in the      */
/* subject matter of this material.  All manufacturing, reproduction,    */
/* use, and sales rights pertaining to this subject matter are governed  */
/* by the license agreement.  The recipient of this software implicitly  */
/* accepts the terms of the license.                                     */
/*                                                                       */
/*************************************************************************/

/*************************************************************************/
/*                                                                       */
/* FILE NAME                                            VERSION          */
/*                                                                       */
/*      urt_extr.h                                      PPP   1.1        */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      UART                                                             */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains function prototypes and structure definitions */
/*      used by the UART  module of PPP  and which are also accessible   */
/*      to other moodules.                                               */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      Glen Johnson,     Accelerated Technology, Inc.                   */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      none                                                             */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*      none                                                             */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*      NAME            DATE           REMARKS                           */
/*      George Clark    01/26/98       Updated for ARM                   */
/*      S. Nguyen       02/20/98       IDT4640 using UART 85C30          */
/*      S. Nguyen       08/26/98       Re-organized directory structure  */
/*                                                                       */
/*************************************************************************/

#include "nucleus.h" /* SNN */

/* Check to see if the file has been included already.  */

#ifndef URT_EXTR_H
#define URT_EXTR_H

#ifdef __cplusplus
extern "C" {
#endif

/* Function Prototypes for UART.C */
STATUS  UART_Init_Port (DV_DEVICE_ENTRY *init_ptr);
STATUS	UART_Reinit_Port();
VOID    UART_PPP_ISR(INT intport);
VOID	UART_PPP_Unknown_Int(INT intport);
CHAR    UART_Get_Char (VOID);
VOID    UART_Put_Char (CHAR c);
STATUS  UART_Carrier (VOID);
VOID    UART_Set_Baud_Rate (uint32  baud_rate);
VOID    UART_Set_DTR (INT boole);
VOID    UART_Change_Communication_Mode (INT mode);

/* Return values from the UART functions. */
#define UART_SUCCESS        NU_SUCCESS
#define UART_FAILURE        -1
#define UART_BAD_COM_PORT   -2

#define COM1                3
#define COM2                2
#define COM3				4

/* Definitions for the number of stop bits. */
#define STOP_BITS_1         0x04 /* From WR4 */
#define STOP_BITS_2         0x0C

/* Definitions for the number of data bits. */
#define DATA_BITS_8         0xC0 /* From WR2 */
#define DATA_BITS_7         0x40
#define DATA_BITS_6         0x80
#define DATA_BITS_5         0x00

/* Definitions for the parity. */
#define PARITY_NONE         0x00
#define PARITY_ODD          0x00
#define PARITY_EVEN         0x01

#ifdef __cplusplus
}
#endif

#endif
