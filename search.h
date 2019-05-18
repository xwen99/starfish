#include "base.h"
#include "ucci.h"
#include "position.h"
#ifndef SEARCH_H
#define SEARCH_H

// ����ǰ�����õ�ȫ�ֱ�����ָ����������
struct SearchStruct {
	PositionStruct pos;                // �д������ľ���
	bool bQuit;						   // �Ƿ��յ��˳�ָ��
	bool bDebug;					   // �Ƿ����ģʽ
	bool bUseHash, bUseBook;           // �Ƿ�ʹ���û���ü��Ϳ��ֿ�
	RC4Struct rc4Random;               // �����
	int nProperTimer, nMaxTimer;       // �ƻ�ʹ��ʱ��
	char szBookFile[1024];             // ���ֿ�
};

extern SearchStruct Search;

// UCCI���湹�����
void BuildPos(PositionStruct& pos, const UcciCommStruct& UcciComm);

// UCCI֧�� - ���Ҷ�ӽ��ľ�����Ϣ
void PopLeaf(PositionStruct& pos);

// ��������������
void SearchMain(int nDepth);

#endif
