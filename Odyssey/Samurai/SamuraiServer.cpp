//
// SamuraiServer.cpp
//  Test CHAOS network routines
//

#ifndef _TRACEf
#define _TRACEf
#include "Trace_Index.h"
#include "Odyssey_trace.h"
#endif
#define PLUS
#include <stdio.h>
#include <stdlib.h>
#include <String.h>
#include <ctype.h>
#include "OsTypes.h"
#include "SamuraiServer.h"
#include "BuildSys.h"
#include "Critical.h"

#include "osheap.h"


CLASSNAME(SamuraiServer,SINGLE);	// Class Link Name used by Buildsys.cpp

SERVELOCAL(SamuraiServer, REQ_SSAPI_RESPONSE);

SamuraiServer::SamuraiServer(DID did): Ddm(did) {

}

Ddm *SamuraiServer::Ctor(DID did) {
	return new SamuraiServer(did);
}

STATUS SamuraiServer::Initialize(Message *pMsg) {  // virtual 


	Tracef("SamuraiServer::Initialize()\n");
		
	Reply(pMsg,OK);
	return OK;
	
}

//Enable -- Start-it-up
STATUS SamuraiServer::Enable(Message *pMsgReq) {
	
	Tracef("SamuraiServer::Enable()\n");
	
	echomode = 0;
	iSessionID = 0;
	counter = 0;
	memset(pMessageBuf, '\0', 132);

	
	Tracef("SamuraiServer: Start-it-up: got here!!!\n\n\n\n\n");
	NetMsgListen* pListenMsg = new NetMsgListen(10, 50, 1, 169, 3001);

	Send(pListenMsg, NULL, (ReplyCallback) & ListenReplyHandler);
	Reply(pMsgReq,OK);
	return OK;
}

//ReplyHandler for calls to network listen
STATUS SamuraiServer::ListenReplyHandler(Message *pMsgReq) {

	NetMsgListen* pListenMsg = (NetMsgListen*) pMsgReq;

	printf("\nNew Connection (%d)\n",pListenMsg->m_iConnectionID);
Tracef("SamuraiServer: ListenReplyHandler got called!!!\n\n\n\n\n");
	//we now call read for that connection
	NetMsgRead* pReadMsg = new NetMsgRead(pListenMsg->m_iConnectionID,pListenMsg->m_iConnectionID);
	Send(pReadMsg, NULL, (ReplyCallback) & ReadReplyHandler);
	
	delete pListenMsg;
	return OK;
}

STATUS SamuraiServer::ReadReplyHandler(Message * pMsgReq) 
{
	int     	val;
	
	Tracef("SamuraiServer::ReadReplyHandler()\n\n\n\n\n");

	NetMsgRead* pReadMsg = (NetMsgRead*) pMsgReq;
	
	// a happy read buffer was received
	if(pReadMsg->DetailedStatusCode == OK) 
	{
		char * pBuf;
		int iBufSize;
		pReadMsg->GetBuffer(pBuf,iBufSize);
		int i=0;

		//zero out pMessageBuf
		memset (pMessageBuf, '\0', 132);
		counter = 0;

		while ((i < iBufSize) && (pBuf[i] != EOF))
		{
			// the ';' char says that command is complete...
			if (pBuf[i] != ';')
				pMessageBuf[counter++] = pBuf[i++];  // continue to build a command from input buffers
			else
			{
				breakup (); 
				
				//setup a queue node
				node * Gorp = new node;
				Gorp->enqueue();				// add command to queue
								
				//handle command string
				val = lookup(messageHash[0]);

				//zero out pMessageBuf
				memset (pMessageBuf, '\0', 132);
				counter = 0;

				switch (val)
				{
					case 0:
						Tracef("echo opcode seen in incoming message\n\n\n");
						// stuff command and parameters from messageHash into a Msg object
						printf(" Broke in Echo\n");
						break;
					case 1:
						Tracef("repeat opcode seen in incoming message\n\n\n");
						printf(" Broke in repeat\n");
						break;
					case 2: 
						Tracef("createtbl opcode seen in incoming message\n\n\n");
						printf(" Broke in createtbl\n");

						// stuff command and parameters from messageHash into a Msg object
						harnessCreateTable();
						
						break;
					default:
						Tracef("the %s operation is not recognized at this time\n", messageHash[0]);
						Tracef("...please contact the developer for addition of this command\n");
				}
				i++;
			}
		}
	}		
	else 
	{
		Tracef("Connection %d has been terminated\n",pReadMsg->m_iConnectionID);
	}
	delete pReadMsg;
	
	return OK;
}

// handles replies to write requests
//  just deletes the message since nothing else is required in this case
//  note that the messages is cast before deleting to invoke the non-virtual
//  destructor, deleting the buffer
//  THIS IS NECESSARY TO PREVENT MEMORY LEAKS!
STATUS SamuraiServer::WriteReplyHandler(Message * pMsgReq) {

	NetMsgWrite* pMsg = (NetMsgWrite*)pMsgReq;

	delete pMsg;
	return OK;
	
}

void SamuraiServer::breakup ()
{
	char  *   ptr;
	char     argStruct [132];
	char   *  tmp;
	int      ctr = 0;
	int      i = 0;
	int 	 j = 0;

	ptr = pMessageBuf;
	tmp = (char *) std::malloc (132);
	memset (tmp, '\0',  132);
	
	j = sizeof(tmp);

	while (* ptr == ' ' || * ptr == '\t')
		* ptr ++;
		
	// grab opcode from command line
	while ((* ptr != ' ') && (* ptr != '\t') && (* ptr != '\0') && (* ptr != ';')&&(*ptr != '('))
		tmp [i ++] = * ptr ++;
	tmp [i] = '\0';  // null terminate the temporary buffer

	strcpy(messageHash[ctr++] ,tmp);	//Opcode in
	j = sizeof(tmp);

    if (* ptr == '\0') 					// I'm done, go south
	{
		strcpy(messageHash[ctr], "\0");
	 	return;
	}
		
	// skip any whitespace
	while ((*ptr == ' ') || (* ptr == '\t') || (* ptr == ',') || (*ptr == '('))
		*ptr ++;

	tmp = (char *) std::malloc (132);
	memset (tmp, '\0',  132);
 	// while you've got args (not a close parenthesis)
 	do
 	{
  		i = 0;
  		while (*ptr != ',')
  		{
   			if ((*ptr != ' ') && (*ptr != '\t') && (*ptr != ')'))
				tmp [i++] = *ptr;

   			if (*ptr == ')')
				break;
   			*ptr ++;
  		}

  		if (*ptr != ')')   // skip commas
   			* ptr ++;

  		tmp [i] = '\0';    // null terminate the temporary buffer
  		strcpy(messageHash[ctr ++],tmp);
  		tmp = (char *) std::malloc (132);
  		memset (tmp, '\0', 132);
 	}
 	while (*ptr != ')');
 	strcpy(messageHash [ctr], "\0");
 	
 	return ;
}


//////////////////////////////////////////////////////////////////////

void SamuraiServer::harnessCreateTable()
{

	STATUS status;
	int i = 2;
	int j = 0;
	int k = 0;
	div_t intval;


	Tracef("In Samurai::harnessCreateTable\n");
	
	// This is the code to create the DiskStatus Table.
	
//		fieldDef	Table_FieldDefs[] = {
//		// Field Definitions follow one per row.
		// FieldName							  Size   Type
//		"rowID",									8,	ROWID_FT, Persistant_PT,
//		"version",									4,	U32_FT, Persistant_PT,
//		"size",										4,	U32_FT, Persistant_PT,
//		"key",										4,	U32_FT, Persistant_PT,
//		"RefreshRate",								4,	U32_FT, Persistant_PT,
//		"ridSRCRecord",								8,	ROWID_FT, Persistant_PT,
//		"num_recoverable_media_errors_no_delay",	4,	U32_FT, Persistant_PT,
//		"num_recoverable_media_errors_delay",		4,	U32_FT, Persistant_PT,
//		"num_recoverable_media_errors_by_retry",	4,	U32_FT, Persistant_PT,
//		"num_recoverable_media_errors_by_ecc",		4,	U32_FT, Persistant_PT,
//		"num_recoverable_nonmedia_errors",			4,	U32_FT, Persistant_PT,
//		"num_bytesprocessedtotal",					4,	U32_FT, Persistant_PT,
//		"num_unrecoverable_media_errors",			4,	U32_FT, Persistant_PT,
//	};
	while(messageHash[i++][0] != '\0')
		k++;
	i = 2;
	intval = div(k,3);
		
//	fieldDef Table_FieldDefs[intval.quot];
	fieldDef* Table_FieldDefs = new(tPCI|tUNCACHED) fieldDef[intval.quot];
	
	while(strcmp(messageHash[i],"\0") != 0)
	{	
		strcpy(Table_FieldDefs[j].name, messageHash[i]);
		i++;
		if (strcmp(messageHash[i],"BINARY_FT") == 0)
		{
			Table_FieldDefs[j].cbField = 1;
			Table_FieldDefs[j].iFieldType = BINARY_FT;
		}
		else
		if (strcmp(messageHash[i],"S32_FT") == 0)
		{
			Table_FieldDefs[j].cbField = sizeof(S32);
			Table_FieldDefs[j].iFieldType = S32_FT;
		}
		else
		if (strcmp(messageHash[i],"U32_FT") == 0)
		{
			Table_FieldDefs[j].cbField = sizeof(U32);
			Table_FieldDefs[j].iFieldType = U32_FT;
		}
		else
		if (strcmp(messageHash[i],"S64_FT") == 0)
		{
			Table_FieldDefs[j].cbField = sizeof(S64);
			Table_FieldDefs[j].iFieldType = S64_FT;
		}
		else
		if (strcmp(messageHash[i],"U64_FT") == 0)
		{
			Table_FieldDefs[j].cbField = sizeof(U64);
			Table_FieldDefs[j].iFieldType = U64_FT;
		}
		else
		if (strcmp(messageHash[i],"STRING16_FT") == 0)
		{
			Table_FieldDefs[j].cbField = 16;
			Table_FieldDefs[j].iFieldType = STRING16_FT;
		}
		else
		if (strcmp(messageHash[i],"STRING32_FT") == 0)
		{
			Table_FieldDefs[j].cbField = 32;
			Table_FieldDefs[j].iFieldType = STRING32_FT;
		}
		else
		if (strcmp(messageHash[i],"STRING64_FT") == 0)
		{
			Table_FieldDefs[j].cbField = 64;
			Table_FieldDefs[j].iFieldType = STRING64_FT;
		}
		else
		if (strcmp(messageHash[i],"ROWID_FT") == 0)
		{
			Table_FieldDefs[j].cbField = sizeof(rowID);
			Table_FieldDefs[j].iFieldType = ROWID_FT;
		}
		else
		if (strcmp(messageHash[i],"BOOL_FT") == 0)
		{
			Table_FieldDefs[j].cbField = sizeof(BOOL);
			Table_FieldDefs[j].iFieldType = BOOL_FT;
		}
		else
		if (strcmp(messageHash[i],"VDN_FT") == 0)
		{
			Table_FieldDefs[j].cbField = sizeof(VDN);
			Table_FieldDefs[j].iFieldType = VDN_FT;
		}
		else
		{
			Tracef("Unsupported field type\n");
		}
		i++;
		
		if ((strcmp(messageHash[i],"y")== 0)||(strcmp(messageHash[i],"Y")== 0))
			Table_FieldDefs[j++].persistFlags = Persistant_PT ;
		else
			Table_FieldDefs[j++].persistFlags = NotPersistant_PT ;
		i++;
	}	
	#if 0	
	m_pciTable_FieldDefs = (fieldDef*)new(tPCI | tUNCACHED) char(sizeof(Table_FieldDefs));
	memcpy( (char*)m_pciTable_FieldDefs,
			(char*)Table_FieldDefs,
			sizeof(Table_FieldDefs)
		  ); 
	#else
	m_pciTable_FieldDefs = Table_FieldDefs;
	#endif	  
	m_pDefineTable = new TSDefineTable;

	status = m_pDefineTable->Initialize
	(
		this,									// Ddm* pDdm,
		messageHash[1],							// String64 prgbTableName,
//		j,										// U32 crgFieldDefs,
		m_pciTable_FieldDefs,					// fieldDef* prgFieldDefsRet,
		sizeof(Table_FieldDefs),				// U32 cbrgFieldDefs,
		j,										// U32 cEntriesRsv,
		false,									// bool* pfPersistant,
		(pTSCallback_t)&DoNothing,				// pTSCallback_t pCallback,
		NULL									// void* pContext
	);

	//  status checks need to be done on all initializes
	if (status == OS_DETAIL_STATUS_SUCCESS)
		m_pDefineTable->Send();
	else
		Tracef("Samurai::harnessTableCreate() status=%u\n",status);
}


STATUS SamuraiServer::DoNothing(void * pContext, Status status)
 {
 	//Do nothing 
 	
 	return OK;
 }

