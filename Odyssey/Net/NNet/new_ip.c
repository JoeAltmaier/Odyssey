/******************************************************************************/
/*                                                                            */
/*    CopyrIght (c)  1993 - 1996 Accelerated Technology, Inc.                 */
/*                                                                            */
/* PROPRIETARY RIGHTS of Accelerated Technology are involved in the           */
/* subject matter of this material.  All manufacturing, reproduction,         */
/* use, and sales rights pertaining to this subject matter are governed       */
/* by the license agreement.  The recipient of this software implicitly       */
/* accepts the terms of the license.                                          */
/*                                                                            */
/******************************************************************************/
/******************************************************************************/
/*                                                                            */
/* FILENAME                                                 VERSION           */
/*                                                                            */
/*  NEW_IP.C                                                  4.0             */
/*                                                                            */
/* DESCRIPTION                                                                */
/*                                                                            */
/*  This file contains the source for utility routines used by many           */
/*  of the TCP/IP files.  Formerly, they were in assembly language.           */
/*  They have been converted to C files for portability.                      */
/*                                                                            */
/* AUTHOR                                                                     */
/*                                                                            */
/*  Barbara G. Harwell, Accelerated Technology Inc.                           */
/*                                                                            */
/* DATA STRUCTURES                                                            */
/*                                                                            */
/*  No data structures defined in this file.                                  */
/*                                                                            */
/* FUNCTIONS                                                                  */
/*                                                                            */
/*     ipcheck                  Checksums an IP header                        */
/*     tcpcheck                 Checksums a TCP header                        */
/*     longswap                 Swaps 4 bytes of a long number                */
/*     intswap                  Swaps 2 bytes of a number                     */
/*     comparen                 Compares 2 strings for equality               */
/*     IP_Check_Buffer_Chain    Checksums an IP Buffer Chain                  */
/*     init_time                Initializes the Network timer.                */ 
/*                                                                            */
/* DEPENDENCIES                                                               */
/*                                                                            */
/*      NUCLEUS.H                                                             */
/*      EXTERNS.H                                                             */
/*                                                                            */
/* HISTORY                                                                    */
/*                                                                            */
/*    NAME               DATE        REMARKS                                  */
/*                                                                            */
/*  Barbara G. Harwell   10/06/92   Initial version.                          */
/*                                                                            */
/******************************************************************************/

#ifdef PLUS
  #include "nucleus.h"
#else
  #include "nu_defs.h"    /* added during ATI mods - 10/20/92, bgh */
  #include "nu_extr.h"
#endif

#ifdef MSC
  #include <time.h>
#endif

#include "externs.h"

#if !(IPCHECK_ASM)
/************************************************************************/
/*                                                                      */
/*  FUNCTION                                "ipcheck"                   */
/*                                                                      */
/*                                                                      */
/*  DESCRIPTION                                                         */
/*                                                                      */
/*      This function checksums an IP header.                           */
/*                                                                      */
/*  AUTHOR                                                              */
/*                                                                      */
/*      B. Harwell,  Accelerated Technology                             */
/*                                                                      */
/*  HISTORY                                                             */
/*                                                                      */
/*      created - 12/22/92                                              */
/*                                                                      */
/*  CALLED FROM                                                         */
/*                                                                      */
/*      IP.C                                                            */
/*      TCP.C                                                           */
/*      UDP.C                                                           */
/*                                                                      */
/*  ROUTINES CALLED                                                     */
/*                                                                      */
/*      N/A                                                             */
/*                                                                      */
/*  INPUTS                                                              */
/*                                                                      */
/*      header                   Pointer to structure to be checksummed */
/*      length                   Length of header structure             */
/*                                                                      */
/*  OUTPUTS                                                             */
/*                                                                      */
/*      sum                      Checksum value of structure            */
/*                                                                      */
/************************************************************************/
uint16 ipcheck (uint16 *header, uint16 length)
{
    uint32 sum;

    for (sum = 0; length > 0; length--)
    {
         sum += *header++;
         /* check for a carry over 16 bits */
    }

    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    return ((int16)~sum);
}
#endif /* IPCHECK_ASM */

/* This function currently only used by ICMP */



/************************************************************************/
/*                                                                      */
/*  FUNCTION                                "IP_Check_Buffer_Chain"     */
/*                                                                      */
/*                                                                      */
/*  DESCRIPTION                                                         */
/*                                                                      */
/*      This function checksums an IP Buffer Chain.                     */
/*                                                                      */
/*  AUTHOR                                                              */
/*                                                                      */
/*      Accelerated Technology                                          */
/*                                                                      */
/*                                                                      */
/*  CALLED FROM                                                         */
/*                                                                      */
/*      IP.C                                                            */
/*      TCP.C                                                           */
/*      UDP.C                                                           */
/*                                                                      */
/*  ROUTINES CALLED                                                     */
/*                                                                      */
/*                                                                      */
/*                                                                      */
/************************************************************************/

uint16 IP_Check_Buffer_Chain (NET_BUFFER *buf_ptr)
{
    uint32      sum;
    uint16      *current_byte, length;
    NET_BUFFER  *temp_buf_ptr;
    uint32      data_len, total_length;
    uint8       odd_length;

    /* Init */
    sum             = 0;
    total_length    = 0;
    odd_length      = NU_FALSE;
    length          = (uint16) buf_ptr->mem_total_data_len;
    
    /* Check for an odd length. */
    if (length % 2)
    {
        /* For an odd length a byte equal to zero is padded to the end 
           of the buffer for checksum computation. This has to be done
           since the checksum is done on 16 bits at a time. Since the 
           buffers are chained there is no easy way to get to the end
           of the buffer. Therefore we will set a flag so that this special
           case can be handled when the loop below reaches the end of
           the buffer. */
        odd_length = NU_TRUE;

    }

    /* Divide the length by two. */
    length = length >> 1;

    /* Get a pointer to the buffer. */
    temp_buf_ptr = buf_ptr;

    /* Loop through the chain computing the checksum on each byte
       of data. */
    while ((temp_buf_ptr) && (total_length < length))
    {
        /* Reset the data pointer. */
        current_byte    = (uint16 *)temp_buf_ptr->data_ptr;
        
        for (data_len = (temp_buf_ptr->data_len >> 1); data_len && 
            (total_length < length); total_length++, data_len--)
            sum += *current_byte++;

        /* Point to the next buffer. */
        temp_buf_ptr = temp_buf_ptr->next_buffer;
    }

    /* Do we need to handle padding the zero and finishing the checksum
       computation? */
    if (odd_length)
    {
        /* If the length left is 1 then we need to update the current byte
           pointer. The loop above did not because it stops before the 
           last byte. */

        /* Make sure temp ptr is valid */
        if (temp_buf_ptr)
            /* See if the length is 1 */
            if (temp_buf_ptr->data_len == 1)
                /* Update the current byte ptr to the data in the next buffer. */
                current_byte = (uint16 *)temp_buf_ptr->data_ptr;
        
        /* Pad the zero */
        ((uchar *)current_byte)[1] = 0;

        /* Do the checksum for the last 16 bits. */
        sum += *current_byte;
    }
    
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    return ((int16)~sum);
}

#if !(TCPCHECK_ASM)
/************************************************************************/
/*                                                                      */
/*  FUNCTION                                "tcpcheck"                  */
/*                                                                      */
/*                                                                      */
/*  DESCRIPTION                                                         */
/*                                                                      */
/*      This function checksums a TCP header.                           */
/*                                                                      */
/*  AUTHOR                                                              */
/*                                                                      */
/*      B. Harwell,  Accelerated Technology                             */
/*                                                                      */
/*  HISTORY                                                             */
/*                                                                      */
/*      created - 12/22/92                                              */
/*                                                                      */
/*  CALLED FROM                                                         */
/*                                                                      */
/*      IP.C                                                            */
/*      TCP.C                                                           */
/*      UDP.C                                                           */
/*                                                                      */
/*  ROUTINES CALLED                                                     */
/*                                                                      */
/*      N/A                                                             */
/*                                                                      */
/*  INPUTS                                                              */
/*                                                                      */
/*      pseudoheader             Pointer to header to be checksummed    */
/*      tcpheader                Pointer to structure to be checksummed */
/*      length                   Length of tcpheader structure          */
/*                                                                      */
/*  OUTPUTS                                                             */
/*                                                                      */
/*      sum                      Checksum value of structure            */
/*                                                                      */
/************************************************************************/
uint16 tcpcheck (uint16 *pseudoheader, NET_BUFFER *buf_ptr)
{
    register uint32 sum;
    register uint16 *pshdr = pseudoheader;
    uint16          *current_byte, length;
    NET_BUFFER      *temp_buf_ptr;
    uint32          data_len, total_length;
    uint8           odd_length;

    /* Init */
    sum             = 0;
    total_length    = 0;
    odd_length      = NU_FALSE;
    length          = (uint16) buf_ptr->mem_total_data_len;

    /*  This used to be a loop.  The loop was removed to save a few
        cycles.  The header length is always 6 16-bit words.  */
    sum += *pshdr++;
    sum += *pshdr++;
    sum += *pshdr++;
    sum += *pshdr++;
    sum += *pshdr++;
    sum += *pshdr++;


    /* Check for an odd length. */
    if (length % 2)
    {
        /* For an odd length a byte equal to zero is padded to the end 
           of the buffer for checksum computation. This has to be done
           since the checksum is done on 16 bits at a time. Since the 
           buffers are chained there is no easy way to get to the end
           of the buffer. Therefore we will set a flag so that this special
           case can be handled when the loop below reaches the end of
           the buffer. */
        odd_length = NU_TRUE;

    }

    /* Divide the length by two. */
    length = length >> 1;

    /* Get a pointer to the buffer. */
    temp_buf_ptr = buf_ptr;

    /* Loop through the chain computing the checksum on each byte
       of data. */
    while ((temp_buf_ptr) && (total_length < length))
    {
        /* Reset the data pointer. */
        current_byte    = (uint16 *)temp_buf_ptr->data_ptr;
        
        for (data_len = (temp_buf_ptr->data_len >> 1); data_len && 
            (total_length < length); total_length++, data_len--)
            sum += *current_byte++;

        /* Point to the next buffer. */
        temp_buf_ptr = temp_buf_ptr->next_buffer;
    }

    /* Do we need to handle padding the zero and finishing the checksum
       computation? */
    if (odd_length)
    {
        /* If the length left is 1 then we need to update the current byte
           pointer. The loop above did not because it stops before the 
           last byte. */

        /* Make sure temp ptr is valid */
        if (temp_buf_ptr)
            /* See if the length is 1 */
            if (temp_buf_ptr->data_len == 1)
                /* Update the current byte ptr to the data in the next buffer. */
                current_byte = (uint16 *)temp_buf_ptr->data_ptr;
        
        /* Pad the zero */
        ((uchar *)current_byte)[1] = 0;

        /* Do the checksum for the last 16 bits. */
        sum += *current_byte;
    }
    

    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    return ((int16)~sum);
}
#endif /* !(TCPCHECK_ASM) */

#if !(LONGSWAP_ASM)
/************************************************************************/
/*                                                                      */
/*  FUNCTION                                "longswap"                  */
/*                                                                      */
/*                                                                      */
/*  DESCRIPTION                                                         */
/*                                                                      */
/*      This function swaps 4 bytes of a long number.                   */
/*                                                                      */
/*  AUTHOR                                                              */
/*                                                                      */
/*      B. Harwell,  Accelerated Technology                             */
/*                                                                      */
/*  HISTORY                                                             */
/*                                                                      */
/*      created - 12/21/92                                              */
/*                                                                      */
/*  CALLED FROM                                                         */
/*                                                                      */
/*      PROTINIT.C                                                      */
/*      TCP.C                                                           */
/*      SOCKETS.C                                                       */
/*      TOOLS.C                                                         */
/*                                                                      */
/*  ROUTINES CALLED                                                     */
/*                                                                      */
/*      N/A                                                             */
/*                                                                      */
/*  INPUTS                                                              */
/*                                                                      */
/*      number                   long number to be byte swapped         */
/*                                                                      */
/*  OUTPUTS                                                             */
/*                                                                      */
/*      return_value             the input value after being byteswapped*/
/*                                                                      */
/************************************************************************/
uint32 longswap(uint32 number)
{
#ifdef SWAPPING
    uint8 *new_num, *org;
    uint32 return_value = 0;

    org     = (uint8 *)&number;
    new_num = (uint8 *)&return_value;

    /* swap the bytes one at time */
    new_num[0] = org[3];
    new_num[1] = org[2];
    new_num[2] = org[1];
    new_num[3] = org[0];

    return(return_value);
#else
    return(number);
#endif  /* SWAPPING */
}
#endif /* !(LONGSWAP_ASM) */

#if !(INTSWAP_ASM)
/************************************************************************/
/*                                                                      */
/*  FUNCTION                                "intswap"                   */
/*                                                                      */
/*                                                                      */
/*  DESCRIPTION                                                         */
/*                                                                      */
/*      This function swaps 2 bytes of a number.                        */
/*                                                                      */
/*  AUTHOR                                                              */
/*                                                                      */
/*      B. Harwell,  Accelerated Technology                             */
/*                                                                      */
/*  HISTORY                                                             */
/*                                                                      */
/*      created - 12/21/92                                              */
/*                                                                      */
/*  CALLED FROM                                                         */
/*                                                                      */
/*      PROTINIT.C                                                      */
/*      TCP.C                                                           */
/*      SOCKETS.C                                                       */
/*      NET.C                                                           */
/*      UDP.C                                                           */
/*      IP.C                                                            */
/*      USER.C                                                          */
/*      ARP.C                                                           */
/*                                                                      */
/*  ROUTINES CALLED                                                     */
/*                                                                      */
/*      N/A                                                             */
/*                                                                      */
/*  INPUTS                                                              */
/*                                                                      */
/*      number                   number to be byte swapped              */
/*                                                                      */
/*  OUTPUTS                                                             */
/*                                                                      */
/*      return_value             the input value after being byteswapped*/
/*                                                                      */
/************************************************************************/
uint16 intswap (uint16 number)
{
#ifdef SWAPPING
    uint8 *new_num, *org;
    uint16 return_value = 0;

    org     = (uint8 *)&number;
    new_num = (uint8 *)&return_value;

    /* swap the bytes in the int16 */
    new_num[0] = org[1];
    new_num[1] = org[0];

    return(return_value);
#else
    return (number);
#endif /* SWAPPING */
}
#endif /* !(INTSWAP_ASM) */

#if !(COMPAREN_ASM)
/************************************************************************/
/*                                                                      */
/*  FUNCTION                                "comparen"                  */
/*                                                                      */
/*                                                                      */
/*  DESCRIPTION                                                         */
/*                                                                      */
/*      This function compares 2 strings for equality.                  */
/*                                                                      */
/*  AUTHOR                                                              */
/*                                                                      */
/*      B. Harwell,  Accelerated Technology                             */
/*                                                                      */
/*  HISTORY                                                             */
/*                                                                      */
/*      created - 12/21/92                                              */
/*                                                                      */
/*  CALLED FROM                                                         */
/*                                                                      */
/*      IP.C                                                            */
/*      ARP.C                                                           */
/*      EQUEUE.C                                                        */
/*      CONFILE.C                                                       */
/*      UDP.C                                                           */
/*      BOOTP.C                                                         */
/*      PROTINIT.C                                                      */
/*      TOOLS.C                                                         */
/*      UTIL.C                                                          */
/*                                                                      */
/*  ROUTINES CALLED                                                     */
/*                                                                      */
/*      N/A                                                             */
/*                                                                      */
/*  INPUTS                                                              */
/*                                                                      */
/*      s1                       Pointer to the first input string      */
/*      s2                       Pointer to the second input string     */
/*      len                      Length of characters to compare        */
/*                                                                      */
/*  OUTPUTS                                                             */
/*                                                                      */
/*      return_value             Equality indicator :                   */
/*                            return 0 if strings 1 and 2 are not equal */
/*                            return 1 if strings 1 and 2 are equal     */
/*                                                                      */
/************************************************************************/
int16 comparen (VOID *s1, VOID *s2, int16 len)
{
    return (!memcmp ((void *)s1, (void *)s2, (uint16)len));
}
#endif /* !(COMPAREN_ASM) */

int32 n_clicks()
{

#ifdef PLUS
    UNSIGNED    current_time;
    current_time = NU_Retrieve_Clock();
#else
    long current_time;   /* TBV - I just changed this from int to long */
    current_time = NU_Read_Time();
#endif

    return ((long)current_time);
}


/************************************************************************/
/*                                                                      */
/*  FUNCTION                                "init_time"                 */
/*                                                                      */
/*                                                                      */
/*  DESCRIPTION                                                         */
/*                                                                      */
/*      This function initializes the time.                             */
/*                                                                      */
/*  AUTHOR                                                              */
/*                                                                      */
/*      B. Harwell,  Accelerated Technology                             */
/*                                                                      */
/*  HISTORY                                                             */
/*                                                                      */
/*      created - 12/22/92                                              */
/*                                                                      */
/*  CALLED FROM                                                         */
/*                                                                      */
/*      netinit                                                         */
/*                                                                      */
/*  ROUTINES CALLED                                                     */
/*                                                                      */
/*      NU_TCP_Time                                                     */    
/*      NU_Set_Clock                                                    */
/*                                                                      */
/*                                                                      */
/************************************************************************/

void init_time()
{
#ifdef PLUS
    UNSIGNED t;
#else
    long t;
#endif

#ifdef MSC
    t = (uint32)time (NU_NULL);
#else
    t = NU_TCP_Time ((uint32 *)NU_NULL);
#endif

#ifdef PLUS
    NU_Set_Clock((UNSIGNED) (t & 0x7FFF));
#else
    NU_Set_Time((unsigned int) (t & 0x7FFF));
#endif
} /* end init_time */

