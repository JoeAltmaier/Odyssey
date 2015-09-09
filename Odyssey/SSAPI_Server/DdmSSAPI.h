//DdmSSAPI.h

#ifndef __DdmSSAPI_H
#define __DdmSSAPI_H

#include "OsTypes.h"
#include "Message.h"
#include "DdmMaster.h"
#include "SsapiRequestMessage.h"
#include "SsapiResponder.h"
#include "utils\SsapiGateway.h"
#include "utils\StringResourceManager.h"
#include "ObjectManager.h"
#include "UserManager.h"
#include "DeviceManager.h"
#include "ListenManager.h"
#include "PHSDataManager.h"
#include "HostManager.h"
#include "LunMapManager.h"
#include "StorageManager.h"
#include "ProcessManager.h"
#include "ConfigIdManager.h"
#include "LogManager.h"
#include "AlarmManager.h"
#include "TableStuffManager.h"
#include "SoftwareImageManager.h"
#include "ProfileManager.h"
#include "ConnectionManager.h"


class Container;

class ObjectTableElement {
public:
	ObjectTableElement() {
		m_pObjectManager = NULL;
		m_iObjectRequestCode = 0;
	}
	
	~ObjectTableElement() {
		if(m_pObjectManager)
			delete m_pObjectManager;
	}
	
	ObjectManager* m_pObjectManager;
	short int m_iObjectRequestCode;
};

class DdmSSAPI : public DdmMaster{

	friend class ObjectManager;

public:
	static Ddm* Ctor(DID did);
	DdmSSAPI(DID did);

	STATUS Initialize(Message* pMsg);
	STATUS Enable(Message* pMsg);
	void SetManagerReady( U32 managerClassType, bool ready = true );

	// methods needed by ObjectManagers to rap with each other (AG)
	ObjectManager* GetObjectManager( U32 managerClassType );
	
	//command handlers

	STATUS ProcessRequest(Message* pReqMsg);

	ListenManager* m_pListenManager;

	ObjectTableElement** m_apObjectTable;
	int m_iObjectTableSize;
	typedef ObjectManager* (*SSAPI_MANAGER_CTOR)(ListenManager*, DdmOsServices*, StringResourceManager*);
private:

	// A well-known routine to inform managers
	void HandleSessionExpiredEvent( SESSION_ID sessionId );
	STATUS StringResourceManagerInitCallback( void*, STATUS ){ return OK; }

	StringResourceManager		*m_pStringResourceManager;
	Container					*m_pRequestQueue;		// queue of outstanding requests
	Container					*m_pNotReadyManagers;	// a list of not ready to serve managers
	bool						m_isInited;
	SsapiGateway				*m_pSsapiGateway;


protected:

	// This callback is called once per recovered alarm. 
	virtual void 	cbRecoverAlarm(void *pAlarmContext_, STATUS status);

	// This callback is called when all alarms have been recovered and the AlarmMaster 
	// has sent the last reply to a MsgRecoverAlarms message.
	virtual void 	cbRecoveryComplete(STATUS status_){}

};


// The cell used in the manager launch table
struct SSAPI_MANAGER_LAUNCH_CELL{

	DdmSSAPI::SSAPI_MANAGER_CTOR	pCtor;
	U32								classType;
	BOOL							isFlexible;
};

#endif