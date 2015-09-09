//DdmSnmp.cpp

#define _TRACEF
#include "Trace_Index.h"
#include "Odyssey_Trace.h"

#include <stdio.h>
#include <String.h>
#include "OsTypes.h"
#include "DdmSnmp.h"
#include "BuildSys.h"
#include "UnicodeString.h"
#include "..\msl\osheap.h"
#include "CtMessages.h"
#include "Event.h"
#include "SSAPI_Codes.h"
#include "SSAPIObjectStates.h"
#include "time.h"

CLASSNAME(DdmSnmp,SINGLE);	// Class Link Name used by Buildsys.cpp

DdmSnmp::DdmSnmp(DID did): Ddm(did) {
}

Ddm *DdmSnmp::Ctor(DID did) {
	return new DdmSnmp(did);
}

STATUS DdmSnmp::Initialize(Message *pMsg) {// virutal
	Reply(pMsg,OK);
	return OK;
}

//Enable -- Start-it-up
STATUS DdmSnmp::Enable(Message *pMsgReq) {

	m_iIndexMapEntries = 0;

	//subscribe to SSAPI data
	SendDeviceListen();
	SendAlarmListen();
	SendDeviceList();

	Reply(pMsgReq,OK);
	return OK;
}

UnicodeString* DdmSnmp::MakeUnicodeString(char* sTemp) {
	StringClass stTemp(sTemp);
	return new UnicodeString(stTemp);
}

void DdmSnmp::SendDeviceList() {
	SsapiRequestMessage* pMsg = new SsapiRequestMessage();
	pMsg->m_iObjectCode = SSAPI_MANAGER_CLASS_TYPE_DEVICE_MANAGER;
	pMsg->m_iRequestCode = SSAPI_OBJECT_MANAGER_LIST;

	ValueSet* pvsReq = new ValueSet();
	//takes a filterset
	pvsReq->AddInt(SSAPI_FILTER_CLASS_TYPE_OBJECT_CLASS_TYPE, SSAPI_FILTER_FID_FILTER_CLASS_TYPE);
	pvsReq->AddInt(1, SSAPI_OBJECT_CLASS_TYPE_FILTER_INCLUDE_DERIVATIONS);
	pvsReq->AddInt(SSAPI_OBJECT_CLASS_TYPE_DEVICE, SSAPI_OBJECT_CLASS_TYPE_FILTER_OBJECT_CLASS_TYPE);

	ValueSet* pvsFiltSet = new ValueSet();
	pvsFiltSet->AddValue(pvsReq, 0);

	pMsg->m_pValueSet->AddValue(pvsFiltSet, SSAPI_OBJECT_MANAGER_LIST_FILTER_SET);

	delete pvsReq;
	delete pvsFiltSet;

	Send(pMsg, NULL, REPLYCALLBACK(DdmSnmp, DeviceListReplyHandler));
}

int DdmSnmp::FindIndex(int iLocation) {
	int i=0;
	for(i=0;i<m_iIndexMapEntries;i++) {
		if(m_aIndexMap[i] == iLocation)
			return i;
	}
	m_aIndexMap[i] = iLocation;
	m_iIndexMapEntries++;
	return i;
}

void DdmSnmp::UpdateDevice(ValueSet* pvsDevice) {

	char sPrefix[100];

	//find out the device type
	int iDeviceType = 0;
	sPrefix[0] = 0;

	int iSsapiLocationCode = SSAPI_BOARD_FID_SLOT;
	int iUniqueLocation = 0;

	if(pvsDevice->GetInt(SSAPI_OBJECT_FID_OBJECT_CLASS_TYPE, &iDeviceType)) {

		switch(iDeviceType) {
		case SSAPI_OBJECT_CLASS_TYPE_CHASSIS:
			UpdateChassisData(pvsDevice);
			break;
		case SSAPI_OBJECT_CLASS_TYPE_IOP:
			sprintf(sPrefix, "IOP_in_slot_");
			iUniqueLocation = 100;
			break;
		case SSAPI_OBJECT_CLASS_TYPE_HBC_BOARD:
			sprintf(sPrefix, "HBC_in_slot_");
			iUniqueLocation = 500;
			break;
		case SSAPI_OBJECT_CLASS_TYPE_NAC_BOARD:
		case SSAPI_OBJECT_CLASS_TYPE_SNAC_BOARD:
			sprintf(sPrefix, "NAC_in_slot_");
			iUniqueLocation = 600;
			break;
		case SSAPI_OBJECT_CLASS_TYPE_SSD_BOARD:
			sprintf(sPrefix, "SSD_in_slot_");
			iUniqueLocation = 700;
			break;
		case SSAPI_OBJECT_CLASS_TYPE_FILLER_BOARD:
			sprintf(sPrefix, "FILLER_in_slot_");
			iUniqueLocation = 800;
			break;
		case SSAPI_OBJECT_CLASS_TYPE_HBC_FILLER_BOARD:
			sprintf(sPrefix, "HBC_FILLER_in_slot_");
			iUniqueLocation = 900;
			break;
		case SSAPI_OBJECT_CLASS_TYPE_HDD_DEVICE:
			sprintf(sPrefix, "HDD_in_bay_");
			iUniqueLocation = 1000;
			iSsapiLocationCode = SSAPI_HDD_FID_BAY_NUMBER;
			break;
		default:
            //printf("ddmSnmp::DeviceReplyHandler() device type %d\n",iDeviceType);
			break;
		};

		if(sPrefix[0] != 0) {

			int iLocation;
			pvsDevice->GetInt(iSsapiLocationCode, &iLocation);

			char sTemp[256];
			sprintf(sTemp, "%s%d", sPrefix, iLocation);

			UnicodeString* pusDescription = MakeUnicodeString(sTemp);

			//determine the opperational status
			int iOpperStatus = SNMP_OPER_STATUS_UP;
			U32 uiStatus = 0;
			pvsDevice->GetU32(SSAPI_OBJECT_FID_STATE, &uiStatus);
			switch(uiStatus) {
			case SSAPI_OBJECT_STATE_DEAD:
			case SSAPI_OBJECT_STATE_DIAG:
				iOpperStatus = SNMP_OPER_STATUS_DOWN;
				break;
			};


			//location is only half right... it may be the same for a disk drive and a board...
			iUniqueLocation += iLocation;
			int iIndex = FindIndex(iUniqueLocation);

			m_SnmpInterface.SetInterface(iIndex, *pusDescription, iOpperStatus);

			delete pusDescription;

			iIndex++;
		}

	}
	else {
		printf("ddmSnmp::DeviceReplyHandler() no device type\n");
	}

}

STATUS DdmSnmp::DeviceListReplyHandler(Message* pMsgReq) {

	SsapiRequestMessage* pMsg = (SsapiRequestMessage*)pMsgReq;

	//parse response
	ValueSet* pVS = (ValueSet*)pMsg->m_pResponseValueSet->GetValue(SSAPI_OBJECT_MANAGER_LIST_OBJECT_VECTOR);

	if(!pVS) {
		delete pMsg;
		return OK;
	}

	for(int i=0;i<pVS->GetCount();i++) {

		ValueSet* pvsDevice = (ValueSet*)pVS->GetValue(i);
		UpdateDevice(pvsDevice);

	}

	delete pMsg;

	m_SnmpInterface.SNMPInitComplete();

	printf("\n_______SNMP Initialized__________\n");

	return OK;
}

void DdmSnmp::UpdateChassisData(ValueSet* pvsChassis) {

	printf("\nSNMP______UpdateChassisData()\n");

	bool bDescSet = false;

	//description
	ValueSet* pvsAssetTag = (ValueSet*)pvsChassis->GetValue(SSAPI_OBJECT_FID_ASSET_TAG_OBJECT);
	if(pvsAssetTag) {
		UnicodeString usVersion;
		if(pvsAssetTag->GetString(SSAPI_ASSET_FID_VERSION_NUMBER, &usVersion)) {
			U32 iDate = 0;
			if(pvsAssetTag->GetU32(SSAPI_ASSET_FID_PRODUCTION_DATE, &iDate)) {
				time_t tDate = iDate;
				StringClass stTemp(asctime(gmtime(&tDate)));
				UnicodeString usDate(stTemp);
				m_SnmpInterface.SetDescription(usVersion, usDate);
				bDescSet = true;
			}
		}
	}

	if(! bDescSet) {
		StringClass stVersion("Version Unknown");
		StringClass stDate("02/27/1975");
		UnicodeString usVersion(stVersion);
		UnicodeString usDate(stDate);
		m_SnmpInterface.SetDescription(usVersion, usDate);
	}

	UnicodeString usTemp;
	//location
	if(pvsChassis->GetString(SSAPI_CHASSIS_FID_LOCATION, &usTemp))
		m_SnmpInterface.SetLocation(usTemp);

	//name, a localized string
	CCtMsgView msgRender;
	U32 uiCode;
	if(pvsChassis->GetU32(SSAPI_DEVICE_FID_NAME, &uiCode)) {

		Event evtParams(uiCode);

		StringClass stTemp;
		if(msgRender.GetMessageText(stTemp, evtParams) == OK) {
			UnicodeString usName(stTemp);
			m_SnmpInterface.SetName(usName);
		}
	}

	//contact
	StringClass stContact("Not Supported");
	UnicodeString usContact(stContact);
	m_SnmpInterface.SetContact(usContact);

	m_SnmpInterface.ClearTrapAddresses();

	//add trap addresses
	ValueSet* vsTraps = (ValueSet*)pvsChassis->GetValue(SSAPI_CHASSIS_FID_TRAP_IP_ADDRESS_VECTOR);
	for(int i=0;i<vsTraps->GetCount();i++) {
		int iAddr = 0;
		if(vsTraps->GetInt(i, &iAddr)) {
			char sIPRaw[4];
			memcpy(sIPRaw, &iAddr, 4);
            printf("AddTrapAddress: %d.%d.%d.%d\n", (unsigned char)sIPRaw[0], (unsigned char)sIPRaw[1], (unsigned char)sIPRaw[2], (unsigned char)sIPRaw[3]);

			m_SnmpInterface.AddTrapAddress(iAddr);
		}
	}
}

void DdmSnmp::SendDeviceListen() {
	SsapiRequestMessage* pMsg = new SsapiRequestMessage();
	pMsg->m_iObjectCode = SSAPI_MANAGER_CLASS_TYPE_DEVICE_MANAGER;
	pMsg->m_iRequestCode = SSAPI_OBJECT_MANAGER_ADD_LISTENER;

	//we only deal with modified... given the fixed set of interfaces... we don't support added!
	pMsg->m_pValueSet->AddInt(SSAPI_LISTENER_TYPE_OBJECT_MODIFIED, SSAPI_OBJECT_MANAGER_ADD_LISTENER_LISTENER_TYPE);

	Send(pMsg, NULL, REPLYCALLBACK(DdmSnmp, ListenReplyHandler));
}

void DdmSnmp::SendAlarmListen() {
	SsapiRequestMessage* pMsg = new SsapiRequestMessage();
	pMsg->m_iObjectCode = SSAPI_MANAGER_CLASS_TYPE_ALARM_MANAGER;
	pMsg->m_iRequestCode = SSAPI_OBJECT_MANAGER_ADD_LISTENER;

	pMsg->m_pValueSet->AddInt(SSAPI_LISTENER_TYPE_OBJECT_ADDED, SSAPI_OBJECT_MANAGER_ADD_LISTENER_LISTENER_TYPE);

	Send(pMsg, NULL, REPLYCALLBACK(DdmSnmp, ListenReplyHandler));
}

STATUS DdmSnmp::ListenReplyHandler(Message* pMsgReq) {
	SsapiRequestMessage* pMsg = (SsapiRequestMessage*)pMsgReq;

	//response to the listen request... says that we are successfully listening
	ValueSet* pvsRetCode = (ValueSet*)pMsg->m_pResponseValueSet->GetValue(SSAPI_RETURN_STATUS_SET);
	if(pvsRetCode) {
		int iRetStatus;
		if(pvsRetCode->GetInt(SSAPI_RETURN_STATUS, &iRetStatus)) {
			if(iRetStatus != SSAPI_RC_SUCCESS) {
				printf("oh fudge, return status = %d\n", iRetStatus);
			}
		}
		else {
			printf("ddmSnmp:: missing return status\n");
		}
		delete pMsg;
		return OK;
	}

	ValueSet* pvsEvt = (ValueSet*)pMsg->m_pResponseValueSet->GetValue(SSAPI_OBJECT_MANAGER_ADD_LISTENER_EVENT_OBJECT);
	if(pvsEvt) {
		int iType;
		pvsEvt->GetInt(SSAPI_EVENT_FID_EVENT_TYPE, &iType);
		switch(iType) {
		case SSAPI_EVENT_OBJECT_MODIFIED:
			{
				ValueSet* pvsObject = (ValueSet*)pvsEvt->GetValue(SSAPI_EVENT_FID_MANAGED_OBJECT);

				if(pvsObject) {
					UpdateDevice(pvsObject);
				}
				else {
					printf("object valueset not found for object modified\n");
				}
			}
			break;
		case SSAPI_EVENT_OBJECT_ADDED:
			{
				ValueSet* pvsObject = (ValueSet*)pvsEvt->GetValue(SSAPI_EVENT_FID_MANAGED_OBJECT);
				if(pvsObject) {
					int iObjectType;
					pvsObject->GetInt(SSAPI_OBJECT_FID_OBJECT_CLASS_TYPE, &iObjectType);
					if(iObjectType == SSAPI_OBJECT_CLASS_TYPE_ALARM) {

						//determine severity
						int iSsapiSeverity;
						int iSnmpSeverity;
						if(!pvsObject->GetInt(SSAPI_ALARM_FID_SEVERITY, &iSsapiSeverity))
							printf("\nget alarm severity failed!!!\n");
						switch(iSsapiSeverity) {
						case SSAPI_ALARM_SEVERITY_CRITICAL:
							iSnmpSeverity = SNMP_TRAP_SPECIFIC_TYPE_CRITICAL;
							break;
						case SSAPI_ALARM_SEVERITY_MINOR:
							iSnmpSeverity = SNMP_TRAP_SPECIFIC_TYPE_MINOR;
							break;
						case SSAPI_ALARM_SEVERITY_WARNING:
							iSnmpSeverity = SNMP_TRAP_SPECIFIC_TYPE_WARNING;
							break;
						default:
							iSnmpSeverity = SNMP_TRAP_SPECIFIC_TYPE_CRITICAL;
							printf("\nssapi severity out of exptected range %d\n", iSsapiSeverity);
						};

						int iTrapKey = m_SnmpTrapHolder.AddTrapData();
						SNMPTrapData* pTrapData = m_SnmpTrapHolder.GetTrapData(iTrapKey);

						pTrapData->SetSNMPSeverity(iSnmpSeverity);

						//find the description
						CCtMsgView msgRender;
						U32 uiCode;
						if(pvsObject->GetU32(SSAPI_ALARM_FID_MESSAGE_CODE, &uiCode)) {

							Event evtParams(uiCode);

							ValueSet* pVS = (ValueSet*)pvsObject->GetValue(SSAPI_ALARM_FID_PARAMETER_VECTOR);
							if(pVS) {
								for(int i=0;i<pVS->GetCount();i++) {
									Value* pVal = pVS->GetValue(i);
									switch(pVal->m_iType) {
									case SSAPI_TYPE_INT:
									case SSAPI_TYPE_U32:
										{
											U32 uiData;
											pVS->GetU32(i, &uiData);
											evtParams.AddEventParameter(uiData);
										}
										break;
									case SSAPI_TYPE_STRING:
										{
											UnicodeString usTemp;
											pVS->GetString(i, &usTemp);
											StringClass asciiString;
											usTemp.GetAsciiString(asciiString);
											char* sData = asciiString.CString();
											evtParams.AddEventParameter(sData);
											delete[] sData;
										}
										break;
									default:
										Tracef("SNMP trap message description parameter type not known %d\n",pVal->m_iType);
										break;
									};
								}
							}
							else {
								Tracef("SNMP trap message description failed to find parameter vector\n");
							}

							StringClass stTemp;
							if(msgRender.GetMessageText(stTemp, evtParams) == OK) {
								UnicodeString usTemp(stTemp);
								pTrapData->SetDescription(usTemp);
							}
							else {
								Tracef("SNMP trap message description localization failure\n");
							}
						}
						else {
							Tracef("SNMP trap message description failed to find message code\n");
						}

						//now resolve the source
						int iObjectManager;

						Value* pValue = pvsObject->GetValue(SSAPI_ALARM_FID_SOURCE_OBJECT_ID);
						pvsObject->GetInt(SSAPI_ALARM_FID_SOURCE_OBJECT_MANAGER, &iObjectManager);

						SsapiRequestMessage* pSourceMessage = new SsapiRequestMessage();
						pSourceMessage->m_iObjectCode = iObjectManager;

						//we will use this to find the correct trap when the response is recieved
						memcpy(pSourceMessage->m_acSenderID, &iTrapKey, sizeof(int));

						pSourceMessage->m_iRequestCode = SSAPI_OBJECT_MANAGER_LIST;

						//make an ID filter to put in this request!!
						ValueSet vsIDs;
						vsIDs.AddValue(pValue, 0);
						ValueSet vsFilter;
						vsFilter.AddInt(SSAPI_FILTER_CLASS_TYPE_DESIGNATOR_ID, SSAPI_FILTER_FID_FILTER_CLASS_TYPE);

						vsFilter.AddValue(&vsIDs, SSAPI_INT_FLOAT_VECTOR_FILTER_FID_VALUE_VECTOR);
						vsFilter.AddInt(SSAPI_OBJECT_FID_ID, SSAPI_INT_FLOAT_VECTOR_FILTER_FID_FIELD_ID);
						vsFilter.AddInt(SSAPI_COMPARATOR_EQUAL, SSAPI_INT_FLOAT_VECTOR_FILTER_FID_COMPARATOR_TYPE);

						ValueSet vsFilterSet;
						vsFilterSet.AddValue(&vsFilter, 0);

						pSourceMessage->m_pValueSet->AddValue(&vsFilterSet, SSAPI_OBJECT_MANAGER_LIST_FILTER_SET);

						Send(pSourceMessage, NULL, REPLYCALLBACK(DdmSnmp, SourceResolutionReplyHandler));
						printf("\nSNMP______got and alarm... resolution started!!\n");
					}
				}
				else {
					printf("\nobject added didn't have an object inside\n");
				}
			}
			break;
		default:
            printf("unhandled event received\n");
			break;
		};
	}

	delete pMsg;
	return OK;
}

void DdmSnmp::AlarmResolutionComplete(int iTrapKey) {
	SNMPTrapData* pTrapData = m_SnmpTrapHolder.GetTrapData(iTrapKey);

	m_SnmpInterface.SendTrap(pTrapData->m_usSourceName, pTrapData->m_usDescription, pTrapData->m_iSNMPSeverity);

	m_SnmpTrapHolder.RemoveTrapData(iTrapKey);
}

void DdmSnmp::PrefixString(char* sPrefix, UnicodeString& usTarget, ValueSet* pVS, int iSsapiCode) {
	StringClass stPrefix(sPrefix);
	UnicodeString usPrefix(stPrefix);
	UnicodeString usTemp;
	pVS->GetString(iSsapiCode, &usTemp);
	usTarget = usPrefix + usTemp;
}

STATUS DdmSnmp::SourceResolutionReplyHandler(Message* pMsgReq) {
	SsapiRequestMessage* pMsg = (SsapiRequestMessage*)pMsgReq;

	int iTrapKey;
	memcpy(&iTrapKey, pMsg->m_acSenderID, sizeof(int));
	SNMPTrapData* pTrapData = m_SnmpTrapHolder.GetTrapData(iTrapKey);

	//parse response
	ValueSet* pVS = (ValueSet*)pMsg->m_pResponseValueSet->GetValue(SSAPI_OBJECT_MANAGER_LIST_OBJECT_VECTOR);

	BOOL bSuccessfull = TRUE;
	if(!pVS || pVS->GetCount() != 1) {
		bSuccessfull = FALSE;
	}
	else {

		//grab our source data
		ValueSet* pvsSource = (ValueSet*)pVS->GetValue(0);

		//find out the object type
		int iObjectType;
		if(pvsSource->GetInt(SSAPI_OBJECT_FID_OBJECT_CLASS_TYPE, &iObjectType)) {

			BOOL bComplete = TRUE;
			UnicodeString usName;
			switch(iObjectType) {
			case SSAPI_OBJECT_CLASS_TYPE_USER:
				pvsSource->GetString(SSAPI_USER_FID_USERNAME, &usName);
				pTrapData->SetSourceName(usName);
				break;
			case SSAPI_OBJECT_CLASS_TYPE_FILLER_BOARD:
				PrefixString("Filler Board ", usName, pvsSource, SSAPI_DEVICE_FID_NAME);
				pTrapData->SetSourceName(usName);
				break;
			case SSAPI_OBJECT_CLASS_TYPE_HBC_FILLER_BOARD:
				PrefixString("HBC Filler Board ", usName, pvsSource, SSAPI_DEVICE_FID_NAME);
				pTrapData->SetSourceName(usName);
				break;
			case SSAPI_OBJECT_CLASS_TYPE_IOP:
			case SSAPI_OBJECT_CLASS_TYPE_HBC_BOARD:
			case SSAPI_OBJECT_CLASS_TYPE_SSD_BOARD:
			case SSAPI_OBJECT_CLASS_TYPE_NAC_BOARD:
			case SSAPI_OBJECT_CLASS_TYPE_SNAC_BOARD:
			case SSAPI_OBJECT_CLASS_TYPE_FAN:
			case SSAPI_OBJECT_CLASS_TYPE_CHASSIS_POWER_SUPPLY:
			case SSAPI_OBJECT_CLASS_TYPE_BUS_SEGMENT:
			case SSAPI_OBJECT_CLASS_TYPE_BATTERY:
			case SSAPI_OBJECT_CLASS_TYPE_DEVICE_COLLECTION:
			case SSAPI_OBJECT_CLASS_TYPE_PCI_COLLECTION:
			case SSAPI_OBJECT_CLASS_TYPE_FC_COLLECTION:
			case SSAPI_OBJECT_CLASS_TYPE_EVC_COLLECTION:
			case SSAPI_OBJECT_CLASS_TYPE_PORT:
			case SSAPI_OBJECT_CLASS_TYPE_FC_PORT:
			case SSAPI_OBJECT_CLASS_TYPE_INTERNAL_FC_PORT:
			case SSAPI_OBJECT_CLASS_TYPE_HDD_DEVICE:
			case SSAPI_OBJECT_CLASS_TYPE_DDH_DEVICE:
			case SSAPI_OBJECT_CLASS_TYPE_DISK_POWER_SUPPLY:
			case SSAPI_OBJECT_CLASS_TYPE_CHASSIS:
			case SSAPI_OBJECT_CLASS_TYPE_BOARD:
			case SSAPI_OBJECT_CLASS_TYPE_POWER_SUPPLY:
				pvsSource->GetString(SSAPI_DEVICE_FID_NAME, &usName);
				pTrapData->SetSourceName(usName);
				break;
			case SSAPI_OBJECT_CLASS_TYPE_ALARM:
				pTrapData->SetSourceName("Alarm");
				break;
			case SSAPI_OBJECT_CLASS_TYPE_HOST:
				pvsSource->GetString(SSAPI_HOST_FID_NAME, &usName);
				pTrapData->SetSourceName(usName);
				break;
			case SSAPI_OBJECT_CLASS_TYPE_LOG_MESSAGE:
				pTrapData->SetSourceName("Log Message");
				break;
			case SSAPI_OBJECT_CLASS_LOG_META_DATA:
				pTrapData->SetSourceName("Log Meta-Data");
				break;
			case SSAPI_OBJECT_CLASS_TYPE_LUN_MAP_ENTRY:
				pvsSource->GetString(SSAPI_LUN_MAP_ENTRY_FID_NAME, &usName);
				pTrapData->SetSourceName(usName);
				break;
			case SSAPI_OBJECT_CLASS_TYPE_PROCESS:
			case SSAPI_OBJECT_CLASS_TYPE_RAID_UTILITY:
			case SSAPI_OBJECT_CLASS_TYPE_RAID_INITIALIZE:
			case SSAPI_OBJECT_CLASS_TYPE_RAID_VERIFY:
			case SSAPI_OBJECT_CLASS_TYPE_RAID_REGENERATE:
			case SSAPI_OBJECT_CLASS_TYPE_RAID_SMART_COPY:
			case SSAPI_OBJECT_CLASS_TYPE_RAID_HOT_COPY:
				pvsSource->GetString(SSAPI_PROCESS_FID_NAME, &usName);
				pTrapData->SetSourceName(usName);
				break;
			case SSAPI_OBJECT_CLASS_TYPE_ARRAY_STORAGE_ELEMENT:
			case SSAPI_OBJECT_CLASS_TYPE_STORAGE_COLLECTION:
			case SSAPI_OBJECT_CLASS_TYPE_STORAGE_ELEMENT:
			case SSAPI_OBJECT_CLASS_TYPE_PARTITION_STORAGE_ELEMENT:
			case SSAPI_OBJECT_CLASS_TYPE_DISK_STORAGE_ELEMENT:
			case SSAPI_OBJECT_CLASS_TYPE_ARRAY_0_STORAGE_ELEMENT:
			case SSAPI_OBJECT_CLASS_TYPE_ARRAY_1_STORAGE_ELEMENT:
			case SSAPI_OBJECT_CLASS_TYPE_ARRAY_HOT_COPY:
			case SSAPI_OBJECT_CLASS_TYPE_ARRAY_HOT_COPY_AUTO:
			case SSAPI_OBJECT_CLASS_TYPE_ARRAY_HOT_COPY_MANUAL:
			case SSAPI_OBJECT_CLASS_TYPE_STORAGE_ELEMENT_PASS_THRU:
				pvsSource->GetString(SSAPI_STORAGE_ELEMENT_B_FID_NAME, &usName);
				pTrapData->SetSourceName(usName);
				break;
			case SSAPI_OBJECT_CLASS_TYPE_SSD_STORAGE_ELEMENT:
				PrefixString("SSD: ", usName, pvsSource, SSAPI_STORAGE_ELEMENT_B_FID_NAME);
				pTrapData->SetSourceName(usName);
				break;
			case SSAPI_OBJECT_CLASS_TYPE_DISK_INTERNAL:
				PrefixString("Internal Disk: ", usName, pvsSource, SSAPI_STORAGE_ELEMENT_B_FID_NAME);
				pTrapData->SetSourceName(usName);
				break;
			case SSAPI_OBJECT_CLASS_TYPE_DISK_EXTERNAL:
				PrefixString("External Disk: ", usName, pvsSource, SSAPI_STORAGE_ELEMENT_B_FID_NAME);
				pTrapData->SetSourceName(usName);
				break;
			case SSAPI_OBJECT_CLASS_TYPE_STORAGE_COLL_SPARE_POOL:
				PrefixString("Spare Pool: ", usName, pvsSource, SSAPI_STORAGE_ELEMENT_B_FID_NAME);
				pTrapData->SetSourceName(usName);
				break;
			case SSAPI_OBJECT_CLASS_TYPE_TAPE:
				PrefixString("Tape: ", usName, pvsSource, SSAPI_STORAGE_ELEMENT_B_FID_NAME);
				pTrapData->SetSourceName(usName);
				break;
			case SSAPI_OBJECT_CLASS_TYPE_SES:
				PrefixString("SES: ", usName, pvsSource, SSAPI_STORAGE_ELEMENT_B_FID_NAME);
				pTrapData->SetSourceName(usName);
				break;
			case SSAPI_OBJECT_CLASS_TYPE_CAPABILITY:
				pTrapData->SetSourceName("Capability");
				break;
			case SSAPI_OBJECT_CLASS_TYPE_SOFTWARE_IMAGE:
				pTrapData->SetSourceName("Software Image");
				break;
			case SSAPI_OBJECT_CLASS_TYPE_SW_IMAGE_DESCRIPTOR:
				pTrapData->SetSourceName("Software Image Descriptor");
				break;
			case SSAPI_OBJECT_CLASS_TYPE_IOP_SW_IMAGE_DESCRIPTOR:
				pTrapData->SetSourceName("IOP Software Image Descriptor");
				break;
			case SSAPI_OBJECT_CLASS_TYPE_NAC_SW_IMAGE_DESCRIPTOR:
				pTrapData->SetSourceName("NAC Software Image Descriptor");
				break;
			case SSAPI_OBJECT_CLASS_TYPE_HBC_SW_IMAGE_DESCRIPTOR:
				pTrapData->SetSourceName("HBC Software Image Descriptor");
				break;
			case SSAPI_OBJECT_CLASS_TYPE_SSD_SW_IMAGE_DESCRIPTOR:
				pTrapData->SetSourceName("SSD Software Image Descriptor");
				break;
			case SSAPI_OBJECT_CLASS_TYPE_DATA_PATH:
			case SSAPI_OBJECT_CLASS_TYPE_REDUNDANT_DATA_PATH:
			case SSAPI_OBJECT_CLASS_TYPE_CLUSTERED_DATA_PATH:
				pvsSource->GetString(SSAPI_DATA_PATH_FID_NAME, &usName);
				pTrapData->SetSourceName(usName);
				break;
			case SSAPI_OBJECT_CLASS_TYPE_CONNECTION:
			case SSAPI_OBJECT_CLASS_TYPE_UPSTREAM_CONNECTION:
			case SSAPI_OBJECT_CLASS_TYPE_DOWNSTREAM_CONNECTION:
				if(! pvsSource->GetString(SSAPI_CONNECTION_FID_NAME, &usName))
					pvsSource->GetString(SSAPI_CONNECTION_FID_WWNAME, &usName);
				pTrapData->SetSourceName(usName);
				break;
			default:
				bSuccessfull = FALSE;
				break;
			};

			if(bComplete && bSuccessfull) {
				AlarmResolutionComplete(iTrapKey);
			}
		}
		else {
			bSuccessfull = FALSE;
		}
	}

	if(!bSuccessfull) {
		pTrapData->SetSourceName("Source Unknown");
		AlarmResolutionComplete(iTrapKey);
	}

	delete pMsg;
	return OK;
}
