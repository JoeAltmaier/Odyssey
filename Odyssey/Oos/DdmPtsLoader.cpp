/* DdmPtsLoader.cpp -- Start PTS and load PTS defaults
 *
 * Copyright (C) ConvergeNet Technologies, 1999
 * Copyright (C) Dell Computer, 2000
 *
 * This material is a confidential trade secret and proprietary 
 * information of ConvergeNet Technologies, Inc. which may not be 
 * reproduced, used, sold or transferred to any third party without the 
 * prior written consent of ConvergeNet Technologies, Inc.  This material 
 * is also copyrighted as an unpublished work under sections 104 and 408 
 * of Title 17 of the United States Code.  Law prohibits unauthorized 
 * use, copying or reproduction.
 *
 * Description:
 *		Load the PTS specified via the PTSNAME macro in BuildSys.cpp
 *
**/
 
// Revision History: 
//  6/30/99 Tom Nelson: Create file
// ** Update Log at end-of-file **

// 100 columns
//34567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890

#define _TRACEF
#define	TRACE_INDEX		TRACE_PTSLOADER
#include "Odyssey_Trace.h"

// Public Includes
#include <String.h>

// Private Includes
#include "DdmPtsLoader.h"
#include "RqOsDdmManager.h"
#include "BuildSys.h"
#include "ServeTable.h"

#include "VirtualRouteTable.h"
#include "VirtualDeviceTable.h"

// BuildSys Linkage

CLASSNAME(DdmPtsLoader,SINGLE);

// .DdmPtsLoader -- Constructor -------------------------------------------------------DdmPtsLoader-
//
DdmPtsLoader::DdmPtsLoader(DID did) : Ddm(did) /*, listenerFindPts(this)*/ {
	TRACE_PROC(DdmPtsLoader::DdmPtsLoader);
}

// .Initialize -- Process Initialize --------------------------------------------------DdmPtsLoader-
//
STATUS DdmPtsLoader::Initialize(Message *_pRequest) {
	TRACE_PROC(DdmPtsLoader::Initialize);

	Reply(_pRequest,OK);	

	return OK;
}

// .Enable -- Process Enable ----------------------------------------------------------DdmPtsLoader-
//
STATUS DdmPtsLoader::Enable(Message *_pMsgEnable) {
	TRACE_PROC(DdmPtsLoader::Enable);

	if (Address::GetSlot() != Address::GetHbcMasterSlot()) {
		if (Address::GetSlotFop() != Address::GetHbcMasterSlot())
			Tracef("[WARNING] DO NOT RUN DdmPtsLoader on IOPs!\n");
			
		Reply(_pMsgEnable,OK);
	}
	else{
		// Load VirtualDevice(s) & Config data
		DdmSvcVirtualLoader *pLoader = new DdmSvcVirtualLoader(this);
		pLoader->Execute(_pMsgEnable, ACTIONCALLBACK(DdmPtsLoader,ProcessLoadVirtualComplete));
	}
	return OK;
}

// .ProcessLoadVirtualAction -- Process Callback --------------------------------------DdmPtsLoader-
//
ERC DdmPtsLoader::ProcessLoadVirtualComplete(void *_pLoader) {
	TRACE_PROC(DdmPtsLoader::ProcessLoadVirtualAction);

	DdmSvcVirtualLoader *pLoader = (DdmSvcVirtualLoader*) _pLoader;
	Message *pMsgEnable = (Message*) pLoader->GetContext();
	
	Reply(pMsgEnable,OK);

	delete _pLoader;
	return OK;
}

//***					  ***
//*** DdmSvcVirtualLoader ***
//***					  ***

// .Execute -- Insert all VirtualDevices and their Config Records ------DdmSvcVirtualLoader-
//
// Note that you may not have two VD of same class both without config records!
//
ERC DdmSvcVirtualLoader::Execute(void *_pContext, ActionCallback _callback) {
	TRACE_PROC(DdmSvcVirtualLoader::Execute);

	pContext = _pContext;
	callback = _callback;
	
	// Create VirtualRouteTable (VRT) Not Persistant
	VirtualRouteRecord::RqDefineTable *pVrtDefine = new VirtualRouteRecord::RqDefineTable(NotPersistant_PT);
	Send(pVrtDefine, REPLYCALLBACK(DdmSvcVirtualLoader,DiscardOkReply));
	
	// Create VirtualDeviceTable (VDT) Persistant
	VirtualDeviceRecord::RqDefineTable *pDefine = new VirtualDeviceRecord::RqDefineTable;
	Send(pDefine, REPLYCALLBACK(DdmSvcVirtualLoader,ProcessDefineTableReply));

	return OK;
}

// .DefineTableReply -- Process Reply ------------------------------------------DdmSvcVirtualLoader-
//
ERC DdmSvcVirtualLoader::ProcessDefineTableReply(Message *_pReply) {
	TRACE_PROC(DdmSvcVirtualLoader::DefineTableReply);

	ERC erc = _pReply->DetailedStatusCode;
	delete _pReply;

	if (erc != OK) {
		// Skip all loading if VirtualDeviceTable already exists
		if (erc != ercTableExists) {
			TRACEF(TRACE_L3,("Define VirtualDeviceTable Reply; Erc=%u (DdmSvcVirtualLoader::ProcessDefineTableReply)\n",erc));
			status = erc;
		}
		Action(pParentDdmSvs,callback,this);
		return OK;
	}

	VirtualEntry *pEntry;

	// Delete Class Config Tables just incase
	for (pEntry = VirtualTable::GetFirst();  pEntry != NULL; pEntry = pEntry->pNextEntry) {
		if (pEntry->pszTableName != NULL) {
			RqPtsDeleteTable *pDelete = new RqPtsDeleteTable(pEntry->pszTableName);
			Send(pDelete,REPLYCALLBACK(DdmSvcVirtualLoader,DiscardReply));
		}
	}

	// Create Class Config Tables
	for (pEntry = VirtualTable::GetFirst();  pEntry != NULL; pEntry = pEntry->pNextEntry) {
		if (pEntry->pszTableName != NULL) {
			// Define Config Table for VirtualDevice class
			RqPtsDefineTable *pDefine = new RqPtsDefineTable(pEntry->pszTableName, Persistant_PT, 20, (fieldDef*)pEntry->pFieldDefs,pEntry->sFieldDefs);
			erc = Send(pDefine, REPLYCALLBACK(DdmSvcVirtualLoader,DiscardOkReply));
		}
	}

	if (!LoadVirtualData(VirtualTable::GetFirst()) )
		TRACEF(TRACE_L3,("No VirtualDevices to load!\n (DdmSvcVirtualLoader::Execute)\n"));

	return OK;
}

// .LoadVirtualData -- Insert all VirtualDevices and their Config Records ------DdmSvcVirtualLoader-
//
// Note that you may not have two VD of same class both without config records!
//
BOOL DdmSvcVirtualLoader::LoadVirtualData(VirtualEntry *_pEntry) {
	TRACE_PROC(DdmSvcVirtualLoader::LoadVirtualData);

	ERC erc;
	RowId ridNone;
	
	if (_pEntry != NULL) {
		if (_pEntry->sData > 0) {
			// On reply InsertDevice with RowId of config record
			erc = InsertConfigData(_pEntry, _pEntry, REPLYCALLBACK(DdmSvcVirtualLoader,ProcessCfgInsertReply));
		}
		else {		
			// InsertDevice without RowId of config record
			erc = InsertVirtualDevice(_pEntry, _pEntry, ridNone, REPLYCALLBACK(DdmSvcVirtualLoader,ProcessVdtInsertReply));
		}
		return TRUE;
	}
	Action(pParentDdmSvs,callback,this);
	status = OK;
	
	return FALSE;
}

// .ProcessCfgInsertReply -- Process Callback -------------------------------------DdmSvcVirtualLoader-
//
ERC DdmSvcVirtualLoader::ProcessCfgInsertReply(Message *_pReply) { 
	TRACE_PROC(DdmSvcVirtualLoader::ProcessCfgInsertReply);

	ERC erc = _pReply->DetailedStatusCode;
	if (erc != OK)
		Tracef("** Config creation failure ; Rq=\"%s\" Erc=%u (PtsLoader::ProcessCfgInsertReply)\n",NameRq(_pReply->reqCode),erc);
	else	
		TRACEF(TRACE_L3,("Config  Rq=\"%s\" Erc=%u (ProcessCfgInsertReply)\n",NameRq(_pReply->reqCode),erc));

	RqPtsInsertRow *pReply = (RqPtsInsertRow *) _pReply;
	VirtualEntry *pEntry = (VirtualEntry *) pReply->GetContext();

	RowId rid = *pReply->GetRowIdDataPtr();
	erc = InsertVirtualDevice(pEntry, pEntry, rid, REPLYCALLBACK(DdmSvcVirtualLoader,ProcessVdtInsertReply));
	
	delete _pReply;
	return OK;	// No Enable yet!	
}
	
// .ProcessVdtInsertReply -- Process Callback --------------------------------------DdmSvcVirtualLoader-
//
// VirtualDeviceTable now filled.
//
// Now define the config data table(s)
//
ERC DdmSvcVirtualLoader::ProcessVdtInsertReply(Message *_pReply) {
	TRACE_PROC(DdmSvcVirtualLoader::ProcessVdtInsertReply);

	TRACEF(TRACE_L3,("Vdt::RqInsertTable reply; Erc=%u (ProcessVdtInsertReply)\n",_pReply->DetailedStatusCode));

	ERC erc = _pReply->DetailedStatusCode;

	if (erc != OK)
		Tracef("** Config creation failure Rq=\"%s\" Erc=%u (ProcessCfgCreationReply)\n",NameRq(_pReply->reqCode),erc);
	else
		TRACEF(TRACE_L3,("Config Rq=\"%s\" Erc=%u (ProcessCfgCreationReply)\n",NameRq(_pReply->reqCode),erc));

	// Do not go on until we get ALL of our replies!
	VirtualEntry *pEntry = (VirtualEntry *) _pReply->GetContext();
	LoadVirtualData(pEntry->pNextEntry);

	delete _pReply;
	return OK;
}

// .InsertConfigData -- Process Callback -------------------------------------------DdmSvcVirtualLoader-
//
ERC DdmSvcVirtualLoader::InsertConfigData(VirtualEntry *pEntry, void* pContext, ReplyCallback callback) {
	TRACE_PROC(DdmSvcVirtualLoader::InsertConfigData);

	if (pEntry->sData > 0) {
		// Insert Config Record
		if (pEntry->sTableRecord < pEntry->sData)
			Tracef("**\n** ERROR data record is larger than table record. vdn=%u; \"%s\"\n",pEntry->vdn,pEntry->pszClassName);
		else {
			// Insert Config Data
			if (pEntry->sTableRecord != pEntry->sData)
				Tracef("**\n** WARNING data record is shorter than table record. vdn=%u; \"%s\"\n",pEntry->vdn,pEntry->pszClassName);
				
			void *pRowData = new char[pEntry->sTableRecord];
			
			if (pEntry->sData != pEntry->sTableRecord)
				Tracef("WARNING Class \"%s\" config cbData=%u != cbTableRecord=%u\n",pEntry->pszClassName,pEntry->sData,pEntry->sTableRecord);

			if (pEntry->sData > pEntry->sTableRecord)
				pEntry->sData = pEntry->sTableRecord;
			
			memcpy(pRowData, pEntry->pData, pEntry->sData);
			TRACEF(TRACE_L3,("Insert Data in table \"%s\" (SendCfgInsert)\n",pEntry->pszTableName));
			
			RqPtsInsertRow *pInsert = new RqPtsInsertRow(pEntry->pszTableName, pRowData, pEntry->sTableRecord);
			ERC erc = Send(pInsert, pContext, callback);
				
			delete pRowData;
			return erc;
		}			
	}			
	return OK;
}

// .InsertVirtualDevice -- Send Insert to VirtualDeviceTable -----------------------DdmSvcVirtualLoader-
//
ERC DdmSvcVirtualLoader::InsertVirtualDevice(VirtualEntry *_pEntry,void *payload, RowId &ridCfg, ReplyCallback callback) {
	TRACE_PROC(DdmSvcVirtualLoader::InsertVirtualDevice);

	// Force RowId equal to BuildSys VDN in VirtualDeviceTable 
	VirtualDeviceRecord::RqQuerySetRID *pSet = new VirtualDeviceRecord::RqQuerySetRID(_pEntry->vdn-1);
	Send(pSet, VirtualTable::GetFirst(), REPLYCALLBACK(DdmPtsLoader,DiscardReply));

	// Should really wait for the above send to return to be sure requests do not get
	// out of order however, since PTS and us are both on HBC we will ignore this issue.
	VirtualDeviceRecord recVdt(_pEntry->pszClassName,_pEntry->slotPrimary,_pEntry->slotSecondary,_pEntry->fStart, ridCfg);		

	VirtualDeviceRecord::RqInsertRow *pMsg = new VirtualDeviceRecord::RqInsertRow(&recVdt,1);
			
	ERC erc = Send(pMsg, payload, callback);
	TRACEF(TRACE_L3, ("INSERTING Vdn=%u; Class=\"%s\"; fStart=%u; slotPrimary=%x; slotSecondary=%x; erc=%u\n",
			  _pEntry->vdn,_pEntry->pszClassName,_pEntry->fStart,_pEntry->slotPrimary,_pEntry->slotSecondary,erc));
	
	return erc; 
}


//**************************************************************************************************
// Update Log:
//	$Log: /Gemini/Odyssey/Oos/DdmPtsLoader.cpp $
// 
// 13    2/15/00 6:06p Tnelson
// Removed references to obsolete files
// 
// 12    2/08/00 8:57p Tnelson
// Fix Load/Delete VirtualDevice request
// Added SYSTEMMASTER Macro
// Added Termination to Ddm
// Fix PtsLoader bug
// 
// 13    2/08/00 6:07p Tnelson
// Load/Delete VirtualDevice fixes, etc.
// New SystemMaster Macro support
// DDM Termination
// 
// 5     9/17/99 9:56p Tnelson
// 
// 4     9/17/99 9:46p Tnelson
// 
// 3     9/17/99 9:45p Tnelson
// Fix string len problem
// 
// 2     9/16/99 3:20p Tnelson


