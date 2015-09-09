/*************************************************************************/
// 
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: ScsiModes.h
// 
// Description:
// This file contains Mode Sense/Select handler interfaces for
// the Scsi Target Server.
// 
// Update Log 
// 
// 12/11/98 Michael G. Panas: Create file
// 03/21/99 Michael G. Panas: Add parms to calc Page 4 Head and Cylinder
//                            values.
/*************************************************************************/


#ifndef __ScsiModes_h
#define __ScsiModes_h

#define	STS_MODE_NUM_HEADS				16		// number of heads
#define	STS_MODE_SECT_PER_TRACK			16		// sectors per track
#define	STS_MODE_ROT_SPEED				3600	// RPM

#define	STS_MODE_TRACKS_PER_ZONE		0		// Tracks Per Zone
#define	STS_MODE_ALT_SECT_PER_ZONE		0		// Alternate Sectors per zone
#define	STS_MODE_ALT_TRACKS_PER_ZONE	0
#define	STS_MODE_ALT_TRKS_PER_LUN		0
#define	STS_MODE_BYTES_PER_SECTOR		512
#define	STS_MODE_INTERLEAVE				1
#define	STS_MODE_TRACK_SKEW				0
#define	STS_MODE_CYL_SKEW				0
#define	STS_MODE_HARD_SECTORED			1

#endif