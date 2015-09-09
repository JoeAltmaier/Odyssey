/*************************************************************************/
/*                                                                       */
/*        Copyright (c) 1993-1998 Accelerated Technology, Inc.           */
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
/*      tftpdefs.h                                      TFTP  4.0        */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      TFTP  - Trivial File Transfer Protocol                           */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains data structure definitions used by the TFTP   */
/*      client routines.                                                 */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      Glen Johnson,     Accelerated Technology, Inc.                   */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      TFTP_CB                             TFTP Control Block           */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*      nucleus.h                           Nucleus PLUS constants       */
/*      target.h                                                         */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      G. Johnson      11-27-1995      Created initial version 1.0      */
/*                                                                       */
/*************************************************************************/


/* TFTP Opcodes defined by RFC 783 */
#define TFTP_RRQ_OPCODE       1
#define TFTP_WRQ_OPCODE       2
#define TFTP_DATA_OPCODE      3
#define TFTP_ACK_OPCODE       4
#define TFTP_ERROR_OPCODE     5

/* Connection Status Values */
#define TRANSFERRING_FILE     100
#define ERROR                 101
#define TRANSFER_COMPLETE     102


/* Error codes.  The first 8 (-100 tp -107 are defined by RFC 783. */
#define TFTP_ERROR            -100  /*   Not defined, see error message    */
#define TFTP_FILE_NFOUND      -101  /*   File not found                    */
#define TFTP_ACCESS_VIOLATION -102  /*   Access violation                  */
#define TFTP_DISK_FULL        -103  /*   Disk full or allocation execeeded */
#define TFTP_BAD_OPERATION    -104  /*   Illegal TFTP operation            */
#define TFTP_UNKNOWN_TID      -105  /*   Unknown transfer ID               */
#define TFTP_FILE_EXISTS      -106  /*   File already exists               */
#define TFTP_NO_SUCH_USER     -107  /*   No such user                      */
#define TFTP_NO_MEMORY        -108
#define TFTP_CON_FAILURE      -109


#define TFTP_Packet_Size   516

/* The TFTP timeout value is how long we are willing to wait for a response
   before retransmitting the last packet.  The default value below is
   approximately 1 second. */
#define TFTP_TIMEOUT     (60 * TICKS_PER_SECOND)

/* How many times should a packet be retransmitted before we give up. */
#define TFTP_NUM_RETRANS   3

/* This what a request packet looks like.  Used to overlay a packet so that the
individual fields can be accessed.  */
typedef struct tftp_req
{
    int16  opcode;         /* TFTP opcode */
    char   data[512];
} TFTP_REQ_PKT;

/* This what a all other packets look like.  Used to overlay a packet so that
   the individual fields can be accessed.  Note that for an error packet the
   block_num field is the error code. */
typedef struct tftp_pkt
{
    int16  opcode;         /* TFTP opcode */
    int16  block_num;      /* block number */
    char   data[512];
} TFTP_PKT;

/* TFTP Control Block  */
typedef struct tftp_cb
{
    TFTP_PKT  send_buf;               /* Last packet sent. */
    TFTP_PKT  recv_buf;               /* Last packet received. */
    char HUGE *buf_ptr;               /* Pointer to user's buffer. */
    int32     total_buf_size;         /* Size of the user's buffer. */
    int32     cur_buf_size;           /* How much space is left in buffer. */
    struct addr_struct server_addr;   /* Server's address. */
    int16     socket_number;          /* The socket the is being used. */
    uint16    block_number;           /* Block number of expected packet. */
    int16     tid;                    /* Server's Transfer ID */
    int16     status;                 /* Status of communication. */

} TFTP_CB;

