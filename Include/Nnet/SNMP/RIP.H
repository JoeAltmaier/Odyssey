/*@*********************************************************************** 
 |                                                                         
 |             Copyright (c) 1995-1997 XACT Incporated                     
 |                                                                         
 | PROPRIETARY RIGHTS of XACT Incorporated are involved in the subject     
 | matter of this material.  All manufacturing, reproduction, use, and     
 | sales rights pertaining to this subject matter are governed by the      
 | license agreement.  The recipient of this software implicitly accepts   
 | the terms of the license.                                               
 |                                                                         
 |                                                                         
 | FILE NAME   :                                        
 | VERSION     :      
 | COMPONENT   :
 | DESCRIPTION :                                                           
 | AUTHOR      :                                                            
 *************************************************************************/
#ifndef _XRIP_H
#define _XRIP_H
/*
 * Receive Frame Information template
 * If the ri_stat field is non-zero, there is viable info there;
 */
typedef struct rip_s {
	u16				rip_used;		/* This rip is being used if not zero 	 */
    u16             rip_stat;    	/* Status as reported in the descriptor  */
    u16             rip_size;    	/* Length of the packet                  */
    u16             rip_seq;     	/* Frame sequence number                 */
	u16				rip_unit;		/* the unit this rip came from			 */
    u16             rip_Status;  	/* For RMON Capture group entry          */
    u16             rip_Len;     	/* For RMON Capture group entry          */
    ul32            rip_ID;      	/* For RMON Capture group entry          */
    ul32            rip_Time;    	/* For RMON Capture group entry          */
    u8             *rip_pktp;  		/* pointer to this packet for this RIP.  */
} rip_t;
#endif
