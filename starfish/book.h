#include <cstdio>
#include "buffer.h"
#include "position.h"

#ifndef BOOK_H
#define BOOK_H


inline int BOOK_POS_CMP(const BookStruct& bk, const PositionStruct& pos) {
	return bk.dwZobristLock < pos.zobr.dwLock1 ? -1 :
		bk.dwZobristLock > pos.zobr.dwLock1 ? 1 : 0;
}

struct BookDataStruct {
	int nLen;
	void init() {
		nLen = 12081;
		return;
	}
	void Read(BookStruct& bk, int nPtr) const {
		bk = bookBuffer[nPtr];
	}
};

// 获取开局库着法
int GetBookMoves(const PositionStruct& pos, BookStruct* lpbks);

#endif
