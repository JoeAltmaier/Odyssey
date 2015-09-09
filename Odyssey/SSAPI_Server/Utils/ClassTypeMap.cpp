//******************************************************************************
// FILE:		ClassTypeMap.cpp
//
// PURPOSE:		Implements the class that wraps up the knowledge of our class
//				diagram in terms of who is derived from whom by using 
//				class type SSAPI codes.
//
// NOTE:		Every class that is derived from will only have ids of its 
//				immediate sub-classes. Recursion is used to reach indirectly
//				dervied subclasses.
//******************************************************************************

#include "ClassTypeMap.h"

#ifdef _DEBUG
	#define CLASS_TYPE_NAME(a) a
#else
	#define CLASS_TYPE_NAME(a) ""
#endif // _DEBUG


//******************************************************************************
// CLASS_TYPE_MAP:
//
// PURPOSE:		Static intializer. Add new classes here. 
//
// NOTES:		1. Make sure the maximum number of derived classes is
//					less than or equal to SSAPI_MAX_DIRECTLY_DERIVED_OBJECTS. If
//					it's not, bump up the define!!!!
//******************************************************************************

static ClassTypeMapCell	CLASS_TYPE_MAP[] = {
	{
		CLASS_TYPE_NAME("ManagedObject"),
		SSAPI_OBJECT_CLASS_TYPE_MANAGED_OBJECT,
		{
			SSAPI_OBJECT_CLASS_TYPE_USER,			SSAPI_OBJECT_CLASS_TYPE_STORAGE_ELEMENT_BASE,
			SSAPI_OBJECT_CLASS_TYPE_HOST,			SSAPI_OBJECT_CLASS_TYPE_CAPABILITY,
			SSAPI_OBJECT_CLASS_TYPE_PHS_DATA,		SSAPI_OBJECT_CLASS_TYPE_DATA_PATH,
			SSAPI_OBJECT_CLASS_TYPE_LOG_MESSAGE,	SSAPI_OBJECT_CLASS_TYPE_LUN_MAP_ENTRY,
			SSAPI_OBJECT_CLASS_TYPE_ALARM,			SSAPI_OBJECT_CLASS_TYPE_SOFTWARE_IMAGE,
			SSAPI_OBJECT_CLASS_TYPE_DEVICE,			SSAPI_OBJECT_CLASS_TYPE_PROCESS,
			SSAPI_OBJECT_CLASS_LOG_META_DATA,		SSAPI_OBJECT_CLASS_TYPE_CONNECTION,
			SSAPI_OBJECT_CLASS_TYPE_CONFIG_ID,		SSAPI_OBJECT_CLASS_TYPE_SW_IMAGE_DESCRIPTOR,
			SSAPI_OBJECT_CLASS_TYPE_PTS_TABLE,		SSAPI_OBJECT_CLASS_TYPE_TABLE_META_DATA,
			SSAPI_OBJECT_CLASS_TYPE_PTS_TABLE_ROW
		},
	},
	{	
		CLASS_TYPE_NAME("SoftwareImageDescriptor"),
		SSAPI_OBJECT_CLASS_TYPE_SW_IMAGE_DESCRIPTOR,
		{
			SSAPI_OBJECT_CLASS_TYPE_IOP_SW_IMAGE_DESCRIPTOR,			
		},
	},
	{	
		CLASS_TYPE_NAME("SoftwareIOPImageDescriptor"),
		SSAPI_OBJECT_CLASS_TYPE_IOP_SW_IMAGE_DESCRIPTOR,
		{
			SSAPI_OBJECT_CLASS_TYPE_NAC_SW_IMAGE_DESCRIPTOR,
			SSAPI_OBJECT_CLASS_TYPE_HBC_SW_IMAGE_DESCRIPTOR,
			SSAPI_OBJECT_CLASS_TYPE_SSD_SW_IMAGE_DESCRIPTOR
		},
	},
	{	
		CLASS_TYPE_NAME("StorageElementBase"),
		SSAPI_OBJECT_CLASS_TYPE_STORAGE_ELEMENT_BASE,
		{
			SSAPI_OBJECT_CLASS_TYPE_STORAGE_COLLECTION,
			SSAPI_OBJECT_CLASS_TYPE_STORAGE_ELEMENT,
			SSAPI_OBJECT_CLASS_TYPE_STORAGE_ELEMENT_PASS_THRU,
		},
	},
	{	
		CLASS_TYPE_NAME("StorageElementPassThru"),
		SSAPI_OBJECT_CLASS_TYPE_STORAGE_ELEMENT_PASS_THRU,
		{
			SSAPI_OBJECT_CLASS_TYPE_TAPE,			SSAPI_OBJECT_CLASS_TYPE_SES,
		},
	},
	{	
		CLASS_TYPE_NAME("StorageElement"),
		SSAPI_OBJECT_CLASS_TYPE_STORAGE_ELEMENT,
		{
			SSAPI_OBJECT_CLASS_TYPE_DISK_STORAGE_ELEMENT,	SSAPI_OBJECT_CLASS_TYPE_PARTITION_STORAGE_ELEMENT,	
			SSAPI_OBJECT_CLASS_TYPE_SSD_STORAGE_ELEMENT,	SSAPI_OBJECT_CLASS_TYPE_ARRAY_STORAGE_ELEMENT,
		},
	},
	{
		CLASS_TYPE_NAME("Storage Element Disk"),
		SSAPI_OBJECT_CLASS_TYPE_DISK_STORAGE_ELEMENT,
		{
			SSAPI_OBJECT_CLASS_TYPE_DISK_INTERNAL,	SSAPI_OBJECT_CLASS_TYPE_DISK_EXTERNAL,
		},
	},
	{
		CLASS_TYPE_NAME("Device"),
		SSAPI_OBJECT_CLASS_TYPE_DEVICE,
		{
			SSAPI_OBJECT_CLASS_TYPE_HDD_DEVICE,		SSAPI_OBJECT_CLASS_TYPE_DDH_DEVICE,
			SSAPI_OBJECT_CLASS_TYPE_PORT,			SSAPI_OBJECT_CLASS_TYPE_BOARD,
			SSAPI_OBJECT_CLASS_TYPE_CHASSIS,		SSAPI_OBJECT_CLASS_TYPE_BUS_SEGMENT,
			SSAPI_OBJECT_CLASS_TYPE_FAN,			SSAPI_OBJECT_CLASS_TYPE_BATTERY,
			SSAPI_OBJECT_CLASS_TYPE_POWER_SUPPLY,	SSAPI_OBJECT_CLASS_TYPE_DEVICE_COLLECTION,
			SSAPI_OBJECT_CLASS_TYPE_CORD_SET,
		},
	},
	{
		CLASS_TYPE_NAME("Process"),
		SSAPI_OBJECT_CLASS_TYPE_PROCESS,
		{
			SSAPI_OBJECT_CLASS_TYPE_RAID_UTILITY,
		},
	},
	{
		CLASS_TYPE_NAME("PHSData"),
		SSAPI_OBJECT_CLASS_TYPE_PHS_DATA,
		{
			SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_INT,	SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_FLOAT,
			SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_INT64,
		},
	},
	{
		CLASS_TYPE_NAME("PHSDataInt"),
		SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_INT,
		{
			SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_TEMPERATURE,	SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_VOLTAGE,
			SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_CURRENT,
		},
	},
	{
		CLASS_TYPE_NAME("PHSDataInt64"),
		SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_INT64,
		{
			SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_TIME,	SSAPI_OBJECT_CLASS_TYPE_PHS_DATA_THROUGHPUT,
		},
	},
	{
		CLASS_TYPE_NAME("Board"),
		SSAPI_OBJECT_CLASS_TYPE_BOARD,
		{
			SSAPI_OBJECT_CLASS_TYPE_FILLER_BOARD,		SSAPI_OBJECT_CLASS_TYPE_IOP,
		},
	},
	{
		CLASS_TYPE_NAME("IOP"),
		SSAPI_OBJECT_CLASS_TYPE_IOP,
		{
			SSAPI_OBJECT_CLASS_TYPE_HBC_BOARD,			SSAPI_OBJECT_CLASS_TYPE_SSD_BOARD,
			SSAPI_OBJECT_CLASS_TYPE_NAC_BOARD,
		},
	},
	{
		CLASS_TYPE_NAME("NAC"),
		SSAPI_OBJECT_CLASS_TYPE_NAC_BOARD,
		{
			SSAPI_OBJECT_CLASS_TYPE_SNAC_BOARD,
		},
	},
	{
		CLASS_TYPE_NAME("Power Supply"),
		SSAPI_OBJECT_CLASS_TYPE_POWER_SUPPLY,
		{
			SSAPI_OBJECT_CLASS_TYPE_CHASSIS_POWER_SUPPLY,	SSAPI_OBJECT_CLASS_TYPE_DISK_POWER_SUPPLY,
		},
	},
	{
		CLASS_TYPE_NAME("Powerable Interface"),
		SSAPI_OBJECT_CLASS_TYPE_POWERABLE_INTERFACE,
		{
			SSAPI_OBJECT_CLASS_TYPE_IOP,					SSAPI_OBJECT_CLASS_TYPE_CHASSIS,
		},
	},
	{
		CLASS_TYPE_NAME("Serviceable Interface"),
		SSAPI_OBJECT_CLASS_TYPE_SERVICEABLE_INTERFACE,
		{
			SSAPI_OBJECT_CLASS_TYPE_IOP,					SSAPI_OBJECT_CLASS_TYPE_FC_PORT,
		},
	},
	{
		CLASS_TYPE_NAME("Lockable Interface"),
		SSAPI_OBJECT_CLASS_TYPE_LOCKABLE_INTERFACE,
		{
			SSAPI_OBJECT_CLASS_TYPE_BOARD,					SSAPI_OBJECT_CLASS_TYPE_HDD_DEVICE,
		},
	},
	{
		CLASS_TYPE_NAME("Device Collection"),
		SSAPI_OBJECT_CLASS_TYPE_DEVICE_COLLECTION,
		{
			SSAPI_OBJECT_CLASS_TYPE_PCI_COLLECTION,			SSAPI_OBJECT_CLASS_TYPE_FC_COLLECTION,
			SSAPI_OBJECT_CLASS_TYPE_EVC_COLLECTION,
		},
	},
	{
		CLASS_TYPE_NAME("Data Path"),
		SSAPI_OBJECT_CLASS_TYPE_DATA_PATH,
		{
			SSAPI_OBJECT_CLASS_TYPE_REDUNDANT_DATA_PATH,	SSAPI_OBJECT_CLASS_TYPE_CLUSTERED_DATA_PATH,
		},
	},
	{
		CLASS_TYPE_NAME("Port Device"),
		SSAPI_OBJECT_CLASS_TYPE_PORT,
		{
			SSAPI_OBJECT_CLASS_TYPE_FC_PORT,	
		},
	},
	{
		CLASS_TYPE_NAME("FC Port Device"),
		SSAPI_OBJECT_CLASS_TYPE_FC_PORT,
		{
			SSAPI_OBJECT_CLASS_TYPE_INTERNAL_FC_PORT,	
		},
	},
	{
		CLASS_TYPE_NAME("Filler Panel"),
		SSAPI_OBJECT_CLASS_TYPE_FILLER_BOARD,
		{
			SSAPI_OBJECT_CLASS_TYPE_HBC_FILLER_BOARD,	
		},
	},
	{
		CLASS_TYPE_NAME("External Port"),
		SSAPI_OBJECT_CLASS_TYPE_CONNECTION,
		{
			SSAPI_OBJECT_CLASS_TYPE_UPSTREAM_CONNECTION,	SSAPI_OBJECT_CLASS_TYPE_DOWNSTREAM_CONNECTION,	
		},
	},
	{
		CLASS_TYPE_NAME("RAID Array"),
		SSAPI_OBJECT_CLASS_TYPE_ARRAY_STORAGE_ELEMENT,
		{
			SSAPI_OBJECT_CLASS_TYPE_ARRAY_0_STORAGE_ELEMENT,	SSAPI_OBJECT_CLASS_TYPE_ARRAY_1_STORAGE_ELEMENT,	
		},
	},
	{
		CLASS_TYPE_NAME("RAID1 Array"),
		SSAPI_OBJECT_CLASS_TYPE_ARRAY_1_STORAGE_ELEMENT,
		{
			SSAPI_OBJECT_CLASS_TYPE_ARRAY_HOT_COPY,	
		},
	},
	{
		CLASS_TYPE_NAME("Hot Copy Array"),
		SSAPI_OBJECT_CLASS_TYPE_ARRAY_HOT_COPY,
		{
			SSAPI_OBJECT_CLASS_TYPE_ARRAY_HOT_COPY_AUTO,	SSAPI_OBJECT_CLASS_TYPE_ARRAY_HOT_COPY_MANUAL, 	
		},
	},
	{
		CLASS_TYPE_NAME("Storage Collection"),
		SSAPI_OBJECT_CLASS_TYPE_STORAGE_COLLECTION,
		{
			SSAPI_OBJECT_CLASS_TYPE_STORAGE_COLL_SPARE_POOL,		
		},
	},
	{
		CLASS_TYPE_NAME("Raid Utility"),
		SSAPI_OBJECT_CLASS_TYPE_RAID_UTILITY,
		{
			SSAPI_OBJECT_CLASS_TYPE_RAID_INITIALIZE,		SSAPI_OBJECT_CLASS_TYPE_RAID_VERIFY,
			SSAPI_OBJECT_CLASS_TYPE_RAID_REGENERATE,		SSAPI_OBJECT_CLASS_TYPE_RAID_SMART_COPY,
			SSAPI_OBJECT_CLASS_TYPE_RAID_HOT_COPY
		},
	},
};



//******************************************************************************
// ClassTypeMap:
//
// PURPOSE:		The default constructor
//******************************************************************************

ClassTypeMap::ClassTypeMap(){
}


//******************************************************************************
// ~ClassTypeMap:
//
// PURPOSE:		The destructor
//******************************************************************************

ClassTypeMap::~ClassTypeMap(){
}


//******************************************************************************
// IsADerivedClass:
//
// PURPOSE:		Determines if a given object is derived (maybe indirectly)
//				from another given object
//
// FUNCTIONALITY:	The method uses recursive decent
//******************************************************************************

bool 
ClassTypeMap::IsADerivedClass( U32 baseClassId, U32 classIdInQuestion, bool includeDerivations ){

	U32					numberOfCells	= sizeof(CLASS_TYPE_MAP) / sizeof(CLASS_TYPE_MAP[0]),
						cellNumber,
						idNumber;

	for( cellNumber = 0; cellNumber < numberOfCells; cellNumber++){

		if( CLASS_TYPE_MAP[ cellNumber ].classTypeId == baseClassId ){

			for( idNumber = 0; idNumber < SSAPI_MAX_DIRECTLY_DERIVED_OBJECTS; idNumber++ ){

				if( CLASS_TYPE_MAP[ cellNumber ].derivedClassIds[ idNumber ] == classIdInQuestion )
					return true;
				else{	
					if( includeDerivations && IsADerivedClass( CLASS_TYPE_MAP[ cellNumber ].derivedClassIds[ idNumber ], classIdInQuestion, includeDerivations ) )
						return true;
				}
			}
		}
	}
	return false;
}

