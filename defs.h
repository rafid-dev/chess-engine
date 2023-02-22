#ifndef DEFS_H
#define DEFS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define DEBUG
#ifndef DEBUG
#define ASSERT(n)
#else
#define ASSERT(n) \
	do {\
		if(!(n)) { \
			printf("%s ", __DATE__); \
			printf("%s: ", __TIME__); \
			printf("Assertion '%s' failed.\n", #n); \
			printf("File '%s'\n", __FILE__); \
			printf("Line %d\n", __LINE__); \
			getchar(); \
			exit(1); \
		} \
	} while(0);
#endif

typedef unsigned long long U64;

#define NAME "Rice SE"
#define BRD_SQ_NUM 120

#define START_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
#define TRICKY_POSITION "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - "
#define MATEIN3POSITION "2r3k1/p4p2/3Rp2p/1p2P1pK/8/1P4P1/P3Q2P/1q6 b - - 0 1"

#define MAXGAMEMOVES 2028
#define MAXPOSITIONMOVES 256
#define MAXDEPTH 64
#define MAXPLY 64

#define INF_BOUND 30000
#define ISMATE 29000

#define MIN(a,b)(((a) < (b)) ? (a) : (b))
#define MAX(a,b)(((a) > (b)) ? (a) : (b))

enum {EMPTY, wP, wN, wB, wR, wQ, wK, bP, bN, bB, bR, bQ, bK};
enum {FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H, FILE_NONE};
enum {RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8, RANK_NONE};

enum {
    A1 = 21, B1, C1, D1, E1, F1, G1, H1,
    A2 = 31, B2, C2, D2, E2, F2, G2, H2,
    A3 = 41, B3, C3, D3, E3, F3, G3, H3,
    A4 = 51, B4, C4, D4, E4, F4, G4, H4,
    A5 = 61, B5, C5, D5, E5, F5, G5, H5,
    A6 = 71, B6, C6, D6, E6, F6, G6, H6,
    A7 = 81, B7, C7, D7, E7, F7, G7, H7,
    A8 = 91, B8, C8, D8, E8, F8, G8, H8, NO_SQ, OFFBOARD
};

enum {
    a8, b8, c8, d8, e8, f8, g8, h8,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a1, b1, c1, d1, e1, f1, g1, h1, no_sq
};

enum {FALSE, TRUE};
enum {WHITE, BLACK, BOTH};
enum {WKCA = 1, WQCA = 2, BKCA = 4, BQCA = 8};

enum {HFNONE, HFALPHA, HFBETA, HFEXACT};

enum {PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING};

typedef struct{
    int move;
    int score;
}S_MOVE;

typedef struct{
    S_MOVE moves[MAXPOSITIONMOVES];
    int count;
}S_MOVELIST;

typedef struct{
    U64 posKey;
    int move;
    int score;
    int depth;
    int flags;
    int age;
}S_HASHENTRY;

typedef struct{
    S_HASHENTRY *ptable;
    int numEntries;
    int newWrite;
    int overWrite;
    int hit;
    int cut;
    int currentAge;
}S_HASHTABLE;

typedef struct{
    int move;
    int castlePerm;
    int enPas;
    int fiftyMove;
    U64 posKey;
    int capture;
}S_UNDO;

typedef struct {

    int pieces[BRD_SQ_NUM];
    U64 pawns[3];

    int KingSq[2];

    int side;
    int enPas;
    int fiftyMove;

    int ply;
    int hisPly;

    int castlePerm;

    U64 posKey;

    int pceNum[13];
    int bigPiece[2];
    int majPiece[2];
    int minPiece[2];
    int material[2];

    U64 bitboards[13];
    U64 occupancies[3];

    S_UNDO history[MAXGAMEMOVES];

    // piece list
    int pList[13][10];

    // pList[piece][0] = E1....
    int PvArray[MAXDEPTH];

    int searchHistory[13][BRD_SQ_NUM];
    int searchKillers[2][MAXDEPTH];
}S_BOARD;

typedef struct {

    int starttime;
    int stoptime;
    int depth;
    int depthset;
    int timeset;
    int movestogo;
    int infinite;
    long nodes;

    int quit;
    int stopped;

    float fh;
    float fhf;
}S_SEARCHINFO;

typedef struct {
    S_SEARCHINFO *info;
    S_BOARD *originalPosition;
    S_HASHTABLE *ttable;
}S_SEARCH_THREAD_DATA;

typedef struct {
    int eval;
    int excluded;
}S_STACK;

#define encode_move(source, target, piece, promoted, capture, double, enpassant, castling) \
    (source) |          \
    (target << 6) |     \
    (piece << 12) |     \
    (promoted << 16) |  \
    (capture << 20) |   \
    (double << 21) |    \
    (enpassant << 22) | \
    (castling << 23)    \

#define FROMSQ(move) ((move & 0xfc0) >> 6)
#define TOSQ(move) ((move & 0xfc0) >> 6)

// extract source square
#define get_move_source(move) (move & 0x3f)

// extract target square
#define get_move_target(move) ((move & 0xfc0) >> 6)

// extract piece
#define get_move_piece(move) ((move & 0xf000) >> 12)

// extract promoted piece
#define get_move_promoted(move) ((move & 0xf0000) >> 16)

// extract capture flag
#define get_move_capture(move) (move & 0x100000)

// extract double pawn push flag
#define get_move_double(move) (move & 0x200000)

// extract enpassant flag
#define get_move_enpassant(move) (move & 0x400000)

// extract castling flag
#define get_move_castling(move) (move & 0x800000)

#define NOMOVE 0

/*MACROS*/
#define FR2SQ(f,r) ((21 + (f))) + ((r) * 10)
#define SQ64(sq120) (Sq120ToSq64[(sq120)])
#define SQ120(sq64) (Sq64ToSq120[(sq64)])
#define POP(b) PopBit(b)
#define CNT(b) __builtin_popcountll(b)
#define GETBIT(bitboard, square) ((bitboard) & (1ULL << (square)))
#define CLRBIT(bb, sq) ((bb) &= ClearMask[(sq)])
#define SETBIT(bitboard, square) ((bitboard) |= (1ULL << (square)))
#define POPLS1B(bitboard, square) ((bitboard) &= ~(1ULL << (square)))

#define IsBQ(p) (PieceBishopQueen[(p)])
#define IsRQ(p) (PieceRookQueen[(p)])
#define IsKn(p) (PieceKnight[(p)])
#define IsKi(p) (PieceKing[(p)])

#define MIRROR64(sq) (Mirror64[(sq)])
/*GLOBALS*/
extern int PieceToPieceColor[13];
extern char* PieceToPieceString[13];

extern int Sq120ToSq64[BRD_SQ_NUM];
extern int Sq64ToSq120[64];
extern U64 SetMask[64];
extern U64 ClearMask[64];
extern U64 PieceKeys[13][120];
extern U64 SideKey;
extern U64 CastleKeys[16];

extern char PceChar[];
extern char SideChar[];
extern char RankChar[];
extern char FileChar[];

extern int PieceBig[13];
extern int PieceMaj[13];
extern int PieceMin[13];
extern int PieceVal[13];
extern int PieceCol[13];
extern int PiecePawn[13];

extern int FilesBrd[BRD_SQ_NUM];
extern int RanksBrd[BRD_SQ_NUM];

extern int PieceKnight[13];
extern int PieceKing[13];
extern int PieceRookQueen[13];
extern int PieceBishopQueen[13];
extern int PieceSlides[13];

extern int Mirror64[64];

extern U64 FileBBMask[64];
extern U64 RankBBMask[64];

extern U64 BlackPassedMask[64];
extern U64 WhitePassedMask[64];
extern U64 IsolatedMask[64];

extern int LMRTable[MAXDEPTH][MAXDEPTH];
extern int LateMovePruningCounts[2][9];

extern char *square_to_coordinates[64];

extern S_HASHTABLE HashTable[1];

// pawn attacks table [side][square]
extern U64 pawn_attacks[2][64];

// knight attacks table [square]
extern U64 knight_attacks[64];

// king attacks table [square]
extern U64 king_attacks[64];

// bishop attack masks
extern U64 bishop_masks[64];

// rook attack masks
extern U64 rook_masks[64];

// bishop attacks table [square][occupancies]
extern U64 bishop_attacks[64][512];

// rook attacks rable [square][occupancies]
extern U64 rook_attacks[64][4096];

inline U64 GetPieceBB (int piece, const S_BOARD *pos){
    int side = (piece < wP) ? WHITE : BLACK;

    return (pos->bitboards[piece] & pos->occupancies[side]);
}

inline int PieceAtBB(int square, const S_BOARD *pos){
    for (int i = wP; i <= bK; i++){
        if (GETBIT(pos->bitboards[i], square)){
            return i;
        }
    }
    return EMPTY;
}

inline int GetPieceSQ (const S_BOARD *pos, int piece){
    for (int sq = 0; sq < 64; sq++){
        if (PieceAtBB(sq, pos) == piece){
            return sq;
        }
    }
}
/* FUNCTIONS */

//init.c
extern void AllInit();

// bitboards.c
extern int PopBit(U64 *bb);
extern int CountBits(U64 bb);
extern void PrintBitboard(U64 bb);
extern int get_ls1b_index(U64 bitboard);
extern void InitLeapersAttacks();
extern void InitSlidersAttack(int bishop);
extern U64 SetOccupancy(int index, int bits_in_mask, U64 attack_mask);
extern void InitMagicNumbers();
extern U64 GetBishopAttacks(int square, U64 occupancy);
extern U64 GetRookAttacks(int square, U64 occupancy);
extern U64 GetQueenAttacks(int square, U64 occupancy);

//hashkeys.c
extern U64 GeneratePosKey(const S_BOARD *pos);

//board.c
extern void ResetBoard(S_BOARD *pos);
extern void PrintBoard (const S_BOARD *pos);
extern int ParseFen(char* fen, S_BOARD *pos);
extern void UpdateListsMaterial(S_BOARD *pos);
extern int CheckBoard(const S_BOARD *pos);
extern void MirrorBoard(S_BOARD *pos);

// attack.c
extern int SqAttacked(const int sq, const int side, const S_BOARD *pos);
extern int IsSquareAttackedBB(int square, int side, const S_BOARD *pos);

// io.c
extern char *PrMove(const int move);
extern char *PrSq(const int sq);
extern void PrintMoveList(S_MOVELIST *list);
extern int ParseMove(char *ptrChar, S_BOARD *pos);

// movegen.c
extern void GenerateAllMoves(const S_BOARD *pos, S_MOVELIST *move_list);
extern void GenerateAllCaps(const S_BOARD *pos, S_MOVELIST *list);
extern int MoveExists(S_BOARD *pos, const int move);
extern void InitMvvlva();

//validate.c
extern int SqOnBoard(const int sq);
extern int SideValid(const int side);
extern int FileRankValid(const int fr);
extern int PieceValidEmpty(const int pce);
extern int PieceValid(const int pce);
extern int SqIs120(const int sq);
extern int PceValidEmptyOffbrd(const int pce);

// makemove.c
extern int MakeMove(S_BOARD *pos, int move);
extern void TakeMove(S_BOARD *pos, const int move);
extern void MakeNullMove(S_BOARD *pos);
extern void TakeNullMove(S_BOARD *pos);

//perft.c
extern void PerftTest(int depth, S_BOARD *pos);

//search.c
extern void SearchPosition(S_BOARD *pos, S_SEARCHINFO *info, S_HASHTABLE* table);
extern int SearchPosition_Thread(void *data);

//misc.c
extern int GetTimeMs();

// pvtable.c
extern void InitHashTable(S_HASHTABLE *table, const int MB);
extern void StoreHashEntry(S_BOARD *pos, S_HASHTABLE* table, const int move, int score, const int flags, const int depth);
extern int ProbeHashEntry(S_BOARD *pos, S_HASHTABLE* table, int *move, int *score, int *flag, int *tteDepth, int alpha, int beta, int depth);
extern int ProbePvMove(const S_BOARD *pos, const S_HASHTABLE* table);
extern int GetPvLine(const int depth, S_BOARD *pos, const S_HASHTABLE* table);
extern void ClearHashTable(S_HASHTABLE *table);

//evaluate.c
extern void init_tables();
extern int EvalPosition(const S_BOARD *pos);

//uci.c
extern void Uci_Loop();

//random.c
extern unsigned int get_random_U32_number();
extern U64 get_random_U64_number();
extern U64 generate_magic_number();

#endif