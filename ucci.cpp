#include <cstdio>
#include <iostream>
#include "parse.h"
#include "ucci.h"  
#pragma comment(lib, "Shlwapi.lib")

/* UCCI指令分析模块由三各UCCI指令解释器组成。
 *
 * 其中第一个解释器"BootLine()"最简单，只用来接收引擎启动后的第一行指令
 * 输入"ucci"时就返回"UCCI_COMM_UCCI"，否则一律返回"UCCI_COMM_UNKNOWN"
 * 前两个解释器都等待是否有输入，如果没有输入则执行待机指令"Idle()"
 * 而第三个解释器("BusyLine()"，只用在引擎思考时)则在没有输入时直接返回"UCCI_COMM_UNKNOWN"
 */
const int MAX_MOVE_NUM = 1024;

static char szFen[LINE_INPUT_MAX_CHAR];
static uint32_t dwCoordList[MAX_MOVE_NUM];

static bool ParsePos(UcciCommStruct& UcciComm, char* lp) {
	int i;
	//输入一个字符串lp，判断是否指定了FEN串
	//StrEqvSkip比较lp和fen，比较时自动忽略大小写，
	//若s1和s2相同则返回1，且lp指针后移strlen("fen")的长度，，否则返回0
	if (StrEqvSkip(lp, "fen ")) {
		strcpy(szFen, lp);
		UcciComm.szFenStr = szFen;//szFenStr为FEN串
	  // 然后判断是否是startpos
	  // 比较lp和startpos，比较时自动忽略大小写，
	  // 若s1和s2相同则返回1，且lp指针后移strlen("fen")的长度，，否则返回0
	  // 简单来说StrEqvSkip识别一个字符串然后指针移到这个字符串后面
	}
	else if (StrEqv(lp, "startpos")) {
		UcciComm.szFenStr = "rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RNBAKABNR w";
	}
	else {
		// 如果两者都不是，就立即返回
		return false;
	}
	// 然后寻找是否指定了后续着法，即是否有"moves"关键字
	UcciComm.nMoveNum = 0;   //nMoveNum为后续着法数目
	if (StrScanSkip(lp, " moves ")) {
		//略过"moves"
		*(lp - strlen(" moves ")) = '\0';
		UcciComm.nMoveNum = MIN((int)(strlen(lp) + 1) / 5, MAX_MOVE_NUM); 	// "moves"后面的每个着法都是1个空格和4个字符
		for (i = 0; i < UcciComm.nMoveNum; i++) {
			dwCoordList[i] = *(uint32_t*)lp; // 4个字符可转换为一个"uint32_t"，存储和处理起来方便
			lp += sizeof(uint32_t) + 1;// lp后移四个字符和一个空格
		}
		UcciComm.lpdwMovesCoord = dwCoordList;
	}
	return true;
}


UcciCommEnum BootLine(void) {
	//该函数用来接受第一条指令，如果是ucci，则返回UCCI_COMM_UCCI
	char szLineStr[LINE_INPUT_MAX_CHAR];
	while (!std::cin.getline(szLineStr, LINE_INPUT_MAX_CHAR)) {
		Idle();
	}
	if (StrEqv(szLineStr, "ucci")) {
		return UCCI_COMM_UCCI;
	}
	else {
		return UCCI_COMM_UNKNOWN;
	}
	//UCCI_COMM_UCCI和UCCI_COMM_UNKNOWN都是ucci指令类型
}

UcciCommEnum IdleLine(UcciCommStruct& UcciComm, bool bDebug) {
	//如果bDebug指令为1，输出当前读到的指令
	char szLineStr[LINE_INPUT_MAX_CHAR];
	char* lp;
	bool bGoTime;

	while (!std::cin.getline(szLineStr, LINE_INPUT_MAX_CHAR)) {
		//输入一个指令
		Idle();
	}
	lp = szLineStr;
	if (bDebug) {
		printf("info idleline [%s]\n", lp);
		fflush(stdout);//清空输入缓冲区
	}
	if (false) {
		// "IdleLine()"是最复杂的UCCI指令解释器，大多数的UCCI指令都由它来解释，包括：
	}
	// 1. "isready"指令
	else if (StrEqv(lp, "isready")) {
		return UCCI_COMM_ISREADY;
	}
	// 2. "position {<special_position> | fen <fen_string>} [moves <move_list>]"指令
	//如果读到position指令，调用ParsePos指令，着法列表保存在UcciComm里
	else if (StrEqvSkip(lp, "position ")) {
		return ParsePos(UcciComm, lp) ? UCCI_COMM_POSITION : UCCI_COMM_UNKNOWN;

	}
	// 3. "go [ponder | draw] <mode>"指令
	else if (StrEqvSkip(lp, "go time ")) {
		bGoTime = true;
		UcciComm.nTime = Str2Digit(lp, 0, 2000000000);
		return UCCI_COMM_GO;
	}
	// 4. "quit"指令
	else if (StrEqv(lp, "quit")) {
		return UCCI_COMM_QUIT;
	}
	// 5. 无法识别的指令
	else {
		return UCCI_COMM_UNKNOWN;
	}
}

UcciCommEnum BusyLine(UcciCommStruct& UcciComm, bool bDebug) {
	char szLineStr[LINE_INPUT_MAX_CHAR];
	char* lp;
	if (std::cin.getline(szLineStr, LINE_INPUT_MAX_CHAR)) {
		if (bDebug) {
			printf("info busyline [%s]\n", szLineStr);
			fflush(stdout);
		}
		// "BusyLine"只能接收"isready"、"quit"这两条指令
		if (false) {
		}
		else if (StrEqv(szLineStr, "isready")) {
			return UCCI_COMM_ISREADY;
		}
		else if (StrEqv(szLineStr, "quit")) {
			return UCCI_COMM_QUIT;
		}
		else {
			lp = szLineStr;
			return UCCI_COMM_UNKNOWN;
		}
	}
	else {
		return UCCI_COMM_UNKNOWN;
	}
}
