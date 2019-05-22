#include "position.h"
#include "book.h"

int GetBookMoves(const PositionStruct& pos, BookStruct* lpbks) {
	BookDataStruct BookData;
	PositionStruct posScan;
	BookStruct bk;
	int nScan, nLow, nHigh, nPtr;
	int i, j, nMoves;
	// 从开局库中搜索着法的例程，有以下几个步骤：
	BookData.init();

	// 1. 用拆半查找法搜索局面；
	posScan = pos;
	for (nScan = 0; nScan < 2; nScan++) {
		nPtr = nLow = 0;
		nHigh = BookData.nLen - 1;
		while (nLow <= nHigh) {
			nPtr = (nLow + nHigh) / 2;
			BookData.Read(bk, nPtr);
			if (BOOK_POS_CMP(bk, posScan) < 0) {
				nLow = nPtr + 1;
			}
			else if (BOOK_POS_CMP(bk, posScan) > 0) {
				nHigh = nPtr - 1;
			}
			else {
				break;
			}
		}
		if (nLow <= nHigh) {
			break;
		}
		// 原局面和镜像局面各搜索一趟
		pos.Mirror(posScan);
	}

	// 2. 如果不到局面，则返回空着；
	if (nScan == 2) {
		return 0;
	}
	__ASSERT_BOUND(0, nPtr, BookData.nLen - 1);

	// 3. 如果找到局面，则向前查找第一个着法；
	for (nPtr--; nPtr >= 0; nPtr--) {
		BookData.Read(bk, nPtr);
		if (BOOK_POS_CMP(bk, posScan) < 0) {
			break;
		}
	}

	// 4. 向后依次读入属于该局面的每个着法；
	nMoves = 0;
	for (nPtr++; nPtr < BookData.nLen; nPtr++) {
		BookData.Read(bk, nPtr);
		if (BOOK_POS_CMP(bk, posScan) > 0) {
			break;
		}
		if (posScan.LegalMove(bk.wmv)) {
			// 如果局面是第二趟搜索到的，则着法必须做镜像
			lpbks[nMoves].nPtr = nPtr;
			lpbks[nMoves].wmv = (nScan == 0 ? bk.wmv : MOVE_MIRROR(bk.wmv));
			lpbks[nMoves].wvl = bk.wvl;
			nMoves++;
			if (nMoves == MAX_GEN_MOVES) {
				break;
			}
		}
	}

	// 5. 对着法按分值排序
	for (i = 0; i < nMoves - 1; i++) {
		for (j = nMoves - 1; j > i; j--) {
			if (lpbks[j - 1].wvl < lpbks[j].wvl) {
				SWAP(lpbks[j - 1], lpbks[j]);
			}
		}
	}
	return nMoves;
}
