#include <cstring>
#include <algorithm>
#include "position.h"
#ifndef MOVESORT_H
#define MOVESORT_H

const int LIMIT_DEPTH = 64;       // �����ļ������
const int SORT_VALUE_MAX = 65535; // �ŷ��������ֵ

extern int* nHistory; // ��ʷ��

// �߷�����׶�
const int PHASE_HASH = 0;
const int PHASE_KILLER_1 = 1;
const int PHASE_KILLER_2 = 2;
const int PHASE_GEN_MOVES = 3;
const int PHASE_REST = 4;

const bool NEXT_ALL = true;    // �ŷ�˳����"MoveSortStruct::NextQuiesc()"ѡ��
const bool ROOT_UNIQUE = true; // �ŷ�˳����"MoveSortStruct::ResetRoot()"ѡ��
// MVV/LVAÿ�������ļ�ֵ
static int cucMvvLva[24] = {
  0, 0, 0, 0, 0, 0, 0, 0,
  5, 1, 1, 3, 4, 3, 2, 0,
  5, 1, 1, 3, 4, 3, 2, 0
};
extern PositionStruct& Pos;
extern uint16_t(*wmvKiller)[2];

// ��MVV/LVAֵ
inline int MvvLva(int mv) {
	return (cucMvvLva[Pos.ucpcSquares[DST(mv)]] << 3) - cucMvvLva[Pos.ucpcSquares[SRC(mv)]];
}

// "sort"��MVV/LVAֵ����ıȽϺ���
bool CompareMvvLva(const int lpmv1, const int lpmv2) {
	return MvvLva(lpmv1) > MvvLva(lpmv2);
}

// "sort"����ʷ������ıȽϺ���
bool CompareHistory(const int lpmv1, const int lpmv2) {
	return nHistory[lpmv1] > nHistory[lpmv2];
}

// �ŷ����нṹ
struct MoveSortStruct {
	int mvHash, mvKiller1, mvKiller2; // �û����߷�������ɱ���߷�
	int nPhase, nIndex, nGenMoves;    // ��ǰ�׶Σ���ǰ���õڼ����߷����ܹ��м����߷�
	int mvs[MAX_GEN_MOVES];           // ���е��߷�
	void Init(int mvHash_) { // ��ʼ�����趨�û����߷�������ɱ���߷�
		mvHash = mvHash_;
		mvKiller1 = wmvKiller[Pos.nDistance][0];
		mvKiller2 = wmvKiller[Pos.nDistance][1];
		nPhase = PHASE_HASH;
	}
	int Next(void); // �õ���һ���߷�
};

// �����ʷ��
inline void ClearHistory(void) {
	memset(nHistory, 0, sizeof(int[65536]));
}

// ���ɱ���ŷ���
inline void ClearKiller(uint16_t(*lpwmvKiller)[2]) {
	memset(lpwmvKiller, 0, LIMIT_DEPTH * sizeof(uint16_t[2]));
}

// �õ���һ���߷�
int MoveSortStruct::Next(void) {
	int mv;
	switch (nPhase) {
		// "nPhase"��ʾ�ŷ����������ɽ׶Σ�����Ϊ��
		// 0. �û����ŷ���������ɺ�����������һ�׶Σ�
		case PHASE_HASH:
			nPhase = PHASE_KILLER_1;
			if (mvHash != 0) {
				return mvHash;
			}
			// ���ɣ�����û��"break"����ʾ"switch"����һ��"case"ִ��������������һ��"case"����ͬ
		  // 1. ɱ���ŷ�����(��һ��ɱ���ŷ�)����ɺ�����������һ�׶Σ�
		case PHASE_KILLER_1:
			nPhase = PHASE_KILLER_2;
			if (mvKiller1 != mvHash && mvKiller1 != 0 && Pos.LegalMove(mvKiller1)) {
				return mvKiller1;
			}
			// 2. ɱ���ŷ�����(�ڶ���ɱ���ŷ�)����ɺ�����������һ�׶Σ�
		case PHASE_KILLER_2:
			nPhase = PHASE_GEN_MOVES;
			if (mvKiller2 != mvHash && mvKiller2 != 0 && Pos.LegalMove(mvKiller2)) {
				return mvKiller2;
			}
			// 3. ���������ŷ�����ɺ�����������һ�׶Σ�
		case PHASE_GEN_MOVES:
			nPhase = PHASE_REST;
			nGenMoves = Pos.GenMoves(mvs);
			std::sort(mvs, mvs + nGenMoves, CompareHistory);
			nIndex = 0;
			// 4. ��ʣ���ŷ�����ʷ��������
		case PHASE_REST:
			while (nIndex < nGenMoves) {
				mv = mvs[nIndex];
				nIndex++;
				if (mv != mvHash && mv != mvKiller1 && mv != mvKiller2) {
					return mv;
				}
			}
			// 5. û���ŷ��ˣ������㡣
		default:
			return 0;
	}
}

// ������߷��Ĵ���
inline void SetBestMove(int mv, int nDepth, uint16_t* lpwmvKiller) {
	nHistory[mv] += nDepth * nDepth;
	if (lpwmvKiller[0] != mv) {
		lpwmvKiller[1] = lpwmvKiller[0];
		lpwmvKiller[0] = mv;
	}
}

#endif
