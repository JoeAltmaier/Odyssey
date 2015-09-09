/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: EnvStatusReport.h
//
//
// Description:
// This header file defines the status data record between the CMB Master
// and the EVC Status Reporter.
// 
// $Log: /Gemini/Include/EnvStatusReport.h $
// 
// 4     9/08/99 9:47a Vnguyen
// Change temperature threshold from U32 to S32.
// 
// 3     9/03/99 5:01p Ewedel
// Made structs into classes, moved CMB_SLOT_RECORD inside of
// ENV_STATUS_RECORD for safer name scoping.
// 
// 2     9/03/99 2:07p Vnguyen
// Clean up record structure.  Rename Temp1 to Temperature, etc.
// 
// 1     8/25/99 2:52p Vnguyen
// Initial check-in.
// 
//
// Update Log: 
// 
/*************************************************************************/

#ifndef __EnvStatusReport_h
#define __EnvStatusReport_h


#include "Address.h" 
#include "EVCRawParameters.h" 



class ENV_STATUS_RECORD
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

      bool  fPresent;         // True: if an IOP is installed
                              // False:  ignore the rest of struct
      S32      Temperature;      // Temperature near the CPU. 
      S32      TemperatureHiThreshold;    // >= Threshold value to turn fans up. 
      S32      TemperatureNormThreshold;  // <= Threshold value to set fan speed to normal.
   }; /* end of class CMB_SLOT_RECORD */

   CtEVCRawParameterRecord EVCRawParameters[2];    // 2 EVC for now. 
   CMB_SLOT_RECORD CmbSlotInfo[CMB_ADDRESS_MAX];

};  /* end of class ENV_STATUS_RECORD */


#endif // __EnvStatusReport_h
