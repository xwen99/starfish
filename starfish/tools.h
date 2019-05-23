#include <sys/timeb.h>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <windows.h>
#include <shlwapi.h>
#ifndef TOOLS_H
#define TOOLS_H

inline int64_t GetTime() {
	timeb tb;
	ftime(&tb);
	return (int64_t)tb.time * 1000 + tb.millitm;
}

inline void Idle(void) {
	Sleep(1);
}

inline char* strcasestr(const char* sz1, const char* sz2) {
	return StrStrI(sz1, sz2);
}

inline int strncasecmp(const char* sz1, const char* sz2, size_t n) {
	return _strnicmp(sz1, sz2, n);
}

inline void StrCutCrLf(char* sz) {
	char* lpsz;
	lpsz = strchr(sz, '\r');
	if (lpsz != NULL) {
		*lpsz = '\0';
	}
	lpsz = strchr(sz, '\n');
	if (lpsz != NULL) {
		*lpsz = '\0';
	}
}

inline bool StrEqv(const char* sz1, const char* sz2) {
	return strncasecmp(sz1, sz2, strlen(sz2)) == 0;
}

inline bool StrEqvSkip(const char*& sz1, const char* sz2) {
	if (strncasecmp(sz1, sz2, strlen(sz2)) == 0) {
		sz1 += strlen(sz2);
		return true;
	}
	else {
		return false;
	}
}

inline bool StrEqvSkip(char*& sz1, const char* sz2) {
	if (strncasecmp(sz1, sz2, strlen(sz2)) == 0) {
		sz1 += strlen(sz2);
		return true;
	}
	else {
		return false;
	}
}

inline bool StrScan(const char* sz1, const char* sz2) {
	return strcasestr(sz1, sz2) != NULL;
}

inline bool StrScanSkip(const char*& sz1, const char* sz2) {
	const char* lpsz;
	lpsz = strcasestr(sz1, sz2);
	if (lpsz == NULL) {
		return false;
	}
	else {
		sz1 = lpsz + strlen(sz2);
		return true;
	}
}

inline bool StrScanSkip(char*& sz1, const char* sz2) {
	char* lpsz;
	lpsz = strcasestr(sz1, sz2);
	if (lpsz == NULL) {
		return false;
	}
	else {
		sz1 = lpsz + strlen(sz2);
		return true;
	}
}

inline bool StrSplitSkip(const char*& szSrc, int nSeparator, char* szDst = NULL) {
	const char* lpsz;
	lpsz = strchr(szSrc, nSeparator);
	if (lpsz == NULL) {
		if (szDst != NULL) {
			strcpy(szDst, szSrc);
		}
		szSrc += strlen(szSrc);
		return false;
	}
	else {
		if (szDst != NULL) {
			strncpy(szDst, szSrc, lpsz - szSrc);
			szDst[lpsz - szSrc] = '\0';
		}
		szSrc = lpsz + 1;
		return true;
	}
}

inline bool StrSplitSkip(char*& szSrc, int nSeparator, char* szDst = NULL) {
	char* lpsz;
	lpsz = strchr(szSrc, nSeparator);
	if (lpsz == NULL) {
		if (szDst != NULL) {
			strcpy(szDst, szSrc);
		}
		szSrc += strlen(szSrc);
		return false;
	}
	else {
		if (szDst != NULL) {
			strncpy(szDst, szSrc, lpsz - szSrc);
			szDst[lpsz - szSrc] = '\0';
		}
		szSrc = lpsz + 1;
		return true;
	}
}

inline int Str2Digit(const char* sz, int nMin, int nMax) {
	int nRet;
	if (sscanf(sz, "%d", &nRet) > 0) {
		return min(max(nRet, nMin), nMax);
	}
	else {
		return nMin;
	}
}

struct RC4Struct {
	uint8_t s[256];
	int x, y;

	void InitZero(void) {
		int i, j;
		uint8_t uc;

		x = y = j = 0;
		for (i = 0; i < 256; i++) {
			s[i] = i;
		}
		for (i = 0; i < 256; i++) {
			j = (j + s[i]) & 255;
			uc = s[i];
			s[i] = s[j];
			s[j] = uc;
		}
	}

	uint8_t NextByte(void) {
		x = (x + 1) & 255;
		y = (y + s[x]) & 255;
		std::swap(s[x], s[y]);
		return s[(s[x] + s[y]) & 255];
	}

	uint32_t NextLong(void) {
		union {
			uint8_t uc[4];
			uint32_t dw;
		} Ret;
		Ret.uc[0] = NextByte();
		Ret.uc[1] = NextByte();
		Ret.uc[2] = NextByte();
		Ret.uc[3] = NextByte();
		return Ret.dw;
	}
};

#endif
