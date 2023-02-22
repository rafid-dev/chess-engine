#include <time.h>
#include <math.h>
#include "defs.h"

#define RAND_64	((U64)rand() | \
					((U64) rand() << 15) | \
					((U64) rand() << 30) | \
					((U64) rand() << 45) | \
					(((U64)rand() & 0xf) << 60))

int Sq120ToSq64[BRD_SQ_NUM];
int Sq64ToSq120[64];

U64 SetMask[64];
U64 ClearMask[64];

U64 PieceKeys[13][120];
U64 SideKey;
U64 CastleKeys[16];

int FilesBrd[BRD_SQ_NUM];
int RanksBrd[BRD_SQ_NUM];

U64 FileBBMask[64];
U64 RankBBMask[64];

U64 BlackPassedMask[64];
U64 WhitePassedMask[64];
U64 IsolatedMask[64];

int LMRTable[MAXDEPTH][MAXDEPTH];
int LateMovePruningCounts[2][9];

int PieceToPieceColor[13] = {EMPTY, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK};
char* PieceToPieceString[13] = {"EMPTY", "WHITE PAWN", "WHITE KNIGHT", "WHITE BISHOP", "WHITE ROOK", "WHITE QUEEN", "WHITE KING", "BLACK PAWN", "BLACK KNIGHT", "BLACK BISHOP", "BLACK ROOK", "BLACK QUEEN", "BLACK KING"};
char *square_to_coordinates[] = {
    "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
    "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
    "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
    "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
    "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
    "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
    "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
    "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
};

void InitEvalMasks(){
	int sq, tsq, r, f;

	for(sq = 0; sq < 8; ++sq){
		FileBBMask[sq] = 0ULL;
		RankBBMask[sq] = 0ULL;
	}

	for (r = RANK_8; r>=RANK_1; r--){
		for (f = FILE_A; f <= FILE_H; f++){
			sq = r * 8 + f;
			FileBBMask[f] |= (1ULL << sq);
			RankBBMask[r] |= (1ULL << sq);
		}
	}

	for(sq = 0; sq < 64; ++sq){
		WhitePassedMask[sq] = 0ULL;
		BlackPassedMask[sq] = 0ULL;
		IsolatedMask[sq] = 0ULL;
	}

	for(sq = 0; sq < 64; ++sq) {
		tsq = sq + 8;

        while(tsq < 64) {
            WhitePassedMask[sq] |= (1ULL << tsq);
            tsq += 8;
        }

        tsq = sq - 8;
        while(tsq >= 0) {
            BlackPassedMask[sq] |= (1ULL << tsq);
            tsq -= 8;
        }

        if(FilesBrd[SQ120(sq)] > FILE_A) {
            IsolatedMask[sq] |= FileBBMask[FilesBrd[SQ120(sq)] - 1];

            tsq = sq + 7;
            while(tsq < 64) {
                WhitePassedMask[sq] |= (1ULL << tsq);
                tsq += 8;
            }

            tsq = sq - 9;
            while(tsq >= 0) {
                BlackPassedMask[sq] |= (1ULL << tsq);
                tsq -= 8;
            }
        }

        if(FilesBrd[SQ120(sq)] < FILE_H) {
            IsolatedMask[sq] |= FileBBMask[FilesBrd[SQ120(sq)] + 1];

            tsq = sq + 9;
            while(tsq < 64) {
                WhitePassedMask[sq] |= (1ULL << tsq);
                tsq += 8;
            }

            tsq = sq - 7;
            while(tsq >= 0) {
                BlackPassedMask[sq] |= (1ULL << tsq);
                tsq -= 8;
            }
        }
	}
}

void InitFilesRanksBrd(){
    int index = 0;
    int file = FILE_A;
    int rank = RANK_1;
    int sq = A1;
    int sq64 = 0;

    for (index = 0; index < BRD_SQ_NUM; ++index){
        FilesBrd[index] = OFFBOARD;
        RanksBrd[index] = OFFBOARD;
    }

    for (rank = 0; rank <= RANK_8; ++rank){
        for (file = FILE_A; file <= FILE_H; ++file){
            sq = FR2SQ(file, rank);
            FilesBrd[sq] = file;
            RanksBrd[sq] = rank;
        }
    }
}

void InitHashKeys() {

	int index = 0;
	int index2 = 0;
	for(index = 0; index < 13; ++index) {
		for(index2 = 0; index2 < 120; ++index2) {
			PieceKeys[index][index2] = RAND_64;
		}
	}
	SideKey = RAND_64;
	for(index = 0; index < 16; ++index) {
		CastleKeys[index] = RAND_64;
	}

}

void InitBitMasks() {
	int index = 0;

	for(index = 0; index < 64; index++) {
		SetMask[index] = 0ULL;
		ClearMask[index] = 0ULL;
	}

	for(index = 0; index < 64; index++) {
		SetMask[index] |= (1ULL << index);
		ClearMask[index] = ~SetMask[index];
	}
}

void InitSq120To64() {

	int index = 0;
	int file = FILE_A;
	int rank = RANK_1;
	int sq = A1;
	int sq64 = 0;
	for(index = 0; index < BRD_SQ_NUM; ++index) {
		Sq120ToSq64[index] = 65;
	}

	for(index = 0; index < 64; ++index) {
		Sq64ToSq120[index] = 120;
	}

	for(rank = RANK_1; rank <= RANK_8; ++rank) {
		for(file = FILE_A; file <= FILE_H; ++file) {
			sq = FR2SQ(file,rank);
			ASSERT(SqOnBoard(sq));
			Sq64ToSq120[sq64] = sq;
			Sq120ToSq64[sq] = sq64;
			sq64++;
		}
	}
}

void InitSearch()
{
    // Init Late Move Reductions Table
    for (int depth = 1; depth < MAXDEPTH; depth++)
    {
        for (int played = 1; played < MAXDEPTH; played++)
        {
            LMRTable[depth][played] = 0.75 + log(depth) * log(played) / 2.25;
        }
    }

	// Init late move pruning table
	for (int depth = 1; depth < 9; depth++) {
        LateMovePruningCounts[0][depth] = 2.5 + 2 * depth * depth / 4.5;
        LateMovePruningCounts[1][depth] = 4.0 + 4 * depth * depth / 4.5;
    }
}


void AllInit(){
    InitSq120To64();
    InitBitMasks();
    InitHashKeys();
    InitFilesRanksBrd();
	InitEvalMasks();
	InitMvvlva();
	InitSearch();
	init_tables();
	InitLeapersAttacks();
	InitSlidersAttack(0);
	InitSlidersAttack(1);
	printf("Initialized!\n");
}