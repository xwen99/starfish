#include "base.h"

#ifndef UCCI_H
#define UCCI_H

const int UCCI_MAX_DEPTH = 32; // UCCI����˼���ļ������

// UCCIָ������
enum UcciCommEnum {
	UCCI_COMM_UNKNOWN, UCCI_COMM_UCCI, UCCI_COMM_ISREADY,
	UCCI_COMM_POSITION, UCCI_COMM_GO, UCCI_COMM_QUIT
};

// UCCIָ����Խ��ͳ������������Ľṹ
union UcciCommStruct {
	/* 2. "position"ָ��ݵ���Ϣ���ʺ���"e_CommPosition"ָ������
	 *    "position"ָ���������þ��棬������ʼ������ͬ�����ŷ����ɵľ���
	 *    ���磬position startpos moves h2e2 h9g8��FEN������"startpos"�����FEN�����ŷ���(MoveNum)����2
	 */
	struct {
		const char* szFenStr;     // FEN�����������(��"startpos"��)Ҳ�ɽ���������ת����FEN��
		int nMoveNum;             // �����ŷ���
		uint32_t* lpdwMovesCoord; // �����ŷ���ָ�����"IdleLine()"�е�һ����̬���飬�����԰�"CoordList"����������
	};
	/* 4. "go"ָ��ݵ���Ϣ���ʺ���"UCCI_COMM_GOָ������ */
	struct {
		int nTime;
	};
};

// ��������������������UCCIָ��������ڲ�ͬ����
UcciCommEnum BootLine(void);                                  // UCCI���������ĵ�һ��ָ�ֻ����"ucci"
UcciCommEnum IdleLine(UcciCommStruct& UcciComm, bool bDebug); // �������ʱ����ָ��
UcciCommEnum BusyLine(UcciCommStruct& UcciComm, bool bDebug); // ����˼��ʱ����ָ�ֻ�������"stop"��"ponderhit"��"probe"

#endif
