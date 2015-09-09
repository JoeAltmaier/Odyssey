/*******************************************************************************
 *
 * All software on this website is made available under the following terms and 
 * conditions.  By downloading this software, you agree to abide by these terms
 * and conditions with respect to this software.
 * 
 * I2O SIG All rights reserved.
 * 
 * These header files are provided, pursuant to your I2O SIG membership
 * agreement, free of charge on an as-is basis without warranty of any kind, 
 * either express or implied, including but not limited to, implied warranties 
 * or merchantability and fitness for a particular purpose.  I2O SIG does not 
 * warrant that this program will meet the user's requirements or that the 
 * operation of these programs will be uninterrupted or error-free.  
 * Acceptance and use of this program constitutes the user's understanding 
 * that he will have no recourse to I2O SIG for any actual or consequential 
 * damages including, but not limited to, loss profits arising out of use 
 * or inability to use this program.
 * 
 * Member is permitted to create derivative works to this header-file program.  
 * However, all copies of the program and its derivative works must contain the
 * I2O SIG copyright notice.
 *
 ******************************************************************************/

#if !defined(I2O_LAN_HDR)
#define I2O_LAN_HDR

#include   "i2omsg.h"      /* Include the Base Message file */
#include   "i2outil.h"     /* Include the Utility Message file */

#define I2OLAN_REV 1_5_1   /* I2O LAN header file revision string */

/*****************************************************************************
 *
 *     I2OLAN.h -- I2O Local Area Network Device Class Message defintion file
 *
 *     This file contains information presented in Chapter 6, Section 10 of
 *     the I2O Specification.
 *
 *     Revision History: (Revision History tracks the revision number of the I2O
 *         specification)
 *
 *     .92     - First marked revsion used for Proof of Concept. 
 *     .93     - Change to match the rev .93 of the spec.
 *     .95     - Updated to Rev .95 of 2/5/96.
 *    1.10     - Updated to the 1.x version of the I2O Specification (10/29/96)
 *    1.51     - Updated to the 1.5 version of the I2O Specification (05/01/97)
 *
 *****************************************************************************/

/*
    NOTES:
    
   Gets, reads, receives, etc. are all even numbered functions.
   Sets, writes, sends, etc. are all odd numbered functions.
   Functions that both send and receive data can be either but an attempt is made
       to use the function number that indicates the greater transfer amount.
   Functions that do not send or receive data use odd function numbers.

   Some functions are synonyms like read, receive and send, write.
    
   All common functions will have a code of less than 0x80.
   Unique functions to a class will start at 0x80.
   Executive Functions start at 0xA0.
    
   Utility Message function codes range from 0 - 0x1f
   Base Message function codes range from 0x20 - 0xfe
   Private Message function code is 0xff.
*/

PRAGMA_ALIGN_PUSH

PRAGMA_PACK_PUSH

/* LAN Class specific functions */

#define    I2O_LAN_PACKET_SEND                         0x3B
#define    I2O_LAN_SDU_SEND                            0x3D
#define    I2O_LAN_RECEIVE_POST                        0x3E

/* LAN Generic Defines */

#define    I2O_LAN_ADDRESS_SZ                          0x08

/****************************************************************************/

/* LAN Parameter Groups */

/****************************************************************************/

/* LAN Configuration and Operating Structures and Defines */

/* LAN Groups */

#define    I2O_LAN_DEVICE_INFO_GROUP_NO                0x0000
#define    I2O_LAN_MAC_ADDRESS_GROUP_NO                0x0001
#define    I2O_LAN_MULTICAST_MAC_ADDRESS_GROUP_NO      0x0002
#define    I2O_LAN_BATCH_CONTROL_GROUP_NO              0x0003
#define    I2O_LAN_OPERATION_GROUP_NO                  0x0004
#define    I2O_LAN_MEDIA_OPERATION_GROUP_NO            0x0005
#define    I2O_LAN_ALTERNATE_ADDRESS_GROUP_NO          0x0006
#define    I2O_LAN_TRANSMIT_INFO_GROUP_NO              0x0007
#define    I2O_LAN_RECEIVE_INFO_GROUP_NO               0x0008


/* - 0000h - Device Information Parameters defines */

#define    I2O_LAN_LANTYPE_ETHERNET                    0x0030
#define    I2O_LAN_LANTYPE_100BASEVG                   0x0040
#define    I2O_LAN_LANTYPE_TOKEN_RING                  0x0050
#define    I2O_LAN_LANTYPE_FDDI                        0x0060
#define    I2O_LAN_LANTYPE_FIBRE_CHANNEL               0x0070

#define    I2O_LAN_FLAGS_PHYSICAL_PORT                 0x00
#define    I2O_LAN_FLAGS_EMULATED_LAN                  0x01
#define    I2O_LAN_FLAGS_SIMPLEX                       0x00
#define    I2O_LAN_FLAGS_DUPLEX                        0x02

/* - 0001h - MAC Addresses Parameter Group */

/* Filter Mask defines */

#define    I2O_LAN_USER_DATA_DISABLE                   0x0001
#define    I2O_LAN_PROMISCIOUS_ENABLE                  0x0002
#define    I2O_LAN_PROMISCIOUS_MC_ENABLE               0x0004
#define    I2O_LAN_BROADCAST_DISABLE                   0x0100
#define    I2O_LAN_MULTICAST_DISABLE                   0x0200
#define    I2O_LAN_FUNCTIONAL_ADDRESS_DISABLE          0x0400

#define    I2O_LAN_MAC_NONE                            0x0000
#define    I2O_LAN_MAC_PRIORITY                        0x0800
#define    I2O_LAN_MAC_ALL                             0x1000
#define    I2O_LAN_MAC_PROMISCIOUS                     0x1800


/* - 0003h -  Batch Control Parameters defines */

#define    I2O_BATCH_FLAGS_BATCH_DISABLED              0x01
#define    I2O_BATCH_FLAGS_CURRENT_MODE_BATCHED        0x02
#define    I2O_BATCH_FLAGS_BATCH_FORCED                0x04

/* - 0004h -  Operation Parameters defines */

#define    I2O_OPERATION_FLAGS_REPORT_TX_ERRORS        0x01

/* - 0005h -  Media Operation Parameters defines */

/* Connector Type defines */

#define    I2O_LAN_CONNECTOR_OTHER                     0x0000
#define    I2O_LAN_CONNECTOR_UNKNOWN                   0x0001
#define    I2O_LAN_CONNECTOR_AUI                       0x0002
#define    I2O_LAN_CONNECTOR_UTP                       0x0003
#define    I2O_LAN_CONNECTOR_BNC                       0x0004
#define    I2O_LAN_CONNECTOR_RJ45                      0x0005
#define    I2O_LAN_CONNECTOR_STP_DB9                   0x0006
#define    I2O_LAN_CONNECTOR_FIBER_MIC                 0x0007
#define    I2O_LAN_CONNECTOR_APPLE_AUI                 0x0008
#define    I2O_LAN_CONNECTOR_MII                       0x0009
#define    I2O_LAN_CONNECTOR_COPPER_DB9                0x000A
#define    I2O_LAN_CONNECTOR_COPPER_AW                 0x000B
#define    I2O_LAN_CONNECTOR_OPTICAL_LW                0x000C
#define    I2O_LAN_CONNECTOR_SERIAL_IF                 0x000D
#define    I2O_LAN_CONNECTOR_OPTICAL_SW                0x000E

/* Connection Type defines */

#define    I2O_LAN_CONNECTION_UNKNOWN                  0x0000
#define    I2O_LAN_CONNECTION_AUI                      (I2O_LAN_LANTYPE_ETHERNET << 4) + 0x0001
#define    I2O_LAN_CONNECTION_10BASE5                  (I2O_LAN_LANTYPE_ETHERNET << 4) + 0x0002
#define    I2O_LAN_CONNECTION_FIORL                    (I2O_LAN_LANTYPE_ETHERNET << 4) + 0x0003
#define    I2O_LAN_CONNECTION_10BASE2                  (I2O_LAN_LANTYPE_ETHERNET << 4) + 0x0004
#define    I2O_LAN_CONNECTION_10BROAD36                (I2O_LAN_LANTYPE_ETHERNET << 4) + 0x0005
#define    I2O_LAN_CONNECTION_10BASE_T                 (I2O_LAN_LANTYPE_ETHERNET << 4) + 0x0006
#define    I2O_LAN_CONNECTION_10BASE_FP                (I2O_LAN_LANTYPE_ETHERNET << 4) + 0x0007
#define    I2O_LAN_CONNECTION_10BASE_FB                (I2O_LAN_LANTYPE_ETHERNET << 4) + 0x0008
#define    I2O_LAN_CONNECTION_10BASE_FL                (I2O_LAN_LANTYPE_ETHERNET << 4) + 0x0009
#define    I2O_LAN_CONNECTION_100BASE_TX               (I2O_LAN_LANTYPE_ETHERNET << 4) + 0x000A
#define    I2O_LAN_CONNECTION_100BASE_FX               (I2O_LAN_LANTYPE_ETHERNET << 4) + 0x000B
#define    I2O_LAN_CONNECTION_100BASE_T4               (I2O_LAN_LANTYPE_ETHERNET << 4) + 0x000C
#define    I2O_LAN_CONNECTION_100BASE_VG               (I2O_LAN_LANTYPE_100BASEVG << 4) + 0x0001
#define    I2O_LAN_CONNECTION_4MBIT                    (I2O_LAN_LANTYPE_TOKEN_RING << 4) + 0x0001
#define    I2O_LAN_CONNECTION_16MBIT                   (I2O_LAN_LANTYPE_TOKEN_RING << 4) + 0x0002
#define    I2O_LAN_CONNECTION_125MBAUD                 (I2O_LAN_LANTYPE_FDDI << 4) + 0x0001
#define    I2O_LAN_CONNECTION_POINT_POINT              (I2O_LAN_LANTYPE_FIBRE_CHANNEL << 4) + 0x0001
#define    I2O_LAN_CONNECTION_ARB_LOOP                 (I2O_LAN_LANTYPE_FIBRE_CHANNEL << 4) + 0x0002
#define    I2O_LAN_CONNECTION_PUBLIC_LOOP              (I2O_LAN_LANTYPE_FIBRE_CHANNEL << 4) + 0x0003
#define    I2O_LAN_CONNECTION_FABRIC                   (I2O_LAN_LANTYPE_FIBRE_CHANNEL << 4) + 0x0004
#define    I2O_LAN_CONNECTION_EMULATION                0x0F00
#define    I2O_LAN_CONNECTION_OTHER                    0x0F01

/* Duplex Mode defines */
#define    I2O_LAN_DUPLEX_MODE_HALF_DUPLEX             0x00
#define    I2O_LAN_DUPLEX_MODE_FULL_DUPLEX             0xFF

/* Link Status defines */

#define    I2O_LAN_LINK_STATUS_UNKNOWN                 0x00
#define    I2O_LAN_LINK_STATUS_NORMAL                  0x01
#define    I2O_LAN_LINK_STATUS_FAILURE                 0x02
#define    I2O_LAN_LINK_STATUS_RESET                   0x03
#define    I2O_LAN_LINK_STATUS_OTHER                   0xFF

/* - 0007h -  Transmit Control Parameters defines */

/* Tx Modes bit map defines */

#define    I2O_LAN_TXMODE_RESERVED                     0x0001
#define    I2O_LAN_TXMODE_NO_DA_IN_SGL                 0x0002
#define    I2O_LAN_TXMODE_CRC_SUPPRESSION              0x0004
#define    I2O_LAN_TXMODE_LOOP_SUPPRESSION             0x0008
#define    I2O_LAN_TXMODE_MAC_INSERTION                0x0010
#define    I2O_LAN_TXMODE_RIF_INSERTION                0x0020
#define    I2O_LAN_TXMODE_IP_CHECKSUM                  0x0040



/* LAN Group 0000h - Device Information Parameter Group */

typedef struct _I2O_LAN_DEVICE_SCALAR {
    U16                    LANType;
    U16                    Flags;
    U8                     AddressFormat;
    U8                     reserved1;
    U16                    reserved2;
    U32                    MinPacketSize;
    U32                    MaxPacketSize;
    U8                     HardwareAddress[I2O_LAN_ADDRESS_SZ];
    U64                    MaxTxWireSpeed;
    U64                    MaxRxWireSpeed;
} I2O_LAN_DEVICE_SCALAR, *PI2O_LAN_DEVICE_SCALAR;


/* LAN Group 0001h - MAC Addresses Parameter Group */

typedef struct _I2O_LAN_MAC_ADDRESS_SCALAR {
    U8                     ActiveAddress[I2O_LAN_ADDRESS_SZ];
    U8                     CurrNetworkAddress[I2O_LAN_ADDRESS_SZ];
    U64                    FunctionalAddressMask;
    U32                    FilterMask;
    U32                    HardwareFilterMask;
    U32                    MaxSizeMulticastTable;
    U32                    MaxFilterPerfect;
    U32                    MaxFilterImperfect;
} I2O_LAN_MAC_ADDRESS_SCALAR, *PI2O_LAN_MAC_ADDRESS_SCALAR;


/* LAN Group 0002h - Multicast MAC Addresses Parameter Group */

typedef struct _I2O_LAN_MULTICAST_MAC_ADDRESS_TABLE {
    U8                     MulticastMACAddress[I2O_LAN_ADDRESS_SZ];
} I2O_LAN_MULTICAST_MAC_ADDRESS_TABLE, *PI2O_LAN_MULTICAST_MAC_ADDRESS_TABLE;


/* LAN Group 0003h - Batch Mode Control Parameter Group */

typedef struct _I2O_LAN_BATCH_CONTROL_SCALAR {
    U32                    BatchFlags;
    U32                    RisingLoadDelay;
    U32                    RisingLoadThreshold;
    U32                    FallingLoadDelay;
    U32                    FallingLoadThreshold;
    U32                    MaxBatchCount;
    U32                    MaxBatchDelay;
    U32                    TransCompDelay;
} I2O_LAN_BATCH_CONTROL_SCALAR, *PI2O_LAN_BATCH_CONTROL_SCALAR;


/* LAN Group 0004h - Operations Parameter Group */

typedef struct _I2O_LAN_OPERATION_SCALAR {
    U32                    PacketPrePad;
    U32                    UserFlags;
    U32                    PacketOrphanLimit;
} I2O_LAN_OPERATION_SCALAR, *PI2O_LAN_OPERATION_SCALAR;


/* LAN Group 0005h - Media Operations Parameter Group */

typedef struct _I2O_LAN_MEDIA_OPERATION_SCALAR {
    U32                    ConnectorType;
    U32                    ConnectionType;
    U64                    CurrentTxWireSpeed;
    U64                    CurrentRxWireSpeed;
    U8                     FullDuplexMode;
    U8                     LinkStatus;
    U8                     BadPacketHandling;
} I2O_LAN_MEDIA_OPERATION_SCALAR, *PI2O_LAN_MEDIA_OPERATION_SCALAR;


/* LAN Group 0006h - Alternate MAC Addresses Parameter Group */

typedef struct _I2O_LAN_ALTERNATE_ADDRESS_TABLE {
    U8                     AlternateAddress[I2O_LAN_ADDRESS_SZ];
} I2O_LAN_ALTERNATE_ADDRESS_TABLE, *PI2O_LAN_ALTERNATE_ADDRESS_TABLE;


/* LAN Group 0007h - Transmit Information Parameter Group */

typedef struct _I2O_LAN_TRANSMIT_INFO_SCALAR {
    U32                    TxMaxPacketSG;
    U32                    TxMaxChainSG;
    U32                    TxMaxPktsOut;
    U32                    TxMaxReqPkts;
    U32                    TxModes;
} I2O_LAN_TRANSMIT_INFO_SCALAR, *PI2O_LAN_TRANSMIT_INFO_SCALAR;


/* LAN Group 0008h - Receive Information Parameter Group */

typedef struct _I2O_LAN_RECEIVE_INFO_SCALAR {
    U32                    RxMaxChain;
    U32                    RxMaxBuckets;
} I2O_LAN_RECEIVE_INFO_SCALAR, *PI2O_LAN_RECEIVE_INFO_SCALAR;



/****************************************************************************/

/* Packet Offset flags in the Bucket Descriptor (BD) defines */

#define    I2O_BD_VALID_PACKET                         0x00
#define    I2O_BD_PACKET_ERROR                         0x01
#define    I2O_BD_SKIP_SPACE                           0x02
#define    I2O_BD_PACKET_BUCKET_MASK                   0x0E
#define    I2O_BD_PACKET_CONTINUED                     0x04
#define    I2O_BD_PACKET_NEXT_BUCKET                   0x08
#define    I2O_BD_LAST_PACKET                          0x40
#define    I2O_BD_LAST_BUCKET                          0xC0

/* Packet Status defines */

#define    I2O_LAN_SUCCESS                             0x00
#define    I2O_LAN_CRC_ERROR                           0x01
#define    I2O_LAN_ALIGNMENT_ERROR                     0x02
#define    I2O_LAN_PACKET_TOO_LONG                     0x03
#define    I2O_LAN_PACKET_TOO_SHORT                    0x04
#define    I2O_LAN_RECEIVE_OVERRUN                     0x05
#define    I2O_LAN_OTHER_ERROR                         0xFF

/* Packet/Bucket Descriptor field size defines */

#define    I2O_PACKET_OFFSET_LENGTH_SZ                 24
#define    I2O_PACKET_FLAGS_SZ                         8
#define    I2O_PACKET_LENGTH_SZ                        24
#define    I2O_PACKET_STATUS_SZ                        8


/* LAN Single Packet Descriptor  */

typedef struct _I2O_PACKET_INFO {
        U32                PacketOffset:I2O_PACKET_OFFSET_LENGTH_SZ;
        U32                PacketFlags:I2O_PACKET_FLAGS_SZ;
        U32                PacketLength:I2O_PACKET_LENGTH_SZ;
        U32                PacketStatus:I2O_PACKET_STATUS_SZ;
} I2O_PACKET_INFO, *PI2O_PACKET_INFO;


/* LAN Receive Bucket Descriptor (BD) */

typedef struct _I2O_BUCKET_DESCRIPTOR {
    I2O_TRANSACTION_CONTEXT    BucketContext;        /* aka TransactionContext */
    I2O_PACKET_INFO            PacketInfo[1];
} I2O_BUCKET_DESCRIPTOR, *PI2O_BUCKET_DESCRIPTOR;


/* LAN Receive Packet Descriptor Block (PDB) */

typedef struct _I2O_PACKET_DESCRIPTOR {
    I2O_BUCKET_DESCRIPTOR      BDEntry[1];
} I2O_PACKET_DESCRIPTOR, *PI2O_PACKET_DESCRIPTOR;


/****************************************************************************/

/* Event Masks */

#define    I2O_LAN_EVENT_LINK_DOWN                     0x01
#define    I2O_LAN_EVENT_LINK_UP                       0x02
#define    I2O_LAN_EVENT_MEDIA_CHANGE                  0x04

/****************************************************************************/

/* Transmit Control Word defines */

#define    I2O_TCW_ACCESS_PRIORITY_MASK                0x00000007
#define    I2O_TCW_SUPPRESS_CRC                        0x00000008
#define    I2O_TCW_SUPPRESS_LOOPBACK                   0x00000010
#define    I2O_TCW_USE_BATCH_RULES                     0x00000000
#define    I2O_TCW_REPLY_IMMEDIATELY                   0x40000000
#define    I2O_TCW_REPLY_UNSUCCESSFUL                  0x80000000
#define    I2O_TCW_NO_REPLY_REQUIRED                   0xC0000000

/****************************************************************************/

/* DetailedStatusCode defines */

#define    I2O_LAN_DSC_SUCCESS                         0x00
#define    I2O_LAN_DSC_DEVICE_FAILURE                  0x01
#define    I2O_LAN_DSC_DESTINATION_NOT_FOUND           0x02
#define    I2O_LAN_DSC_TRANSMIT_ERROR                  0x03
#define    I2O_LAN_DSC_TRANSMIT_ABORTED                0x04
#define    I2O_LAN_DSC_RECEIVE_ERROR                   0x05
#define    I2O_LAN_DSC_RECEIVE_ABORTED                 0x06
#define    I2O_LAN_DSC_DMA_ERROR                       0x07
#define    I2O_LAN_DSC_BAD_PACKET_DETECTED             0x08
#define    I2O_LAN_DSC_OUT_OF_MEMORY                   0x09
#define    I2O_LAN_DSC_BUCKET_OVERRUN                  0x0A
#define    I2O_LAN_DSC_IOP_INTERNAL_ERROR              0x0B
#define    I2O_LAN_DSC_CANCELED                        0x0C
#define    I2O_LAN_DSC_INVALID_TRANSACTION_CONTEXT     0x0D
#define    I2O_LAN_DSC_DESTINATION_ADDRESS_DETECTED    0x0E
#define    I2O_LAN_DSC_DESTINATION_ADDRESS_OMITTED     0x0F
#define    I2O_LAN_DSC_PARTIAL_PACKET_RETURNED         0x10


/****************************************************************************/


/* I2O LAN Packet or SDU Send Function Message Frame structure. */

typedef struct _I2O_LAN_SEND_MESSAGE {
   I2O_MESSAGE_FRAME           StdMessageFrame;
   U32                         TransmitControlWord;
   I2O_SG_ELEMENT              SGL;
} I2O_LAN_SEND_MESSAGE, *PI2O_LAN_SEND_MESSAGE;


    /* I2O LAN Send Reply function structure */

typedef struct _I2O_LAN_SEND_REPLY {
   I2O_MESSAGE_FRAME           StdMessageFrame;
   I2O_TRL_CONTROL_WORD        TrlControlWord;
   U16                         DetailedStatusCode;
   U8                          reserved;
   U8                          RequestStatus;
   I2O_TRANSACTION_CONTEXT     TransactionContext[1];
} I2O_LAN_SEND_REPLY, *PI2O_LAN_SEND_REPLY;

/****************************************************************************/


/* I2O LAN Receive Post Buckets Function Message Frame structure. */

typedef struct _I2O_LAN_RECEIVE_POST_MESSAGE {
   I2O_MESSAGE_FRAME           StdMessageFrame;
   U32                         BucketCount;
   I2O_SG_ELEMENT              SGL;
} I2O_LAN_RECEIVE_POST_MESSAGE, *PI2O_LAN_RECEIVE_POST_MESSAGE;


    /* I2O LAN Indirect Receive Reply function structure */

typedef struct _I2O_LAN_RECEIVE_REPLY {
   I2O_MESSAGE_FRAME           StdMessageFrame;
   I2O_TRL_CONTROL_WORD        TrlControlWord;
   U16                         DetailedStatusCode;
   U8                          reserved;
   U8                          RequestStatus;
   U32                         BucketsRemaining;
   I2O_PACKET_DESCRIPTOR       PdbArray;
} I2O_LAN_RECEIVE_REPLY, *PI2O_LAN_RECEIVE_REPLY;


/****************************************************************************/


/* LAN Statistics Structures and Defines */

#define    I2O_LAN_HISTORICAL_STATS_GROUP_NO                       0x0100
#define    I2O_LAN_SUPPORTED_OPTIONAL_HISTORICAL_STATS_GROUP_NO    0x0180
#define    I2O_LAN_OPTIONAL_HISTORICAL_STATS_GROUP_NO              0x0181
#define    I2O_LAN_802_3_HISTORICAL_STATS_GROUP_NO                 0x0200
#define    I2O_LAN_SUPPORTED_802_3_HISTORICAL_STATS_GROUP_NO       0x0280
#define    I2O_LAN_OPTIONAL_802_3_HISTORICAL_STATS_GROUP_NO        0x0281
#define    I2O_LAN_802_5_HISTORICAL_STATS_GROUP_NO                 0x0300
#define    I2O_LAN_SUPPORTED_802_5_HISTORICAL_STATS_GROUP_NO       0x0380
#define    I2O_LAN_OPTIONAL_802_5_HISTORICAL_STATS_GROUP_NO        0x0381
#define    I2O_LAN_FDDI_HISTORICAL_STATS_GROUP_NO                  0x0400
#define    I2O_LAN_SUPPORTED_FDDI_HISTORICAL_STATS_GROUP_NO        0x0480
#define    I2O_LAN_OPTIONAL_FDDI_HISTORICAL_STATS_GROUP_NO         0x0481
#define    I2O_LAN_FC_HISTORICAL_STATS_GROUP_NO                    0x0500
#define    I2O_LAN_SUPPORTED_FC_HISTORICAL_STATS_GROUP_NO          0x0580
#define    I2O_LAN_OPTIONAL_FC_HISTORICAL_STATS_GROUP_NO           0x0581


/* LAN Group 0100h - Required Historical Statistics Parameter Group */

typedef struct _I2O_LAN_HIST_STATS_SCALAR {
    U64                    TotalPacketsTransmitted;
    U64                    TotalBytesTransmitted;
    U64                    TotalPacketsReceived;
    U64                    TotalBytesReceived;
    U64                    TotalTransmitErrors;
    U64                    TotalReceiveErrors;
    U64                    ReceiveNoBuffer;
    U64                    AdapterResetCount;
} I2O_LAN_HIST_STATS_SCALAR, *PI2O_LAN_HIST_STATS_SCALAR;


/* LAN Group 0180h - Optional Supported Generic Historical Statistics Parameter Group */

typedef struct _I2O_LAN_OPT_HIST_STATS_SUPP_SCALAR {
    U64                    StatisticsSupported;
} I2O_LAN_OPT_HIST_STATS_SUPP_SCALAR, *PI2O_LAN_OPT_HIST_STATS_SUPP_SCALAR;


/* LAN Group 0181h - Optional Generic Historical Statistics Parameter Group */

typedef struct _I2O_LAN_OPT_HIST_STATS_SCALAR {
    U64                    TxRetryCount;
    U64                    ReceiveCRCErrorCount;
    U64                    DirectedBytesTx;
    U64                    DirectedPacketsTx;
    U64                    MulticastBytesTx;
    U64                    MulticastPacketsTx;
    U64                    BroadcastBytesTx;
    U64                    BroadcastPacketsTx;
    U64                    DirectedBytesRx;
    U64                    DirectedPacketsRx;
    U64                    MulticastBytesRx;
    U64                    MulticastPacketsRx;
    U64                    BroadcastBytesRx;
    U64                    BroadcastPacketsRx;
    U64                    TotalGroupAddrTxCount;
    U64                    TotalGroupAddrRxCount;
} I2O_LAN_OPT_HIST_STATS_SCALAR, *PI2O_LAN_OPT_HIST_STATS_SCALAR;


/*-----------------------------------*/
/* - 802.3 Media specific statistics */

/* LAN Group 0200h - Required 802.3 Historical Statistics Parameter Group */

typedef struct _I2O_LAN_802_3_HIST_STATS_SCALAR {
    U64                    RxAlignmentError;
    U64                    TxOneCollision;
    U64                    TxMultipleCollisions;
    U64                    TxDeferred;
    U64                    TxLateCollision;
    U64                    TxMaxCollisions;
    U64                    TxCarrierLost;
    U64                    TxExcessiveDeferrals;
} I2O_LAN_802_3_HIST_STATS_SCALAR, *PI2O_LAN_802_3_HIST_STATS_SCALAR;


/* LAN Group 0280h - Optional Supported 802.3 Historical Statistics Parameter Group */

typedef struct _I2O_LAN_OPT_802_3_HIST_STATS_SUPP_SCALAR {
    U64                    StatisticsSupported;
} I2O_LAN_OPT_802_3_HIST_STATS_SUPP_SCALAR, *PI2O_LAN_OPT_802_3_HIST_STATS_SUPP_SCALAR;


/* LAN Group 0281h - Optional 802.3 Historical Statistics Parameter Group */

typedef struct _I2O_LAN_OPT_802_3_HIST_STATS_SCALAR {
    U64                    RxOverrun;
    U64                    TxUnderrun;
    U64                    TxHeartbeatFailure;
} I2O_LAN_OPT_802_3_HIST_STATS_SCALAR, *PI2O_LAN_OPT_802_3_HIST_STATS_SCALAR;


/*-----------------------------------*/
/* - 802.5 Media specific statistics */

/* LastRingStatus bit map defines */

#define    I2O_LAN_802_5_RING_STATUS_RING_RECOVERY     0x0000000000000020
#define    I2O_LAN_802_5_RING_STATUS_SINGLE_STATION    0x0000000000000040
#define    I2O_LAN_802_5_RING_STATUS_COUNTER_OVERFLOW  0x0000000000000080
#define    I2O_LAN_802_5_RING_STATUS_REMOVE_RECEIVED   0x0000000000000100
#define    I2O_LAN_802_5_RING_STATUS_AUTO_REMOVAL_ERROR 0x0000000000000400
#define    I2O_LAN_802_5_RING_STATUS_LOBE_WIRE_FAULT   0x0000000000000800
#define    I2O_LAN_802_5_RING_STATUS_TRANSMIT_BEACON   0x0000000000001000
#define    I2O_LAN_802_5_RING_STATUS_SOFT_ERROR        0x0000000000002000
#define    I2O_LAN_802_5_RING_STATUS_HARD_ERROR        0x0000000000004000
#define    I2O_LAN_802_5_RING_STATUS_SIGNAL_LOSS       0x0000000000008000


/* LAN Group 0300h - Required 802.5 Historical Statistics Parameter Group */

typedef struct _I2O_LAN_802_5_HIST_STATS_SCALAR {
    U64                    LineErrors;
    U64                    LostFrames;
    U64                    ACError;
    U64                    TxAbortDelimter;
    U64                    BurstErrors;
    U64                    FrameCopiedErrors;
    U64                    FrequencyErrors;
    U64                    InternalError;
    U64                    LastRingStatus;
    U64                    TokenError;
    U64                    UpstreamNodeAddress;
    U64                    LastRingID;
    U64                    LastBeaconType;
} I2O_LAN_802_5_HIST_STATS_SCALAR, *PI2O_LAN_802_5_HIST_STATS_SCALAR;


/* LAN Group 0380h - Optional Supported 802.5 Historical Statistics Parameter Group */

typedef struct _I2O_LAN_OPT_802_5_HIST_STATS_SUPP_SCALAR {
    U64                    StatisticsSupported;
} I2O_LAN_OPT_802_5_HIST_STATS_SUPP_SCALAR, *PI2O_LAN_OPT_802_5_HIST_STATS_SUPP_SCALAR;


/* LAN Group 0381h - Optional 802.5 Historical Statistics Parameter Group */

/* typedef struct _I2O_LAN_OPT_802_5_HIST_STATS_SCALAR { */
    /* None defined at this time */
/* } I2O_LAN_OPT_802_5_HIST_STATS_SCALAR, *PI2O_LAN_OPT_802_5_HIST_STATS_SCALAR; */


/*-----------------------------------*/
/* - FDDI Media specific statistics */

/* ConfigurationState defines */

#define    I2O_LAN_FDDI_CONFIG_STATE_ISOLATED          0x0000000000000000
#define    I2O_LAN_FDDI_CONFIG_STATE_LOCAL_A           0x0000000000000001
#define    I2O_LAN_FDDI_CONFIG_STATE_LOCAL_B           0x0000000000000002
#define    I2O_LAN_FDDI_CONFIG_STATE_LOCAL_AB          0x0000000000000003
#define    I2O_LAN_FDDI_CONFIG_STATE_LOCAL_S           0x0000000000000004
#define    I2O_LAN_FDDI_CONFIG_STATE_WRAP_A            0x0000000000000005
#define    I2O_LAN_FDDI_CONFIG_STATE_WRAP_B            0x0000000000000006
#define    I2O_LAN_FDDI_CONFIG_STATE_WRAP_AB           0x0000000000000007
#define    I2O_LAN_FDDI_CONFIG_STATE_WRAP_S            0x0000000000000008
#define    I2O_LAN_FDDI_CONFIG_STATE_C_WRAP_A          0x0000000000000009
#define    I2O_LAN_FDDI_CONFIG_STATE_C_WRAP_B          0x000000000000000A
#define    I2O_LAN_FDDI_CONFIG_STATE_C_WRAP_S          0x000000000000000B
#define    I2O_LAN_FDDI_CONFIG_STATE_THROUGH           0x000000000000000C

/* Ring Management State (RingMgmtState) defines */

#define    I2O_LAN_FDDI_RING_MGMT_STATE_ISOLATED       0x0000000000000000
#define    I2O_LAN_FDDI_RING_MGMT_STATE_NON_OP         0x0000000000000001
#define    I2O_LAN_FDDI_RING_MGMT_STATE_RIND_OP        0x0000000000000002
#define    I2O_LAN_FDDI_RING_MGMT_STATE_DETECT         0x0000000000000003
#define    I2O_LAN_FDDI_RING_MGMT_STATE_NON_OP_DUP     0x0000000000000004
#define    I2O_LAN_FDDI_RING_MGMT_STATE_RING_OP_DUP    0x0000000000000005
#define    I2O_LAN_FDDI_RING_MGMT_STATE_DIRECTED       0x0000000000000006
#define    I2O_LAN_FDDI_RING_MGMT_STATE_TRACE          0x0000000000000007

/* LConnection State defines */

#define    I2O_LAN_FDDI_LCONNECT_STATE_OFF             0x0000000000000000
#define    I2O_LAN_FDDI_LCONNECT_STATE_BREAK           0x0000000000000001
#define    I2O_LAN_FDDI_LCONNECT_STATE_TRACE           0x0000000000000002
#define    I2O_LAN_FDDI_LCONNECT_STATE_CONNECT         0x0000000000000003
#define    I2O_LAN_FDDI_LCONNECT_STATE_NEXT            0x0000000000000004
#define    I2O_LAN_FDDI_LCONNECT_STATE_SIGNAL          0x0000000000000005
#define    I2O_LAN_FDDI_LCONNECT_STATE_JOIN            0x0000000000000006
#define    I2O_LAN_FDDI_LCONNECT_STATE_VERIFY          0x0000000000000007
#define    I2O_LAN_FDDI_LCONNECT_STATE_ACTIVE          0x0000000000000008
#define    I2O_LAN_FDDI_LCONNECT_STATE_MAINTENANCE     0x0000000000000009


/* LAN Group 0400h - Required FDDI Historical Statistics Parameter Group */

typedef struct _I2O_LAN_FDDI_HIST_STATS_SCALAR {
    U64                    ConfigurationState;
    U64                    UpstreamNode;
    U64                    DownStreamNode;
    U64                    FrameErrors;
    U64                    FramesLost;
    U64                    RingMgmtState;
    U64                    LCTFailures;
    U64                    LEMRejects;
    U64                    LEMCount;
    U64                    LConnectionState;
} I2O_LAN_FDDI_HIST_STATS_SCALAR, *PI2O_LAN_FDDI_HIST_STATS_SCALAR;


/* LAN Group 0480h - Optional Supported FDDI Historical Statistics Parameter Group */

typedef struct _I2O_LAN_OPT_FDDI_HIST_STATS_SUPP_SCALAR {
    U64                    StatisticsSupported;
} I2O_LAN_OPT_FDDI_HIST_STATS_SUPP_SCALAR, *PI2O_LAN_OPT_FDDI_HIST_STATS_SUPP_SCALAR;


/* LAN Group 0481h - Optional FDDI Historical Statistics Parameter Group */

/*typedef struct _I2O_LAN_OPT_FDDI_HIST_STATS_SCALAR { */
    /* None defined at this time */
/*} I2O_LAN_OPT_FDDI_HIST_STATS_SCALAR, *PI2O_LAN_OPT_FDDI_HIST_STATS_SCALAR; */



/*-----------------------------------*/
/* - Fibre Channel Media specific statistics */

/* LAN Group 0500h - Required Fibre Channel Historical Statistics Parameter Group */

/* typedef struct _I2O_LAN_FC_HIST_STATS_SCALAR { */
    /* None defined at this time */
/* } I2O_LAN_FC_HIST_STATS_SCALAR, *PI2O_LAN_FC_HIST_STATS_SCALAR; */


/* LAN Group 0580h - Optional Supported Fibre Channel Historical Statistics Parameter Group */

typedef struct _I2O_LAN_OPT_FC_HIST_STATS_SUPP_SCALAR {
    U64                    StatisticsSupported;
} I2O_LAN_OPT_FC_HIST_STATS_SUPP_SCALAR, *PI2O_LAN_OPT_FC_HIST_STATS_SUPP_SCALAR;


/* LAN Group 0581h - Optional Fibre Channel Historical Statistics Parameter Group */

/* typedef struct _I2O_LAN_OPT_FC_HIST_STATS_SCALAR { */
    /* None defined at this time */
/* } I2O_LAN_OPT_FC_HIST_STATS_SCALAR, *PI2O_LAN_OPT_FC_HIST_STATS_SCALAR; */


PRAGMA_PACK_POP

PRAGMA_ALIGN_POP

#endif     /* I2O_LAN_HDR */


