//************************************************************************
// FILE:		SoftwareImageManager.h
//
// PURPOSE:		Defines the manager object that will be in charge of all
//				software image management in the SSAPI layer
//************************************************************************

#ifndef __SOFTWARE_IMAGE_MANAGER_H__
#define	__SOFTWARE_IMAGE_MANAGER_H__

#include "ObjectManager.h"
#include "SList.h"
#include "CmdSender.h"

#define	SOFTWARE_IMAGE_MANAGER_NAME		"SoftwareImageManager"

class Image;
class SoftwareDescriptor;
class SoftwareImage;
class Message;
class SsapiLocalResponder;
class SsapiResponder;

class SoftwareImageManager : public ObjectManager{
	
	static SoftwareImageManager				*m_pThis;
	bool									m_isIniting;
	SsapiLocalResponder						*m_pObjectAddedResponder;
	CmdSender								*m_pUMQueue;


//************************************************************************
// SoftwareImageManager:
//
// PURPOSE:		Default constructor
//************************************************************************

SoftwareImageManager( ListenManager *pLM, DdmServices *pParent );


public:


//************************************************************************
// Ctor:
//
// PURPOSE:		Creates the manager
//************************************************************************

static ObjectManager* Ctor(	ListenManager			*pLManager, 
							DdmServices				*pParent, 
							StringResourceManager	*pSRManager ){

	return m_pThis? m_pThis : m_pThis = new SoftwareImageManager( pLManager, pParent );
}

//************************************************************************
// GetName:
//
// PURPOSE:		Returns the name of the manager
//************************************************************************

virtual StringClass GetName() { return StringClass(SOFTWARE_IMAGE_MANAGER_NAME); }


//************************************************************************
// Dispatch:
//
// PURPOSE:		Responsible for dispatching a request. Must either let
//				the base class handle it (if that one can) or handle it
//				itself or .... fail it!
//************************************************************************

bool Dispatch( ValueSet *pRequestParms, U32 requestCode, SsapiResponder *pResponder);


protected:

//************************************************************************
// ObjectDeletedCallbackHandler:
//
// PURPOSE:		This method may be provided by subclasses to be notified
//				by events from other object managers. 
//************************************************************************

virtual void ObjectDeletedCallbackHandler( SSAPIEvent *pEvent , bool isLast );


//************************************************************************
// ObjectAddedEventCallback:
//
// PURPOSE:		This method is called when an object is added in to the
//				SSAPI framework
//************************************************************************

void ObjectAddedEventCallback( ValueSet *pVs, bool isLast, int eventObjectId );


private:


//************************************************************************
// QueryDescriptorsCallback:
//
// PURPOSE:		Callback for descriptor query
//
// FUNCTIONALITY:	If pContext == NULL, this is the first ever query, otherwise
//					pContext will point to the key (RowId) of the image that
//					needs to be updated. In such case, the key must be
//					deleted in this callback
//************************************************************************

STATUS QueryDescriptorsCallback( Message *pMsg_);


//************************************************************************
// CreateSoftwareDescriptor:
//
// PURPOSE:		Creates a SoftwareDescriptor object given an Image object
//************************************************************************

SoftwareDescriptor* CreateSoftwareDescriptor( const Image *pImage );


//************************************************************************
// CreateSoftwareImages:
//
// PURPOSE:		Creates software image objects and puts them into the 
//				container;
//************************************************************************

void CreateSoftwareImages( Image &image, Container &container, DesignatorId &descriptorId );


//************************************************************************
// ProcessAddImage:
// 
// PURPOSE:		Sends request to the UpgradeMaster
//************************************************************************

void ProcessAddImage( void *pImage, U32 imageSize, SsapiResponder *pResponder );
STATUS ProcessAddImageCallback( Message *pMsg_ );


//************************************************************************
// ProcessRemoveImage:
//
// PURPOSE:		Sends the command to the UM and does necessary tricks if the
//				request succeeds.
//************************************************************************

void ProcessRemoveImage( SoftwareDescriptor &descriptor, SsapiResponder *pResponder );
STATUS ProcessRemoveImageCallback( Message *pMsg_ );


//************************************************************************
// ProcessSetDefault:
//
// PURPOSE:		Sends the command to the UM
//************************************************************************

void ProcessSetDefault( SoftwareDescriptor &descriptor, SsapiResponder *pResponder );
STATUS ProcessSetDefaultCallback( Message *pMsg_ );


//************************************************************************
// ProcessSetPrimary:
//
// PURPOSE:		Sends the command to the UM
//************************************************************************

void ProcessSetPrimary( SoftwareImage &image, SsapiResponder *pResponder );
STATUS ProcessSetPrimaryCallback( Message *pMsg_ );


//************************************************************************
// GetPrimaryImageBySlot:
//
// PURPOSE:		Performs a lookup of the primary image per slot
//************************************************************************

SoftwareImage* GetPrimaryImageBySlot( int slotNumber );


//************************************************************************
// ProcessAssociateImage:
//
// PURPOSE:		Performs actions needed to associated a device with a 
//				descriptor.
//************************************************************************

void ProcessAssociateImage( SoftwareDescriptor &descriptor, DesignatorId &deviceId, SsapiResponder *pResponder );
STATUS ProcessAssociateImageCallback( Message *pMsg_ );


//************************************************************************
// ProcessUnassociateImage:
//
// PURPOSE:		Performs actions necessary to unassociate a devic with
//				a descriptor
//************************************************************************

void ProcessUnassociateImage( SoftwareDescriptor &descriptor, DesignatorId &deviceId, SsapiResponder *pResponder );
STATUS ProcessUnassociateImageCallback( Message *pMsg_ );


//************************************************************************
// UMCmdQInitReply:
//
// PURPOSE:		Callback for the UM command Q
//************************************************************************

void UMCmdQInitReply( STATUS rc ){  }


//************************************************************************
// UMEventHandler:
//
// PURPOSE:		CAlled by the UM command Q on command completetion and
//				to report events
//************************************************************************

void UMEventHandler( STATUS comletionCode, void *pStatusData );


//************************************************************************
// UMCommandCompletionCallback:
//
// PURPOSE:		Called back to for all commands submitted to the UM Q.
//************************************************************************

void UMCommandCompletionCallback(	STATUS			completionCode,
									void			*pResultData,
									void			*pCmdData,
									void			*pCmdContext );



//************************************************************************
// ProcessBootImage:
//
// PURPOSE:		Dispatches a request to the UM to boot an image
//************************************************************************

void ProcessBootImage( int slot, DesignatorId imageId, SsapiResponder *pResponder );


//************************************************************************
//************************************************************************
};

struct IMAGE_CELL{

	SsapiResponder		*pResponder;
	DesignatorId		imageId;
	DesignatorId		deviceId;

	IMAGE_CELL( DesignatorId id, SsapiResponder *p ){
		pResponder = p;
		imageId = id;
	}
};

#endif	// __SOFTWARE_IMAGE_MANAGER_H__