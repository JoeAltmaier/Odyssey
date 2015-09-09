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
// File: DriveMonitorIsm.h
// 
// Description:
// This file contains the derived class for the DriveMonitorIsm
// 
// Update Log 
//	$Log: /Gemini/Odyssey/IsmDriveMonitor/DriveMonitorIsm.h $
// 
// 23    2/02/00 5:49p Jlane
// Add prototype for FinishInitialize() as part of changes to remove
// internal disks.
// 
// 22    1/11/00 7:27p Mpanas
// New PathTable changes
// 
// 21    12/23/99 6:09p Jlane
// Readd new vd create support previously backed out.
// 
// 20    11/10/99 3:58p Jlane
// Roll back in previous VD Create support code.
// 
// 18    10/12/99 8:33p Mpanas
// Add support for removed drives
// - Update DiskDescriptor with DriveRemoved
// - correct several bugs, flags not inverted correctly
// - Flag drives that have been changed (with a trace message only)
// 
// 17    10/11/99 7:20p Mpanas
// Second part of BSA VD Create, check for prior
// BSA device entries (like in BuildSys) 
// Note: this change needs the fix in VirtualDeviceTable.h
// Initialize to zero pad the class string.
// 
// 16    10/07/99 8:09p Mpanas
// First cut of BSA VD Create
// 
// 15    9/30/99 2:44p Mpanas
// Fix DriveMonitor / LoopMonitor race to update LoopDescriptor
// make sure we use row ID as key, copy listen data to local data
// 
// 14    9/16/99 9:56p Mpanas
// More device states
// 
// 13    9/14/99 8:41p Mpanas
// Complete re-write of DriveMonitor
// - Scan in sequence
// - Start Motors in sequence
// - LUN Scan support
// - Better table update
// - Re-organize sources
// 
// 12    8/14/99 9:37p Mpanas
// Support for new LoopMonitor
// 
// 11    7/15/99 11:53p Mpanas
// Changes to support Multiple FC Instances
// and support for NAC
// -New Message front end for the DriveMonitor
// - remove all external entry points
// 
// 10    5/19/99 6:22p Mpanas
// Add Write/Read/Verify Test
// 
// 9     4/26/99 12:47p Mpanas
// changes to Include IDLUN in the SCSI Payload of message
// instead of signature
// 
// 8     4/09/99 3:20a Mpanas
// Add Test code to do a continuous Read when
// requested
// 
// 7     4/08/99 7:13a Mpanas
// Remove unused method
// 
// 6     3/22/99 8:22p Mpanas
// Drive Monitor PTS support code changes
// 
// 09/16/98 Michael G. Panas: Create file
// 02/12/99 Michael G. Panas: convert to new Oos model
// 03/04/99 Michael G. Panas: Add Table Service support, move config
//                            to DmConfig.h
/*************************************************************************/


#ifndef __DriveMonitorIsm_h
#define __DriveMonitorIsm_h

#include "Ddm.h"			// include base class
#include "DmConfig.h"

// Tables referenced
#include "StorageRollCallTable.h"
#include "DiskDescriptor.h"
#include "PathDescriptor.h"
#include "DeviceDescriptor.h"

#include "LoopDescriptor.h"
#include "VirtualDeviceTable.h"
#include "BsaConfig.h"

#include "Slist.h"			// includes Container.h (from SSAPI Util)

// DriveMonitor Device state structure
// Keyed internally by FC ID/LUN
struct DM_DEVICE_STATE {
	U32						 state;		// Entry state
	U32						 type;		// device type
	U32						 num_LUNs;	// number of LUNs found on this ID
	PathDescriptor			*pPD;		// PathDescriptor record
	DiskDescriptor			*pDD;		// DiskDescriptor record
	StorageRollCallRecord	*pRC;		// StorageRollCall record
	DeviceDescriptor		*pDevice;	// misc device records
	rowID					 ridPD;		// row ID of the PathDescriptor entry
	rowID					 ridDD;		// row ID of the DiskDescriptor entry
	rowID					 ridRC;		// row ID of the RollCall entry
	rowID					 rid;		// row ID of the Device Descriptor entry
	VDN						 Vdn;		// Bsa/Es VirtualDevice
};

#define	DEVICE_STATE_ADD_DD			0x01	// need to add the Descriptor to PTS
#define	DEVICE_STATE_ADD_RC			0x02	// need to add the RollCall to PTS
#define	DEVICE_STATE_READ_RC		0x04	// need to read the RollCall from the PTS
#define	DEVICE_STATE_ADD_PATH		0x08	// need to add the Path Descriptor to PTS
#define	DEVICE_STATE_DEVICE_FAILURE	0x10	// device failed
#define	DEVICE_STATE_VDN_FOUND		0x20	// pre-configed BSA or ES VDN found
#define	DEVICE_STATE_NEW_DEVICE		0x40	// a device is found (new)
#define	DEVICE_STATE_DEVICE_FOUND	0x80	// a device is found (old)
#define	DEVICE_MASK					0xC0	// any device found mask

struct DM_CONTEXT;
struct DMT_CONTEXT;

// define the derived class for the FCP Scsi Drive Monitor ISM
class DriveMonitorIsm: public Ddm {

public:

typedef STATUS (DriveMonitorIsm::*pDMCallback_t)(void *, STATUS);

	// Data recovered from the Persistent Data Service
	DM_CONFIG config;
	
	DriveMonitorIsm(DID did);
	VDN		MyVd;			// my virtual device number
	DID		MyDid;			// my DID

	static
	Ddm *Ctor(DID did);

	STATUS Initialize(Message *pMsg);
	STATUS FinishInitialize(Message *pMsg);
	STATUS Enable(Message *pMsg);
	STATUS Quiesce(Message *pMsg);
	
	// DmScan.cpp
	// member methods to handle scan
	void	*DM_Build_Plex_Request(Message *pMsg, U32 start, U32 end, U16 flags);
	void	DM_Do_Plex_Request(DM_CONTEXT *pDmc);
	void	DM_Complete_Plex_Request(void *p, U32 status);
	void	DM_Do_Plex_Request_2(DM_CONTEXT *pDmc);
	void	DM_Complete_Plex_Request_2(void *p, U32 status);
	void	DM_Handle_Plex_Timeout(void *p, U32 status);
	void	DM_Do_Finish(DM_CONTEXT *pDmc);
	void	DM_Removed_Drive_Callback(void *p, U32 status);

	void	DM_Scan_All_Devices(Message *pMsg);
	void	DM_Scan_One_Device(Message *pMsg);
	void	DM_Do_Scan_8_Luns(DM_CONTEXT *pDmc);
	
	void	DM_Do_Inquiry(DM_CONTEXT *pDmc);
	STATUS 	DM_Do_Inquiry_Callback(Message *pMsg);
	
	void	DM_Do_Report_Luns(DM_CONTEXT *pDmc);
	STATUS 	DM_Do_Report_Luns_Callback(Message *pMsg);
	
	// DmDisks.cpp
	// member methods to handle disk spinup, read cap, and read SN
	U32		DM_Check_DDH_Slot(void);
	void	DM_Do_Disk(DM_CONTEXT *pDmc);
	void	DM_Do_Inquiry_Serial_Number(DM_CONTEXT *pDmc);
	void	DM_Do_Test_Unit(DM_CONTEXT *pDmc);
	void	DM_Do_Start_Unit(DM_CONTEXT *pDmc);
	void	DM_Do_Read_Capacity(DM_CONTEXT *pDmc);
	void	DM_Do_Test_Unit_Wait(DM_CONTEXT *pDmc);
	
	STATUS 	DM_Do_Inquiry_Serial_Number_Callback(Message *pMsg);
	STATUS 	DM_Do_Test_Unit_Callback(Message *pMsg);
	STATUS 	DM_Do_Start_Unit_Callback(Message *pMsg);
	STATUS 	DM_Do_Start_Unit_Timer_Callback(Message *pMsg);
	STATUS 	DM_Do_Read_Capacity_Callback(Message *pMsg);
	STATUS 	DM_Do_Test_Unit_Wait_Callback(Message *pMsg);
	STATUS 	DM_Do_Test_Unit_Timer_Callback(Message *pMsg);
	
	void	DM_Do_Update_PTS(DM_CONTEXT *pDmc);
	void	DM_Do_Update_PTS_1(DM_CONTEXT *pDmc);
	void	DM_Do_Update_PTS_2(DM_CONTEXT *pDmc);

	void	DM_Start_One_Disk(Message *pMsg);
	void	DM_Stop_One_Disk(Message *pMsg);
	void	DM_Stop_All_Disks(Message *pMsg);
	void	DM_Stop_All_Disks_Callback(void *p, U32 status);
	
	void	DM_Do_Start_Stop_One_Disk(DM_CONTEXT *pDmc);
	STATUS 	DM_Do_Start_Stop_One_Unit_Callback(Message *pMsg);
	STATUS 	DM_Do_Start_One_Unit_Timer_Callback(Message *pMsg);
	
	void	DM_Do_Finish_Msg(void *p, U32 status);

	// DmTapes.cpp
	void	DM_Do_Tape(DM_CONTEXT *pDmc);
	void	DM_Do_Update_Tape_PTS(DM_CONTEXT *pDmc);
	void	DM_Do_Update_Tape_PTS_1(DM_CONTEXT *pDmc);
	void	DM_Do_Update_Tape_PTS_2(DM_CONTEXT *pDmc);

	// DmSES.cpp
	void	DM_Do_SES(DM_CONTEXT *pDmc);
	void	DM_Do_Update_SES_PTS(DM_CONTEXT *pDmc);
	void	DM_Do_Update_SES_PTS_1(DM_CONTEXT *pDmc);
	void	DM_Do_Update_SES_PTS_2(DM_CONTEXT *pDmc);

	// DmOther.cpp
	void	DM_Do_Other(DM_CONTEXT *pDmc);

	// DmSCSI.cpp
	void	DM_Build_SCSI_Message(Message *pMsg, SCSI_COMMANDS Cmd,
				void *pData, long length, U32 drive_number, U32 lun_number);
	void	DM_Display_Sense(Message *pMsg);

	// DmBsa.cpp
	void	DM_Create_Bsa_Device(void *p,
								DM_DEVICE_STATE *pDMState,
								pDMCallback_t Callback);
	STATUS	DM_Create_InsertBSACfg(void *pClientContext, STATUS status);
	STATUS	DM_Create_InstBSAVD(void *pClientContext, STATUS status);
	STATUS	DM_Create_InstBSAVDReply(Message *pMsg);
	//STATUS	DM_Create_Listen4VD(void *pClientContext, STATUS status);
	//STATUS	DM_Create_VDRListenReply(void *pClientContext, STATUS status);
	U32		DM_Find_Bsa_Device(DM_DEVICE_STATE *pDMState);
	STATUS 	DM_Create_Bsa_End(void *pClientContext, STATUS status);

	// DmTest.cpp
	// member methods for testing
	void	DM_Read_Drive(DMT_CONTEXT *pDmc);
	void	DM_Write_Drive_Block(DMT_CONTEXT *pDmc,
				U32 logical_block_address, U32 block_count);
	void	DM_Read_Drive_Block(DMT_CONTEXT *pDmc,
				U32 logical_block_address, U32 block_count);
	STATUS	Verify_Read(DMT_CONTEXT *pDmc);

	STATUS 	DM_Read_Drive_Callback(Message *pMsg);
	STATUS 	DM_Write_Drive_Block_Callback(Message *pMsg);
	STATUS 	DM_Read_Drive_Block_Callback(Message *pMsg);

	// DmTableInit.cpp
	// Initial Table methods for the drive monitor
	STATUS	DmTableInitialize(Message *pMsg);
	
	STATUS 	DmTblDoRollCall(void *pClientContext, STATUS status);
	
	STATUS 	DmTblDoVDT(void *pClientContext, STATUS status);
	STATUS 	DmTblVdtReply1(void *pClientContext, STATUS status);

	STATUS 	DmTblDoBsaConfig(void *pClientContext, STATUS status);
	STATUS 	DmTblBsaReply1(void *pClientContext, STATUS status);
	STATUS 	DmTblBsaReply2(void *pClientContext, STATUS status);
	STATUS 	DmTblBsaReply3(void *pClientContext, STATUS status);

	STATUS 	DmTblDoPathDescriptor(void *pClientContext, STATUS status);
	STATUS 	DmTblPDReply1(void *pClientContext, STATUS status);
	STATUS 	DmTblPDReply2(void *pClientContext, STATUS status);
	STATUS 	DmTblPDReply3(void *pClientContext, STATUS status);
	
	STATUS 	DmTblDoDescriptors(void *pClientContext, STATUS status);
	STATUS 	DmTblDoDiskDescriptor(void *pClientContext, STATUS status);
	STATUS 	DmTblDDReply1(void *pClientContext, STATUS status);
	STATUS 	DmTblDDReply2(void *pClientContext, STATUS status);
	STATUS 	DmTblDDReply2a(void *pClientContext, STATUS status);
	STATUS 	DmTblDDReply3(void *pClientContext, STATUS status);
	
	STATUS 	DmTblDoLoopDesc(void *pClientContext, STATUS status);
	STATUS 	DmTblLpDescReply1(void *pClientContext, STATUS status);
	STATUS 	DmTblLpDescReply2(void *pClientContext, STATUS status);
	STATUS 	DmTblReplyListenCallback(void *pClientContext, STATUS status);

	STATUS 	DmTblReplyLast(void *pClientContext, STATUS status);
	
	// DmTables.cpp
	U32		DM_Check_PTS_Update_OK(void);

	// Update PathDescriptor table entry
	STATUS	DmTableUpdatePD(DM_CONTEXT *pDmc, pDMCallback_t Callback);
	STATUS	DmTblUpdReadPDDesc(void *pClientContext, STATUS status);
	STATUS	DmTblUpdAddPDDesc(void *pClientContext, STATUS status);
	STATUS 	DmTblUpdModifyPDDesc(void *pClientContext, STATUS status);
	STATUS 	DmTableUpdatePDEnd(void *pClientContext, STATUS status);
	
	// Update DiskDescriptor table entry
	STATUS	DmTableUpdateDD(DM_CONTEXT *pDmc, pDMCallback_t Callback);
	STATUS	DmTblUpdReadDesc(void *pClientContext, STATUS status);
	STATUS	DmTblUpdAddDesc(void *pClientContext, STATUS status);
	STATUS 	DmTblUpdModifyDesc(void *pClientContext, STATUS status);
	STATUS 	DmTableUpdateDDEnd(void *pClientContext, STATUS status);
	
	// Update DeviceDescriptor table entry
	STATUS	DmTableUpdateDeviceDesc(DM_CONTEXT *pDmc, pDMCallback_t Callback);
	STATUS	DmTblUpdReadDevDesc(void *pClientContext, STATUS status);
	STATUS	DmTblUpdAddDevDesc(void *pClientContext, STATUS status);
	STATUS 	DmTblUpdModifyDevDesc(void *pClientContext, STATUS status);
	STATUS 	DmTableUpdateDevDEnd(void *pClientContext, STATUS status);
	
	// Update StorageRollcall table entry
	STATUS	DmTableUpdateRC(DM_CONTEXT *pDmc, pDMCallback_t Callback);
	STATUS 	DmTblUpdReadRollCall(void *pClientContext, STATUS status);
	STATUS	DmTblUpdAddRollCall(void *pClientContext, STATUS status);
	STATUS 	DmTblUpdModifyRollCall(void *pClientContext, STATUS status);
	STATUS 	DmTableUpdateRCEnd(void *pClientContext, STATUS status);
	
	// Find descriptor
	STATUS	DmFindDescriptor(DM_CONTEXT *pDmc, pDMCallback_t Callback);
	STATUS 	DmFindDiskDescriptor(void *pClientContext, STATUS status);
	STATUS	DmFindDiskDescriptor1(void *pClientContext, STATUS status);
	STATUS 	DmFindDeviceDescriptor(void *pClientContext, STATUS status);
	STATUS	DmFindDeviceDescriptor1(void *pClientContext, STATUS status);
	STATUS 	DmFindPathDescriptors(void *pClientContext, STATUS status);
	STATUS 	DmFindPathDescriptors1(void *pClientContext, STATUS status);
	STATUS 	DmFindDescriptorEnd(void *pClientContext, STATUS status);
	
	// DmTableListen.cpp
	STATUS	DMStartListens(void );
	STATUS	DmRCListenUpdate(void *pClientContext, STATUS status);


	// PTS Table Data
	PathDescriptor		*DM_TS_Path_Desc;	// temp data
	DiskDescriptor		*DM_TS_Disk_Desc;	// temp data
	DeviceDescriptor	*DM_TS_Device_Desc;	// temp data
	
	LoopDescriptorEntry	*DM_Loop_Desc;	// to save our LoopDescriptor
	
	Container			*pDD;			// Containers for all devices

	U32		DM_Scan_In_Progress;		// Only one at a time allowed
	U32		DM_Drives;					// num of drives scanned
	U32		DM_Good_Drives;				// num of good drives
	U32		DM_Bad_Drives;				// num of bad drives
	U32		DM_Removed_Drives;			// num of removed drives
	U32		DM_Drive_Test[20];			// drive testing flags

protected:
	STATUS DoWork(Message *pMsg);

private:
	U32					m_fHaveDDHs;
	U32					m_nPaths;
	U32					m_next;

	TSDefineTable*		m_pDefineTable;
	TSGetTableDef*		m_pTSGetTableDef;
	TSInsertRow*		m_pInsertRow;
	TSReadRow*			m_pSRCReadRow;
	TSEnumTable*		m_EnumTable;
	TSModifyRow*		m_ModifyRow;
	TSReadTable*		m_ReadTable;
	
	rowID				m_RowID1;
	rowID				m_RowID2;
	U32					m_Index1;
	U32					m_Index2;
	U32					m_nTableRows;
	U32					m_nVdtTableRows;
	U32					m_nBsaTableRows;
	U32					m_nTableRowsRead;
	U32					m_numBytesEnumed;

	// Listen on StorageRollCall 
	TSListen*			m_Listen;
	U32	*				m_LType;		// Listen Type return
	U32					m_LSize;		// Size of returned row
	LoopDescriptorEntry		*m_Loop_Desc;	// LoopDescriptor listen reply data
	StorageRollCallRecord	*m_Roll_Call;

	// BSA VD Create members
	TSListen*						m_pListen4VD;
	U32								m_ListenerID;
	U32*							m_pListenReplyType;
	VirtualDeviceRecord				m_NewVDTRecord;
	//VCCreateResult				m_VCCreateResult;
	BSA_CONFIG						m_BSAConfigRec;
	VirtualDeviceRecord				*pVdt;
	BSA_CONFIG						*pBSAConfig;
};


// Drive Monitor internal context
// used for the life of a plex
struct DM_CONTEXT {
	U16				flags;				// plex flags
	U8				drive_number;		// current drive ID number
	U8				last_drive;			// last disk drive ID number
	U16				lun_number;			// current drive LUN number
	U16				last_lun;			// last disk drive LUN number
	U16				start;				// starting ID/LUN number
	U16				last;				// ending ID/LUN number
	U32				num_devices;		// devices to scan in this plex
	U32				num_good;			// good devices found
	U32				num_bad;			// bad devices found
	U32				retries;			// number of retries allowed/left
	U32				index;				// table index
	U32				*pluns;				// pointer to list of LUNs from ReportLuns
	void			*buffer;			// pointer to buffer (if used)
	Message			*pMsg;				// pointer to message for this device
	Message			*pReply;			// non-zero if we need a reply
	DriveMonitorIsm::pDMCallback_t	 Callback;	// saved Callback address
	DM_DEVICE_STATE	*pDMState;			// saved device state container
	DM_CONTEXT		*pDmc;				// original plex request
};

// plex flags
#define	PLEX_UPDATE			0x01		// update totals when done
#define	PLEX_FOUND_DISK		0x02		// found a disk device (last_drive & last_lun valid)
#define	PLEX_LUN_REQUEST	0x04		// scanning LUNs in this plex only
#define	PLEX_REPLY_NEEDED	0x08		// always reply when this plex done

// Drive Monitor internal context for test modes
struct DMT_CONTEXT {
	U32				drive_number;		// current drive ID number
	U32				index;				// table index
	U32				start;				// starting ID/LUN number
	U32				last;				// ending ID/LUN number
	U32				retries;			// number of retries allowed/left
	VDN				Vd;					// virtual device number for this device
	void			*buffer;			// pointer to buffer (if used)
	Message			*pMsg;				// pointer to message for this device
	Message			*pReply;			// non-zero if we need a reply
	U32 			logical_block_address; // Used by Ext_Sts_Read_Verify_Test
	U32 			block_count; 		// Used by Ext_Sts_Read_Verify_Test
};


#endif // __DriveMonitorIsm_h