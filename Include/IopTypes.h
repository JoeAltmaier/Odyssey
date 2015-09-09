/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: IopTypes.h
// 
// Description:
// This file contains an enum defining the IOP card type constants
// used by hardware.  These are the values used for reporting IOP type
// both by raw hardware and via the CMB.
// 
// $Log: /Gemini/Include/IopTypes.h $
// 
// 3     12/17/99 7:59p Ewedel
// Added board type code for Triton HBC.
// 
// 2     7/15/99 5:46p Ewedel
// Changed old type IOPTY_NIC_RAC to IOPTY_NAC.  Also renumbered types to
// match current actual hardware def (supplied independently by Jeff N).
// And added EVC / DDH type codes.
// 
// 1     3/09/99 4:49p Ewedel
// Initial revision.
//
/*************************************************************************/

#if !defined(_IopTypes_h_)
#define _IopTypes_h_


//  here are the IOP type values -- these are the same as the
//  codes returned by hardware for IOP type.
//  For general utility, we also include the other board types
//  which can be accessed via CMB:  EVC and DDH.

enum IopType {
   IOPTY_HBC = 1,       // host bridge controller (original four-bus version)
   IOPTY_NAC = 2,       // multi-port fibre channel NIC/RAC hybrid
   IOPTY_SSD = 3,       // solid state drive
   IOPTY_NIC = 4,       // network interface card (fibre channel)
   IOPTY_RAC = 5,       // RAID array controller (fibre channel)
   IOPTY_HBC_TRI = 6,   // "Triton" system HBC (two-bus bridge)
   IOPTY_EVC = 129,     // environmental controller (not an IOP)
   IOPTY_DDH = 130      // disk drive fibre hub (not an IOP)
};


#endif /* _IopTypes_h_  */
