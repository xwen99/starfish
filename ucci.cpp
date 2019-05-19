#include <cstdio>
#include "base2.h"
#include "parse.h"
#include "pipe.h"
#include "ucci.h"  
#pragma comment(lib, "Shlwapi.lib")

/* UCCIָ�����ģ��������UCCIָ���������ɡ�
 *
 * ���е�һ��������"BootLine()"��򵥣�ֻ������������������ĵ�һ��ָ��
 * ����"ucci"ʱ�ͷ���"UCCI_COMM_UCCI"������һ�ɷ���"UCCI_COMM_UNKNOWN"
 * ǰ�������������ȴ��Ƿ������룬���û��������ִ�д���ָ��"Idle()"
 * ��������������("BusyLine()"��ֻ��������˼��ʱ)����û������ʱֱ�ӷ���"UCCI_COMM_UNKNOWN"
 */
static PipeStruct pipeStd;

const int MAX_MOVE_NUM = 1024;

static char szFen[LINE_INPUT_MAX_CHAR];
static uint32_t dwCoordList[MAX_MOVE_NUM];

static bool ParsePos(UcciCommStruct& UcciComm, char* lp) {
	int i;
	//����һ���ַ���lp���ж��Ƿ�ָ����FEN��
	//StrEqvSkip�Ƚ�lp��fen���Ƚ�ʱ�Զ����Դ�Сд��
	//��s1��s2��ͬ�򷵻�1����lpָ�����strlen("fen")�ĳ��ȣ������򷵻�0
	if (StrEqvSkip(lp, "fen ")) {
		strcpy(szFen, lp);
		UcciComm.szFenStr = szFen;//szFenStrΪFEN��
	  // Ȼ���ж��Ƿ���startpos
	  // �Ƚ�lp��startpos���Ƚ�ʱ�Զ����Դ�Сд��
	  // ��s1��s2��ͬ�򷵻�1����lpָ�����strlen("fen")�ĳ��ȣ������򷵻�0
	  // ����˵StrEqvSkipʶ��һ���ַ���Ȼ��ָ���Ƶ�����ַ�������
	}
	else if (StrEqv(lp, "startpos")) {
		UcciComm.szFenStr = "rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RNBAKABNR w";
	}
	else {
		// ������߶����ǣ�����������
		return false;
	}
	// Ȼ��Ѱ���Ƿ�ָ���˺����ŷ������Ƿ���"moves"�ؼ���
	UcciComm.nMoveNum = 0;   //nMoveNumΪ�����ŷ���Ŀ
	if (StrScanSkip(lp, " moves ")) {
		//�Թ�"moves"
		*(lp - strlen(" moves ")) = '\0';
		UcciComm.nMoveNum = MIN((int)(strlen(lp) + 1) / 5, MAX_MOVE_NUM); 	// "moves"�����ÿ���ŷ�����1���ո��4���ַ�
		for (i = 0; i < UcciComm.nMoveNum; i++) {
			dwCoordList[i] = *(uint32_t*)lp; // 4���ַ���ת��Ϊһ��"uint32_t"���洢�ʹ�����������
			lp += sizeof(uint32_t) + 1;// lp�����ĸ��ַ���һ���ո�
		}
		UcciComm.lpdwMovesCoord = dwCoordList;
	}
	return true;
}


UcciCommEnum BootLine(void) {
	//�ú����������ܵ�һ��ָ������ucci���򷵻�UCCI_COMM_UCCI
	char szLineStr[LINE_INPUT_MAX_CHAR];
	pipeStd.Open();
	while (!pipeStd.LineInput(szLineStr)) {
		Idle();
	}
	if (StrEqv(szLineStr, "ucci")) {
		return UCCI_COMM_UCCI;
	}
	else {
		return UCCI_COMM_UNKNOWN;
	}
	//UCCI_COMM_UCCI��UCCI_COMM_UNKNOWN����ucciָ������
}

UcciCommEnum IdleLine(UcciCommStruct& UcciComm, bool bDebug) {
	//���bDebugָ��Ϊ1�������ǰ������ָ��
	char szLineStr[LINE_INPUT_MAX_CHAR];
	char* lp;
	bool bGoTime;

	while (!pipeStd.LineInput(szLineStr)) {
		//����һ��ָ��
		Idle();
	}
	lp = szLineStr;
	if (bDebug) {
		printf("info idleline [%s]\n", lp);
		fflush(stdout);//������뻺����
	}
	if (false) {
		// "IdleLine()"����ӵ�UCCIָ����������������UCCIָ����������ͣ�������
	}
	// 1. "isready"ָ��
	else if (StrEqv(lp, "isready")) {
		return UCCI_COMM_ISREADY;
	}
	// 2. "position {<special_position> | fen <fen_string>} [moves <move_list>]"ָ��
	//�������positionָ�����ParsePosָ��ŷ��б�����UcciComm��
	else if (StrEqvSkip(lp, "position ")) {
		return ParsePos(UcciComm, lp) ? UCCI_COMM_POSITION : UCCI_COMM_UNKNOWN;

	}
	// 3. "go [ponder | draw] <mode>"ָ��
	else if (StrEqvSkip(lp, "go time ")) {
		bGoTime = true;
		UcciComm.nTime = Str2Digit(lp, 0, 2000000000);
		return UCCI_COMM_GO;
	}
	// 4. "quit"ָ��
	else if (StrEqv(lp, "quit")) {
		return UCCI_COMM_QUIT;
	}
	// 5. �޷�ʶ���ָ��
	else {
		return UCCI_COMM_UNKNOWN;
	}
}

UcciCommEnum BusyLine(UcciCommStruct& UcciComm, bool bDebug) {
	char szLineStr[LINE_INPUT_MAX_CHAR];
	char* lp;
	if (pipeStd.LineInput(szLineStr)) {
		if (bDebug) {
			printf("info busyline [%s]\n", szLineStr);
			fflush(stdout);
		}
		// "BusyLine"ֻ�ܽ���"isready"��"ponderhit"��"stop"������ָ��
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
