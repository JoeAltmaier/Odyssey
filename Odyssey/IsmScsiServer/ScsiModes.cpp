/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// File: ScsiModes.c
// 
// Description:
// Handle the SCSI Mode Sense and Mode Select commands for this Virtual Circuit
// 
// Update Log 
//	$Log: /Gemini/Odyssey/IsmScsiServer/ScsiModes.cpp $
// 
// 10    11/15/99 4:08p Mpanas
// Re-organize sources
// - Add new files: ScsiInquiry.cpp, ScsiReportLUNS.cpp, 
//   ScsiReserveRelease.cpp
// - Remove unused headers: ScsiRdWr.h, ScsiMessage.h, ScsiXfer.h
// - New Listen code
// - Use Callbacks
// - All methods part of SCSI base class
// - Remove DoWork() and replace with RequestDefault() and ReplyDefault()
//
// 
// 06/01/98 Michael G. Panas: Create file
// 01/20/99 Michael G. Panas: implement basic mode page handling.  Support
//                            mode pages 1, 2, 4 only for now.
// 02/19/99 Michael G. Panas: convert to new Message format
// 03/21/99 Michael G. Panas: add code to calc mode page 3 and 4 values
/*************************************************************************/

#include "SCSIServ.h"
#include "ScsiSense.h"
#include "FcpMessageFormats.h"
#include "ScsiModes.h"

#include "string.h"

/*************************************************************************/
// Forward References
/*************************************************************************/

/*************************************************************************/
// Global references
/*************************************************************************/

#pragma pack(1)


// default values for mode pages
MODE_PAGE_1		Page1 = 
	{
	{0,0, RW_ERROR_RECOVERY, PAGE1_LEN},
	1, 1, 0, 0, 0, 0, 0, 0,		// flags
	1,							// RdRetry
	0,							// Span
	0,							// HeadOffset
	0,							// DataStrbOffset
	0,							// Res1
	1,							// WrRetry
	0,							// Res2
	0							// RecoveryTimeLimit
	};
MODE_PAGE_2		Page2 =
	{
	{0,0, DISCONNECT_RECONNECT, PAGE2_LEN},
	128,						// BufferFullRatio
	16,							// BufferEmptyRatio
	0,							// BusInactivityLimit
	0,							// DisconnectTimeLimit
	0,							// ConnectTimeLimit
	8,							// MaxBurst
	0, 0,						// EDMP, Res
	{0,0,0}						// Res2
	};
	
MODE_PAGE_3		Page3 =
	{
	{0,0, FORMAT_DEVICE, PAGE3_LEN},
	STS_MODE_TRACKS_PER_ZONE,		// TracksPerZone
	STS_MODE_ALT_SECT_PER_ZONE,		// AltSectPerZone
	STS_MODE_ALT_TRACKS_PER_ZONE,	// AltTracksPerZone
	STS_MODE_ALT_TRKS_PER_LUN,		// AltTracksPerLUN
	STS_MODE_SECT_PER_TRACK,		// SectorsPerTrack
	STS_MODE_BYTES_PER_SECTOR,		// BytesPerPhysSect
	STS_MODE_INTERLEAVE,			// Interleave
	STS_MODE_TRACK_SKEW,			// TrackSkew
	STS_MODE_CYL_SKEW,				// CylSkew
	0, 1, 0, 0, 0,					// SSEC, HSEC, RMB, SURF, res1
	{0,0,0}							// Res2
	};

MODE_PAGE_4		Page4 =
	{
	{0,0, RIGID_DISK_GEOMETRY, PAGE4_LEN},
	{0,0,0},					// Number of Cylinders
	STS_MODE_NUM_HEADS,			// Number of Heads
	{0,0,0},					// WrPrecompStartCyl
	{0,0,0},					// ReducedWrCurrentStartCyl
	0,							// DriveStepRate
	{0,0,0},					// LandingZoneCyl
	0,							// Res1
	0,							// RPL
	0,							// RotationalOffset
	0,							// Res2
	STS_MODE_ROT_SPEED,			// MediumRotationRate (in RPM)
	{0,0}						// Res3 
	};

// default values for the mode parameter header
MODE_HEADER6	ModeHeader6 =
{
	0,			// length (n - 1)
	0,			// medium type
	0,			// device specific
	0,			// block descriptor length (0 or 8)
};

MODE_HEADER10	ModeHeader10 =
{
	0,			// length (n  - 1)
	0,			// medium type
	0,			// device specific
	0,			// reserved (zero)
	0,			// block descriptor length (0 or 8)
};

// default values for the block descriptor
BLOCK_DESC		BlockDesc =
{
	0,			// density
	{0,0,0},	// number of blocks
	0,			// reserved (zero)
	{0, (512>>8), 0}			// block length
};

// size of all available mode pages
U32				AllPagesLen = sizeof(MODE_PAGE_1) +
							sizeof(MODE_PAGE_2) +
							sizeof(MODE_PAGE_3) +
							sizeof(MODE_PAGE_4);

// changable masks
// these values include a one bit for each bit in the mode page that
// can be changed.  TODO: generate real masks, for now all are not changeable
U8	Mask_Page_1[sizeof(MODE_PAGE_1)] = {0};
U8	Mask_Page_2[sizeof(MODE_PAGE_2)] = {0};
U8	Mask_Page_3[sizeof(MODE_PAGE_3)] = {0};
U8	Mask_Page_4[sizeof(MODE_PAGE_4)] = {0};

// TEMP hack to use Seagate Data for testing
U8	Seagate_Modes[] =
{
	0xa7,0x00,0x10,0x08,0x01,0x0f,0x59,0xc8,
	0x00,0x00,0x02,0x00,0x81,0x0a,0xc0,0x0c,
	0xe8,0x00,0x00,0x00,0x05,0x00,0xff,0xff,
	0x82,0x0e,0x80,0x80,0x00,0x0a,0x00,0x00,
	0x00,0x00,0x02,0x2b,0x00,0x00,0x00,0x00,
	0x83,0x16,0x14,0xd0,0x00,0x00,0x00,0x0a,
	0x00,0x00,0x00,0xd6,0x02,0x00,0x00,0x01,
	0x00,0x24,0x00,0x2c,0x40,0x00,0x00,0x00,
	0x84,0x16,0x00,0x1b,0x32,0x0c,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x27,0x3d,0x00,0x00,
	0x87,0x0a,0x00,0x0c,0xe8,0x00,0x00,0x00,
	0x00,0x00,0xff,0xff,0x88,0x12,0x14,0x00,
	0xff,0xff,0x00,0x00,0xff,0xff,0xff,0xff,
	0x80,0x03,0x00,0x00,0x00,0x00,0x00,0x00,
	0x8a,0x0a,0x02,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x99,0x06,0x00,0x00,
	0x00,0x00,0x00,0x00,0x9a,0x0a,0x00,0x00,
	0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x04,
	0x9c,0x0a,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x01,0x80,0x02,0x00,0x00
};

//******************************************************************************
//	ScsiBuildModeSenseData
//	Build a set of default Mode Pages for this Virtual Circuit.  These values
//  are built at the address pointed to by p.  This area must be large enough to
//  hold all the default mode pages ( <=255 bytes).  It is assumed that p[0]
//  is the total count byte.  No header is saved with this data, since it could
//  be a 4 or 7 byte header with or without an 8 byte blockdescriptor attached.
//******************************************************************************
void ScsiServerIsm::ScsiBuildModeSenseData(U8 *p)
{
	U8			*pnext = p;
	U32			 Cap = (U32)pStsExport->Capacity;
	
	TRACE_ENTRY(ScsiBuildModeSenseData);
	
	// Calculate the number of Cylinders from this formula:
	//	Num_Cyl = Capacity / (STS_MODE_NUM_HEADS * STS_MODE_SECT_PER_TRACK)
	U32 num_cyl = Cap / (STS_MODE_NUM_HEADS * STS_MODE_SECT_PER_TRACK);
	
	Page4.NumberofCylinders[0] = (num_cyl >> 16) & 0xff;
	Page4.NumberofCylinders[1] = (num_cyl >> 8) & 0xff;
	Page4.NumberofCylinders[2] = num_cyl & 0xff;
	
	// skip the count
	pnext++;
	
	memcpy(pnext, (U8 *)&Page1, sizeof(MODE_PAGE_1));
	memcpy(pnext+sizeof(MODE_PAGE_1),
			(U8 *)&Page2, sizeof(MODE_PAGE_2));
	memcpy(pnext+sizeof(MODE_PAGE_1)+sizeof(MODE_PAGE_2),
			(U8 *)&Page3, sizeof(MODE_PAGE_3));
	memcpy(pnext+sizeof(MODE_PAGE_1)+sizeof(MODE_PAGE_2)+sizeof(MODE_PAGE_3),
			(U8 *)&Page4, sizeof(MODE_PAGE_4));

	// calculate the size
	p[0] = sizeof(MODE_PAGE_1) + sizeof(MODE_PAGE_2) +
				sizeof(MODE_PAGE_3)+sizeof(MODE_PAGE_4);
	
} // ScsiBuildModeSenseData


//******************************************************************************
//	ScsiServerModeSense
//	Handle the Mode Sense command for this Virtual Circuit
//******************************************************************************
U32
ScsiServerIsm::ScsiServerModeSense(SCSI_CONTEXT *p_context)
{
	U8					*pData, *pHdr, *pPage;
	long				 length, hdr_len;
	SCB_PAYLOAD			*pP = 
						 (SCB_PAYLOAD *)((Message *)p_context->pMsg)->GetPPayload();
	PMODE_SENSE10		 pMs = (PMODE_SENSE10)&pP->CDB[0];
	U32					 Cap = (U32)pStsExport->Capacity;
	
	TRACE_ENTRY(ScsiServerModeSense);
	
	// validate some fields
	if (pP->ByteCount == 0)
	{
		// zero length is not an error, just return
		// good status
		ScsiReply(p_context, 0);
		return(0);
	}
	
	// build the header
	BlockDesc.NumBlocks[0] = (Cap >> 16) & 0xff;
	BlockDesc.NumBlocks[1] = (Cap >> 8) & 0xff;
	BlockDesc.NumBlocks[2] = (Cap) & 0xff;
	
	if (pMs->Cmd == CMD_MODE_SENSE6)
	{
		ModeHeader6.BlockDescLength = (pMs->DBD) ? 0 : sizeof(BLOCK_DESC);
		hdr_len = (sizeof(MODE_HEADER6) + ModeHeader6.BlockDescLength);
		pHdr = new unsigned char[hdr_len];
		memcpy(pHdr, &ModeHeader6, sizeof(MODE_HEADER6));
		if (ModeHeader6.BlockDescLength)
			memcpy(pHdr + sizeof(MODE_HEADER6), &BlockDesc, sizeof(BLOCK_DESC));
	}
	else
	{
		ModeHeader10.BlockDescLength = (pMs->DBD) ? 0 : sizeof(BLOCK_DESC);
		hdr_len = (sizeof(MODE_HEADER10) + ModeHeader10.BlockDescLength);
		pHdr = new unsigned char[hdr_len];
		memcpy(pHdr, &ModeHeader10, sizeof(MODE_HEADER10));
		if (ModeHeader10.BlockDescLength)
			memcpy(pHdr + sizeof(MODE_HEADER10), &BlockDesc, sizeof(BLOCK_DESC));
	}
	
	if ((pMs->PageCode == RIGID_DISK_GEOMETRY) ||
			(pMs->PageCode == RETURN_ALL_PAGES))	
	{
		// Calculate the number of Cylinders from this formula:
		//	Num_Cyl = Capacity / (STS_MODE_NUM_HEADS * STS_MODE_SECT_PER_TRACK)
		U32 num_cyl = Cap / (STS_MODE_NUM_HEADS * STS_MODE_SECT_PER_TRACK);
		
		Page4.NumberofCylinders[0] = (num_cyl >> 16) & 0xff;
		Page4.NumberofCylinders[1] = (num_cyl >> 8) & 0xff;
		Page4.NumberofCylinders[2] = num_cyl & 0xff;
		
		TRACE_HEX(TRACE_L7, "\n\rMode Sense Number of Cylinders = ", (U32)num_cyl);
		
	}
			
	// check which page or pages are wanted
	switch (pMs->PageCode)
	{
	case RETURN_ALL_PAGES:
		length = AllPagesLen + hdr_len;
		
		// allocate the data area
		pData = new unsigned char[length];
		
		// was the allocation correct?
		if (length > pP->ByteCount)
		{
			length = pP->ByteCount;
		}
		
		// update the length in the mode parameter header
		pHdr[0] = (U8)length - 1;
		
		// build up the data area
		// copy the header first
		memcpy(pData, pHdr, hdr_len);
		// clean up
		delete pHdr;
		
		// find out where to get the page data
		switch (pMs->PC)
		{
		case CHANGEABLE_VALUES:
			pPage = (U8 *)&Mask_Page_4;
			memcpy(pData+hdr_len, (U8 *)&Mask_Page_1, sizeof(MODE_PAGE_1));
			memcpy(pData+hdr_len+sizeof(MODE_PAGE_1),
					(U8 *)&Mask_Page_2, sizeof(MODE_PAGE_2));
			memcpy(pData+hdr_len+sizeof(MODE_PAGE_1)+sizeof(MODE_PAGE_2),
					(U8 *)&Mask_Page_3, sizeof(MODE_PAGE_3));
			memcpy(pData+hdr_len+sizeof(MODE_PAGE_1)+sizeof(MODE_PAGE_2)+sizeof(MODE_PAGE_3),
					(U8 *)&Mask_Page_4, sizeof(MODE_PAGE_4));
					
			TRACE_DUMP_HEX(TRACE_L7, "\n\rMode Sense Data (changeable)= ", pData, length);
		
			break;
		case CURRENT_VALUES:
		case DEFAULT_VALUES:
		case SAVED_VALUES:
#if 0
			memcpy(pData, (U8 *)&Seagate_Modes, sizeof(length));
#else
			// TODO:
			// get the mode page data from the correct area
			// for now just copy the DEFAULT_VALUES
			// make sure actual len does not exceed Length allocated
			memcpy(pData+hdr_len, (U8 *)&Page1, sizeof(MODE_PAGE_1));
			memcpy(pData+hdr_len+sizeof(MODE_PAGE_1),
					(U8 *)&Page2, sizeof(MODE_PAGE_2));
			memcpy(pData+hdr_len+sizeof(MODE_PAGE_1)+sizeof(MODE_PAGE_2),
					(U8 *)&Page3, sizeof(MODE_PAGE_3));
			memcpy(pData+hdr_len+sizeof(MODE_PAGE_1)+sizeof(MODE_PAGE_2)+sizeof(MODE_PAGE_3),
					(U8 *)&Page4, sizeof(MODE_PAGE_4));
#endif			
			TRACE_DUMP_HEX(TRACE_L7, "\n\rMode Sense Data (saved)= ", pData, length);
		
			break;
		}
		
		TRACE_HEX(TRACE_L7, "\n\rMode Sense Len = ", (U32)length);
		
		// copy the structure to the SGL address in the message
		ScsiSendData(p_context, (U8 *)pData, length);
		
		// reply to the message (with no errors)
		ScsiReply(p_context, length);
		
		// cleanup
		delete pData;
		break;
	
	case RW_ERROR_RECOVERY:
		length = sizeof(MODE_PAGE_1) + hdr_len;
		
		// was the allocation correct?
		if (length > pP->ByteCount)
		{
			length = pP->ByteCount;
		}
		
		// allocate the data area
		pData = new unsigned char[length];
		
		// update the length in the mode parameter header
		pHdr[0] = (U8)length - 1;
		
		// build up the data area
		// copy the header first
		memcpy(pData, pHdr, hdr_len);
		
		// find out where to get the data
		switch (pMs->PC)
		{
		case CHANGEABLE_VALUES:
			pPage = (U8 *)&Mask_Page_1;
			break;
		case CURRENT_VALUES:
		case DEFAULT_VALUES:
		case SAVED_VALUES:
			// TODO:
			// get the mode page data from the correct area
			// for now just point to the DEFAULT_VALUES
			pPage = (U8 *)&Page1;
			break;
		}
		
		// then copy the page data
		memcpy(pData+hdr_len, pPage, sizeof(MODE_PAGE_1));
		
		// copy the structure to the SGL address in the message
		ScsiSendData(p_context, (U8 *)pData, length);
		
		// reply to the message (with no errors)
		ScsiReply(p_context, length);
		
		// cleanup
		delete pData;
		delete pHdr;
		break;
	
	case DISCONNECT_RECONNECT:
		length = sizeof(MODE_PAGE_2) + hdr_len;
		
		// was the allocation correct?
		if (length > pP->ByteCount)
		{
			length = pP->ByteCount;
		}
		
		// allocate the data area
		pData = new unsigned char[length];
		
		// update the length in the mode parameter header
		pHdr[0] = (U8)length - 1;
		
		// build up the data area
		// copy the header first
		memcpy(pData, pHdr, hdr_len);
		
		// find out where to get the data
		switch (pMs->PC)
		{
		case CHANGEABLE_VALUES:
			pPage = (U8 *)&Mask_Page_2;
			break;
		case CURRENT_VALUES:
		case DEFAULT_VALUES:
		case SAVED_VALUES:
			// TODO:
			// get the mode page data from the correct area
			// for now just point to the DEFAULT_VALUES
			pPage = (U8 *)&Page2;
			break;
		}
		
		// then copy the page data
		memcpy(pData+hdr_len, pPage, sizeof(MODE_PAGE_2));
		
		// copy the structure to the SGL address in the message
		ScsiSendData(p_context, (U8 *)pData, length);
		
		// reply to the message (with no errors)
		ScsiReply(p_context, length);
		
		// cleanup
		delete pData;
		delete pHdr;
		break;
	
	case FORMAT_DEVICE:
	
		length = sizeof(MODE_PAGE_3) + hdr_len;
		
		// was the allocation correct?
		if (length > pP->ByteCount)
		{
			length = pP->ByteCount;
		}
		
		// allocate the data area
		pData = new unsigned char[length];
		
		// update the length in the mode parameter header
		pHdr[0] = (U8)length - 1;
		
		// build up the data area
		// copy the header first
		memcpy(pData, pHdr, hdr_len);
		
		// find out where to get the data
		switch (pMs->PC)
		{
		case CHANGEABLE_VALUES:
			pPage = (U8 *)&Mask_Page_3;
			break;
		case CURRENT_VALUES:
		case DEFAULT_VALUES:
		case SAVED_VALUES:
			// TODO:
			// get the mode page data from the correct area
			// for now just point to the DEFAULT_VALUES
			pPage = (U8 *)&Page3;
			break;
		}
		
		// then copy the page data
		memcpy(pData+hdr_len, pPage, sizeof(MODE_PAGE_3));
		
		// copy the structure to the SGL address in the message
		ScsiSendData(p_context, (U8 *)pData, length);
		
		// reply to the message (with no errors)
		ScsiReply(p_context, length);
		
		// cleanup
		delete pData;
		delete pHdr;
		break;
	
	case RIGID_DISK_GEOMETRY:
	
		length = sizeof(MODE_PAGE_4) + hdr_len;
		
		// was the allocation correct?
		if (length > pP->ByteCount)
		{
			length = pP->ByteCount;
		}
		
		// allocate the data area
		pData = new unsigned char[length];
		
		// update the length in the mode parameter header
		pHdr[0] = (U8)length - 1;
		
		// build up the data area
		// copy the header first
		memcpy(pData, pHdr, hdr_len);
		
		// find out where to get the data
		switch (pMs->PC)
		{
		case CHANGEABLE_VALUES:
			pPage = (U8 *)&Mask_Page_4;
			break;
		case CURRENT_VALUES:
		case DEFAULT_VALUES:
		case SAVED_VALUES:
			// TODO:
			// get the mode page data from the correct area
			// for now just point to the DEFAULT_VALUES
			pPage = (U8 *)&Page4;
			break;
		}
		
		// then copy the page data
		memcpy(pData+hdr_len, pPage, sizeof(MODE_PAGE_4));
		
		// copy the structure to the SGL address in the message
		ScsiSendData(p_context, (U8 *)pData, length);
		
		// reply to the message (with no errors)
		ScsiReply(p_context, length);
		
		// cleanup
		delete pData;
		delete pHdr;
		break;
	
	default:
error:
		// cause an error, not a page we support
		SetStatus(SENSE_ILLEGAL_REQUEST, ASC_INVALID_FIELD_IN_CDB);
		ScsiErrReply(p_context);
		break;
	}
		
	return(0);
	
} // ScsiServerModeSense


//******************************************************************************
//	ScsiServerModeSelect
//	Handle the Mode Select command for this Virtual Circuit
//******************************************************************************
U32
ScsiServerIsm::ScsiServerModeSelect(SCSI_CONTEXT *p_context)
{
	TRACE_ENTRY(ScsiServerModeSelect);
	
	// cause an error, Mode Select not supported yet
	SetStatus(SENSE_ILLEGAL_REQUEST, ASC_INVALID_FIELD_IN_CDB);
	ScsiErrReply(p_context);
	return(0);
	
} // ScsiServerModeSelect
