//SamuraiServer.h

#ifndef __SamuraiServer_H
#define __SamuraiServer_H

#include "OsTypes.h"
#include "Message.h"
#include "Ddm.h"
#include "NetMsgs.h"
#include "PnPMsg.h"
#include "Table.h"
#include "Trace_Index.h"
#include "TraceMon.h"
#include "Odyssey_Trace.h"
#include "PTSCommon.h"


#define PLUS

class SamuraiServer: public Ddm {

public:
	char     pMessageBuf[132];
	char  	 messageHash[32][132];
	int      counter;
	char     tbl [132][32];	
	
	static Ddm *Ctor(DID);
	SamuraiServer(DID);

	STATUS Initialize(Message *);
	STATUS Enable(Message *);
	STATUS ListenReplyHandler(Message *);
	STATUS WriteReplyHandler(Message *);
	STATUS ReadReplyHandler(Message *);
	STATUS OnHoldCreateTable(Message *);

	int echomode;
	int iSessionID;

	TSDefineTable	*m_pDefineTable;		// for table creation
	fieldDef		*m_pciTable_FieldDefs;
	void harnessCreateTable ();
	STATUS DoNothing(void *, Status);
		
	void	breakup ();
	void 	build_lookup ();
	int     lookup (char *);

};


class node
{
	char   nBuffer[132];		// buffer to contain message
	char   result[20];	  	    // result string
	bool   expResponse; 		// true if needs response, false if not
	node * next;
	node * prev;
	
public:
	void enqueue();
	int dequeue();
	bool isEmpty();	
};

node * head;
node * tail;

#endif

