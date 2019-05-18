#ifndef MOVESORT_H
#define MOVESORT_H

#include <cstring>
#include <algorithm>
#include "base.h"
#include "position.h"

const int LIMIT_DEPTH = 64;       // 搜索的极限深度
const int SORT_VALUE_MAX = 65535; // 着法序列最大值

// "nHistory"只在"movesort.cpp"一个模块中使用
int nHistory[65536]; // 历史表

// 走法排序阶段
const int PHASE_HASH = 0;
const int PHASE_KILLER_1 = 1;
const int PHASE_KILLER_2 = 2;
const int PHASE_GEN_MOVES = 3;
const int PHASE_REST = 4;

const bool NEXT_ALL = true;    // 着法顺序函数"MoveSortStruct::NextQuiesc()"选项
const bool ROOT_UNIQUE = true; // 着法顺序函数"MoveSortStruct::ResetRoot()"选项
// MVV/LVA每种子力的价值
static BYTE cucMvvLva[24] = {
  0, 0, 0, 0, 0, 0, 0, 0,
  5, 1, 1, 3, 4, 3, 2, 0,
  5, 1, 1, 3, 4, 3, 2, 0
};
extern PositionStruct& Pos;
extern uint16_t (*wmvKiller)[2];

// 求MVV/LVA值
inline int MvvLva(int mv) {
	return (cucMvvLva[Pos.ucpcSquares[DST(mv)]] << 3) - cucMvvLva[Pos.ucpcSquares[SRC(mv)]];
}

// "sort"按MVV/LVA值排序的比较函数
bool CompareMvvLva(const int lpmv1, const int lpmv2) {
	return MvvLva(lpmv1) > MvvLva(lpmv2);
}

// "sort"按历史表排序的比较函数
bool CompareHistory(const int lpmv1, const int lpmv2) {
	return nHistory[lpmv1] > nHistory[lpmv2];
}

// 着法序列结构
struct MoveSortStruct {
	int mvHash, mvKiller1, mvKiller2; // 置换表走法和两个杀手走法
	int nPhase, nIndex, nGenMoves;    // 当前阶段，当前采用第几个走法，总共有几个走法
	int mvs[MAX_GEN_MOVES];           // 所有的走法
	void Init(int mvHash_) { // 初始化，设定置换表走法和两个杀手走法
		mvHash = mvHash_;
		mvKiller1 = wmvKiller[Pos.nDistance][0];
		mvKiller2 = wmvKiller[Pos.nDistance][1];
		nPhase = PHASE_HASH;
	}
	int Next(void); // 得到下一个走法
};

// 清空历史表
inline void ClearHistory(void) {
	memset(nHistory, 0, sizeof(int[65536]));
}

// 清空杀手着法表
inline void ClearKiller(uint16_t(*lpwmvKiller)[2]) {
	memset(lpwmvKiller, 0, LIMIT_DEPTH * sizeof(uint16_t[2]));
}

// 得到下一个走法
int MoveSortStruct::Next(void) {
	int mv;
	switch (nPhase) {
		// "nPhase"表示着法启发的若干阶段，依次为：

		// 0. 置换表着法启发，完成后立即进入下一阶段；
		case PHASE_HASH:
			nPhase = PHASE_KILLER_1;
			if (mvHash != 0) {
				return mvHash;
			}
			// 技巧：这里没有"break"，表示"switch"的上一个"case"执行完后紧接着做下一个"case"，下同

		  // 1. 杀手着法启发(第一个杀手着法)，完成后立即进入下一阶段；
		case PHASE_KILLER_1:
			nPhase = PHASE_KILLER_2;
			if (mvKiller1 != mvHash && mvKiller1 != 0 && Pos.LegalMove(mvKiller1)) {
				return mvKiller1;
			}

			// 2. 杀手着法启发(第二个杀手着法)，完成后立即进入下一阶段；
		case PHASE_KILLER_2:
			nPhase = PHASE_GEN_MOVES;
			if (mvKiller2 != mvHash && mvKiller2 != 0 && Pos.LegalMove(mvKiller2)) {
				return mvKiller2;
			}

			// 3. 生成所有着法，完成后立即进入下一阶段；
		case PHASE_GEN_MOVES:
			nPhase = PHASE_REST;
			nGenMoves = Pos.GenMoves(mvs);
			std::sort(mvs, mvs + nGenMoves, CompareHistory);
			nIndex = 0;

			// 4. 对剩余着法做历史表启发；
		case PHASE_REST:
			while (nIndex < nGenMoves) {
				mv = mvs[nIndex];
				nIndex++;
				if (mv != mvHash && mv != mvKiller1 && mv != mvKiller2) {
					return mv;
				}
			}

			// 5. 没有着法了，返回零。
		default:
			return 0;
	}
}

// 对最佳走法的处理
inline void SetBestMove(int mv, int nDepth, uint16_t* lpwmvKiller) {
	nHistory[mv] += SQR(nDepth);
	if (lpwmvKiller[0] != mv) {
		lpwmvKiller[1] = lpwmvKiller[0];
		lpwmvKiller[0] = mv;
	}
}

#endif
