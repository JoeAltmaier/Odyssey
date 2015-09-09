#include "ClassTable.h"
#include "DdmManager.h"
#include "PersistentData.h"
#include "DdmPartition.h"
#include "DdmNull.h"
#include "DdmTimer.h"
#include "TestDdm.h"

	ClassDef ctPts={"HDM_PTS", PersistentData::Ctor};
	ClassDef ctDmgr={"HDM_DMGR", DdmManager::Ctor};
	ClassDef ctTest={"HDM_TEST", TestDdm::Ctor};
	ClassDef ctPart={"HDM_PART", DdmPartition::Ctor};
	ClassDef ctNull={"HDM_NULL", DdmNull::Ctor};
	ClassDef ctTime={"HDM_TIMR", DdmTimer::Ctor};

	ClassDef *pPClass[]={
		&ctPts,
		&ctDmgr,
		&ctTest,
		&ctPart,
		&ctNull,
		&ctTime,
		NULL
		};

