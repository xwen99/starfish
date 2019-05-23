#include "position.h"
#include "book.h"

int GetBookMoves(const PositionStruct& pos, BookStruct* lpbks) {
	BookDataStruct BookData;
	PositionStruct posScan;
	BookStruct bk;
	int nScan, nLow, nHigh, nPtr;
	int i, j, nMoves;
	// �ӿ��ֿ��������ŷ������̣������¼������裺
	BookData.init();

	// 1. ���۰���ҷ��������棻
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
		// ԭ����;�����������һ��
		pos.Mirror(posScan);
	}

	// 2. ����������棬�򷵻ؿ��ţ�
	if (nScan == 2) {
		return 0;
	}

	// 3. ����ҵ����棬����ǰ���ҵ�һ���ŷ���
	for (nPtr--; nPtr >= 0; nPtr--) {
		BookData.Read(bk, nPtr);
		if (BOOK_POS_CMP(bk, posScan) < 0) {
			break;
		}
	}

	// 4. ������ζ������ڸþ����ÿ���ŷ���
	nMoves = 0;
	for (nPtr++; nPtr < BookData.nLen; nPtr++) {
		BookData.Read(bk, nPtr);
		if (BOOK_POS_CMP(bk, posScan) > 0) {
			break;
		}
		if (posScan.LegalMove(bk.wmv)) {
			// ��������ǵڶ����������ģ����ŷ�����������
			lpbks[nMoves].nPtr = nPtr;
			lpbks[nMoves].wmv = (nScan == 0 ? bk.wmv : MOVE_MIRROR(bk.wmv));
			lpbks[nMoves].wvl = bk.wvl;
			nMoves++;
			if (nMoves == MAX_GEN_MOVES) {
				break;
			}
		}
	}

	// 5. ���ŷ�����ֵ����
	for (i = 0; i < nMoves - 1; i++) {
		for (j = nMoves - 1; j > i; j--) {
			if (lpbks[j - 1].wvl < lpbks[j].wvl) {
				std::swap(lpbks[j - 1], lpbks[j]);
			}
		}
	}
	return nMoves;
}
