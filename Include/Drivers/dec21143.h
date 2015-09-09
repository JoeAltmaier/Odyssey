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
/*    DEC21143.H                                                            */
/*                                                                          */
/* DESCRIPTION                                                              */
/*                                                                          */
/*    This file contains the definitions and macros necessary for the       */
/*    DEC21143 driver.                                                      */
/*                                                                          */
/* AUTHOR                                                                   */
/*                                                                          */
/*    Uriah T. Pollock                                                      */
/*                                                                          */
/* DATA STRUCTURES                                                          */
/*                                                                          */
/*    DEC21143_XDATA                                                        */
/*    DEC21143_DESCRIPTOR                                                   */
/*                                                                          */
/* FUNCTIONS                                                                */
/*                                                                          */
/*    NONE                                                                  */
/*                                                                          */
/* DEPENDENCIES                                                             */
/*                                                                          */
/*    "dev.h"                                                               */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*       NAME                 DATE            REMARKS                       */
/*                                                                          */
/*  Uriah T. Pollock        01/12/99      Created initial version 1.0       */
/*                                                                          */
/****************************************************************************/
#include "dev.h"

#ifdef __cplusplus
extern "C" {
#endif

/* This macro defines the number of buffer descriptors to allocate. Note
   that the one for RX will also allocate the same number of net buffers
   for storing the incoming packets. */
#define DEC21143_MAX_RX_DESCRIPTORS         32

/* We compute the value by determining how many buffers are needed to send
   a max sized ethernet packet. Then just to be safe we double it. */
#define DEC21143_MAX_TX_DESCRIPTORS         ((((DEC21143_ETHERNET_MTU +         \
                                            DEC21143_ETHERNET_ADDRESS_SIZE)     \
                                            / NET_MAX_BUFFER_SIZE) + 1) * 8)

/* Define the length of time we will wait for autonegotiation to
   complete. */
#define DEC21143_NEOGITATION_TIMEOUT        (15 * TICKS_PER_SECOND);

/* Define basic ethernet sizes. */
#define DEC21143_ETHERNET_CRC_SIZE          4
#define DEC21143_ETHERNET_ADDRESS_SIZE      6
#define DEC21143_ETHERNET_HEADER_SIZE       14
#define DEC21143_ETHERNET_MTU               1500

/* This macro defines which interrupts will be enabled on the
   DEC chip. Since the same bits positions are used for enabling
   interrupts and for getting status of interrupts this macro
   is used in both places in the driver. */
#define DEC21143_INTERRUPTS     (CSR5_TX_INTERRUPT                  |   \
                                 CSR5_TX_JABBER_TIMEOUT             |   \
                                 CSR5_TX_UNDERFLOW                  |   \
                                 CSR5_RX_INTERRUPT                  |   \
                                 CSR5_RX_PROCESS_STOPPED            |   \
                                 CSR5_RX_BUFFER_UNAVAIL             |   \
                                 CSR5_FATAL_BUS_ERROR               |   \
                                 CSR5_ABNORMAL_INTERRUPT_SUMMARY    |   \
                                 CSR5_NORMAL_INTERRUPT_SUMMARY      |   \
                                 CSR5_AUTONEGOTIATION_COMPLETE      |   \
                                 CSR5_LINK_FAIL                     |   \
                                 CSR5_LINK_CHANGED)

/* Define the buffer descriptor structure used for sending and receiving
   packets.
*/

typedef struct _dec21143_descriptor DEC21143_DESCRIPTOR;

typedef struct _dec21143_descriptor
{
    uint32              status;             /* own bit and status bits              */
    uint32              control_count;      /* buffer byte count and control bits   */
    NET_BUFFER          *buffer;            /* buffer address                       */
    DEC21143_DESCRIPTOR *next_descriptor;   /* next descriptor address              */
} dsc;

/* Define values for the descriptor control and status fields. Some are
   common between both RX and TX descriptor, DESx, some are only for
   RX, RDESx, and some are only for TX, TDESx.

    0 coorresponds to the status field
    1 coorresponds to the control_count field
    2 coorresponds to the buffer field
    3 coorresponds to the next_descriptor field

*/

/* Common values. */
#define DES0_OWN_BIT                    (1 << 31)
#define DEC21143_SETUP_FRAME_LENGTH     192

#define DES1_END_OF_RING                (1 << 25)
#define DES1_SECOND_ADDRESS_CHAINED     (1 << 24)

/* RX Values. */
#define RDES0_CRC_ERROR                 (1 << 1)
#define RDES0_DRIBBLING_BIT             (1 << 2)
#define RDES0_REPORT_ON_MII_ERROR       (1 << 3)
#define RDES0_RECEIVE_WATCHDOG          (1 << 4)
#define RDES0_FRAME_TYPE                (1 << 5)
#define RDES0_COLLISION_SEEN            (1 << 6)
#define RDES0_FRAME_TOO_LONG            (1 << 7)
#define RDES0_LAST_DESCRIPTOR           (1 << 8)
#define RDES0_FIRST_DESCRIPTOR          (1 << 9)
#define RDES0_MULTICAST_FRAME           (1 << 10)
#define RDES0_RUNT_FRAME                (1 << 11)
#define RDES0_DATA_TYPE_MASK            0x00003000UL        /* bits 13:12 */
#define RDES0_DESCRIPTOR_ERROR          (1 << 14)
#define RDES0_ERROR_SUMMARY             (1 << 15)
#define RDES0_FRAME_LENGTH_ISOLATE      0x3FFF0000Ul        /* bits 29:16 */
#define RDES0_FILTERING_FAIL            (1 << 30)
#define RDES0_OWN_BIT                   (1 << 31)

#define RDES1_END_OF_RING               (1 << 25)
#define RDES1_SECOND_ADDRESS_CHAINED    (1 << 24)
#define	RDES0_FIRSTLAST_DESCRIPTOR		(RDES0_FIRST_DESCRIPTOR | RDES0_LAST_DESCRIPTOR)


/* TX Values. */
#define TDES0_DEFERRED                  (1 << 0)
#define TDES0_UNDERFLOW_ERROR           (1 << 1)
#define TDES0_LINK_FAIL_REPORT          (1 << 2)
#define TDES0_COLLISION_COUNT_MASK      0x00000078UL        /* bits 6:3   */
#define TDES0_HEARTBEAT_FAIL            (1 << 7)
#define TDES0_EXCESSIVE_COLLISIONS      (1 << 8)
#define TDES0_LATE_COLLISION            (1 << 9)
#define TDES0_NO_CARRIER                (1 << 10)
#define TDES0_LOSS_OF_CARRIER           (1 << 11)
#define TDES0_TX_JABBER_TIMEOUT         (1 << 14)
#define TDES0_ERROR_SUMMARY             (1 << 15)
#define TDES0_OWN_BIT                   (1 << 31)

#define TDES1_BUFFER_1_SIZE_MASK        0x000003FFUL        /* bits 10:0  */
#define TDES1_DISABLED_PADDING          (1 << 23)
#define TDES1_SECOND_ADDRESS_CHAINED    (1 << 24)
#define TDES1_TRANSMIT_END_OF_RING      (1 << 25)
#define TDES1_ADD_CRC_DISABLE           (1 << 26)
#define TDES1_SETUP_PACKET              (1 << 27)
#define TDES1_FIRST_SEGMENT             (1 << 29)
#define TDES1_LAST_SEGMENT              (1 << 30)
#define TDES1_INTERRUPT_ON_COMPLETION   (1 << 31)
#define TDES1_PERFECT_FILTERING         0x00000000UL        /* bits 22 and 28 */
#define TDES1_HASH_FILTERING            (1 << 22)           /* bits 22 and 28 */
#define TDES1_INVERSE_FILTERING         (1 << 28)           /* bits 22 and 28 */
#define TDES1_HASH_ONLY_FILTERING       ((1 << 22) | (1 << 28)) /* bits 22 and 28 */
#define TDES1_FILTERING_CLEAR           (~((1 << 22) | (1 << 28))) /* bits 22 and 28 */
#define TDES1_COLLISIONS_ISOLATE        ((1 << 3) | (1 << 4) | \
                                        (1 << 5) | (1 << 6))


/* This structure is used to keep track of extended data that is required for
   each registered DEC21143 device. */
typedef struct _dec21143_xdata
{
    /* Declare the pointers to the TX and RX buffer descriptors. */
    DEC21143_DESCRIPTOR     *DEC21143_First_RX_Descriptor;
    DEC21143_DESCRIPTOR     *DEC21143_First_TX_Descriptor;

    DEC21143_DESCRIPTOR     *DEC21143_Current_RX_Descriptor;
    DEC21143_DESCRIPTOR     *DEC21143_Current_TX_Descriptor;

    DEC21143_DESCRIPTOR     *DEC21143_Previous_TX_Descriptor;

    /* Declare the pointers used during packet reception. */
    NET_BUFFER              *DEC21143_Buffer_Ptr;
    NET_BUFFER              *DEC21143_Work_Ptr;

    /* PCI Card ID. Used for accessing the card through the PCI
       config registers. */
    uint32                  DEC21143_PCI_ID;

    /* Chip revision and a saved copy of CSR6 . */
    uint32                  DEC21143_Saved_CSR6;
    uint16                  DEC21143_Revision;

    /* Data size counters used during packet reception. */
    int16                   DEC21143_Total_Data_Size;
    int16                   DEC21143_Current_Data_Size;

    /* Receive errors. */
    uint16                  DEC21143_CRC_Errors;
    uint16                  DEC21143_Collisions_Seen;
    uint16                  DEC21143_Frames_Too_Long;
    uint16                  DEC21143_Runt_Frames;
    uint16                  DEC21143_Descriptor_Errors;
    uint16                  DEC21143_Descriptor_Incomplete;

    /* Transmit errors. */
    uint16                  DEC21143_Underflows;
    uint16                  DEC21143_Collisions;
    uint16                  DEC21143_Excessive_Collisions;
    uint16                  DEC21143_Late_Collisions;
    uint16                  DEC21143_No_Carriers;
    uint16                  DEC21143_Loss_Of_Carriers;
    uint16                  DEC21143_TX_Jabber_Timeouts;

    /* Has the card been completly initialized? This is used when
       the link changes speed after initialization is done. */
    uint8                   DEC21143_Init_Completed;

    /* Padding to keep everything word aligned. */
    uint8                   pad0;


} DEC21143_XDATA;


/* Define the new macros for reading data from your Ethernet chip, and
   writing data to the Ethernet chip.
*/

#define OUTDW	DEC_IO_Write_Reg
#define INDW    DEC_IO_Read_Reg
#define	UTL_Zero	bzero

#if defined(PROTECTED_MODE) && !defined(__BORLANDC__)
/*
 * Define the new macros for reading data from your Ethernet chip, and
 * writing data to the Ethernet chip.
 */

  extern uchar inp(int);
  extern void outp(int, uchar);
  extern int inport(int);
  extern void outport(int, int);

  #define OUTB(offset, value)    outp ((int16)offset, (uchar)value)
  #define OUTW(offset, value)    outport ((int16)offset, (int16)value)
  #define INB(offset)            (uchar)inp ((sshort)offset)
  #define INW(offset)            (ushort)inport ((ushort)offset)

#elif defined(PROTECTED_MODE) && defined(__BORLANDC__)
/*
 * Define the new macros for reading data from your Ethernet chip, and
 * writing data to the Ethernet chip.
 */
  #define OUTB(offset, value)    outp ((unsigned)offset, (int)value)
  #define OUTW(offset, value)    outport ((int)offset, (int)value)
  #define INB(offset)            (uchar)inp ((unsigned)offset)
  #define INW(offset)            (ushort)inport ((int)offset)

  #define ASM   asm  /* Used for some inline assembly. */

#elif !defined(PROTECTED_MODE) && defined(__BORLANDC__)
/*
 * Define the new macros for reading data from your Ethernet chip, and
 * writing data to the Ethernet chip.
 */

  #define OUTB(offset, value)    outportb ((int16)offset, (uchar)value)
  #define OUTW(offset, value)    outport ((int16)offset, (int16)value)
  #define INB(offset)            (uchar)inportb ((sshort)offset)
  #define INW(offset)            (ushort)inpw ((ushort)offset)

  #define ASM   asm  /* Used for some inline assembly. */

#elif defined (_MSC_VER) /* If Microsoft C is being used. */
/*
 * Define the new macros for reading data from your Ethernet chip, and
 * writing data to the Ethernet chip.
 */
  #define OUTW(offset, value)     (ushort)_outpw ((ushort)offset, (ushort)value)
  #define INW(offset)             (ushort)_inpw ((ushort)offset)
  #define OUTB(offset, value)     (sshort)_outp ((ushort)offset, (sshort)value)
  #define INB(offset)             (sshort)_inp ((ushort)offset)

  #define ASM    __asm   /* used for inline assembly. */

#endif /* Port macros */


/* -------------------- DEC21143 registers -------------------------- */

/* Define the offsets for all the configuration registers
   on the DEC21143 chip
*/
#define CFID    0x00
#define CFCS    0x04
#define CFRV    0x08
#define CFLT    0x0C
#define CBIO    0x10
#define CBMA    0x14
#define CCIS    0x28
#define CSID    0x2C
#define CBER    0x30
#define CCAP    0x34
#define CFIT    0x3C
#define CFDD    0x40
#define CWUA0   0x44
#define CWUA1   0x48
#define SOP0    0x4C
#define SOP1    0x50
#define CWUC    0x54
#define CCID    0xDC
#define CPMC    0xE0

/* Define the offsets for all the control and status registers
   on the DEC21143 chip
*/
#define CSR0    0x00
#define CSR1    0x08
#define CSR1_PM 0x08
#define CSR2    0x10
#define CSR2_PM 0x10
#define CSR3    0x18
#define CSR4    0x20
#define CSR5    0x28
#define CSR6    0x30
#define CSR7    0x38
#define CSR8    0x40
#define CSR9    0x48
#define CSR10   0x50
#define CSR11   0x58
#define CSR12   0x60
#define CSR13   0x68
#define CSR14   0x70
#define CSR15   0x78

/* --------------------- end registers ------------------------------ */


/* -------------------- register values ----------------------------- */

/* The following section contains macros for various register values
   used by the driver.
*/

/****** Confiuration register values *****
*/

/* CFCS register */
#define CFCS_IO_SPACE_ACCESS        0x00000001UL    /* bit 0        */
#define CFCS_MEMORY_SPACE_ACCESS    0x00000002UL    /* bit 1        */
#define CFCS_MASTER_OPERATION       0x00000004UL    /* bit 2        */
#define CFCS_PARITY_ERROR_RESPONSE  0x00000040UL    /* bit 6        */

/* CFLT register */
#define CFLT_INITIALIZE             0x0000FF00UL

/* CFDD register */
#define CFDD_SLEEP_MODE             0x80000000UL    /* bit 31       */

/* CFIT register */
#define CFIT_GET_INTERRUPT_LINE     (0xFF)



/****** Control and status register values *****
*/

/* CSR0 */
#define CSR0_CACHE_ALIGNMENT_NONE   0x00000000UL    /* bits 15:14   */
#define CSR0_CACHE_ALIGNMENT_8      0x00004000UL    /* bits 15:14   */
#define CSR0_CACHE_ALIGNMENT_16     0x00008000UL    /* bits 15:14   */
#define CSR0_CACHE_ALIGNMENT_32     0x0000C000UL    /* bits 15:14   */
#define CSR0_BURST_LENGTH_0         0x00000000UL    /* bits 13:8    */
#define CSR0_BURST_LENGTH_1         0x00000100UL    /* bits 13:8    */
#define CSR0_BURST_LENGTH_2         0x00000200UL    /* bits 13:8    */
#define CSR0_BURST_LENGTH_4         0x00000400UL    /* bits 13:8    */
#define CSR0_BURST_LENGTH_8         0x00000800UL    /* bits 13:8    */
#define CSR0_BURST_LENGTH_16        0x00001000UL    /* bits 13:8    */
#define CSR0_BURST_LENGTH_32        0x00002000UL    /* bits 13:8    */
#define CSR0_SOFTWARE_RESET         (1 << 0)
#define CSR0_BLE         			(1 << 7)
#define CSR0_DBO         			(1 << 20)


/* CSR1 */


/* CSR2 */


/* CSR3 */


/* CSR4 */


/* CSR5 */
#define CSR5_TX_INTERRUPT               (1 << 0)
#define CSR5_TX_PROCESS_STOPPED         (1 << 1)
#define CSR5_TX_BUFFER_UNAVAIL          (1 << 2)
#define CSR5_TX_JABBER_TIMEOUT          (1 << 3)
#define CSR5_AUTONEGOTIATION_COMPLETE   (1 << 4)
#define CSR5_TX_UNDERFLOW               (1 << 5)
#define CSR5_RX_INTERRUPT               (1 << 6)
#define CSR5_RX_BUFFER_UNAVAIL          (1 << 7)
#define CSR5_RX_PROCESS_STOPPED         (1 << 8)
#define CSR5_RX_WATCHDOG_TIMEOUT        (1 << 9)
#define CSR5_EARLY_TX_INTERRUPT         (1 < 10)
#define CSR5_TIMER_EXPIRE_MITIGATION    (1 << 11)
#define CSR5_LINK_FAIL                  (1 << 12)
#define CSR5_FATAL_BUS_ERROR            (1 << 13)
#define CSR5_EARLY_RX_INTERRUPT         (1 << 14)
#define CSR5_ABNORMAL_INTERRUPT_SUMMARY (1 << 15)
#define CSR5_NORMAL_INTERRUPT_SUMMARY   (1 << 16)
#define CSR5_RECEIVE_STATE_MASK         ((1 << 17) | (1 << 18) | (1 << 19))
#define CSR5_TRANSMIT_STATE_MASK        ((1 << 20) | (1 << 21) | (1 << 22))
#define CSR5_ERROR_BITS_MASK            ((1 << 23) | (1 << 24) | (1 << 25))
#define CSR5_GENERAL_PURPOSE_PORT_INT   (1 << 26)
#define CSR5_LINK_CHANGED               (1 << 27)

/* CSR6 */
#define CSR6_START_RX                   (1 << 1)
#define CSR6_PASS_BAD_FRAMES            (1 << 3)
#define CSR6_START_BACKOFF_COUNTER      (1 << 5)
#define CSR6_PROMISCUOUS_MODE           (1 << 6)
#define CSR6_PASS_ALL_MULTICAST         (1 << 7)
#define CSR6_FULL_DUPLEX_MODE           (1 << 9)
#define CSR6_FORCE_COLLISION_MODE       (1 << 12)
#define CSR6_START_TX                   (1 << 13)
#define CSR6_CAPTURE_EFFECT_ENABLE      (1 << 17)
#define CSR6_PORT_SELECT                (1 << 18)
#define CSR6_HEARTBEAT_DISABLE          (1 << 19)
#define CSR6_STORE_AND_FORWARD          (1 << 21)
#define CSR6_TX_THRESHOLD_MODE          (1 << 22)
#define CSR6_PCS_FUNCTION               (1 << 23)
#define CSR6_SCRAMBLER_MODE             (1 << 24)
#define CSR6_IGNORE_DST_ADDRESS_MSB     (1 << 26)
#define CSR6_RX_ALL                     (1 << 30)


/* CSR7 */
#define CSR7_TX_INTERRUPT_ENABLE        (1 << 0)
#define CSR7_TX_STOP_ENABLE             (1 << 1)
#define CSR7_TX_BUFFER_UNAVAIL_ENABLE   (1 << 2)
#define CSR7_TX_JABBER_TO_ENABLE        (1 << 3)
#define CSR7_LINK_PASS_ENABLE           (1 << 4)
#define CSR7_UNDERFLOW_INTERRUPT_ENABLE (1 << 5)
#define CSR7_RX_INTERRUPT_ENABLE        (1 << 6)
#define CSR7_RX_BUFFER_UNAVAIL_ENABLE   (1 << 7)
#define CSR7_RX_STOPPED_ENABLE          (1 << 8)
#define CSR7_RX_WATCHDOG_TO_ENABLE      (1 << 9)
#define CSR7_EARLY_TX_INTERRUPT_ENABLE  (1 << 10)
#define CSR7_GEN_PURP_TIMER_INTERRUPT   (1 << 11)
#define CSR7_LINK_FAIL_ENABLE           (1 << 12)
#define CSR7_FATAL_BUS_ERROR_ENABLE     (1 << 13)
#define CSR7_EARLY_RX_INTERRUPT_ENABLE  (1 << 14)
#define CSR7_ABNORMAL_INTERRUPT_ENABLE  (1 << 15)
#define CSR7_NORMAL_INTERRUPT_ENABLE    (1 << 16)
#define CSR7_GEN_PURP_PORT_ENABLE       (1 << 26)
#define CSR7_LINK_CHANGED_ENABLE        (1 << 27)

/* CSR8 */

/* CSR9 */
#define CSR9_ROM_WRITE_COMMAND          (5 << 6)
#define CSR9_ROM_READ_COMMAND           (6 << 6)
#define CSR9_ROM_ERASE_COMMAND          (7 << 6)


/* CSR10 */

/* CSR11 */

/* CSR12 */
#define CSR12_MII_RX_PORT_ACTIVITY      (1 << 0)
#define CSR12_100MB_LINK_STATUS         (1 << 1)
#define CSR12_10MB_LINK_STATUS          (1 << 2)
#define CSR12_AUTOPOLARITY_STATE        (1 << 3)
#define CSR12_AUI_RX_PORT_ACTIVITY      (1 << 8)
#define CSR12_10BASET_RX_PORT_ACTIVITY  (1 << 9)
#define CSR12_NON_STABLE_NLPS_DETECTED  (1 << 10)
#define CSR12_TRANSMIT_REMOTE_FAULT     (1 << 11)
#define CSR12_LINK_PARTNER_NEGOTIABLE   (1 << 15)
#define CSR12_AUTONEGOTIATION_STATE     (0x7000)        /* bits 14:12   */
#define CSR12_AUTONEGOTIATION_START     (1 << 12)
#define CSR12_AUTONEGOTIATION_COMPLETE  ((1 << 14) | (1 << 12))

#define CSR12_CODE_100MB_FD             (1 << 3)
#define CSR12_CODE_100MB_HD             (1 << 2)
#define CSR12_CODE_10MB_FD              (1 << 1)
#define CSR12_CODE_10MB_HD              (1 << 0)


/* CSR13 */
#define CSR13_SIA_RESET                 (1 << 0)
#define CSR13_SELECT_AUI                (1 << 3)

/* CSR14 */
#define CSR14_ENCODER_ENABLE            (1 << 0)
#define CSR14_LOOPBACK_ENABLE           (1 << 1)
#define CSR14_DRIVER_ENABLE             (1 << 2)
#define CSR14_LINK_PULSE_SEND_ENABLE    (1 << 3)
#define CSR14_NORMAL_COMPENSATION_MODE  ((1 << 4) | (1 << 5))
#define CSR14_10BASET_HD_ENABLE         (1 << 6)
#define CSR14_AUTONEGOTIATION_ENABLE    (1 << 7)
#define CSR14_RECEIVE_SQUELCH_ENABLE    (1 << 8)
#define CSR14_COLLISION_SQUELCH_ENABLE  (1 << 9)
#define CSR14_COLLISION_DETECT_ENABLE   (1 << 10)
#define CSR14_SIGNAL_QUALITY_ENABLE     (1 << 11)
#define CSR14_LINK_TEST_ENABLE          (1 << 12)
#define CSR14_AUTOPOLARITY_ENABLE       (1 << 13)
#define CSR14_SET_POLARITY_PLUS         (1 << 14)
#define CSR14_10BASET_AUI_AUTOSENSING   (1 << 15)
#define CSR14_100BASE_TX_HD             (1 << 16)
#define CSR14_100BASE_TX_FD             (1 << 17)
#define CSR14_100BASE_T4                (1 << 18)


/* CSR15 */
#define CSR15_LED_GEP_0_SELECT          (1 << 20)
#define CSR15_LED_GEP_1_SELECT          (1 << 21)
#define CSR15_LED_GEP_2_SELECT          (1 << 22)
#define CSR15_LED_GEP_3_SELECT          (1 << 23)
#define CSR15_CONTROL_WRITE_ENABLE      (1 << 27)


/* --------------------- end register values ------------------------ */



#define CLEAR_ALL                       0x00000000
#define DEC21143_VENDER_DEVICE_ID       0x00191011
#define DEC21143_SETUP_FRAME_HW_ADDR    156

/* Offset into SRAM for the MAC layer address */
#define IEEE_ADDRESS_OFFSET 0x14

/* Values for reading the EEPROM. */
#define EE_SHIFT_CLK        0x02        /* EEPROM shift clock. */
#define EE_CS			            0x01        /* EEPROM chip select. */
#define EE_DATA_WRITE	      0x04        /* EEPROM chip data in. */
#define EE_WRITE_0		        0x01
#define EE_WRITE_1		        0x05
#define EE_DATA_READ	       0x08        /* EEPROM chip data out. */
#define EE_ENB              (0x4800 | EE_CS)

/* Values for reading and writing to the MII registers. */
#define MDIO_SHIFT_CLK	     0x10000
#define MDIO_DATA_WRITE0    0x00000
#define MDIO_DATA_WRITE1    0x20000
#define MDIO_ENB		          0x00000     /* Ignore the 0x02000 databook setting. */
#define MDIO_ENB_IN		       0x40000
#define MDIO_DATA_READ	     0x80000

/*
 * These defines are the defines for the offsets to the I/O ports for the
 * 8259 PIC controller.
 */
#define MR8259A             0x21       /* offset for IRQ's 0 - 7 */
#define MR8259B             0xa1       /* offset for IRQ's 8 - 15 */
#define IR8259A             0x20       /* mask for enable/disable IRQ's 0 - 7 */
#define IR8259B             0xa0       /* mask for enable/disable IRQ's 8 - 15 */
#define EOI                 0x20       /* end of interrupt value */

#define UNKNOWN_INT         0x7f       /* too much network traffic int */
#define RESET_UNKNOWN_INT   0x0a       /* reset this interrupt         */

/* Define vector numbers for various IRQ lines */
#define IRQ_2_VECTOR        0x0a
#define IRQ_9_VECTOR        0x0a
#define IRQ_3_VECTOR        0x0b
#define IRQ_4_VECTOR        0x0c
#define IRQ_5_VECTOR        0x0d
#define IRQ_7_VECTOR        0x0f
#define IRQ_10_VECTOR       0x72
#define IRQ_11_VECTOR       0x73
#define IRQ_12_VECTOR       0x74
#define IRQ_15_VECTOR       0x77

/* --------------------- PCI register values ------------------------ */

/* Registers for communicating with PCI devices via the PCI bus */
#define DEC21143_PCI_CONFIG_ADDR_REG    0x0CF8
#define DEC21143_PCI_CONFIG_DATA_REG    0x0CFC
#define DEC21143_PCI_CONFIG_ENABLE      (1 << 31)

/* Register offsets for use when communicating with the PCI devices
   via the PCI bus, ie. using the above registers. */
#define PCI_CFID            0x00
#define PCI_CFCS            0x01
#define PCI_CFRV            0x02
#define PCI_CFLT            0x03
#define PCI_CBIO            0x04
#define PCI_CBMA            0x05
#define PCI_RESV0           0x06
#define PCI_RESV1           0x07
#define PCI_RESV2           0x08
#define PCI_RESV3           0x09
#define PCI_CCIS            0x0A
#define PCI_CSID            0x0B
#define PCI_CBER            0x0C
#define PCI_CCAP            0x0D
#define PCI_RESV4           0x0E
#define PCI_CFIT            0x0F
#define PCI_CFDD            0x10
#define PCI_CWUA0           0x11
#define PCI_CWUA1           0x12
#define PCI_SOP0            0x13
#define PCI_SOP1            0x14
#define PCI_CWUC            0x15


/* --------------------- end PCI registers -------------------------- */

#define	ETHER_INT_INDEX	0
#define	ETHER_BASE_ADDR	0x10004000

/* Function prototypes. */

STATUS DEC21143_Init(DV_DEVICE_ENTRY *);
STATUS DEC21143_Get_Address (uint8 *, uchar *);
STATUS DEC21143_Open (uchar *, DV_DEVICE_ENTRY *);
STATUS DEC21143_TX_Packet (DV_DEVICE_ENTRY *, NET_BUFFER *);
STATUS DEC21143_Ioctl(DV_DEVICE_ENTRY *, INT, DV_REQ *);
STATUS DEC21143_Init_Setup_Frame (uint8 *, NET_BUFFER *);
STATUS DEC21143_Process_Setup_Frame (DV_DEVICE_ENTRY *, NET_BUFFER *);
STATUS DEC21143_Negotiate_Link (DV_DEVICE_ENTRY *, uint32 *);
STATUS DEC21143_Init_Link (DV_DEVICE_ENTRY *, uint32 *);
VOID   DEC21143_RX_Packet (DV_DEVICE_ENTRY *, DEC21143_XDATA *);
VOID   DEC21143_RX_HISR (VOID);
VOID   DEC21143_TX_HISR (VOID);
VOID   DEC21143_LISR (INT);
VOID   DEC21143_Reset(DV_DEVICE_ENTRY *);
VOID   DEC21143_Delay(VOID);
VOID   DEC21143_Write_PCI (uint32, uint8, uint32);
VOID   Unknown_Int_LISR (INT vector);
VOID   DEC21143_Update_Setup_Frame (DV_DEVICE_ENTRY *, NET_BUFFER *);
VOID   DEC21143_Negotiate_Link_HISR (VOID);
uint32 DEC21143_Find_PCI_ID (uint8);
uint32 DEC21143_Read_PCI (uint32, uint8);
uint16 DEC21143_Read_EEPROM (uint32, uint8);
DEC21143_DESCRIPTOR *DEC21143_Allocate_Descriptor (uint8 ,NU_MEMORY_POOL *, uint8);

#ifdef __cplusplus
}
#endif