//WriteQue.cpp

#define _TRACEF
#include "Trace_Index.h"
#include "Odyssey_Trace.h"

#include <stdio.h>
#include <String.h>
#include "OsTypes.h"

#include "stdlib.h"
#include "String.h"

#include "WriteQue.h"

#define TOO_MANY_QUE_NODES 500
#define QUE_STOP_SIZE 250

WriteQue::WriteQue (int iPriorityQues,double* aWeights) {
	
	m_pfSet = 0x00;
	m_iQueSize = 0;

	if(iPriorityQues > PRIORITY_FIELD_SIZE) {
		Tracef("Que Manager is only able to have %d weights.",PRIORITY_FIELD_SIZE);
		iPriorityQues = PRIORITY_FIELD_SIZE;
	}

	m_iPriorityQues = iPriorityQues;
	m_aiQueSize = new int[m_iPriorityQues];
	m_apHead = new QueNode*[m_iPriorityQues];//a read que and a write que
	m_apTail = new QueNode*[m_iPriorityQues];
	
	int i;
	for(i=0;i<PRIORITY_FIELD_SIZE;i++) {
		m_arWeights[i] = new double[m_iPriorityQues];
	}

	for(i=0;i<m_iPriorityQues;i++) {
		int j;
		for(j=0;j<PRIORITY_FIELD_SIZE;j++) 
			m_arWeights[j][i] = 0.0;

		m_aiQueSize[i] = 0;
		m_apHead[i] = NULL;
		m_apTail[i] = NULL;
	}

	InitWeights(aWeights);
}

WriteQue::~WriteQue() {
	int i;

	for(i=0;i<PRIORITY_FIELD_SIZE;i++)
		delete[] m_arWeights[i];

	for(i=0;i<m_iPriorityQues;i++) {
		while( m_apHead[i] ) {
			m_apTail[i] = m_apHead[i]->m_pNext;
			delete m_apHead[i];
			m_apHead[i] = m_apTail[i];
		}
	}

	delete[] m_aiQueSize;
	delete[] m_apHead;
	delete[] m_apTail;
}

int WriteQue::GetQueSize() {
	return m_iQueSize;
}

bool WriteQue::QueAvailable() {
	if(m_iQueSize > QUE_STOP_SIZE)
		return false;
	return true;
}

void WriteQue::AddToQue(int iPriorityIndex,NetMsgWrite* pMsg) {
	if(!pMsg)
		return;

	QueNode* pNode;

	if(m_iQueSize > QUE_STOP_SIZE) {
		printf("QueNode very Large...%d nodes.\n",m_iQueSize);
	}

	pNode = new QueNode;

	m_iQueSize++;

	pNode->m_pMsg = pMsg;
	pNode->m_pNext = NULL;

	if(m_apTail[iPriorityIndex])
		m_apTail[iPriorityIndex]->m_pNext = pNode;
	else
		m_apHead[iPriorityIndex] = pNode;

	m_apTail[iPriorityIndex] = pNode;

	SetBit(iPriorityIndex);

}

bool WriteQue::GrabFromQue(QueNode* & pNode,int iPriorityIndex) {

	pNode = m_apHead[iPriorityIndex];
	if(!pNode) {
		return false;
	}

	m_iQueSize--;

	if(m_apTail[iPriorityIndex] == m_apHead[iPriorityIndex])
		m_apTail[iPriorityIndex] = NULL;
	if(m_apHead[iPriorityIndex])
		m_apHead[iPriorityIndex] = m_apHead[iPriorityIndex]->m_pNext;
	if(!m_apHead[iPriorityIndex])
		UnsetBit(iPriorityIndex);

	return true;
}

bool WriteQue::CheckQue(QueNode* & pNode, int& iIndex, bool bPeek) {

	iIndex = ChooseQueIndex();
	if(iIndex < 0) {
		pNode = NULL;
		return false;
	}
	pNode = m_apHead[iIndex];

	if(!pNode) {
		Tracef("IMPOSSIBLE WriteQue::CheckQue() Condition!!");
		if(BitSet(m_pfSet,iIndex))
			UnsetBit(iIndex);
		return false;
	}

	if(bPeek)
		return true;

	m_iQueSize--;

	if(m_apTail[iIndex] == m_apHead[iIndex])
		m_apTail[iIndex] = NULL;
	if(m_apHead[iIndex])
		m_apHead[iIndex] = m_apHead[iIndex]->m_pNext;
	if(!m_apHead[iIndex])
		UnsetBit(iIndex);

	return true;
}

int WriteQue::ChooseQueIndex() {
	double rRand = ((double)(rand() % 100))/(double)100.0;
	double rBottom = 0.0;
	if(m_pfSet) {
		for(int i=1;i<=m_iPriorityQues;i++) {
			if(rRand >= rBottom && rRand < rBottom + m_arWeights[m_pfSet - 1][i - 1]) {
				return i - 1;//to avoid zero problems
			}
			else 
				rBottom = m_arWeights[m_pfSet - 1][i - 1];
		}
	}
	return -1;
}

void WriteQue::SetBit(int iBit) {
	PRIORITY_FIELD_TYPE pfBitSet = 0x01;
	pfBitSet <<= iBit;
	m_pfSet |= pfBitSet;
}

void WriteQue::UnsetBit(int iBit) {
	PRIORITY_FIELD_TYPE pfBitSet = 0x01;
	pfBitSet <<= iBit;
	m_pfSet ^= pfBitSet;
}

bool WriteQue::BitSet(PRIORITY_FIELD_TYPE pfIndex,int iBit) {
	PRIORITY_FIELD_TYPE pfBitSet = 0x01;
	
	pfBitSet <<= iBit;//iBit is an index, we need it to be a bit
	pfBitSet |= pfIndex;
	pfBitSet ^= pfIndex;

	return ! pfBitSet;
}

void WriteQue::InitWeights(double* aWeights) {
	bool bRangeFound = false;
	for(PRIORITY_FIELD_TYPE i=1;i<=PRIORITY_FIELD_SIZE;i++) {
		//first compute the range
		double rRange = 0.0;
		int j;
		for(j=0;j<m_iPriorityQues;j++) {
			if(BitSet(i,j)) {
				rRange += aWeights[j]/2.0;//read gets half, write gets half
				if(rRange > 0.0)
					bRangeFound = true;
			}
		}
		//now compute the weights
		for(j=0;j<m_iPriorityQues;j++) {
			if(BitSet(i,j)) {
				m_arWeights[i - 1][j] = (aWeights[j]/2.0) / rRange;//read gets half, write gets half
			}
			else {
				m_arWeights[i - 1][j] = 0.0;
			}
		}
	}
	if(!bRangeFound)
		Tracef("WriteQue Weights are not valid... all zero.");
}