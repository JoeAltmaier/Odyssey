/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// (c) Copyright 1999 ConvergeNet Technologies, Inc.
//     All Rights Reserved.
//
// File: CmbSendEventDefs.h
// 
// Description:
//  This file contains common definitions shared between the
//  sender and receiver code for event log entry forwarding.
//
// 
// $Log: $
// 
/*************************************************************************/


#ifndef _CmbSendEventDefs_h_
#define _CmbSendEventDefs_h_

#ifndef Simple_H
# include  "Simple.h"
#endif


//  each of these structures is placed at the start of the payload
//  area of a CMB k_eCmbCmdSendLogEntry command packet.

//  structure of standard meta-header used with each
//  k_eCmbCmdSendLogEntry command packet, except first-fragments:
struct CmbSendEvtHdr {
   U8    bEventSeqNum;        // common seqnum for all frags in this event
   U8    bFragSeqNum;         // seqnum of this frag within event
};

//  structure of augmented meta-header used with first-frags only:
//  (first-frags are distinguished by bFragSeqNum == 0)
struct CmbSendEvtHdrFirst {
   CmbSendEvtHdr  std;
   U8             cFragsTotal;   // how many frags used to send this event
                                 //  [frag seqnums of 0..(cFragsTotal - 1)]
};


//  for use with CmbPacket parameter indexing, here are some
//  simple byte offsets for the above struct entries:
#define  offCmbSendEvtHdrEventSeqNum   (0)
#define  offCmbSendEvtHdrFragSeqNum    (1)
#define  offCmbSendEvtHdrFragCount     (2)


//  and here's a clean way of specifying the "first fragment" seqnum
#define  CmbSendEvtFirstFragSeqNum     (0)


#endif  // #ifndef _CmbSendEventDefs_h_

