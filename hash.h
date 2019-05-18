#include <cstring>
#include "position.h"

#ifndef HASH_H
#define HASH_H

// 置换表标志，只用在"RecordHash()"函数中
const int HASH_SIZE = 1 << 20; // 置换表大小
const int HASH_ALPHA = 1;      // ALPHA节点的置换表项
const int HASH_BETA = 2;       // BETA节点的置换表项
const int HASH_PV = 3;         // PV节点的置换表项

const int NULL_DEPTH = 2;    // 空着裁剪的深度

// 置换表结构，置换表信息夹在两个Zobrist校验锁中间，可以防止存取冲突
struct HashStruct {
	uint8_t ucDepth, ucFlag;	// 深度，标志位
	int16_t svl;				// 分值
	uint16_t wmv, wReserved;	// 最佳着法，内存填充
	uint32_t dwLock0, dwLock1;	// Zobrist校验锁
}; // hsh
extern HashStruct* hshItems;
// 提取置换表项
static int ProbeHash(const PositionStruct& pos, int vlAlpha, int vlBeta, int nDepth, int& mv) {
	BOOL bMate; // 杀棋标志：如果是杀棋，那么不需要满足深度条件
	HashStruct hsh;

	hsh = hshItems[pos.zobr.dwKey & (HASH_SIZE - 1)];
	if (hsh.dwLock0 != pos.zobr.dwLock0 || hsh.dwLock1 != pos.zobr.dwLock1) {
		mv = 0;
		return -MATE_VALUE;
	}
	mv = hsh.wmv;
	bMate = FALSE;
	if (hsh.svl > WIN_VALUE) {
		if (hsh.svl < BAN_VALUE) {
			return -MATE_VALUE; // 可能导致搜索的不稳定性，立刻退出，但最佳着法可能拿到
		}
		hsh.svl -= pos.nDistance;
		bMate = TRUE;
	}
	else if (hsh.svl < -WIN_VALUE) {
		if (hsh.svl > -BAN_VALUE) {
			return -MATE_VALUE; // 同上
		}
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

// 保存置换表项
static void RecordHash(const PositionStruct& pos, int nFlag, int vl, int nDepth, int mv) {
	HashStruct hsh;
	hsh = hshItems[pos.zobr.dwKey & (HASH_SIZE - 1)];
	if (hsh.ucDepth > nDepth) {
		return;
	}
	hsh.ucFlag = nFlag;
	hsh.ucDepth = nDepth;
	if (vl > WIN_VALUE) {
		if (mv == 0 && vl <= BAN_VALUE) {
			return; // 可能导致搜索的不稳定性，并且没有最佳着法，立刻退出
		}
		hsh.svl = vl + pos.nDistance;
	}
	else if (vl < -WIN_VALUE) {
		if (mv == 0 && vl >= -BAN_VALUE) {
			return; // 同上
		}
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