// changed to galileo z85c30 eval code (rcb 4/1/99)

/****************************************************************************/
/*                                                                          */
/*      Copyright (c) 1998 by Accelerated Technology, Inc.                  */
/*                                                                          */
/* PROPRIETARY RIGHTS of Accelerated Technology are involved in the subject */
/* matter of this material.  All manufacturing, reproduction, use and sales */
/* rights pertaining to this subject matter are governed by the license     */
/* agreement.  The recipient of this software implicity accepts the terms   */
/* of the license.                                                          */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/*                                                                          */
/* FILENAME                                                 VERSION         */
/*                                                                          */
/*    UART.C                                                 1.1            */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*    This file contains the UART specific functions.  The functions in     */
/*    this file will have to be modified for each UART used.                */
/*                                                                          */
/* AUTHOR                                                                   */
/*                                                                          */
/*    Glen Johnson                                                          */
/*                                                                          */
/* DATA STRUCTURES                                                          */
/*                                                                          */
/*    UART_Communication_Mode                                               */
/*                                                                          */
/* FUNCTIONS                                                                */
/*                                                                          */
/*    UART_Init_Port                                                        */
/*    UART_Port_To_Vector                                                   */
/*    UART_LISR                                                             */
/*    UART_Get_Char                                                         */
/*    UART_Put_Char                                                         */
/*    UART_Carrier                                                          */
/*    UART_Set_Baud_Rate                                                    */
/*    UART_Set_DTR                                                          */
/*    UART_Change_Communication_Mode                                        */
/*                                                                          */
/* DEPENDENCIES                                                             */
/*                                                                          */
/*    nucleus.h                                                             */
/*    target.h                                                              */
/*    externs.h                                                             */
/*    protocol.h                                                            */
/*    ppp_defs.h                                                            */
/*    ppp_extr.h                                                            */
/*    mdm_extr.h                                                            */
/*    urt_defs.h                                                            */
/*    urt_extr.h                                                            */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*    NAME             DATE              REMARKS                            */
/*    S. Nguyen        02/20/98          Initial version for IDT4640        */
/*    S. Nguyen        04/21/98          For VNET 4.0                       */
/*    S. Nguyen        08/26/98          Re-organized directory structure   */
/*                                                                          */
/****************************************************************************/
/*                            UART 85C30                                    */
/****************************************************************************/

#include "nucleus.h"
#include "target.h"
#include "externs.h"
#include "protocol.h"
#include "ppp_defs.h"
#include "ppp_extr.h"
#include "mdm_extr.h"
#include "urt_defs.h"
#include "urt_extr.h"
#ifdef VER3_2
#else
#include "mdm_defs.h"
#include "tcp_errs.h"
#endif

#define PCLK    3686400      /* Frequency in PCLK pin */

UNSIGNED_CHAR   *port;

INT             UART_Communication_Mode;

/* These global variables are used to keep track of errors. */
UNSIGNED UART_Parity_Error;
UNSIGNED UART_Frame_Error;
UNSIGNED UART_Overrun_Error;
UNSIGNED UART_Unknown_Interrupt;

/* Local function prototypes */
VOID Delay (VOID);
VOID Write_Register (UNSIGNED_CHAR offset_add, UNSIGNED_CHAR data);

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*    UART_Init_Port                                                        */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*    This function intializes the COM port that will be used for PPP       */
/*    communications.                                                       */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*    NU_Etopen                                                             */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*    NU_Control_Interrupts                                                 */
/*    UART_Set_Baud_Rate                                                    */
/*    INBYTE                  - This is a macro                             */
/*    OUTBYTE                 - This is a macro                             */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*    PPP_INIT *  :   PPP initalization structure.                          */
/*                                                                          */
/* OUTPUTS                                                                  */
/*                                                                          */
/*    STATUS      :   Returns UART_SUCCESS if successful initialization,    */
/*                    else a negative value is returned.                    */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*    NAME          DATE      REMARKS                                       */
/*                                                                          */
/*    G. Johnson    1/9/97    Created Initial version.                      */
/*                                                                          */
/****************************************************************************/
#ifdef VER3_2
STATUS UART_Init_Port(PPP_INIT *init_ptr)
#else
STATUS UART_Init_Port(DV_DEVICE_ENTRY *init_ptr)
#endif
{
   INT old_state;
#ifdef VER3_2
#else
   INT  vector;
   VOID (*NU_old_vect_routine)(INT); 

   vector = UART_Port_To_Vector ((INT)init_ptr->dev_com_port);
   if (vector == UART_BAD_COM_PORT)
      return (-1);

   init_ptr->dev_vect = vector;

   /* Register the ISR that will handle interrupts generated by the UART. */
   if (NU_Register_LISR (vector, UART_LISR, &NU_old_vect_routine) != NU_SUCCESS)
   {
      NU_Tcp_Log_Error (DRV_RESOURCE_ALLOCATION_ERROR, TCP_FATAL,
                          __FILE__, __LINE__);
      return(-1);
   }


#endif
#ifdef VER3_2
   switch (init_ptr->sl_com_port)
#else
   switch (init_ptr->dev_com_port)
#endif   
   {
      case COM1:
         port = (void *)COM1_BASE_ADDRESS;
         Write_Register (9, 0x80);  /* Channel A reset */
         break;

      case COM2:
         port = (void *)COM2_BASE_ADDRESS;
#ifdef INCLUDE_UDB

         /* WW9 is shared by both UDB and PPP.  UDB has
            already initialized channel B.  Another
            attempt to initialize this channel will clear
            all values, and thus will cause a transmission
            error */
#else
         Write_Register (9, 0x40);  /* Channel B reset */
#endif
         break;

      default:
         return (UART_BAD_COM_PORT);
   }

   /* disable interrupts */
   old_state = NU_Control_Interrupts(NU_DISABLE_INTERRUPTS);

   /* Modes */

   Write_Register (0, 0xC0);  /* reset Tx underrun latch */ 
   Write_Register (4, 0x44);  /* 16x clock, 1 stop bit */
   Write_Register (1, 0x00);  /* disable interrupts */
   Write_Register (2, 0x00);  /* interrupt vector */ 
   Write_Register (3, 0xC0);  /* Rx 8 bits, Rx disabled */
   Write_Register (5, 0x60);  /* Tx 8 bits, Tx disabled */
   Write_Register (6, 0x00);  /* sync character */
   Write_Register (7, 0x00);  /* sync character */
#ifdef INCLUDE_UDB
#else
   Write_Register (9, 0x02);  /* no vector */
#endif
   Write_Register (10, 0x00); /* NRZ mode */
   Write_Register (11, 0x50); /* use BR generator */
#ifdef VER3_2
   UART_Set_Baud_Rate (init_ptr->sl_baud_rate);
#else
   UART_Set_Baud_Rate (init_ptr->dev_baud_rate);
#endif   
   Write_Register (14, 0x80); /* BRG source = RTxC input */
   /* Enables */
   Write_Register (14, 0x81); /* enable BRG */
   Write_Register (3, 0xC1);  /* enable Rx */
   Write_Register (5, 0xEA);  /* DRT, Tx 8 bits and Tx enable, RTS */
   /* Interrupt Status */
   Write_Register (15, 0x00); /* disable external interrupts */
   Write_Register (0, 0x10);  /* reset ext/stat interrupts */
   Write_Register (0, 0x10);  /* reset ext/stat interrupts */
   Write_Register (1, 0x10);  /* Rx interrupt enable */
#ifdef INCLUDE_UDB
#else
   Write_Register (9, 0x0A);  /* master interrupt enable */
#endif

   /* The PPP driver will begin in terminal mode. */
#ifdef VER3_2
   UART_Communication_Mode = PPP_TERMINAL_COMMUNICATION;
#else
   UART_Communication_Mode = MDM_TERMINAL_COMMUNICATION;
#endif

   /* Initialize the error counters. */
   UART_Parity_Error      = 0;
   UART_Frame_Error       = 0;
   UART_Overrun_Error     = 0;
   UART_Unknown_Interrupt = 0;

   /* enable interrupts */
   NU_Control_Interrupts(old_state);

   return (UART_SUCCESS);

}   /* UART_Init_Port */


/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*    UART_Port_To_Vector                                                   */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*    This function converts a COM port number into the Interrupt vector    */
/*    number used by that COM port.                                         */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*    NU_Etopen                                                             */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*    none                                                                  */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*    INT         :   A COM port identifier                                 */
/*                                                                          */
/* OUTPUTS                                                                  */
/*                                                                          */
/*    INT         :   An interrupt vector number.                           */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*    NAME          DATE      REMARKS                                       */
/*                                                                          */
/*    G. Johnson    1/9/97    Created Initial version.                      */
/*                                                                          */
/****************************************************************************/
INT  UART_Port_To_Vector (INT c_port)
{
   switch (c_port)
   {
      case COM1:
         return (COM1_INT_VECTOR);

      case COM2:
         return (COM2_INT_VECTOR);

      default:
         return (UART_BAD_COM_PORT);
   }

} /* end UART_Port_To_Vector */

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*    UART_LISR                                                             */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*    This is the entry function for the ISR that services the UART.        */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*    none                                                                  */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*    NU_Control_Interrupts                                                 */
/*    PPP_Receive                                                           */
/*    MDM_Receive                                                           */
/*    INBYTE                                                                */
/*    OUTBYTE                                                               */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*    INT         :   Interrupt vector                                      */
/*                                                                          */
/* OUTPUTS                                                                  */
/*                                                                          */
/*    none                                                                  */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*    NAME          DATE      REMARKS                                       */
/*                                                                          */
/*    G. Johnson    1/9/97    Created Initial version.                      */
/*                                                                          */
/****************************************************************************/
VOID UART_LISR (INT vector)
{
   INT             err_status;             /* Error status - lsr       */
   INT             old_level;              /* Old interrupt level      */
   UNSIGNED_CHAR   status;
#ifdef VER3_2
#else
   DV_DEVICE_ENTRY *device;
#endif   

   status = INBYTE (port + OFFS_CNTL);
   Delay();

   if (status & RxFULL)
   {
#ifdef VER3_2         
      switch (UART_Communication_Mode)
      {
         case PPP_NETWORK_COMMUNICATION:
            PPP_Receive();
            break;

         case PPP_TERMINAL_COMMUNICATION:
         default:
            MDM_Receive();
            break;
      }
#else
      switch (UART_Communication_Mode)
      {
         case MDM_NETWORK_COMMUNICATION:
            device = DEV_Get_Dev_For_Vector (vector);
            device->dev_receive (device);
            break; 
            
         case MDM_TERMINAL_COMMUNICATION:
         default:
            MDM_Receive();
            break;
      }
#endif      
   }
   else
   {
      /* This should not occur because only the receive and error interrupts
         were enabled during initialization. */

      /* read the register to clear the interrupt */
      UART_Unknown_Interrupt++;
   }

} /* UART_LISR */

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*    UART_Get_Char                                                         */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*    This function reads the last received character from the UART.        */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*    PPP_Receive                                                           */
/*    MDM_Receive                                                           */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*    INBYTE      -  This is a macro                                        */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*    INT         :   Interrupt vector                                      */
/*                                                                          */
/* OUTPUTS                                                                  */
/*                                                                          */
/*    CHAR        :  Character read                                         */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*    NAME          DATE      REMARKS                                       */
/*                                                                          */
/*    G. Johnson    1/9/97    Created Initial version.                      */
/*                                                                          */
/****************************************************************************/
CHAR UART_Get_Char (VOID)
{
   UNSIGNED_CHAR status;
   UNSIGNED_CHAR alpha;

   status = INBYTE (port + OFFS_CNTL);
   Delay();
   if (status & RxFULL)
   {
      alpha = INBYTE (port + OFFS_DATA);
      Delay();
   }
   return (alpha & 0xFF);

}

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*    UART_Put_Char                                                         */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*    This writes a character out to the serial port.                       */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*    NU_Xmit_Packet                                                        */
/*    MDM_Control_String                                                    */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*    INBYTE       -  This is a macro                                       */
/*    OUTBYTE      -  This is a macro                                       */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*    CHAR        :   Character tp to be written to the serial port.        */
/*                                                                          */
/* OUTPUTS                                                                  */
/*                                                                          */
/*    none                                                                  */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*    NAME          DATE      REMARKS                                       */
/*                                                                          */
/*    G. Johnson    1/9/97    Created Initial version.                      */
/*                                                                          */
/****************************************************************************/
VOID UART_Put_Char (CHAR c)
{
   INT old_state;

   while (!(INBYTE (port + OFFS_CNTL) & TxEMPTY))
      Delay();

   /* disable interrupts */
   old_state = NU_Control_Interrupts (NU_DISABLE_INTERRUPTS);

   /* Send the character. */
   OUTBYTE (port + OFFS_DATA, c & 0xFF);
   Delay();

   /* enable interrupts */
   NU_Control_Interrupts (old_state);

}

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*    UART_Carrier                                                          */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*    This function checks for a carrier.                                   */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*    MDM_Hangup                                                            */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*    INBYTE    -  This is a macro                                          */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*    none                                                                  */
/*                                                                          */
/* OUTPUTS                                                                  */
/*                                                                          */
/*    STATUS    :  The status of the detection.                             */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*    NAME          DATE      REMARKS                                       */
/*                                                                          */
/*    G. Johnson    1/9/97    Created Initial version.                      */
/*                                                                          */
/****************************************************************************/
STATUS UART_Carrier(VOID)
{
   INT status;

   status = INBYTE (port + OFFS_CNTL);
   Delay();

   return (status & MdmDCD ? NU_TRUE : NU_FALSE); 
}

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*    UART_Set_Baud_Rate                                                    */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*    This function sets the UART buad rate.                                */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*    UART_Init_Port                                                        */
/*    MDM_Dial                                                              */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*    OUTBYTE   -  This is a macro                                          */
/*    NU_Control_Interrupts                                                 */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*    uint32   :  The new baud rate.                                        */
/*                                                                          */
/* OUTPUTS                                                                  */
/*                                                                          */
/*    none                                                                  */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*    NAME          DATE      REMARKS                                       */
/*                                                                          */
/*    G. Johnson    1/9/97    Created Initial version.                      */
/*                                                                          */
/****************************************************************************/
VOID UART_Set_Baud_Rate (uint32  br)
{
   INT tc;  /* time constant, divisor */
   INT old_state;

   if (br == 0)
      return;

   tc = (PCLK / (2 * 16 * br)) - 2;

   old_state = NU_Control_Interrupts (NU_DISABLE_INTERRUPTS);

   Write_Register (12, tc & 0xFF);
   Write_Register (13, (tc >> 8) & 0xFF);

   NU_Control_Interrupts (old_state);

} /* UART_Set_Baud_Rate */

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*    UART_Set_DTR                                                          */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*    This function changes the DTR (Data Terminal Ready) state to either   */
/*    true or false.                                                        */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*    MDM_Hangup                                                            */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*    OUTBYTE  -  This is a macro                                           */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*    INT      :  A boolean, either TRUE or FALSE                           */
/*                                                                          */
/* OUTPUTS                                                                  */
/*                                                                          */
/*    none                                                                  */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*    NAME          DATE      REMARKS                                       */
/*                                                                          */
/*    G. Johnson    1/9/97    Created Initial version.                      */
/*                                                                          */
/****************************************************************************/
VOID UART_Set_DTR (INT bool)
{
   if (bool)
      Write_Register (5, 0xEA);
   else
      Write_Register (5, 0x6A);

} /* UART_Set_DTR */

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*    UART_Change_Communication_Mode                                        */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*    This function switches the serial port between terminal mode and      */
/*    PPP mode.  The Mode affects where incomming characters are directed.  */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*    MDM_Change_Communication_Mode                                         */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*    none                                                                  */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*    INT      :  The mode of operation desired.                            */
/*                                                                          */
/* OUTPUTS                                                                  */
/*                                                                          */
/*    none                                                                  */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*    NAME          DATE      REMARKS                                       */
/*                                                                          */
/*    G. Johnson    1/9/97    Created Initial version.                      */
/*                                                                          */
/****************************************************************************/
VOID UART_Change_Communication_Mode (INT mode)
{

   UART_Communication_Mode = mode;

} /* UART_Change_Communication_Mode */

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*    Delay                                                                 */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*    none                                                                  */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */     
/*    none                                                                  */
/* OUTPUTS                                                                  */
/*                                                                          */
/*    none                                                                  */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*    NAME         DATE      REMARKS                                       */
/*                                                                          */
/*    S. Nguyen    2/10/98    Created Initial version.                      */
/*                                                                          */
/****************************************************************************/
VOID Delay (VOID)
{
   int i;
   for (i = 0; i < 10; i++)
      ;
}

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*    Write_Register                                                        */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*    Shorten repetitive function calls                                     */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*    none                                                                  */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */     
/*    none                                                                  */
/*                                                                          */
/* OUTPUTS                                                                  */
/*                                                                          */
/*    none                                                                  */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*    NAME         DATE       REMARKS                                       */
/*                                                                          */
/*    S. Nguyen    2/20/98    Created initial version.                      */
/*                                                                          */
/****************************************************************************/
VOID Write_Register (UNSIGNED_CHAR offset_addr, UNSIGNED_CHAR data)
{
   OUTBYTE (port + OFFS_CNTL, offset_addr);
   Delay();
   OUTBYTE (port + OFFS_CNTL, data);
   Delay();
}
