#include <cassert>
#include <sys/timeb.h>
#include <cstdint>
#include <windows.h>

#ifndef BASE_H
#define BASE_H

#define __ASSERT(a) assert(a)
#define __ASSERT_BOUND(a, b, c) assert((a) <= (b) && (b) <= (c))
#define __ASSERT_BOUND_2(a, b, c, d) assert((a) <= (b) && (b) <= (c) && (c) <= (d))

inline bool EQV(bool bArg1, bool bArg2) {
	return bArg1 ? bArg2 : !bArg2;
}

inline bool XOR(bool bArg1, bool bArg2) {
	return bArg1 ? !bArg2 : bArg2;
}

template <typename T> inline T MIN(T Arg1, T Arg2) {
	return Arg1 < Arg2 ? Arg1 : Arg2;
}

template <typename T> inline T MAX(T Arg1, T Arg2) {
	return Arg1 > Arg2 ? Arg1 : Arg2;
}

template <typename T> inline T ABS(T Arg) {
	return Arg < 0 ? -Arg : Arg;
}

template <typename T> inline T SQR(T Arg) {
	return Arg * Arg;
}

template <typename T> inline void SWAP(T& Arg1, T& Arg2) {
	T Temp;
	Temp = Arg1;
	Arg1 = Arg2;
	Arg2 = Temp;
}

inline int64_t GetTime() {
	timeb tb;
	ftime(&tb);
	return (int64_t)tb.time * 1000 + tb.millitm;
}

inline void Idle(void) {
	Sleep(1);
}

#endif