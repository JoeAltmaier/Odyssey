/* DdmTestManager.cpp -- Manages keeping track of and launching test DDMs
 *
 * Copyright (C) ConvergeNet Technologies, 1998,99
 *
 * This material is a confidential trade secret and proprietary 
 * information of ConvergeNet Technologies, Inc. which may not be 
 * reproduced, used, sold or transferred to any third party without the 
 * prior written consent of ConvergeNet Technologies, Inc.  This material 
 * is also copyrighted as an unpublished work under sections 104 and 408 
 * of Title 17 of the United States Code.  Law prohibits unauthorized 
 * use, copying or reproduction.
 *
 * Description:
 * 		TestManager accepts registration of TestDdm's, and allows
 *		clients to run these tests through a central interface
 *
 *		Need to convert the TestName and TestArgs to either a character array
 *		or SGL to work across boards
 *
**/

#ifndef _TRACEF
#define _TRACEF
#include "Odyssey_Trace.h"
#endif

#define	TRACE_INDEX		TRACE_TEST_MGR

#define MAXSTRUCTSIZE 1000

// Public Includes
#include <String.h>

#include "DdmManager.h"
#include "BuildSys.h"

#include "DdmTestManager.h"
#include "TestTable.h"
#include "TestMsgs.h"
#include "Network.h"
#include <ctype.h>

// BuildSys Linkage

CLASSNAME(DdmTestManager, SINGLE);

//SERVELOCAL(DdmTestManager, REQ_TEST_REGISTER);
//SERVELOCAL(DdmTestManager, REQ_TEST_RUN);
SERVEVIRTUAL(DdmTestManager, REQ_TEST_REGISTER);
SERVEVIRTUAL(DdmTestManager, REQ_TEST_RUN);

DdmTestManager::DdmTestManager(DID did) : Ddm(did) {

	TRACE_ENTRY(DdmTestManager::DdmTestManager(DID did));

}

Ddm *DdmTestManager::Ctor(DID did)
{

	TRACE_ENTRY(DdmTestManager::Ctor(DID did));
	return (new DdmTestManager(did));

}

STATUS DdmTestManager::Initialize(Message *pMsg)
{

	TRACE_ENTRY(DdmTestManager::Initialize(Message*));

	DispatchRequest(REQ_TEST_REGISTER, REQUESTCALLBACK(DdmTestManager, ProcessRegistration));
	DispatchRequest(REQ_TEST_RUN, REQUESTCALLBACK(DdmTestManager, ProcessRun));
	
	Reply(pMsg, OK);
	return OK;

}

STATUS DdmTestManager::Enable(Message *pMsg)
{

	TRACE_ENTRY(DdmTestManager::Enable(Message*));

	Reply(pMsg, OK);
	return OK;

}

STATUS DdmTestManager::Quiesce (Message *pMsg)
{ 

	TRACE_ENTRY(DdmTestManager::Quiesce(Message*));   
   
   //  * * *  do local Quiesce stuff here.  * * *

   Reply (pMsg, OK);
   return OK;      // (must *always* return success)

}

STATUS DdmTestManager::ProcessRegistration(Message *pMsgReq)
{

	TRACE_ENTRY(DdmTestManager::ProcessRegistration(Message*));   

	TestMsgRegister* pMsg = (TestMsgRegister *)pMsgReq;
	
	int iSize;
	char *pTestName;
	char *pTestArgs;
	
	pMsg->GetName(pTestName, iSize);
	pMsg->GetArgs(pTestArgs, iSize);
	
	TRACEF(TRACE_L8, ("------------------------------------\n"));	
	TRACEF(TRACE_L8, (" Test Process Registration Received\n"));
	TRACEF(TRACE_L8, ("------------------------------------\n"));	
	TRACEF(TRACE_L8, ("Test Name: 		%s\n", pTestName));	
	TRACEF(TRACE_L8, ("Test Arg List:	%s\n", pTestArgs));		
	TRACEF(TRACE_L8, ("DID:				%d\n", pMsg->did));			
	TRACEF(TRACE_L8, ("Slot:			%d\n", pMsg->slot));				
	TRACEF(TRACE_L8, ("------------------------------------\n\n"));	
	
	char *pFullName;
	
	// Remove warning
	pFullName = new char[80];

	// Add slot number to name conversion switch statement
	// For now use slot number in hex
	sprintf(pFullName, "%02X.%s", pMsg->slot, pTestName);

	Os::AddTestEntry(pFullName, pTestArgs, pMsg->did, pMsg->slot);
	
	delete[] pFullName;

	Reply(pMsg, OK);
	return OK;

}

STATUS DdmTestManager::ProcessRun(Message *pMsgReq)
{

	STATUS status;

	TRACE_ENTRY(DdmTestManager::ProcessRun(Message*));   

	TestMsgRun* pMsg = (TestMsgRun *)pMsgReq;	

	int iSize;
	char *pTestName;
	char *pTestArgs;
	
	pMsg->GetName(pTestName, iSize);
	pMsg->GetArgs(pTestArgs, iSize);

	TRACEF(TRACE_L8, ("---------------------------\n"));	
	TRACEF(TRACE_L8, (" Test Process Run Received\n"));
	TRACEF(TRACE_L8, ("---------------------------\n"));	
	TRACEF(TRACE_L8, ("Test Name: 		%s\n", pTestName));	
	TRACEF(TRACE_L8, ("Test Args:		%s\n", pTestArgs));		
	TRACEF(TRACE_L8, ("Handle:			%d\n", pMsg->iTestHandle));				
	TRACEF(TRACE_L8, ("---------------------------\n"));

	TestEntry* foundEntry = TestTable::Find(pTestName);
	if (foundEntry)
	{
		TRACEF(TRACE_L8, (" Test Process Match Found!\n"));
		TRACEF(TRACE_L8, ("---------------------------\n"));	
		TRACEF(TRACE_L8, ("DID:				%d\n", foundEntry->did));			
		TRACEF(TRACE_L8, ("Slot:			%d\n", foundEntry->slot));				
//		TRACEF(TRACE_L8, ("Function:		%8x\n", foundEntry->function));	
		TRACEF(TRACE_L8, ("---------------------------\n\n"));			
	}
	else
	{
		TRACEF(TRACE_L8, (" No Test Match Found!!!!!!\n"));
		TRACEF(TRACE_L8, ("---------------------------\n\n"));
		Reply(pMsg, UNKNOWN_TEST);
		return OK;
	}

	TRACEF(TRACE_L8, ("Sending request to Test DDM %s\n", pTestName));
	
	// Parse the argument stream against the rules list
	// Make a 1k buffer to pass in if necessary
	char* bitpack = new char[MAXSTRUCTSIZE];
	status = ParseString(pTestArgs, foundEntry->pszRules, bitpack);
	// if we couldn't parse it, reply right now!
	if (status == -1)
	{
	  Reply(pMsg, PARSE_ERROR);
	  return OK;	
	}
	// Pack into SGL
	TestMsgRun* pMsgTest = new TestMsgRun(pTestName, pTestArgs, pMsg->iTestHandle);
	pMsgTest->SetStruct(bitpack, status);
	delete[] bitpack;
	pMsgTest->SetRunRequest(pMsg);
	Send(foundEntry->did, pMsgTest, REPLYCALLBACK(DdmTestManager, InputStream));

	return OK;
}

// Received a replycallback from the Test DDM
// If non-last reply, send back to initial caller with
// The correct handle.  If last reply, the Test DDM is
// done and thus so are we
STATUS DdmTestManager::InputStream(Message *pMsgReq)
{

	TRACE_ENTRY(DdmTestManager::InputStream(Message*));

	TestMsgRun* pMsg = (TestMsgRun *)pMsgReq;
	
	if (!pMsgReq->IsLast())
	{
		char* pBuf;
		int iSize;
		pMsg->GetBuffer(pBuf, iSize);	
		TestMsgRun* pMsgReply = new TestMsgRun(pMsg->pRunRequestMsg);
		pMsgReply->SetWriteBuffer(pBuf, iSize);
		Reply(pMsgReply, OK, false);
	}
	else
	{	
	// should we try and get the buffer or spec the api as having
	// last message be bufferless?
		Reply(pMsg->pRunRequestMsg, OK);
		
	}
	
	delete pMsg;
	return OK;

}

// rules format:
//  "0d 4d 8c 9w 13s"
// where
//  d:  int
//  c:  char
//  w:  word
//  s:  string
//  i: ip address

int DdmTestManager::getWord(char *stringfull, int position, char *word)
{

  int i;
  
  for(i=position; i < (int)strlen(stringfull); i++)
  {
    if ((stringfull[i] == ' ') || (stringfull[i] == 0))
    {
      word[i-position] = 0;
      return i;
    }
    word[i-position] = stringfull[i];
  }

  word[i-position] = 0;
  return i;

}

int DdmTestManager::getQuotedString(char *stringfull, int position, char *word)
{

  int i;
  
  if (stringfull[position++] != '\'')
  {
    TRACEF(TRACE_L4, ("Invalid String!\n"));
	return(-1);
  }

  for(i=position; i < (int)strlen(stringfull); i++)
  {
    if ((stringfull[i] == '\'') || (stringfull[i] == 0))
    {
      word[i-position] = 0;
      return (i+1);
    }
    word[i-position] = stringfull[i];
  }

//  printf("how did we get here?\n");

  word[i-position] = 0;
  return (i+1);

}

int DdmTestManager::getIPFromString(char *stringfull, int position, char *ipout)
{

  int i;
  int ip = 0;
  int curr = 0;
  int ippos = 0;

  for (i=position; i < (int)strlen(stringfull); i++)
  {
    if (!isDigit(stringfull[i]) && !(stringfull[i] == '.') && !(stringfull[i] == ' '))
	{
      TRACEF(TRACE_L4, ("Invalid IP Address!\n"));	
	  return(-1);
	}

	if (isDigit(stringfull[i]))
	{
      curr = curr*10 + stringfull[i] - 48;
	}
	else if (stringfull[i] == '.')
	{
      ip = ip | (curr << ((3-ippos++)*8));
	  curr = 0;
	}
	else if (stringfull[i] == ' ')
	{
      if (ippos == 3)
	  {
        ip = ip | curr;
		memcpy(ipout, &ip, sizeof(int));
		return(0);
	  }
	  else
	  {
        TRACEF(TRACE_L4, ("Invalid IP Address!\n"));	
		return(-1);
	  }
	}
  }
  // Must be @ the end of a string to parse
  if (ippos == 3)
  {
    ip = ip | curr;
	memcpy(ipout, &ip, sizeof(int));
    return(0);
  }
  else
  {
    TRACEF(TRACE_L4, ("Invalid IP Address!\n"));	
	return(-1);
  }  

}

bool DdmTestManager::isDigit(char ch)
{

	return (ch >= '0' && ch <= '9' ? true : false);
	
}

// Damn we really need to get a common atoi!! This one is way too simple
int atoi (char *s)
{
	int i = 0;
	while(s && *s && isdigit(*s))
	{
		i = i * 10 + (*s - '0');
		++s;
	}
	return i;
}

STATUS DdmTestManager::ParseString(char *inStr, char *ruleStr, char *bitpack)
{

  char currWord[200];
  int pos, i, tempi, rulenum;
  int packsize;

  enum rule_types {Integer, String, Word, Char, IP};

  TRACEF(TRACE_L4, ("Rules: <%s>\n", ruleStr));
  TRACEF(TRACE_L4, ("Input: <%s>\n\n", inStr));

  // Prescan rules to get number of rules
  rulenum = 0;
  for (pos=0; pos < (int)strlen(ruleStr); pos++)
  {

	// Get the location
	tempi = 0;
    while (isDigit(ruleStr[pos]))
	{
      tempi = 10*tempi + ruleStr[pos++] - 48;
	}

	pos++;
	rulenum++;

  }

  // Allocate space to store the rules
  TRACEF(TRACE_L4, ("Allocating memory for %d rules\n", rulenum));
  Rule* rules = new Rule[rulenum];

  // Parse rules
  rulenum = 0;
  for (pos=0; pos < (int)strlen(ruleStr); pos++)
  {

	// Get the location
	tempi = 0;
    while (isDigit(ruleStr[pos]))
	{
      tempi = 10*tempi + ruleStr[pos++] - 48;
	}

	// We are at the type specifying character
	switch(ruleStr[pos++])
	{
	  case 'd':
	    TRACEF(TRACE_L4, ("Integer @ %d\n", tempi));
		rules[rulenum].pos = tempi;
		rules[rulenum++].type = Integer;
		break;
	  case 'c':
		TRACEF(TRACE_L4, ("Char @ %d\n", tempi));
		rules[rulenum].pos = tempi;
		rules[rulenum++].type = Char;
		break;
	  case 'w':
		TRACEF(TRACE_L4, ("Word @ %d\n", tempi));
		rules[rulenum].pos = tempi;
		rules[rulenum++].type = Word;
		break;
	  case 's':
		TRACEF(TRACE_L4, ("String @ %d\n", tempi));
		rules[rulenum].pos = tempi;
		rules[rulenum++].type = String;
		break;
	  case 'i':
		TRACEF(TRACE_L4, ("IP @ %d\n", tempi));
		rules[rulenum].pos = tempi;
		rules[rulenum++].type = IP;
		break;
      default:
		TRACEF(TRACE_L4, ("Invalid format specifier\n"));
	}
  }

  // allocate space for buffer to pass to function

  pos = 0;
  // Parse input string
  for (i=0; i < (rulenum); i++)
  {
	if ((pos >= 0) && ( pos <= (int)strlen(inStr)))
	{
	  switch (rules[i].type) {

        case Integer:
          pos = getWord(inStr, pos, currWord) + 1;
          tempi = atoi(currWord);
		  memcpy((void *)&bitpack[rules[i].pos], &tempi, sizeof(int));
		  packsize = rules[i].pos + sizeof(int);
          TRACEF(TRACE_L4, ("   Int: %d\n", tempi));
          break;
      
        case String:
		  pos = getQuotedString(inStr, pos, currWord) + 1;
		  memcpy(&bitpack[rules[i].pos], currWord, strlen(currWord)+1);
		  packsize = rules[i].pos + strlen(currWord) + 1;
		  TRACEF(TRACE_L4, ("String: %s\n", currWord));
          break;

        case Word:
          pos = getWord(inStr, pos, currWord) + 1;
		  memcpy((void *)&bitpack[rules[i].pos], currWord, strlen(currWord)+1);
		  packsize = rules[i].pos + strlen(currWord) + 1;
          TRACEF(TRACE_L4, ("  Word: %s\n", currWord));
          break;

        case Char:
          TRACEF(TRACE_L4, ("  Char: %c\n", inStr[pos]));
		  memcpy((void *)&bitpack[rules[i].pos], &inStr[pos], sizeof(char));
		  packsize = rules[i].pos + sizeof(char);
 		  pos += 2;
          break;

        case IP:
		  pos = getWord(inStr, pos, currWord) + 1;
		  getIPFromString(currWord, 0, (char *)&tempi);
		  TRACEF(TRACE_L4, ("    IP: %d.%d.%d.%d\n", IPa(tempi), IPb(tempi), IPc(tempi), IPd(tempi)));
		  memcpy((void *)&bitpack[rules[i].pos], &tempi, sizeof(int));
		  packsize = rules[i].pos + sizeof(int);
		  break;

        default:
		  TRACEF(TRACE_L4, ("Invalid input string!\n"));
		  return(-1);
          break;
	  }
	}  
	else
	{
      TRACEF(TRACE_L4, ("Invalid input string!\n"));
	  return(-1);
	}
    
  }
  
  return(packsize);

}