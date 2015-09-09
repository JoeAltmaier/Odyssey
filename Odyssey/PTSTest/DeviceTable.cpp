#include "DeviceTable.h"
#include "Interrupt.h"
#include "Dma.h"
#include "DdmManager.h"
#include "Messenger.h"
#include "Transport.h"
#include "FailSafe.h"
#include "Configuration.h"

	DeviceTable::DeviceDef aDevice[8]={
		{"Configuration",Configuration::Initialize},
		{"Interrupt", 	Interrupt::Initialize},
		{"Dma", 		Dma::Initialize},
		{"DdmManager", 	DdmManager::DeviceInitialize},
		{"Transport", 	Transport::DeviceInitialize},
		{"Messenger", 	Messenger::DeviceInitialize},
		{"FailSafe", 	FailSafe::Initialize},
		{NULL, NULL}
		};

	DeviceTable::DeviceDef *DeviceTable::pDevice=aDevice;
