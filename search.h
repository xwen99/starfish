#include "base.h"
#include "ucci.h"
#include "position.h"
#ifndef SEARCH_H
#define SEARCH_H

// 搜索前可设置的全局变量，指定搜索参数
struct SearchStruct {
	PositionStruct pos;                // 有待搜索的局面
	bool bQuit;						   // 是否收到退出指令
	bool bDebug;					   // 是否调试模式
	bool bUseHash, bUseBook;           // 是否使用置换表裁剪和开局库
	RC4Struct rc4Random;               // 随机数
	int nProperTimer, nMaxTimer;       // 计划使用时间
	char szBookFile[1024];             // 开局库
};

extern SearchStruct Search;

// UCCI局面构造过程
void BuildPos(PositionStruct& pos, const UcciCommStruct& UcciComm);

// UCCI支持 - 输出叶子结点的局面信息
void PopLeaf(PositionStruct& pos);

// 搜索的启动过程
void SearchMain(int nDepth);
//void SearchMain(void);

#endif
