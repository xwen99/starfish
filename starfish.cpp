#include <cstdio>
#include "parse.h"
#include "ucci.h"
#include "position.h"
#include "hash.h"
#include "search.h"

inline void PrintLn(const char* sz) {
	printf("%s\n", sz);
	fflush(stdout);
}

int main(void) {
	UcciCommStruct UcciComm;
	if (BootLine() != UCCI_COMM_UCCI) {
		return 0;
	}
	InitZobrist();
	Search.pos.FromFen(cszStartFen);
	Search.pos.nDistance = 0;
	Search.bQuit = false;
	Search.bDebug = true;
	Search.bUseHash = true;
	Search.bUseBook = true;
	PrintLn("ucciok");
	// 以下是接收指令和提供对策的循环体
	while (!Search.bQuit) {
		switch (IdleLine(UcciComm, Search.bDebug)) {
			case UCCI_COMM_ISREADY:
				PrintLn("readyok");
				break;
			case UCCI_COMM_POSITION:
				BuildPos(Search.pos, UcciComm);
				Search.pos.nDistance = 0;
				break;
			case UCCI_COMM_GO:
				Search.nMaxTimer = UcciComm.nTime;
				SearchMain(UCCI_MAX_DEPTH);
				break;
			case UCCI_COMM_QUIT:
				Search.bQuit = true;
				break;
			default:
				break;
		}
	}
	PrintLn("bye");
	return 0;
}
