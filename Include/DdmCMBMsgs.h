// DdmCMBMsgs.h -- Define Ddm CMB message structure
//
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
//  This file contains the CMB DDM-specific subclasses of Message.
//  These derived message classes are used to request particular
//  CMB DDM-supplied operations.
//
//  At present, the following derived messages classes are defined here:
//       MsgCmbIopControl - IOP power control, PCI window, etc.
//       MsgCmbEvcControl - Fan speed, HBC power control, etc.
//       MsgCmbDdhControl - Drive bay locking, FC port bypass
//       MsgCmbPollAllIops - Refresh the complete IOP Status table
//       MsgCmbPollEnvironment - Read all chassis environmental params
//       MsgCmbSendSmallMsg - Send a small CHAOS message to another IOP's DDM
//       MsgCmbSetMipsState - Set the current IOP's CMB "MIPS State" value
//       
//       
//       
//
//  There is also one message used largely for internal CMB DDM purposes:
//       MsgCmbUpdateIopStatus - Update just one IOP's data in IOP Status table.
//
//
// $Log: /Gemini/Include/DdmCMBMsgs.h $
// 
// 22    1/20/00 4:09p Eric_wedel
// Added EVC Control support for SetBootConditions, and new message
// MsgCmbEvcPowerControl to enable box shutdown.  Also tidied up payload
// initialization some.
// 
// 21    12/13/99 7:27p Jlane
// [ewx]  Changed bool param in payload struct to BOOL.  It turns out that
// bool types can be interpreted as having different sizes depending on CW
// project settings.  Yikes!
// 
// 20    12/13/99 4:29p Ewedel
// Removed obsolete EVC master stuff, added new controls for SMP v54
// adjust value, and aux supply enable flags.
// 
// 19    11/19/99 6:32p Ewedel
// Updated comments.
// 
// 18    11/19/99 3:07p Agusev
// Added a conditional (upon the platform) explicit pointer cast
// 
// 17    11/16/99 7:39p Ewedel
// Oops, made iDdhBayNumberMax valid.
// 
// 16    11/16/99 6:28p Ewedel
// Tidied up DDH control command.
// 
// 15    11/11/99 7:39p Ewedel
// Added messages MsgCmbDdhControl and MsgCmbPollEnvironment.
// 
// 14    10/27/99 12:16p Ewedel
// Cleaned up definitions of IOP and EVC control messages.
// 
// 13    10/08/99 11:55a Ewedel
// Modified MsgCmbSetMipsState to use a payload constructor (thanks, TRN).
// 
// 12    10/04/99 8:03p Ewedel
// Added new message class MsgCmbSetMipsState.
// 
// 11    9/03/99 4:38p Ewedel
// Tweaked MsgCmbEvcControl so that "high" and "normal" thresholds are
// independently settable.
// 
// 10    8/24/99 8:14p Ewedel
// Removed obsolete MsgCmbUpdateEvcStatus class.
// 
// 9     8/12/99 7:11p Ewedel
// Removed MsgCmbSendSmallMsg constructor, which now lives in a .cpp file
// in the DdmCmb project.
// 
// 8     8/11/99 7:36p Ewedel
// Added new message wrapper MsgCmbSendSmallMsg, and removed obsolete
// MsgCmbReturn*Status() messages.
// 
// 7     3/12/99 10:55a Ewedel
// Changed MRC return type to REQUESTCODE to work with new RequestCode
// scheme.
// 
// 6     3/11/99 6:13p Ewedel
// Added MyRequestCode() members to each message subclass, and removed
// STATUS fields, which are redundant with Message::DetailedStatusCode.
// Also stripped tabs.
// 
// 5     3/03/99 8:00p Ewedel
// Fixed various compile errors, added modest constructors to each message
// class.  Added new message classes MsgCmbUpdateEvcStatus and
// MsgCmbUpdateIopStatus.
// 
// 4     3/03/99 6:48p Jlane
// Added #define for ReturnEVCStatus SGL index.
// 
// 3     3/03/99 1:08p Jlane
// Added more Image Management Actions as a result of BIUM design..
// 
// 2     3/02/99 5:13p Jlane
// Added messages for initial prototytpe (and beyond hopefully).
// 
// 1     3/02/99 4:06p Jlane
// Initial checkin.
//

#ifndef _DdmCMBMsgs_h
#define _DdmCMBMsgs_h


#include "Message.h"
#include "RequestCodes.h"
#include "CTEvent.h"


#ifndef _IOPStatusRecord_h
# include "IOPStatusTable.h"     // PTS table def
#endif

#ifndef Simple_H
# include  "Simple.h"            // simple type definitions
#endif

#ifndef _EVCRawParameters_h
# include  "EVCRawParameters.h"
#endif


//  ask CMB DDM to do something (see eAction) to a given IOP
class MsgCmbIopControl : public Message {
public:

   static inline REQUESTCODE  MyRequestCode (void)
      {  return (CMB_IOP_CONTROL);  };

   enum Action {
      NOP = 1,
      PowerOn,
      PowerOff,
      EnableRemoval,
      DisableRemoval,
      EnablePCI,
      DisablePCI,
      Reset,         // hands control back to IOP's MIPS boot ROM
      NMI            // strobe NMI to a board (for crash dump)
     };

   MsgCmbIopControl(TySlot eTargetSlot, Action eAction) : Message(MyRequestCode()),
                                        m_Payload (eTargetSlot, eAction)
      {}
      
   struct Payload
   {
      TySlot   eTargetSlot;   // Slot this command is for
      Action   eAction;    // Command action for slot.

      Payload (TySlot _eTargetSlot, Action _eAction) : eTargetSlot(_eTargetSlot),
                                                       eAction(_eAction)
               {};
   };
    
   Payload  m_Payload;

};  /* end of class MsgCmbIopControl */


class MsgCmbEvcControl : public Message {
public:

   static inline REQUESTCODE  MyRequestCode (void)
      {  return (CMB_EVC_CONTROL);  };

   enum Action {
      NOP = 1,          // (invalid, to force user to define an action)
      SetIntakeFanRpm,  // set target RPMs of intake fan pair
      SetExhaustFanRpm, // set target RPMs of exhaust fan pair
      SetFanUpThresh,   // set auto fan up temp threshold
      SetFanRestoreThresh, // set fan-restore temp threshold
      SetSmpV54Adjust,  // set EVCs' 54v supply trimming values
      SetAuxSupplyEnable, // set EVCs' aux supply enable flags
      SetBootConditions // set boot condition flags
      };


   MsgCmbEvcControl(Action eAction) : Message(MyRequestCode()),
                                      m_Payload (eAction)
      {};

   struct Payload
   {
      Action   eAction;

      union {
         U32      ulTargetFanSpeed;    // fan drive duty cycle (%), 100 = full speed
         U32      ulFanUpThresh;       // temp at which exhaust fans go to full-on
                                       //  (degrees C)
         U32      ulFanRestoreThresh;  // temp at which exhaust fans restore to their
                                       //  standard, configured speed (degrees C)
         U8       bSmpV54Adjust;       // value fed to EVC's DAC to trim 54 volts
         struct {
            BOOL  fChangeEnable;       // must be TRUE to alter enable state
            BOOL  fEnableState;        // TRUE == turn aux on, FALSE == off
         }        aAuxSupplyEnable[2]; // selectively controls enables of auxes
         struct {
            BOOL  fServiceOnly;        // TRUE -> keyswitch only active in
                                       //  "service" position.
         }        BootConditions;
      } u;

      Payload (Action _eAction = NOP)
         {  memset (this, 0, sizeof (*this));
            eAction = _eAction;  };
   };

   Payload  m_Payload;
   
};  /* end of class MsgCmbEvcControl */


class MsgCmbEvcPowerControl : public Message {
public:

   static inline REQUESTCODE  MyRequestCode (void)
      {  return (CMB_EVC_POWER_CONTROL);  };

   enum Action {
      NOP = 1,          // (invalid, to force user to define an action)
      SetAllPowerOff    // turn off all power to the system:
                        //  kills HBCs, DDHs, and all battery drainers
      };


   MsgCmbEvcPowerControl(Action eAction) : Message(MyRequestCode()),
                                      m_Payload (eAction)
      {};

   struct Payload
   {
      Action   eAction;

      Payload (Action _eAction = NOP)
         {  memset (this, 0, sizeof (*this));
            eAction = _eAction;  };
   };

   Payload  m_Payload;
   
};  /* end of class MsgCmbEvcPowerControl */


class MsgCmbDdhControl : public Message {
public:

   static inline REQUESTCODE  MyRequestCode (void)
      {  return (CMB_DDH_CONTROL);  };

   enum Action {
      NOP = 1,          // (invalid, to force user to define an action)
      SetBayLockState,  // lock or unlock a drive bay (u.fLockBay)
      SetDriveBypass    // enable or disable bypass of a drive (u.aBypassFlags[])
      };
      
   enum {   
      //  maximum permissible value of iBay (0-based limit)
      iDdhBayNumberMax  =  4
   };


   MsgCmbDdhControl(Action eAction) : Message(MyRequestCode()),
                                      m_Payload (eAction)
      {};

   struct Payload
   {
      Action   eAction;

      TySlot   eDdh;             // DDH whose bay we're to operate on
                                 //  (CMB_DDH0 .. CMB_DDH3)
      U32      iBay;             // DDH-relative bay number to work on
                                 //  (0 .. iDdhBayNumberMax)

      union {
         BOOL     fLockBay;         // TRUE to lock drive, FALSE to unlock it
         struct {
            BOOL  fChangeThisPort;  // TRUE to change this port's setting,
                                    //  FALSE to leave it as-is
            BOOL  fBypassPort;      // TRUE to remove bay's port from this FC,
                                    //  FALSE to connect it to FC loop.
         } aBypassFlags[2];         // two sets, one for each FC path in DDH
      } u;

      Payload (Action _eAction = NOP) : eAction (_eAction)
         {};
   };

   Payload  m_Payload;


};  /* end of class MsgCmbDdhControl */


class MsgCmbPollAllIops : public Message {
public:

   static inline REQUESTCODE  MyRequestCode (void)
      {  return (CMB_POLL_ALL_IOPS);  };

   MsgCmbPollAllIops() : Message(MyRequestCode())
      {  };
   
};  /* end of class MsgCmbPollAllIops */


//  request that the CMB DDM do a single sweep of all environmental parameters
class MsgCmbPollEnvironment : public Message {
public:

   static inline REQUESTCODE  MyRequestCode (void)
      {  return (CMB_POLL_ENVIRONMENT);  };

   class CPayload;      // forward ref for our constructor

   MsgCmbPollEnvironment (const CPayload& EnvPollData) :
                                    Message(MyRequestCode())
               {  SetPayload (EnvPollData);  };

   //  note:  CPayload is a mite too big to actually place as a CHAOS
   //         message payload, so we attach it as an SGL instead.
   //         See GetPayload() and SetPayload() below.
   class CPayload
   {
   public:

      class CMB_SLOT_RECORD
      {
      public:
     
         CMB_SLOT_RECORD (void)
         {
            fPresent = FALSE;
            Temperature = 0;
            TemperatureHiThreshold = TemperatureNormThreshold = 0;
         };
     
         BOOL  fPresent;         // True: if an IOP is installed
                                 // False:  ignore the rest of struct
         S32      Temperature;      // Temperature near the CPU. 
         S32      TemperatureHiThreshold;    // >= Threshold value to turn fans up. 
         S32      TemperatureNormThreshold;  // <= Threshold value to set fan speed to normal.
      }; /* end of class MsgCmbPollEnvironment::CPayload::CMB_SLOT_RECORD */
     
      CtEVCRawParameterRecord EVCRawParameters[2];    // 2 EVCs for now. 
      CMB_SLOT_RECORD CmbSlotInfo[CMB_ADDRESS_MAX];

   };  /* end of class MsgCmbPollEnvironment::CPayload */

   //  read our message's present payload
   inline BOOL  GetPayload (CPayload *& pPayload)
   {
   U32   cbPayload;

#ifdef WIN32
	  GetSgl (0, (void **)&pPayload, &cbPayload);
#else
      GetSgl (0, &pPayload, &cbPayload);
#endif
      return ((pPayload != NULL) && (cbPayload == sizeof (*pPayload)));
   }

   //  set our message's payload
   inline void  SetPayload (const CPayload& Payload)
   {
      AddSgl (0, (void *) &Payload, sizeof (Payload), SGL_COPY);
   }

};  /* end of class ::MsgCmbPollEnvironment */


//  forwards a very small request code-based message to another IOP via the CMB
class MsgCmbSendSmallMsg : public Message {
public:

   static inline REQUESTCODE  MyRequestCode (void)
      {  return (CMB_SEND_SMALL_MSG);  };

   MsgCmbSendSmallMsg(REQUESTCODE eTargetRequest, TySlot eTargetIop,
                      void *pvTargetPayload, U32 cbTargetPayload);

   class CPayload {
   public:
      CPayload (REQUESTCODE eTargetRequest, TySlot eTargetIop) :
                  m_eTargetRequest (eTargetRequest),
                  m_eTargetIop (eTargetIop)
            {};

      REQUESTCODE m_eTargetRequest;
      TySlot      m_eTargetIop;
      U32         m_cbTargetPayload;
      U8          m_abTargetPayload [20];
   };  /* end of class MsgCmbSendSmallMsg::CPayload */

   inline BOOL  GetPayload (CPayload *& pPayload)
   {
      pPayload = (CPayload *) Message::GetPPayload ();
      return (pPayload != NULL);
   }
};  /* end of class ::MsgCmbSendSmallMsg */


//  ask CMB DDM to set our local IOP/HBC "MIPS State" to given value
//  -- this causes an update to the local board's row in the IOP Status
//     table, and to the local AVR's "MIPS State" value
class MsgCmbSetMipsState : public Message {
public:

   static inline REQUESTCODE  MyRequestCode (void)
      {  return (CMB_SET_MIPS_STATE);  };

   enum Action {
      NOP = 1,          // (invalid, to force user to define an action)
      SetOperating,     // normal running state
      SetSuspended      // we're not allowed to touch the PCI bus
      };

   MsgCmbSetMipsState(Action eAction) : Message(MyRequestCode()),
                                        m_Payload (eAction)
      {};

   struct Payload
   {
      Action   eAction;

      Payload (Action _eAction = NOP) : eAction (_eAction)
         {};
   };

   Payload  m_Payload;
    
};  /* end of class MsgCmbSetMipsState */


//  ask CMB DDM to update given IOP slot's status in PTS' IOP Status table
class MsgCmbUpdateIopStatus : public Message {
public:

   static inline REQUESTCODE  MyRequestCode (void)
      {  return (CMB_UPDATE_IOP_STATUS);  };

   MsgCmbUpdateIopStatus() : Message(MyRequestCode())
      {  };
   
   TySlot         eTargetSlot;   // Slot this command is for.

};  /* end of class MsgCmbUpdateIopStatus */


#endif  // #ifndef _DdmCMBMsgs_h


