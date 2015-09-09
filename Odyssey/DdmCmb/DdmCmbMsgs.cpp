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
// File: DdmCmbMsgs.cpp
//
// Description:
//    CMB DDM module.  Contains CMB DDM interface message class member
//    routines which are too big to be inlined.
//
// $Log: /Gemini/Odyssey/DdmCmb/DdmCmbMsgs.cpp $
// 
// 1     8/12/99 7:14p Ewedel
// Initial revision.
// 
/*************************************************************************/

#include  "DdmCMBMsgs.h"


#include  <assert.h>




//
//  MsgCmbSendSmallMsg::MsgCmbSendSmallMsg (eTargetRequest, eTargetIop,
//                                          pvTargetPayload, cbTargetPayload)
//
//  Description:
//    Our only constructor.  Called when somebody is building a
//    "small message" send request for the CMB DDM.
//
//    The CMB DDM sends "small messages" via the CMB bus to the specified
//    IOP, where they are dispatched via the given target request code.
//    MsgCmbSendSmallMsg is used to carry a "small message" request to
//    the CMB DDM.
//
//    It is expected that the CMB "small message" capability will be used
//    *only* when normal PCI-based transport facilities are broken or
//    not yet established.
//
//    ** These messages are relatively slow to deliver.  Also, the payload
//       must not exceed 20 bytes (!).
//
//  Inputs:
//    eTargetRequest - Request code to use when sending small message
//             on destination IOP.  This must be a REQUEST_CODE value from
//             include\oos\RequestCodes.h.
//    eTargetIop - Where to send the message.  The CMB DDM will deliver the
//             message to this IOP, and then Send() it as a normal CHAOS
//             message using the given eTargetRequest code.
//    pvTargetPayload - Data to supply as payload of final CHAOS message
//             sent on destination IOP.
//    cbTargetPayload - One-based count of bytes in *pvTargetPayload.
//             This value must be less than or equal to 20 bytes.
//
//  Outputs:
//    none
//


MsgCmbSendSmallMsg::MsgCmbSendSmallMsg(REQUESTCODE eTargetRequest,
                                       TySlot eTargetIop,
                                       void *pvTargetPayload,
                                       U32 cbTargetPayload)
                     : Message(MyRequestCode())
{


   assert (cbTargetPayload <= 20);

   //  build up our tracking object for the caller's message data
   CPayload Payload (eTargetRequest, eTargetIop);

   if (cbTargetPayload > sizeof (Payload.m_abTargetPayload))
      {
      //  trim it back, we're a constructor and can't just fail
      cbTargetPayload = sizeof (Payload.m_abTargetPayload);
      }

   Payload.m_cbTargetPayload = cbTargetPayload;
   if (cbTargetPayload > 0)
      {
      assert (pvTargetPayload != NULL);

      if (pvTargetPayload != NULL)
         {
         memcpy (Payload.m_abTargetPayload, pvTargetPayload, cbTargetPayload);
         }
      }

   //  got payload built up, now add it to our CMB-bound message body
   AddPayload (&Payload, sizeof (Payload));

   return;

};  /* end of MsgCmbSendSmallMsg::MsgCmbSendSmallMsg */


