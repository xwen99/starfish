#include "base.h"

#ifndef RC4PRNG_H
#define RC4PRNG_H

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
		SWAP(s[x], s[y]);
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
