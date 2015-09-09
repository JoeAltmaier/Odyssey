/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// SystemVdn.h
//
// Description:
// This file contains a fixed list of virtual device numbers that will be
// used in test builds.  When the system tables are defined for virtual
// device numbers, this file will go away.
// 
// Update Log:
//	$Log: /Gemini/Include/SystemVdn.h $
// 
// 17    2/11/00 1:47p Rbraun
// Added VDNs for Test Manager and Network Manager SYSTEMMASTERs
// 
// 16    9/17/99 11:00a Cchan
// Added VDNs (vdnHSCSI, vdnSM) necessary for HBC SCSI (QL1040B).
// 
// 15    9/16/99 11:15p Jlane
// Add VirtualManager vdn.
// 
// 14    7/28/99 7:51p Mpanas
// Add E2 NAC values
// 
// 13    7/24/99 8:01p Jlane
// Change NAC slot
// 
// 12    7/15/99 11:26p Mpanas
// Changes to support Multiple FC Instances
// 
// 11    6/29/99 9:19p Mpanas
// Remove the MY_SLOT() macro since it
// is not used anymore
// 
// 10    6/18/99 7:37p Mpanas
// Add Vdn for the LoopMonitor
// 
// 9     6/03/99 9:43p Mpanas
// Add more BSAs for test config
// 
// 8     5/14/99 4:27p Mpanas
// Add RAID support for test
// 
// 7     5/13/99 5:57p Mpanas
// Remove Echo SCSI
// 
// 6     4/28/99 6:24p Jfrandeen
// Add SSD virtual device numbers
// 
// 5     4/26/99 4:06p Mpanas
// Temp comment out DDM Registrar
// 
// 4     3/30/99 9:14p Jlane
// Merged in HBC devices.
// 
// 3     3/30/99 8:26p Mpanas
// Global Virtual Device Entries
// Include this in your BuildSys.cpp file
// 
// 2     3/18/99 5:10p Mpanas
// Remove #ifdefs
// 
// 03/03/99 Michael G. Panas: Create file
/*************************************************************************/

#ifndef __SystemVdn_h
#define __SystemVdn_h

// Temp Values used on E1 only
#define	RAC_SLOT	IOP_RAC0
#define	NIC_SLOT	IOP_NIC1

// these defines for E2 only
#define	NAC_SLOT	IOP_RAC0	// #24 (DDH access Upper)
#define	NAC_SLOT1	IOP_RAC1	// #28 (DDH access Lower)
#define	NAC_SLOT2	IOP_NIC0	// #27
#define	NAC_SLOT3	IOP_NIC1	// #31
#define	NAC_SLOT4	IOP_APP1	// #26
#define	NAC_SLOT5	IOP_APP3	// #30
#define	NAC_SLOT6	IOP_APP0	// #25
#define	NAC_SLOT7	IOP_APP2	// #29

// SSD slots on E2
#define	NAC_SLOT8	IOP_SSDU0	// #16
#define	NAC_SLOT9	IOP_SSDL0	// #20


//=========================================================================
// Virtual Device Numbers
//
// These numbers will eventually come from the persistant
// table service.
//
enum {
	vdnPTS = 1,			// PTS class interface
	vdnVMS,				// VirtualManager class interface
	vdn_RedundancyMgr,	// Listens for and handles class
						// registration by Registrars on IOPS.
	vdn_Registrar,		// Registers classes on an IOP
	vdn_CMB,			// The Card Management Bus Ddm
	vdn_BootMgr,		// The Actual Boot Process Driver.
	vdnFC_Master,		// the FC Master device
	vdn_TestMgr,		// Neptune Test Manager
	vdn_NetMgr,			// Networking Ddm
	
	vdnLM,				// FC Loop Monitor DDM (one per FC board)
	vdnLM1,
	vdnLM2,
	vdnLM3,
	vdnLM4,
	vdnLM5,
	vdnLM6,
	vdnLM7,

	vdnInit,		// FCP Initiator DDM
	vdnInit1,
	vdnInit2,
	vdnInit3,
	vdnInit4,
	vdnInit5,
	vdnInit6,
	vdnInit7,
	vdnInit8,
	vdnInit9,
	vdnInit10,
	vdnInit11,
	vdnInit12,
	vdnInit13,
	vdnInit14,
	vdnInit15,
	vdnInit16,
	vdnInit17,
	vdnInit18,
	vdnInit19,
	vdnInit20,
	
	vdnTarget,		// FCP Target DDM
	vdnTarget1,
	vdnTarget2,
	vdnTarget3,
	vdnTarget4,
	vdnTarget5,
	vdnTarget6,
	vdnTarget7,
	vdnTarget8,
	vdnTarget9,
	vdnTarget10,
	vdnTarget11,
	vdnTarget12,
	vdnTarget13,
	vdnTarget14,
	vdnTarget15,
	vdnTarget16,
	vdnTarget17,
	vdnTarget18,
	vdnTarget19,
	vdnTarget20,
	
	vdnDM,			// Drive Monitor ISM (one per Init Loop)
	vdnDM1,
	vdnDM2,
	vdnDM3,
	vdnDM4,
	vdnDM5,
	vdnDM6,
	vdnDM7,
	vdnDM8,
	vdnDM9,
	vdnDM10,
	vdnDM11,
	vdnDM12,
	vdnDM13,
	vdnDM14,
	vdnDM15,
	vdnDM16,
	vdnDM17,
	vdnDM18,
	vdnDM19,
	vdnDM20,
	
	vdnHSCSI,	// HSCSI initiator
	vdnSM,		// ScsiMonitor ISM for HSCSI
		
	vdnSTS0,	// SCSI target Server 0
	vdnSTS1,	// SCSI target Server 1
	vdnSTS2,	// SCSI target Server 2
	vdnSTS3,	// SCSI target Server 3
	vdnSTS4,	// SCSI target Server 4
	vdnSTS5,	// SCSI target Server 5
	vdnSTS6,	// SCSI target Server 6
	vdnSTS7,	// SCSI target Server 7

	vdnBSA0,	// BSA ISM 0
	vdnBSA1,	// BSA ISM 1
	vdnBSA2,	// BSA ISM 2
	vdnBSA3,	// BSA ISM 3
	vdnBSA4,	// BSA ISM 4
	vdnBSA5,	// BSA ISM 5
	vdnBSA6,	// BSA ISM 6
	vdnBSA7,	// BSA ISM 7
	vdnBSA8,	// BSA ISM 8
	vdnBSA9,	// BSA ISM 9
	vdnBSA10,	// BSA ISM 10
	vdnBSA11,	// BSA ISM 11
	vdnBSA12,	// BSA ISM 12
	vdnBSA13,	// BSA ISM 13
	vdnBSA14,	// BSA ISM 14
	vdnBSA15,	// BSA ISM 15
	vdnBSA16,	// BSA ISM 16
	vdnBSA17,	// BSA ISM 17
	vdnBSA18,	// BSA ISM 18
	vdnBSA19,	// BSA ISM 19
	
	vdnBSA20,	// BSA ISM 20
	vdnBSA21,	// BSA ISM 21
	vdnBSA22,	// BSA ISM 22
	vdnBSA23,	// BSA ISM 23
	vdnBSA24,	// BSA ISM 24
	vdnBSA25,	// BSA ISM 25
	vdnBSA26,	// BSA ISM 16
	vdnBSA27,	// BSA ISM 17
	vdnBSA28,	// BSA ISM 18
	vdnBSA29,	// BSA ISM 19
	
	vdnRD0,		// RAM Disk 0
	vdnRD1,		// RAM Disk 1
	vdnRD2,		// RAM Disk 2
	vdnRD3,		// RAM Disk 3
	vdnRD4,		// RAM Disk 4
	vdnRD5,		// RAM Disk 5
	
	vdnSSD0,	// SSD 0
	vdnSSD1,	// SSD 1
	vdnSSD2,	// SSD 2
	vdnSSD3,	// SSD 3
	vdnSSD4,	// SSD 4
	vdnSSD5,	// SSD 5
	
	vdnRAID0,	// Raid 0
	vdnRAID1,	// Raid 1
	vdnRAID2,	// Raid 2
	vdnRAID3,	// Raid 3
	vdnRAID4,	// Raid 4
	vdnRAID5,	// Raid 5
	vdnRAID6,	// Raid 6
	vdnRAID7,	// Raid 7
	
	vdnLAST		// last entry in table
};



#endif