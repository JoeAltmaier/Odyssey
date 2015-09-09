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
// This class implements the bootblock sucker (boot blucker)
// 
// Update Log: 
// 9/31/99 Ryan Braun: Create file
/*************************************************************************/

#ifndef _TRACEf
#define _TRACEf
#include "Trace_Index.h"
#include "Odyssey_trace.h"
#endif

#include "BootBlucker.h"
#include "HbcSubnet.h"
#include "BuildSys.h"

DEVICENAME(BootBlucker, BootBlucker::Initialize);

void BootBlucker::Initialize() {

//	Tracef("BootBlucker::Initialize()\n");
	
	// If we are the slave HBC, suck away!
	if ((Address::iSlotMe != Address::iSlotHbcMaster) && ((Address::iSlotMe == IOP_HBC0) || (Address::iSlotMe == IOP_HBC1)))
		SuckBootBlock();
	
}

void BootBlucker::SuckBootBlock()
{

	int 		socketd;
	STATUS		status;
	struct		addr_struct	serverAddr;
	bootblock_t	recvblock;
	
//	Tracef("BootBlucker::SuckBootBlock()\n");
	
	// Set up the server address as the master HBC
	serverAddr.family = NU_FAMILY_IP;
	serverAddr.port = htons(PORT_BOOTBLOCK);
	memcpy(&serverAddr.id.is_ip_addrs, &ip_iSlot[Address::iSlotHbcMaster], 4);

	socketd = NU_Socket(NU_FAMILY_IP, NU_TYPE_STREAM, 0);
	if (socketd < 0)
	{
		Tracef("BootBlucker: Unable to obtain a socket (%d)\n", socketd);
	}
	else
	{
		// Keep trying to connect
		// We can't proceed with the boot unless we get this bootblock
		status = NU_Connect(socketd, &serverAddr, 0);
		while (status < 0)
		{
			Tracef("BootBlucker: Unable to connect to master - retrying(%d)\n", status);		
			NU_Sleep(300);
			status = NU_Connect(socketd, &serverAddr, 0);
		}
			
		// set socket to blocking operations
		NU_Fcntl(socketd, NU_SETFLAG, NU_BLOCK);
		
		// We have a connection to the master now
		// Get the bootblock
		status = NU_Recv(socketd, (char *)&recvblock, sizeof(bootblock_t), 0);

		// Clean up and close up shop
		NU_Close_Socket(socketd);
		
		// Currently we want to keep the cabinet, slot, and aPa, aP, aCb data that was
		// in the slave's bootblock.  The rest we can take from the transmitted master
		// block
		recvblock.b_slot = bootblock.b_slot;
		recvblock.b_type = bootblock.b_type;
		recvblock.b_memmap.iCabinet = bootblock.b_memmap.iCabinet;
		recvblock.b_memmap.iSlot = bootblock.b_memmap.iSlot;
		memcpy(recvblock.b_memmap.aPa, bootblock.b_memmap.aPa, 8);
		memcpy(recvblock.b_memmap.aP, bootblock.b_memmap.aP, 8);
		memcpy(recvblock.b_memmap.aCb, bootblock.b_memmap.aCb, 8);
		
		// Copy the newly invented and received block into our bootblock
		memcpy((void *)&bootblock, (void *)&recvblock, sizeof(bootblock_t));
		
		Tracef("Bootblock received from master\n");		
		
	}
	

}