//WriteQue.hpp

#ifndef __WriteQue_H
#define __WriteQue_H

#include "OsTypes.h"
#include "NetMsgs.h"

#define QUE_NODE_GROWTH_SIZE 500

struct QueNode {
	NetMsgWrite* m_pMsg;
	QueNode* m_pNext;
};

#define PRIORITY_FIELD_TYPE unsigned short
#define PRIORITY_FIELD_SIZE (sizeof(PRIORITY_FIELD_TYPE) * 4)

class WriteQue {
public:
	WriteQue(int iPriorityQues,double* aWeights);
	~WriteQue();

	bool GrabFromQue(QueNode* & pNode,int iPriorityIndex);
	bool QueAvailable();

	void AddToQue(int iPriorityIndex,NetMsgWrite* pMsg);

	bool CheckQue(QueNode* & pNode, int& iIndex, bool bPeek);

	int GetQueSize();
private:
	int m_iQueSize;
	int ChooseQueIndex();

	void UnsetBit(int iBit);
	bool BitSet(PRIORITY_FIELD_TYPE pfIndex,int iBit);
	void SetBit(int iBit);
	void InitWeights(double* aWeights);
	PRIORITY_FIELD_TYPE m_pfSet;

	double* m_arWeights[PRIORITY_FIELD_SIZE];

	int m_iPriorityQues;
	int* m_aiQueSize;
	QueNode** m_apHead;
	QueNode** m_apTail;
};

#endif