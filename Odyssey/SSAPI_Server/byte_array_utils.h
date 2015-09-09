//byte_array_utils.hpp

#ifndef __byte_array_utils_H
#define __byte_array_utils_H

class byte_array_utils {
public:
static int Write(char* pDest, char* pSource, int iSize) {
#ifndef WIN32
	memcpy(pDest, pSource,iSize);
#else 
	for(int i=0;i<iSize;i++) {
		pDest[i] = pSource[iSize - i - 1];
	}
#endif
	return iSize;
}

};

#endif