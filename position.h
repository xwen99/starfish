#include <cstring>
#include "base.h"
#include "rc4prng.h"

#ifndef POSITION_H
#define POSITION_H

const int MAX_GEN_MOVES = 128; // 最大的生成走法数
const int MAX_MOVES = 256;     // 最大的历史走法数
const int MATE_VALUE = 10000;           // 最高分值，即将死的分值
const int BAN_VALUE = MATE_VALUE - 100; // 长将判负的分值，低于该值将不写入置换表(参阅"hash.cpp")
const int WIN_VALUE = MATE_VALUE - 200; // 搜索出胜负的分值界限，超出此值就说明已经搜索出杀棋了
const int NULLOKAY_MARGIN = 200;        // 空着裁剪可以不检验的子力价值边界
const int NULLSAFE_MARGIN = 400;        // 允许使用空着裁剪的条件的子力价值边界
const int DRAW_VALUE = 20;              // 和棋时返回的分数(取负值)

// 每种子力的类型编号
const int KING_TYPE = 0;
const int ADVISOR_TYPE = 1;
const int BISHOP_TYPE = 2;
const int KNIGHT_TYPE = 3;
const int ROOK_TYPE = 4;
const int CANNON_TYPE = 5;
const int PAWN_TYPE = 6;

// 每种子力的开始序号和结束序号
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

// 棋盘范围
const int RANK_TOP = 3;
const int RANK_BOTTOM = 12;
const int FILE_LEFT = 3;
const int FILE_RIGHT = 11;

const int ADVANCED_VALUE = 3;  // 先行权分值
const int NULL_MARGIN = 400;   // 空步裁剪的子力边界

extern const bool ccInBoard[256];    // 棋盘区域表
extern const bool ccInFort[256];     // 城池区域表
extern const int8_t ccLegalSpan[512];   // 合理着法跨度表
extern const int8_t ccKnightPin[512];   // 马腿表
extern const uint8_t cucvlPiecePos[7][256];	// 子力位置价值表
extern const int cnPieceTypes[48];	// 棋子序号对应的棋子类型

// "GenMoves"参数
const bool GEN_CAPTURE = true;

// 起始局面的FEN串
const char* const cszStartFen = "rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RNBAKABNR w";

// 棋子类型对应的棋子符号
const char* const cszPieceBytes = "KABNRCP";

// 棋子类型
inline int PIECE_TYPE(int pc) {
	return cnPieceTypes[pc];
}
// 棋子下标
inline int PIECE_INDEX(int pc) {
	return pc & 15;
}

// 判断棋子是否在棋盘中
inline bool IN_BOARD(int sq) {
	return ccInBoard[sq];
}

// 判断棋子是否在九宫中
inline bool IN_FORT(int sq) {
	return ccInFort[sq];
}

// 获得格子的横坐标
inline int RANK_Y(int sq) {
	return sq >> 4;
}

// 获得格子的纵坐标
inline int FILE_X(int sq) {
	return sq & 15;
}

// 根据纵坐标和横坐标获得格子
inline int COORD_XY(int x, int y) {
	return x + (y << 4);
}

// 翻转格子
inline int SQUARE_FLIP(int sq) {
	return 254 - sq;
}

// 纵坐标水平镜像
inline int FILE_FLIP(int x) {
	return 14 - x;
}

// 横坐标垂直镜像
inline int RANK_FLIP(int y) {
	return 15 - y;
}

// 格子水平镜像
inline int MIRROR_SQUARE(int sq) {
	return COORD_XY(FILE_FLIP(FILE_X(sq)), RANK_Y(sq));
}

inline int SQUARE_FORWARD(int sq, int sd) {
	return sq - 16 + (sd << 5);
}

inline int SQUARE_BACKWARD(int sq, int sd) {
	return sq + 16 - (sd << 5);
}

// 走法是否符合帅(将)的步长
inline bool KING_SPAN(int sqSrc, int sqDst) {
	return ccLegalSpan[sqDst - sqSrc + 256] == 1;
}

// 走法是否符合仕(士)的步长
inline bool ADVISOR_SPAN(int sqSrc, int sqDst) {
	return ccLegalSpan[sqDst - sqSrc + 256] == 2;
}

// 走法是否符合相(象)的步长
inline bool BISHOP_SPAN(int sqSrc, int sqDst) {
	return ccLegalSpan[sqDst - sqSrc + 256] == 3;
}

// 相(象)眼的位置
inline int BISHOP_PIN(int sqSrc, int sqDst) {
	return (sqSrc + sqDst) >> 1;
}

// 马腿的位置
inline int KNIGHT_PIN(int sqSrc, int sqDst) {
	return sqSrc + ccKnightPin[sqDst - sqSrc + 256];
}

// 是否未过河
inline bool HOME_HALF(int sq, int sd) {
	return (sq & 0x80) != (sd << 7);
}

// 是否已过河
inline bool AWAY_HALF(int sq, int sd) {
	return (sq & 0x80) == (sd << 7);
}

// 是否在河的同一边
inline bool SAME_HALF(int sqSrc, int sqDst) {
	return ((sqSrc ^ sqDst) & 0x80) == 0;
}

// 是否在同一行
inline bool SAME_RANK(int sqSrc, int sqDst) {
	return ((sqSrc ^ sqDst) & 0xf0) == 0;
}

// 是否在同一列
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

// Zobrist表
static struct {
	ZobristStruct Player;
	ZobristStruct Table[14][256];
} Zobrist;

// 初始化Zobrist表
static void InitZobrist(void) {
	int i, j;
	RC4Struct rc4;

	rc4.InitZero();
	Zobrist.Player.InitRC4(rc4);
	for (i = 0; i < 14; i++) {
		for (j = 0; j < 256; j++) {
			Zobrist.Table[i][j].InitRC4(rc4);
		}
	}
}

// 历史走法信息
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

// 获得走法的起点
inline int SRC(int mv) {
	return mv & 255;
}

// 获得走法的终点
inline int DST(int mv) {
	return mv >> 8;
}

// 根据起点和终点获得走法
inline int MOVE(int sqSrc, int sqDst) {
	return sqSrc + (sqDst << 8);
}

// 走法水平镜像
inline int MOVE_MIRROR(int mv) {
	return MOVE(MIRROR_SQUARE(SRC(mv)), MIRROR_SQUARE(DST(mv)));
}

inline uint32_t MOVE_COORD(int mv) {      // 把着法转换成字符串
	union {
		char c[4];
		uint32_t dw;
	} Ret;
	Ret.c[0] = FILE_X(SRC(mv)) - FILE_LEFT + 'a';
	Ret.c[1] = '9' - RANK_Y(SRC(mv)) + RANK_TOP;
	Ret.c[2] = FILE_X(DST(mv)) - FILE_LEFT + 'a';
	Ret.c[3] = '9' - RANK_Y(DST(mv)) + RANK_TOP;
	// 断言输出着法的合理性
	__ASSERT_BOUND('a', Ret.c[0], 'i');
	__ASSERT_BOUND('0', Ret.c[1], '9');
	__ASSERT_BOUND('a', Ret.c[2], 'i');
	__ASSERT_BOUND('0', Ret.c[3], '9');
	return Ret.dw;
}

inline int COORD_MOVE(uint32_t dwMoveStr) { // 把字符串转换成着法
	int sqSrc, sqDst;
	char* lpArgPtr;
	lpArgPtr = (char*)& dwMoveStr;
	sqSrc = COORD_XY(lpArgPtr[0] - 'a' + FILE_LEFT, '9' - lpArgPtr[1] + RANK_TOP);
	sqDst = COORD_XY(lpArgPtr[2] - 'a' + FILE_LEFT, '9' - lpArgPtr[3] + RANK_TOP);
	return (IN_BOARD(sqSrc) && IN_BOARD(sqDst) ? MOVE(sqSrc, sqDst) : 0);
}

// 局面结构
struct PositionStruct {
	int sdPlayer;                   // 轮到谁走，0=红方，1=黑方
	uint8_t ucpcSquares[256];		// 棋盘上的棋子
	uint8_t ucsqPieces[48];			// 每个棋子放的位置，0表示被吃
	int vlWhite, vlBlack;           // 红、黑双方的子力价值
	int nDistance, nMoveNum;        // 距离根节点的步数，历史走法数
	MoveStruct mvsList[MAX_MOVES];  // 历史走法信息列表
	ZobristStruct zobr;             // Zobrist

	void ClearBoard(void) {         // 清空棋盘
		sdPlayer = vlWhite = vlBlack = nDistance = 0;
		memset(ucpcSquares, 0, 256);
		memset(ucsqPieces, 0, 48);
		zobr.InitZero();
	}
	void SetIrrev(void) {           // 清空(初始化)历史走法信息
		mvsList[0].Set(0, 0, Checked(), zobr.dwKey);
		nMoveNum = 1;
	}
	void ChangeSide(void) {         // 交换走子方
		sdPlayer = 1 - sdPlayer;
		zobr.Xor(Zobrist.Player);
	}
	void AddPiece(int sq, int pc) { // 在棋盘上放一枚棋子
		int pt;
		ucpcSquares[sq] = pc;
		ucsqPieces[pc] = sq;
		pt = PIECE_TYPE(pc);
		// 红方加分，黑方(注意"cucvlPiecePos"取值要颠倒)减分
		if (pc < 32) {
			vlWhite += cucvlPiecePos[pt][sq];
			zobr.Xor(Zobrist.Table[pt][sq]);
		}
		else {
			vlBlack += cucvlPiecePos[pt][SQUARE_FLIP(sq)];
			zobr.Xor(Zobrist.Table[pt + 7][sq]);
		}
	}
	void DelPiece(int sq, int pc) { // 从棋盘上拿走一枚棋子
		int pt;
		ucpcSquares[sq] = 0;
		ucsqPieces[pc] = 0;
		pt = PIECE_TYPE(pc);
		// 红方减分，黑方(注意"cucvlPiecePos"取值要颠倒)加分
		if (pc < 32) {
			vlWhite -= cucvlPiecePos[pt][sq];
			zobr.Xor(Zobrist.Table[pt][sq]);
		}
		else {
			vlBlack -= cucvlPiecePos[pt][SQUARE_FLIP(sq)];
			zobr.Xor(Zobrist.Table[pt + 7][sq]);
		}
	}
	int Evaluate(void) const {      // 局面评价函数
		return (sdPlayer == 0 ? vlWhite - vlBlack : vlBlack - vlWhite) + ADVANCED_VALUE;
	}
	bool InCheck(void) const {      // 是否被将军
		return mvsList[nMoveNum - 1].ucbCheck;
	}
	bool Captured(void) const {     // 上一步是否吃子
		return mvsList[nMoveNum - 1].ucpcCaptured != 0;
	}
	int MovePiece(int mv);                      // 搬一步棋的棋子
	void UndoMovePiece(int mv, int pcCaptured); // 撤消搬一步棋的棋子
	bool MakeMove(int mv);                      // 走一步棋
	void UndoMakeMove(void) {                   // 撤消走一步棋
		nDistance--;
		nMoveNum--;
		ChangeSide();
		UndoMovePiece(mvsList[nMoveNum].wmv, mvsList[nMoveNum].ucpcCaptured);
	}
	void NullMove(void) {                       // 走一步空步
		uint32_t dwKey;
		dwKey = zobr.dwKey;
		ChangeSide();
		mvsList[nMoveNum].Set(0, 0, false, dwKey);
		nMoveNum++;
		nDistance++;
	}
	void UndoNullMove(void) {                   // 撤消走一步空步
		nDistance--;
		nMoveNum--;
		ChangeSide();
	}
	// 生成所有走法，如果"bCapture"为"TRUE"则只生成吃子走法
	int GenMoves(int* mvs, bool bCapture = false) const;
	bool LegalMove(int mv) const;               // 判断走法是否合理
	bool Checked(void) const;                   // 判断是否被将军
	bool IsMate(void);                          // 判断是否被杀
	int DrawValue(void) const {                 // 和棋分值
		return (nDistance & 1) == 0 ? -DRAW_VALUE : DRAW_VALUE;
	}
	int RepStatus(int nRecur = 1) const;        // 检测重复局面
	int RepValue(int nRepStatus) const {        // 重复局面分值
		int vlReturn;
		vlReturn = ((nRepStatus & 2) == 0 ? 0 : nDistance - BAN_VALUE) +
			((nRepStatus & 4) == 0 ? 0 : BAN_VALUE - nDistance);
		return vlReturn == 0 ? DrawValue() : vlReturn;
	}
	bool NullOkay(void) const {                 // 判断是否允许空步裁剪
		return (sdPlayer == 0 ? vlWhite : vlBlack) > NULL_MARGIN;
	}
	// 局面处理过程
	void FromFen(const char* szFen); // FEN串识别
	void ToFen(char* szFen) const;   // 生成FEN串
	void Mirror(PositionStruct & posMirror) const; // 对局面镜像
	void DrawBoard();	// 打印局面
};

#endif
