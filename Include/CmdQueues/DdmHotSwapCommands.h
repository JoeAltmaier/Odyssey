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
// File: DdmHotSwapCommands.h
// 
// Description:
//    Hot Swap Master DDM control interface definitions.
//    These are the commands submitted to the Hot Swap Master
//    by other DDMs in the Odyssey system.
//
//    Commands are submitted via class CmdSender, see include\CmdSender.h
//    and friends for the particulars.
// 
// $Log: /Gemini/Include/CmdQueues/DdmHotSwapCommands.h $
// 
// 3     12/17/99 7:56p Ewedel
// Rearranged parameter order to work around strange CodeWarrior
// optimization "feature".
// 
// 2     12/14/99 8:00p Ewedel
// Updated to reflect current definition of Hot Swap "master" functions.
// Also, now takes RowId as param to select drive to operate on.
// 
// 1     10/11/99 7:49p Ewedel
// First cut.
// 
/*************************************************************************/

#ifndef _DdmHotSwapCommands_h_
#define _DdmHotSwapCommands_h_

#ifndef __Address_h
# include  "Address.h"        // for TySlot
#endif

#ifndef Simple_H
# include  "Simple.h"         // for BOOL, U32, etc.
#endif

#ifndef CTtypes_H
# include  "CtTypes.h"        // for rowID
#endif


//  common Hot Swap Master control queue name root
#define     HSW_CONTROL_QUEUE    "HswMasterControlQueue"

//  size of CMB control queue's command messages
#define     HSW_CONTROL_COMMAND_SIZE   (sizeof (CHotSwapIntf))

//  size of CMB control queue's status messages (unused)
#define     HSW_CONTROL_STATUS_SIZE    (0)



class CHotSwapIntf {
public:

   enum Cmd {
      k_eUnknown = 0,
      k_eReleaseDriveBay,     // enable a drive to be removed
      k_eRestoreDriveBay      // lock drive into bay, place it in FC loop?
   };

   //NOTE:  RID needs to come first, as it must be 8-byte aligned
   //       due to some questionable CW optimizations.
   
   //  RID of internal drive whose drive bay we operate on
   //  [this points into the DiskDescriptorTable]
   RowId   ridDiskDescr;

   //  what operation we're to perform
   Cmd     eCommand;


   //  a humble constructor, just to keep things a bit tidy
   CHotSwapIntf (Cmd _eCommand = k_eUnknown)
   {
      eCommand = _eCommand;
   };

};  /* end of class CHotSwapIntf */


#endif   // #ifndef _DdmHotSwapCommands_h_


