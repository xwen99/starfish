#ifndef BOOK_H
#define BOOK_H

#include <cstdio>
#include "base.h"
#include "position.h"

struct BookStruct {
	union {
		uint32_t dwZobristLock;
		int nPtr;
	};
	uint16_t wmv, wvl;
}; // bk

inline int BOOK_POS_CMP(const BookStruct& bk, const PositionStruct& pos) {
	return bk.dwZobristLock < pos.zobr.dwLock1 ? -1 :
		bk.dwZobristLock > pos.zobr.dwLock1 ? 1 : 0;
}

struct BookFileStruct {
	FILE* fp;
	int nLen;
	bool Open(const char* szFileName, bool bEdit = false) {
		fp = fopen(szFileName, bEdit ? "r+b" : "rb");
		if (fp == NULL) {
			return false;
		}
		else {
			fseek(fp, 0, SEEK_END);
			nLen = ftell(fp) / sizeof(BookStruct);
			return true;
		}
	}
	void Close(void) const {
		fclose(fp);
	}
	void Read(BookStruct& bk, int nPtr) const {
		fseek(fp, nPtr * sizeof(BookStruct), SEEK_SET);
		fread(&bk, sizeof(BookStruct), 1, fp);
	}
	void Write(const BookStruct& bk, int nPtr) const {
		fseek(fp, nPtr * sizeof(BookStruct), SEEK_SET);
		fwrite(&bk, sizeof(BookStruct), 1, fp);
	}
};

// 获取开局库着法
int GetBookMoves(const PositionStruct& pos, const char* szBookFile, BookStruct* lpbks);

#endif
