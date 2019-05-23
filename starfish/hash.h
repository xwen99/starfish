#include <cstring>
#include "position.h"

#ifndef HASH_H
#define HASH_H

// �û����־��ֻ����"RecordHash()"������
const int HASH_SIZE = 1 << 20; // �û����С
const int HASH_ALPHA = 1;      // ALPHA�ڵ���û�����
const int HASH_BETA = 2;       // BETA�ڵ���û�����
const int HASH_PV = 3;         // PV�ڵ���û�����

const int NULL_DEPTH = 2;    // ���Ųü������

// �û���ṹ���û�����Ϣ��������ZobristУ�����м䣬���Է�ֹ��ȡ��ͻ
struct HashStruct {
	uint8_t ucDepth, ucFlag;	// ��ȣ���־λ
	int16_t svl;				// ��ֵ
	uint16_t wmv, wReserved;	// ����ŷ����ڴ����
	uint32_t dwLock0, dwLock1;	// ZobristУ����
}; // hsh
extern HashStruct* hshItems;
// ��ȡ�û�����
static int ProbeHash(const PositionStruct& pos, int vlAlpha, int vlBeta, int nDepth, int& mv) {
	bool bMate; // ɱ���־�������ɱ�壬��ô����Ҫ�����������
	HashStruct hsh;

	hsh = hshItems[pos.zobr.dwKey & (HASH_SIZE - 1)];
	if (hsh.dwLock0 != pos.zobr.dwLock0 || hsh.dwLock1 != pos.zobr.dwLock1) {
		mv = 0;
		return -MATE_VALUE;
	}
	mv = hsh.wmv;
	bMate = FALSE;
	if (hsh.svl > WIN_VALUE) {
		hsh.svl -= pos.nDistance;
		bMate = TRUE;
	}
	else if (hsh.svl < -WIN_VALUE) {
		hsh.svl += pos.nDistance;
		bMate = TRUE;
	}
	if (hsh.ucDepth >= nDepth || bMate) {
		if (hsh.ucFlag == HASH_BETA) {
			return (hsh.svl >= vlBeta ? hsh.svl : -MATE_VALUE);
		}
		else if (hsh.ucFlag == HASH_ALPHA) {
			return (hsh.svl <= vlAlpha ? hsh.svl : -MATE_VALUE);
		}
		return hsh.svl;
	}
	return -MATE_VALUE;
};

// �����û�����
static void RecordHash(const PositionStruct& pos, int nFlag, int vl, int nDepth, int mv) {
	HashStruct hsh;
	hsh = hshItems[pos.zobr.dwKey & (HASH_SIZE - 1)];
	if (hsh.ucDepth > nDepth) {
		return;
	}
	hsh.ucFlag = nFlag;
	hsh.ucDepth = nDepth;
	if (vl > WIN_VALUE) {
		hsh.svl = vl + pos.nDistance;
	}
	else if (vl < -WIN_VALUE) {
		hsh.svl = vl - pos.nDistance;
	}
	else {
		hsh.svl = vl;
	}
	hsh.wmv = mv;
	hsh.dwLock0 = pos.zobr.dwLock0;
	hsh.dwLock1 = pos.zobr.dwLock1;
	hshItems[pos.zobr.dwKey & (HASH_SIZE - 1)] = hsh;
};
#endif