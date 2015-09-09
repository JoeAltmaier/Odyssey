//******************************************************************************
// FILE:		DdmSsapiTest.h
//
// PURPOSE:		Defines DDM-based object for testing/debugging purposes
//******************************************************************************

#ifndef __DDM_SSAPI_TEST_H__
#define __DDM_SSAPI_TEST_H__

#include "StringResourceManager.h"
#ifdef WIN32
#include <stdio.h>
#define Tracef printf
#endif 

#pragma pack(4)

#include "ddm.h"
#include "DdmMaster.h"
#include "ListenManager.h"
#include "UserManager.h"
#include "Kernel.h"
class ShadowTable;




class DdmSsapiTest : public DdmMaster {
	
	ShadowTable			*pUserTable;
	ShadowTable			*m_pIopStatusTable;
	ShadowTable			*m_pEvcStatusTable;
	ShadowTable			*m_pDiskTable;
	ShadowTable			*m_pDiskStatusTable;
	ShadowTable			*m_pDiskPerfTable;
	ShadowTable			*m_pLoopTable;
	ShadowTable			*m_pHostTable;
	ShadowTable			*m_pHostConnTable1;
	ShadowTable			*m_pSysConfigTable;
	ShadowTable			*m_pHbaTable;
	ShadowTable			*m_pPortTable;
	ShadowTable			*m_pExportTable;
	ShadowTable			*m_pExportUserInfoTable;
	ShadowTable			*m_pSRCTable;
	ShadowTable			*m_pMemberTable;
	ShadowTable			*m_pSpareTable;
	ShadowTable			*m_pArrayTable;
	ShadowTable			*m_pSsdTable;
	ShadowTable			*m_pSsdPTable;
	ShadowTable			*m_pSsdSTable;
	ShadowTable			*m_pRaidSTable;
	ShadowTable			*m_pRaidPTable;
	ShadowTable			*m_pStsSTable;
	ShadowTable			*m_pStsPTable;
	ShadowTable			*m_pIdTable;
	ShadowTable			*m_pDeviceTable;
	ShadowTable			*m_pPathTable;
	CT_Timer			m_timer;

	StringResourceManager	*m_pSRManager;

	rowID				m_tempRowId;

	U16					m_DiskPerfTableNumber, m_DiskStatusTableNumber;
	U32					m_stringResources;

public:



//******************************************************************************
// Ctor:
//
// PURPOSE:		Serves as a means for synamic creation
//******************************************************************************

static Ddm* Ctor( DID did ) { return new DdmSsapiTest( did ); }


//******************************************************************************
// ~DdmSsapiTest:
//
// PURPOSE:		class destructor
//******************************************************************************

~DdmSsapiTest();




private:


//******************************************************************************
// DdmSsapiTest:
//
// PURPOSE:		Default constructor
//******************************************************************************

DdmSsapiTest( DID did );


//******************************************************************************
// Initialize:
//
// PURPOSE:		DDM-model complience + possible init stuff
//******************************************************************************

STATUS Initialize( Message *pMsg );


//******************************************************************************
// ListenCallbackHandler:
//
// PURPOSE:		Hadnles listen replies from the PTs
//******************************************************************************

STATUS ListenCallbackHandler( void *pContext, U32 rc, ShadowTable* );
STATUS ListenCallbackHandler( void *pContext, STATUS rc );
STATUS ListenCallbackHandler1( void *pContext, STATUS rc );

//******************************************************************************
// The following functions are responsible for populating the UserAccessTable
//******************************************************************************

STATUS StartPopulatingUserTable();
STATUS TableDefined( void *pContext, STATUS rc );
STATUS TableInitialized( void *pContext, STATUS rc );
STATUS RowInserted( void *pContext, STATUS rc );
void UserTableIsDone();

STATUS HandleAllTablesInitedCallback( void *pContext, STATUS rc );

void DonePopulatingTables();


//******************************************************************************
// The following methods are responsible for populating the following tables:
// -- IOP
// -- EVC
// -- DISK DESCRIPTOR
//******************************************************************************

void PopulateEvcTable();
void PopulateIopTable();
void PopulateDiskDescriptorTable();
void PopulateDiskStatusTable();
void PopulateDiskPerfTable();
void PopulateLoopTable();
void PopulateHostConnTable();
void PopulateHostTable();
void PopulateSysConfigTable();
void PopulateHbaTable();
void PopulatePortTable();
void PopulateExportTable();
void PopulateSRCTable();
void PopulateExportUserInfoTable();
void PopulateStringResources();
void PopulateMemberTable();
void PopulateSpareTable();
void PopulateArrayTable();
void PopulateSsdTable();
void PopulateSsdPTable();
void PopulateSsdSTable();
void PopulateRaidSTable();
void PopulateRaidPTable();
void PopulateStsSTable();
void PopulateStsPTable();
void PopulateIdTable();
void PopulatePathTable();
void PopulateDeviceTable();

STATUS SRMReadyCallback( void*, STATUS );
STATUS SRMStringInsertedCallback( void*, STATUS );
STATUS AddImageCallback( Message *pMsg );
STATUS TimerExparation( Message *pMsg );

//******************************************************************************
// Periodicly called methods
//******************************************************************************
public:
void ModifyLoopTable();
void ModifyEvcTable();
void ModifyIopTable();
void ModifyDiskPerfTable();

};

typedef	void (DdmSsapiTest::*PERIODICLY_CALLED_METHOD)();

void TimerExparation( UNSIGNED );

#endif // __DDM_SSAPI_TEST_H__
