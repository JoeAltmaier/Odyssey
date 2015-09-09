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
// File: RmstrCapabilities.h
// 
// Description:
// Defines the Rmstr interface for Commands/Status.
// 
// $Log: /Gemini/Include/Raid/RmstrCapabilities.h $
// 
// 2     8/11/99 2:59p Dpatel
// Added version size to tables, changed cap to I64.
// 
// 1     8/02/99 3:00p Dpatel
// Initial creation..
// 
//
/*************************************************************************/

#ifndef __RmstrCapabilities_h
#define __RmstrCapabilities_h

#include "CtTypes.h"
#include "TableMsgs.h"
#include "DdmOsServices.h"
#include "PtsCommon.h"
#include "RowId.h"
#include "ReadTable.h"
#include "Listen.h"
#include "Fields.h"

#include "TableServices.h"
#include "RmstrCapabilityTable.h"


#pragma	pack(4)






class RmstrCapabilities : DdmServices {
public:
	RmstrCapabilities(DdmServices *pParentDdm);
	~RmstrCapabilities();
	void GetRmstrCapability(
		RMSTR_CAPABILITY_CODE		code,
		U32							*pFound,
		RMSTR_CAPABILITY_DESCRIPTOR *pCapabilityDescriptor,
		pTSCallback_t				cb,
		void						*pContext);
private:
	STATUS GetRmstrCapabilityReply (void *_pContext, STATUS status);

private:
	TableServices			*m_pTableServices;

	struct CAPABILITY_CONTEXT{
		void			*pData;
		void			*pData1;
		void			*pData2;
		U32				value;
		U32				value1;
		U32				value2;
		void			*pParentContext;
		pTSCallback_t	pCallback;

		CAPABILITY_CONTEXT(){
			pData = NULL;
			pData1 = NULL;
			pData2 = NULL;
			value = 0;
			value1 = 0;
			value2 = 0;
			pParentContext = NULL;
			pCallback = NULL;
		}
		~CAPABILITY_CONTEXT(){
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
		}
	};

};
#endif


