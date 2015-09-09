/*************************************************************************/
// This material is a confidential trade secret and proprietary 
// information of ConvergeNet Technologies, Inc. which may not be 
// reproduced, used, sold or transferred to any third party without the 
// prior written consent of ConvergeNet Technologies, Inc.  This material 
// is also copyrighted as an unpublished work under sections 104 and 408 
// of Title 17 of the United States Code.  Law prohibits unauthorized 
// use, copying or reproduction.
//
// Description:
// This class define our HBC subnet constants
// 
// Update Log: 
// 08/19/99 Joe Altmaier: Created 
/*************************************************************************/

#include "DeviceId.h"
#include "Network.h"

#ifdef __cplusplus
extern "C" IP_ADDRESS ip_iSlot[NSLOT];
#else
extern IP_ADDRESS ip_iSlot[NSLOT];
#endif

#define NETADDR 0xDFFFFF00
#define SUBNET	0xFFFFFFFC
