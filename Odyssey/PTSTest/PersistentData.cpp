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
// This module declares the tables for the dummy PersistentData service
// 
// Update Log: 
// 9/1/98 Joe Altmaier: Create file
/*************************************************************************/

#include "PersistentData.h"

	struct {VirtualDevice vd; long long offset; long long size;} PartitionParams={6, 0x800000, 0x1000000};
	struct {VirtualDevice vd;} TestParams={4};
	struct {long capacity;} NullParams={0x1800000};
	struct {long divisor;} TimerParams={10000};

	char *aTag[]={
		"/Device/VirtualDevice00", // PTS
		"/Device/VirtualDevice01", // DdmManager
		"/Device/VirtualDevice02", // Transport
		"/Device/VirtualDevice03", // DdmTimer
		"/Device/VirtualDevice04",
		"/Device/VirtualDevice05",
		"/Device/VirtualDevice06",
		0
		};
	void *aPValue[]={
		0,
		0,
		0,
		&TimerParams,
		&PartitionParams,
		&TestParams,
		&NullParams,
		};
	int aSize[]={
		0,
		0,
		0,
		sizeof(TimerParams),
		sizeof(PartitionParams),
		sizeof(TestParams),
		sizeof(NullParams),
		};
