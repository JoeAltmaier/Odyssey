/****************************************************************************/
/*                                                                          */
/*      Copyright (c) 1999 by Accelerated Technology, Inc.                  */
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
/* FILENAME                                                                 */
/*                                                                          */
/*    DEC21143.C                                                            */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*    This file contains all of the source code required to  interface with */
/*    a DEC21143.  It handles PCI card location, initialization, transmit,  */
/*    receive, and error processing. NOTE: this driver only supports one    */
/*    PCI bus.                                                              */
/*                                                                          */
/* AUTHOR                                                                   */
/*                                                                          */
/*    Uriah T. Pollock,  Accelerated Technology Inc.                        */
/*                                                                          */
/* DATA STRUCTURES                                                          */
/*                                                                          */
/*     NU_HISR                          DEC21143_RX_HISR_CB                 */
/*     NU_HISR                          DEC21143_TX_HISR_CB                 */
/*     NU_HISR                          DEC21143_Negotiate_Link_HISR_CB     */
/*     DV_DEVICE_ENTRY                  *DEC21143_TX_Device_Buffer[]        */
/*     INT                              DEC21143_Read                       */
/*     INT                              DEC21143_Write                      */
/*     DV_DEVICE_ENTRY                  *DEC21143_Negotiate_Device_Buffer[] */
/*     INT                              DEC21143_Negotiate_Read             */
/*     INT                              DEC21143_Negotiate_Write            */
/*     Check in file 'dec21143.h'                                           */
/*                                                                          */
/* FUNCTIONS                                                                */
/*                                                                          */
/*     DEC21143_Init                                                        */
/*     DEC21143_Open                                                        */
/*     DEC21143_Negotiate_Link                                              */
/*     DEC21143_Init_Link                                                   */
/*     DEC21143_Negotiate_Link_HISR                                         */
/*     DEC21143_Get_Address                                                 */
/*     DEC21143_RX_Packet                                                   */
/*     DEC21143_RX_HISR                                                     */
/*     DEC21143_TX_Packet                                                   */
/*     DEC21143_TX_HISR                                                     */
/*     DEC21143_LISR                                                        */
/*     DEC21143_Ioctl                                                       */
/*     DEC21143_Delay                                                       */
/*     DEC21143_Allocate_Descriptor                                         */
/*     DEC21143_Find_PCI_ID                                                 */
/*     DEC21143_Write_PCI                                                   */
/*     DEC21143_Read_PCI                                                    */
/*     DEC21143_Read_EEPROM                                                 */
/*     DEC21143_Init_Setup_Frame                                            */
/*     DEC21143_Process_Setup_Frame                                         */
/*     DEC21143_Update_Setup_Frame                                          */
/*     Unknown_Int_Lisr                                                     */
/*                                                                          */
/* DEPENDENCIES                                                             */
/*                                                                          */
/*     "dev.h"                                                              */
/*     "externs.h"                                                          */
/*     "protocol.h"                                                         */
/*     "tcp_errs.h"                                                         */
/*     "snmp.h"                                                             */
/*     "data.h"                                                             */
/*     "dec21143.h"                                                         */
/*     "net.h"                                                              */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*       NAME                 DATE            REMARKS                       */
/*                                                                          */
/*  Uriah T. Pollock        01/12/99      Created initial version 1.0       */
/*                                                                          */
/****************************************************************************/
#define	PLUS	1
#include "nucleus.h"
#include "externs.h"
#include "protocol.h"
#include "tcp_errs.h"
#include "types.h"
#if SNMP_INCLUDED
#include "xtypes.h"
#include "snmp.h"
#endif
#include "data.h"
#include "dec21143.h"
#include "net.h"
//#include "types.h"
#include "pcidev.h"
#include "pcimap.h"
#include "mii.h"
#include "mips_util.h"

#define	NET_SUPPORT 1
#ifndef	NET_SUPPORT
NU_EVENT_GROUP    Buffers_Available;
#endif
/* MII Support functions */
extern	U32 mdio_read(U32 io_addr, U32 phy_addr, U32 reg);
extern	void mdio_write(U32 io_addr, U32 phy_addr, U32 reg, U32 value);
/* Prototypes */
uint32	getset_ether_base_addr(uint32 pci_id, int instance);
void	DEC_IO_Write_Reg(uint32	io_addr, uint32 val);
uint32	DEC_IO_Read_Reg(uint32	io_addr);
void	DEC21143_Regdump(DV_DEVICE_ENTRY *device);
void	PHY_Regdump(DV_DEVICE_ENTRY *device);
void	Reset_Phy6611(DV_DEVICE_ENTRY *device);
void	Link_Status_Print(DV_DEVICE_ENTRY *device, uint32 use_100MB,
							uint32 use_full_duplex);
uint32	Is10BaseT(DV_DEVICE_ENTRY *device);
STATUS	MIIinit_Phy(DV_DEVICE_ENTRY *device);
/* Receive, Transmit, and Negotiate HISR Control Blocks. */
NU_HISR DEC21143_RX_HISR_CB;
NU_HISR DEC21143_TX_HISR_CB;
NU_HISR DEC21143_Negotiate_Link_HISR_CB;

/* This ring buffer is used for communication between the LISR and the transmit
   HISR. The LISR only writes to the buffer and the HISR only reads from the
   buffer.  Whenever a transmit complete interrupt occurs and there is another
   packet pending transmission the LISR will write a device pointer into the
   location pointed to by DEC21143_Write.  The LISR will then activate the
   transmit HISR.  The HISR will retrieve the device pointer from the location
   pointed to by ISR_Read, move the read index forward, and transmit the next
   packet.  Worst case, there will be an active HISR for each device. So there
   should be at least one location per DEC21143 device in the ring buffer. The
   size of the ring buffer can be optimized by modifying MAX_DEC21143_DEVICES.
*/
#define         MAX_DEC21143_DEVICES  5
DV_DEVICE_ENTRY *DEC21143_TX_Device_Buffer[MAX_DEC21143_DEVICES];
INT             DEC21143_Read;
INT             DEC21143_Write;

DV_DEVICE_ENTRY *DEC21143_Negotiate_Device_Buffer[MAX_DEC21143_DEVICES];
INT             DEC21143_Negotiate_Read;
INT             DEC21143_Negotiate_Write;

char *ether_err_mess[] = {
			"Parity",
			"Master Abort",
			"Target Abort",
			"Unknown",
			"Unknown",
			"Unknown",
			"Unknown"};

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*  DEC21143_Init                                                           */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*  This function initializes the device structure and calls the device     */
/*  open function.                                                          */
/*                                                                          */
/* AUTHOR                                                                   */
/*                                                                          */
/*    Uriah T. Pollock,  Accelerated Technology Inc.                        */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*    DEV_Init_Devices                                                      */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*    DEC21143_Open                                                         */
/*    NU_Allocate_Memory                                                    */
/*    UTL_Zero                                                              */
/*    NU_Tcp_Log_Error                                                      */
/*    sizeof                                                                */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*    DV_DEVICE_ENTRY * : Pointer to the device to be initialized.          */
/*                                                                          */
/* OUTPUTS                                                                  */
/*                                                                          */
/*    NU_SUCCESS or a negative value on failure.                            */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*       NAME                 DATE            REMARKS                       */
/*                                                                          */
/*  Uriah T. Pollock        01/12/99      Created initial version 1.0       */
/*                                                                          */
/****************************************************************************/
STATUS DEC21143_Init(DV_DEVICE_ENTRY *device)
{
    VOID            *pointer;

    /*  Initialize the various function pointers. */
    device->dev_open       = DEC21143_Open;
    device->dev_start      = DEC21143_TX_Packet;
#ifdef	NET_SUPPORT
    device->dev_output     = NET_Ether_Send;
    device->dev_input      = NET_Ether_Input;
#endif
    device->dev_ioctl      = DEC21143_Ioctl;
    device->dev_type       = DVT_ETHER;

    /* The IO base address is gotten below in the function
       DEC21143_Open.
    device->dev_io_addr    = ;
    */

    device->dev_addrlen    = DEC21143_ETHERNET_ADDRESS_SIZE;
    device->dev_hdrlen     = DEC21143_ETHERNET_HEADER_SIZE;
    device->dev_mtu        = DEC21143_ETHERNET_MTU;

    /* The DEC21143 is a simplex controller.  Broadcasts are allowed. */
    device->dev_flags |= (DV_BROADCAST | DV_SIMPLEX | DV_MULTICAST);

    /* Allocate the memory required for the DEC21143 data. */
    if ( NU_Allocate_Memory ( &System_Memory, &pointer, sizeof(DEC21143_XDATA),
                                  NU_NO_SUSPEND ) != NU_SUCCESS)
    {
        NU_Tcp_Log_Error (TCP_SESS_MEM, TCP_FATAL, __FILE__, __LINE__);
        return (-1);
    }

    /* Clear the data. */
    UTL_Zero (pointer, sizeof(DEC21143_XDATA));

    /* The device structure includes 4 fields that are unused. Two are reserved
       for system use, and two are for reserved for users.  We are going to use
       one of the fields reserved for users to store a pointer to the data for
       this DEC21143 device. */
    device->user_defined_1 = (uint32)pointer;

    /*  Initialize the device.  */
    return ( (*(device->dev_open)) (device->dev_mac_addr, device) );

} /* DEC21143_Init */

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*  DEC21143_Open                                                           */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*    This function will handle the initilization of the Ethernet H/W you   */
/*    are using for communication.  This function will also register an     */
/*    LISR with the kernel.  The LISR will service all interrupts generated */
/*    by the DEC21143. All steps listed in comments below are taken from    */
/*    page 4-30 of the DIGITAL Semiconductor 21143 PCI/CardBus 10/100-Mb/s  */
/*    Ethernet LAN Controller Hardware Reference Manual.                    */
/*                                                                          */
/* AUTHOR                                                                   */
/*                                                                          */
/*    Uriah T. Pollock,  Accelerated Technology Inc.                        */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*    DEC21143_Init                                                         */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*    OUTDW                                                                 */
/*    INDW                                                                  */
/*    NU_Tcp_Log_Error                                                      */
/*    DEC21143_Find_PCI_ID                                                  */
/*    DEC21143_Read_PCI                                                     */
/*    NU_Register_LISR                                                      */
/*    NU_Allocate_Memory                                                    */
/*    NU_Create_HISR                                                        */
/*    DEC21143_Delay                                                        */
/*    DEC21143_Set_RXCFG                                                    */
/*    NU_Register_LISR                                                      */
/*    NU_Create_HISR                                                        */
/*    DEC21143_Write_PCI                                                    */
/*    sizeof                                                                */
/*    DEC21143_Get_Address                                                  */
/*    DEC21143_Delay                                                        */
/*    DEC21143_Allocate_Descriptor                                          */
/*    DEC21143_Init_Setup_Frame                                             */
/*    DEC21143_Process_Setup_Frame                                          */
/*    MEM_Buffer_Dequeue                                                    */
/*    DEC21143_Negotiate_Link                                               */
/*    NU_Sleep                                                              */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*    uchar *           : Pointer the the storage area for the MAC address. */
/*    DV_DEVICE_ENTRY * : Pointer to the device to be initialized.          */
/*                                                                          */
/* OUTPUTS                                                                  */
/*                                                                          */
/*    NU_SUCCESS is returned if no errors occur, otherwise -1 is returned.  */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*       NAME                 DATE            REMARKS                       */
/*                                                                          */
/*  Uriah T. Pollock        01/12/99      Created initial version 1.0       */
/*                                                                          */
/****************************************************************************/
STATUS DEC21143_Open (uchar *ether_addr, DV_DEVICE_ENTRY *device)
{
    NET_BUFFER          *setup_frame;
    STATUS              ret_status;
    VOID                *pointer;
    uint8               eeprom_data[128];
    uint8               x;
    uint32              csr6_val;
    DEC21143_XDATA      *dec_data;
    VOID                (*DEC21143_old_vect_routine)(INT);
    static INT          open_count = 0;
	INT					instance;

	instance = open_count;
    /* Point at the DEC21143 xtended data. */
    dec_data = (DEC21143_XDATA *)device->user_defined_1;

    /* Collect some SNMP data */
    //SNMP_ifDescr(device->dev_index, "DEC21143 Ethernet: Software revision 1.0");
    //SNMP_ifType(device->dev_index, 6);
    //SNMP_ifMtu(device->dev_index, device->dev_mtu);

    /* First thing, find the card and store off its PCI ID. */
    /* On the Odyssey with 2 MACs, init the OOB enet before internal */
    /* On eval, init in order */
#ifdef INCLUDE_ODYSSEY
    if ( open_count == 0)
    	dec_data->DEC21143_PCI_ID = DEC21143_Find_PCI_ID (1);
    else if (open_count == 1)
    	dec_data->DEC21143_PCI_ID = DEC21143_Find_PCI_ID (0);
    else
    	dec_data->DEC21143_PCI_ID = DEC21143_Find_PCI_ID (open_count);
#else
	dec_data->DEC21143_PCI_ID = DEC21143_Find_PCI_ID (open_count);
#endif

    /* Make sure the card was found. */
    if (dec_data->DEC21143_PCI_ID == 0x0)
    {
        /* Log the error and get out. */
        NU_Tcp_Log_Error (DRV_RESOURCE_ALLOCATION_ERROR, TCP_FATAL, __FILE__, __LINE__);

        return (-1);
    }


    /* Read the chip revision. Step 2.A */
    dec_data->DEC21143_Revision =
                    DEC21143_Read_PCI (dec_data->DEC21143_PCI_ID, PCI_CFRV);


#ifdef	INCLUDE_ODYSSEY
	/* Get the Interrupt Info from the PCI Configuration Space */
	device->dev_irq  = pciconf_readb(dec_data->DEC21143_PCI_ID, PCI_CONF_ILINE);
#endif
	/* Int Vector is same as Int line for Us */
	device->dev_vect = device->dev_irq;

    /* read old, load new */
    NU_Register_LISR (device->dev_vect, DEC21143_LISR,
                        &DEC21143_old_vect_routine);

    /*  The HISR can be shared by all DEC21143 devices. So only create one. */
    if (open_count++ == 0)
    {

        /* create the HISR for handling the interrupt from the Ethernet card */
        ret_status = NU_Allocate_Memory (&System_Memory, &pointer, 2000,
                                        NU_NO_SUSPEND);
        if (ret_status != NU_SUCCESS)
        {
            NU_Tcp_Log_Error (TCP_SESS_MEM, TCP_FATAL, __FILE__, __LINE__);
            return (-1);
        }

        pointer = (void *)ALIGN((uint32)pointer, 8);

        ret_status = NU_Create_HISR (&DEC21143_RX_HISR_CB, "ETHER_RX",
                                    DEC21143_RX_HISR,
                                    0, pointer, 2000);
        if (ret_status != NU_SUCCESS)
        {
            NU_Tcp_Log_Error (TCP_HRDWR_INIT, TCP_FATAL, __FILE__, __LINE__);
            return (-1);
        }

        /* create the HISR for transmitting packets */
        if (NU_Allocate_Memory (&System_Memory, &pointer, 2000,
                                NU_NO_SUSPEND) != NU_SUCCESS)
        {
            NU_Tcp_Log_Error (TCP_SESS_MEM, TCP_FATAL, __FILE__, __LINE__);
            return (-1);
        }

        pointer = (void *)ALIGN((uint32)pointer, 8);

        if (NU_Create_HISR (&DEC21143_TX_HISR_CB, "ETHER_TX",
                            DEC21143_TX_HISR, 0, pointer, 2000) != NU_SUCCESS)
        {
            NU_Tcp_Log_Error (TCP_HRDWR_INIT, TCP_FATAL, __FILE__, __LINE__);
            return (-1);
        }

        /* create the HISR for renegotiating the link */
        if (NU_Allocate_Memory (&System_Memory, &pointer, 2000,
                                NU_NO_SUSPEND) != NU_SUCCESS)
        {
            NU_Tcp_Log_Error (TCP_SESS_MEM, TCP_FATAL, __FILE__, __LINE__);
            return (-1);
        }

        pointer = (void *)ALIGN((uint32)pointer, 8);

        if (NU_Create_HISR (&DEC21143_Negotiate_Link_HISR_CB, "ETHER_NL",
                            DEC21143_Negotiate_Link_HISR, 0, pointer, 2000) != NU_SUCCESS)
        {
            NU_Tcp_Log_Error (TCP_HRDWR_INIT, TCP_FATAL, __FILE__, __LINE__);
            return (-1);
        }

    }

    /* After the reset wake the chip up. */
    DEC21143_Write_PCI (dec_data->DEC21143_PCI_ID, PCI_CFDD, CLEAR_ALL);

#ifdef	INCLUDE_ODYSSEY
	/* For Odyssey , we need Memory Access and Bus Master */
    DEC21143_Write_PCI (dec_data->DEC21143_PCI_ID, PCI_CFCS,
                                        (CFCS_MEMORY_SPACE_ACCESS   |
                                            CFCS_MASTER_OPERATION   |
                                            CFCS_PARITY_ERROR_RESPONSE));
	/* Get the Device I/O Address from PCI Configuration Space */
	device->dev_io_addr = pciconf_readl(dec_data->DEC21143_PCI_ID,
							PCI_CONF_BASE1) & PCI_BASE_MEMADDR_M;
#else
    /* Set the chip to accept register accesses and to have the ability
       to take master operation of the PCI bus. Step 2.D */
    DEC21143_Write_PCI (dec_data->DEC21143_PCI_ID, PCI_CFCS,
                                        (CFCS_IO_SPACE_ACCESS   |
                                            CFCS_MASTER_OPERATION   |
                                            CFCS_PARITY_ERROR_RESPONSE));
	device->dev_io_addr = getset_ether_base_addr(dec_data->DEC21143_PCI_ID,
												instance);
#endif

    /* Set the cache line size to zero and the latency timer to max,
       0xFF. Step 2.E */
    DEC21143_Write_PCI (dec_data->DEC21143_PCI_ID, PCI_CFLT, CFLT_INITIALIZE);


    /* Read in the entire EEPROM */
    for (x = 0; x < (sizeof (eeprom_data) / 2); x++)
        ((uint16 *)eeprom_data) [x] = DEC21143_Read_EEPROM
                                    (device->dev_io_addr, x);

    /* Get the physical address from the EEPROM data just read in
       and store it in the device structure. */
    DEC21143_Get_Address(eeprom_data, ether_addr);
	{
		int i;

#if 0
		ether_addr[0] = 0x00;
		ether_addr[1] = 0x01;
		ether_addr[2] = 0x02;
		ether_addr[3] = 0x03;
		ether_addr[4] = 0x04;
		ether_addr[5] = instance;
#endif
		printf("\nMAC Address --- ");
		for(i=0; i < 6; i++)
			printf("%02X:", ether_addr[i]);
		printf("\n");
	}

    /* Disable all interrupts from the DEC chip. */
    OUTDW (device->dev_io_addr + CSR7, CLEAR_ALL);

    /* Select the port before chip reset */

    /* Read the register. */
    csr6_val = INDW (device->dev_io_addr + CSR6);

    /* Turn off the transmitter and receiver and port select. */
    csr6_val &= ~(CSR6_START_TX | CSR6_START_RX | CSR6_PORT_SELECT);

    /* Write it to the register. */
    OUTDW (device->dev_io_addr + CSR6, csr6_val);

    /* Software reset of the chip, this does not reset the port
       select done above. */
    OUTDW (device->dev_io_addr + CSR0, CLEAR_ALL);
    DEC21143_Delay();
    OUTDW (device->dev_io_addr + CSR0, CSR0_SOFTWARE_RESET);
    DEC21143_Delay();
    OUTDW (device->dev_io_addr + CSR0, CLEAR_ALL);
    DEC21143_Delay();

#ifdef	INCLUDE_EV64120
    /* Reset the 6611 PHY. The 6611 chip has a bug because of
	 * which we need to issue this Reset - Sudhir Kasargod */
	Reset_Phy6611(device);
#endif
#ifdef	INCLUDE_ODYSSEY
	if ( MIIinit_Phy(device) != NU_SUCCESS ) {
		printf("Error Initializing the PHY\n\r");
		return(-1);
	}
#endif



    /* Now make sure there aren't any other bits set that we do not want. */
    csr6_val = INDW (device->dev_io_addr + CSR6);

    csr6_val &= (~(CSR6_PASS_BAD_FRAMES | CSR6_PASS_ALL_MULTICAST |
                    CSR6_FORCE_COLLISION_MODE | CSR6_IGNORE_DST_ADDRESS_MSB |
                    CSR6_RX_ALL | CSR6_TX_THRESHOLD_MODE |
                    CSR6_PROMISCUOUS_MODE));

    /* Write it to the register. */
    OUTDW (device->dev_io_addr + CSR6, csr6_val);

    /* After the reset wake the chip up. */
    DEC21143_Write_PCI (dec_data->DEC21143_PCI_ID, PCI_CFDD, CLEAR_ALL);

#ifdef	INCLUDE_ODYSSEY
	/* For Odyssey , we need Memory Access and Bus Master */
    DEC21143_Write_PCI (dec_data->DEC21143_PCI_ID, PCI_CFCS,
                                        (CFCS_MEMORY_SPACE_ACCESS   |
                                            CFCS_MASTER_OPERATION   |
                                            CFCS_PARITY_ERROR_RESPONSE));
#else
    /* Set the chip to accept register accesses and to have the ability
       to take master operation of the PCI bus. Step 2.D */
    DEC21143_Write_PCI (dec_data->DEC21143_PCI_ID, PCI_CFCS,
                                        (CFCS_IO_SPACE_ACCESS   |
                                            CFCS_MASTER_OPERATION   |
                                            CFCS_PARITY_ERROR_RESPONSE));
#endif

    /* Set the cache line size to zero and the latency timer to max,
       0xFF. Step 2.E */
    DEC21143_Write_PCI (dec_data->DEC21143_PCI_ID, PCI_CFLT, CFLT_INITIALIZE);

    /* Set the cache alignment and burst length and reset all other
       bits. Step 3 */
    OUTDW (device->dev_io_addr + CSR0, (CSR0_CACHE_ALIGNMENT_16 |
                                        CSR0_BURST_LENGTH_16 | CSR0_DBO ));

    /* Turn on the interrupts that we want to handle and mask those
       that we don't. Step 4*/
#ifdef	INCLUDE_ODYSSEY
    OUTDW (device->dev_io_addr + CSR7, DEC21143_INTERRUPTS
								 | CSR7_GEN_PURP_PORT_ENABLE );
#else
    OUTDW (device->dev_io_addr + CSR7, DEC21143_INTERRUPTS);
#endif

    /* Initialize the RX buffer descriptor and set its base address.
       Step 5 */
    dec_data->DEC21143_First_RX_Descriptor =
    dec_data->DEC21143_Current_RX_Descriptor =
                    DEC21143_Allocate_Descriptor (DEC21143_MAX_RX_DESCRIPTORS,
                                                    &System_Memory,
                                                    NU_TRUE);

    /* Make sure the allocation worked. */
    if (dec_data->DEC21143_First_RX_Descriptor)

        /* Store the base address. */
        OUTDW (device->dev_io_addr + CSR3, addr_vtopci((uint32) (dec_data->DEC21143_First_RX_Descriptor)));
    else
    {
        /* Log the error and get out. */
        NU_Tcp_Log_Error (DRV_RESOURCE_ALLOCATION_ERROR, TCP_FATAL, __FILE__, __LINE__);

        return (-1);
    }

    /* Initialize the TX buffer descriptor and set its base address.
       Step 5 */
    dec_data->DEC21143_First_TX_Descriptor =
    dec_data->DEC21143_Current_TX_Descriptor =
                    DEC21143_Allocate_Descriptor (DEC21143_MAX_TX_DESCRIPTORS,
                                                    &System_Memory,
                                                    NU_FALSE);

    /* Make sure the allocation worked. */
    if (dec_data->DEC21143_First_TX_Descriptor)

        /* Store the base address. */
        OUTDW (device->dev_io_addr + CSR4, addr_vtopci((uint32) (dec_data->DEC21143_First_TX_Descriptor)));
    else
    {
        /* Log the error and get out. */
        NU_Tcp_Log_Error (DRV_RESOURCE_ALLOCATION_ERROR, TCP_FATAL, __FILE__, __LINE__);

        return (-1);
    }

    /* Create the initial setup frame used for filtering RX frames. Make
       sure this worked. */
    if (DEC21143_Init_Setup_Frame (ether_addr, setup_frame =
            MEM_Buffer_Dequeue (&MEM_Buffer_Freelist)) != NU_SUCCESS)
    {
        /* This is a fatal error. */
        NU_Tcp_Log_Error (DRV_RESOURCE_ALLOCATION_ERROR, TCP_FATAL, __FILE__, __LINE__);

        return (-1);
    }

    /* Now that we have created the setup frame
       give it to the DEC chip for processing. */
    ret_status = DEC21143_Process_Setup_Frame (device, setup_frame);

    /* Make sure we were able to process the frame. */
    if (ret_status != NU_SUCCESS)
    {
        /* This is a fatal error. */
        NU_Tcp_Log_Error (DRV_RESOURCE_ALLOCATION_ERROR, TCP_FATAL, __FILE__, __LINE__);

        return (-1);
    }

    /* Turn on the transmitter so that the setup frame can be proccessed. */
    csr6_val |= CSR6_START_TX;

    /* Write it to the register. */
    OUTDW (device->dev_io_addr + CSR6, csr6_val);

    /* Step 6 is currently not needed. */

    /* Select the port to use, speed, and the duplex. Step 7 - last step */
    ret_status = DEC21143_Negotiate_Link (device, &csr6_val);


    if (ret_status == NU_SUCCESS)
    {
        device->dev_flags |= DV_UP;
    }

    return (ret_status);

}  /* end DEC21143_Open */

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*  DEC21143_Negotiate_Link                                                 */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*    This function will start the autonegotiation of the link and wait for */
/* its completion or for a timeout to occur.                                */
/*                                                                          */
/* AUTHOR                                                                   */
/*                                                                          */
/*    Uriah T. Pollock,  Accelerated Technology Inc.                        */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*    DEC21143_Open                                                         */
/*    DEC21143_Negotiate_Link_HISR                                          */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*    OUTDW                                                                 */
/*    INDW                                                                  */
/*    NU_Retrieve_Clock                                                     */
/*    DEC21143_Init_Link                                                    */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*    DV_DEVICE_ENTRY * : Pointer to the device to be initialized.          */
/*    uint32 *          : Pointer to the current CSR6 value.                */
/*                                                                          */
/* OUTPUTS                                                                  */
/*                                                                          */
/*    NU_SUCCESS is returned if no errors occur, otherwise -1 is returned.  */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*       NAME                 DATE            REMARKS                       */
/*                                                                          */
/*  Uriah T. Pollock        01/12/99      Created initial version 1.0       */
/*                                                                          */
/****************************************************************************/
STATUS DEC21143_Negotiate_Link (DV_DEVICE_ENTRY *device, uint32 *csr6_val)
{
    STATUS      ret_status;
    UNSIGNED    timeout;
    uint32      val;
	DEC21143_XDATA      *dec_data;
#ifndef INCLUDE_ODYSSEY
	uint32      csr12_val;
#endif
    /* Autonegotiation, avertising half-duplex, full-duplex,
       10BASE-T, and 100BASE-TX. */

	val = 0;
	dec_data = (DEC21143_XDATA *)device->user_defined_1;
#ifdef	INCLUDE_ODYSSEY
	/* Read the PHY Status Register */
	val = mdio_read(device->dev_io_addr+CSR9, 1, PHY_STAT_OFF);
    	/* Make sure the port select is not set. */
    *csr6_val &= (~(CSR6_PORT_SELECT));
    OUTDW (device->dev_io_addr + CSR6, *csr6_val);

	OUTDW (device->dev_io_addr + CSR13, CLEAR_ALL);
	OUTDW (device->dev_io_addr + CSR14, CLEAR_ALL);

	*csr6_val |= ( 1 << 9);
	OUTDW (device->dev_io_addr + CSR6, *csr6_val);

	*csr6_val |= ( 1 << 18);
	OUTDW (device->dev_io_addr + CSR6, *csr6_val);
	/* If the Link is Down Start Auto-Negotiation */
	if ( !(val & PHY_STAT_LINKUP)) {

		/* Start Auto Negotiation with MII PHY */
		mdio_write(device->dev_io_addr+CSR9, 1, MII_CNTL_OFF,
								MII_CNTL_DEFAULT | MII_CNTL_ANEG_START);

    	ret_status =  -1;
    	/* Get the current clock value. */
    	timeout = NU_Retrieve_Clock();

    	/* Add to it the negotiation timeout value. */
    	timeout += DEC21143_NEOGITATION_TIMEOUT;

    	/* Now wait for negotiation to complete, or for it to timeout. */
    	do
    	{
			DEC21143_Delay();
			/* Read the PHY Status Register */
			val = mdio_read(device->dev_io_addr+CSR9, 1, PHY_STAT_OFF);
			if ( val == 0xFFFF) {
				val = 0;
				continue;
			}

    	} while (((val & PHY_STAT_ANEG_DONE) == 0) &&
                (timeout > NU_Retrieve_Clock()));
	}
	if ( val & PHY_STAT_LINKUP) {
		/* Depnding Upon PHY program the MAC for Half or Full Duplex */
		if ( val & PHY_STAT_FULLDUP )
			*csr6_val |= ( 1 << 9);
		else
			*csr6_val &= ~( 1 << 9);
		OUTDW (device->dev_io_addr + CSR6, *csr6_val);
		Link_Status_Print(device, val & PHY_STAT_100, val & PHY_STAT_FULLDUP);
    	ret_status = NU_SUCCESS;
	} else {
		printf("%s : Link Down\n\r", device->dev_net_if_name);
    		ret_status = NU_SUCCESS;
	}

#else
    /* Make sure the port select is not set. */
    *csr6_val &= (~(CSR6_PORT_SELECT));
    OUTDW (device->dev_io_addr + CSR6, *csr6_val);

    /* Turn on the PCS function for the symbolic PHY. */
    OUTDW (device->dev_io_addr + CSR6, (*csr6_val | CSR6_PCS_FUNCTION ));

    /* Reset the SIA */
    OUTDW (device->dev_io_addr + CSR13, CLEAR_ALL);

    /* Turn on everything that we want to advertise to the link
       partner as well as a few other things needed to get
       negotiation going. */

    OUTDW (device->dev_io_addr + CSR14, (CSR14_ENCODER_ENABLE           |
                                         CSR14_LOOPBACK_ENABLE          |
                                         CSR14_DRIVER_ENABLE            |
                                         CSR14_LINK_PULSE_SEND_ENABLE   |
                                         CSR14_NORMAL_COMPENSATION_MODE |
                                         CSR14_10BASET_HD_ENABLE        |
                                         CSR14_AUTONEGOTIATION_ENABLE   |
                                         CSR14_RECEIVE_SQUELCH_ENABLE   |
                                         CSR14_COLLISION_SQUELCH_ENABLE |
                                         CSR14_COLLISION_DETECT_ENABLE  |
                                         CSR14_SIGNAL_QUALITY_ENABLE    |
                                         CSR14_LINK_TEST_ENABLE         |
                                         CSR14_AUTOPOLARITY_ENABLE      |
                                         CSR14_SET_POLARITY_PLUS        |
                                         CSR14_10BASET_AUI_AUTOSENSING  |
                                         CSR14_100BASE_TX_HD            |
                                         CSR14_100BASE_TX_FD            |
                                         CSR14_100BASE_T4));



    /* Set the SIA to 10BASE-T mode and wake it up. */
    OUTDW (device->dev_io_addr + CSR13, CSR13_SIA_RESET);
    /* Get the current clock value. */
    timeout = NU_Retrieve_Clock();

    /* Add to it the negotiation timeout value. */
    timeout += DEC21143_NEOGITATION_TIMEOUT;

    /* Now wait for negotiation to complete, or for it to timeout. */
    do
    {
        /* Read CSR12. It contains the status of the negotiation. */
        csr12_val = INDW (device->dev_io_addr + CSR12);

    } while (((csr12_val & CSR12_AUTONEGOTIATION_COMPLETE) !=
                CSR12_AUTONEGOTIATION_COMPLETE) &&
                (timeout > NU_Retrieve_Clock()));
    ret_status = DEC21143_Init_Link (device, csr6_val);
#endif


    /* Make sure the link was correctly negotiated. */
    if (ret_status == NU_SUCCESS)
    {
        /* Turn on the receiver. */
        *csr6_val |= CSR6_START_RX;
        OUTDW (device->dev_io_addr + CSR6, *csr6_val);

        /* Set the current status to operational. */
        //SNMP_ifAdminStatus(device->dev_index, 1);
        //SNMP_ifOperStatus(device->dev_index, 1);

        /* Initialize the ring buffer pointers. */
        DEC21143_Read = DEC21143_Write = 0;

	}
	/* Save CSR6 for later use. */
   	dec_data->DEC21143_Saved_CSR6 = *csr6_val;
    /* Set the initialization complete flag. */
    dec_data->DEC21143_Init_Completed = NU_TRUE;

    return (ret_status);

} /* end DEC21143_Negotiate_Link */

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*  DEC21143_Init_Link                                                      */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*    This function will read the current link status and initialize it     */
/* based on the link partners code word or on the link status.              */
/*                                                                          */
/* AUTHOR                                                                   */
/*                                                                          */
/*    Uriah T. Pollock,  Accelerated Technology Inc.                        */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*    DEC21143_Negotiate_Link                                               */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*    OUTDW                                                                 */
/*    INDW                                                                  */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*    DV_DEVICE_ENTRY * : Pointer to the device to be initialized.          */
/*    uint32 *          : Pointer to the current CSR6 value.                */
/*                                                                          */
/* OUTPUTS                                                                  */
/*                                                                          */
/*    NU_SUCCESS is returned if no errors occur, otherwise -1 is returned.  */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*       NAME                 DATE            REMARKS                       */
/*                                                                          */
/*  Uriah T. Pollock        01/12/99      Created initial version 1.0       */
/*                                                                          */
/****************************************************************************/
STATUS DEC21143_Init_Link (DV_DEVICE_ENTRY *device, uint32 *csr6_val)
{
    uint32  csr14_val, csr12_val;
    uint16  link_code_word;
    uint32   use_full_duplex, use_100MB;

    /* Read CSR12 */
    csr12_val = INDW (device->dev_io_addr + CSR12);

    /* Get the link partners code word. It is in the upper 16 bits
       of CSR12. */
#ifdef	NUCLEUS_FOLKS_SHIFTED_16_I_DONT_KNOW_WHY
    link_code_word = ((csr12_val >> 16) & 0x00FF);
#else
	link_code_word = ((csr12_val >> 21) & 0x00FF);
#endif
    /* Did negotiation complete and we have a link code or did we timeout? */
    if (((csr12_val & CSR12_AUTONEGOTIATION_COMPLETE) ==
                            CSR12_AUTONEGOTIATION_COMPLETE) &&
                            (link_code_word != NU_NULL))
    {

        /* Read in CSR14, this is used below. */
        csr14_val = INDW (device->dev_io_addr + CSR14);

        /* Check for all valid settings. */

        /* 100BASE-TX Full Duplex */
        if (((link_code_word & CSR12_CODE_100MB_FD) == CSR12_CODE_100MB_FD) &&
            ((csr14_val & CSR14_100BASE_TX_FD) == CSR14_100BASE_TX_FD))
        {
            use_full_duplex = NU_TRUE;

            use_100MB = NU_TRUE;
        }

        /* 100BASE-TX Half Duplex */
        else if (((link_code_word & CSR12_CODE_100MB_HD) == CSR12_CODE_100MB_HD) &&
            ((csr14_val & CSR14_100BASE_T4) == 0x0) &&
            ((csr14_val & CSR14_100BASE_TX_FD) == 0x0) &&
            ((csr14_val & CSR14_100BASE_TX_HD) == CSR14_100BASE_TX_HD))
        {
            use_full_duplex = NU_FALSE;

            use_100MB = NU_TRUE;
        }

        /* 100BASE-TX Half Duplex */
        else if (((link_code_word & CSR12_CODE_100MB_HD) == CSR12_CODE_100MB_HD) &&
            ((csr14_val & CSR14_100BASE_TX_HD) == CSR14_100BASE_TX_HD))
        {
            use_full_duplex = NU_FALSE;

            use_100MB = NU_TRUE;
        }

        /* 10BASE-T Full Duplex */
        else if (((link_code_word & CSR12_CODE_10MB_FD) == CSR12_CODE_10MB_FD) &&
            ((csr14_val & CSR14_100BASE_T4) == 0x0) &&
            ((csr14_val & CSR14_100BASE_TX_FD) == 0x0) &&
            ((csr14_val & CSR14_100BASE_TX_HD) == 0x0) &&
            ((*csr6_val & CSR6_FULL_DUPLEX_MODE) == CSR6_FULL_DUPLEX_MODE))
        {
            use_full_duplex = NU_TRUE;

            use_100MB = NU_FALSE;
        }

        /* 10BASE-T Full Duplex */
        else if (((link_code_word >> 1) == (CSR12_CODE_10MB_FD >> 1)) &&
            ((*csr6_val & CSR6_FULL_DUPLEX_MODE) == CSR6_FULL_DUPLEX_MODE))
        {
            use_full_duplex = NU_TRUE;

            use_100MB = NU_FALSE;
        }

        /* 10BASE-T Half Duplex */
        else if (((link_code_word & CSR12_CODE_10MB_HD) == CSR12_CODE_10MB_HD) &&
            ((csr14_val & CSR14_100BASE_T4) == 0x0) &&
            ((csr14_val & CSR14_100BASE_TX_FD) == 0x0) &&
            ((csr14_val & CSR14_100BASE_TX_HD) == 0x0) &&
            ((csr14_val & CSR14_10BASET_HD_ENABLE) == CSR14_10BASET_HD_ENABLE) &&
            ((*csr6_val & CSR6_FULL_DUPLEX_MODE) == 0x0))
        {
            use_full_duplex = NU_FALSE;

            use_100MB = NU_FALSE;
        }

        /* 10BASE-T Half Duplex */
        else if ((link_code_word == CSR12_CODE_10MB_HD) &&
            ((csr14_val & CSR14_10BASET_HD_ENABLE) == CSR14_10BASET_HD_ENABLE))
        {
            use_full_duplex = NU_FALSE;

            use_100MB = NU_FALSE;
        }
        else {
            /* We have nothing to go on for initializing the link.
               Return with an error. */
            return (-1);
        }

        /* Now that we know what the link partner can do, initialize the
           chip. */
        Link_Status_Print(device, use_100MB, use_full_duplex);

        /* Do we do 100MB? */
        if (use_100MB)
        {
            /* Reset the SIA */
            OUTDW (device->dev_io_addr + CSR13, CLEAR_ALL);

            OUTDW (device->dev_io_addr + CSR14, CLEAR_ALL);

            /* Preserve CSR6s value and add in the functions we
               need to 100MB mode. */
            *csr6_val |= (CSR6_PORT_SELECT |
                     CSR6_PCS_FUNCTION | CSR6_SCRAMBLER_MODE  |
                     CSR6_HEARTBEAT_DISABLE | CSR6_CAPTURE_EFFECT_ENABLE);

            /* Do we need to add in full duplex. */
            if (use_full_duplex)
                *csr6_val |= CSR6_FULL_DUPLEX_MODE;

            /* Write out CSR6. */
            OUTDW (device->dev_io_addr + CSR6, *csr6_val);

            /* Set the speed for this device in SNMP. */
            //SNMP_ifSpeed(device->dev_index, 100000000); /* 100 Mbps */
        }
        else
        {
            /* Otherwise we do 10MB */

            /* Do we need full duplex? */
            if (use_full_duplex)
            {
                *csr6_val |= CSR6_FULL_DUPLEX_MODE;

                /* Write out CSR6. */
                OUTDW (device->dev_io_addr + CSR6, *csr6_val);
            }

            /* Reset the SIA */
            OUTDW (device->dev_io_addr + CSR13, CLEAR_ALL);

            OUTDW (device->dev_io_addr + CSR14, 0x7f3f);

            OUTDW (device->dev_io_addr + CSR15, 0x0008);

            /* Set the SIA to 10BASE-T mode and wake it up. */
            OUTDW (device->dev_io_addr + CSR13, CSR13_SIA_RESET);

            /* Set the speed for this device in SNMP. */
           // SNMP_ifSpeed(device->dev_index, 10000000); /* 10 Mbps */
        }

    } /* end if negotiation completed with a valid link code */
    else
    {

        /* Since negotiation timedout, we do not have a link code
           word to tell us what to initialize the link as, so we will
           use the link status bits in CSR12. NOTE: the logic for
           these bits is reversed. If it is set then that link
           is in the fail state. */

        /* Check for 100Mbs link status first. */
        if (!(csr12_val & CSR12_100MB_LINK_STATUS))
        {
            /* Since the 100MB link is in the pass state we will
               initialize the chip for 100MB half duplex. */

            /* Half duplex mode using the SYM port. */

            /* Reset the SIA */
            OUTDW (device->dev_io_addr + CSR13, CLEAR_ALL);

            OUTDW (device->dev_io_addr + CSR14, CLEAR_ALL);

            /* Preserve CSR6s value and add in the functions we
               need to 100MB mode. */
            *csr6_val |= (CSR6_PORT_SELECT |
                     CSR6_PCS_FUNCTION | CSR6_SCRAMBLER_MODE  |
                     CSR6_HEARTBEAT_DISABLE | CSR6_CAPTURE_EFFECT_ENABLE);

            /* Write out CSR6. */
            OUTDW (device->dev_io_addr + CSR6, *csr6_val);

            /* Set the speed for this device in SNMP. */
            //SNMP_ifSpeed(device->dev_index, 100000000); /* 100 Mbps */

            Link_Status_Print(device, 1, 0);

        }
        else
            if (!(csr12_val & CSR12_10MB_LINK_STATUS))
            {
                /* Since the 10MB link is in the pass state we will
                   initialize the chip for 10MB half duplex. */

                /* Half duplex mode using the 10BASE-T port. */

                /* Reset the SIA */
                OUTDW (device->dev_io_addr + CSR13, CLEAR_ALL);

                OUTDW (device->dev_io_addr + CSR14, 0x7f3f);

                OUTDW (device->dev_io_addr + CSR15, 0x0008);

                /* Set the SIA to 10BASE-T mode and wake it up. */
                OUTDW (device->dev_io_addr + CSR13, CSR13_SIA_RESET);

                /* Set the speed for this device in SNMP. */
                //SNMP_ifSpeed(device->dev_index, 10000000); /* 10 Mbps */
                Link_Status_Print(device, 0, 0);
            }
            else {
            	printf("%s : Link Down\n\r", device->dev_net_if_name);
                /* We have nothing to go on for initializing the link.
                   Return with an error. */
                return (-1);
            }

    } /* end if the negotiation timedout. */

    return (NU_SUCCESS);

} /* end DEC21143_Init_Link */

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*  DEC21143_Negotiate_Link_HISR                                            */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*    This function simply get the device that activated the HISR and       */
/* renegotiates the link. This only used when the link changes after        */
/* initialization has completed.                                            */
/*                                                                          */
/* AUTHOR                                                                   */
/*                                                                          */
/*    Uriah T. Pollock,  Accelerated Technology Inc.                        */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*    Activated by DEC21143_LISR                                            */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*    DEC21143_Negotiate_Link                                               */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*    NONE                                                                  */
/*                                                                          */
/* OUTPUTS                                                                  */
/*                                                                          */
/*    NONE                                                                  */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*       NAME                 DATE            REMARKS                       */
/*                                                                          */
/*  Uriah T. Pollock        01/12/99      Created initial version 1.0       */
/*                                                                          */
/****************************************************************************/
VOID DEC21143_Negotiate_Link_HISR (VOID)
{
    DV_DEVICE_ENTRY     *device;
    DEC21143_XDATA      *dec_data;

    /* Get a pointer to the device. */
    device = DEC21143_Negotiate_Device_Buffer[DEC21143_Negotiate_Read];

    /* Clear out this entry. */
    DEC21143_Negotiate_Device_Buffer[DEC21143_Negotiate_Read] = NU_NULL;

    /* Update the read index. */
    if(DEC21143_Negotiate_Read >= (MAX_DEC21143_DEVICES -1))
        DEC21143_Negotiate_Read = 0;
    else
        DEC21143_Negotiate_Read++;

    /* Make sure we got a device pointer. */
    if (device == NU_NULL)
        return;

    /* Get a pointer to the DECs extended data. */
    dec_data = (DEC21143_XDATA *) device->user_defined_1;

    /* All we need to do here is to renegotiate the link. If it passes
       then mark the link as up. */
    if (DEC21143_Negotiate_Link (device, &dec_data->DEC21143_Saved_CSR6)
                                                == NU_SUCCESS)
        /* Mark the device as up. */
        device->dev_flags |= DV_UP;
    else
    	/* Mark the device as not up. */
    	device->dev_flags &= ~DV_UP;


} /* end DEC21143_Negotiate_Link_HISR */

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*  DEC21143_Get_Address                                                    */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*  This function reads the chips MAC address from the eeprom data passed   */
/*  in.                                                                     */
/*                                                                          */
/* AUTHOR                                                                   */
/*                                                                          */
/*    Uriah T. Pollock,  Accelerated Technology Inc.                        */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*    DEC21143_Init                                                         */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*    NONE                                                                  */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*    uchar * : pointer to the location to store the read Ethernet address. */
/*    uint8 * : Address of the eeprom data.                                 */
/*                                                                          */
/* OUTPUTS                                                                  */
/*                                                                          */
/*    STATUS  : Returns a value of NU_SUCCESS                               */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*       NAME                 DATE            REMARKS                       */
/*                                                                          */
/*  Uriah T. Pollock        01/12/99      Created initial version 1.0       */
/*                                                                          */
/****************************************************************************/
STATUS DEC21143_Get_Address (uint8 *eeprom_data, uchar *ether_addr)
{
    uint8 x;

    /* Read the hardware address from the EEPROM on the board. */
    for (x = 0; x < 6; x++)
        ether_addr [x] = eeprom_data [IEEE_ADDRESS_OFFSET + x];

    return (NU_SUCCESS);

} /* end DEC21143_Get_Address */

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*    DEC21143_RX_Packet                                                    */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*    This function will handle the incomming data packets from the         */
/* Ethernet board. It is called from the LISR whenever a receive interrupt  */
/* occurrs. It will handle the parsing and storing of the current packet    */
/* from the wire.                                                           */
/*                                                                          */
/* AUTHOR                                                                   */
/*                                                                          */
/*    Uriah T. Pollock,  Accelerated Technology Inc.                        */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*    DEC21143_LISR                                                         */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*    MEM_One_Buffer_Chain_Free                                             */
/*    MEM_Buffer_Enqueue                                                    */
/*    NU_Activate_HISR                                                      */
/*    NU_Tcp_Log_Error                                                      */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*    DV_DEVICE_ENTRY * : Pointer to the device to receive the packet with. */
/*    DEC21143_XDATA *  : Pointer to the devices extended data area.        */
/*                                                                          */
/* OUTPUTS                                                                  */
/*                                                                          */
/*    NONE                                                                  */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*       NAME                 DATE            REMARKS                       */
/*                                                                          */
/*  Uriah T. Pollock        01/12/99      Created initial version 1.0       */
/*                                                                          */
/****************************************************************************/
VOID DEC21143_RX_Packet (DV_DEVICE_ENTRY *device, DEC21143_XDATA *dec_data)
{
    /* Loop through all the descriptors, processing frames as we go,
       until we run into one that is owned by the DEC chip. More
       than one frame could be ready to be processed. */
    do
    {

        /* Init */
        dec_data->DEC21143_Current_Data_Size    =
        dec_data->DEC21143_Total_Data_Size      = 0;
        /* Get buffer pointer to the first buffer. */
        dec_data->DEC21143_Buffer_Ptr =
        		(NET_BUFFER *) addr_pcitov((uint32)(dec_data->
               	DEC21143_Current_RX_Descriptor->buffer),
				(U32)dec_data & 0xF0000000);
		/* The new buffer to be allocated */
		dec_data->DEC21143_Work_Ptr = NU_NULL;
        /* Save the device that this packet came off of. */
        dec_data->DEC21143_Buffer_Ptr->mem_buf_device = device;

        /* Is this the first segment of this frame. If so we need to get
           a pointer to the buffer and save off the device that generated
           this buffer. */
        if ((dec_data->DEC21143_Current_RX_Descriptor->status &
                    RDES0_FIRSTLAST_DESCRIPTOR) != RDES0_FIRSTLAST_DESCRIPTOR){
			dec_data->DEC21143_Descriptor_Incomplete++;
			dec_data->DEC21143_Work_Ptr = dec_data->DEC21143_Buffer_Ptr;
            /* Null the buffer pointer so that this buffer can not
               be freed again by the code below. */
            dec_data->DEC21143_Buffer_Ptr = NU_NULL;
		}

        /* Did we get any errors during reception? */
        if (dec_data->DEC21143_Current_RX_Descriptor->status &
        				RDES0_ERROR_SUMMARY) {
           	/* Free the buffer */
			dec_data->DEC21143_Work_Ptr = dec_data->DEC21143_Buffer_Ptr;

           	/* Null the buffer pointer so that this buffer can not
              be freed again by the code below. */
            dec_data->DEC21143_Buffer_Ptr = NU_NULL;

            /* Find out which error and log it. */

            /* Was it a CRC error? Only valid if there is not a
            runt or collision seen error. */
            if ((dec_data->DEC21143_Current_RX_Descriptor->status &
                        RDES0_CRC_ERROR) &&
                        (!((dec_data->DEC21143_Current_RX_Descriptor->status
                        & RDES0_COLLISION_SEEN) |
                        (dec_data->DEC21143_Current_RX_Descriptor->status
                        & RDES0_RUNT_FRAME)))) {
            	dec_data->DEC21143_CRC_Errors++;
            }

            /* Was it a collision seen error? */
            if (dec_data->DEC21143_Current_RX_Descriptor->status &
                        RDES0_COLLISION_SEEN) {
            	dec_data->DEC21143_Collisions_Seen++;
            }

            /* Was it a frame too long error? */
            if (dec_data->DEC21143_Current_RX_Descriptor->status &
                        RDES0_FRAME_TOO_LONG) {
            	dec_data->DEC21143_Frames_Too_Long++;
            }

            /* Was it a runt frame error? */
            if (dec_data->DEC21143_Current_RX_Descriptor->status &
                        RDES0_RUNT_FRAME) {
            	dec_data->DEC21143_Runt_Frames++;
            }

            /* Was it a descriptor error? */
            if (dec_data->DEC21143_Current_RX_Descriptor->status &
                        RDES0_DESCRIPTOR_ERROR) {
            	dec_data->DEC21143_Descriptor_Errors++;
            }
        }
       	/* Get the total data size of this frame. */
       	dec_data->DEC21143_Total_Data_Size = ((dec_data->
                                DEC21143_Current_RX_Descriptor->status &
                                RDES0_FRAME_LENGTH_ISOLATE) >> 16);
		/* If the length is invalid Discard this frame */
		if ( dec_data->DEC21143_Total_Data_Size <= 0) {
           	/* Free the buffer */
			dec_data->DEC21143_Work_Ptr = dec_data->DEC21143_Buffer_Ptr;

           	/* Null the buffer pointer so that this buffer can not
              be freed again by the code below. */
            dec_data->DEC21143_Buffer_Ptr = NU_NULL;
		}

        /* Allocate a new buffer */
		if ( dec_data->DEC21143_Work_Ptr == NU_NULL) {
			dec_data->DEC21143_Work_Ptr =
								MEM_Buffer_Dequeue_nocrit(&MEM_Buffer_Freelist);
			/* If no Buffer is available use the one received */
			if ( dec_data->DEC21143_Work_Ptr == NU_NULL ) {
				dec_data->DEC21143_Work_Ptr = dec_data->DEC21143_Buffer_Ptr;

           		/* Null the buffer pointer so that this buffer can not
              	be freed again by the code below. */
            	dec_data->DEC21143_Buffer_Ptr = NU_NULL;
			}
		}

		/* Now we have packet to send up */
        if ( dec_data->DEC21143_Buffer_Ptr ) {
            /* Take away the CRC */
            dec_data->DEC21143_Total_Data_Size -= DEC21143_ETHERNET_CRC_SIZE;
            /* Set the total data length for this buffer chain. */
            dec_data->DEC21143_Buffer_Ptr->data_len =
            dec_data->DEC21143_Buffer_Ptr->mem_total_data_len =
                            dec_data->DEC21143_Total_Data_Size;
            dec_data->DEC21143_Buffer_Ptr->next_buffer = NU_NULL;
#ifdef	INCLUDE_ODYSSEY
			/* Sync the Buffer to the Cache */
			mips_sync_cache(dec_data->DEC21143_Buffer_Ptr,
					dec_data->DEC21143_Total_Data_Size, SYNC_CACHE_FOR_CPU);
#endif
            /* Put the buffer on the incoming buffer list. */
            MEM_Buffer_Enqueue_nocrit(&MEM_Buffer_List,
											dec_data->DEC21143_Buffer_Ptr);

            /* Activate the RX HISR. The HISR will tell the stack
               that there is a packet ready for processing. */
            NU_Activate_HISR (&DEC21143_RX_HISR_CB);

		}
		/* Allocate the new buffer to the Descriptor */
       	/* Init the buffer. */
		dec_data->DEC21143_Work_Ptr->data_ptr =
				dec_data->DEC21143_Work_Ptr->mem_parent_packet;
		/* Init the buffer field a physical address */
		dec_data->DEC21143_Current_RX_Descriptor->buffer = (NET_BUFFER *)
				addr_vtopci((uint32)(dec_data->DEC21143_Work_Ptr));


        /* Preserve the end of ring bit. */
        if (dec_data->DEC21143_Current_RX_Descriptor->control_count
                            & RDES1_END_OF_RING)
        {
            /* Set the second address chained bit, the
               end of ring bit, and the buffer size. */
            dec_data->DEC21143_Current_RX_Descriptor->control_count
                                        = (RDES1_SECOND_ADDRESS_CHAINED |
                                           RDES1_END_OF_RING            |
                                           NET_PARENT_BUFFER_SIZE);

            /* Set the descriptor as owned by the DEC chip so that it
               can receive a frame with it. Also clearing out
               all other bits. */
            dec_data->DEC21143_Current_RX_Descriptor->status
                                                = RDES0_OWN_BIT;

            /* Move to the head of the descriptor list. */
            dec_data->DEC21143_Current_RX_Descriptor =
                        dec_data->DEC21143_First_RX_Descriptor;
        }
        else
        {
            /* Set the second address chained bit,
               and the buffer size. */
            dec_data->DEC21143_Current_RX_Descriptor->control_count
                                        = (RDES1_SECOND_ADDRESS_CHAINED |
                                           NET_PARENT_BUFFER_SIZE);

            /* Set the descriptor as owned by the DEC chip so that it
               can receive a frame with it. Also clearing out
               all other bits. */
            dec_data->DEC21143_Current_RX_Descriptor->status
                                                = RDES0_OWN_BIT;

            /* Move to the next descriptor in the list. */
            dec_data->DEC21143_Current_RX_Descriptor =
            	(DEC21143_DESCRIPTOR *) addr_pcitov((uint32)
				(dec_data->DEC21143_Current_RX_Descriptor ->next_descriptor),
				(U32)(dec_data->DEC21143_First_RX_Descriptor) & 0xF0000000);

        } /* end if the end of ring bit is not set */

    } while (!(dec_data->DEC21143_Current_RX_Descriptor->status &
            RDES0_OWN_BIT));    /* end while there are frames to process */

} /* end DEC21143_RX_Packet */

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*   DEC21143_RX_HISR                                                       */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*    This function simply sets an event that lets the upper layer software */
/*    know that there is at least one packet waiting to be processed.       */
/*                                                                          */
/* AUTHOR                                                                   */
/*                                                                          */
/*    Uriah T. Pollock,  Accelerated Technology Inc.                        */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*    Activated by DEC21143_RX_Packet                                       */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*    NU_Set_Events                                                         */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*    NONE                                                                  */
/*                                                                          */
/* OUTPUTS                                                                  */
/*                                                                          */
/*    NONE                                                                  */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*       NAME                 DATE            REMARKS                       */
/*                                                                          */
/*  Uriah T. Pollock        01/12/99      Created initial version 1.0       */
/*                                                                          */
/****************************************************************************/
VOID DEC21143_RX_HISR (VOID)
{
   NU_Set_Events(&Buffers_Available, (UNSIGNED)2, NU_OR);

}  /* end DEC21143_RX_HISR routine */

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*    DEC21143_TX_Packet                                                    */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*    This function will handle placing the passed in packet onto the       */
/* actual wire.                                                             */
/*                                                                          */
/* AUTHOR                                                                   */
/*                                                                          */
/*    Uriah T. Pollock,  Accelerated Technology Inc.                        */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*    NET_Ether_Send                                                        */
/*    DEC21143_TX_HISR                                                      */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*    OUTDW                                                                 */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*    DV_DEVICE_ENTRY * : Pointer to the device to send the packet with.    */
/*    NET_BUFFER *      : Pointer to the packet to send.                    */
/*                                                                          */
/* OUTPUTS                                                                  */
/*                                                                          */
/*    STATUS            : Returns NU_SUCCESS                                */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*       NAME                 DATE            REMARKS                       */
/*                                                                          */
/*  Uriah T. Pollock        01/12/99      Created initial version 1.0       */
/*                                                                          */
/****************************************************************************/
STATUS DEC21143_TX_Packet (DV_DEVICE_ENTRY *device, NET_BUFFER *buf_ptr)
{
    DEC21143_DESCRIPTOR     *head_of_frame;
    DEC21143_XDATA          *dec_data;


    /* Get a pointer to the extended data for this device. */
    dec_data = (DEC21143_XDATA *) device->user_defined_1;

    /* Save off a pointer to the head of the frame. */
    head_of_frame = dec_data->DEC21143_Current_TX_Descriptor;
	if ( head_of_frame->status & TDES0_OWN_BIT )
		return(-1);

#if SNMP_INCLUDED
    /* Is this a unicast or a non-unicast packet. */
    //if ( (buf_ptr->mem_flags & NET_BCAST) || (buf_ptr->mem_flags & NET_MCAST) )
        //SNMP_ifOutNUcastPkts_Inc(device->dev_index);
    //else
        //SNMP_ifOutUcastPkts_Inc(device->dev_index);
#endif
    /* Loop through all the buffers that make up this frame and assign
       a descriptor to each one. */
    do
    {
        /* If this is not the first segment/descriptor of this frame
           then set the ownership bit. We do not want to set this bit
           in the first frame until we have completed building the
           entire frame. Setting it in the first one will cause a race
           condition between the CPU completing the frame and the DEC chip
           trying to obtain the descriptors for transmission. Note we
           are also clearing all other bits in the status field. */
        if (dec_data->DEC21143_Current_TX_Descriptor != head_of_frame)
            dec_data->DEC21143_Current_TX_Descriptor->status = TDES0_OWN_BIT;

        /* Make sure no leftover bits are set. Be sure to maintain the
           end of ring bit if it is set. */
        if (dec_data->DEC21143_Current_TX_Descriptor->control_count &
                                        TDES1_TRANSMIT_END_OF_RING)

            /* Maintain the end of ring bit and second address chained bit. */
            dec_data->DEC21143_Current_TX_Descriptor->control_count =
                                    (TDES1_TRANSMIT_END_OF_RING |
                                     TDES1_SECOND_ADDRESS_CHAINED);
        else
            /* Maintain only the second address chained bit since the
               end of ring bit was not set. */
            dec_data->DEC21143_Current_TX_Descriptor->control_count =
                                    TDES1_SECOND_ADDRESS_CHAINED;

        /* Set the length for this descriptor. */
        dec_data->DEC21143_Current_TX_Descriptor->control_count |=
                                                buf_ptr->data_len;

        /* Point the descriptor to the buffers data area. */
        dec_data->DEC21143_Current_TX_Descriptor->buffer =
                 (NET_BUFFER *) addr_vtopci((uint32)(buf_ptr->data_ptr));
#ifdef	INCLUDE_ODYSSEY
		/* Sync the Buffer to DRAM */
		mips_sync_cache(buf_ptr->data_ptr, buf_ptr->data_len,
													SYNC_CACHE_FOR_DEV);
#endif


        /* Now move on to the next buffer in the chain. */
        buf_ptr = buf_ptr->next_buffer;

        /* Are there anymore buffers in this chain? */
        if (!buf_ptr)
        {

            /* Since this is the last one set the last segment bit and
               the interrupt on completion bit. */
            dec_data->DEC21143_Current_TX_Descriptor->control_count |=
                                    (TDES1_LAST_SEGMENT |
                                     TDES1_INTERRUPT_ON_COMPLETION);

            /* Also set the previous descriptor to point to this one.
               This will be used when the TX interrupt goes off and
               we want to get the status of this packet. */
            dec_data->DEC21143_Previous_TX_Descriptor =
                        dec_data->DEC21143_Current_TX_Descriptor;

        }

        /* Move to the next descriptor in the list. We do this if we are
           going to use it or not. If we don't use it then it is ready
           for the next time through this function. Be sure to check for
           the end of ring bit, we do not want to fall off of the end
           of the list. */
        if (dec_data->DEC21143_Current_TX_Descriptor->control_count &
                                        TDES1_TRANSMIT_END_OF_RING)

            /* It is set so start back at the beginning of the list. */
            dec_data->DEC21143_Current_TX_Descriptor =
                dec_data->DEC21143_First_TX_Descriptor;

        else
            /* It is not set so just move to the next one. */
            dec_data->DEC21143_Current_TX_Descriptor =
				(DEC21143_DESCRIPTOR *) addr_pcitov((uint32)
                (dec_data->DEC21143_Current_TX_Descriptor->next_descriptor),
				(U32)(dec_data->DEC21143_First_TX_Descriptor) & 0xF0000000);


    } while (buf_ptr);  /* while there are more buffers to process */

    /* Now that we are done building the frame, set the first segment
       bit in the first segment of this frame. */
    head_of_frame->control_count |= TDES1_FIRST_SEGMENT;

    /* Give ownership of the entire frame by setting the own bit
       of the first segment of the frame. We are also clearing
       all other bits here too. */
    head_of_frame->status = TDES0_OWN_BIT;

    /* Start the TX polling, just in case it has suspended. Any
       value can be written to this register to resume TX polling. */
    OUTDW (device->dev_io_addr + CSR1, 0xDEADBEEF);

    return (NU_SUCCESS);

} /* end DEC21143_TX_Packet */

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*   DEC21143_TX_HISR                                                       */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*    This function will transmit any packets that are waiting in the       */
/* transmit queue. It also moves the packet just transmitted from the       */
/* transmit queue to its correct deallocation list.                         */
/*                                                                          */
/* AUTHOR                                                                   */
/*                                                                          */
/*    Uriah T. Pollock,  Accelerated Technology Inc.                        */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*    Activated by the LISR                                                 */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*    NU_Local_Control_Interrupts                                           */
/*    MEM_Buffer_Dequeue                                                    */
/*    MEM_One_Buffer_Chain_Free                                             */
/*    DEC21143_TX_Packet                                                    */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*    NONE                                                                  */
/*                                                                          */
/* OUTPUTS                                                                  */
/*                                                                          */
/*    NONE                                                                  */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*       NAME                 DATE            REMARKS                       */
/*                                                                          */
/*  Uriah T. Pollock        01/12/99      Created initial version 1.0       */
/*                                                                          */
/****************************************************************************/
VOID DEC21143_TX_HISR (VOID)
{
    STATUS              previous_int_value;
    NET_BUFFER          *buf_ptr;
    DV_DEVICE_ENTRY     *device;
    DEC21143_XDATA      *dec_data;

    /* Get a pointer to the device. */
    device = DEC21143_TX_Device_Buffer[DEC21143_Read];

    /* Clear out this entry. */
    DEC21143_TX_Device_Buffer[DEC21143_Read] = NU_NULL;

    /* Update the read index. */
    if(DEC21143_Read >= (MAX_DEC21143_DEVICES -1))
        DEC21143_Read = 0;
    else
        DEC21143_Read++;

    /* Make sure we got a device pointer. */
    if (device == NU_NULL)
        return;

    /* Get a pointer to the DECs extended data. */
    dec_data = (DEC21143_XDATA *) device->user_defined_1;

    /* Do not worry about errors if this is the setup frame. */
    if (!(dec_data->DEC21143_Previous_TX_Descriptor->control_count
            & TDES1_SETUP_PACKET))
    {

        /* Did this frame have any errors during transmission. */
        if (dec_data->DEC21143_Previous_TX_Descriptor->status
                                                    & TDES0_ERROR_SUMMARY)
        {

            /* We did get an error. See which one and log it. */

            /* Was it an underflow error? */
            if (dec_data->DEC21143_Previous_TX_Descriptor->status
                                & TDES0_UNDERFLOW_ERROR)
                dec_data->DEC21143_Underflows++;

            /* Was it an excessive collision error? */
            if (dec_data->DEC21143_Previous_TX_Descriptor->status
                                & TDES0_EXCESSIVE_COLLISIONS)
                dec_data->DEC21143_Excessive_Collisions++;

            /* Was it a late collision error? */
            if (dec_data->DEC21143_Previous_TX_Descriptor->status
                                & TDES0_LATE_COLLISION)
                dec_data->DEC21143_Late_Collisions++;

            /* Was it a no carrier error? */
            if (dec_data->DEC21143_Previous_TX_Descriptor->status
                                & TDES0_NO_CARRIER)
                dec_data->DEC21143_No_Carriers++;

            /* Was it a loss of carrier error? */
            if (dec_data->DEC21143_Previous_TX_Descriptor->status
                                & TDES0_LOSS_OF_CARRIER)
                dec_data->DEC21143_Loss_Of_Carriers++;

            /* Was it a transmit jabber timeout error? */
            if (dec_data->DEC21143_Previous_TX_Descriptor->status
                                & TDES0_TX_JABBER_TIMEOUT)
                dec_data->DEC21143_TX_Jabber_Timeouts++;
        }
        else
            /* Otherwise we might have gotten 0 to 15 collisions.
               Add these into the collsion count. */
            dec_data->DEC21143_Collisions +=
                ((TDES1_COLLISIONS_ISOLATE &
                dec_data->DEC21143_Previous_TX_Descriptor->status) >> 3);

    } /* end if this is not the setup frame */

    /* Lock out interrupts.  */
    previous_int_value = NU_Local_Control_Interrupts(NU_DISABLE_INTERRUPTS);

    /* If there is an item on the transmit list (there should be everytime we
     * get here) then remove it because it has just been successfully
     * transmitted. */
    if(device->dev_transq.head)
    {
        buf_ptr = MEM_Buffer_Dequeue(&device->dev_transq);

        MEM_One_Buffer_Chain_Free (buf_ptr, buf_ptr->mem_dlist);

        /* If there is another item on the list, transmit it. */
        if(device->dev_transq.head)
        {
            /*  Re-enable interrupts. */
            NU_Local_Control_Interrupts(previous_int_value);

            /* Transmit the next packet. */
            DEC21143_TX_Packet(device, device->dev_transq.head);
        }
        else
            /*  Re-enable interrupts. */
            NU_Local_Control_Interrupts(previous_int_value);
    }
    else

        /*  Re-enable interrupts. */
        NU_Local_Control_Interrupts(previous_int_value);

}  /* end DEC21143_TX_HISR */


/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*    DEC21143_LISR                                                         */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*    This function handles all interrupts  generated by the DEC21143.  If  */
/* a transmit interrupt occurs a HISR is activated to complete processing   */
/* of the packet. If a receive interrupt occurs the RX packet routine is    */
/* called. The Negotiation HISR is activated if a link changed or fail      */
/* interrupt occurs.                                                        */
/*                                                                          */
/* AUTHOR                                                                   */
/*                                                                          */
/*    Uriah T. Pollock,  Accelerated Technology Inc.                        */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*    No functions call this function                                       */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*    DEV_Get_Dev_For_Vector                                                */
/*    OUTDW                                                                 */
/*    INDW                                                                  */
/*    NU_Activte_HISR                                                       */
/*    DEC21143_RX_Packet                                                    */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*    vector        :  The interrupt vector on which the interrupt occured. */
/*                                                                          */
/* OUTPUTS                                                                  */
/*                                                                          */
/*    NONE                                                                  */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*       NAME                 DATE            REMARKS                       */
/*                                                                          */
/*  Uriah T. Pollock        01/12/99      Created initial version 1.0       */
/*                                                                          */
/****************************************************************************/
VOID DEC21143_LISR (INT vector)
{
    uint32              int_status, read_val, temp = 1;
    DV_DEVICE_ENTRY     *device;
    DEC21143_XDATA      *dec_data;

    /*  Find the device for this interrupt. */
    device = DEV_Get_Dev_For_Vector(vector);

    /*  If the device is not correct then exit */
    if (device == (DV_DEVICE_ENTRY *) 0)
        return;

    /* Point at the DEC21143 xtended data. */
    dec_data = (DEC21143_XDATA *)device->user_defined_1;

    /* Keep processing interrupts until there are no more. */
    while ((int_status = INDW(device->dev_io_addr + CSR5)) &
                                                    DEC21143_INTERRUPTS)
    {

        /* There are two main interrupt types on the DEC21143,
           abnormal and normal. Which one is this? It could be both.
           Check the normal types first, these occure most often. */
        if (int_status & CSR5_NORMAL_INTERRUPT_SUMMARY)
        {
            /* Is it an early receive interrupt? */
            if (int_status & CSR5_EARLY_RX_INTERRUPT)
            {
                /* This interrupt is not enabled. Reset it anyway. */
                OUTDW (device->dev_io_addr + CSR5, CSR5_EARLY_RX_INTERRUPT);
            }

            /* Is it a receive interrupt? */
            if (int_status & CSR5_RX_INTERRUPT)
            {
                /* Reset the interrupt first thing. */
                OUTDW (device->dev_io_addr + CSR5, CSR5_RX_INTERRUPT);

                /* This means either a good packet was received or
                   some sort of receive error occured. Call the
                   receive function to sort this out. */

                /* Before calling, though, make sure that the current
                   descriptor is not owned by the DEC chip. If it is,
                   this is probably an interrupt for a packet that was
                   processed during the processing of the previous packet. */
                if (!(dec_data->DEC21143_Current_RX_Descriptor->status
                            & RDES0_OWN_BIT))
                    DEC21143_RX_Packet (device, dec_data);

            } /* end if RX interrupt */

            /* Is it a transmit interrupt? */
            if (int_status & CSR5_TX_INTERRUPT)
            {
                /* Reset the interrupt first thing. */
                OUTDW (device->dev_io_addr + CSR5, CSR5_TX_INTERRUPT);

#ifndef PACKET
                /* This means either a packet was successfully
                   transmitted or some sort of transmit error
                   occured. Put the device pointer in the device
                   buffer and activate the TX hisr. It will take
                   care of logging the error and/or sending
                   the next packet, if there is one ready. */

                /* Pass the device pointer to the HISR. */
                DEC21143_TX_Device_Buffer[DEC21143_Write] = device;

                /* Point to the next location in the ring buffer. */
                if(DEC21143_Write >= (MAX_DEC21143_DEVICES -1))
                    DEC21143_Write = 0;
                else
                    DEC21143_Write++;

                /* Activate the transmit HISR. */
                NU_Activate_HISR (&DEC21143_TX_HISR_CB);
#endif

            }

            /* Is it a transmit buffer unavailable interrupt? */
            if (int_status & CSR5_TX_BUFFER_UNAVAIL)
            {
                /* This interrupt is not enabled. Reset it anyway. */
                OUTDW (device->dev_io_addr + CSR5, CSR5_TX_BUFFER_UNAVAIL);

            }

            /* Is it a general puspose timer or interrupt
               mitigation control interrupt? */
            if (int_status & CSR5_TIMER_EXPIRE_MITIGATION)
            {
                /* This interrupt is not enabled. Reset it anyway. */
                OUTDW (device->dev_io_addr + CSR5, CSR5_TIMER_EXPIRE_MITIGATION);

            }

        } /* end if this is a normal interrupt */

        /* Now see if there are any abnormal interrupts. */
        if (int_status & CSR5_ABNORMAL_INTERRUPT_SUMMARY)
        {
            /* Is it a transmit process stopped interrupt? */
            if (int_status & CSR5_TX_PROCESS_STOPPED)
            {
                /* This interrupt is not enabled. Reset it anyway. */
                OUTDW (device->dev_io_addr + CSR5, CSR5_TX_PROCESS_STOPPED);
            }

            /* Is it a transmit jabber timeout interrupt? */
            if (int_status & CSR5_TX_JABBER_TIMEOUT)
            {
                 /* Reset the interrupt first thing. */
                OUTDW (device->dev_io_addr + CSR5, CSR5_TX_JABBER_TIMEOUT);
            }

            /* Is it a link pass or autonegotiation interrupt? */
            if (int_status & CSR5_AUTONEGOTIATION_COMPLETE)
            {
                 /* Reset the interrupt first thing. */
                OUTDW (device->dev_io_addr + CSR5, CSR5_AUTONEGOTIATION_COMPLETE);
#ifdef	INCLUDE_EV64120
				if ( Is10BaseT(device) && (!(device->dev_flags & DV_UP))) {

                	if (dec_data->DEC21143_Init_Completed) {
                		/* Pass the device pointer to the HISR. */
                		DEC21143_Negotiate_Device_Buffer[DEC21143_Negotiate_Write] = device;

                		/* Point to the next location in the ring buffer. */
                		if(DEC21143_Negotiate_Write >= (MAX_DEC21143_DEVICES -1))
                    		DEC21143_Negotiate_Write = 0;
                		else
                  			DEC21143_Negotiate_Write++;

                		/* If initialization has been completed then this
                		 * interrupt means the link has changed. We need to
                		 * renegotiate and set the link speed. */
                    	NU_Activate_HISR (&DEC21143_Negotiate_Link_HISR_CB);
                    }
                    /* Mark the device as up. */
                	device->dev_flags |= DV_UP;
                }
#endif
            }

            /* Is it a transmit underflow interrupt? */
            if (int_status & CSR5_TX_UNDERFLOW)
            {
                 /* Reset the interrupt first thing. */
                OUTDW (device->dev_io_addr + CSR5, CSR5_TX_UNDERFLOW);
            }

            /* Is it a receive buffer unavailable interrupt? */
            if (int_status & CSR5_RX_BUFFER_UNAVAIL)
            {
                 /* Reset the interrupt first thing. */
                OUTDW (device->dev_io_addr + CSR5, CSR5_RX_BUFFER_UNAVAIL);
            }

            /* Is it a receive process stopped interrupt? */
            if (int_status & CSR5_RX_PROCESS_STOPPED)
            {
                /* This interrupt is not enabled. Reset it anyway. */
                OUTDW (device->dev_io_addr + CSR5, CSR5_RX_PROCESS_STOPPED);
            }

            /* Is it a receive watchdog timeout interrupt? */
            if (int_status & CSR5_RX_WATCHDOG_TIMEOUT)
            {
                /* This interrupt is not enabled. Reset it anyway. */
                OUTDW (device->dev_io_addr + CSR5, CSR5_RX_WATCHDOG_TIMEOUT);
            }

            /* Is it an early transmit interrupt? */
            if (int_status & CSR5_EARLY_TX_INTERRUPT)
            {
                /* This interrupt is not enabled. Reset it anyway. */
                OUTDW (device->dev_io_addr + CSR5, CSR5_EARLY_TX_INTERRUPT);
            }

            /* Is it a link fail interrupt? Only 10BASE-T mode. */
            if (int_status & CSR5_LINK_FAIL)
            {
                 /* Reset the interrupt first thing. */
                OUTDW (device->dev_io_addr + CSR5, CSR5_LINK_FAIL);


                if (dec_data->DEC21143_Init_Completed) {
                	/* Pass the device pointer to the HISR. */
                	DEC21143_Negotiate_Device_Buffer[DEC21143_Negotiate_Write] = device;

                	/* Point to the next location in the ring buffer. */
                	if(DEC21143_Negotiate_Write >= (MAX_DEC21143_DEVICES -1))
                   		DEC21143_Negotiate_Write = 0;
                	else
                    	DEC21143_Negotiate_Write++;

                	/* If initialization has been completed then this interrupt
                   	means the link has changed. We need to renegotiate and
                   	set the link speed. */
                    NU_Activate_HISR (&DEC21143_Negotiate_Link_HISR_CB);
                }

                /* Mark the device as not up. */
                device->dev_flags &= ~DV_UP;


            }

            /* Is it a fatal bus error interrupt? */
            if (int_status & CSR5_FATAL_BUS_ERROR)
            {
                 /* Reset the interrupt first thing. */
                OUTDW (device->dev_io_addr + CSR5, CSR5_FATAL_BUS_ERROR);
                printf("%s: Fatal Error: %s\n\r", device->dev_net_if_name,
                			ether_err_mess[(int_status >> 23) & 0x7]); 
            }

            /* Is it a generial purpose port interrupt? */
            if (int_status & CSR5_GENERAL_PURPOSE_PORT_INT)
            {
#ifdef INCLUDE_ODYSSEY
                read_val = INDW (device->dev_io_addr + CSR15);
				/* If bit 17 is 0 then PHY has Interrupted */
				if ( !(read_val & ( 1 << 17))) {
					read_val = mdio_read(device->dev_io_addr+CSR9, 1,
												PHY_INTSTAT_OFF);
					if ( read_val & PHY_INTSTAT_MINT) {
                		if (dec_data->DEC21143_Init_Completed) {
                			/* Pass the device pointer to the HISR. */
                			DEC21143_Negotiate_Device_Buffer[DEC21143_Negotiate_Write] = device;

                			/* Point to the next location in the ring buffer. */
                			if(DEC21143_Negotiate_Write >= (MAX_DEC21143_DEVICES -1))
                    			DEC21143_Negotiate_Write = 0;
                			else
                    			DEC21143_Negotiate_Write++;

                    		NU_Activate_HISR (&DEC21143_Negotiate_Link_HISR_CB);
                		}

                		/* Mark the device as not up. */
                		device->dev_flags &= ~DV_UP;
						/* Clear the Interrupt by reading PHY reg 1 and 18 */
						read_val = mdio_read(device->dev_io_addr+CSR9, 1,
												MII_STAT_OFF);
						read_val = mdio_read(device->dev_io_addr+CSR9, 1,
												PHY_INTSTAT_OFF);
					}

				}
#endif
                /* This interrupt is not enabled. Reset it anyway. */
                OUTDW (device->dev_io_addr + CSR5, CSR5_GENERAL_PURPOSE_PORT_INT);
            }

            /* Is it a link changed interrupt? Only 100BASE-T mode. */
            if (int_status & CSR5_LINK_CHANGED)
            {
                 /* Reset the interrupt first thing. */
                OUTDW (device->dev_io_addr + CSR5, CSR5_LINK_CHANGED);

                /* Only try to negotiate it if the link is in the fail
                   state. */

                /* Read CSR12<1> to see if the link went up or
                   if it went down. */
                read_val = INDW (device->dev_io_addr + CSR12);
                /* Check bit 1. This bit set means the link is in a
                   fail state, reset means the link is in pass state. */
                if ((read_val & CSR12_100MB_LINK_STATUS) ==
                                CSR12_100MB_LINK_STATUS)
                {

                    if (dec_data->DEC21143_Init_Completed) {
                    	/* Pass the device pointer to the HISR. */
                    	DEC21143_Negotiate_Device_Buffer[DEC21143_Negotiate_Write] = device;

                    	/* Point to the next location in the ring buffer. */
                    	if(DEC21143_Negotiate_Write >= (MAX_DEC21143_DEVICES -1))
                        	DEC21143_Negotiate_Write = 0;
                    	else
                        	DEC21143_Negotiate_Write++;

                    	/* If initialization has been completed then this
                    	 * interrupt means the link has changed. We need to
                    	 * renegotiate and set the link speed. */
                        NU_Activate_HISR (&DEC21143_Negotiate_Link_HISR_CB);
                    }
                    /* Mark the device as not up. */
                    device->dev_flags &= ~DV_UP;

                }


            } /* end if a link changed interrupt */

        } /* end if this is an abnormal interrupt */


    } /* end while there are interrupts to process */


#ifdef I80X86
    /* clear any stray interrupts */
    if (device -> dev_irq > 7)
    {
        OUTB (IR8259B, EOI);
    }
    OUTB (IR8259A, EOI);
#endif /* I80X86 */

}  /* end DEC21143_LISR */

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*    DEC21143_Ioctl                                                        */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*    This function processes IOCTL requests to the DEC21143 driver.        */
/*                                                                          */
/* AUTHOR                                                                   */
/*                                                                          */
/*    Uriah T. Pollock,  Accelerated Technology Inc.                        */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*    IP_Add_Multi                                                          */
/*    IP_Delete_Multi                                                       */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*    NET_Add_Multi                                                         */
/*    NET_Del_Multi                                                         */
/*    DEC21143_Init_Setup_Frame                                             */
/*    NU_Tcp_Log_Error                                                      */
/*    DEC21143_Update_Setup_Frame                                           */
/*    DEC21143_Process_Setup_Frame                                          */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*    DV_DEVICE_ENTRY * : Pointer to the device for this request.           */
/*    INT               : Which option to do.                               */
/*    DV_REQ *          : Pointer to the request                            */
/*                                                                          */
/* OUTPUTS                                                                  */
/*                                                                          */
/*    STATUS        :  NU_SUCCES, NU_INVAL, or NU_NO_BUFFERS                */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*       NAME                 DATE            REMARKS                       */
/*                                                                          */
/*  Uriah T. Pollock        01/12/99      Created initial version 1.0       */
/*                                                                          */
/****************************************************************************/
STATUS  DEC21143_Ioctl(DV_DEVICE_ENTRY *dev, INT option, DV_REQ *d_req)
{
    STATUS          ret_status;
    NET_BUFFER      *setup_frame;

    switch (option)
    {
    case DEV_ADDMULTI :


#ifdef	NET_SUPPORT
        /* Join the ethernet multicast group. */
        ret_status = NET_Add_Multi(dev, d_req);
#else
		ret_status = NU_RESET;
#endif

        /* A status of NU_RESET means the operation was a success and as a
           result the ethernet setup frame needs to be reset. */
        if (ret_status == NU_RESET)
        {
            /* Allocate a buffer and pass it to the setup frame intialization
               routine. */

            /* Create the initial setup frame used for filtering RX frames.
               Make sure this worked. */
            if (DEC21143_Init_Setup_Frame (dev->dev_mac_addr, setup_frame =
                    MEM_Buffer_Dequeue (&MEM_Buffer_Freelist)) != NU_SUCCESS)
            {
                /* This is a bad error. */
                NU_Tcp_Log_Error (DRV_RESOURCE_ALLOCATION_ERROR,
                                        TCP_SEVERE, __FILE__, __LINE__);

                return (NU_NO_BUFFERS);
            }

            /* Update this setup frame with all the multicast addresses. */
            DEC21143_Update_Setup_Frame (dev, setup_frame);

            /* Now that we have created the setup frame
               give it to the DEC chip for processing. */
            ret_status = DEC21143_Process_Setup_Frame (dev, setup_frame);

        }

        break;

    case DEV_DELMULTI :

        /* Join the ethernet multicast group. */
#ifdef	NET_SUPPORT
        ret_status = NET_Del_Multi(dev, d_req);
#else
		ret_status = NU_RESET;
#endif

        /* A status of NU_RESET means the operation was a success and as a
           result the ethernet setup frames needs to be reset. */
        if (ret_status == NU_RESET)
        {
            /* Allocate a buffer and pass it to the setup frame intialization
               routine. */

            /* Create the initial setup frame used for filtering RX frames.
               Make sure this worked. */
            if (DEC21143_Init_Setup_Frame (dev->dev_mac_addr, setup_frame =
                    MEM_Buffer_Dequeue (&MEM_Buffer_Freelist)) != NU_SUCCESS)
            {
                /* This is a bad error. */
                NU_Tcp_Log_Error (DRV_RESOURCE_ALLOCATION_ERROR,
                                        TCP_SEVERE, __FILE__, __LINE__);

                return (NU_NO_BUFFERS);
            }

            /* Update this setup frame with all the multicast addresses. */
            DEC21143_Update_Setup_Frame (dev, setup_frame);

            /* Now that we have created the setup frame
               give it to the DEC chip for processing. */
            ret_status = DEC21143_Process_Setup_Frame (dev, setup_frame);

        }

        break;

    default :

        ret_status = NU_INVAL;

        break;

    }

    return (ret_status);

} /* DEC21143_Ioctl */

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*   DEC21143_Delay                                                         */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*    This function simply delays processing while the controller           */
/* initializes itself.                                                      */
/*                                                                          */
/* AUTHOR                                                                   */
/*                                                                          */
/*    Uriah T. Pollock,  Accelerated Technology Inc.                        */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*    DEC21143_Open                                                         */
/*    DEC21143_Read_EEPROM                                                  */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*    NONE                                                                  */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*    NONE                                                                  */
/*                                                                          */
/* OUTPUTS                                                                  */
/*                                                                          */
/*    NONE                                                                  */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*       NAME                 DATE            REMARKS                       */
/*                                                                          */
/*  Uriah T. Pollock        01/12/99      Created initial version 1.0       */
/*                                                                          */
/****************************************************************************/
VOID DEC21143_Delay (VOID)
{
    UNSIGNED time;

    for (time = 0; time < 1000; time++) ;
}

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*   DEC21143_Allocate_Descriptor                                           */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*    This function allocates descriptors to be used by the DEC chip for    */
/* packet TX and RX. Its links them together to form a ring list and sets   */
/* the correct bits in the status and control fields. If add_buffers is     */
/* true it will also allocate a buffer for each descriptor and point it to  */
/* the buffer.                                                              */
/*                                                                          */
/* AUTHOR                                                                   */
/*                                                                          */
/*    Uriah T. Pollock,  Accelerated Technology Inc.                        */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*    DEC21143_Open                                                         */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*    NU_Allocate_Memory                                                    */
/*    UTL_Zero                                                              */
/*    MEM_Buffer_Dequeue                                                    */
/*    MEM_Buffer_Enqueue                                                    */
/*    NU_Tcp_Log_Error                                                      */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*    uint8             : Number of decriptors to allocate                  */
/*    NU_MEMORY_POOL *  : Pointer to the memory pool to use for allocation. */
/*    uint8             : Do these descriptors need buffers?                */
/*                                                                          */
/* OUTPUTS                                                                  */
/*                                                                          */
/*    DEC21143_DESCRIPTOR * : Pointer to the head of the descriptor list.   */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*       NAME                 DATE            REMARKS                       */
/*                                                                          */
/*  Uriah T. Pollock        01/12/99      Created initial version 1.0       */
/*                                                                          */
/****************************************************************************/
DEC21143_DESCRIPTOR *DEC21143_Allocate_Descriptor (uint8 num_descriptors,
                                                    NU_MEMORY_POOL *mem_pool,
                                                    uint8 add_buffers)
{
    DEC21143_DESCRIPTOR *first_descriptor, *work_descriptor, *prev_descriptor;
    STATUS              ret_status;
	U32					seg_addr;

    /* Validate parameters. */
    if ((num_descriptors == NU_NULL) || (mem_pool == NU_NULL))
        return (NU_NULL);

    /* Allocate the first descriptor and save its address off to be
       returned. */
    ret_status = NU_Allocate_Memory (mem_pool, (VOID **) &first_descriptor,
                                        sizeof (DEC21143_DESCRIPTOR)+ 32,
                                        NU_NO_SUSPEND);
	seg_addr = (U32)first_descriptor & 0xF0000000;
	first_descriptor = (DEC21143_DESCRIPTOR *)ALIGN(first_descriptor, 32);
	/* Whether code it running in cache or not, access descriptors uncached */
	first_descriptor = (DEC21143_DESCRIPTOR *)((U32)first_descriptor
															| 0xA0000000);

    /* Make sure it worked */
    if (ret_status == NU_SUCCESS)
    {
        /* Store the first as the previous. */
        prev_descriptor = first_descriptor;

        /* Zero it out. */
        UTL_Zero (prev_descriptor, sizeof (DEC21143_DESCRIPTOR));

        /* Set the address chained bit. */
        prev_descriptor->control_count = RDES1_SECOND_ADDRESS_CHAINED;

        /* Do we need to link in a net buffer to this descriptor. This will
           be the case for RX descriptors. */
        if (add_buffers)
        {
            /* Get one from the buffer free list. */
            prev_descriptor->buffer =
                            MEM_Buffer_Dequeue (&MEM_Buffer_Freelist);

            /* Make sure it worked. */
            if (prev_descriptor->buffer == NU_NULL)
            {
                /* Set the status to ~NU_SUCCESS so that we fall into
                   the error cleanup code below. */
                ret_status = ~NU_SUCCESS;
            }
            else
            {
                /* Init the buffer. */
                ((NET_BUFFER *)prev_descriptor->buffer)->data_ptr =
                    (((NET_BUFFER *)prev_descriptor->buffer)->
                    mem_parent_packet);

				/* Make the buffer field a physical address */
				prev_descriptor->buffer = (NET_BUFFER *)
					addr_vtopci((uint32)(prev_descriptor->buffer));

                /* Set the size of this buffer. */
                prev_descriptor->control_count |= NET_PARENT_BUFFER_SIZE;

                /* Set the descriptor as owned by the DEC chip so that it
                   can receive a frame with it. */
                prev_descriptor->status = RDES0_OWN_BIT;
            }

        } /* if we need to add a buffer */

        /* Loop and get the rest. */
        while ((--num_descriptors) && (ret_status == NU_SUCCESS))
        {
            /* Allocate the memory */
            ret_status = NU_Allocate_Memory (mem_pool,
                                        (VOID **) &work_descriptor,
                                        sizeof (DEC21143_DESCRIPTOR)+ 32,
                                        NU_NO_SUSPEND);
			/* Whether code it running in cache or not, access descriptors
			 * uncached */
			work_descriptor = (DEC21143_DESCRIPTOR *)ALIGN(work_descriptor, 32);
			work_descriptor = (DEC21143_DESCRIPTOR *)((U32)work_descriptor
															| 0xA0000000);

            /* Make sure we got one. */
            if (ret_status == NU_SUCCESS)
            {
                /* Zero it out. */
                UTL_Zero (work_descriptor, sizeof (DEC21143_DESCRIPTOR));

                /* Set the address chained bit. */
                work_descriptor->control_count = DES1_SECOND_ADDRESS_CHAINED;

                /* Link it into the list. */
                prev_descriptor->next_descriptor = (DEC21143_DESCRIPTOR *)
						addr_vtopci((uint32)work_descriptor);

                /* Do we need to link in a net buffer to this descriptor.
                   This will be the case for RX descriptors. */
                if (add_buffers)
                {
                    /* Get one from the buffer free list. */
                    work_descriptor->buffer =
                            MEM_Buffer_Dequeue (&MEM_Buffer_Freelist);

                    /* Make sure it worked. */
                    if (work_descriptor->buffer == NU_NULL)
                    {
                        /* Set the status to ~NU_SUCCESS so that these
                           buffers will be freed. */
                        ret_status = ~NU_SUCCESS;

                        /* Set the count to zero so that we get out of the
                           while loop. */
                        num_descriptors = 0;

                    }
                    else
                    {
                        /* Init the buffer. */
                        ((NET_BUFFER *)work_descriptor->buffer)->data_ptr =
                            (((NET_BUFFER *)work_descriptor->buffer)->
                            mem_parent_packet);

						/* Make the buffer field a physical address */
						work_descriptor->buffer = (NET_BUFFER *)
							addr_vtopci((uint32)(work_descriptor->buffer));

                        /* Set the size of this buffer. */
                        work_descriptor->control_count |= NET_PARENT_BUFFER_SIZE;

                        /* Set the descriptor as owned by the DEC so that it
                           can receive a frame with it. */
                        work_descriptor->status = RDES0_OWN_BIT;

                    }

                } /* if we need to add a buffer */

                /* Move to next one. */
                prev_descriptor = (DEC21143_DESCRIPTOR *)
						addr_pcitov((uint32)(prev_descriptor->next_descriptor),
						(U32)prev_descriptor & 0xF0000000);

            } /* if the alloc worked. */

        } /* while more to alloc */

    } /* if the first alloc worked */
    else
        /* Make the list NULL since the allocation failed. */
        first_descriptor = NU_NULL;

    /* Did we get any errors? */
    if (ret_status != NU_SUCCESS)
    {
        /* Log the error and get out. */
        NU_Tcp_Log_Error (DRV_RESOURCE_ALLOCATION_ERROR, TCP_FATAL,
                                                __FILE__, __LINE__);

        /* We need to free up all memory that was allocated */
        for (work_descriptor = first_descriptor; work_descriptor != NU_NULL; )
        {
            /* Is there a buffer that needs to be freed? */
            if (work_descriptor->buffer)
                MEM_Buffer_Enqueue (&MEM_Buffer_Freelist,
                   (NET_BUFFER *) addr_pcitov((uint32)(work_descriptor->buffer),
					(U32)(&MEM_Buffer_Freelist) & 0xF0000000 ));

            /* Save off this descriptor. */
            prev_descriptor = work_descriptor;

            /* Move to the next descriptor. */
            work_descriptor = (DEC21143_DESCRIPTOR *)
            		addr_pcitov((uint32)(work_descriptor->next_descriptor),
					(U32)work_descriptor & 0xF0000000);

			prev_descriptor = (DEC21143_DESCRIPTOR *)
					(((U32)prev_descriptor & 0x0FFFFFFF) | seg_addr);
            /* Deallocate the descriptor. */
            NU_Deallocate_Memory (prev_descriptor);
        }

        /* Make the return pointer NULL */
        first_descriptor = NU_NULL;
    }
    else
        /* Otherwise mark the last descriptor as the end. */
        work_descriptor->control_count |= DES1_END_OF_RING;

    /* Return the pointer to the head of the list. */
    return (first_descriptor);

}

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*   DEC21143_Find_PCI_ID                                                   */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*    This function searches the PCI bus for the DEC21143 card. Upon        */
/* finding it the function returns the ID of that PCI card. This function   */
/* can be used for more than one card. NOTE: only one PCI bus is supported  */
/* by this code.                                                            */
/*                                                                          */
/* AUTHOR                                                                   */
/*                                                                          */
/*    Uriah T. Pollock,  Accelerated Technology Inc.                        */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*    DEC21143_Open                                                         */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*    OUTDW                                                                 */
/*    INDW                                                                  */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*    uint8             : The number of the card to find. This is used for  */
/*                        multicard application.                            */
/*                                                                          */
/* OUTPUTS                                                                  */
/*                                                                          */
/*    uint32            : The PCI ID for the card found                     */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*       NAME                 DATE            REMARKS                       */
/*                                                                          */
/*  Uriah T. Pollock        01/12/99      Created initial version 1.0       */
/*                                                                          */
/****************************************************************************/
uint32 DEC21143_Find_PCI_ID (uint8 card_num)
{
	uint32	pci_id;

#ifdef INCLUDE_ODYSSEY
	pci_id = get_pci_id(PCI_E_BUSNUM, ETHER_CNTL_ID, card_num);
#else
	pci_id = get_pci_id(0, ETHER_CNTL_ID, card_num);
#endif
    return (pci_id);
}

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*   DEC21143_Write_PCI                                                     */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*    Writes out data to the PCI bus for a specific PCI device.             */
/*                                                                          */
/* AUTHOR                                                                   */
/*                                                                          */
/*    Uriah T. Pollock,  Accelerated Technology Inc.                        */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*    DEC21143_Open                                                         */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*    OUTDW                                                                 */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*    uint32                : The PCI ID to write to.                       */
/*    uint8                 : The register number on the card to write to.  */
/*    uint32                : The data to write.                            */
/*                                                                          */
/* OUTPUTS                                                                  */
/*                                                                          */
/*    NONE                                                                  */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*       NAME                 DATE            REMARKS                       */
/*                                                                          */
/*  Uriah T. Pollock        01/12/99      Created initial version 1.0       */
/*                                                                          */
/****************************************************************************/
VOID DEC21143_Write_PCI (uint32 pci_id, uint8 reg_num, uint32 data_val)
{
	pciconf_writel(pci_id, reg_num << 2, data_val);
} /* end DEC21143_Write_PCI */

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*   DEC21143_Read_PCI                                                      */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*    Reads a 32bit value from one register on a PCI card.                  */
/*                                                                          */
/* AUTHOR                                                                   */
/*                                                                          */
/*    Uriah T. Pollock,  Accelerated Technology Inc.                        */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*    DEC21143_Open                                                         */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*    OUTDW                                                                 */
/*    INDW                                                                  */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*    uint32                : The PCI ID for the card to read.              */
/*    uint8                 : The register number to read.                  */
/*                                                                          */
/* OUTPUTS                                                                  */
/*                                                                          */
/*    uint32                : The data read from the PCI card.              */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*       NAME                 DATE            REMARKS                       */
/*                                                                          */
/*  Uriah T. Pollock        01/12/99      Created initial version 1.0       */
/*                                                                          */
/****************************************************************************/
uint32 DEC21143_Read_PCI (uint32 pci_id, uint8 reg_num)
{
	return(pciconf_readl(pci_id, reg_num << 2));

} /* end DEC21143_Read_PCI */

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*   DEC21143_Read_EEPROM                                                   */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*    Reads in two bytes from the EEPROM located on the DEC card.           */
/*                                                                          */
/* AUTHOR                                                                   */
/*                                                                          */
/*    Uriah T. Pollock,  Accelerated Technology Inc.                        */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*    DEC21143_Open                                                         */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*    DEC21143_Delay                                                        */
/*    OUTDW                                                                 */
/*    INDW                                                                  */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*    uint32                : The address of the EEPROM                     */
/*    uint8                 : The area in the EEPROM to read                */
/*                                                                          */
/* OUTPUTS                                                                  */
/*                                                                          */
/*    uint16                : The two bytes read from the EEPROM            */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*       NAME                 DATE            REMARKS                       */
/*                                                                          */
/*  Uriah T. Pollock        01/12/99      Created initial version 1.0       */
/*                                                                          */
/****************************************************************************/
uint16 DEC21143_Read_EEPROM (uint32 io_addr, uint8 location)
{
    int     x, read_cmd;
    uint16  ret_val, data_val;

    /* Init */
    ret_val = 0;
    read_cmd = (location | CSR9_ROM_READ_COMMAND);

    OUTDW (io_addr + CSR9, EE_ENB & ~EE_CS);
    OUTDW (io_addr + CSR9, EE_ENB);

    /* Shift the read command bits out. */
    for (x = 10; x >= 0; x--)
    {
        data_val = (read_cmd & (1 << x)) ? EE_DATA_WRITE : 0;

        OUTDW (io_addr + CSR9, (EE_ENB | data_val));
        DEC21143_Delay();

        OUTDW (io_addr + CSR9, (EE_ENB | EE_SHIFT_CLK | data_val));
        DEC21143_Delay();

        OUTDW (io_addr + CSR9, (EE_ENB | data_val));
        DEC21143_Delay();
    }

    OUTDW (io_addr + CSR9, EE_ENB);

    /* Read in the data, two bytes */
    for (x = 16; x > 0; x--)
    {
        OUTDW (io_addr + CSR9, (EE_ENB | EE_SHIFT_CLK));
        DEC21143_Delay();

        ret_val = (ret_val << 1) | ((INDW (io_addr + CSR9) & EE_DATA_READ) ? 1 : 0);

        OUTDW (io_addr + CSR9, EE_ENB);
        DEC21143_Delay();
    }

    /* Terminate the EEPROM access. */
    OUTDW (io_addr + CSR9, (EE_ENB & ~EE_CS));

    /* Return the value read. */
    return (ret_val);

} /* end DEC21143_Read_EEPROM */

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*  DEC21143_Init_Setup_Frame                                               */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*    This function is responsible for initializing the setup frame. The    */
/* only MAC addresses added to the setup fame by this function is the cards */
/* address and the broadcast addresss.                                      */
/*                                                                          */
/* AUTHOR                                                                   */
/*                                                                          */
/*    Uriah T. Pollock,  Accelerated Technology Inc.                        */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*    DEC21143_Open                                                         */
/*    DEC21143_Ioctl                                                        */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*    UTL_Zero                                                              */
/*    sizeof                                                                */
/*    NU_Tcp_Log_Error                                                      */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*    uint8 *           : Pointer to the hardware address.                  */
/*    NET_BUFFER *      : Pointer to a buffer to use as the setup frame.    */
/*                                                                          */
/* OUTPUTS                                                                  */
/*                                                                          */
/*    STATUS            : NU_SUCCES if the parameters are valid,            */
/*                        -1 otherwise.                                     */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*       NAME                 DATE            REMARKS                       */
/*                                                                          */
/*  Uriah T. Pollock        01/12/99      Created initial version 1.0       */
/*                                                                          */
/****************************************************************************/
STATUS DEC21143_Init_Setup_Frame (uint8 *ether_addr, NET_BUFFER *buf_ptr)
{
    STATUS      ret_status;
    uint8       x;

    /* Validate parms. */
    if (buf_ptr && ether_addr)
    {
        /* Zero it out, clearing all hash entries. */
        UTL_Zero (buf_ptr, sizeof (NET_BUFFER));

        /* Set the data pointer to the parent packet address. */
        buf_ptr->data_ptr = buf_ptr->mem_parent_packet;

#ifdef DEC21143_HASH_PERFECT_FILTERING

        /* Hash Perfecting filtering use a hash table and a single
           perfect hash MAC layer address. */

        /* Add the MAC layer address. It starts at offset 156 and jumps
           2 bytes every 2 bytes. */
        buf_ptr->mem_parent_packet[DEC21143_SETUP_FRAME_HW_ADDR]
                                                        = ether_addr[0];
        buf_ptr->mem_parent_packet[DEC21143_SETUP_FRAME_HW_ADDR + 1]
                                                        = ether_addr[1];
        buf_ptr->mem_parent_packet[DEC21143_SETUP_FRAME_HW_ADDR + 4]
                                                        = ether_addr[2];
        buf_ptr->mem_parent_packet[DEC21143_SETUP_FRAME_HW_ADDR + 5]
                                                        = ether_addr[3];
        buf_ptr->mem_parent_packet[DEC21143_SETUP_FRAME_HW_ADDR + 8]
                                                        = ether_addr[4];
        buf_ptr->mem_parent_packet[DEC21143_SETUP_FRAME_HW_ADDR + 9]
                                                        = ether_addr[5];

#else
        /* The default is perfect filtering. This uses 16 exact
           addresses. Note that one of these MUST be the broadcast
           address and that all 16 spots MUST be filled in. */

        /* Set the first 15 to the local address and the last one
           to the broadcast address. */
        for (x = 0; x < 15; x++)
        {
            ((uint16 *) buf_ptr->mem_parent_packet)[x * 6]
                                                = ((uint16 *) ether_addr)[0];

            ((uint16 *) buf_ptr->mem_parent_packet)[(x * 6) + 2]
                                                = ((uint16 *) ether_addr)[1];

            ((uint16 *) buf_ptr->mem_parent_packet)[(x * 6) + 4]
                                                = ((uint16 *) ether_addr)[2];
        }

        /* Now do the broadcast address. */
        ((uint16 *) buf_ptr->mem_parent_packet)[(x * 6)]     = 0xFFFF;
        ((uint16 *) buf_ptr->mem_parent_packet)[(x * 6) + 2] = 0xFFFF;
        ((uint16 *) buf_ptr->mem_parent_packet)[(x * 6) + 4] = 0xFFFF;

#endif

        /* Set this buffers deallocation list. */
        buf_ptr->mem_dlist = &MEM_Buffer_Freelist;

        ret_status = NU_SUCCESS;

    }
    else
    {
        /* This is a fatal error. */
        NU_Tcp_Log_Error (DRV_RESOURCE_ALLOCATION_ERROR, TCP_FATAL,
                                        __FILE__, __LINE__);

        /* Set the status to something negative. */
        ret_status = -1;
    }

    /* Return completion status. */
    return (ret_status);

}  /* end DEC21143_Init_Setup_Frame */

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*   DEC21143_Process_Setup_Frame                                           */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*    Takes a setup frame and sets up the descriptors properly for the DEC  */
/* chip to process it.                                                      */
/*                                                                          */
/* AUTHOR                                                                   */
/*                                                                          */
/*    Uriah T. Pollock,  Accelerated Technology Inc.                        */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*    DEC21143_Open                                                         */
/*    DEC21143_Ioctl                                                        */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*    NU_Change_Preemption                                                  */
/*    MEM_Buffer_Enqueue                                                    */
/*    OUTDW                                                                 */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*    DV_DEVICE_ENTRY *     : Pointer to the device to RX the setup frame.  */
/*    NET_BUFFER *          : Pointer to the buffer that holds the setup    */
/*                            frame.                                        */
/*                                                                          */
/* OUTPUTS                                                                  */
/*                                                                          */
/*    STATUS                : NU_SUCCESS if all went well, -1 otherwise.    */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*       NAME                 DATE            REMARKS                       */
/*                                                                          */
/*  Uriah T. Pollock        01/12/99      Created initial version 1.0       */
/*                                                                          */
/****************************************************************************/
STATUS DEC21143_Process_Setup_Frame (DV_DEVICE_ENTRY *device, NET_BUFFER *setup_frame)
{
    DEC21143_XDATA  *dec_data;
    STATUS          ret_status;
    OPTION          old_preempt;

    /* Init, assume success. */
    ret_status = NU_SUCCESS;

    /* We must be the only thread executing when we do this. It is just
       not safe to play with the descriptors like this when there are
       other threads executing. */
    old_preempt = NU_Change_Preemption(NU_NO_PREEMPT);

    /* Wait for all packets currently on the TX queue to be sent. */
    while (device->dev_transq.head != NU_NULL) ;

    /* At this point the driver should not be busy sending anything. */

    /* Get a pointer to the dec extended data. */
    dec_data = (DEC21143_XDATA *) device->user_defined_1;

    /* Make sure the current descriptor is not in use by the DEC chip. */
    if (!(dec_data->DEC21143_Current_TX_Descriptor->status & TDES0_OWN_BIT))
    {
        /* The setup frame must be preceded by a descriptor with zero length
           buffer. Build that one here. */

        /* Do we need to keep the end of ring bit or not? */
        if (dec_data->DEC21143_Current_TX_Descriptor->control_count &
                        TDES1_TRANSMIT_END_OF_RING)
        {

            /* Mark this as a setup frame, set filtering type to hash, and
               set other needed bits for processing of this frame. Note
               we also add the end of ring bit. */
            dec_data->DEC21143_Current_TX_Descriptor->control_count =

#ifdef DEC21143_HASH_PERFECT_FILTERING

                                (TDES1_HASH_FILTERING           |
#else
                                (TDES1_PERFECT_FILTERING        |
#endif

                                 TDES1_SETUP_PACKET             |
                                 TDES1_SECOND_ADDRESS_CHAINED   |
                                 TDES1_TRANSMIT_END_OF_RING);

            /* Set this as the previous descriptor. */
            dec_data->DEC21143_Previous_TX_Descriptor =
                dec_data->DEC21143_Current_TX_Descriptor;

            /* Move to the head of the list since this is the last
               descriptor in the ring. */
            dec_data->DEC21143_Current_TX_Descriptor =
                (DEC21143_DESCRIPTOR *)
                dec_data->DEC21143_First_TX_Descriptor;

        }
        else
        {

            /* Mark this as a setup frame, set filtering type to hash, and
               set other needed bits for processing of this frame. */
            dec_data->DEC21143_Current_TX_Descriptor->control_count =
#ifdef DEC21143_HASH_PERFECT_FILTERING

                                (TDES1_HASH_FILTERING           |
#else
                                (TDES1_PERFECT_FILTERING        |
#endif
                                 TDES1_SETUP_PACKET             |
                                 TDES1_SECOND_ADDRESS_CHAINED);

            /* Set this as the previous descriptor. */
            dec_data->DEC21143_Previous_TX_Descriptor =
                dec_data->DEC21143_Current_TX_Descriptor;

            /* Move to the next descriptor in the list. */
            dec_data->DEC21143_Current_TX_Descriptor =
                (DEC21143_DESCRIPTOR *) addr_pcitov((uint32)
                (dec_data->DEC21143_Current_TX_Descriptor->next_descriptor),
				(U32)(dec_data->DEC21143_First_TX_Descriptor) & 0xF0000000);

        }

        /* Now set the own bit in the previous descriptor, the one
           we just setup, so that the DEC chip can process it. This is
           for the descriptor with a zero length buffer. */
        dec_data->DEC21143_Previous_TX_Descriptor->status = TDES0_OWN_BIT;

        /* Now put the actual setup frame in the descriptor list. */

        /* Point the descriptor to this setup frame. */
        dec_data->DEC21143_Current_TX_Descriptor->buffer =
        	(NET_BUFFER *) addr_vtopci((uint32)(setup_frame->data_ptr));
#ifdef	INCLUDE_ODYSSEY
		/* Sync the Buffer to DRAM */
		mips_sync_cache(setup_frame->data_ptr, DEC21143_SETUP_FRAME_LENGTH,
													SYNC_CACHE_FOR_DEV);
#endif

        /* Do we need to keep the end of ring bit or not? */
        if (dec_data->DEC21143_Current_TX_Descriptor->control_count &
                        TDES1_TRANSMIT_END_OF_RING)
        {

            /* Mark this as a setup frame, set filtering type to hash, and
               set other needed bits for processing of this frame. Note
               we also add the end of ring bit. */
            dec_data->DEC21143_Current_TX_Descriptor->control_count =

#ifdef DEC21143_HASH_PERFECT_FILTERING

                                (TDES1_HASH_FILTERING           |
#else
                                (TDES1_PERFECT_FILTERING        |
#endif

                                 TDES1_SETUP_PACKET             |
                                 TDES1_SECOND_ADDRESS_CHAINED   |
                                 TDES1_INTERRUPT_ON_COMPLETION  |
                                 TDES1_TRANSMIT_END_OF_RING);

            /* Add in the size. This MUST be 192 bytes. */
            dec_data->DEC21143_Current_TX_Descriptor->control_count |=
                                DEC21143_SETUP_FRAME_LENGTH;

            /* Set this as the previous descriptor. */
            dec_data->DEC21143_Previous_TX_Descriptor =
                dec_data->DEC21143_Current_TX_Descriptor;

            /* Move to the head of the list since this is the last
               descriptor in the ring. */
            dec_data->DEC21143_Current_TX_Descriptor =
                (DEC21143_DESCRIPTOR *)
                dec_data->DEC21143_First_TX_Descriptor;

        }
        else
        {

            /* Mark this as a setup frame, set filtering type to hash, and
               set other needed bits for processing of this frame. */
            dec_data->DEC21143_Current_TX_Descriptor->control_count =
#ifdef DEC21143_HASH_PERFECT_FILTERING

                                (TDES1_HASH_FILTERING           |
#else
                                (TDES1_PERFECT_FILTERING        |
#endif
                                 TDES1_SETUP_PACKET             |
                                 TDES1_SECOND_ADDRESS_CHAINED   |
                                 TDES1_INTERRUPT_ON_COMPLETION);

            /* Add in the size. This MUST be 192 bytes. */
            dec_data->DEC21143_Current_TX_Descriptor->control_count |=
                                DEC21143_SETUP_FRAME_LENGTH;

            /* Set this as the previous descriptor. */
            dec_data->DEC21143_Previous_TX_Descriptor =
                dec_data->DEC21143_Current_TX_Descriptor;

            /* Move to the next descriptor in the list. */
            dec_data->DEC21143_Current_TX_Descriptor =
                (DEC21143_DESCRIPTOR *) addr_pcitov((uint32)
                (dec_data->DEC21143_Current_TX_Descriptor->next_descriptor),
				(U32)(dec_data->DEC21143_First_TX_Descriptor) & 0xF0000000);

        }

        /* Place the buffer on the device's transmit queue so that this
           buffer will be freed. */
        MEM_Buffer_Enqueue(&device->dev_transq, setup_frame);

        /* Now set the own bit in the previous descriptor, the one
           we just setup, so that the DEC chip can process it. */
        dec_data->DEC21143_Previous_TX_Descriptor->status = TDES0_OWN_BIT;

        /* Start the TX polling, just in case it has suspended. Any
           value can be written to this register to resume TX polling. */
        OUTDW (device->dev_io_addr + CSR1, 0xDEADBEEF);

    } /* end if the current descriptor is owned by the DEC chip */
    else

        /* We could not process the setup frame because of lack of
           TX descriptors. */
        ret_status = -1;

    NU_Change_Preemption(old_preempt);

    /* Return the status. */
    return (ret_status);

} /* end of DEC21143_Process_Setup_Frame */

/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*   DEC21143_Update_Setup_Frame                                            */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*    This function takes a pre-initialized setup frame and adds to it all  */
/* the multicast addresses associated with the device.                      */
/*                                                                          */
/* AUTHOR                                                                   */
/*                                                                          */
/*    Uriah T. Pollock,  Accelerated Technology Inc.                        */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*    DEC21143_Ioctl                                                        */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*    NONE                                                                  */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*    DV_DEVICE_ENRTRY *        : Pointer to the device from which to       */
/*                                use multicast addresses.                  */
/*    NET_BUFFER *              : Pointer to the buffer that holds the      */
/*                                setup frame.                              */
/*                                                                          */
/* OUTPUTS                                                                  */
/*                                                                          */
/*    NONE                                                                  */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*       NAME                 DATE            REMARKS                       */
/*                                                                          */
/*  Uriah T. Pollock        01/12/99      Created initial version 1.0       */
/*                                                                          */
/****************************************************************************/
VOID DEC21143_Update_Setup_Frame (DV_DEVICE_ENTRY *device, NET_BUFFER *setup_frame)
{
    uint8       x;
    NET_MULTI   *multi_ptr;

    /* This setup frame must have already been initialized. */

    /* For perfect filtering this means that the first entry has the
       chips MAC address in it and the last address has the
       broadcast address in it. There are are a total of 16 available
       entries, so there are 14 left.
    */

    /* Start at the second entry and add up to the 15th, or until the end of
       the list is reached. NOTE that only 14 multicast addresses can be
       added. If there are more than that they will not be added. */
    for (x = 1, multi_ptr = device->dev_ethermulti;
         x < 15, multi_ptr;
         x++, multi_ptr = multi_ptr->nm_next)
    {
        ((uint16 *) setup_frame->mem_parent_packet)[x * 6]
                                        = ((uint16 *) multi_ptr->nm_addr)[0];

        ((uint16 *) setup_frame->mem_parent_packet)[(x * 6) + 2]
                                        = ((uint16 *) multi_ptr->nm_addr)[1];

        ((uint16 *) setup_frame->mem_parent_packet)[(x * 6) + 4]
                                        = ((uint16 *) multi_ptr->nm_addr)[2];
    }

} /* end DEC21143_Update_Setup_Frame */

#ifdef PROTECTED_MODE
/****************************************************************************/
/* FUNCTION                                                                 */
/*                                                                          */
/*   Unknown_Int_LISR                                                       */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*  This LISR will simply reset the interrupt. This interrupt hardly ever   */
/* occurs. It is only generated when interrupts are coming in faster than   */
/* the CPU can respond to them. Code execution can resume normally after    */
/* this interrupt is reset.                                                 */
/*                                                                          */
/* AUTHOR                                                                   */
/*                                                                          */
/*    Uriah T. Pollock,  Accelerated Technology Inc.                        */
/*                                                                          */
/* CALLED BY                                                                */
/*                                                                          */
/*    Interrupt                                                             */
/*                                                                          */
/* CALLS                                                                    */
/*                                                                          */
/*    OUTB                                                                  */
/*                                                                          */
/* INPUTS                                                                   */
/*                                                                          */
/*    vector     - vetor number that caused this interrupt                  */
/*                                                                          */
/* OUTPUTS                                                                  */
/*                                                                          */
/*    NONE                                                                  */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*       NAME                 DATE            REMARKS                       */
/*                                                                          */
/*  Uriah T. Pollock        01/12/99      Created initial version 1.0       */
/*                                                                          */
/****************************************************************************/
VOID Unknown_Int_LISR (INT vector)
{
    switch ( vector )
    {
        case IRQ_15_VECTOR :

            OUTB(IR8259B, RESET_UNKNOWN_INT);

            break;

        case UNKNOWN_INT   :

            OUTB(IR8259A, RESET_UNKNOWN_INT);

            break;
    }

    if ( vector > IRQ_15_VECTOR )
    {
        OUTB(IR8259A, EOI);
    }
    else
    {
        OUTB(IR8259A, EOI);
        OUTB(IR8259B, EOI);
    }
}
#endif


uint32
getset_ether_base_addr(uint32 pci_id, int instance)
{
	uint32 io_addr;

	io_addr = ETHER_BASE_ADDR + (instance * 4096);
	pciconf_writel(pci_id, PCI_CONF_BASE0, io_addr);
	return(io_addr);
}


void
DEC_IO_Write_Reg(uint32	io_addr, uint32 val)
{
	uint32	*vaddr;
#ifdef	INCLUDE_ODYSSEY
	vaddr	= (uint32 *)(io_addr - 0x80000000);
	*vaddr = DWSWAP(val);
#else
	vaddr	= (uint32 *)(io_addr + 0xA0000000);
	*vaddr = DWSWAP(val);
#endif
}

uint32
DEC_IO_Read_Reg(uint32	io_addr)
{
	uint32	val;
#ifdef	INCLUDE_ODYSSEY
	val	= *((uint32 *)(io_addr - 0x80000000));
	return(DWSWAP(val));
#else
	val	= *((uint32 *)(io_addr + 0xA0000000));
	return(DWSWAP(val));
#endif
}


#define		CSR15_DEFAULT_20_27		0x00000000
void
Reset_Phy6611(DV_DEVICE_ENTRY *device)
{
	uint32      csr15_val;
/* Reset the 6611 PHY. The 6611 chip has a bug because of
 * which we need to issue this Reset - Sudhir Kasargod */
/* We need to create this default values for CSR15 bit 20-27
 * becuase these bits are write only
 */

	csr15_val = INDW (device->dev_io_addr + CSR15);
	/* clear the bits 20-27 */
	csr15_val &= ~0x0FF00000;

	/* Set the defaults for bits 20-27 */
	csr15_val |= CSR15_DEFAULT_20_27;
	/* Enable Control Write Enable */
	csr15_val |= CSR15_CONTROL_WRITE_ENABLE;
	/* Select gep0 */
	csr15_val  &= ~CSR15_LED_GEP_0_SELECT;
	/* Make gep0 as output */
	csr15_val |= ( 1 << 16);
	/* write the value to the chip */
	OUTDW (device->dev_io_addr + CSR15, csr15_val);

	/* disable Control Write Enable, so that we can drive gep0 */
	csr15_val &= ~CSR15_CONTROL_WRITE_ENABLE;
	/* set gep0 pin */
	csr15_val |= ( 1 << 16);
	/* write the value to the chip */
	OUTDW (device->dev_io_addr + CSR15, csr15_val);

	DEC21143_Delay();

	/* reset gep0 which drives RESET low and write to the chip */
	csr15_val &= ~( 1 << 16);
	OUTDW (device->dev_io_addr + CSR15, csr15_val);

	/* The reset line should be low for 100 ms */
	NU_Sleep(10);

	/* set gep0 pin */
	csr15_val |= ( 1 << 16);
	/* write the value to the chip */
	OUTDW (device->dev_io_addr + CSR15, csr15_val);

	/* Make the gep0 input signal */
	/* Enable Control Write Enable */
	csr15_val |= CSR15_CONTROL_WRITE_ENABLE;
	/* Make gep0 as input */
	csr15_val &= ~( 1 << 16);
	/* write the value to the chip */
	OUTDW (device->dev_io_addr + CSR15, csr15_val);

	/* Disable Control Write Enable */
	csr15_val &= ~CSR15_CONTROL_WRITE_ENABLE;
	/* write the value to the chip */
	OUTDW (device->dev_io_addr + CSR15, csr15_val);
}

void
Link_Status_Print(DV_DEVICE_ENTRY *device, uint32 use_100MB, uint32 use_full_duplex)
{
	printf("%s : ", device->dev_net_if_name);
	if (use_100MB)
		printf("100 Mbps ");
	else
		printf("10 Mbps ");

	if ( use_full_duplex )
		printf("full-duplex Link Up\n\r");
	else
		printf("half-duplex Link Up\n\r");

}

uint32
Is10BaseT(DV_DEVICE_ENTRY *device)
{
    uint32  csr14_val, csr12_val;
    uint16  link_code_word;

    /* Read CSR12 */
    csr12_val = INDW (device->dev_io_addr + CSR12);

    /* Get the link partners code word. It is in the upper 16 bits
       of CSR12. */
	link_code_word = ((csr12_val >> 21) & 0x00FF);

    /* Read in CSR14, this is used below. */
    csr14_val = INDW (device->dev_io_addr + CSR14);



    /* 10BASE-T Full Duplex */
    if (((link_code_word & CSR12_CODE_10MB_FD) == CSR12_CODE_10MB_FD) &&
            ((csr14_val & CSR14_100BASE_T4) == 0x0) &&
            ((csr14_val & CSR14_100BASE_TX_FD) == 0x0) &&
            ((csr14_val & CSR14_100BASE_TX_HD) == 0x0)){
    	return(1);

    }

        /* 10BASE-T Full Duplex */
    else if (((link_code_word >> 1) == (CSR12_CODE_10MB_FD >> 1)))
    {
		return(1);
    }

	/* 10BASE-T Half Duplex */
    else if (((link_code_word & CSR12_CODE_10MB_HD) == CSR12_CODE_10MB_HD) &&
            ((csr14_val & CSR14_100BASE_T4) == 0x0) &&
            ((csr14_val & CSR14_100BASE_TX_FD) == 0x0) &&
            ((csr14_val & CSR14_100BASE_TX_HD) == 0x0) &&
            ((csr14_val & CSR14_10BASET_HD_ENABLE) == CSR14_10BASET_HD_ENABLE))
    {
		return(1);
    }

    /* 10BASE-T Half Duplex */
    else if ((link_code_word == CSR12_CODE_10MB_HD) &&
            ((csr14_val & CSR14_10BASET_HD_ENABLE) == CSR14_10BASET_HD_ENABLE))
    {
		return(1);
    }
    else if (!(csr12_val & CSR12_10MB_LINK_STATUS)) {
        return (1);
    } else {
    	return(0);
    }

}
STATUS
MIIinit_Phy(DV_DEVICE_ENTRY *device)
{
	int i;
	uint32      csr15_val;

	/* Address for the PHY is hard wired to 1 */
	/* Find out whether we can detect the Level1's Phy */
	if ( mdio_read(device->dev_io_addr+CSR9, 1, MII_ID_1_OFF) != 0x7810) {
		printf("Could not detect the PHY\n\r");
		return(-1);
	}


	/* Try to Reset the PHY */
	mdio_write(device->dev_io_addr+CSR9, 1, MII_CNTL_OFF, MII_CNTL_RESET);
	for(i=0; i < 100; i++) {
		if ( !(mdio_read(device->dev_io_addr+CSR9, 1, MII_CNTL_RESET) &
					MII_CNTL_RESET))
		{
			break;
		}

	}

	/* Configure the Auto Negotiation Advertisement Register */
	mdio_write(device->dev_io_addr+CSR9, 1, MII_ANEG_ADV_OFF, MII_ADV_DEFAULT);

	/* Configure the Configuration Register Register */
	mdio_write(device->dev_io_addr+CSR9, 1, PHY_CONF_OFF, PHY_CONF_DEFAULT);

	/* Enable the Interrupt */
	mdio_write(device->dev_io_addr+CSR9, 1, PHY_INTEN_OFF, PHY_INTEN_ENABLE);

	/* Configure the Control Register */
	mdio_write(device->dev_io_addr+CSR9, 1, MII_CNTL_OFF, MII_CNTL_DEFAULT);

	/* Make gp1 as an Input(which is default) and Enable Interrupt on gp1 */

	csr15_val = INDW (device->dev_io_addr + CSR15);
	/* clear the bits 20-27 */
	csr15_val &= ~0x0FF00000;

	/* Set the defaults for bits 20-27 */
	csr15_val |= CSR15_DEFAULT_20_27;
	/* Enable Control Write Enable */
	csr15_val |= CSR15_CONTROL_WRITE_ENABLE;
	/* Select gep1 */
	csr15_val  &= ~CSR15_LED_GEP_1_SELECT;
	/* Make gep1 as input */
	csr15_val &= ~( 1 << 17);
	/* Enable gep1 Interrupt */
	csr15_val |= ( 1 << 25);
	/* write the value to the chip */
	OUTDW (device->dev_io_addr + CSR15, csr15_val);

	/* disable Control Write Enable, so that we can drive gep0 */
	csr15_val &= ~CSR15_CONTROL_WRITE_ENABLE;
	/* write the value to the chip */
	OUTDW (device->dev_io_addr + CSR15, csr15_val);
	/* Clear any pending Interrupt by reading CSR15 */
	csr15_val = INDW (device->dev_io_addr + CSR15);

	return(NU_SUCCESS);
}

void
DEC21143_Regdump(DV_DEVICE_ENTRY *device)
{
    uint32  csr_val;
    DEC21143_XDATA      *dec_data;

    csr_val = INDW (device->dev_io_addr + CSR0);
    printf("CSR0  :%08X\n\r", csr_val);

    csr_val = INDW (device->dev_io_addr + CSR3);
    printf("CSR3  :%08X\n\r", csr_val);

    csr_val = INDW (device->dev_io_addr + CSR4);
    printf("CSR4  :%08X\n\r", csr_val);

    csr_val = INDW (device->dev_io_addr + CSR5);
    printf("CSR5  :%08X\n\r", csr_val);

    csr_val = INDW (device->dev_io_addr + CSR6);
    printf("CSR6  :%08X\n\r", csr_val);

    csr_val = INDW (device->dev_io_addr + CSR7);
    printf("CSR7  :%08X\n\r", csr_val);

    csr_val = INDW (device->dev_io_addr + CSR8);
    printf("CSR8  :%08X\n\r", csr_val);

    csr_val = INDW (device->dev_io_addr + CSR9);
    printf("CSR9  :%08X\n\r", csr_val);

    csr_val = INDW (device->dev_io_addr + CSR12);
    printf("CSR12 :%08X\n\r", csr_val);

    csr_val = INDW (device->dev_io_addr + CSR13);
    printf("CSR13 :%08X\n\r", csr_val);

    csr_val = INDW (device->dev_io_addr + CSR14);
    printf("CSR14 :%08X\n\r", csr_val);

    csr_val = INDW (device->dev_io_addr + CSR15);
    printf("CSR15 :%08X\n\r", csr_val);

    /* Point at the DEC21143 xtended data. */
    dec_data = (DEC21143_XDATA *)device->user_defined_1;

	printf("PCI Status %04X\n\r", pciconf_readw(dec_data->DEC21143_PCI_ID, PCI_CONF_STAT));

}
void
PHY_Regdump(DV_DEVICE_ENTRY *device)
{
	printf("Level1 PHY Register Dump:\n\r");
	printf("MII_00 %04X\n\r", mdio_read(device->dev_io_addr+CSR9, 1, 0));
	printf("MII_04 %04X\n\r", mdio_read(device->dev_io_addr+CSR9, 1, 4));
	printf("MII_05 %04X\n\r", mdio_read(device->dev_io_addr+CSR9, 1, 5));
	printf("MII_06 %04X\n\r", mdio_read(device->dev_io_addr+CSR9, 1, 6));
	printf("PHY_17 %04X\n\r", mdio_read(device->dev_io_addr+CSR9, 1, 17));
	printf("PHY_18 %04X\n\r", mdio_read(device->dev_io_addr+CSR9, 1, 18));
	printf("PHY_19 %04X\n\r", mdio_read(device->dev_io_addr+CSR9, 1, 19));
	printf("PHY_20 %04X\n\r", mdio_read(device->dev_io_addr+CSR9, 1, 20));
}

void
phy_make_intr(DV_DEVICE_ENTRY *device)
{
	/* Enable and gengerate the Interrupt */
	mdio_write(device->dev_io_addr+CSR9, 1, PHY_INTEN_OFF,
					PHY_INTEN_ENABLE | PHY_INTEN_TINT);
}

void
DEC21143_Printcounters (DV_DEVICE_ENTRY *device)
{
    DEC21143_XDATA          *dec_data;

    /* Get a pointer to the extended data for this device. */
    dec_data = (DEC21143_XDATA *) device->user_defined_1;

	printf("CRC Errors		: %d\n\r", dec_data->DEC21143_CRC_Errors);
	printf("Collisions Seen		: %d\n\r", dec_data->DEC21143_Collisions_Seen);
	printf("Long Frames		: %d\n\r", dec_data->DEC21143_Frames_Too_Long);
	printf("Runt Frames		: %d\n\r", dec_data->DEC21143_Runt_Frames);
	printf("Desc Errors		: %d\n\r", dec_data->DEC21143_Descriptor_Errors);
	printf("Desc Incomplete		: %d\n\r",dec_data->DEC21143_Descriptor_Incomplete);
	printf("Underflows		: %d\n\r", dec_data->DEC21143_Underflows);
	printf("Collisions		: %d\n\r", dec_data->DEC21143_Collisions);
	printf("Exc Collisions		: %d\n\r", dec_data->DEC21143_Excessive_Collisions);
	printf("Late Collisions		: %d\n\r", dec_data->DEC21143_Late_Collisions);
	printf("No Carriers		: %d\n\r", dec_data->DEC21143_No_Carriers);
	printf("Loss  Carriers		: %d\n\r", dec_data->DEC21143_Loss_Of_Carriers);
	printf("Jabber Timeouts		: %d\n\r", dec_data->DEC21143_TX_Jabber_Timeouts);
}
