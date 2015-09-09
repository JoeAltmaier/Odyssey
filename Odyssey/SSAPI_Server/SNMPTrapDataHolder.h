//SNMPTrapDataHolder.h

#ifndef _SNMPTrapDataHolder_H
#define _SNMPTrapDataHolder_H

#include "SNMPTrapData.h"

class SNMPTrapDataHolder {

	SNMPTrapData** m_apSNMPTrapData;
	int m_iTraps;

	int m_iTrapKey;

public:
	SNMPTrapDataHolder() {
		m_apSNMPTrapData = NULL;
		m_iTraps = 0;
		m_iTrapKey = 1;
	}

	~SNMPTrapDataHolder() {
		if(m_apSNMPTrapData) {

			for(int i=0;i<m_iTraps;i++) {
				if(m_apSNMPTrapData[i])
					delete m_apSNMPTrapData[i];
			}

			delete[] m_apSNMPTrapData;
		}
	}

	//returns the trap key
	int AddTrapData() {

		for(int i=0;i<m_iTraps;i++) {
			if(! m_apSNMPTrapData[i]) {
				m_apSNMPTrapData[i] = new SNMPTrapData();
				m_apSNMPTrapData[i]->SetTrapKey(m_iTrapKey++);
				return m_iTrapKey - 1;
			}
		}

		SNMPTrapData** aNew = new SNMPTrapData*[m_iTraps + 1];
		if(m_apSNMPTrapData) {
			memcpy(aNew, m_apSNMPTrapData, sizeof(SNMPTrapData*) * m_iTraps);
			delete[] m_apSNMPTrapData;
		}
		m_apSNMPTrapData = aNew;
		m_apSNMPTrapData[m_iTraps++] = new SNMPTrapData();
		m_apSNMPTrapData[m_iTraps - 1]->SetTrapKey(m_iTrapKey++);
		return m_iTrapKey - 1;
	}

	SNMPTrapData* GetTrapData(int iTrapKey) {
		for(int i=0;i<m_iTraps;i++) {
			if(m_apSNMPTrapData[i]) {
				if(m_apSNMPTrapData[i]->m_iTrapKey == iTrapKey) {
					return m_apSNMPTrapData[i];
				}
			}
		}
		return NULL;
	}

	void RemoveTrapData(int iTrapKey) {
		for(int i=0;i<m_iTraps;i++) {
			if(m_apSNMPTrapData[i]) {
				if(m_apSNMPTrapData[i]->m_iTrapKey == iTrapKey) {
					delete m_apSNMPTrapData[i];
					m_apSNMPTrapData[i] = NULL;
					break;
				}
			}
		}
	}
};

#endif