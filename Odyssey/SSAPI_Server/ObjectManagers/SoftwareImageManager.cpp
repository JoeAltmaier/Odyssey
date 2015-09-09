//************************************************************************
// FILE:		SoftwareImageManager.cpp
//
// PURPOSE:		Implements the manager object that will be in charge of all
//				software image management in the SSAPI layer
//************************************************************************



#include "SoftwareImageManager.h"
#include "UpgradeMasterMessages.h"
#include "UpgradeMasterCommands.h"
#include "UpgradeEvents.h"
#include "SoftwareDescriptorObjects.h"
#include "SoftwareImage.h"
#include "DeviceManager.h"
#include "SlotMap.h"
#include "SsapiLocalResponder.h"
#include "ListenManager.h"
#include "Ssapievents.h"
#include "ClassTypeMap.h"
#include "Iop.h"

#include "Trace_Index.h"
#ifdef TRACE_INDEX
#undef TRACE_INDEX
#endif
#define TRACE_INDEX TRACE_SSAPI_MANAGERS

SoftwareImageManager* SoftwareImageManager::m_pThis = NULL;


//************************************************************************
// SoftwareImageManager:
//
// PURPOSE:		Default constructor
//************************************************************************

SoftwareImageManager::SoftwareImageManager( ListenManager *pLM, DdmServices *pParent )
:ObjectManager(pLM, DesignatorId(RowId(), SSAPI_MANAGER_CLASS_TYPE_SOFTWARE_UPDATE_MANAGER), pParent ){

	SetIsReadyToServiceRequests( false );
	m_isIniting = true;

	// query descriptors
	Send( new MsgQueryImages(), NULL, REPLYCALLBACK(SoftwareImageManager, QueryDescriptorsCallback ) );

	SSAPI_TRACE( TRACE_L2, "\nSoftwareImageManager: Initializing...");

	// register for OBJECT_ADDED events
	m_pObjectAddedResponder = new SsapiLocalResponder( this, (LOCAL_EVENT_CALLBACK)METHOD_ADDRESS(SoftwareImageManager,ObjectAddedEventCallback) ); 
	GetListenManager()->AddListenerForObjectAddedEvent( SSAPI_LISTEN_OWNER_ID_ANY, m_pObjectAddedResponder->GetSessionID(), CALLBACK_METHOD(m_pObjectAddedResponder, 1) );
	
	// Initialize communication channels (command Q) with the UM Master
	m_pUMQueue = new CmdSender(UPMSTR_CMD_QUEUE_TABLE,
								this);
	m_pUMQueue->csndrInitialize((pInitializeCallback_t)METHOD_ADDRESS(SoftwareImageManager,UMCmdQInitReply));
	m_pUMQueue->csndrRegisterForEvents((pEventCallback_t)METHOD_ADDRESS(SoftwareImageManager,UMEventHandler));
}



//************************************************************************
// Dispatch:
//
// PURPOSE:		Responsible for dispatching a request. Must either let
//				the base class handle it (if that one can) or handle it
//				itself or .... fail it!
//************************************************************************

bool
SoftwareImageManager::Dispatch( ValueSet *pRequestParms, U32 requestCode, SsapiResponder *pResponder){
	
	void				*pImage;
	U32					imageSize, i;
	DesignatorId		id;
	SoftwareDescriptor	*pDescriptor;
	SoftwareImage		*pSoftwareImage;
	DeviceManager		*pDManager;
	int					slot;
	bool				found;

	if( ObjectManager::Dispatch( pRequestParms, requestCode, pResponder ) )
		return true;
	
	switch( requestCode ){
		case SSAPI_SW_UPDATE_MANAGER_ADD_IMAGE:
			pRequestParms->GetU32( SSAPI_SW_UPDATE_MANAGER_ADD_IMAGE_BINARY_BLOB_SIZE, &imageSize );
			pImage = new (tBIG) char[ imageSize ];
			pRequestParms->GetGenericValue( (char *)pImage, imageSize, SSAPI_SW_UPDATE_MANAGER_ADD_IMAGE_BINARY_BLOB );
			ProcessAddImage( pImage, imageSize, pResponder );
			delete [] pImage;
			break;

		case SSAPI_SW_UPDATE_MANAGER_REMOVE_IMAGE:
			pRequestParms->GetGenericValue( (char *)&id, sizeof(id), SSAPI_SW_UPDATE_MANAGER_REMOVE_IMAGE_IMAGE_DESCRIPTOR_ID );
			pDescriptor = (SoftwareDescriptor *)GetManagedObject( &id );
			if( !pDescriptor ){
				pResponder->RespondToRequest( SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_INVALID_PARAM_EXCEPTION );
				return true;
			}	
			ProcessRemoveImage( *pDescriptor, pResponder );
			break;

		case SSAPI_SW_UPDATE_MANAGER_SET_DEFAULT:
			pRequestParms->GetGenericValue( (char *)&id, sizeof(id), SSAPI_SW_UPDATE_MANAGER_SET_DEFAULT_DESCRIPTOR_ID );
			pDescriptor = (SoftwareDescriptor *)GetManagedObject( &id );
			if( !pDescriptor ){
				pResponder->RespondToRequest( SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_INVALID_PARAM_EXCEPTION );
				return true;
			}	
			ProcessSetDefault( *pDescriptor, pResponder );
			break;

		case SSAPI_SW_UPDATE_MANAGER_SET_PRIMARY:
			pRequestParms->GetGenericValue( (char *)&id, sizeof(id), SSAPI_SW_UPDATE_MANAGER_SET_PRIMARY_IMAGE_ID );
			pSoftwareImage = (SoftwareImage *)GetManagedObject( &id );
			if( !pSoftwareImage ){
				pResponder->RespondToRequest( SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_INVALID_PARAM_EXCEPTION );
				return true;
			}	
			ProcessSetPrimary( *pSoftwareImage, pResponder );
			break;

		case SSAPI_SW_UPDATE_MANAGER_ASSOCIATE_IMAGE:
			pRequestParms->GetGenericValue( (char *)&id, sizeof(id), SSAPI_SW_UPDATE_MANAGER_ASSOCIATE_IMAGE_DESCRIPTOR_ID );
			pDescriptor = (SoftwareDescriptor *)GetManagedObject( &id );
			if( !pDescriptor ){
				pResponder->RespondToRequest( SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_INVALID_PARAM_EXCEPTION );
				return true;
			}
			pRequestParms->GetGenericValue( (char *)&id, sizeof(id), SSAPI_SW_UPDATE_MANAGER_ASSOCIATE_IMAGE_DEVICE_ID );
			pDManager = (DeviceManager *)GetObjectManager( SSAPI_MANAGER_CLASS_TYPE_DEVICE_MANAGER );
			if( !pDManager->IsThereSuchDevice( id ) ){
				pResponder->RespondToRequest( SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_INVALID_PARAM_EXCEPTION );
				return true;
			}
			ProcessAssociateImage( *pDescriptor, id, pResponder );
			break;

		case SSAPI_SW_UPDATE_MANAGER_UNASSOCIATE_IMAGE:
			pRequestParms->GetGenericValue( (char *)&id, sizeof(id), SSAPI_SW_UPDATE_MANAGER_UNASSOCIATE_IMAGE_DESCRIPTOR_ID );
			pDescriptor = (SoftwareDescriptor *)GetManagedObject( &id );
			if( !pDescriptor ){
				pResponder->RespondToRequest( SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_INVALID_PARAM_EXCEPTION );
				return true;
			}
			pRequestParms->GetGenericValue( (char *)&id, sizeof(id), SSAPI_SW_UPDATE_MANAGER_UNASSOCIATE_IMAGE_DEVICE_ID );
			pDManager = (DeviceManager *)GetObjectManager( SSAPI_MANAGER_CLASS_TYPE_DEVICE_MANAGER );
			if( !pDManager->IsThereSuchDevice( id ) ){
				pResponder->RespondToRequest( SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_INVALID_PARAM_EXCEPTION );
				return true;
			}
			ProcessUnassociateImage( *pDescriptor, id, pResponder );
			break;

		case SSAPI_SW_UPDATE_MANAGER_BOOT_PRIMARY_IMAGE:
			pRequestParms->GetInt( SSAPI_SW_UPDATE_MANAGER_BOOT_PRIMARY_IMAGE_SLOT_NUMBER, &slot );
			//find the primary image for that slot
			found = false;
			for( i = 0; i < GetManagedObjectCount(); i++ ){
				if( GetManagedObject(i)->GetClassType() == SSAPI_OBJECT_CLASS_TYPE_SOFTWARE_IMAGE ){
					pSoftwareImage = (SoftwareImage *)GetManagedObject( i );
					if( (pSoftwareImage->GetSlotNumber() == slot) && pSoftwareImage->GetIsPrimary() ){
						ProcessBootImage( slot, pSoftwareImage->GetDescriptorId(), pResponder );
						found = true;
					}
				}
			}
			if( !found )
				pResponder->RespondToRequest( SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_INVALID_PARAM_EXCEPTION );
			break;

		case SSAPI_SW_UPDATE_MANAGER_BOOT_PRIMARY_ALTERNATE_IMAGE:
			pRequestParms->GetInt( SSAPI_SW_UPDATE_MANAGER_BOOT_PRIMARY_ALTERNATE_IMAGE_SLOT_NUMBER, &slot );
			//find the alternate image for that slot
			found = false;
			for( i = 0; i < GetManagedObjectCount(); i++ ){
				if( GetManagedObject(i)->GetClassType() == SSAPI_OBJECT_CLASS_TYPE_SOFTWARE_IMAGE ){
					pSoftwareImage = (SoftwareImage *)GetManagedObject( i );
					if( (pSoftwareImage->GetSlotNumber() == slot) && !pSoftwareImage->GetIsPrimary() ){
						ProcessBootImage( slot, pSoftwareImage->GetDescriptorId(), pResponder );
						found = true;
					}
				}
			}
			if( !found )
				pResponder->RespondToRequest( SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_INVALID_PARAM_EXCEPTION );
			break;
	}

	return true;
}


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

STATUS 
SoftwareImageManager::QueryDescriptorsCallback( Message *pMsg_ ){

	MsgQueryImages		*pMsg	= (MsgQueryImages *)pMsg_;
	RowId				*pRowId = (RowId *)pMsg->GetContext();
	ImageIterator		*pIterator = NULL;
	const Image			*pImage;
	SoftwareDescriptor	*pDescriptor;
	SList				container, temp;
	DesignatorId		id;
	CONTAINER_ELEMENT	element;

	pMsg->GetImages( &pIterator );

	if( !pRowId ){	// full query
		for( pImage = pIterator->GetFirst(); pImage; pImage = pIterator->GetNext() ){
			pDescriptor = CreateSoftwareDescriptor( pImage );
			id = pDescriptor->GetDesignatorId();
			container.Add( (CONTAINER_ELEMENT) pDescriptor );
			CreateSoftwareImages( (Image &)*pImage, temp, id );
			// copy pointers
			while( temp.Count() ){
				temp.GetAt( element, 0 );
				temp.RemoveAt( 0 );
				container.AddAt( element, container.Count() );
			}
		}
		AddObjectsIntoManagedObjectsVector( container );

		if( m_isIniting ){
			m_isIniting = false;
			SSAPI_TRACE( TRACE_L2, "\nSoftwareImageManager: Done! Objects built: ", GetManagedObjectCount());
			SetIsReadyToServiceRequests( true );
		}
	}
	else {		// query for specific image descriptor
		for( pImage = pIterator->GetFirst(); pImage; pImage = pIterator->GetNext() ){
			if( *pRowId == pImage->GetKey() ){
				pDescriptor = CreateSoftwareDescriptor( pImage );
				id = pDescriptor->GetDesignatorId();
				container.Add( (CONTAINER_ELEMENT) pDescriptor );
				CreateSoftwareImages( (Image &)*pImage, temp, id );
				// copy pointers
				while( temp.Count() ){
					temp.GetAt( element, 0 );
					temp.RemoveAt( 0 );
					container.AddAt( element, container.Count() );
				}
				break;
			}
		}
		AddObjectsIntoManagedObjectsVector( container );
		delete pRowId;
	}

	delete pIterator;
	delete pMsg;
	return OK;
}


//************************************************************************
// CreateSoftwareDescriptor:
//
// PURPOSE:		Creates a SoftwareDescriptor object given an Image object
//************************************************************************

SoftwareDescriptor* 
SoftwareImageManager::CreateSoftwareDescriptor( const Image *pImage ){

	SoftwareDescriptor		*pDescriptor = NULL;

	switch( pImage->GetType() ){
		case HBC_IMAGE:
			pDescriptor = new SoftwareDescriptorHbc( GetListenManager(), this );
			break;

		case NAC_IMAGE:
			pDescriptor = new SoftwareDescriptorNac( GetListenManager(), this );
			break;

		case SSD_IMAGE:
			pDescriptor =  new SoftwareDescriptorSsd( GetListenManager() ,this );
			break;

		default:
			ASSERT(0);
			break;
	}

	pDescriptor->BuildYourselfFromImageObject( (Image &)*pImage );

	return pDescriptor;
}


//************************************************************************
// CreateSoftwareImages:
//
// PURPOSE:		Creates software image objects and puts them into the 
//				container;
//************************************************************************

void 
SoftwareImageManager::CreateSoftwareImages( Image &image, Container &container, DesignatorId &descriptorId ){

	DeviceManager	*pDManager = (DeviceManager *)GetObjectManager(SSAPI_MANAGER_CLASS_TYPE_DEVICE_MANAGER);
	SlotMap			map;
	int				slotNumber;
	DesignatorId	id;
	SoftwareImage	*pImage;
	ImageDesc		*pAssociation;

	for( slotNumber = SlotMap::GetMinSlotNumber(); slotNumber <= SlotMap::GetMaxSlotNumber(); slotNumber++ ){
		id = pDManager->GetIopBySlot( slotNumber );
		pAssociation = (ImageDesc *)image.GetAssociation( (TySlot)slotNumber );
		if( pAssociation && (id != DesignatorId()) ){
			pImage = new SoftwareImage( GetListenManager(), this );
			pImage->BuildYourselfFromImageDesc(	*pAssociation,
												slotNumber,
												descriptorId,
												id );

			container.Add( (CONTAINER_ELEMENT)pImage );
		}
	}
}


//************************************************************************
// ProcessAddImage:
// 
// PURPOSE:		Sends request to the UpgradeMaster
//************************************************************************

void 
SoftwareImageManager::ProcessAddImage( void *pImage, U32 imageSize, SsapiResponder *pResponder ){

	STATUS rc;

	rc = Send(	new MsgAddImage( imageSize, pImage ), 
				pResponder, 
				REPLYCALLBACK(SoftwareImageManager, ProcessAddImageCallback) );
	
	if( rc == OK )
		SetIsReadyToServiceRequests( false );
	else
		pResponder->RespondToRequest( SSAPI_EXCEPTION_INTERNAL, CTS_SSAPI_INTERNAL_EXCEPTION_CMD_FAILED );

	ASSERT( rc == OK );
}


STATUS 
SoftwareImageManager::ProcessAddImageCallback( Message *pMsg_ ){

	MsgAddImage		*pMsg = (MsgAddImage *)pMsg_;
	SsapiResponder	*pResponder = (SsapiResponder *)pMsg->GetContext();
		
	if( pMsg->Status() != OK )
		pResponder->RespondToRequest( SSAPI_EXCEPTION_INTERNAL, pMsg->Status() );
	else{ 	// query for complete info
		Send( new MsgQueryImages(), new RowId(pMsg->GetImageKey()), REPLYCALLBACK(SoftwareImageManager, QueryDescriptorsCallback ) );
		pResponder->RespondToRequest( SSAPI_RC_SUCCESS );
	}

	SetIsReadyToServiceRequests( true );
	delete pMsg;
	return OK;
}


//************************************************************************
// ProcessRemoveImage:
//
// PURPOSE:		Sends the command to the UM and does necessary tricks if the
//				request succeeds.
//************************************************************************

void 
SoftwareImageManager::ProcessRemoveImage( SoftwareDescriptor &descriptor, SsapiResponder *pResponder ){

	IMAGE_CELL		*pCell = new IMAGE_CELL( descriptor.GetDesignatorId(), pResponder );
	CmdDeleteImage	*pObj = new CmdDeleteImage( descriptor.GetDesignatorId().GetRowId() );

	pObj->Send( m_pUMQueue, 
				(pCmdCompletionCallback_t)METHOD_ADDRESS( SoftwareImageManager, UMCommandCompletionCallback ),
				pCell );
	delete pObj;
}


//************************************************************************
// ProcessSetDefault:
//
// PURPOSE:		Sends the command to the UM
//************************************************************************

void 
SoftwareImageManager::ProcessSetDefault( SoftwareDescriptor &descriptor, SsapiResponder *pResponder ){

	IMAGE_CELL	*pCell = new IMAGE_CELL( descriptor.GetDesignatorId(), pResponder );
	CmdAssignDefaultImage *pObj = new CmdAssignDefaultImage( descriptor.GetDesignatorId().GetRowId() );
	
	pObj->Send(	m_pUMQueue,
				(pCmdCompletionCallback_t)METHOD_ADDRESS( SoftwareImageManager, UMCommandCompletionCallback ),
				pCell );

	delete pObj;
}


//************************************************************************
// ProcessSetPrimary:
//
// PURPOSE:		Sends the command to the UM
//************************************************************************

void 
SoftwareImageManager::ProcessSetPrimary( SoftwareImage &image, SsapiResponder *pResponder ){

	IMAGE_CELL			*pCell = new IMAGE_CELL( image.GetDesignatorId(), pResponder );
	DesignatorId		id = image.GetDescriptorId();
	SoftwareDescriptor	*pDescriptor = (SoftwareDescriptor *)GetManagedObject( &id );

	ASSERT( pDescriptor );
	CmdMakePrimary		*pObj = new CmdMakePrimary( (TySlot)image.GetSlotNumber(), pDescriptor->GetDesignatorId().GetRowId() );

	pObj->Send(	m_pUMQueue,
				(pCmdCompletionCallback_t)METHOD_ADDRESS( SoftwareImageManager, UMCommandCompletionCallback ),
				pCell );
	delete pObj;
}


//************************************************************************
// GetPrimaryImageBySlot:
//
// PURPOSE:		Performs a lookup of the primary image per slot
//************************************************************************

SoftwareImage* 
SoftwareImageManager::GetPrimaryImageBySlot( int slotNumber ){

	U32				i;
	ManagedObject	*pObj;
	SoftwareImage	*pImage;

	for( i = 0; i < GetManagedObjectCount(); i++ ){
		pObj = GetManagedObject( i );
		if( pObj->GetClassType() == SSAPI_OBJECT_CLASS_TYPE_SOFTWARE_IMAGE ){
			pImage = (SoftwareImage *)pObj;
			if( ( pImage->GetSlotNumber() == slotNumber ) && pImage->GetIsPrimary() )
				return pImage;
		}
	}
	return NULL;
}


//************************************************************************
// ProcessAssociateImage:
//
// PURPOSE:		Performs actions needed to associated a device with a 
//				descriptor.
//************************************************************************

void 
SoftwareImageManager::ProcessAssociateImage( SoftwareDescriptor &descriptor, DesignatorId &deviceId, SsapiResponder *pResponder ){

	IMAGE_CELL		*pCell = new IMAGE_CELL( descriptor.GetDesignatorId(), pResponder );
	U32				i;
	TySlot			slot;
	bool			found = false;
	DeviceManager	*pDM = (DeviceManager *)GetObjectManager( SSAPI_MANAGER_CLASS_TYPE_DEVICE_MANAGER );

	// Find the slot of this IOP
	for( i = SlotMap::GetMinSlotNumber(); i <= SlotMap::GetMaxSlotNumber(); i++ ){
		if( pDM->GetIopBySlot( i ) == deviceId ){
			slot = (TySlot)i;
			found = true;
			break;
		}
	}

	ASSERT( found );
	if( !found ){
		pResponder->RespondToRequest( SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_INVALID_PARAM_EXCEPTION );
		delete pCell;
		return;
	}
	
	CmdAssociateImage *pObj = new CmdAssociateImage( descriptor.GetDesignatorId().GetRowId(), slot );
	pObj->Send(	m_pUMQueue,
				(pCmdCompletionCallback_t)METHOD_ADDRESS( SoftwareImageManager, UMCommandCompletionCallback ),
				pCell );
	delete pObj;
}



//************************************************************************
// ProcessUnassociateImage:
//
// PURPOSE:		Performs actions necessary to unassociate a devic with
//				a descriptor
//************************************************************************

void 
SoftwareImageManager::ProcessUnassociateImage( SoftwareDescriptor &descriptor, DesignatorId &deviceId, SsapiResponder *pResponder ){

	IMAGE_CELL		*pCell = new IMAGE_CELL( descriptor.GetDesignatorId(), pResponder );
	U32				i;
	TySlot			slot;
	bool			found = false;
	DeviceManager	*pDM = (DeviceManager *)GetObjectManager( SSAPI_MANAGER_CLASS_TYPE_DEVICE_MANAGER );

	// Find the slot of this IOP
	for( i = SlotMap::GetMinSlotNumber(); i <= SlotMap::GetMaxSlotNumber(); i++ ){
		if( pDM->GetIopBySlot( i ) == deviceId ){
			slot = (TySlot)i;
			found = true;
			break;
		}
	}

	ASSERT( found );
	if( !found ){
		pResponder->RespondToRequest( SSAPI_EXCEPTION_INVALID_PARAMETER, CTS_SSAPI_INVALID_PARAM_EXCEPTION );
		delete pCell;
		return;
	}

	pCell->deviceId = deviceId;
	CmdUnassociateImage *pCmd = new CmdUnassociateImage( descriptor.GetDesignatorId().GetRowId(), slot );
	pCmd->Send(	m_pUMQueue,
				(pCmdCompletionCallback_t)METHOD_ADDRESS( SoftwareImageManager, UMCommandCompletionCallback ),
				pCell );
	delete pCmd;
}


//************************************************************************
// ObjectAddedEventCallback:
//
// PURPOSE:		This method is called when an object is added in to the
//				SSAPI framework
//************************************************************************

void 
SoftwareImageManager::ObjectAddedEventCallback( ValueSet *pVs, bool isLast, int eventObjectId ){

	SSAPIEvent				*pEvent = new SSAPIEventObjectAdded( NULL );
	ValueSet				*pChild = new ValueSet;
	ClassTypeMap			map;
	int						classType;

	*pEvent = *(ValueSet *)pVs->GetValue( eventObjectId );
	*pChild = *(ValueSet *)pEvent->GetValue( SSAPI_EVENT_FID_MANAGED_OBJECT );
	pChild->GetInt( SSAPI_OBJECT_FID_OBJECT_CLASS_TYPE, &classType );

	// if this is an IOP, get images for it!
	if( map.IsADerivedClass( SSAPI_OBJECT_CLASS_TYPE_IOP, classType, true ) )
		// run a full query ---- hacky, wacky, but the only thing I can do....
		Send( new MsgQueryImages(), NULL, REPLYCALLBACK(SoftwareImageManager, QueryDescriptorsCallback ) );

	delete pEvent;
	delete pChild;
}


//************************************************************************
// ObjectDeletedCallbackHandler:
//
// PURPOSE:		This method may be provided by subclasses to be notified
//				by events from other object managers. 
//************************************************************************

void 
SoftwareImageManager::ObjectDeletedCallbackHandler( SSAPIEvent *pEvent , bool isLast ){

	ValueSet				*pChild = new ValueSet;
	ClassTypeMap			map;
	int						classType;
	Iop						*pIop;
	SoftwareImage			*pImage;
	U32						i;
	SList					container;

	*pChild = *(ValueSet *)pEvent->GetValue( SSAPI_EVENT_FID_MANAGED_OBJECT );
	pChild->GetInt( SSAPI_OBJECT_FID_OBJECT_CLASS_TYPE, &classType );

	// if this is an IOP, remove images for it!
	if( map.IsADerivedClass( SSAPI_OBJECT_CLASS_TYPE_IOP, classType, true ) ){
		pIop = (Iop *)pChild;
		for( i = 0; i < GetManagedObjectCount(); i++ ){
			if( GetManagedObject(i)->GetClassType() == SSAPI_OBJECT_CLASS_TYPE_SOFTWARE_IMAGE ){
				pImage = (SoftwareImage *)GetManagedObject(i);
				if( pImage->GetSlotNumber() == pIop->GetSlotNumber() )
					container.Add( (CONTAINER_ELEMENT)pImage );
			}
		}
		DeleteObjectsFromTheSystem( container );
	}
		
	delete pChild;
}


//************************************************************************
// UMEventHandler:
//
// PURPOSE:		CAlled by the UM command Q on command completetion and
//				to report events
//************************************************************************

void 
SoftwareImageManager::UMEventHandler( STATUS eventCode, void *pEvent ){

	SoftwareDescriptor	*pDescriptor;
	SoftwareImage		*pImage1, *pImage2;
	SList				container;
	DesignatorId		id, deviceId;
	U32					i;
	DeviceManager		*pDM = (DeviceManager *)GetObjectManager( SSAPI_MANAGER_CLASS_TYPE_DEVICE_MANAGER );


	// of not initialized -- ignore, we'll get the latest data when we get
	// the query back
	if( m_isIniting )
		return;

	switch( eventCode ){
		case EVT_IMAGE_DELETED:
		{
				DeleteImageEvent *pEventDeleted = (DeleteImageEvent *)pEvent;
				GetDesignatorIdByRowId( pEventDeleted->GetImageKey(), id );
				if( (pDescriptor = (SoftwareDescriptor *)GetManagedObject( &id ) ) != NULL ){
					container.Add( (CONTAINER_ELEMENT)pDescriptor );
					DeleteObjectsFromTheSystem( container );
				}
				else
					ASSERT(0);
				break;
		}

		case EVT_DEFAULT_IMAGE_ASSIGNED:
		{
			AssignDefaultEvent *pEventDefault = (AssignDefaultEvent *)pEvent;
			GetDesignatorIdByRowId( pEventDefault->GetImageKey(), id );
			if( (pDescriptor = (SoftwareDescriptor *)GetManagedObject( &id ) ) != NULL ){
				// find the one that used be default
				for( i = 0; i < GetManagedObjectCount(); i++ ){
					if( GetManagedObject(i)->GetClassType() == pDescriptor->GetClassType() )
						if( ((SoftwareDescriptor *)GetManagedObject(i))->GetIsDefault() )
							((SoftwareDescriptor *)GetManagedObject(i))->SetIsDefault( false, true );
				}
				pDescriptor->SetIsDefault( true, true );
			}
			else
				ASSERT(0);
			break;
		}

		case EVT_MADE_PRIMARY:
		{
			MakePrimaryEvent *pEventPrimary = (MakePrimaryEvent *)pEvent;
			pImage1 = GetPrimaryImageBySlot( pEventPrimary->GetSlot() );
			if( pImage1 )
				pImage1->SetIsPrimary( false, true );
			GetDesignatorIdByRowId( pEventPrimary->GetImageKey(), id );
			// now find the new one...
			for( i = 0; i < GetManagedObjectCount(); i++ )
				if( GetManagedObject( i )->GetClassType() == SSAPI_OBJECT_CLASS_TYPE_SOFTWARE_IMAGE ){
					pImage2 = (SoftwareImage *)GetManagedObject( i );
					if( pImage2->GetDescriptorId() == id ){
						pImage2->SetIsAccepted( true, false );
						pImage2->SetIsPrimary( true, true );
					}
				}
			break;
		}

		case EVT_IMAGE_UNASSOCIATED:
		{
			UnassociateImageEvent *pEventUnass = (UnassociateImageEvent *)pEvent;
			GetDesignatorIdByRowId( pEventUnass->GetImageKey(), id );
			deviceId = pDM->GetIopBySlot( pEventUnass->GetSlot() );
			// find and delete that association
			for( i = 0 ; i < GetManagedObjectCount(); i++ ){
				if( GetManagedObject( i )->GetClassType() == SSAPI_OBJECT_CLASS_TYPE_SOFTWARE_IMAGE ){
					pImage1 = (SoftwareImage *)GetManagedObject( i );
					if( (pImage1->GetDescriptorId() == id) && (pImage1->GetDeviceId() == deviceId) ){
						container.Add( (CONTAINER_ELEMENT)pImage1 );
						break;
					}
				}
			}
			DeleteObjectsFromTheSystem( container );
			Send( new MsgQueryImages(), new RowId(pEventUnass->GetImageKey()), REPLYCALLBACK(SoftwareImageManager, QueryDescriptorsCallback ) );
			break;
		}

		case EVT_IMAGE_ASSOCIATED:
		{
			AssociateImageEvent *pEventAss = (AssociateImageEvent *)pEvent;
			Send( new MsgQueryImages(), new RowId(pEventAss->GetImageKey()), REPLYCALLBACK(SoftwareImageManager, QueryDescriptorsCallback ) );
			break;
		}

		case EVT_IMAGE_BOOTED:
		{
			BootImageEvent	*pEventBoot = (BootImageEvent *)pEvent;
			// send a query to get new current attribute values
			Send( new MsgQueryImages(), new RowId(pEventBoot->GetImageKey()), REPLYCALLBACK(SoftwareImageManager, QueryDescriptorsCallback ) );
			break;
		}
		default:
			ASSERT(0);
			break;
	}
}


//************************************************************************
// UMCommandCompletionCallback:
//
// PURPOSE:		Called back to for all commands submitted to the UM Q.
//************************************************************************

void 
SoftwareImageManager::UMCommandCompletionCallback(	STATUS			completionCode,
													void			*pResultData,
													void			*pCmdData,
													void			*pCmdContext ){
	SsapiResponder *pResponder;
	CmdInfoBase *pCmd = (CmdInfoBase *)new CmdInfoBase( pCmdData );
	IMAGE_CELL	*pCell = (IMAGE_CELL *)pCmdContext;

	switch( pCmd->GetOpcode() ){
		case CMND_DELETE_IMAGE:
		case CMND_ASSIGN_DEFAULT:
		case CMND_MAKE_PRIMARY:
		case CMND_ASSOCIATE_IMAGE:
		case CMND_UNASSOCIATE_IMAGE:
		case CMND_BOOT_IMAGE:
			pResponder = pCell->pResponder;
			delete pCell;
			if( completionCode == OK )
				pResponder->RespondToRequest( SSAPI_RC_SUCCESS );
			else
				pResponder->RespondToRequest( SSAPI_EXCEPTION_INTERNAL, completionCode );
			break;


		default:
			ASSERT(0);
			break;
	}
	
	delete pCmd;
}


//************************************************************************
// ProcessBootImage:
//
// PURPOSE:		Dispatches a request to the UM to boot an image
//************************************************************************

void 
SoftwareImageManager::ProcessBootImage( int slot, DesignatorId imageId, SsapiResponder *pResponder ){

	IMAGE_CELL		*pCell = new IMAGE_CELL( imageId, pResponder );
	CmdBootImage	*pObj = new CmdBootImage( (TySlot)slot, imageId.GetRowId() );

	pObj->Send(	m_pUMQueue,
				(pCmdCompletionCallback_t)METHOD_ADDRESS( SoftwareImageManager, UMCommandCompletionCallback ),
				pCell );
	delete pObj;
}

