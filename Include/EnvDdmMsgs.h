/*************************************************************************/
// Copyright (C) ConvergeNet Technologies, 1998,99 
//
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// EnvDdmMsgs.h -- Define EnvDdm message structure
//
//  This file contains the ENV DDM-specific subclasses of Message.
//  These derived message classes are used to request particular
//  ENV DDM-supplied operations.
//
//  At present, the following derived messages classes are defined here:
//       MsgEnvKeyswitch - Report Keyswitch change state from the CMB DDM
//       MsgEnvPowerEvent - Report power event state from the CMB DDM
//
//
// $Log: /Gemini/Include/EnvDdmMsgs.h $
// 
// 1     12/13/99 1:31p Vnguyen
// Initial check-in for Environment Ddm.
// 
/*************************************************************************/

#ifndef _EnvDdmMsgs_h
#define _EnvDdmMsgs_h


#include "Simple.h"
#include "Message.h"
#include "RequestCodes.h"
#include "CTEvent.h"

#include "EVCStatusRecord.h"


class MsgEnvKeyswitch : public Message {
public:

	static inline REQUESTCODE  MyRequestCode (void)
      {  return (ENV_KEYSWITCH_CHANGE_STATE);  };


	MsgEnvKeyswitch(CT_EVC_KEYPOS ePosition, U32 _TargetEVC) : Message(MyRequestCode()),
										m_Payload (ePosition, _TargetEVC)
		{};

	struct Payload
	{
		CT_EVC_KEYPOS	ePosition;

		// Zero based, not on TySlot (EVC0 = 0, EVC1 = 1)
		U32				ulTargetEVC;	// EVC this message is for

		Payload (CT_EVC_KEYPOS _ePosition, U32 _TargetEVC) : ePosition (_ePosition),
										ulTargetEVC (_TargetEVC)
			{};
	};

	Payload  m_Payload;
};  /* end of class MsgEnvKeyswitch */


class MsgEnvPowerEvent : public Message {
public:

	static inline REQUESTCODE  MyRequestCode (void)
      {  return (ENV_POWER_EVNT);  };

	MsgEnvPowerEvent(S32 _CurrentFuse0, S32 _CurrentFuse1, U32 _TargetEVC) : Message(MyRequestCode()),
										m_Payload (_CurrentFuse0, _CurrentFuse1, _TargetEVC)
		{};

	struct Payload
	{
		S32		slCurrentFuse0;		// Cuurent reading across fuse 0
		S32		slCurrentFuse1;		// Cuurent reading across fuse 1

		// Zero based, not on TySlot (EVC0 = 0, EVC1 = 1)
		U32		ulTargetEVC;		// EVC this message is for

		Payload (S32 _CurrentFuse0, S32 _CurrentFuse1, U32 _TargetEVC) : 
				slCurrentFuse0 (_CurrentFuse0),
				slCurrentFuse1 (_CurrentFuse1),
				ulTargetEVC (_TargetEVC)
			{};
	};

	Payload  m_Payload;
};  /* end of class MsgEnvPowerEvent */


#endif  // #ifndef _EnvDdmMsgs_h
