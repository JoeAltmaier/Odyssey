//SNMPTrapData.h

#ifndef __SNMPTrapData_H
#define __SNMPTrapData_H

#include "UnicodeString.h"

//used to store trap data while we wait to resolve all pieces

class SNMPTrapData {
public:

	UnicodeString m_usSourceName;
	UnicodeString m_usDescription;
	int m_iSNMPSeverity;
	int m_iTrapKey;

	SNMPTrapData() {
		m_iSNMPSeverity = 0;
		m_iTrapKey = 0;
	}

	~SNMPTrapData() {}

	void SetTrapKey(int iTrapKey) {
		m_iTrapKey = iTrapKey;
	}

	void SetSourceName(UnicodeString & usSourceName) {
		m_usSourceName = usSourceName;
	}

	void SetDescription(UnicodeString & usDescription) {
		m_usDescription = usDescription;
	}

	void SetSourceName(char* sSourceName) {
		StringClass stSourceName(sSourceName);
		UnicodeString usSourceName(stSourceName);
		SetSourceName(usSourceName);
	}

	void SetSNMPSeverity(int iSeverity) {
		m_iSNMPSeverity = iSeverity;
	}
};

#endif