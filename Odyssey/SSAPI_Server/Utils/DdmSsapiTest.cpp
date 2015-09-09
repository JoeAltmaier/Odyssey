//******************************************************************************
// FILE:		DdmSsapiTest.cpp
//
// PURPOSE:		Implements DDM-based object for testing/debugging purposes
//******************************************************************************



#include "Odyssey_Trace.h"
#include "DdmSsapiTest.h"
#include "Message.h"
#include "Buildsys.h"
#include "SCSI.h"
#include "PathDEscriptor.h"
#include "Devicedescriptor.h"
#include "ShadowTable.h"
#include "UserAccessTable.h"
#include "UnicodeString.h"
#include "ListenManager.h"
#include "User.h"
#include "SSAPIEvents.h"
#include "UnicodeString.h"
#include "SsapiLocalResponder.h"
#include "EvcStatusRecord.h"
#include "IopStatusTable.h"
#include "CtTypes.h"
#include "STSData.h"
#include "DiskDescriptor.h"
#include "SSAPITypes.h"
#include "IopTypes.h"
#include "deviceId.h"
#include "address.h"
#include "odyssey.h"
#include "DiskStatusTable.h"
#include "DiskPerformanceTable.h"
#include "LoopDescriptor.h"
#include "HostDescriptorTable.h"
#include "HostConnectionDescriptorTable.h"
#include "SystemConfigSettingsTable.h"
#include "FCPortDatabaseTable.h"
#include "ExportTable.h"
#include "ExportTableUserInfoTable.h"
#include "StorageRollCallTable.h"
#include "RaidMemberTable.h"
#include "ArrayDescriptor.h"
#include "RaidSpareDescriptor.h"
#include "..\..\MSL\OsHeap.h"
#include "Event.h"
#include "virtualDeviceTable.h"
#include "imghdr.h"
#ifdef TRACE_INDEX
#undef TRACE_INDEX
#include "UpgradeMasterMessages.h"
#endif
#include "SsdDescriptor.h"
#include "SSDStatusTable.h"
#include "SSDPerformanceTable.h"
#include "ArrayStatusTable.h"
#include "ArrayPerformanceTable.h"
#include "STSPerfTable.h"
#include "STSStatTable.h"

#include "Trace_Index.h"
#ifdef TRACE_INDEX
#undef TRACE_INDEX
#endif
#define TRACE_INDEX TRACE_SSAPI_MANAGERS

CLASSNAME(DdmSsapiTest, SINGLE);

RowId imageOneKey;
void* pImageOne;
U32 cbImageOne = 1024;
RowId imageTwoKey;
void* pImageTwo;
U32 cbImageTwo = 2056;

#define		EVC_TABLE				0x00000001
#define		IOP_TABLE				0x00000002
#define		DISK_TABLE				0x00000004
#define		DISK_STATUS_TABLE_		0x00000008
#define		DISK_PERF_TABLE			0x00000010
#define		LOOP_TABLE				0x00000020
#define		HOST_TABLE				0x00000040
#define		HOST_CONN_TABLE			0x00000080
#define		SYS_CONFIG_TABLE		0x00000100
#define		HBA_TABLE				0x00000200
#define		PORT_TABLE				0x00000400
#define		EXPORT_TABLE_			0x00000800
#define		EXPORT_USER_INFO_TABLE	0x00001000
#define		SRC_TABLE				0x00002000
#define		MEMBER_TABLE			0x00004000
#define		SPARE_TABLE				0x00008000
#define		ARRAY_TABLE				0x00010000
#define		PATH_TABLE				0x00020000
#define		DEVICE_TABLE			0x00040000
#define		SSD_TABLE				0x00080000
#define		SSDP_TABLE				0x00100000
#define		SSDS_TABLE				0x00200000
#define		ARRP_TABLE				0x00400000
#define		ARRS_TABLE				0x00800000
#define		STSS_TABLE				0x01000000
#define		STSP_TABLE				0x02000000
#define		ID_TABLE				0x04000000


#define LAST_TABLE	EXPORT_TABLE_

//#define	TEST_RAMBO 1
DdmSsapiTest *pSsapiTestDdm;

#ifdef TEST_RAMBO
ShadowTable *m_pSesTable;
#endif

//******************************************************************************
// ARRAY status & performance tables;
//******************************************************************************
int arrS = sizeof(ArrayPerformanceRecord);
ArrayPerformanceRecord raidPTable[] = {
{{0,0,0,},1, arrS }
};

int arrSS = sizeof(ArrayStatusRecord);
ArrayStatusRecord raidSTable[] = {
{{0,0,0,},1, arrSS }
};

//******************************************************************************
// LUN(STS) status & performance tables;
//******************************************************************************
int stsSS = sizeof(STSStatRecord);
STSStatRecord stsSTable[] = {
{{0,0,0,},1, stsSS, 5, 5, {0,0,0}, 12, 234, 231, 3, 0 }
};

int stsPS = sizeof(STSPerfRecord);
STSPerfRecord stsPTable[] = {
{{0,0,0,},1, stsPS, 3, 2, 1, { 0,0,0}, 12, 123, 1, 3, 2321, 12, 4,87,0 }
};


//******************************************************************************
// Path descriptor
//******************************************************************************
int phD = sizeof(PathDescriptor);
PathDescriptor pathTable[] = {
// For Disks
{{0,0,0,},PATH_DESC_VERSION,phD,0,7,0,0,DriveReady,0,0,{11,0,1},{11,0,1}},
{{0,0,0,},PATH_DESC_VERSION,phD,1,12,0,0,DriveReady,0,0,{11,0,2},{11,0,2}},
{{0,0,0,},PATH_DESC_VERSION,phD,2,666,1,0,DriveReady,0,0,{11,0,2}},
{{0,0,0,},PATH_DESC_VERSION,phD,1,1,0,0,DriveReady,0,0,{11,0,3},{11,0,3}},
{{0,0,0,},PATH_DESC_VERSION,phD,2,999,2,0,DriveReady,0,0,{11,0,3}},
{{0,0,0,},PATH_DESC_VERSION,phD,0,19,0,0,DriveReady,0,0,{11,0,4},{11,0,4}},
// for Tape & Ses
{{0,0,0,},PATH_DESC_VERSION,phD,0,45,0,0,DriveReady,0,0,{23,0,1},{23,0,1}},
{{0,0,0,},PATH_DESC_VERSION,phD,1,46,3,0,DriveReady,0,0,{23,0,1},},
{{0,0,0,},PATH_DESC_VERSION,phD,2,4,5,0,DriveReady,0,0,{23,0,2},{23,0,2}},
};

//******************************************************************************
// Device Descriptor
//******************************************************************************

int dvD = sizeof(DeviceDescriptor);
DeviceDescriptor deviceTable[] = {
{{0,0,0},DEVICE_DESC_VERSION,dvD,0,5,1,"ConnerSES01", "asders",DriveReady, SCSI_DEVICE_TYPE_ENCLOSURE_SERVICES },
{{0,0,0},DEVICE_DESC_VERSION,dvD,0,5,1,"SuperTape02", "asders",DriveReady, SCSI_DEVICE_TYPE_SEQUENTIAL }
};

//******************************************************************************
// SSD Descriptor
//******************************************************************************
int ssdS = sizeof(SSD_Descriptor);
SSD_Descriptor ssdTable[] = {
{{0,0,0,},SSD_DESC_VERSION, ssdS, 10000, 0, "SSD v1.0", {10,0,3}, 0 }
};


//******************************************************************************
// SSD status table
//******************************************************************************
int ssdsS = sizeof(SSDStatusRecord);
SSDStatusRecord ssdSTable[] = {
{{0,0,0,},CT_SSDST_TABLE_VERSION, ssdsS, 0,5,{0,0,0},123,23414,12 }
};


//******************************************************************************
// SSD performance table
//******************************************************************************
int ssdpS = sizeof(SSDPerformanceRecord);
SSDPerformanceRecord ssdPTable[] = {
{{0,0,0,},CT_SSDPT_TABLE_VERSION, ssdpS,0,5,7,{0,0,0},89000,5,8,3,9,67,1,3,5,1,15,18,13,19,167,11,13,15,11,0,0,0,12,13,11,45,46,2 }
};


//******************************************************************************
// Export User Info Table
//******************************************************************************
int euiS = sizeof(ExportTableUserInfoRecord);
ExportTableUserInfoRecord userInfoTable[] = {
{{0,0,0,},EXPORT_TABLE_USER_INFO_TABLE_VERSION, euiS,{34,0,1,},{24,0,1,}},
};

//******************************************************************************
// Export Table
//******************************************************************************
int etS = sizeof(ExportTableEntry);
ExportTableEntry exportTable[] = {
{{0,0,0,},EXPORT_TABLE_VERSION, etS,{0,0,0,}, ProtocolFibreChannel,5,50,0,0, 24, 23, 12,"ser#",12000000000, 0,StateConfiguredAndActive,StateConfiguredAndActive,"wwname",{18,0,1,},{13,0,1},{0,0,0,},{0,0,0,},{0,0,0,}, {17,0,1}, EXPORT_ENTRY_ATTRIBUTE_CACHED },
};

//******************************************************************************
// RAID Descriptor Table
//******************************************************************************
RAID_ARRAY_DESCRIPTOR arrayTable[] = {
{{0,0,0,},0, sizeof(RAID_ARRAY_DESCRIPTOR),0,{13,0,5,},100,100,5,5,RAID1,RAID_FAULT_TOLERANT,RAID_INIT_COMPLETE,(RAID_PECKING_ORDER)1,2,0,1},
	
};


//******************************************************************************
// RaidSpare Table
//******************************************************************************
RAID_SPARE_DESCRIPTOR spareTable[] = {
{{0,0,0,},0, sizeof(RAID_SPARE_DESCRIPTOR),RAID_DEDICATED_SPARE,{13,0,2,},{17,0,1,},{0,0,0,},100000,0,0},
{{0,0,0,},0, sizeof(RAID_SPARE_DESCRIPTOR),RAID_GENERAL_POOL_SPARE,{13,0,4,},{0,0,0,},{0,0,0,},150000,0,0},
};

//******************************************************************************
// Raid Member Table
//******************************************************************************
int rmtS = sizeof(RAID_ARRAY_MEMBER);
RAID_ARRAY_MEMBER memberTable[] = {
{{0,0,0,},0,rmtS,{17,0,1,},{13,0,1,},0, RAID_STATUS_UP, 0,3,RAID_QUEUE_ELEVATOR,0,1000,5,0},
{{0,0,0,},0,rmtS,{17,0,1,},{13,0,3,},0, RAID_STATUS_UP, 1,3,RAID_QUEUE_ELEVATOR,0,1000,5,0},
};


//******************************************************************************
// Storage Roll Call table
//******************************************************************************
int srcS = sizeof(StorageRollCallRecord);
StorageRollCallRecord SRCTable[] = {
{{0,0,0,},STORAGE_ROLL_CALL_TABLE_VERSION, srcS, 12000000, 1, SRCTypeFCDisk, 50, {11,0,1,}, {7,0,1,}, {8,0,1,}, 0 },
{{0,0,0,},STORAGE_ROLL_CALL_TABLE_VERSION, srcS, 24000000, 0, SRCTypeFCDisk, 60, {11,0,2,}, {7,0,2,}, {8,0,2,}, 0 },
{{0,0,0,},STORAGE_ROLL_CALL_TABLE_VERSION, srcS, 24000000, 0, SRCTypeFCDisk, 61, {11,0,3,}, {0,0,0,}, {0,0,0,}, 0 },
{{0,0,0,},STORAGE_ROLL_CALL_TABLE_VERSION, srcS, 24000000, 0, SRCTypeFCDisk, 62, {11,0,4,}, {0,0,0,}, {0,0,0,}, 0 },
{{0,0,0,},STORAGE_ROLL_CALL_TABLE_VERSION, srcS, 100000, 0, SRCTypeSSD, 62, {25,0,1,}, {26,0,0,}, {27,0,0,}, 0 },

//{{0,0,0,},STORAGE_ROLL_CALL_TABLE_VERSION, srcS, 24000000, 0, SRCTypeArray, 68, {17,0,1,}, {0,0,0,}, {0,0,0,}, 0, {19, 0, 4} },
};


//******************************************************************************
// ExternalPortDescriptor
//******************************************************************************
int ppT = sizeof(FCPortDatabaseRecord);
FCPortDatabaseRecord portTable[] = {
{{0,0,0,},FC_PORT_DATABASE_TABLE_VERSION, ppT, {12,0,1,},1, "77sd2", FC_PORT_TYPE_INITIATOR,FC_PORT_STATUS_ACTIVE  },
{{0,0,0,},FC_PORT_DATABASE_TABLE_VERSION, ppT, {12,0,2,},12, "s3d2", FC_PORT_TYPE_INITIATOR ,FC_PORT_STATUS_ACTIVE  },
{{0,0,0,},FC_PORT_DATABASE_TABLE_VERSION, ppT, {12,0,3,},6, "sasd", FC_PORT_TYPE_INITIATOR ,FC_PORT_STATUS_ACTIVE  },
{{0,0,0,},FC_PORT_DATABASE_TABLE_VERSION, ppT, {12,0,4,},23, "3453", FC_PORT_TYPE_INITIATOR ,FC_PORT_STATUS_ACTIVE  },
{{0,0,0,},FC_PORT_DATABASE_TABLE_VERSION, ppT, {12,0,5,},99, "2134", FC_PORT_TYPE_INITIATOR ,FC_PORT_STATUS_ACTIVE  },
{{0,0,0,},FC_PORT_DATABASE_TABLE_VERSION, ppT, {12,0,1,},8, " sdfsd", FC_PORT_TYPE_INITIATOR ,FC_PORT_STATUS_ACTIVE  },
{{0,0,0,},FC_PORT_DATABASE_TABLE_VERSION, ppT, {12,0,2,},56, "sad34", FC_PORT_TYPE_TARGET ,FC_PORT_STATUS_ACTIVE  },
{{0,0,0,},FC_PORT_DATABASE_TABLE_VERSION, ppT, {12,0,3,},45, "545555", FC_PORT_TYPE_TARGET, FC_PORT_STATUS_LOOP_DOWN  },
};

//******************************************************************************
// HostConnectionTable
//******************************************************************************
int hctS = sizeof(HostConnectionDescriptorRecord);
HostConnectionDescriptorRecord connTable[] = {
{{0,0,0,},HOST_DESCRIPTOR_TABLE_VERSION, hctS, ePATH_TYPE_REDUNDANT,1,{{16,0,1,},}, {1,}, {19,0,2,},{20,0,2,},{14,0,1}, {34,0,3}},
//{{0,0,0,},HOST_DESCRIPTOR_TABLE_VERSION, hctS, ePATH_TYPE_REDUNDANT,1,{{12,0,2,},}, {1,}, {19,0,3,},{20,0,3,},{10,0,1}, },
};

//******************************************************************************
// HostDescriptorTable
//******************************************************************************
int htS = sizeof(HostDescriptorRecord);
HostDescriptorRecord hostTable1[] = {
{{0,0,0,},HOST_DESCRIPTOR_TABLE_VERSION, htS,"", "", 1, 0x00110022, 1, {16,0,1} },
{{0,0,0,},HOST_DESCRIPTOR_TABLE_VERSION, htS,"", "", 2, 0x00113300, 0, },
};


//******************************************************************************
// IOPStatusTable ---> put your inital IOPs here
//******************************************************************************
IOPStatusRecord		IOPTable[10]; // populated in the routine


//******************************************************************************
// DiskDescriptorTable ---> put your inital HDDs here
//
// NOTE:	row ids of the status and performance tables are inserted in the
//			PopulateDiskTable() method
//******************************************************************************
INQUIRY		inc;
U64			u64;
DiskDescriptor	DiskTable[]={	//DriveStatus			VDN 
{{0,0,0,},DISK_DESC_VERSION,sizeof(DiskDescriptor),0,1,"FC Disk0001","fgfd",DriveReady,TypeFCDisk,DRIVE_LOCKED,24000,0,0 },
{{0,0,0,},DISK_DESC_VERSION,sizeof(DiskDescriptor),6,1,"FC Disk0002","gnbvn",DriveReady,TypeFCDisk,DRIVE_LOCKED,12000,0,0 },
{{0,0,0,},DISK_DESC_VERSION,sizeof(DiskDescriptor),1,1,"FC Disk0003","tryrty",DriveReady,TypeFCDisk,DRIVE_LOCKED,24000,0,0 },
{{0,0,0,},DISK_DESC_VERSION,sizeof(DiskDescriptor),2,1,"FC Disk0004","rttrer",DriveReady,TypeFCDisk,DRIVE_LOCKED,24000,0,0 },
};

//******************************************************************************
// DiskStatusTable ---> put your inital disk PHS data here
//******************************************************************************
DiskStatusRecord _DiskStatusTable[] = {
{{0,0,0,},1,sizeof(DiskStatusRecord),0,777,{0,0,0,},2,4,6,7,8,9,1 },
{{0,0,0,},1,sizeof(DiskStatusRecord),0,777,{0,0,0,},8,5,3,7,8,9,0 },
{{0,0,0,},1,sizeof(DiskStatusRecord),0,777,{0,0,0,},2,4,6,7,8,9,8 },
{{0,0,0,},1,sizeof(DiskStatusRecord),0,777,{0,0,0,},0,1,6,5,8,3,5 },
};

//******************************************************************************
// DiskStatusTable ---> put your inital disk PHS data here
//******************************************************************************
int dprS = sizeof(DiskPerformanceRecord);
DiskPerformanceRecord DiskPerfTable[] = {
{{0,0,0,},1,dprS,0,67456,888,0,{0,0,0},2384234,8,6,32,5,32,5,5,234,8,0,34,5,6,3,2,78,45,234,213,567,34,23,234,5,0,1,2,},
{{0,0,0,},1,dprS,0,67456,888,0,{0,0,0},456,8,6,32,5,32,5,5,234,8,0,34,5,6,3,2,78,45,234,213,567,34,23,234,5,0,1,2,},
{{0,0,0,},1,dprS,0,67456,888,0,{0,0,0},24534,8,6,32,5,32,5,5,234,8,0,34,5,6,3,2,78,45,234,213,567,34,23,234,5,0,1,2,},
{{0,0,0,},1,dprS,0,67456,888,0,{0,0,0},9999,8,6,32,5,32,5,5,234,8,0,34,5,6,3,2,78,45,234,213,567,34,23,234,5,0,1,2,}
};


//******************************************************************************
// LoopDescriptor Table (FcPort Table) ---> put your ports here
//******************************************************************************
int ldsS = sizeof(LoopDescriptorEntry);
LoopDescriptorEntry LoopTable[] = { // LoopState
{{0,0,0,},LOOP_DESCRIPTOR_TABLE_VERSION,ldsS,0,IOP_RAC0,222,0,0,0,IOPTY_NAC,-1,LoopUp,LoopUp,0,{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
{{0,0,0,},LOOP_DESCRIPTOR_TABLE_VERSION,ldsS,1,IOP_RAC0,555,0,0,0,IOPTY_NAC,-1,LoopUp,LoopUp,0,{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
{{0,0,0,},LOOP_DESCRIPTOR_TABLE_VERSION,ldsS,2,IOP_RAC0,222,0,0,0,IOPTY_NAC,-1,LoopUp,LoopUp,0,{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
{{0,0,0,},LOOP_DESCRIPTOR_TABLE_VERSION,ldsS,0,IOP_RAC1,333,0,0,0,IOPTY_NAC,-1,LoopUp,LoopUp,0,{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
{{0,0,0,},LOOP_DESCRIPTOR_TABLE_VERSION,ldsS,1,IOP_RAC1,555,0,0,0,IOPTY_NAC,-1,LoopUp,LoopUp,0,{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
{{0,0,0,},LOOP_DESCRIPTOR_TABLE_VERSION,ldsS,2,IOP_RAC1,777,0,0,0,IOPTY_NAC,-1,LoopUp,LoopUp,0,{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
};


PERIODICLY_CALLED_METHOD	METHODS[] = {
		
//		METHOD_ADDRESS( DdmSsapiTest, ModifyLoopTable ),
//		METHOD_ADDRESS( DdmSsapiTest, ModifyEvcTable ),
//		METHOD_ADDRESS( DdmSsapiTest, ModifyIopTable ),
		METHOD_ADDRESS( DdmSsapiTest, ModifyDiskPerfTable ),
};
#define		TIMEOUT			5		// in seconds
//#define		LOG_EVENTS


//******************************************************************************
// ~DdmSsapiTest:
//
// PURPOSE:		class destructor
//******************************************************************************

DdmSsapiTest::~DdmSsapiTest(){
}


//******************************************************************************
// DdmSsapiTest:
//
// PURPOSE:		Default constructor
//******************************************************************************

DdmSsapiTest::DdmSsapiTest( DID did )
			:DdmMaster( did ){

	m_stringResources = 0;

	pSsapiTestDdm = this;



}


//******************************************************************************
// Initialize:
//
// PURPOSE:		DDM-model complience + possible init stuff
//******************************************************************************

STATUS 
DdmSsapiTest::Initialize( Message *pMsg ){


#ifdef LOG_EVENTS
	U32			i;

	Event		*pEvent = new Event(ELOG_TEST_INFO2);
	LogEvent(ELOG_TEST_INFO1, "An arg");
	LogEvent(ELOG_TEST_INFO2);
	LogEvent(ELOG_TEST_WARNING1, 5);
	LogEvent(ELOG_TEST_WARNING2);
	LogEvent(ELOG_TEST_ERROR1, 12.2);
	LogEvent(ELOG_TEST_ERROR2);
	LogEvent(ELOG_TEST_INTERNAL1, "Another Arg");
	LogEvent(ELOG_TEST_INTERNAL2);

	for( i = 0; i < 60; i++ )
		LogEvent(ELOG_TEST_WARNING1, i);

	delete pEvent;
#endif  /* #ifdef LOG_EVENTS */
	StartPopulatingUserTable();
	UserTableIsDone();
	return Ddm::Initialize(pMsg);
}


//******************************************************************************
// the following functions are responsible for populating the UserTable
//******************************************************************************

STATUS DdmSsapiTest::StartPopulatingUserTable(){
	pUserTable = new ShadowTable(	USER_ACCESS_TABLE_NAME,
									this,
									(ShadowTable::SHADOW_LISTEN_CALLBACK)&DdmSsapiTest::ListenCallbackHandler,
									(ShadowTable::SHADOW_LISTEN_CALLBACK)&DdmSsapiTest::ListenCallbackHandler,
									(ShadowTable::SHADOW_LISTEN_CALLBACK)&DdmSsapiTest::ListenCallbackHandler,
									sizeof(UserAccessTableEntry));
	
	return pUserTable->DefineTable( userAccessTableDefintion, sizeOfUserAccessTableEntry, 10, (pTSCallback_t)&DdmSsapiTest::TableDefined, NULL );
}

STATUS 
DdmSsapiTest::TableDefined( void *pContext, STATUS rc ){

	pUserTable->Initialize( (pTSCallback_t)&DdmSsapiTest::TableInitialized, NULL );
	return OK;
}


STATUS 
DdmSsapiTest::TableInitialized( void *pContext, STATUS rc ){

	UserAccessTableEntry	user;
	rowID *p;
	UnicodeString	us;

	memset( &user, 0, sizeof( user ) );
	user.version = USER_ACCESS_TABLE_VERSION;
	user.size = sizeof(user);
	us = UnicodeString((StringClass)"rambo"); us.CString( user.userName, 1024 );
	us = UnicodeString((StringClass)"rambo");		us.CString( user.password, 1024 );
	us = UnicodeString((StringClass)"Rambo-II");		us.CString( user.firstName, 1024 );
	us = UnicodeString((StringClass)"");		us.CString( user.lastName, 1024 );
	us = UnicodeString((StringClass)"Default account for Mr. Rambo");		us.CString( user.description, 1024 );
	us = UnicodeString((StringClass)"");	us.CString( user.email, 1024 );
	us = UnicodeString((StringClass)"");	us.CString( user.phoneNumber1, 1024 );
	us = UnicodeString((StringClass)"Parks' S/W");	us.CString( user.department, 1024 );

	p	= new rowID;
	pUserTable->InsertRow( &user, p, (pTSCallback_t)&DdmSsapiTest::RowInserted, p );
	memset( &user, 0, sizeof( user ) );
	user.version = USER_ACCESS_TABLE_VERSION;
	user.size = sizeof(user);
	us = UnicodeString((StringClass)"ct"); us.CString( user.userName, 1024 );
	us = UnicodeString((StringClass)"ct");		us.CString( user.password, 1024 );
	us = UnicodeString((StringClass)"Default");		us.CString( user.firstName, 1024 );
	us = UnicodeString((StringClass)"User");		us.CString( user.lastName, 1024 );
	us = UnicodeString((StringClass)"A default account");		us.CString( user.description, 1024 );
	us = UnicodeString((StringClass)"ct@dell.com");	us.CString( user.email, 1024 );
	us = UnicodeString((StringClass)"(800)444-2322");	us.CString( user.phoneNumber1, 1024 );
	us = UnicodeString((StringClass)"QA & Development");	us.CString( user.department, 1024 );

	p	= new rowID;
	pUserTable->InsertRow( &user, p, (pTSCallback_t)&DdmSsapiTest::RowInserted, p );
	
	return OK;
}

STATUS 
DdmSsapiTest::RowInserted( void *pContext, STATUS rc ){

	RowId	*pR	= (RowId *)pContext;
	
//	if( pR->LoPart == 2 )
//		UserTableIsDone();
	delete pContext;
	return OK;
}

//******************************************************************************
// ListenCallbackHandler:
//
// PURPOSE:		Hadnles listen replies from the PTs
//******************************************************************************

STATUS 
DdmSsapiTest::ListenCallbackHandler( void *pContext, U32 rc, ShadowTable *p ){
	

	if( rc == OK ){
		TRACE_STRING( TRACE_L2, ".");
	}


	if( EXPORT_USER_INFO_TABLE == (U32)pContext ){
		int i;
		i = 0;
	}
	
	return OK;
}

STATUS 
DdmSsapiTest::ListenCallbackHandler( void *pContext, STATUS rc ){

	return ListenCallbackHandler( pContext, (U32) rc, NULL );
}

STATUS 
DdmSsapiTest::ListenCallbackHandler1( void *pContext, STATUS rc ){

	if( rc == OK ){
		TRACE_STRING( TRACE_L2, ".");
	}
	else{
		TRACE_STRING( TRACE_L2, "PTS req failed!");
	}
	return OK;
}


//******************************************************************************
//******************************************************************************
void 
DdmSsapiTest::UserTableIsDone(){

	STATUS		status = OK;


	/*****************************
	/* 
	/* CREATE ALL SHADOW TABLES
	/*
	/*****************************/

	m_pIopStatusTable	= new ShadowTable(	CT_IOPST_TABLE_NAME,
											this,
											(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
											(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
											(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
											sizeof(IOPStatusRecord));

	m_pEvcStatusTable	= new ShadowTable(	EVC_STATUS_TABLE,
											this,
											(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
											(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
											(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
											sizeof(EVCStatusRecord));
	
	m_pDiskTable		= new ShadowTable(	DISK_DESC_TABLE,
											this,
											(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
											(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
											(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
											sizeof(DiskDescriptor));

	m_pDiskStatusTable	= new ShadowTable(	CT_DST_TABLE_NAME,
											this,
											(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
											(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
											(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
											sizeof(DiskStatusRecord));

	m_pDiskPerfTable	= new ShadowTable(	CT_DPT_TABLE_NAME,
											this,
											(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
											(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
											(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
											sizeof(DiskPerformanceRecord));

	m_pLoopTable		= new ShadowTable(	LOOP_DESCRIPTOR_TABLE,
											this,
											(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
											(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
											(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
											sizeof(LoopDescriptorEntry));
	m_pHostTable		= new ShadowTable(	HOST_DESCRIPTOR_TABLE_NAME,
											this,
											(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
											(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
											(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
											sizeof(HostDescriptorRecord));
	m_pHostConnTable1	= new ShadowTable(	HOST_CONNECTION_DESCRIPTOR_TABLE_NAME,
											this,
											(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
											(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
											(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
											sizeof(HostConnectionDescriptorRecord));

	m_pSysConfigTable	= new ShadowTable(	SYSTEM_CONFIG_SETTINGS_TABLE_NAME,
											this,
											(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
											(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
											(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
											sizeof(SystemConfigSettingsRecord));

	m_pExportTable		= new ShadowTable(	EXPORT_TABLE,
											this,
											(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
											(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
											(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
											sizeof(ExportTableEntry));
	
	m_pExportUserInfoTable= new ShadowTable(EXPORT_TABLE_USER_INFO_TABLE_NAME,
											this,
											(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
											(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
											(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
											sizeof(ExportTableUserInfoRecord));

	m_pSRCTable			= new ShadowTable(	STORAGE_ROLL_CALL_TABLE,
											this,
											(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
											(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
											(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
											sizeof(StorageRollCallRecord));


	m_pPortTable		= new ShadowTable(	FC_PORT_DATABASE_TABLE_NAME,
											this,
											(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
											(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
											(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
											sizeof(FCPortDatabaseRecord));

	m_pMemberTable		= new ShadowTable(	RAID_MEMBER_DESCRIPTOR_TABLE,
											this,
											(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
											(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
											(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
											sizeof(RAID_ARRAY_MEMBER));


	m_pArrayTable		= new ShadowTable(	RAID_ARRAY_DESCRIPTOR_TABLE,
											this,
											(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
											(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
											(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
											sizeof(RAID_ARRAY_DESCRIPTOR));

	m_pSpareTable		= new ShadowTable(	RAID_SPARE_DESCRIPTOR_TABLE,
											this,
											(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
											(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
											(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
											sizeof(RAID_SPARE_DESCRIPTOR));

	m_pDeviceTable		= new ShadowTable(	DEVICE_DESC_TABLE,
											this,
											(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
											(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
											(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
											sizeof(DeviceDescriptor));

	m_pPathTable		= new ShadowTable(	PATH_DESC_TABLE,
											this,
											(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
											(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
											(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
											sizeof(PathDescriptor));

	m_pSsdTable			= new ShadowTable(	SSD_DESCRIPTOR_TABLE,
											this,
											(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
											(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
											(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
											sizeof(SSD_Descriptor));
	m_pSsdSTable		= new ShadowTable(	CT_SSDST_TABLE,
											this,
											(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
											(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
											(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
											sizeof(SSDStatusRecord));
	m_pSsdPTable		= new ShadowTable(	CT_SSDPT_TABLE,
											this,
											(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
											(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
											(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
											sizeof(SSDPerformanceRecord));

	m_pRaidPTable		= new ShadowTable(	CT_ARRAYPT_TABLE,
											this,
											(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
											(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
											(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
											sizeof(ArrayPerformanceRecord));
	m_pRaidSTable		= new ShadowTable(	CT_ARRAYST_TABLE,
											this,
											(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
											(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
											(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
											sizeof(ArrayStatusRecord));
	m_pStsPTable		= new ShadowTable(	CT_STSPT_TABLE_NAME,
											this,
											(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
											(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
											(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
											sizeof(STSPerfRecord));
	m_pStsSTable		= new ShadowTable(	CT_STSST_TABLE_NAME,
											this,
											(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
											(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
											(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
											sizeof(STSStatRecord));

	m_pIdTable			= new ShadowTable(	STS_DATA_TABLE,
											this,
											(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
											(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
											(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
											sizeof(StsData));


	/*****************************
	/* 
	/* DEFINE ALL SHADOW TABLES
	/*
	/*****************************/
	status |= 	m_pDiskStatusTable->DefineTable((fieldDef *)aDiskStatusTable_FieldDefs,
												cbDiskStatusTable_FieldDefs,
												10,
												(pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
												(void *)DISK_STATUS_TABLE );

	status |= 	m_pDiskPerfTable->DefineTable(		(fieldDef *)aDiskPerformanceTable_FieldDefs,
												cbDiskPerformanceTable_FieldDefs,
												10,
												(pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
												(void *)DISK_PERF_TABLE );
	status |= m_pEvcStatusTable->DefineTable(	(fieldDef *)aEvcStatusTable_FieldDefs,
												cbEvcStatusTable_FieldDefs,
												10,
												(pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
												(void *)EVC_TABLE );

	status |= 	m_pIopStatusTable->DefineTable(	(fieldDef *)aIopStatusTable_FieldDefs,
												cbIopStatusTable_FieldDefs,
												20,
												(pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
												(void *)IOP_TABLE );

	status |= 	m_pDiskTable->DefineTable(		(fieldDef *)DiskDescriptorTable_FieldDefs,
												cbDiskDescriptorTable_FieldDefs,
												10,
												(pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
												(void *)DISK_TABLE );
	status |= 	m_pLoopTable->DefineTable(		(fieldDef *)Loop_Descriptor_Table_FieldDefs,
												cbLoop_Descriptor_Table_FieldDefs,
												10,
												(pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
												(void *)LOOP_TABLE );
	status |= 	m_pHostConnTable1->DefineTable(	(fieldDef *)HostConnectionDescriptorTable_FieldDefs,
												cbHostConnectionDescriptor_FieldDefs,
												10,
												(pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
												(void *)HOST_CONN_TABLE );
	status |= 	m_pHostTable->DefineTable(		(fieldDef *)HostDescriptorTable_FieldDefs,
												cbHostDescriptor_FieldDefs,
												10,
												(pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
												(void *)HOST_TABLE );
	status |= 	m_pSysConfigTable->DefineTable(	(fieldDef *)SystemConfigSettingsTableFieldDefs,
												cbSystemConfigSettingsTableFieldDefs,
												2,
												(pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
												(void *)SYS_CONFIG_TABLE );
	status |= 	m_pPortTable->DefineTable(		(fieldDef *)FCPortDatabaseTable_FieldDefs,
												cbFCPortDatabase_FieldDefs,
												10,
												(pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
												(void *)PORT_TABLE );

	status |= 	m_pSRCTable->DefineTable(		(fieldDef *)StorageRollCallTable_FieldDefs,
												cbStorageRollCallTable_FieldDefs,
												10,
												(pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
												(void *)SRC_TABLE );

	status |= m_pExportUserInfoTable->DefineTable((fieldDef *)ExportTableUserInfoTable_FieldDefs,
												cbExportTableUserInfoTable_FieldDefs,
												10,
												(pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
												(void *)EXPORT_USER_INFO_TABLE );

	status |= m_pMemberTable->DefineTable(		(fieldDef *)MemberDescriptorTable_FieldDefs,
												sizeofMemberDescriptorTable_FieldDefs,
												10,
												(pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
												(void *)MEMBER_TABLE );

	status |= m_pSpareTable->DefineTable(		(fieldDef *)SpareDescriptorTable_FieldDefs,
												sizeofSpareDescriptorTable_FieldDefs,
												10,
												(pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
												(void *)SPARE_TABLE );

	status |= m_pArrayTable->DefineTable(		(fieldDef *)ArrayDescriptorTable_FieldDefs,
												sizeofArrayDescriptorTable_FieldDefs,
												10,
												(pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
												(void *)ARRAY_TABLE );

	status |= m_pExportTable->DefineTable(		(fieldDef *)ExportTable_FieldDefs,
												cbExportTable_FieldDefs,
												10,
												(pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
												(void *)EXPORT_TABLE_ );

	status |= m_pDeviceTable->DefineTable(		(fieldDef *)DeviceDescriptorTable_FieldDefs,
												cbDeviceDescriptorTable_FieldDefs,
												10,
												(pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
												(void *)DEVICE_TABLE);

	status |= m_pPathTable->DefineTable(		(fieldDef *)PathDescriptorTable_FieldDefs,
												cbPathDescriptorTable_FieldDefs,
												10,
												(pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
												(void *)PATH_TABLE );

	status |= m_pSsdTable->DefineTable(			(fieldDef *)SSD_descriptor_table_field_defs,
												cb_SSD_descriptor_table_field_defs,
												5,
												(pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
												(void *)SSD_TABLE );

	status |= m_pSsdSTable->DefineTable(		(fieldDef *)aSSDStatusTable_FieldDefs,
												cbSSDStatusTable_FieldDefs,
												5,
												(pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
												(void *)SSDS_TABLE );

	status |= m_pSsdPTable->DefineTable(		(fieldDef *)aSSDPerformanceTable_FieldDefs,
												cbSSDPerformanceTable_FieldDefs,
												5,
												(pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
												(void *)SSDP_TABLE );

	status |= m_pRaidPTable->DefineTable(		(fieldDef *)aArrayPerformanceTable_FieldDefs,
												cbArrayPerformanceTable_FieldDefs,
												5,
												(pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
												(void *)ARRP_TABLE );
	status |= m_pRaidSTable->DefineTable(		(fieldDef *)aArrayStatusTable_FieldDefs,
												cbArrayStatusTable_FieldDefs,
												5,
												(pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
												(void *)ARRS_TABLE );
	status |= m_pStsPTable->DefineTable(		(fieldDef *)aSTSPerfTable_FieldDefs,
												cbSTSPerfTable_FieldDefs,
												5,
												(pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
												(void *)STSP_TABLE );
	status |= m_pStsSTable->DefineTable(		(fieldDef *)aSTSStatTable_FieldDefs,
												cbSTSStatTable_FieldDefs,
												5,
												(pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
												(void *)STSS_TABLE );
	status |= m_pIdTable->DefineTable(			(fieldDef *)STSDataTable_FieldDefs,
												cbSTSDataTable_FieldDefs,
												5,
												(pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
												(void *)ID_TABLE );
	


	
	/*****************************
	/* 
	/* INITIALIZE ALL SHADOW TABLES
	/*
	/*****************************/
	status |= m_pEvcStatusTable->Initialize( (pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest,HandleAllTablesInitedCallback), (void *)EVC_TABLE );
	status |= m_pIopStatusTable->Initialize( (pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest,HandleAllTablesInitedCallback), (void *)IOP_TABLE );
	status |= m_pDiskTable->Initialize( (pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest,HandleAllTablesInitedCallback), (void *)DISK_TABLE );
	status |= m_pDiskStatusTable->Initialize( (pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest,HandleAllTablesInitedCallback), (void *)DISK_STATUS_TABLE_ );
	status |= m_pDiskPerfTable->Initialize( (pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest,HandleAllTablesInitedCallback), (void *)DISK_PERF_TABLE );
	status |= m_pLoopTable->Initialize( (pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest,HandleAllTablesInitedCallback), (void *)LOOP_TABLE );
	status |= m_pHostConnTable1->Initialize( (pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest,HandleAllTablesInitedCallback), (void *)HOST_CONN_TABLE );
	status |= m_pHostTable->Initialize( (pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest,HandleAllTablesInitedCallback), (void *)HOST_TABLE );
	status |= m_pPortTable->Initialize( (pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest,HandleAllTablesInitedCallback), (void *)PORT_TABLE );
//	status |= m_pSysConfigTable->Initialize( (pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest,HandleAllTablesInitedCallback), (void *)SYS_CONFIG_TABLE );
	status |= m_pSRCTable->Initialize( (pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest,HandleAllTablesInitedCallback), (void *)SRC_TABLE );
	status |= m_pExportUserInfoTable->Initialize( (pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest,HandleAllTablesInitedCallback), (void *)EXPORT_USER_INFO_TABLE );
	status |= m_pMemberTable->Initialize( (pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest,HandleAllTablesInitedCallback), (void *)MEMBER_TABLE );
	status |= m_pSpareTable->Initialize( (pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest,HandleAllTablesInitedCallback), (void *)SPARE_TABLE );
	status |= m_pArrayTable->Initialize( (pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest,HandleAllTablesInitedCallback), (void *)ARRAY_TABLE );
	status |= m_pExportTable->Initialize( (pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest,HandleAllTablesInitedCallback), (void *)EXPORT_TABLE_ );
	status |= m_pDeviceTable->Initialize( (pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest,HandleAllTablesInitedCallback), (void *)DEVICE_TABLE );
	status |= m_pPathTable->Initialize( (pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest,HandleAllTablesInitedCallback), (void *)PATH_TABLE );
	status |= m_pSsdTable->Initialize( (pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest,HandleAllTablesInitedCallback), (void *)SSD_TABLE );
	status |= m_pSsdSTable->Initialize( (pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest,HandleAllTablesInitedCallback), (void *)SSDS_TABLE );
	status |= m_pSsdPTable->Initialize( (pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest,HandleAllTablesInitedCallback), (void *)SSDP_TABLE );
	status |= m_pRaidSTable->Initialize( (pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest,HandleAllTablesInitedCallback), (void *)ARRS_TABLE );
	status |= m_pRaidPTable->Initialize( (pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest,HandleAllTablesInitedCallback), (void *)ARRP_TABLE );
	status |= m_pStsSTable->Initialize( (pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest,HandleAllTablesInitedCallback), (void *)STSS_TABLE );
	status |= m_pStsPTable->Initialize( (pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest,HandleAllTablesInitedCallback), (void *)STSP_TABLE );
	status |= m_pIdTable->Initialize( (pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest,HandleAllTablesInitedCallback), (void *)ID_TABLE );
	///////// INSERT IMAGES ////////
#ifdef WIN32
	img_hdr_t* pImgHdr;
	char* pImage;
	char* imageName;
	// image one
	pImageOne = new(tZERO) char[sizeof(img_hdr_t) + cbImageOne];
	pImgHdr = (img_hdr_t*)pImageOne;
	pImgHdr->i_signature = IMG_SIGNATURE;
	pImgHdr->i_headerversion = HEADER_VERSION;
	pImgHdr->i_mjr_ver = 1;
	pImgHdr->i_mnr_ver = 1;
	pImgHdr->i_imagesize = 1024;
	imageName = (char*)&pImgHdr->i_imagename;
	strcpy(imageName, "imageone");
	pImgHdr->i_type = HBC_IMAGE;
	pImgHdr->i_imageoffset = sizeof(img_hdr_t);
	pImgHdr->i_day = 1;
	pImgHdr->i_month = 1;
	pImgHdr->i_year = 1991;
	pImgHdr->i_sec = 1;
	pImgHdr->i_min = 1;
	pImgHdr->i_hour = 1;
	pImage = (char*)pImageOne + sizeof(img_hdr_t);
	memset(pImage, 1, cbImageOne);
	Send(new MsgAddImage(sizeof(img_hdr_t) + cbImageOne, pImageOne) , REPLYCALLBACK(DdmSSAPITest, AddImageCallback));

	// image two
	pImageTwo = new(tZERO) char[sizeof(img_hdr_t) + cbImageTwo];
	pImgHdr = (img_hdr_t*)pImageTwo;
	pImgHdr->i_signature = IMG_SIGNATURE;
	pImgHdr->i_headerversion = HEADER_VERSION;
	pImgHdr->i_mjr_ver = 1;
	pImgHdr->i_mnr_ver = 2;
	pImgHdr->i_imagesize = 2056;
	imageName = (char*)&pImgHdr->i_imagename;
	strcpy(imageName, "imagetwo");
	pImgHdr->i_type = HBC_IMAGE;
	pImgHdr->i_imageoffset = sizeof(img_hdr_t);
	pImgHdr->i_day = 2;
	pImgHdr->i_month = 2;
	pImgHdr->i_year = 1992;
	pImgHdr->i_sec = 2;
	pImgHdr->i_min = 2;
	pImgHdr->i_hour =21;
	pImage = (char*)pImageTwo + sizeof(img_hdr_t);
	memset(pImage, 1, cbImageTwo);
	Send(new MsgAddImage(sizeof(img_hdr_t) + cbImageTwo, pImageOne) , REPLYCALLBACK(DdmSSAPITest, AddImageCallback));
#endif
	/////////


	// must be the last one
	m_pSRManager = new StringResourceManager( this, (pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest,SRMReadyCallback) );
}


//************************************************************************
//
//************************************************************************
STATUS 
DdmSsapiTest::HandleAllTablesInitedCallback( void *pContext, STATUS rc ){

	U32		tableType = (U32)pContext;

	switch( tableType ){

		case DEVICE_TABLE:
#ifdef WIN32
			PopulateDeviceTable();
#endif
			break;

		case PATH_TABLE:
#ifdef WIN32
			PopulatePathTable();
#endif
			break;

		case IOP_TABLE:
#ifdef WIN32
			PopulateIopTable();
#endif
			break;

		case EVC_TABLE:
#ifdef WIN32
			PopulateEvcTable();
#endif
			break;

		case DISK_TABLE:
#ifdef WIN32
			PopulateDiskDescriptorTable();
#endif
			break;

		case DISK_STATUS_TABLE_:
#ifdef WIN32
			PopulateDiskStatusTable();
#endif
			break;

		case DISK_PERF_TABLE:
#ifdef WIN32
			PopulateDiskPerfTable();
#endif
			break;

		case LOOP_TABLE:
#ifdef WIN32
			PopulateLoopTable();
#endif
			break;

		case HOST_CONN_TABLE:
#ifdef WIN32
			PopulateHostConnTable();
#endif
			break;

		case HOST_TABLE:
#ifdef WIN32
			PopulateHostTable();
#endif
			break;

		case HBA_TABLE:
#ifdef WIN32
			PopulateHbaTable();
#endif
			break;

		case SYS_CONFIG_TABLE:
			PopulateSysConfigTable();
			
			break;

		case PORT_TABLE:
#ifdef WIN32
			PopulatePortTable();
#endif
			break;

		case SRC_TABLE:
#ifdef WIN32
			PopulateSRCTable();
#endif
			break;

		case EXPORT_USER_INFO_TABLE:
#ifdef WIN32
			PopulateExportUserInfoTable();
#endif
			break;
			
		case EXPORT_TABLE_:
#ifdef WIN32
			PopulateExportTable();
#endif
			break;
			
		case MEMBER_TABLE:
#ifdef WIN32
			//PopulateMemberTable();
#endif
			break;

		case SPARE_TABLE:
#ifdef WIN32
			//PopulateSpareTable();
#endif
			break;

		case ARRAY_TABLE:
#ifdef WIN32
			//PopulateArrayTable();
#endif
			break;

		case SSD_TABLE:
#ifdef WIN32
			PopulateSsdTable();
#endif
			break;
		case SSDS_TABLE:
#ifdef WIN32
			PopulateSsdSTable();
			break;
		case SSDP_TABLE:
			PopulateSsdPTable();
			break;
		case STSP_TABLE:
			PopulateStsPTable();
			break;
		case STSS_TABLE:
			PopulateStsSTable();
			break;
		case ARRP_TABLE:
			PopulateRaidPTable();
			break;
		case ARRS_TABLE:
			PopulateRaidSTable();
			break;
		case ID_TABLE:
			PopulateIdTable();
			break;
#endif

		default:
			//ASSERT(0);
			break;
	}

	return OK;
}


//************************************************************************
//
//************************************************************************

void 
DdmSsapiTest::PopulateEvcTable(){

	EVCStatusRecord	row;
	char			*pSerial = "CHASSIS v.1.0";

	row.afEvcReachable[0] = row.afEvcReachable[1] = 1;
	row.ExitAirTemp[0]	= 68;
	row.ExitAirTemp[1]	= 71;
	row.ExitTempFanUpThresh	= 80;
	row.ExitTempFanNormThresh = 70;
	row.FanSpeed[0] = row.FanSpeed[2] = 213;
	row.FanSpeed[1] = row.FanSpeed[3] = 156;
	row.fInputOK[0] = row.fInputOK[1] = row.fInputOK[2] = 1;
	row.fOutputOK[0] = row.fOutputOK[1] = row.fOutputOK[2] = 1;
	memset( &row.fFanFailOrOverTemp, 0, sizeof(row.fFanFailOrOverTemp) );

	row.DCtoDC33Current[0] = 3;
	row.DCtoDC5Current[0] = 5;
	row.DCtoDC12ACurrent[0] = row.DCtoDC12ACurrent[1] = 12;
	row.DCtoDC12BCurrent[0] = row.DCtoDC12BCurrent[1] = 11;
	row.DCtoDC12CCurrent[0] = row.DCtoDC12CCurrent[1]= 13;
	row.DCtoDC33Temp[0] = row.DCtoDC33Temp[1] = 14;
	row.DCtoDC5Temp[0] = row.DCtoDC5Temp[1] = 21;
	row.DCtoDC12ATemp[0] = row.DCtoDC12ATemp[1]= 20;
	row.DCtoDC12BTemp[0] = row.DCtoDC12BTemp[1] = 12;
	row.DCtoDC12BTemp[0] = row.DCtoDC12BTemp[1] = 26;
	row.DCtoDC33Voltage  = 34;
	row.DCtoDC5Voltage  = 24;
	row.DCtoDC12Voltage  = 129;


	row.fBatteryInstalled[0] = row.fBatteryInstalled[1] = 1;
	row.BatteryTemperature[0] = 12;row.BatteryTemperature[0] = 14;
	row.BatteryCurrent[0] = row.BatteryCurrent[1] = 24;

	m_pEvcStatusTable->InsertRow( &row, &m_tempRowId, (pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler), NULL );
}


//************************************************************************
//
//************************************************************************

void PopulateIOPCell(	IOPStatusRecord *pCell,     
						IopType         IOP_Type,
						TySlot          Slot,
   						TySlot          RedundantSlot,
						String32        Manufacturer,
					    U32             ulAvrSwVersion,
						U32             ulAvrSwRevision,
					    String16        strHwPartNo,
						U32             ulHwRevision,
					    U32             ulHwMfgDate,
						String16        SerialNumber,
					    String16        ChassisSerialNumber,
						U32             ulIopEpldRevision,
						U32             ulIopMipsSpeed,
						U32             ulIopPciSpeed,
					    IopState        eIOPCurrentState,
						S32             Temp,
					    U32             TempHiThreshold,
						U32             TempNormThreshold ){

	pCell->IOP_Type			= IOP_Type;
 	pCell->Slot				= Slot;
   	pCell->RedundantSlot	= RedundantSlot;
	memcpy( &pCell->Manufacturer, Manufacturer, sizeof(Manufacturer) );
	pCell->ulAvrSwVersion	= ulAvrSwVersion;
	pCell->ulAvrSwRevision	= ulAvrSwRevision;
	memcpy( &pCell->strHwPartNo, strHwPartNo, sizeof(strHwPartNo) );
	pCell->ulHwRevision		= ulHwRevision;
	pCell->ulHwMfgDate		= ulHwMfgDate;
	memcpy( &pCell->SerialNumber, SerialNumber, sizeof(SerialNumber) );
	memcpy( &pCell->ChassisSerialNumber, ChassisSerialNumber, sizeof(ChassisSerialNumber) );
	pCell->ulIopEpldRevision = ulIopEpldRevision;
	pCell->ulIopMipsSpeed	= ulIopMipsSpeed;
	pCell->ulIopPciSpeed	= ulIopPciSpeed;
	pCell->eIOPCurrentState	= eIOPCurrentState;
	pCell->Temp				= Temp;
	pCell->TempHiThreshold  = TempHiThreshold;
	pCell->TempNormThreshold= TempNormThreshold;
}
						

void 
DdmSsapiTest::PopulateIopTable(){

	PopulateIOPCell( &IOPTable[0], IOPTY_NAC,IOP_RAC0,IOP_RAC1,"CNT",12,23,"",0,0, "0.001A","",0,0,0,IOPS_OPERATING,67,80,70 );
	PopulateIOPCell( &IOPTable[1], IOPTY_NAC,IOP_RAC1,IOP_RAC0,"CNT",1,231,"",0,0,"0.001Aa","",0,0,0,IOPS_OPERATING,75,80,70 );
	PopulateIOPCell( &IOPTable[2], IOPTY_SSD,IOP_SSDU0,IOP_SSDU1,"CNT",110,0,"",0,0,"0.023A","",0,0,0,IOPS_OPERATING,67,90,80 );
	PopulateIOPCell( &IOPTable[3], IOPTY_SSD,IOP_SSDU1,IOP_SSDU0,"CNT",0,0,"",0,0,"0.002","",0,0,0,IOPS_OPERATING,89,90,80 );
	PopulateIOPCell( &IOPTable[4], IOPTY_HBC,IOP_HBC1,IOP_HBC0,"CNT",0,0,"",0,0,"0.12","",0,0,0,IOPS_OPERATING,67,90,80 );
	PopulateIOPCell( &IOPTable[5], IOPTY_HBC,IOP_HBC0,IOP_HBC1,"CNT",0,0,"",0,0,"0.12","",0,0,0,IOPS_OPERATING,40,90,80 );
	PopulateIOPCell( &IOPTable[6], IOPTY_DDH,CMB_DDH0,CMB_DDH0,"CNT",0,0,"",0,0,"0.12A","",0,0,0,IOPS_OPERATING,40,90,80 );
	PopulateIOPCell( &IOPTable[7], IOPTY_DDH,CMB_DDH1,CMB_DDH1,"CNT",0,0,"",0,0,"0.12b","",0,0,0,IOPS_OPERATING,40,90,80 );
	PopulateIOPCell( &IOPTable[8], IOPTY_DDH,CMB_DDH2,CMB_DDH2,"CNT",0,0,"",0,0,"0.12c","",0,0,0,IOPS_OPERATING,40,90,80 );
	PopulateIOPCell( &IOPTable[9], IOPTY_DDH,CMB_DDH3,CMB_DDH3,"CNT",0,0,"",0,0,"0.12d","",0,0,0,IOPS_OPERATING,40,90,80 );

	for( U32 i = 0; i < sizeof(IOPTable)/sizeof(IOPTable[0]); i++ )
		m_pIopStatusTable->InsertRow( &IOPTable[i], &m_tempRowId, (pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler), NULL );
}


//************************************************************************
//
//************************************************************************

void 
DdmSsapiTest::PopulateDiskDescriptorTable(){
	for( U32 i = 0; i < sizeof(DiskTable)/sizeof(DiskTable[0]); i++ ){
		m_pDiskTable->InsertRow( &DiskTable[i], &m_tempRowId, (pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler), NULL );
	}
}


void 
DdmSsapiTest::PopulateDiskStatusTable(){
	for( U32 i = 0; i < sizeof(_DiskStatusTable)/sizeof(_DiskStatusTable[0]); i++ )
		m_pDiskStatusTable->InsertRow( &_DiskStatusTable[i], &m_tempRowId, (pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler), (void *)0 );
	
}


void 
DdmSsapiTest::PopulateDiskPerfTable(){
	for( U32 i = 0; i < sizeof(DiskPerfTable)/sizeof(DiskPerfTable[0]); i++ )
		m_pDiskPerfTable->InsertRow( &DiskPerfTable[i], &m_tempRowId, (pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler), (void *)DISK_PERF_TABLE );
}

void 
DdmSsapiTest::PopulateLoopTable(){
	for( U32 i = 0; i < sizeof(LoopTable)/sizeof(LoopTable[0]); i++ )
		m_pLoopTable->InsertRow( &LoopTable[i], &m_tempRowId, (pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler), NULL );
}


void 
DdmSsapiTest::PopulatePathTable(){
	for( U32 i = 0; i < sizeof(pathTable)/sizeof(pathTable[0]); i++ )
		m_pPathTable->InsertRow( &pathTable[i], &m_tempRowId, (pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler), (void *)PATH_TABLE );
}

void 
DdmSsapiTest::PopulateDeviceTable(){

	for( U32 i = 0; i < sizeof(deviceTable)/sizeof(deviceTable[0]); i++ )
		m_pDeviceTable->InsertRow( &deviceTable[i], &m_tempRowId, (pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler), (void *)DEVICE_TABLE );
}

void 
DdmSsapiTest::PopulateHostConnTable(){

	connTable[0].ridHost = RowId(14,0,1);
	connTable[0].ridName = RowId(34, 0, 3);
	for( U32 i = 0; i < sizeof(connTable)/sizeof(connTable[0]); i++ )
		m_pHostConnTable1->InsertRow( &connTable[i], &m_tempRowId, (pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler1), (void *)HOST_TABLE );
}


void 
DdmSsapiTest::PopulateHostTable(){

	UnicodeString	u;

	u = StringClass("Host1");
	u.CString( &hostTable1[0].name, sizeof(hostTable1[0].name) );
	u = StringClass("Host2");
	u.CString( &hostTable1[1].name, sizeof(hostTable1[0].name) );
	u = StringClass("San Jose's Orion");
	u.CString( &hostTable1[0].description, sizeof(hostTable1[0].description) );
	u = StringClass("Nashua's SS");
	u.CString( &hostTable1[1].description, sizeof(hostTable1[0].description) );
		


	for( U32 i = 0; i < sizeof(hostTable1)/sizeof(hostTable1[0]); i++ )
		m_pHostTable->InsertRow( &hostTable1[i], &m_tempRowId, (pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler1), (void *)HOST_TABLE );
	
}

void 
DdmSsapiTest::PopulateHbaTable(){

}


void 
DdmSsapiTest::PopulateSysConfigTable(){
	SystemConfigSettingsRecord		r;
	
	memset( &r, 0, sizeof(r) );
	r.size = sizeof(r);
	r.version = 1;
	r.ipAddress = 0xFE340123;
	r.snmpTrapAddress[0] = 0xdf300001;
	r.snmpTrapAddress[1] = 0xdf303307;
	r.snmpTrapAddressCount = 2;
	r.subnetMask = 0xFFFFFF00;
	UnicodeString us( StringClass("New Hampshire") );
	us.CString( r.location, sizeof( r.location ) );
	us = StringClass("1st Gemini");
	us.CString( r.hostName, sizeof( r.hostName ) );
	m_pSysConfigTable->InsertRow( &r, &m_tempRowId, (pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler), (void *)0 );
}

void 
DdmSsapiTest::DonePopulatingTables(){
	
#ifdef WIN32	
	Sleep( 10000 );


	Send(	new RqOsTimerStart(TIMEOUT*1000000,TIMEOUT*1000000),
			NULL,
			REPLYCALLBACK(DdmSsapiTest,TimerExparation) );
#endif

}

void TimerExparation( UNSIGNED ){
	int i,j;
	static int counter = 0;
#ifdef WIN32
	OsHeap::heapSmall.ReportDeltaMemoryUsage("Periodic update");

#ifndef TEST_RAMBO
	//for( j = 0; j < 1; j++ )
		//for( i = 0; i < sizeof(METHODS)/sizeof(METHODS[0]); i++)
			//(pSsapiTestDdm->*METHODS[i])();

#else
	
	if( !counter ){
		m_pPathTable			= new ShadowTable(	PTS_VIRTUAL_DEVICE_TABLE,
												pSsapiTestDdm,
												(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
												(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
												(ShadowTable::SHADOW_LISTEN_CALLBACK)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
												sizeof(VirtualDeviceRecord));

		m_pPathTable->DefineTable(	(fieldDef *)VirtualDeviceRecord::FieldDefs(),
									VirtualDeviceRecord::FieldDefsSize() ,
									1,
									(pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),
									(void *)SES_TABLE );
	}
	else if( counter == 1){
		m_pPathTable->DeleteTable((pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler), NULL);
	}
	else
		delete m_pPathTable;
	counter++;
#endif

#ifdef LOG_EVENTS
	for( i = 0; i < 8; i++ )
		LogEvent(ELOG_TEST_WARNING1, counter++);
#endif

#endif
}


STATUS 
DdmSsapiTest::TimerExparation( Message *pMsg ){


	::TimerExparation(0);

	delete ((RqOsTimerStart *)pMsg);
	return OK;
}

void 
DdmSsapiTest::ModifyLoopTable(){
	RowId	id = RowId(12,0,1);
	LoopTable[2].ActualLoopState = (LoopTable[2].ActualLoopState == LoopQuiesce)? LoopUp : LoopQuiesce;
	m_pLoopTable->ModifyRow( id.GetRowID(), &LoopTable[2], (pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler),NULL );
}


void 
DdmSsapiTest::ModifyEvcTable(){

	static int	iii = 0;

	RowId			id = RowId( 9, 0 , 1);
	EVCStatusRecord	row;
	char			*pSerial = "CHASSIS v.1.0";

	row.ExitAirTemp[0]	= 68;
	row.ExitAirTemp[1]	= 71;
	row.ExitTempFanUpThresh	= 80;
	row.ExitTempFanNormThresh = 70;
	row.FanSpeed[0] = row.FanSpeed[2] = 213;
	row.FanSpeed[1] = row.FanSpeed[3] = 156;
	row.fInputOK[0] = row.fInputOK[1] = row.fInputOK[2] = 1;
	row.fOutputOK[0] = row.fOutputOK[1] = row.fOutputOK[2] = 1;
	memset( &row.fFanFailOrOverTemp, 0, sizeof(row.fFanFailOrOverTemp) );

	row.DCtoDC33Current[0] = 3;
	row.DCtoDC5Current[0] = 5;
	row.DCtoDC12ACurrent[0] = row.DCtoDC12ACurrent[1] = 12;
	row.DCtoDC12BCurrent[0] = row.DCtoDC12BCurrent[1] = 11;
	row.DCtoDC12CCurrent[0] = row.DCtoDC12CCurrent[1]= 13;
	row.DCtoDC33Temp[0] = row.DCtoDC33Temp[1] = 14;
	row.DCtoDC5Temp[0] = row.DCtoDC5Temp[1] = 21;
	row.DCtoDC12ATemp[0] = row.DCtoDC12ATemp[1]= 20;
	row.DCtoDC12BTemp[0] = row.DCtoDC12BTemp[1] = 12;
	row.DCtoDC12BTemp[0] = row.DCtoDC12BTemp[1] = 26;
	row.DCtoDC33Voltage = 34;
	row.DCtoDC5Voltage = 24;
	row.DCtoDC12Voltage = 129;

	row.fBatteryInstalled[0] = row.fBatteryInstalled[1] = 1;
	row.BatteryTemperature[0] = 12;row.BatteryTemperature[0] = 14;
	row.BatteryCurrent[0] = row.BatteryCurrent[1] = 24;

	if( iii ){
		row.ExitAirTemp[0]	= 68;
		row.ExitAirTemp[1]	= 71;
		iii = 0;
	}
	else{
		iii = 1;
		row.ExitAirTemp[0]	= 40;
		row.ExitAirTemp[1]	= 89;
	}

	m_pEvcStatusTable->ModifyRow( id.GetRowID(), &row, (pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler), NULL );
}

void 
DdmSsapiTest::ModifyIopTable(){

	RowId		id = RowId( 10, 0, 5 );
	
	IOPTable[4].Temp += 10;
	m_pIopStatusTable->ModifyRow( id.GetRowID(), &IOPTable[4], (pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler), NULL );
}


void 
DdmSsapiTest::ModifyDiskPerfTable(){

	RowId	id( 8, 0, 1 );

	DiskPerfTable[0].AvgReadsPerSec++;				// Average number of reads per second
	DiskPerfTable[0].MaxReadsPerSec++;
	DiskPerfTable[0].MinReadsPerSec++;
	DiskPerfTable[0].AvgWritesPerSec++;			// Average number of writes per second
	DiskPerfTable[0].MaxWritesPerSec++;
	DiskPerfTable[0].MinWritesPerSec++;
	DiskPerfTable[0].AvgTransferPerSec++;			// Average number of transfers (read + write) per second
	DiskPerfTable[0].MaxTransferPerSec++;
	DiskPerfTable[0].MinTransferPerSec++;
	DiskPerfTable[0].AvgBytesReadPerSec++;			// Total bytes read
	DiskPerfTable[0].MaxBytesReadPerSec++;			// Total bytes read
	DiskPerfTable[0].MinBytesReadPerSec++;			// Total bytes read
	DiskPerfTable[0].AvgBytesWrittenPerSec++;		// Total bytes written
	DiskPerfTable[0].MaxBytesWrittenPerSec++;		// Total bytes written
	DiskPerfTable[0].MinBytesWrittenPerSec++;		// Total bytes written
	DiskPerfTable[0].AvgBytesTransferredPerSec++;	// Total bytes transferred (read+write)
	DiskPerfTable[0].MaxBytesTransferredPerSec++;	// Total bytes transferred (read+write)
	DiskPerfTable[0].MinBytesTransferredPerSec++;	// Total bytes transferred (read+write)
	DiskPerfTable[0].AvgReadSize++;				// # of bytes read /# of reads
	DiskPerfTable[0].MaxReadSize++;				// # of bytes read /# of reads
	DiskPerfTable[0].MinReadSize++;				// # of bytes read /# of reads
	DiskPerfTable[0].AvgWriteSize++;
	DiskPerfTable[0].MaxWriteSize++;
	DiskPerfTable[0].MinWriteSize++;

	m_pDiskPerfTable->ModifyRow( id, &DiskPerfTable[0], (pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler), NULL );
}


void
DdmSsapiTest::PopulatePortTable(){

	for( U32 i = 0; i < sizeof(portTable)/sizeof(portTable[0]); i++ )
		m_pPortTable->InsertRow( &portTable[i], &m_tempRowId, (pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler), (void *)PORT_TABLE );
}


void
DdmSsapiTest::PopulateExportTable(){
	for( U32 i = 0; i < sizeof(exportTable)/sizeof(exportTable[0]); i++ )
		m_pExportTable->InsertRow( &exportTable[i], &m_tempRowId, (pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler), (void *)EXPORT_TABLE );
}


void
DdmSsapiTest::PopulateSRCTable(){
	for( U32 i = 0; i < sizeof(SRCTable)/sizeof(SRCTable[0]); i++ )
		m_pSRCTable->InsertRow( &SRCTable[i], &m_tempRowId, (pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler), (void *)EXPORT_TABLE );
}


void
DdmSsapiTest::PopulateExportUserInfoTable(){
	for( U32 i = 0; i < sizeof(userInfoTable)/sizeof(userInfoTable[0]); i++ )
		m_pExportUserInfoTable->InsertRow( &userInfoTable[i], &m_tempRowId, (pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler), (void *)EXPORT_USER_INFO_TABLE );

}


void
DdmSsapiTest::PopulateMemberTable(){

	for( U32 i = 0; i < sizeof(memberTable)/sizeof(memberTable[0]); i++ )
		m_pMemberTable->InsertRow( &memberTable[i], &m_tempRowId, (pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler), (void *)MEMBER_TABLE );

}


void
DdmSsapiTest::PopulateSpareTable(){
	for( U32 i = 0; i < sizeof(spareTable)/sizeof(spareTable[0]); i++ )
		m_pSpareTable->InsertRow( &spareTable[i], &m_tempRowId, (pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler), (void *)SPARE_TABLE );
}


void
DdmSsapiTest::PopulateSsdTable(){
	for( U32 i = 0; i < sizeof(ssdTable)/sizeof(ssdTable[0]); i++ )
		m_pSsdTable->InsertRow( &ssdTable[i], &m_tempRowId, (pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler), (void *)SSD_TABLE );
}

void
DdmSsapiTest::PopulateSsdPTable(){
	for( U32 i = 0; i < sizeof(ssdPTable)/sizeof(ssdPTable[0]); i++ )
		m_pSsdPTable->InsertRow( &ssdPTable[i], &m_tempRowId, (pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler), (void *)SSDP_TABLE );
}
void
DdmSsapiTest::PopulateSsdSTable(){
	for( U32 i = 0; i < sizeof(ssdSTable)/sizeof(ssdSTable[0]); i++ )
		m_pSsdSTable->InsertRow( &ssdSTable[i], &m_tempRowId, (pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler), (void *)SSDS_TABLE );
}


void
DdmSsapiTest::PopulateRaidSTable(){
	for( U32 i = 0; i < sizeof(raidSTable)/sizeof(raidSTable[0]); i++ )
		m_pRaidSTable->InsertRow( &raidSTable[i], &m_tempRowId, (pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler), (void *)SSDP_TABLE );
}


void
DdmSsapiTest::PopulateRaidPTable(){

	for( U32 i = 0; i < sizeof(raidPTable)/sizeof(raidPTable[0]); i++ )
		m_pRaidPTable->InsertRow( &raidPTable[i], &m_tempRowId, (pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler), (void *)SSDP_TABLE );
}


void
DdmSsapiTest::PopulateStsSTable(){
	for( U32 i = 0; i < sizeof(stsSTable)/sizeof(stsSTable[0]); i++ )
		m_pStsSTable->InsertRow( &stsSTable[i], &m_tempRowId, (pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler), (void *)SSDP_TABLE );
}


void
DdmSsapiTest::PopulateStsPTable(){
	for( U32 i = 0; i < sizeof(stsPTable)/sizeof(stsPTable[0]); i++ )
		m_pStsPTable->InsertRow( &stsPTable[i], &m_tempRowId, (pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler), (void *)SSDP_TABLE );
}


void 
DdmSsapiTest::PopulateArrayTable(){
	RowId	rid = RowId(15, 0, 1);
	memcpy( &arrayTable[0].members[0], &rid, sizeof(rid) );

	rid = RowId(15, 0, 2);
	memcpy( &arrayTable[0].members[1], &rid, sizeof(rid) );

	rid = RowId(16, 0, 1);
	memcpy( &arrayTable[0].spares[0], &rid, sizeof(rid) );

	arrayTable[0].numberSpares = 1;
	arrayTable[0].numberMembers = 2;

	for( U32 i = 0; i < sizeof(arrayTable)/sizeof(arrayTable[0]); i++ )
		m_pArrayTable->InsertRow( &arrayTable[i], &m_tempRowId, (pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler), (void *)ARRAY_TABLE );
}

void
DdmSsapiTest::PopulateStringResources(){


	UnicodeString	us = StringClass("Lun1 Name");
	RowId			*p;
	
	p = new RowId;
	m_pSRManager->WriteString(us, p, (pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest, SRMStringInsertedCallback ), p );
	m_stringResources++;
	
	us = "";
	us = StringClass("Lun1 Description");
	p = new RowId;
	m_pSRManager->WriteString(us, p, (pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest, SRMStringInsertedCallback ), p );
	m_stringResources++;

	us = "";
	us = StringClass("Host1");
	m_pSRManager->WriteString(us, &m_tempRowId, (pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest, SRMStringInsertedCallback ), NULL );
	m_stringResources++;

	us = "";
	us = StringClass("HostConnection1");
	m_pSRManager->WriteString(us, &m_tempRowId, (pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest, SRMStringInsertedCallback ), NULL );
	m_stringResources++;

	us = "";
	us = StringClass("Host2");
	m_pSRManager->WriteString(us, &m_tempRowId, (pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest, SRMStringInsertedCallback ), NULL );
	m_stringResources++;

	us = "";
	us = StringClass("Host2 Description");
	m_pSRManager->WriteString(us, &m_tempRowId, (pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest, SRMStringInsertedCallback ), NULL );
	m_stringResources++;

	us = "";
	us = StringClass("Gemini Array");
	m_pSRManager->WriteString(us, &m_tempRowId, (pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest, SRMStringInsertedCallback ), NULL );
	m_stringResources++;

	us = "";
	us = StringClass("SparePool");
	m_pSRManager->WriteString(us, &m_tempRowId, (pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest, SRMStringInsertedCallback ), NULL );
	m_stringResources++;
}

STATUS 
DdmSsapiTest::SRMReadyCallback( void*, STATUS ){

	PopulateStringResources();

	return OK;
}


STATUS 
DdmSsapiTest::SRMStringInsertedCallback( void *p, STATUS ){

	RowId	*pR = (RowId *)p;
	if( !--m_stringResources )
		DonePopulatingTables();
	return OK;
}


STATUS 
DdmSsapiTest::AddImageCallback( Message *pMsg_ ){

	MsgAddImage	*pMsg = (MsgAddImage *)pMsg_;

	delete pMsg;
	return OK;
}


void 
DdmSsapiTest::PopulateIdTable(){
	StsData	row;

	memset( &row, 0, sizeof(row) );
	row.size = sizeof(StsData);
	row.vdSTS = 5;
	strcpy( (char *)row.InqData.VendorId, "DELL" );
	strcpy( (char *)row.InqData.ProductId, "Clariion Lun" );
	strcpy( (char *)row.InqData.ProductRevision, "2.4" );
	m_pIdTable->InsertRow( &row, &m_tempRowId, (pTSCallback_t)METHOD_ADDRESS(DdmSsapiTest,ListenCallbackHandler), (void *)ID_TABLE );
}