#include <cstring>
#include "base.h"
#include "rc4prng.h"

#ifndef POSITION_H
#define POSITION_H

const int MAX_GEN_MOVES = 128; // ���������߷���
const int MAX_MOVES = 256;     // ������ʷ�߷���
const int MATE_VALUE = 10000;           // ��߷�ֵ���������ķ�ֵ
const int BAN_VALUE = MATE_VALUE - 100; // �����и��ķ�ֵ�����ڸ�ֵ����д���û���(����"hash.cpp")
const int WIN_VALUE = MATE_VALUE - 200; // ������ʤ���ķ�ֵ���ޣ�������ֵ��˵���Ѿ�������ɱ����
const int NULLOKAY_MARGIN = 200;        // ���Ųü����Բ������������ֵ�߽�
const int NULLSAFE_MARGIN = 400;        // ����ʹ�ÿ��Ųü���������������ֵ�߽�
const int DRAW_VALUE = 20;              // ����ʱ���صķ���(ȡ��ֵ)

// ÿ�����������ͱ��
const int KING_TYPE = 0;
const int ADVISOR_TYPE = 1;
const int BISHOP_TYPE = 2;
const int KNIGHT_TYPE = 3;
const int ROOK_TYPE = 4;
const int CANNON_TYPE = 5;
const int PAWN_TYPE = 6;

// ÿ�������Ŀ�ʼ��źͽ������
const int KING_FROM = 0;
const int ADVISOR_FROM = 1;
const int ADVISOR_TO = 2;
const int BISHOP_FROM = 3;
const int BISHOP_TO = 4;
const int KNIGHT_FROM = 5;
const int KNIGHT_TO = 6;
const int ROOK_FROM = 7;
const int ROOK_TO = 8;
const int CANNON_FROM = 9;
const int CANNON_TO = 10;
const int PAWN_FROM = 11;
const int PAWN_TO = 15;

// ���̷�Χ
const int RANK_TOP = 3;
const int RANK_BOTTOM = 12;
const int FILE_LEFT = 3;
const int FILE_RIGHT = 11;

const int ADVANCED_VALUE = 3;  // ����Ȩ��ֵ
const int NULL_MARGIN = 400;   // �ղ��ü��������߽�

extern const bool ccInBoard[256];    // ���������
extern const bool ccInFort[256];     // �ǳ������
extern const int8_t ccLegalSpan[512];   // �����ŷ���ȱ�
extern const int8_t ccKnightPin[512];   // ���ȱ�
extern const uint8_t cucvlPiecePos[7][256];	// ����λ�ü�ֵ��
extern const int cnPieceTypes[48];	// ������Ŷ�Ӧ����������

// "GenMoves"����
const bool GEN_CAPTURE = true;

// ��ʼ�����FEN��
const char* const cszStartFen = "rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RNBAKABNR w";

// �������Ͷ�Ӧ�����ӷ���
const char* const cszPieceBytes = "KABNRCP";

// �������Ͷ�Ӧ���������ӷ��ű�ʶ
const char* const cszPieceBytesInChineseBlack[7] = { "��","ʿ","��","�a","܇","�h","��" };
const char* const cszPieceBytesInChineseRed[7] = { "˧","��","��","��","��","��","��" };

// ��������
inline int PIECE_TYPE(int pc) {
	return cnPieceTypes[pc];
}
// �����±�
inline int PIECE_INDEX(int pc) {
	return pc & 15;
}

// �ж������Ƿ���������
inline bool IN_BOARD(int sq) {
	return ccInBoard[sq];
}

// �ж������Ƿ��ھŹ���
inline bool IN_FORT(int sq) {
	return ccInFort[sq];
}

// ��ø��ӵĺ�����
inline int RANK_Y(int sq) {
	return sq >> 4;
}

// ��ø��ӵ�������
inline int FILE_X(int sq) {
	return sq & 15;
}

// ����������ͺ������ø���
inline int COORD_XY(int x, int y) {
	return x + (y << 4);
}

// ��ת����
inline int SQUARE_FLIP(int sq) {
	return 254 - sq;
}

// ������ˮƽ����
inline int FILE_FLIP(int x) {
	return 14 - x;
}

// �����괹ֱ����
inline int RANK_FLIP(int y) {
	return 15 - y;
}

// ����ˮƽ����
inline int MIRROR_SQUARE(int sq) {
	return COORD_XY(FILE_FLIP(FILE_X(sq)), RANK_Y(sq));
}

inline int SQUARE_FORWARD(int sq, int sd) {
	return sq - 16 + (sd << 5);
}

inline int SQUARE_BACKWARD(int sq, int sd) {
	return sq + 16 - (sd << 5);
}

// �߷��Ƿ����˧(��)�Ĳ���
inline bool KING_SPAN(int sqSrc, int sqDst) {
	return ccLegalSpan[sqDst - sqSrc + 256] == 1;
}

// �߷��Ƿ������(ʿ)�Ĳ���
inline bool ADVISOR_SPAN(int sqSrc, int sqDst) {
	return ccLegalSpan[sqDst - sqSrc + 256] == 2;
}

// �߷��Ƿ������(��)�Ĳ���
inline bool BISHOP_SPAN(int sqSrc, int sqDst) {
	return ccLegalSpan[sqDst - sqSrc + 256] == 3;
}

// ��(��)�۵�λ��
inline int BISHOP_PIN(int sqSrc, int sqDst) {
	return (sqSrc + sqDst) >> 1;
}

// ���ȵ�λ��
inline int KNIGHT_PIN(int sqSrc, int sqDst) {
	return sqSrc + ccKnightPin[sqDst - sqSrc + 256];
}

// �Ƿ�δ����
inline bool HOME_HALF(int sq, int sd) {
	return (sq & 0x80) != (sd << 7);
}

// �Ƿ��ѹ���
inline bool AWAY_HALF(int sq, int sd) {
	return (sq & 0x80) == (sd << 7);
}

// �Ƿ��ںӵ�ͬһ��
inline bool SAME_HALF(int sqSrc, int sqDst) {
	return ((sqSrc ^ sqDst) & 0x80) == 0;
}

// �Ƿ���ͬһ��
inline bool SAME_RANK(int sqSrc, int sqDst) {
	return ((sqSrc ^ sqDst) & 0xf0) == 0;
}

// �Ƿ���ͬһ��
inline bool SAME_FILE(int sqSrc, int sqDst) {
	return ((sqSrc ^ sqDst) & 0x0f) == 0;
}

inline int SIDE_TAG(int sd) {
	return 16 + (sd << 4);
}

inline int OPP_SIDE_TAG(int sd) {
	return 32 - (sd << 4);
}

struct ZobristStruct {
	uint32_t dwKey, dwLock0, dwLock1;
	void InitZero(void) {
		dwKey = dwLock0 = dwLock1 = 0;
	}
	void InitRC4(RC4Struct& rc4) {
		dwKey = rc4.NextLong();
		dwLock0 = rc4.NextLong();
		dwLock1 = rc4.NextLong();
	}
	void Xor(const ZobristStruct& zobr) {
		dwKey ^= zobr.dwKey;
		dwLock0 ^= zobr.dwLock0;
		dwLock1 ^= zobr.dwLock1;
	}
	void Xor(const ZobristStruct& zobr1, const ZobristStruct& zobr2) {
		dwKey ^= zobr1.dwKey ^ zobr2.dwKey;
		dwLock0 ^= zobr1.dwLock0 ^ zobr2.dwLock0;
		dwLock1 ^= zobr1.dwLock1 ^ zobr2.dwLock1;
	}
}; // zobr

// Zobrist��
extern struct ZobristTable{
	ZobristStruct Player;
	ZobristStruct Table[14][256];
} Zobrist;

// ��ʼ��Zobrist��
void InitZobrist(void);

// ��ʷ�߷���Ϣ
struct MoveStruct {
	uint16_t wmv;
	uint8_t ucpcCaptured, ucbCheck;
	uint32_t dwKey;

	void Set(int mv, int pcCaptured, bool bCheck, uint32_t dwKey_) {
		wmv = mv;
		ucpcCaptured = pcCaptured;
		ucbCheck = bCheck;
		dwKey = dwKey_;
	}
}; // mvs

// ����߷������
inline int SRC(int mv) {
	return mv & 255;
}

// ����߷����յ�
inline int DST(int mv) {
	return mv >> 8;
}

// ���������յ����߷�
inline int MOVE(int sqSrc, int sqDst) {
	return sqSrc + (sqDst << 8);
}

// �߷�ˮƽ����
inline int MOVE_MIRROR(int mv) {
	return MOVE(MIRROR_SQUARE(SRC(mv)), MIRROR_SQUARE(DST(mv)));
}

inline uint32_t MOVE_COORD(int mv) {      // ���ŷ�ת�����ַ���
	union {
		char c[4];
		uint32_t dw;
	} Ret;
	Ret.c[0] = FILE_X(SRC(mv)) - FILE_LEFT + 'a';
	Ret.c[1] = '9' - RANK_Y(SRC(mv)) + RANK_TOP;
	Ret.c[2] = FILE_X(DST(mv)) - FILE_LEFT + 'a';
	Ret.c[3] = '9' - RANK_Y(DST(mv)) + RANK_TOP;
	// ��������ŷ��ĺ�����
	__ASSERT_BOUND('a', Ret.c[0], 'i');
	__ASSERT_BOUND('0', Ret.c[1], '9');
	__ASSERT_BOUND('a', Ret.c[2], 'i');
	__ASSERT_BOUND('0', Ret.c[3], '9');
	return Ret.dw;
}

inline int COORD_MOVE(uint32_t dwMoveStr) { // ���ַ���ת�����ŷ�
	int sqSrc, sqDst;
	char* lpArgPtr;
	lpArgPtr = (char*)& dwMoveStr;
	sqSrc = COORD_XY(lpArgPtr[0] - 'a' + FILE_LEFT, '9' - lpArgPtr[1] + RANK_TOP);
	sqDst = COORD_XY(lpArgPtr[2] - 'a' + FILE_LEFT, '9' - lpArgPtr[3] + RANK_TOP);
	return (IN_BOARD(sqSrc) && IN_BOARD(sqDst) ? MOVE(sqSrc, sqDst) : 0);
}

// ����ṹ
struct PositionStruct {
	int sdPlayer;                   // �ֵ�˭�ߣ�0=�췽��1=�ڷ�
	uint8_t ucpcSquares[256];		// �����ϵ�����
	uint8_t ucsqPieces[48];			// ÿ�����ӷŵ�λ�ã�0��ʾ����
	int vlWhite, vlBlack;           // �졢��˫����������ֵ
	int nDistance, nMoveNum;        // ������ڵ�Ĳ�������ʷ�߷���
	MoveStruct mvsList[MAX_MOVES];  // ��ʷ�߷���Ϣ�б�
	ZobristStruct zobr;             // Zobrist

	void ClearBoard(void) {         // �������
		sdPlayer = vlWhite = vlBlack = nDistance = 0;
		memset(ucpcSquares, 0, 256);
		memset(ucsqPieces, 0, 48);
		zobr.InitZero();
	}
	void SetIrrev(void) {           // ���(��ʼ��)��ʷ�߷���Ϣ
		mvsList[0].Set(0, 0, Checked(), zobr.dwKey);
		nMoveNum = 1;
	}
	void ChangeSide(void) {         // �������ӷ�
		sdPlayer = 1 - sdPlayer;
		zobr.Xor(Zobrist.Player);
	}
	void AddPiece(int sq, int pc) { // �������Ϸ�һö����
		int pt;
		ucpcSquares[sq] = pc;
		ucsqPieces[pc] = sq;
		pt = PIECE_TYPE(pc);
		// �췽�ӷ֣��ڷ�(ע��"cucvlPiecePos"ȡֵҪ�ߵ�)����
		if (pc < 32) {
			vlWhite += cucvlPiecePos[pt][sq];
			zobr.Xor(Zobrist.Table[pt][sq]);
		}
		else {
			vlBlack += cucvlPiecePos[pt][SQUARE_FLIP(sq)];
			zobr.Xor(Zobrist.Table[pt + 7][sq]);
		}
	}
	void DelPiece(int sq, int pc) { // ������������һö����
		int pt;
		ucpcSquares[sq] = 0;
		ucsqPieces[pc] = 0;
		pt = PIECE_TYPE(pc);
		// �췽���֣��ڷ�(ע��"cucvlPiecePos"ȡֵҪ�ߵ�)�ӷ�
		if (pc < 32) {
			vlWhite -= cucvlPiecePos[pt][sq];
			zobr.Xor(Zobrist.Table[pt][sq]);
		}
		else {
			vlBlack -= cucvlPiecePos[pt][SQUARE_FLIP(sq)];
			zobr.Xor(Zobrist.Table[pt + 7][sq]);
		}
	}
	int Evaluate(void) const {      // �������ۺ���
		return (sdPlayer == 0 ? vlWhite - vlBlack : vlBlack - vlWhite) + ADVANCED_VALUE;
	}
	bool InCheck(void) const {      // �Ƿ񱻽���
		return mvsList[nMoveNum - 1].ucbCheck;
	}
	bool Captured(void) const {     // ��һ���Ƿ����
		return mvsList[nMoveNum - 1].ucpcCaptured != 0;
	}
	int MovePiece(int mv);                      // ��һ���������
	void UndoMovePiece(int mv, int pcCaptured); // ������һ���������
	bool MakeMove(int mv);                      // ��һ����
	void UndoMakeMove(void) {                   // ������һ����
		nDistance--;
		nMoveNum--;
		ChangeSide();
		UndoMovePiece(mvsList[nMoveNum].wmv, mvsList[nMoveNum].ucpcCaptured);
	}
	void NullMove(void) {                       // ��һ���ղ�
		uint32_t dwKey;
		dwKey = zobr.dwKey;
		ChangeSide();
		mvsList[nMoveNum].Set(0, 0, false, dwKey);
		nMoveNum++;
		nDistance++;
	}
	void UndoNullMove(void) {                   // ������һ���ղ�
		nDistance--;
		nMoveNum--;
		ChangeSide();
	}
	// ���������߷������"bCapture"Ϊ"TRUE"��ֻ���ɳ����߷�
	int GenMoves(int* mvs, bool bCapture = false) const;
	bool LegalMove(int mv) const;               // �ж��߷��Ƿ����
	bool Checked(void) const;                   // �ж��Ƿ񱻽���
	bool IsMate(void);                          // �ж��Ƿ�ɱ
	int DrawValue(void) const {                 // �����ֵ
		return (nDistance & 1) == 0 ? -DRAW_VALUE : DRAW_VALUE;
	}
	int RepStatus(int nRecur = 1) const;        // ����ظ�����
	int RepValue(int nRepStatus) const {        // �ظ������ֵ
		int vlReturn;
		vlReturn = ((nRepStatus & 2) == 0 ? 0 : nDistance - BAN_VALUE) +
			((nRepStatus & 4) == 0 ? 0 : BAN_VALUE - nDistance);
		return vlReturn == 0 ? DrawValue() : vlReturn;
	}
	bool NullOkay(void) const {                 // �ж��Ƿ�����ղ��ü�
		return (sdPlayer == 0 ? vlWhite : vlBlack) > NULL_MARGIN;
	}
	// ���洦�����
	void FromFen(const char* szFen); // FEN��ʶ��
	void ToFen(char* szFen) const;   // ����FEN��
	void Mirror(PositionStruct & posMirror) const; // �Ծ��澵��
	void DrawBoard(uint32_t mv = NULL);	// ��ӡ����
};

#endif
