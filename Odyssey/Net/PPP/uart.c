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
/*    Ryan Braun															*/
/*                                                                          */
/* DATA STRUCTURES                                                          */
/*                                                                          */
/*    UART_Communication_Mode                                               */
/*                                                                          */
/* FUNCTIONS                                                                */
/*                                                                          */
/*    UART_Init_Port                                                        */
/*    UART_PPP_ISR   														*/
/*	  UART_PPP_Unknown_Int				                                    */
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
/* 11/17/99 RCB                                                             */
/*	Customized for the Odyssey Exar serial ports (16550 compatible)			*/
/*  Used current serial driver routines										*/
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
#include "mdm_defs.h"
#include "tcp_errs.h"
#include "Odyssey_Trace.h"

#define PCLK    3686400      /* Frequency in PCLK pin */

// from tty.h
#define	FIOCINT				1
#define	TTYRX				2

INT			   pppPort;
DV_DEVICE_ENTRY *pppDevice;

INT             UART_Communication_Mode;

/* These global variables are used to keep track of errors. */
UNSIGNED UART_Parity_Error;
UNSIGNED UART_Frame_Error;
UNSIGNED UART_Overrun_Error;
UNSIGNED UART_Unknown_Interrupt;

extern void ttyinit(int port, int baud);
extern void ttyioctl(int port, int cmd, int arg);
extern int ttyin(int port);
extern int ttyout(int port, int data);
extern int ttydcd(int port);
extern void setbaud(int port, int baud);
extern void setdtr(int port, int arg);


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
/****************************************************************************/
STATUS UART_Init_Port(DV_DEVICE_ENTRY *init_ptr)
{

	pppPort = (INT)init_ptr->dev_com_port;
	pppDevice = init_ptr;

	Tracef("Init port %d at %d bps\n", pppPort, init_ptr->dev_baud_rate);
	ttyinit(pppPort, init_ptr->dev_baud_rate);
		
	ttyioctl(pppPort, FIOCINT, TTYRX);
	return (UART_SUCCESS);

}   /* UART_Init_Port */

STATUS UART_Reinit_Port()
{

	Tracef("Init port %d at %d bps\n", pppPort, pppDevice->dev_baud_rate);
	ttyinit(pppPort, pppDevice->dev_baud_rate);
		
	ttyioctl(pppPort, FIOCINT, TTYRX);
	return (UART_SUCCESS);

}

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*    UART_PPP_ISR                                                          */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*    This is the bogus entry function for the ISR that services the UART.  */
/*		It is actually called from the serial driver ISR when there is a	*/
/*		receive interrupt													*/
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*    none                                                                  */
/*                                                                          */
/****************************************************************************/
VOID UART_PPP_ISR (INT intport)
{

   if (intport == pppPort)
   {
	   switch (UART_Communication_Mode)
	   {
    	  case MDM_NETWORK_COMMUNICATION:
        	 pppDevice->dev_receive (pppDevice);
	         break; 
            
    	  case MDM_TERMINAL_COMMUNICATION:
    	     MDM_Receive();
        	 break;
		  /* Do nothing - to be used when other programs want the port */
		  case MDM_OTHER_COMMUNICATION:
	      default:
        	 break;
        }
   }

} /* UART_PPP_ISR */

VOID UART_PPP_Unknown_Int(INT intport)
{

	printf("Unknown PPP Interrupt\n");

	// If the unknown interrupt was on our port, make note of it
	if (intport == pppPort)
		UART_Unknown_Interrupt++;

}	/* UART_PPP_ISR */

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
/****************************************************************************/
CHAR UART_Get_Char (VOID)
{

	CHAR ch;

	if (ttyhit(pppPort))
	{
		ch = (char)ttyin(pppPort);
		printf("[%02x]", ch);
		return (ch);
	}
	else
	{
		printf("No read data\n");
		return(0);
	}

}

/****************************************************************************/
/* FUNCTION                                     .                            */
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
/****************************************************************************/
VOID UART_Put_Char (CHAR c)
{

//	ttyout(pppPort, c);
	ttyD_out(c);

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
/* OUTPUTS                                                                  */
/*                                                                          */
/*    STATUS    :  The status of the detection.                             */
/*                                                                          */
/****************************************************************************/
STATUS UART_Carrier(VOID)
{

	Tracef("UART_Carrier (%d)\n\r", ttydcd(pppPort));
	return (ttydcd(pppPort));
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

	setbaud(pppPort, br);

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
/****************************************************************************/
VOID UART_Set_DTR (INT boole)
{

	Tracef("UART_Set_DTR(%d)\n", boole);
	setdtr(pppPort, boole);
#if 0
   if (boole)
      Write_Register (5, 0xEA);
   else
      Write_Register (5, 0x6A);
#endif

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
