/*************************************************************************
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// Description:
// 
*************************************************************************/
#include "UpgradeCmdQueue.h"

CmdInfoBase::CmdInfoBase(void* pData)
{
	U8* p = (U8*) pData;

	memcpy(&cib, p, sizeof(cib));
	p += sizeof(cib);

	pImage = new (tBIG) U8[cib.cbImage];
	memcpy(pImage, p, cib.cbImage);
}

U32 CmdInfoBase::WriteAsStruct(void** pData)
{
	*pData = new (tBIG) U8[(sizeof(cib) + cib.cbImage)];
	U8* p = (U8*)*pData;

	memcpy(p, &cib, sizeof(cib));
	p += sizeof(cib);

	memcpy(p, pImage, cib.cbImage);

	return (sizeof(cib) + cib.cbImage);
}