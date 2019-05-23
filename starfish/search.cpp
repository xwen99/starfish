#include <cstdio>
#include "position.h"
#include "hash.h"
#include "ucci.h"
#include "book.h"
#include "movesort.h"
#include "search.h"

const int IID_DEPTH = 2;         // �ڲ�������������
const int SMP_DEPTH = 6;         // �������������
const int UNCHANGED_DEPTH = 4;   // δ�ı�����ŷ������

const int DROPDOWN_VALUE = 20;   // ���ķ�ֵ
const int RESIGN_VALUE = 300;    // ����ķ�ֵ
const int DRAW_OFFER_VALUE = 40; // ��͵ķ�ֵ

SearchStruct Search;
PositionStruct& Pos = Search.pos;
// ������Ϣ���Ƿ�װ��ģ���ڲ���
static struct {
	int64_t llTime;                     // ��ʱ��
	uint32_t mvResult;					// ������
	bool bStop;				            // ��ֹ�ź�
	int nUnchanged;                     // δ�ı�����ŷ������
	uint16_t wmvKiller[LIMIT_DEPTH][2]; // ɱ���ŷ���
	HashStruct HashTable[HASH_SIZE];	// �û���
	int nHistoryTable[65536];			// ��ʷ��
	MoveSortStruct MoveSort;            // �������ŷ�����
} Search2;
HashStruct* hshItems = Search2.HashTable;
uint16_t(*wmvKiller)[2] = Search2.wmvKiller;
int* nHistory = Search2.nHistoryTable;

void BuildPos(PositionStruct& pos, const UcciCommStruct& UcciComm) {
	int i, mv;
	pos.FromFen(UcciComm.szFenStr);
	for (i = 0; i < UcciComm.nMoveNum; i++) {
		mv = COORD_MOVE(UcciComm.lpdwMovesCoord[i]);
		if (mv == 0) {
			break;
		}
		if (pos.LegalMove(mv) && pos.MakeMove(mv) && pos.Captured()) {
			// ʼ����pos.nMoveNum��ӳû���ӵĲ���
			pos.SetIrrev();
		}
	}
}

// �ظ��ü�
static int RepPruning(const PositionStruct& pos, int vlBeta) {
	int vlRep = pos.RepStatus();
	if (vlRep > 0) {
		return pos.RepValue(vlRep);
	}
	return -MATE_VALUE;
}

// �����;������ۺ���
inline int Evaluate(const PositionStruct& pos) {
	int vl;
	vl = pos.Evaluate();
	return vl == pos.DrawValue() ? vl - 1 : vl;
}

// ��̬��������
static int SearchQuiesc(PositionStruct& pos, int vlAlpha, int vlBeta) {
	int vlBest, vl, nGenMoves;
	int mvs[MAX_GEN_MOVES];
	// ��̬�������̰������¼������裺
	// 1. �ظ��ü���
	vl = RepPruning(pos, vlBeta);
	if (vl > -MATE_VALUE) {
		return vl;
	}
	// 2. �ﵽ������ȣ�ֱ�ӷ�������ֵ��
	if (pos.nDistance == LIMIT_DEPTH) {
		return Evaluate(pos);
	}
	// 3. ��ʼ����
	vlBest = -MATE_VALUE;
	// 4. ���ڱ������ľ��棬����ȫ���ŷ���
	if (pos.InCheck()) {
		nGenMoves = pos.GenMoves(mvs);
		std::sort(mvs, mvs + nGenMoves, CompareHistory);
	}
	else {
		// 5. ����δ�������ľ��棬�������ŷ�ǰ���ȳ��Կ���(��������)�����Ծ��������ۣ�
		vl = Evaluate(pos);
		if (vl >= vlBeta) {
			return vl;
		}
		vlBest = vl;
		vlAlpha = max(vl, vlAlpha);
		// 6. ����δ�������ľ��棬���ɲ��������г����ŷ�(MVV(LVA)����)��
		nGenMoves = pos.GenMoves(mvs, GEN_CAPTURE);
		std::sort(mvs, mvs + nGenMoves, CompareMvvLva);
	}
	// 7. ��Alpha-Beta�㷨������Щ�ŷ���
	for (int i = 0; i < nGenMoves; i++) {
		if (pos.MakeMove(mvs[i])) {
			vl = -SearchQuiesc(pos, -vlBeta, -vlAlpha);
			pos.UndoMakeMove();
			if (vl > vlBest) {
				if (vl >= vlBeta) {
					return vl;
				}
				vlBest = vl;
				vlAlpha = max(vl, vlAlpha);
			}
		}
	}
	// 8. ���ط�ֵ��
	if (vlBest == -MATE_VALUE) {
		return pos.nDistance - MATE_VALUE;
	}
	else {
		return vlBest;
	}
}

const bool NO_NULL = false; // "SearchPV()"�Ĳ������Ƿ��ֹ���Ųü�

// ��������������
static int SearchPV(int vlAlpha, int vlBeta, int nDepth, bool bNoNull = false) {
	int nNewDepth, nHashFlag, vlBest, vl, nCurrTimer;
	int mvBest, mvHash, mv;
	MoveSortStruct MoveSort;
	// ��ȫ�������̰������¼������裺
	// 1. ��Ҷ�ӽ�㴦���þ�̬������
	if (nDepth <= 0) {
		//		return Evaluate(Search.pos);
		return SearchQuiesc(Search.pos, vlAlpha, vlBeta);
	}
	// 2. �޺��ü���
	vl = RepPruning(Search.pos, vlBeta);
	if (vl > -MATE_VALUE) {
		return vl;
	}
	// 3. �û��ü���
	vl = ProbeHash(Search.pos, vlAlpha, vlBeta, nDepth, mvHash);
	if (Search.bUseHash && vl > -MATE_VALUE) {
		// ����PV��㲻�����û��ü������Բ��ᷢ��PV·���жϵ����
		return vl;
	}
	// 4. �ﵽ������ȣ�ֱ�ӷ�������ֵ��
	if (Search.pos.nDistance == LIMIT_DEPTH) {
		return Evaluate(Search.pos);
	}
	// 5. ���Կ��Ųü���
	if (bNoNull && !Search.pos.InCheck() && Search.pos.NullOkay()) {
		Search.pos.NullMove();
		vl = -SearchPV(-vlBeta, 1 - vlBeta, nDepth - NULL_DEPTH - 1, NO_NULL);
		Search.pos.UndoNullMove();

		if (vl >= vlBeta) {
			return vl;
		}
	}
	// 6. ��ʼ����
	mvBest = 0;
	nHashFlag = HASH_ALPHA;
	vlBest = -MATE_VALUE;
	MoveSort.Init(mvHash);
	// 7. ����"MoveSortStruct::Next()"���ŷ�˳����һ������
	while ((mv = MoveSort.Next()) != 0) {
		if (Search.pos.MakeMove(mv)) {
			// 8. ����ѡ�������죻
			nNewDepth = (Search.pos.InCheck() ? nDepth : nDepth - 1);
			// 9. ��Ҫ����������			
			if (vlBest == -MATE_VALUE) {
				vl = -SearchPV(-vlBeta, -vlAlpha, nNewDepth);
			}
			else {
				vl = -SearchPV(-vlAlpha - 1, -vlAlpha, nNewDepth);
				if (vl > vlAlpha && vl < vlBeta) {
					vl = -SearchPV(-vlBeta, -vlAlpha, nNewDepth);
				}
			}
			Search.pos.UndoMakeMove();
			if (Search2.bStop) {
				return vlBest;
			}
			// 10. Alpha-Beta�߽��ж���
			if (vl > vlBest) {
				vlBest = vl;
				if (vl >= vlBeta) {
					mvBest = mv;
					nHashFlag = HASH_BETA;
					break;
				}
				if (vl > vlAlpha) {
					vlAlpha = vl;
					mvBest = mv;
					nHashFlag = HASH_PV;
				}
			}

			nCurrTimer = (int)(GetTime() - Search2.llTime);
			if (nCurrTimer > Search.nMaxTimer) {
				Search2.bStop = true;
			}
		}
	}

	// 11. �����û�����ʷ���ɱ���ŷ���
	if (vlBest == -MATE_VALUE) {
		return Search.pos.nDistance - MATE_VALUE;
	}
	else {
		RecordHash(Search.pos, nHashFlag, vlBest, nDepth, mvBest);
		if (mvBest != 0) {
			SetBestMove(mvBest, nDepth, Search2.wmvKiller[Search.pos.nDistance]);
		}
		return vlBest;
	}
}

// �������������
static int SearchRoot(int nDepth) {
	int nNewDepth, vlBest, vl, mv, nCurrTimer;
	// ������������̰������¼������裺
	// 1. ��ʼ��
	vlBest = -MATE_VALUE;
	Search2.MoveSort.Init(Search2.mvResult);
	// 2. ��һ����ÿ���ŷ�
	while ((mv = Search2.MoveSort.Next()) != 0) {
		if (Search.pos.MakeMove(mv)) {
			// 3. ����ѡ��������(ֻ���ǽ�������)
			nNewDepth = (Search.pos.InCheck() ? nDepth : nDepth - 1);
			// 4. ��Ҫ��������
			if (vlBest == -MATE_VALUE) {
				vl = -SearchPV(-MATE_VALUE, MATE_VALUE, nNewDepth, NO_NULL);
			}
			else {
				vl = -SearchPV(-vlBest - 1, -vlBest, nNewDepth);
				if (vl > vlBest) { // ���ﲻ��Ҫ" && vl < MATE_VALUE"��
					vl = -SearchPV(-MATE_VALUE, -vlBest, nNewDepth, NO_NULL);
				}
			}
			Search.pos.UndoMakeMove();
			if (Search2.bStop) {
				return vlBest;
			}
			// 5. Alpha-Beta�߽��ж�("vlBest"������"SearchPV()"�е�"vlAlpha")
			if (vl > vlBest) {
				// 6. �����������һ�ŷ�����ô"δ�ı�����ŷ�"�ļ�������1����������
				Search2.nUnchanged = (vlBest == -MATE_VALUE ? Search2.nUnchanged + 1 : 0);
				vlBest = vl;
				// 7. ����������ŷ�ʱ��¼��Ҫ����
				Search2.mvResult = mv;
			}

			nCurrTimer = (int)(GetTime() - Search2.llTime);
			if (nCurrTimer > Search.nMaxTimer) {
				Search2.bStop = true;
			}

		}
	}
	RecordHash(Search.pos, HASH_PV, vlBest, nDepth, Search2.mvResult);
	SetBestMove(Search2.mvResult, nDepth, Search2.wmvKiller[Search.pos.nDistance]);
	return vlBest;
}

// ����������
void SearchMain(int nDepth) {
	int i, vl, vlLast;
	int nCurrTimer, nLimitTimer;
	int nBookMoves;
	BookStruct bks[MAX_GEN_MOVES];
	// ���������̰������¼������裺

	// 2. �ӿ��ֿ��������ŷ�
	if (Search.bUseBook) {
		// a. ��ȡ���ֿ��е������߷�
		nBookMoves = GetBookMoves(Search.pos, bks);
		if (nBookMoves > 0) {
			vl = 0;
			for (i = 0; i < nBookMoves; i++) {
				vl += bks[i].wvl;
			}
			// b. ����Ȩ�����ѡ��һ���߷�
			vl = Search.rc4Random.NextLong() % (uint32_t)vl;
			for (i = 0; i < nBookMoves; i++) {
				vl -= bks[i].wvl;
				if (vl < 0) {
					break;
				}
			}
			// c. ������ֿ��е��ŷ�����ѭ�����棬��ô��������ŷ�
			Search.pos.MakeMove(bks[i].wmv);
			if (Search.pos.RepStatus(3) == 0) {
				Search2.mvResult = bks[i].wmv;
				Search.pos.UndoMakeMove();
				uint32_t result = MOVE_COORD(Search2.mvResult);
				printf("bestmove %.4s\n", (const char*)& result);
				fflush(stdout);
				if (Search.bDebug)
					Search.pos.DrawBoard(Search2.mvResult);
				return;
			}
			Search.pos.UndoMakeMove();
		}
	}

	// 3. ������Ϊ���򷵻ؾ�̬����ֵ
	if (nDepth == 0) {
		vl = SearchQuiesc(Search.pos, -MATE_VALUE, MATE_VALUE);
		//		vl = Evaluate(Search.pos);
		if (Search.bDebug) {
			printf("info depth 0 score %d\n", vl);
			fflush(stdout);
		}
		return;
	}

	// 4. ��ʼ��ʱ��ͼ�����
	Search2.bStop = false;
	Search2.nUnchanged = 0;
	Search2.mvResult = 0;
	ClearKiller(Search2.wmvKiller);
	ClearHistory();
	memset(Search2.HashTable, 0, sizeof(Search2.HashTable));
	// ���� ClearHash() ��Ҫ����һ��ʱ�䣬���Լ�ʱ�����Ժ�ʼ�ȽϺ���
	Search2.llTime = GetTime();
	vlLast = 0;
	nCurrTimer = 0;

	// 5. ��������������
	for (i = 1; i <= nDepth; i++) {
		// 6. ���������
		vl = SearchRoot(i);
		if (Search2.bStop) {
			if (vl > -MATE_VALUE) {
				vlLast = vl; // ������vlLast�������ж������Ͷ����������Ҫ�������һ��ֵ
			}
			break; // û����������"vl"�ǿɿ�ֵ
		}
		if (Search.bDebug) {
			printf("info depth %d score %d\n", i, vl);
			fflush(stdout);
		}
		nCurrTimer = (int)(GetTime() - Search2.llTime);
		// 7. �������ʱ�䳬���ʵ�ʱ�ޣ�����ֹ����
		nLimitTimer = Search.nMaxTimer;
		// a. �����ǰ����ֵû�����ǰһ��ܶ࣬��ô�ʵ�ʱ�޼���
		nLimitTimer = (vl + DROPDOWN_VALUE >= vlLast ? nLimitTimer / 2 : nLimitTimer);
		// b. �������ŷ��������û�б仯����ô�ʵ�ʱ�޼���
		nLimitTimer = (Search2.nUnchanged >= UNCHANGED_DEPTH ? nLimitTimer / 2 : nLimitTimer);
		if (nCurrTimer > nLimitTimer) {
			vlLast = vl;
			break; // �����Ƿ�������"vlLast"���Ѹ���
		}

		vlLast = vl;

		// 8. ������ɱ������ֹ����
		if (vlLast > WIN_VALUE || vlLast < -WIN_VALUE) {
			break;
		}

	}

	// 9. �������ŷ�
	uint32_t result = MOVE_COORD(Search2.mvResult);
	printf("bestmove %.4s\n", (const char*)& result);
	fflush(stdout);
	if (Search.bDebug)
		Search.pos.DrawBoard(Search2.mvResult);
}
