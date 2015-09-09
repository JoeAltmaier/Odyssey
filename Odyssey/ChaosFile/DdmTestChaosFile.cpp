#include "ddm.h"
#include "chaosfile.h"
#include "chaosfilemsgs.h"
#include "ddmtimer.h"
#include "assert.h"
#include "buildsys.h"

#include <stdlib.h>
extern "C" void bzero(void *, size_t);

class DdmTestChaosFile : public Ddm
{
public:
	static Ddm *Ctor (DID did_) ;
	DdmTestChaosFile (DID did_) : Ddm(did_), testpass(0)  {}

	virtual STATUS Enable (Message *pMsg_);
private:
	int testpass;
	STATUS StartTest(void *);

	void TestCreation();
	void TestClose();
	void TestOpen();
	void TestReadWrite();

	ChaosFile *cf[30];

};

// Class Link Name used by Buildsys.cpp.  Must match CLASSENTRY in buildsys.cpp
CLASSNAME (DdmTestChaosFile, SINGLE);  

// test various creation conditions.  There are enough files here to cause the partition table page to grow. If
// file 30 doesn't fail (out of space), something went wrong.

char *tcf_names[30] = { "name 1", "name 2", "name 3", "name 3", "name 5", "name 6 is way too long", "name 7", "name 8", "name 9", "",
					"11","12","13","14","15","16","17","18","19","20","21","22","23","24","25","26","27","28",
					"name 29","name 30" };
int tcf_sizes[30] = {50, 28, 1, 0, 121, 0, 159, 39, 113,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,112,1}; 

char *contents[30];

STATUS CreateResults[30] = {cfSuccess, cfSuccess, cfSuccess, cfDuplicateName, cfSuccess, cfInvalidName, cfSuccess, cfSuccess, cfOutOfSpace, 
			cfInvalidName, cfSuccess, cfSuccess , cfSuccess, cfSuccess , cfSuccess , cfSuccess , cfSuccess , cfSuccess , cfSuccess , cfSuccess , cfSuccess , cfSuccess , cfSuccess , 
			cfSuccess , cfSuccess , cfSuccess , cfSuccess , cfSuccess ,
			cfSuccess , cfOutOfSpace  };

STATUS CloseResults[30] = {cfSuccess, cfSuccess, cfSuccess, cfInvalidHandle, cfSuccess, cfInvalidHandle, cfSuccess, cfSuccess, cfInvalidHandle, 
			cfInvalidHandle, cfSuccess, cfSuccess , cfSuccess, cfSuccess , cfSuccess , cfSuccess , cfSuccess , cfSuccess , cfSuccess , cfSuccess , cfSuccess , cfSuccess , cfSuccess ,
			cfSuccess , cfSuccess , cfSuccess , cfSuccess , cfSuccess ,
			cfSuccess , cfInvalidHandle  };

STATUS OpenResults[30] = {cfSuccess, cfSuccess, cfSuccess, cfFileNotFound, cfSuccess, cfFileNotFound, cfSuccess, cfSuccess, cfFileNotFound, 
			cfFileNotFound, cfSuccess, cfSuccess , cfSuccess, cfSuccess , cfSuccess , cfSuccess , cfSuccess , cfSuccess , cfSuccess , cfSuccess , cfSuccess , cfSuccess , cfSuccess ,
			cfSuccess , cfSuccess , cfSuccess , cfSuccess , cfSuccess ,
			cfSuccess , cfFileNotFound  };

U32 handles[30];

STATUS DdmTestChaosFile::Enable (Message *pMsg_)
{
	
	Reply(pMsg_, OK);
	STATUS status = Action(ACTIONCALLBACK(DdmTestChaosFile, StartTest), NULL);
	return OK;
}


Ddm *DdmTestChaosFile::Ctor(DID did)
{
	return new DdmTestChaosFile(did);
}


STATUS DdmTestChaosFile::StartTest(void *context) 
{
	ChaosFile cf0(tcf_names[0]);

	if (cf0.IsValid())
	{
		printf("Persistence check OK!\n");
		return OK;
	}	
	TestCreation();
	TestClose();
	TestOpen();
		
	for (int i=0; i < 30; ++i)
	{
		if (i != 4)
		{
			printf("Closing #%d...", i);
			if (OK == cf[i]->Close())
				printf("Sucess!\n");
			else
				printf("Failed!\n");
		}
	}

	TestReadWrite();
		
	return OK;
}

void DdmTestChaosFile::TestCreation()
{
	for (int i=0; i < 30; ++i)
	{
		cf[i] = new ChaosFile(tcf_names[i], tcf_sizes[i]*2048);
		assert (cf[i]->GetErrorCode() == CreateResults[i]);
		if (cf[i]->GetErrorCode() == cfSuccess)
		{
			assert (cf[i]->GetSize() == 0);
			assert (cf[i]->GetMaxSize() == tcf_sizes[i]*2048);
		}
	}
}

void DdmTestChaosFile::TestClose()
{

	for (int i=0; i < 30; i+=2) // close all the even files
		assert(cf[i]->Close() == CloseResults[i]);

	assert(!cf[0]->IsValid());
	assert(cf[0]->Close() == cfInvalidHandle);
	
}

void DdmTestChaosFile::TestOpen()
{
	for (int i=0; i < 30; i+=2) // re-open all the even files
	{
		assert(!cf[i]->IsValid());
		delete cf[i];
		cf[i] = new ChaosFile(tcf_names[i]);
		assert(cf[i]->GetErrorCode() == OpenResults[i]);
	}

	// try opening a file that is already open

	ChaosFile *cf = new ChaosFile(tcf_names[1]);
	assert (cf->GetErrorCode() == cfFileAlreadyOpen);
	delete cf;

}

void DdmTestChaosFile::TestReadWrite()
{
	srand( 2 ); 

	// test writing and reading file 4

	U32 maxSize = 10000; // cf[4]->GetMaxSize();
	char *bufferW = new(tBIG|tZERO) char[maxSize];
	char *bufferR = new(tBIG|tZERO) char[maxSize];

	for (int i = 0; i < maxSize; i =  ((i+1000 < maxSize) ? i + 1000 : maxSize))
	{
		for (int k=i; k < i; ++k)
			*(bufferW+k) = (rand() % 26) + 'A';
		assert(cf[4]->Write(bufferW, 0, i) == cfSuccess);
		assert(cf[4]->Read(bufferR, 0, cf[4]->GetSize()) == cfSuccess);
		assert(strncmp(bufferW, bufferR, cf[4]->GetSize()) == 0);
		printf("wrote %d bytes\n", i);
	}
	delete cf[4]; // close, and flush the fss
	cf[4] = new ChaosFile(tcf_names[4]);  // reopen
	
	
	// verify persistence
	bzero(bufferR, maxSize);
//	assert(cf[4]->GetSize() == maxSize);
	assert(cf[4]->Read(bufferR, 0, cf[4]->GetSize()) == cfSuccess);
	assert(strncmp(bufferW, bufferR, cf[4]->GetSize()) == 0);
	delete bufferR;
	delete bufferW;
}

