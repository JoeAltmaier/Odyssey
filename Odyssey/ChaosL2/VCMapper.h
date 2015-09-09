/*************************************************************************
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
*	This file contains the declaration of the Virtual Circuit Mapper class
*   This class listens to the Virtual Device Table and builds and maintains
*   maps of all virtual circuits in the system.  This information does not
*	persist.  It is rebuilt on each boot and after failover.
* 
*
*************************************************************************/

#ifndef VCMapper_h
#define VCMapper_h

#include "DdmOsServices.h"
#include "VdnHash_T.h"

typedef void (DdmServices::*VcMapperCallback)(STATUS);
#ifdef WIN32
#	define VCMAPPERCALLBACK(clas,method)	(VcMapperCallback) method
#elif defined(__ghs__) // Green Hills
#	define VCMAPPERCALLBACK(clas,method)	(VcMapperCallback) &clas::method
#else	// MetroWerks
#	define VCMAPPERCALLBACK(clas,method)	(VcMapperCallback)&method
#endif

class VCMapper : public DdmServices
{

public:

   //  Constructor
	VCMapper (DdmServices *pParentDdm_);
	// D'tor 
	~VCMapper();

	// Data stored in the VDN hash table
	struct VdnData
	{
		VdnData() {}
		VdnData( const VdnData &vd) 
		{ *this = vd; }
		const VdnData &operator=(const VdnData &vd)
		{
			if (&vd != this)
			{
				vdnList = vd.vdnList;
				didPrimary = vd.didPrimary;
				didSecondary = vd.didSecondary;
				slotPrimary = vd.slotPrimary;
				slotSecondary = vd.slotSecondary;
				
			}
			return *this;
		}
		Array_T<VDN> vdnList;
		DID didPrimary, didSecondary;
		TySlot slotPrimary, slotSecondary;
	};
	
	// Get all the necessary info about the supplied VDN, including 
	// an array containing all VDNs that the supplied VDN talks to
	// (if it is in the config data, it is considered to be talked to)
	// and, if the VDN is IOP Local, which slot it is on.
	const VdnData &GetVdnData(const VDN &vdn_) const;

	// Get a list of VDNs on the specified slot.  Does not return IOP_LOCALs
	Array_T<VDN> GetVdnList(TySlot slot_) const;
	
	void Stop(VcMapperCallback pVcmCallback_);

	

private: 

	STATUS ProcessEnumerateClassTableReply(Message *pMsg_);
	STATUS ProcessVirtualDeviceTableListenReply(Message *pMsg_);
	STATUS ProcessStopListenReply(Message *pMsg_);
	STATUS ProcessVirtualMasterGetConfigReply(Message *pMsg_);

	VdnData vdEmpty;  // an empty array returned if a VDN has no dependents
	VdnHash_T<VdnData> htVdn;
	
	U32 listenerID;
	
}; 


#endif


/*************************************************************************/
// Update Log:
//	$Log: /Gemini/Odyssey/ChaosL2/VCMapper.h $
// 
// 1     12/09/99 1:45a Iowa
// 1     10/3/1999 		Bbutler
//		Created
/*************************************************************************/
