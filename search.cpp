#include <cstdio>
#include "base2.h"
#include "position.h"
#include "hash.h"
#include "ucci.h"
#include "book.h"
#include "movesort.h"
#include "search.h"
#include <time.h>

const int IID_DEPTH = 2;         // 内部迭代加深的深度
const int SMP_DEPTH = 6;         // 并行搜索的深度
const int UNCHANGED_DEPTH = 4;   // 未改变最佳着法的深度

const int DROPDOWN_VALUE = 20;   // 落后的分值
const int RESIGN_VALUE = 300;    // 认输的分值
const int DRAW_OFFER_VALUE = 40; // 提和的分值

SearchStruct Search;
PositionStruct &Pos = Search.pos;
// 搜索信息，是封装在模块内部的
static struct {
	int64_t llTime;                     // 计时器
	uint32_t mvResult;					// 走棋结果
	bool bStop;				            // 中止信号
	int nUnchanged;                     // 未改变最佳着法的深度
	uint16_t wmvKiller[LIMIT_DEPTH][2]; // 杀手着法表
	HashStruct HashTable[HASH_SIZE];	// 置换表
	int nHistoryTable[65536];			// 历史表
	MoveSortStruct MoveSort;            // 根结点的着法序列
} Search2;
HashStruct* hshItems = Search2.HashTable;
uint16_t (*wmvKiller)[2] = Search2.wmvKiller;
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
			// 始终让pos.nMoveNum反映没吃子的步数
			pos.SetIrrev();
		}
	}
}

// 中断例程
static bool Interrupt(void) {
	if ((int)(GetTime() - Search2.llTime) > Search.nMaxTimer) {
		Search2.bStop = true;
		return true;
	}

	UcciCommStruct UcciComm;
	// 先调用UCCI解释程序，再判断是否中止
	switch (BusyLine(UcciComm, Search.bDebug)) {
		case UCCI_COMM_ISREADY:
			// "isready"指令实际上没有意义
			printf("readyok\n");
			fflush(stdout);
			return false;
		case UCCI_COMM_QUIT:
			// "quit"指令发送退出信号
			Search.bQuit = Search2.bStop = true;
			return true;
		default:
			return false;
	}
}

// 无害裁剪
static int HarmlessPruning(const PositionStruct& pos, int vlBeta) {
	int vl, vlRep;

	// 1. 杀棋步数裁剪；
	vl = pos.nDistance - MATE_VALUE;
	if (vl >= vlBeta) {
		return vl;
	}

	// 2. 重复裁剪；
	vlRep = pos.RepStatus();
	if (vlRep > 0) {
		return pos.RepValue(vlRep);
	}

	return -MATE_VALUE;
}

// 调整型局面评价函数
inline int Evaluate(const PositionStruct& pos) {
	int vl;
	vl = pos.Evaluate();
	return vl == pos.DrawValue() ? vl - 1 : vl;
}

// 静态搜索例程
static int SearchQuiesc(PositionStruct& pos, int vlAlpha, int vlBeta) {
	int vlBest, vl, nGenMoves;
	int mvs[MAX_GEN_MOVES];
	// 静态搜索例程包括以下几个步骤：

	// 1. 无害裁剪；
	vl = HarmlessPruning(pos, vlBeta);
	if (vl > -MATE_VALUE) {
		return vl;
	}

	// 4. 达到极限深度，直接返回评价值；
	if (pos.nDistance == LIMIT_DEPTH) {
		return Evaluate(pos);
	}
	__ASSERT(Search.pos.nDistance < LIMIT_DEPTH);

	// 5. 初始化；
	vlBest = -MATE_VALUE;

	// 6. 对于被将军的局面，生成全部着法；
	if (pos.InCheck()) {
		nGenMoves = pos.GenMoves(mvs);
		std::sort(mvs, mvs + nGenMoves, CompareHistory);
	}
	else {
		// 7. 对于未被将军的局面，在生成着法前首先尝试空着(空着启发)，即对局面作评价；
		vl = Evaluate(pos);
		__ASSERT_BOUND(1 - WIN_VALUE, vl, WIN_VALUE - 1);
		__ASSERT(vl > vlBest);
		if (vl >= vlBeta) {
			return vl;
		}
		vlBest = vl;
		vlAlpha = MAX(vl, vlAlpha);

		// 8. 对于未被将军的局面，生成并排序所有吃子着法(MVV(LVA)启发)；
		nGenMoves = pos.GenMoves(mvs, GEN_CAPTURE);
		std::sort(mvs, mvs + nGenMoves, CompareMvvLva);
	}

	// 9. 用Alpha-Beta算法搜索这些着法；
	for (int i = 0; i < nGenMoves; i++) {
		__ASSERT(pos.InCheck() || pos.ucpcSquares[DST(mvs[i])] > 0);
		if (pos.MakeMove(mvs[i])) {
			vl = -SearchQuiesc(pos, -vlBeta, -vlAlpha);
			pos.UndoMakeMove();
			if (vl > vlBest) {
				if (vl >= vlBeta) {
					return vl;
				}
				vlBest = vl;
				vlAlpha = MAX(vl, vlAlpha);
			}
		}
	}

	// 10. 返回分值。
	if (vlBest == -MATE_VALUE) {
		__ASSERT(pos.InCheck());
		return pos.nDistance - MATE_VALUE;
	}
	else {
		return vlBest;
	}
}

const bool NO_NULL = false; // "SearchPV()"的参数，是否禁止空着裁剪

// 主变例搜索过程
static int SearchPV(int vlAlpha, int vlBeta, int nDepth, bool bNoNull = false) {
	int nNewDepth, nHashFlag, vlBest, vl;
	int mvBest, mvHash, mv;
	MoveSortStruct MoveSort;
	// 完全搜索例程包括以下几个步骤：

	// 1. 在叶子结点处调用静态搜索；
	
	if (nDepth <= 0) {
		__ASSERT(nDepth >= -NULL_DEPTH);
//		return Evaluate(Search.pos);
		return SearchQuiesc(Search.pos, vlAlpha, vlBeta);
	}

	// 2. 无害裁剪；
	vl = HarmlessPruning(Search.pos, vlBeta);
	if (vl > -MATE_VALUE) {
		return vl;
	}

	// 3. 置换裁剪；
	vl = ProbeHash(Search.pos, vlAlpha, vlBeta, nDepth, mvHash);
	if (Search.bUseHash && vl > -MATE_VALUE) {
		// 由于PV结点不适用置换裁剪，所以不会发生PV路线中断的情况
		return vl;
	}

	// 4. 达到极限深度，直接返回评价值；
	__ASSERT(Search.pos.nDistance > 0);
	if (Search.pos.nDistance == LIMIT_DEPTH) {
		return Evaluate(Search.pos);
	}
	__ASSERT(Search.pos.nDistance < LIMIT_DEPTH);

	// 5. 中断调用
	vlBest = -MATE_VALUE;
	if (Interrupt()) {
		return vlBest;
	}

	// 5. 尝试空着裁剪；
	if (bNoNull && !Search.pos.InCheck() && Search.pos.NullOkay()) {
		Search.pos.NullMove();
		vl = -SearchPV(-vlBeta, 1 - vlBeta, nDepth - NULL_DEPTH - 1, NO_NULL);
		Search.pos.UndoNullMove();
		if (Search2.bStop) {
			return vlBest;
		}

		if (vl >= vlBeta) {
			return vl;
		}
	}

	// 6. 初始化；
	mvBest = 0;
	nHashFlag = HASH_ALPHA;
	vlBest = -MATE_VALUE;
	MoveSort.Init(mvHash);

	// 7. 按照"MoveSortStruct::Next()"的着法顺序逐一搜索；
	while ((mv = MoveSort.Next()) != 0) {
		if (Search.pos.MakeMove(mv)) {

			// 8. 尝试选择性延伸；
			nNewDepth = (Search.pos.InCheck() ? nDepth : nDepth - 1);

			// 9. 主要变例搜索；
			
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

			// 10. Alpha-Beta边界判定；
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
		}
	}

	// 11. 更新置换表、历史表和杀手着法表。
	if (vlBest == -MATE_VALUE) {
		__ASSERT(Search.pos.InCheck());
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

// 根结点搜索例程
static int SearchRoot(int nDepth) {
	int nNewDepth, vlBest, vl, mv;
	// 根结点搜索例程包括以下几个步骤：

	// 1. 初始化
	vlBest = -MATE_VALUE;
	Search2.MoveSort.Init(Search2.mvResult);

	// 2. 逐一搜索每个着法
	while ((mv = Search2.MoveSort.Next()) != 0) {
		if (Search.pos.MakeMove(mv)) {

			// 3. 尝试选择性延伸(只考虑将军延伸)
			nNewDepth = (Search.pos.InCheck() ? nDepth : nDepth - 1);

			// 4. 主要变例搜索
			if (vlBest == -MATE_VALUE) {
				vl = -SearchPV(-MATE_VALUE, MATE_VALUE, nNewDepth, NO_NULL);
				__ASSERT(vl > vlBest);
			}
			else {
				vl = -SearchPV(-vlBest - 1, -vlBest, nNewDepth);
				if (vl > vlBest) { // 这里不需要" && vl < MATE_VALUE"了
					vl = -SearchPV(-MATE_VALUE, -vlBest, nNewDepth, NO_NULL);
				}
			}
			Search.pos.UndoMakeMove();
			if (Search2.bStop) {
				return vlBest;
			}

			// 5. Alpha-Beta边界判定("vlBest"代替了"SearchPV()"中的"vlAlpha")
			if (vl > vlBest) {

				// 6. 如果搜索到第一着法，那么"未改变最佳着法"的计数器加1，否则清零
				Search2.nUnchanged = (vlBest == -MATE_VALUE ? Search2.nUnchanged + 1 : 0);
				vlBest = vl;

				// 7. 搜索到最佳着法时记录主要变例
				Search2.mvResult = mv;
			}
		}
	}
	RecordHash(Search.pos, HASH_PV, vlBest, nDepth, Search2.mvResult);
	SetBestMove(Search2.mvResult, nDepth, Search2.wmvKiller[Search.pos.nDistance]);
	return vlBest;
}

// 主搜索例程
void SearchMain(int nDepth) {
	int i, vl, vlLast;
	int nCurrTimer, nLimitTimer;
	int nBookMoves;
	BookStruct bks[MAX_GEN_MOVES];
	// 主搜索例程包括以下几个步骤：

	// 2. 从开局库中搜索着法
	if (Search.bUseBook) {
		// a. 获取开局库中的所有走法
		nBookMoves = GetBookMoves(Search.pos, Search.szBookFile, bks);
		if (nBookMoves > 0) {
			vl = 0;
			for (i = 0; i < nBookMoves; i++) {
				vl += bks[i].wvl;
			}
			// b. 根据权重随机选择一个走法
			vl = Search.rc4Random.NextLong() % (uint32_t)vl;
			for (i = 0; i < nBookMoves; i++) {
				vl -= bks[i].wvl;
				if (vl < 0) {
					break;
				}
			}
			__ASSERT(vl < 0);
			__ASSERT(i < nBookMoves);
			// c. 如果开局库中的着法构成循环局面，那么不走这个着法
			Search.pos.MakeMove(bks[i].wmv);
			if (Search.pos.RepStatus(3) == 0) {
				Search2.mvResult = MOVE_COORD(bks[i].wmv);
				Search.pos.UndoMakeMove();
				return;
			}
			Search.pos.UndoMakeMove();
		}
	}

	// 3. 如果深度为零则返回静态搜索值
	if (nDepth == 0 && Search.bDebug) {
		printf("info depth 0 score %d\n", SearchQuiesc(Search.pos, -MATE_VALUE, MATE_VALUE));
		fflush(stdout);
		return;
	}

	// 5. 初始化时间和计数器
	Search2.bStop  = false;
	Search2.nUnchanged = 0;
	Search2.mvResult = 0;
	ClearKiller(Search2.wmvKiller);
	ClearHistory();
	memset(Search2.HashTable, 0, sizeof(Search2.HashTable));
	// 由于 ClearHash() 需要消耗一定时间，所以计时从这以后开始比较合理
	Search2.llTime = GetTime();
	vlLast = 0;
	nCurrTimer = 0;

	// 6. 做迭代加深搜索
	for (i = 1; i <= nDepth; i++) {
		// 8. 搜索根结点
		vl = SearchRoot(i);
		if (Search2.bStop) {
			if (vl > -MATE_VALUE) {
				vlLast = vl; // 跳出后，vlLast会用来判断认输或投降，所以需要给定最近一个值
			}
			break; // 没有跳出，则"vl"是可靠值
		}
		if (Search.bDebug) {
			printf("info depth %d score %d\n", i, vl);
			fflush(stdout);
		}
		nCurrTimer = (int)(GetTime() - Search2.llTime);
		// 9. 如果搜索时间超过适当时限，则终止搜索
		nLimitTimer = Search.nProperTimer;
		// a. 如果当前搜索值没有落后前一层很多，那么适当时限减半
		nLimitTimer = (vl + DROPDOWN_VALUE >= vlLast ? nLimitTimer / 2 : nLimitTimer);
		// b. 如果最佳着法连续多层没有变化，那么适当时限减半
		nLimitTimer = (Search2.nUnchanged >= UNCHANGED_DEPTH ? nLimitTimer / 2 : nLimitTimer);
		if (nCurrTimer > nLimitTimer) {
			vlLast = vl;
			break; // 不管是否跳出，"vlLast"都已更新
		}

		vlLast = vl;

		// 10. 搜索到杀棋则终止搜索
		if (vlLast > WIN_VALUE || vlLast < -WIN_VALUE) {
			break;
		}

	}

	// 12. 输出最佳着法
	uint32_t result = MOVE_COORD(Search2.mvResult);
	printf("bestmove %.4s\n", (const char*)& result);
	fflush(stdout);
	if (Search.bDebug)
		Search.pos.DrawBoard();
}
