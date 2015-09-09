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
/*      tftpc.c                                         TFTP  4.0        */
/*                                                                       */
/* COMPONENT                                                             */
/*                                                                       */
/*      TFTPC  -  TFTP Client                                            */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This file contains the TFTP routines necessary to get and to put */
/*      a file to a  TFTP server.                                        */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      Glen Johnson,     Accelerated Technology, Inc.                   */
/*                                                                       */
/* DATA STRUCTURES                                                       */
/*                                                                       */
/*      None                                                             */
/*                                                                       */
/* FUNCTIONS                                                             */
/*                                                                       */
/*      TFTPC_Get                 Get a file from a TFTP server.         */
/*      TFTPC_Put                 Put a file to a TFTP server.           */
/*      TFTPC_Read_Request        Send a read request to a TFTP server.  */
/*      TFTPC_Recv                Recv a packet.                         */
/*      TFTPC_Process_Data        Process a data packet.                 */
/*      TFTPC_Ack                 Send a TFTP ack.                       */
/*      TFTPC_Write_Request       Send a write request to a TFTP server. */
/*      TFTPC_Process_Ack         Process an ack packet.                 */
/*      TFTPC_Send_Data           Send a TFTP data packet.               */
/*      TFTPC_Retransmit          Retransmit the last TFTP packet.       */
/*      TFTPC_Error               Send a TFTP error packet.              */
/*                                                                       */
/* DEPENDENCIES                                                          */
/*                                                                       */
/*      tftp.h                              TFTP data structures         */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      G. Johnson      11-27-1995      Created initial version 1.0      */
/*      D. Sharer       04-13-1998      Modified for Net 4.0             */
/*                                                                       */
/*************************************************************************/


#include"nucleus.h"
#include "target.h"
#include "protocol.h"
#include"sockdefs.h"
#include"socketd.h"
#include"externs.h"
#include"tftpdefs.h"
#include"tftpextr.h"
#include"tftp.h"

extern NU_MEMORY_POOL  System_Memory;

/* TFTP defines 8 error codes, with values 0 through 7.  In the event that one
*  of these errors is received it will be passed back to the application.  Since
*  a value of 0 is defined to be NU_SUCCESS in the Nucleus PLUS OS it is
*  necessary to redefine the first error code, and since the convention at ATI
*  is to return negative numbers for error codes all were redefined below.
*
*  VALUE       ERROR
*  -100        Not defined, see error message (if any)
*  -101        File not found
*  -102        Access violation
*  -103        Disk full or allocation execeeded
*  -104        Illegal TFTP operation
*  -105        Unknown transfer ID
*  -106        File already exists
*  -107        No such user
*/
int16  tftp_errors[] = {-100, -101, -102, -103, -104, -105, -106, -107};

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      TFTPC_Get                                                        */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function is called from a user application.  It retrieves a */
/*      file from a TFTP server.                                         */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      Glen Johnson,     Accelerated Technology, Inc.                   */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      NU_Allocate_Memory                  Allocate memory              */
/*      NU_Deallocate_Memory                Deallocate memory            */
/*      TFTPC_Read_Request                  Send a tftp read request     */
/*      TFTPC_Recv                          Receive a tftp packet        */
/*      TFTPC_Process_Data                  Process a received packet    */
/*      TFTPC_Retransmit                    Retransmit the last packet   */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      fbuffer                             Pointer to the user's buffer.*/
/*      buf_size                            Size of the user's buffer.   */
/*      remote_ip                           The target ip address.       */
/*      remote_fname                        The file to get.             */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      The number of bytes received when succesful.  Else various error */
/*      codes are returned.                                              */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      G. Johnson      11-20-1995      Created initial version 1.0      */
/*                                                                       */
/*************************************************************************/
int32 TFTPC_Get(char *fbuffer, int32 buf_size, char *remote_ip,
                char *remote_fname)
{
    TFTP_CB  *tftp_con;
    STATUS   status;
    int16    bytes_received, retransmits;
    int32    total_bytes;

    /* Is the buffer size valid. */
    if(buf_size <= 0)
        return (-1);

    /* Allocate memory for the TFTP Control Block. */
    status = NU_Allocate_Memory (&System_Memory, (VOID **)&tftp_con,
                                 sizeof(TFTP_CB), NU_SUSPEND);
    if (status != NU_SUCCESS)
    {
        return(TFTP_NO_MEMORY);
    }

    /* initialize the buffer size fields of the TFTP CB. */
    tftp_con->total_buf_size = buf_size;
    tftp_con->cur_buf_size = buf_size;

    /* Setup the pointer to the user's buffer. */
    tftp_con->buf_ptr = fbuffer;

    /* Send a TFTP Read Request to a server. */
    if ((status = TFTPC_Read_Request(remote_ip, remote_fname,
                                     tftp_con)) < 0)
    {
        NU_Deallocate_Memory((void *) tftp_con);
        return(status);
    }

    /* Initialize retransmits. */
    retransmits = 0;

    /* Get the servers response. */
    while(((bytes_received = TFTPC_Recv(tftp_con)) == NU_NO_DATA) &&
          (retransmits < TFTP_NUM_RETRANS))
    {
            TFTPC_Retransmit(tftp_con, (int16)status);
            retransmits++;
    }

    /* If the response is what we expected, then setup the server's TID.  Else
       return an error. */
    if (bytes_received)
        tftp_con->tid = tftp_con->server_addr.port;
    else
    {
        NU_Deallocate_Memory((void *) tftp_con);
        return(TFTP_CON_FAILURE);
    }

    /* Process the first data packet that was received from the server. */
    if((status = TFTPC_Process_Data(tftp_con, bytes_received)) != NU_SUCCESS)
    {
        NU_Deallocate_Memory((void *) tftp_con);
        return(status);
    }

    while (tftp_con->status == TRANSFERRING_FILE)
    {
        /* Initialize retransmits. */
        retransmits = 0;

        /* Retrieve the next data packet. */
        while(((bytes_received = TFTPC_Recv(tftp_con)) == NU_NO_DATA) &&
              (retransmits < TFTP_NUM_RETRANS))
        {
            TFTPC_Retransmit(tftp_con, 4);
            retransmits++;
        }

        /* If a data packet was received then process it.  Else a problem
           occured, and we need to exit. */
        if (bytes_received)
        {
            if((status = TFTPC_Process_Data(tftp_con, bytes_received)) !=
                                    NU_SUCCESS)
                tftp_con->status = ERROR;
        }
        else
        {
            tftp_con->status = ERROR;
            status = TFTP_CON_FAILURE;
        }
    }

    /* If everything went okay then calculate the number of bytes that were
       received in this file. Else we should return the last error code that was
       received.  */
    if(status == NU_SUCCESS)
        total_bytes = tftp_con->total_buf_size - tftp_con->cur_buf_size;
    else
        total_bytes = status;

    /* Close the socket, freeing any resources that were used. */
    NU_Close_Socket(tftp_con->socket_number);

    /* Deallocate the memory used for the TFTP control block. */
    NU_Deallocate_Memory((void *) tftp_con);

    return(total_bytes);
} /* end TFTPC_Get */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      TFTPC_Put                                                        */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function is called from a user application.  It puts a file */
/*      to a TFTP server.                                                */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      Glen Johnson,     Accelerated Technology, Inc.                   */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      Application                                                      */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      NU_Allocate_Memory                  Allocate memory              */
/*      NU_Deallocate_Memory                Deallocate memory            */
/*      TFTPC_Write_Request                 Send a tftp write request    */
/*      TFTPC_Recv                          Receive a tftp packet        */
/*      TFTPC_Process_Ack                   Process a received packet    */
/*      TFTPC_Retransmit                    Retransmit the last packet   */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      fbuffer                             Pointer to the user's buffer.*/
/*      buf_size                            Amount of data in user's Buf.*/
/*      remote_ip                           The target ip address.       */
/*      remote_fname                        What the file will be called */
/*                                          on the remote host.          */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      The number of bytes put to the server when succesful.  Else      */
/*      various error conditions are returned.                           */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      G. Johnson      11-20-1995      Created initial version 1.0      */
/*                                                                       */
/*************************************************************************/
int32 TFTPC_Put(char *fbuffer, int32 buf_size, char *remote_ip,
                char *remote_fname)
{
    TFTP_CB  *tftp_con;
    STATUS   status;
    int16    bytes_received, retransmits, bytes_sent = 0;
    int32    total_bytes;

    /* Is the buffer size valid. */
    if(buf_size <= 0)
        return (-1);

    /* Allocate memory for the TFTP Control Block. */
    status = NU_Allocate_Memory (&System_Memory, (VOID **)&tftp_con,
                                 sizeof(TFTP_CB), NU_SUSPEND);
    if (status != NU_SUCCESS)
    {
        return(TFTP_NO_MEMORY);
    }

    tftp_con->total_buf_size = buf_size;
    tftp_con->cur_buf_size = buf_size;

    /* Setup a pointer to the user's buffer. */
    tftp_con->buf_ptr = fbuffer;

    /* Send a write request to the server. */
    if ((status = TFTPC_Write_Request(remote_ip, remote_fname,
                                     tftp_con)) < 0)
    {
        NU_Deallocate_Memory((void *) tftp_con);
        return(TFTP_CON_FAILURE);
    }

    /* Initialize the retransmit counter. */
    retransmits = 0;

    /* While a ack of the write request has not been received and the maximum
       number of retransmits has not yet been reached.  Retransmit the write
       request. */
    while(((bytes_received = TFTPC_Recv(tftp_con)) == NU_NO_DATA) &&
          (retransmits < TFTP_NUM_RETRANS))
    {
        TFTPC_Retransmit(tftp_con, bytes_sent);
        retransmits++;
    }

    if (bytes_received > 0)
        tftp_con->tid = tftp_con->server_addr.port;
    else
    {
        NU_Deallocate_Memory((void *) tftp_con);
        return(TFTP_CON_FAILURE);
    }

    /* Process the server's response. */
    if((status = TFTPC_Process_Ack(tftp_con)) != NU_SUCCESS)
    {
        NU_Deallocate_Memory((void *) tftp_con);
        return(status);
    }

    /* The connection is now established.  While the status is ok continue to
       transmit data.  */
    while (tftp_con->status == TRANSFERRING_FILE)
    {
        /* Send a data packet. */
        if((bytes_sent = TFTPC_Send_Data(tftp_con)) < 0)
        {
            status = TFTP_CON_FAILURE;
            break;
        }

        /* Initialize the retransmit counter. */
        retransmits = 0;

        /* While a ack of the data packet has not been received and the maximum
           number of retransmits has not yet been reached.  Retransmit the last
           data packet. */
        while(((bytes_received = TFTPC_Recv(tftp_con)) == NU_NO_DATA) &&
              (retransmits < TFTP_NUM_RETRANS))
        {
            TFTPC_Retransmit(tftp_con, bytes_sent);
            retransmits++;
        }

        /* Process the ACK. */
        if (bytes_received)
        {
            if((status = TFTPC_Process_Ack(tftp_con)) != NU_SUCCESS)
            {
                break;
            }
        }
        else
        {
            status = bytes_received;
            tftp_con->status = ERROR;
        }
    }  /* while transferring file. */

    /* If everything went okay then calculate the number of bytes that were
       sent. Else we should return the last error code that was
       received.  */
    if(status == NU_SUCCESS)
        total_bytes = tftp_con->total_buf_size - tftp_con->cur_buf_size;
    else
        total_bytes = status;

    /* Close the socket, freeing any resources that were used. */
    NU_Close_Socket(tftp_con->socket_number);

    /* Deallocate the memory used for the TFTP control block. */
    NU_Deallocate_Memory((void *) tftp_con);

    return(total_bytes);

} /* end TFTPC_Put */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      TFTPC_Read_Request                                               */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function is responsible for sending a TFTP read request to  */
/*      a TFTP server.                                                   */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      Glen Johnson,     Accelerated Technology, Inc.                   */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      TFTPC_Get                                                        */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      NU_Socket                           Create a UDP socket          */
/*      NU_Send_To                          Send a tftp read request     */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      remote_ip                           The server's ip address.     */
/*      remote_fname                        The file to read.            */
/*      tftp_con                            The pointer to TFTP Control  */
/*                                          Block                        */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_SUCCESS                The request was successfully sent.     */
/*      NU_NO_MEMORY              The socket descriptor was not allocated*/
/*      NU_NO_SOCKET_SPACE        All sockets descriptors are used.      */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      G. Johnson      11-20-1995      Created initial version 1.0      */
/*                                                                       */
/*************************************************************************/
STATUS TFTPC_Read_Request(char *remote_ip,
                char *remote_fname, TFTP_CB *tftp_con)
{
    int16         bytes_sent, send_size;
    TFTP_REQ_PKT  *pkt;
    int		i;

    /* Create a socket. */
    if ((tftp_con->socket_number =
            NU_Socket(NU_FAMILY_IP, NU_TYPE_DGRAM, 0)) < 0)
    {
        return(TFTP_CON_FAILURE);
    }

    /* fill in a structure with the server address */
    tftp_con->server_addr.family    = NU_FAMILY_IP;
    tftp_con->server_addr.port      = 69;
    memcpy(tftp_con->server_addr.id.is_ip_addrs, remote_ip, 4);
    tftp_con->server_addr.name = "tftp";

    /* Initialize block_number and transfer status. */
    tftp_con->block_number = 1;
    tftp_con->status = TRANSFERRING_FILE;

    pkt = (TFTP_REQ_PKT *)&tftp_con->send_buf;

    pkt->opcode = intswap(TFTP_RRQ_OPCODE);
    sprintf(pkt->data, "%s%c%s%c", remote_fname, 0, "octet", 0);

    send_size = strlen(remote_fname) + 1 + strlen("octet");

    /* Send the read request. */
    bytes_sent = NU_Send_To(tftp_con->socket_number,
                            (char *)pkt, (uint16)(send_size+2),
                            0, &tftp_con->server_addr, 0);

    return (bytes_sent);
} /* end TFTPC_Read_Request */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      TFTPC_Recv                                                       */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function is responsible for receving data from a TFTP       */
/*      server.  NU_Select is used to timeout.                           */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      Glen Johnson,     Accelerated Technology, Inc.                   */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      TFTPC_Get                                                        */
/*      TFTPC_Put                                                        */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      NU_FD_Init                          Initialize a bit field.      */
/*      NU_FD_Set                           Set a bit in a bit field.    */
/*      NU_Select                           Check for data on a UDP sock.*/
/*      NU_Recv_From                        Recv data from a udp socket. */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      tftp_con                            The pointer to TFTP Control  */
/*                                          Block                        */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      The number of bytes received when successful.                    */
/*      NU_NO_DATA  when NU_Select fails to find a data ready socket.    */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      G. Johnson      11-20-1995      Created initial version 1.0      */
/*      D. Sharer       04-13-1998      Modified For Net 4.0             */
/*                                                                       */
/*************************************************************************/
int16 TFTPC_Recv(TFTP_CB *tftp_con)
{
    FD_SET    readfs, writefs, exceptfs;
    int16     bytes_received, status,clilen;

    /* Do a select on this socket.  In the case that the foreign port fails to
       respond we don't want to suspend on receive forever. */
    NU_FD_Init(&readfs);
    NU_FD_Set(tftp_con->socket_number, &readfs);
    if((status = NU_Select(NPORTS, &readfs, &writefs, &exceptfs, TFTP_TIMEOUT))
                != NU_SUCCESS)
        return(status);

    /*  We must have received something.  Go get the server's response.  */
    bytes_received = NU_Recv_From(tftp_con->socket_number,
                                  (char *)&tftp_con->recv_buf,
                                  TFTP_Packet_Size, 0,
                                  &tftp_con->server_addr,
                                  &clilen);
    return(bytes_received);
} /* end TFTPC_Recv */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      TFTPC_Process_Data                                               */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function is responsible processing a data packet whenever   */
/*      a read request is in progress.                                   */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      Glen Johnson,     Accelerated Technology, Inc.                   */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      TFTPC_Get                                                        */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      TFTPC_Ack                           Acknowledge a data packet.   */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      tftp_con                            The pointer to TFTP Control  */
/*                                          Block                        */
/*      bytes_received                      Number of bytes in the packet*/
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_SUCCESS whenever the expected data was received, -1 otherwise.*/
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      G. Johnson      11-20-1995      Created initial version 1.0      */
/*                                                                       */
/*************************************************************************/
STATUS TFTPC_Process_Data(TFTP_CB *tftp_con, int16 bytes_received)
{
    int16 data_size;

    /* What kind of packet is this. */
    switch(intswap(tftp_con->recv_buf.opcode))
    {
        case TFTP_DATA_OPCODE:
            /* If data was received make sure that block number and TID is
               correct. */
            if((tftp_con->block_number == intswap(tftp_con->recv_buf.block_num))
                && (tftp_con->tid == tftp_con->server_addr.port))
            {
                /* Calculate the amount of data in this packet. */
                data_size = bytes_received - 4;

                /* Make sure there is enough room left in the user's buffer for
                   the received data. */
                if(tftp_con->cur_buf_size < data_size)
                    data_size = (int16)tftp_con->cur_buf_size;

                memcpy(tftp_con->buf_ptr, tftp_con->recv_buf.data,
                       data_size);

                /* If 512 bytes of data was copied then send an ACK.  We know
                   that the other side will send at least one more data packet,
                   and that all data in the current packet was accepted. */
                if(data_size == 512)
                {
                    TFTPC_Ack(tftp_con);
                }
                /* Else if less data was copied than was received then we have
                   filled the user's buffer, and can accept no more data.  Send
                   an error condition indicating that no more data can be
                   accepted.
                */
                else if(data_size < (bytes_received - 4))
                {
                    tftp_con->status = TRANSFER_COMPLETE;
                    TFTPC_Error(tftp_con, 3, "Buffer full.");
                }
                /* Else the last data packet has been received.  We are done.
                   Send the last ack. */
                else
                {
                    tftp_con->status = TRANSFER_COMPLETE;
                    TFTPC_Ack(tftp_con);
                }

                /* Update the amount of space left in the user's buffer and the
                   pointer into the user's buffer. */
                tftp_con->cur_buf_size -= data_size;
                tftp_con->buf_ptr += data_size;

                /* Increment the block number. */
                tftp_con->block_number++;
            }
            else
                return(TFTP_CON_FAILURE);
            break;

        case TFTP_ERROR_OPCODE:
            if (intswap(tftp_con->recv_buf.block_num) <= 7)
                return (tftp_errors[intswap(tftp_con->recv_buf.block_num)]);
            else
                return(TFTP_CON_FAILURE);

        case TFTP_RRQ_OPCODE:
        case TFTP_WRQ_OPCODE:
        case TFTP_ACK_OPCODE:
        default:
            return (TFTP_CON_FAILURE);
    }
    return (NU_SUCCESS);
}  /* TFTPC_Process_Data */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      TFTPC_Ack                                                        */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function is responsible for sending an acknowledgement of   */
/*      a TFTP data packet.                                              */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      Glen Johnson,     Accelerated Technology, Inc.                   */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      TFTPC_Process_Data                                               */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      NU_Send_To                          Send an acknowledgement.     */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      tftp_con                            The pointer to TFTP Control  */
/*                                          Block                        */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      The Number of bytes sent on success.                             */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      G. Johnson      11-20-1995      Created initial version 1.0      */
/*                                                                       */
/*************************************************************************/
STATUS TFTPC_Ack(TFTP_CB *tftp_con)
{
    /* Setup the ACK packet. */
    tftp_con->send_buf.opcode = intswap(TFTP_ACK_OPCODE);
    tftp_con->send_buf.block_num = intswap(tftp_con->block_number);

    /* Send the ACK packet. */
    return (NU_Send_To(tftp_con->socket_number,
                            (char *)&tftp_con->send_buf, 4, 0,
                            &tftp_con->server_addr, 0));
} /* end TFTPC_Ack */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      TFTPC_Write_Request                                              */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function is responsible for sending a TFTP write request to */
/*      a TFTP server.                                                   */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      Glen Johnson,     Accelerated Technology, Inc.                   */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      TFTPC_Put                                                        */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      NU_Socket                           Create a UDP socket          */
/*      NU_Send_To                          Send a tftp write request    */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      remote_ip                           The server's ip address.     */
/*      remote_fname                        The file to read.            */
/*      tftp_con                            The pointer to TFTP Control  */
/*                                          Block                        */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_SUCCESS                The request was successfully sent.     */
/*      NU_NO_MEMORY              The socket descriptor was not allocated*/
/*      NU_NO_SOCKET_SPACE        All sockets descriptors are used.      */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      G. Johnson      11-20-1995      Created initial version 1.0      */
/*                                                                       */
/*************************************************************************/
STATUS TFTPC_Write_Request(char *remote_ip, char *remote_fname,
                           TFTP_CB *tftp_con)
{
    int16         bytes_sent, send_size;
    TFTP_REQ_PKT  *pkt;

    /* Create a socket. */
    if ((tftp_con->socket_number =
            NU_Socket(NU_FAMILY_IP, NU_TYPE_DGRAM, 0)) < 0)
    {
        return(tftp_con->socket_number);
    }

    /* fill in a structure with the server address */
    tftp_con->server_addr.family    = NU_FAMILY_IP;
    tftp_con->server_addr.port      = 69;
    memcpy(tftp_con->server_addr.id.is_ip_addrs, remote_ip, 4);
    tftp_con->server_addr.name = "tftp";

    /* Initialize block number and status. */
    tftp_con->block_number = 0;
    tftp_con->status = TRANSFERRING_FILE;

    pkt = (TFTP_REQ_PKT *)&tftp_con->send_buf;

    pkt->opcode = intswap(TFTP_WRQ_OPCODE);
    send_size = sprintf(pkt->data, "%s%c%s%c", remote_fname, 0, "octet", 0);

    /* Send the write request. */
    bytes_sent = NU_Send_To(tftp_con->socket_number,
                            (char *)pkt, (uint16)(send_size+2),
                            0, &tftp_con->server_addr, 0);

    return (bytes_sent);
} /* end TFTPC_Write_Request */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      TFTPC_Process_Ack                                                */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function is responsible processing an ack packet whenever   */
/*      a write request is in progress.                                  */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      Glen Johnson,     Accelerated Technology, Inc.                   */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      TFTPC_Put                                                        */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      tftp_con                            The pointer to TFTP Control  */
/*                                          Block                        */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      NU_SUCCESS whenever the expected data was received, -1 otherwise.*/
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      G. Johnson      11-20-1995      Created initial version 1.0      */
/*                                                                       */
/*************************************************************************/
STATUS TFTPC_Process_Ack(TFTP_CB *tftp_con)
{
    /* What kind of packet is this. */
    switch(intswap(tftp_con->recv_buf.opcode))
    {
        case TFTP_ACK_OPCODE:
            /* Make sure the block number and TID are correct. */
            if((tftp_con->block_number == intswap(tftp_con->recv_buf.block_num))
                && (tftp_con->tid == tftp_con->server_addr.port))
            {
                tftp_con->block_number++;
            }
            else
                return(TFTP_CON_FAILURE);
            break;

        case TFTP_ERROR_OPCODE:
            if (intswap(tftp_con->recv_buf.block_num) <= 7)
                return (tftp_errors[intswap(tftp_con->recv_buf.block_num)]);
            else
                return(TFTP_CON_FAILURE);

        case TFTP_RRQ_OPCODE:
        case TFTP_WRQ_OPCODE:
        case TFTP_DATA_OPCODE:
        default:
            return (TFTP_CON_FAILURE);
    }
    return (NU_SUCCESS);
}  /* TFTPC_Process_Ack */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      TFTPC_Send_Data                                                  */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function is responsible for sending an acknowledgement of   */
/*      a TFTP data packet.  The data is copied from the user's buffer   */
/*      into the TFTP CB send buffer.  This function also updates the    */
/*      pointer into the user's buffer, and the number of bytes left     */
/*      to send in the buffer.                                           */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      Glen Johnson,     Accelerated Technology, Inc.                   */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      TFTPC_Put                                                        */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      NU_Send_To                          Send an acknowledgement.     */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      tftp_con                            The pointer to TFTP Control  */
/*                                          Block                        */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      The Number of bytes sent on success.                             */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      G. Johnson      11-20-1995      Created initial version 1.0      */
/*                                                                       */
/*************************************************************************/
STATUS TFTPC_Send_Data(TFTP_CB *tftp_con)
{
    int16 num_bytes;

    /* Fill in the opcode and block number. */
    tftp_con->send_buf.opcode = intswap(TFTP_DATA_OPCODE);
    tftp_con->send_buf.block_num = intswap(tftp_con->block_number);

    /* Determine the number of bytes that will be sent in this packet. */
    if(tftp_con->cur_buf_size >= 512)
        num_bytes = 512;
    else
        num_bytes = (int16)tftp_con->cur_buf_size;

    /* Copy the data from the user's buffer into the send buffer. */
    memcpy(tftp_con->send_buf.data, tftp_con->buf_ptr, num_bytes);

    /* Update the number of bytes left to send and the pointer into the user's
       buffer. */
    tftp_con->cur_buf_size -= num_bytes;
    tftp_con->buf_ptr += num_bytes;

    /* If this is the last packet update the status. */
    if (num_bytes < 512)
        tftp_con->status = TRANSFER_COMPLETE;

    /* Send the data. */
    return (NU_Send_To(tftp_con->socket_number,
                            (char *)&tftp_con->send_buf, (uint16)(num_bytes + 4), 0,
                            &tftp_con->server_addr, 0));
} /* end TFTPC_Send_Data */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      TFTPC_Retransmit                                                 */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function will retransmit the last packet sent.              */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      Glen Johnson,     Accelerated Technology, Inc.                   */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      TFTPC_Put                                                        */
/*      TFTPC_Get                                                        */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      NU_Send_To                          Send a UDP datagram.         */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      tftp_con                            The pointer to TFTP Control  */
/*                                          Block                        */
/*      nbytes                              The number of bytes to       */
/*                                          retransmit.                  */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      The Number of bytes sent on success.                             */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      G. Johnson      11-26-1995      Created initial version 1.0      */
/*                                                                       */
/*************************************************************************/
STATUS TFTPC_Retransmit(TFTP_CB *tftp_con, int16 nbytes)
{
    /* Retransmit the last packet. */
    return(NU_Send_To(tftp_con->socket_number, (char *)&tftp_con->send_buf,
                      nbytes, 0, &tftp_con->server_addr, 0));
}/* TFTPC_Retransmit */

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      TFTPC_Error                                                      */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function will send an error packet.                         */
/*                                                                       */
/* AUTHOR                                                                */
/*                                                                       */
/*      Glen Johnson,     Accelerated Technology, Inc.                   */
/*                                                                       */
/* CALLED BY                                                             */
/*                                                                       */
/*      TFTPC_Process_Data                                               */
/*                                                                       */
/* CALLS                                                                 */
/*                                                                       */
/*      NU_Send_To                          Send a UDP datagram.         */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      tftp_con                            The pointer to TFTP Control  */
/*                                          Block                        */
/*      error_code                          The TFTP error code.         */
/*      err_string                          The error message to send.   */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      The Number of bytes sent on success.                             */
/*                                                                       */
/* HISTORY                                                               */
/*                                                                       */
/*         NAME            DATE                    REMARKS               */
/*                                                                       */
/*      G. Johnson      11-26-1995      Created initial version 1.0      */
/*                                                                       */
/*************************************************************************/
STATUS TFTPC_Error(TFTP_CB *tftp_con, int16 error_code, char *err_string)
{
    int16         bytes_sent, send_size;

    /* Fill in the opcode and block number. */
    tftp_con->send_buf.opcode = intswap(TFTP_ERROR_OPCODE);
    tftp_con->send_buf.block_num = intswap(error_code);

    send_size = sprintf(tftp_con->send_buf.data, "%s%c", err_string, 0);

    /* Send the datagram. */
    bytes_sent = NU_Send_To(tftp_con->socket_number,
                            (char *)&tftp_con->send_buf, (uint16)(send_size+4),
                            0, &tftp_con->server_addr, 0);

    return (bytes_sent);
} /* end TFTPC_Error */

