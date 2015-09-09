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
// This class defines our HBC subnet
// 
// Update Log: 
// 08/19/99 Joe Altmaier: Created 
/*************************************************************************/

#include "HbcSubnet.h"

/*
At the end of each
of the class A, B, and C ranges, exists one subnet of reserved IPs which
the "authorities" have deemed are unused.  No one will be using these. 
They are routable so no one should be using them on an fake internal
network either.  

Network: 223.255.255.0/30
Subnet: 255.255.255.252
IPs: 223.255.255.1 and 223.255.255.2
*/
	U32 ip_iSlot[NSLOT] = {0xDFFFFF01, 0xDFFFFF02};
//	U32 ip_iSlot[NSLOT] = {0x0A0101C8, 0x0A0101C9};

