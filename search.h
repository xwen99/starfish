#include "tools.h"
#include "ucci.h"
#include "position.h"
#ifndef SEARCH_H
#define SEARCH_H

// ����ǰ�����õ�ȫ�ֱ�����ָ����������
struct SearchStruct {
	PositionStruct pos;                // �������ľ���
	bool bQuit;						   // �Ƿ��յ��˳�ָ��
	bool bDebug;					   // �Ƿ����ģʽ
	bool bUseHash, bUseBook;           // �Ƿ�ʹ���û���ü��Ϳ��ֿ�
	RC4Struct rc4Random;               // �����
	int nMaxTimer;					   // ���ʹ��ʱ��
	char szBookFile[1024];             // ���ֿ�
};

extern SearchStruct Search;

// UCCI���湹�����
void BuildPos(PositionStruct& pos, const UcciCommStruct& UcciComm);

// ��������������
void SearchMain(int nDepth);
//void SearchMain(void);

#endif
