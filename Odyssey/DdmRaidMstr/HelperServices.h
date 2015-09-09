/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// (c) Copyright 1999 ConvergeNet Technologies, Inc.
//     All Rights Reserved.
//
// File: HelperServices.h
// 
// Description:
// 
// $Log: /Gemini/Odyssey/DdmRaidMstr/HelperServices.h $
// 
// 5     11/09/99 2:50p Dpatel
// win32 compilation etc.
// 
// 4     11/08/99 9:36a Jlane
// Made changes to utilize new VirtualMaster LoadVirtualDevice message.
// 
// 3     10/07/99 1:27p Dpatel
// fixed some bugs in the VD create..
// 
// 2     9/16/99 5:42p Dpatel
// 
// 1     9/15/99 4:01p Dpatel
// Initial creation
// 
// 
//
/*************************************************************************/

#ifndef __HelperServices_h
#define __HelperServices_h

#include "CtEvent.h"

#include "CtTypes.h"
#include "TableMsgs.h"
#include "DdmOsServices.h"
#include "PtsCommon.h"
#include "ReadTable.h"
#include "Listen.h"
#include "Fields.h"
#include "Rows.h"
#include "Listen.h"
#include "Table.h"


#include "RqOsTimer.h"  // Timer service messages

#include "StorageRollCallTable.h"
#include "DiskDescriptor.h"
#include "VirtualDeviceTable.h"
#include "SystemStatusTable.h"


#include "TableServices.h"
#include "UnicodeString.h"
#include "StringResourceManager.h"



#pragma	pack(4)



class HelperServices: DdmServices {
public:
	HelperServices(DdmServices *pParentDdm);
	~HelperServices();

	void ReadStorageElementName(
		UnicodeString				*pUnicodeString,
		U32							*pSlotIDRet,
		rowID						*pSRCTRID,
		pTSCallback_t				cb,
		void						*pContext);
	void DeleteStorageElementName(
		rowID						*pSRCTRID,
		pTSCallback_t				cb,
		void						*pContext);

	// Virtual Device Operations
	// inserts a virtual device in the virtual device table,
	STATUS CreateVirtualDevice(
		VirtualDeviceRecord			*pVDRecord,
		pTSCallback_t				cb,
		void						*pContext);

private:
	DdmServices			*m_pParentDdm;
	
	TableServices			*m_pTableServices;
	StringResourceManager	*m_pStringResourceManager;


	struct HELPER_CONTEXT{
		U32				state;
		void			*pData;
		void			*pData1;
		void			*pData2;
		void			*pData3;
		U32				value;
		U32				value1;
		U32				value2;
		U32				value3;
		rowID			newRowId;
		void			*pParentContext;
		pTSCallback_t	pCallback;

		HELPER_CONTEXT(){
			state = 0;
			pData = NULL;
			pData1 = NULL;
			pData2 = NULL;
			pData3 = NULL;
			value = 0;
			value1 = 0;
			value2 = 0;
			value3 = 0;
			pParentContext = NULL;
			pCallback = NULL;
		}
		~HELPER_CONTEXT(){
			if (pData){
				delete pData;
				pData = NULL;
			}
			if (pData1){
				delete pData1;
				pData1 = NULL;
			}
			if (pData2){
				delete pData2;
				pData2 = NULL;
			}
			if (pData3){
				delete pData3;
				pData3 = NULL;
			}
		}
	};

private:
	STATUS ReadStorageElementNameSRCReadReply(
		void							*_pContext, 
		STATUS							status);
	STATUS ReadStorageElementNameDiskDescriptorReadReply(
		void							*_pContext, 
		STATUS							status);
	STATUS DeleteStorageElementNameSRCReadReply(
		void							*_pContext,
		STATUS							status);

	STATUS StringResourceManagerInitializedReply(
		void						*pContext,
		STATUS						status);

	// Create Virtual Device methods
	STATUS CheckExistingVirtualDeviceRecord(
		void						*pHelperContext,
		STATUS						status);
	STATUS LoadVirtualDevice(
		void						*pHelperContext,
		STATUS						status);
	STATUS LoadVirtualDeviceReply(Message *pMsgRet);

	STATUS CleanVDCreate(
		void						*_pHelperContext, 
		STATUS						status);
};
#endif


