#include "defs.h"

const int PawnIsolatedMg = -5;
const int PawnIsolatedEg = -10;
const int PawnPassed[8] = { 0, 5, 10, 20, 35, 60, 100, 200 };
const int RookOpenFile = 10;
const int RookSemiOpenFile = 5;
const int QueenOpenFile = 5;
const int QueenSemiOpenFile = 3;
const int BishopPair = 30;

int MaterialDraw(const S_BOARD *pos) {
	
    if (!pos->pceNum[wR] && !pos->pceNum[bR] && !pos->pceNum[wQ] && !pos->pceNum[bQ]) {
	  if (!pos->pceNum[bB] && !pos->pceNum[wB]) {
	      if (pos->pceNum[wN] < 3 && pos->pceNum[bN] < 3) {  return TRUE; }
	  } else if (!pos->pceNum[wN] && !pos->pceNum[bN]) {
	     if (abs(pos->pceNum[wB] - pos->pceNum[bB]) < 2) { return TRUE; }
	  } else if ((pos->pceNum[wN] < 3 && !pos->pceNum[wB]) || (pos->pceNum[wB] == 1 && !pos->pceNum[wN])) {
	    if ((pos->pceNum[bN] < 3 && !pos->pceNum[bB]) || (pos->pceNum[bB] == 1 && !pos->pceNum[bN]))  { return TRUE; }
	  }
	} else if (!pos->pceNum[wQ] && !pos->pceNum[bQ]) {
        if (pos->pceNum[wR] == 1 && pos->pceNum[bR] == 1) {
            if ((pos->pceNum[wN] + pos->pceNum[wB]) < 2 && (pos->pceNum[bN] + pos->pceNum[bB]) < 2)	{ return TRUE; }
        } else if (pos->pceNum[wR] == 1 && !pos->pceNum[bR]) {
            if ((pos->pceNum[wN] + pos->pceNum[wB] == 0) && (((pos->pceNum[bN] + pos->pceNum[bB]) == 1) || ((pos->pceNum[bN] + pos->pceNum[bB]) == 2))) { return TRUE; }
        } else if (pos->pceNum[bR] == 1 && !pos->pceNum[wR]) {
            if ((pos->pceNum[bN] + pos->pceNum[bB] == 0) && (((pos->pceNum[wN] + pos->pceNum[wB]) == 1) || ((pos->pceNum[wN] + pos->pceNum[wB]) == 2))) { return TRUE; }
        }
    }
  return FALSE;
}

int mg_pawn_table[64] = {
      0,   0,   0,   0,   0,   0,  0,   0,
     98, 134,  61,  95,  68, 126, 34, -11,
     -6,   7,  26,  31,  65,  56, 25, -20,
    -14,  13,   6,  21,  23,  12, 17, -23,
    -27,  -2,  -5,  12,  17,   6, 10, -25,
    -26,  -4,  -4, -10,   3,   3, 33, -12,
    -35,  -1, -20, -23, -15,  24, 38, -22,
      0,   0,   0,   0,   0,   0,  0,   0,
};

int eg_pawn_table[64] = {
      0,   0,   0,   0,   0,   0,   0,   0,
    178, 173, 158, 134, 147, 132, 165, 187,
     94, 100,  85,  67,  56,  53,  82,  84,
     32,  24,  13,   5,  -2,   4,  17,  17,
     13,   9,  -3,  -7,  -7,  -8,   3,  -1,
      4,   7,  -6,   1,   0,  -5,  -1,  -8,
     13,   8,   8,  10,  13,   0,   2,  -7,
      0,   0,   0,   0,   0,   0,   0,   0,
};

int mg_knight_table[64] = {
    -167, -89, -34, -49,  61, -97, -15, -107,
     -73, -41,  72,  36,  23,  62,   7,  -17,
     -47,  60,  37,  65,  84, 129,  73,   44,
      -9,  17,  19,  53,  37,  69,  18,   22,
     -13,   4,  16,  13,  28,  19,  21,   -8,
     -23,  -9,  12,  10,  19,  17,  25,  -16,
     -29, -53, -12,  -3,  -1,  18, -14,  -19,
    -105, -21, -58, -33, -17, -28, -19,  -23,
};

int eg_knight_table[64] = {
    -58, -38, -13, -28, -31, -27, -63, -99,
    -25,  -8, -25,  -2,  -9, -25, -24, -52,
    -24, -20,  10,   9,  -1,  -9, -19, -41,
    -17,   3,  22,  22,  22,  11,   8, -18,
    -18,  -6,  16,  25,  16,  17,   4, -18,
    -23,  -3,  -1,  15,  10,  -3, -20, -22,
    -42, -20, -10,  -5,  -2, -20, -23, -44,
    -29, -51, -23, -15, -22, -18, -50, -64,
};

int mg_bishop_table[64] = {
    -29,   4, -82, -37, -25, -42,   7,  -8,
    -26,  16, -18, -13,  30,  59,  18, -47,
    -16,  37,  43,  40,  35,  50,  37,  -2,
     -4,   5,  19,  50,  37,  37,   7,  -2,
     -6,  13,  13,  26,  34,  12,  10,   4,
      0,  15,  15,  15,  14,  27,  18,  10,
      4,  15,  16,   0,   7,  21,  33,   1,
    -33,  -3, -14, -21, -13, -12, -39, -21,
};

int eg_bishop_table[64] = {
    -14, -21, -11,  -8, -7,  -9, -17, -24,
     -8,  -4,   7, -12, -3, -13,  -4, -14,
      2,  -8,   0,  -1, -2,   6,   0,   4,
     -3,   9,  12,   9, 14,  10,   3,   2,
     -6,   3,  13,  19,  7,  10,  -3,  -9,
    -12,  -3,   8,  10, 13,   3,  -7, -15,
    -14, -18,  -7,  -1,  4,  -9, -15, -27,
    -23,  -9, -23,  -5, -9, -16,  -5, -17,
};

int mg_rook_table[64] = {
     32,  42,  32,  51, 63,  9,  31,  43,
     27,  32,  58,  62, 80, 67,  26,  44,
     -5,  19,  26,  36, 17, 45,  61,  16,
    -24, -11,   7,  26, 24, 35,  -8, -20,
    -36, -26, -12,  -1,  9, -7,   6, -23,
    -45, -25, -16, -17,  3,  0,  -5, -33,
    -44, -16, -20,  -9, -1, 11,  -6, -71,
    -19, -13,   1,  17, 16,  7, -37, -26,
};

int eg_rook_table[64] = {
    13, 10, 18, 15, 12,  12,   8,   5,
    11, 13, 13, 11, -3,   3,   8,   3,
     7,  7,  7,  5,  4,  -3,  -5,  -3,
     4,  3, 13,  1,  2,   1,  -1,   2,
     3,  5,  8,  4, -5,  -6,  -8, -11,
    -4,  0, -5, -1, -7, -12,  -8, -16,
    -6, -6,  0,  2, -9,  -9, -11,  -3,
    -9,  2,  3, -1, -5, -13,   4, -20,
};

int mg_queen_table[64] = {
    -28,   0,  29,  12,  59,  44,  43,  45,
    -24, -39,  -5,   1, -16,  57,  28,  54,
    -13, -17,   7,   8,  29,  56,  47,  57,
    -27, -27, -16, -16,  -1,  17,  -2,   1,
     -9, -26,  -9, -10,  -2,  -4,   3,  -3,
    -14,   2, -11,  -2,  -5,   2,  14,   5,
    -35,  -8,  11,   2,   8,  15,  -3,   1,
     -1, -18,  -9,  10, -15, -25, -31, -50,
};

int eg_queen_table[64] = {
     -9,  22,  22,  27,  27,  19,  10,  20,
    -17,  20,  32,  41,  58,  25,  30,   0,
    -20,   6,   9,  49,  47,  35,  19,   9,
      3,  22,  24,  45,  57,  40,  57,  36,
    -18,  28,  19,  47,  31,  34,  39,  23,
    -16, -27,  15,   6,   9,  17,  10,   5,
    -22, -23, -30, -16, -16, -23, -36, -32,
    -33, -28, -22, -43,  -5, -32, -20, -41,
};

int mg_king_table[64] = {
    -65,  23,  16, -15, -56, -34,   2,  13,
     29,  -1, -20,  -7,  -8,  -4, -38, -29,
     -9,  24,   2, -16, -20,   6,  22, -22,
    -17, -20, -12, -27, -30, -25, -14, -36,
    -49,  -1, -27, -39, -46, -44, -33, -51,
    -14, -14, -22, -46, -44, -30, -15, -27,
      1,   7,  -8, -64, -43, -16,   9,   8,
    -15,  36,  12, -54,   8, -28,  24,  14,
};

int eg_king_table[64] = {
    -74, -35, -18, -18, -11,  15,   4, -17,
    -12,  17,  14,  17,  17,  38,  23,  11,
     10,  17,  23,  15,  20,  45,  44,  13,
     -8,  22,  24,  27,  26,  33,  26,   3,
    -18,  -4,  21,  24,  27,  23,   9, -11,
    -19,  -3,  11,  21,  23,  16,   7,  -9,
    -27, -11,   4,  13,  14,   4,  -5, -17,
    -53, -34, -21, -11, -28, -14, -24, -43
};

int empty_table[64];

int* mg_pesto_table[6] =
{
    mg_pawn_table,
    mg_knight_table,
    mg_bishop_table,
    mg_rook_table,
    mg_queen_table,
    mg_king_table
};

int* eg_pesto_table[6] =
{
    eg_pawn_table,
    eg_knight_table,
    eg_bishop_table,
    eg_rook_table,
    eg_queen_table,
    eg_king_table
};


int mg_value[6] = {82, 337, 365, 477, 1025,  0};
int eg_value[6] = {94, 281, 297, 512,  936,  0};

int PieceToPieceType[13] = {EMPTY, PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING, PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING};

int gamephaseInc[12] = {0,0,1,1,1,1,2,2,4,4,0,0};
int mg_table[12][64];
int eg_table[12][64];

const int mirror_square[64] =
{
	a1, b1, c1, d1, e1, f1, g1, h1,
	a2, b2, c2, d2, e2, f2, g2, h2,
	a3, b3, c3, d3, e3, f3, g3, h3,
	a4, b4, c4, d4, e4, f4, g4, h4,
	a5, b5, c5, d5, e5, f5, g5, h5,
	a6, b6, c6, d6, e6, f6, g6, h6,
	a7, b7, c7, d7, e7, f7, g7, h7,
	a8, b8, c8, d8, e8, f8, g8, h8
};

void init_tables()
{
    
    for (int sq = 0; sq < 64;sq++){
        for (int PIECE = PAWN; PIECE <= KING; PIECE++){
            mg_table[PIECE][sq] = mg_value[PIECE] + mg_pesto_table[PIECE][sq];
            eg_table[PIECE][sq] = eg_value[PIECE] + eg_pesto_table[PIECE][sq];
            mg_table[PIECE+6][sq] = mg_value[PIECE] + mg_pesto_table[PIECE][mirror_square[sq]];
            eg_table[PIECE+6][sq] = eg_value[PIECE] + eg_pesto_table[PIECE][mirror_square[sq]];
        }
    }
}

int EvalPosition(const S_BOARD *pos)
{
    int mg[2];
    int eg[2];
    int gamePhase = 0;

    mg[WHITE] = 0;
    mg[BLACK] = 0;
    eg[WHITE] = 0;
    eg[BLACK] = 0;

    int sq, piece;

    // White's pieces

    piece = wP;
    U64 white_pawns = pos->bitboards[piece];
    while (white_pawns){
        sq = get_ls1b_index(white_pawns);

        mg[WHITE] += mg_table[piece-1][sq];
        eg[WHITE] += eg_table[piece-1][sq];    
        gamePhase += gamephaseInc[piece-1];

        pop_bit(white_pawns, sq);
    }

    piece = wN;
    U64 white_knights = pos->bitboards[piece];
    while (white_knights){
        sq = get_ls1b_index(white_knights);

        mg[WHITE] += mg_table[piece-1][sq];
        eg[WHITE] += eg_table[piece-1][sq];    
        gamePhase += gamephaseInc[piece-1];

        pop_bit(white_knights, sq);
    }

    piece = wB;
    U64 white_bishops = pos->bitboards[piece];
    while (white_bishops){
        sq = get_ls1b_index(white_bishops);

        mg[WHITE] += mg_table[piece-1][sq];
        eg[WHITE] += eg_table[piece-1][sq];    
        gamePhase += gamephaseInc[piece-1];

        pop_bit(white_bishops, sq);
    }

    piece = wR;
    U64 white_rooks = pos->bitboards[piece];
    while (white_rooks){
        sq = get_ls1b_index(white_rooks);

        mg[WHITE] += mg_table[piece-1][sq];
        eg[WHITE] += eg_table[piece-1][sq];    
        gamePhase += gamephaseInc[piece-1];

        pop_bit(white_rooks, sq);
    }

    piece = wQ;
    U64 white_queens = pos->bitboards[piece];
    while (white_queens){
        sq = get_ls1b_index(white_queens);

        mg[WHITE] += mg_table[piece-1][sq];
        eg[WHITE] += eg_table[piece-1][sq];    
        gamePhase += gamephaseInc[piece-1];

        pop_bit(white_queens, sq);
    }

    int king_square = get_ls1b_index(pos->bitboards[wK]);
    mg[WHITE] += mg_table[wK-1][king_square];
    eg[WHITE] += eg_table[wK-1][king_square];    
    gamePhase += gamephaseInc[wK-1];


    /* Black's pieces */
    piece = bP;
    U64 black_pawns = pos->bitboards[piece];
    while (black_pawns){
        sq = get_ls1b_index(black_pawns);

        mg[BLACK] += mg_table[piece-1][sq];
        eg[BLACK] += eg_table[piece-1][sq];    
        gamePhase += gamephaseInc[piece-1];

        pop_bit(black_pawns, sq);
    }

    piece = bN;
    U64 black_knights = pos->bitboards[piece];
    while (black_knights){
        sq = get_ls1b_index(black_knights);

        mg[BLACK] += mg_table[piece-1][sq];
        eg[BLACK] += eg_table[piece-1][sq];    
        gamePhase += gamephaseInc[piece-1];

        pop_bit(black_knights, sq);
    }

    piece = bB;
    U64 black_bishops = pos->bitboards[piece];
    while (black_bishops){
        sq = get_ls1b_index(black_bishops);

        mg[BLACK] += mg_table[piece-1][sq];
        eg[BLACK] += eg_table[piece-1][sq];    
        gamePhase += gamephaseInc[piece-1];

        pop_bit(black_bishops, sq);
    }

    piece = bR;
    U64 black_rooks = pos->bitboards[piece];
    while (black_rooks){
        sq = get_ls1b_index(black_rooks);

        mg[BLACK] += mg_table[piece-1][sq];
        eg[BLACK] += eg_table[piece-1][sq];    
        gamePhase += gamephaseInc[piece-1];

        pop_bit(black_rooks, sq);
    }

    piece = bQ;
    U64 black_queens = pos->bitboards[piece];
    while (black_queens){
        sq = get_ls1b_index(black_queens);

        mg[BLACK] += mg_table[piece-1][sq];
        eg[BLACK] += eg_table[piece-1][sq];    
        gamePhase += gamephaseInc[piece-1];

        pop_bit(black_queens, sq);
    }

    king_square = get_ls1b_index(pos->bitboards[bK]);
    mg[BLACK] += mg_table[bK-1][king_square];
    eg[BLACK] += eg_table[bK-1][king_square];
    gamePhase += gamephaseInc[bK-1];


    /* Tapered Evaluation */
	int otherSide = pos->side^1;

    int mgScore = mg[pos->side] - mg[otherSide];
    int egScore = eg[pos->side] - eg[otherSide];
    int mgPhase = gamePhase;
    if (mgPhase > 24) mgPhase = 24; /* In case of early promotion */
    int egPhase = 24 - mgPhase;
    return (mgScore * mgPhase + egScore * egPhase) / 24;
}