#include "defs.h"

#define MOVE(f,t,ca,pro,fl) ( (f) | ((t) << 7) | ( (ca) << 14 ) | ( (pro) << 20 ) | (fl))
#define SQOFFBOARD(sq) (FilesBrd[(sq)]==OFFBOARD)

const int LoopSlidePce[8] = {
 wB, wR, wQ, 0, bB, bR, bQ, 0
};

const int LoopNonSlidePce[6] = {
 wN, wK, 0, bN, bK, 0
};

const int LoopSlideIndex[2] = { 0, 4 };
const int LoopNonSlideIndex[2] = { 0, 3 };

const int NumDir[13] = {
 0, 0, 8, 4, 4, 8, 8, 0, 8, 4, 4, 8, 8
};

const int PceDir[13][8] = {
	{ 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0 },
	{ -8, -19,	-21, -12, 8, 19, 21, 12 },
	{ -9, -11, 11, 9, 0, 0, 0, 0 },
	{ -1, -10,	1, 10, 0, 0, 0, 0 },
	{ -1, -10,	1, 10, -9, -11, 11, 9 },
	{ -1, -10,	1, 10, -9, -11, 11, 9 },
	{ 0, 0, 0, 0, 0, 0, 0 },
	{ -8, -19,	-21, -12, 8, 19, 21, 12 },
	{ -9, -11, 11, 9, 0, 0, 0, 0 },
	{ -1, -10,	1, 10, 0, 0, 0, 0 },
	{ -1, -10,	1, 10, -9, -11, 11, 9 },
	{ -1, -10,	1, 10, -9, -11, 11, 9 }
};

const int VictimScore[13] = {0, 100, 200, 300, 400, 500, 600, 100, 200, 300, 400, 500, 600};
static int MvvLvaScores[13][13];

void InitMvvlva(){
	int attacker;
	int victim;

	for (attacker = wP; attacker <= bK; ++attacker){
		for (victim = wP; victim <= bK; ++victim){
			MvvLvaScores[victim][attacker] = VictimScore[victim] + 6 - (VictimScore[attacker]/100);
		}
	}	
}

int MoveExists(S_BOARD *pos, const int move){
	S_MOVELIST list[1];
	GenerateAllMoves(pos, list);

	int moveNum = 0;
	for (moveNum = 0; moveNum < list->count; ++moveNum){
		if (!MakeMove(pos, list->moves[moveNum].move)){
			continue;
		}
		TakeMove(pos, move);
		if (list->moves[moveNum].move == move){
			return TRUE;
		}
	}
	return FALSE;
}

static inline void add_move(S_MOVELIST *move_list, int move)
{
    // strore move
    move_list->moves[move_list->count].move = move;
    
    // increment move count
    move_list->count++;
}

void GenerateAllMoves(const S_BOARD *pos, S_MOVELIST *move_list){
	move_list->count = 0;

    int source_square, target_square;
    U64 bitboard, attacks;

	for (int piece = wP; piece <= bK; ++piece)
    {
        // init piece bitboard copy
        bitboard = pos->bitboards[piece];
        
        // generate WHITE pawns & WHITE king castling moves
        if (pos->side == WHITE)
        {
            // pick up WHITE pawn bitboards index
            if (piece == wP)
            {
                // loop over WHITE pawns within WHITE pawn bitboard
                while (bitboard)
                {
                    // init source square
                    source_square = get_ls1b_index(bitboard);
                    
                    // init target square
                    target_square = source_square - 8;
                    
                    // generate quiet pawn moves
                    if (!(target_square < a8) && !GETBIT(pos->occupancies[BOTH], target_square))
                    {
                        if (source_square >= a7 && source_square <= h7)
                        {                            
                            add_move(move_list, encode_move(source_square, target_square, piece, wQ, 0, 0, 0, 0));
                            add_move(move_list, encode_move(source_square, target_square, piece, wR, 0, 0, 0, 0));
                            add_move(move_list, encode_move(source_square, target_square, piece, wB, 0, 0, 0, 0));
                            add_move(move_list, encode_move(source_square, target_square, piece, wN, 0, 0, 0, 0));
                        }
                        
                        else
                        {
                            // one square ahead pawn move
                            add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
                            
                            // two squares ahead pawn move
                            if ((source_square >= a2 && source_square <= h2) && !GETBIT(pos->occupancies[BOTH], target_square - 8)){
                                add_move(move_list, encode_move(source_square, target_square - 8, piece, 0, 0, 1, 0, 0));
                            }
                        }
                    }
                    
                    // init pawn attacks bitboard
                    attacks = pawn_attacks[pos->side][source_square] & pos->occupancies[BLACK];
                    
                    // generate pawn captures
                    while (attacks)
                    {
                        // init target square
                        target_square = get_ls1b_index(attacks);
                        
                        // pawn promotion
                        if (source_square >= a7 && source_square <= h7)
                        {
                            add_move(move_list, encode_move(source_square, target_square, piece, wQ, 1, 0, 0, 0));
                            add_move(move_list, encode_move(source_square, target_square, piece, wR, 1, 0, 0, 0));
                            add_move(move_list, encode_move(source_square, target_square, piece, wR, 1, 0, 0, 0));
                            add_move(move_list, encode_move(source_square, target_square, piece, bN, 1, 0, 0, 0));
                        }
                        
                        else{
                            // one square ahead pawn move
                            add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));
                        }
                        // pop ls1b of the pawn attacks
                        POPLS1B(attacks, target_square);
                    }
                    
                    // generate enpassant captures
                    if (pos->enPas != NO_SQ)
                    {
                        // lookup pawn attacks and bitwise AND with enpassant square (bit)
                        U64 enpassant_attacks = pawn_attacks[pos->side][source_square] & (1ULL << pos->enPas);
                        
                        // make sure enpassant capture available
                        if (enpassant_attacks)
                        {
                            // init enpassant capture target square
                            int target_enpassant = get_ls1b_index(enpassant_attacks);
                            add_move(move_list, encode_move(source_square, target_enpassant, piece, 0, 1, 0, 1, 0));
                        }
                    }
                    
                    // pop ls1b from piece bitboard copy
                    POPLS1B(bitboard, source_square);
                }
            }
            
            // castling moves
            if (piece == wK)
            {
                // king pos->side castling is available
                if (pos->castlePerm & WKCA)
                {

                    // make sure square between king and king's rook are empty
                    if (!GETBIT(pos->occupancies[BOTH], f1) && !GETBIT(pos->occupancies[BOTH], g1))
                    {
                        // make sure king and the f1 squares are not under attacks
                        if (!IsSquareAttackedBB(e1, BLACK, pos) && !IsSquareAttackedBB(f1, BLACK, pos)){
                            add_move(move_list, encode_move(e1, g1, piece, 0, 0, 0, 0, 1));
                        }
                    }
                }
                
                // queen side castling is available
                if (pos->castlePerm & WQCA)
                {

                    // make sure square between king and queen's rook are empty
                    if (!GETBIT(pos->occupancies[BOTH], d1) && !GETBIT(pos->occupancies[BOTH], c1) && !GETBIT(pos->occupancies[BOTH], b1))
                    {
                        // make sure king and the d1 squares are not under attacks
                        if (!IsSquareAttackedBB(e1, BLACK, pos) && !IsSquareAttackedBB(d1, BLACK, pos)){
                            add_move(move_list, encode_move(e1, c1, piece, 0, 0, 0, 0, 1));
                        }
                    }
                }
            }
        }
        
        // generate BLACK pawns & BLACK king castling moves
        else
        {
            // pick up BLACK pawn bitboards index
            // pick up WHITE pawn bitboards index
            if (piece == bP)
            {
                // loop over WHITE pawns within WHITE pawn bitboard
                while (bitboard)
                {
                    // init source square
                    source_square = get_ls1b_index(bitboard);
                    
                    // init target square
                    target_square = source_square + 8;
                    
                    // generate quiet pawn moves
                    if (!(target_square > h1) && !GETBIT(pos->occupancies[BOTH], target_square))
                    {
                       // pawn promotion
                        if (source_square >= a2 && source_square <= h2)
                        {
                            add_move(move_list, encode_move(source_square, target_square, piece, bQ, 0, 0, 0, 0));
                            add_move(move_list, encode_move(source_square, target_square, piece, bR, 0, 0, 0, 0));
                            add_move(move_list, encode_move(source_square, target_square, piece, bB, 0, 0, 0, 0));
                            add_move(move_list, encode_move(source_square, target_square, piece, bN, 0, 0, 0, 0));
                        }
                        
                        else
                        {
                            // one square ahead pawn move
                            add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
                            
                            // two squares ahead pawn move
                            if ((source_square >= a7 && source_square <= h7) && !GETBIT(pos->occupancies[BOTH], target_square + 8)){
                                add_move(move_list, encode_move(source_square, target_square + 8, piece, 0, 0, 1, 0, 0));
                            }
                        }
                    }
                    
                    // init pawn attacks bitboard
                    attacks = pawn_attacks[pos->side][source_square] & pos->occupancies[WHITE];
                    
                    // generate pawn captures
                    while (attacks)
                    {
                        // init target square
                        target_square = get_ls1b_index(attacks);
                         // pawn promotion
                        if (source_square >= a2 && source_square <= h2)
                        {
                            add_move(move_list, encode_move(source_square, target_square, piece, bQ, 1, 0, 0, 0));
                            add_move(move_list, encode_move(source_square, target_square, piece, bR, 1, 0, 0, 0));
                            add_move(move_list, encode_move(source_square, target_square, piece, bB, 1, 0, 0, 0));
                            add_move(move_list, encode_move(source_square, target_square, piece, bN, 1, 0, 0, 0));
                        }
                        
                        else{
                            // one square ahead pawn move
                            add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));
                        }
                        // pop ls1b of the pawn attacks
                        POPLS1B(attacks, target_square);
                    }
                    
                    // generate enpassant captures
                    if (pos->enPas != NO_SQ)
                    {
                        // lookup pawn attacks and bitwise AND with enpassant square (bit)
                        U64 enpassant_attacks = pawn_attacks[pos->side][source_square] & (1ULL << pos->enPas);
                        
                        // make sure enpassant capture available
                        if (enpassant_attacks)
                        {
                            // init enpassant capture target square
                            int target_enpassant = get_ls1b_index(enpassant_attacks);
                            add_move(move_list, encode_move(source_square, target_enpassant, piece, 0, 1, 0, 1, 0));
                        }
                    }
                    
                    // pop ls1b from piece bitboard copy
                    POPLS1B(bitboard, source_square);
                }
            }
            
            // castling moves
            if (piece == bK)
            {
                // king side castling is available
                if (pos->castlePerm & BKCA)
                {
                    // make sure square between king and king's rook are empty
                    if (!GETBIT(pos->occupancies[BOTH], f8) && !GETBIT(pos->occupancies[BOTH], g8))
                    {
                        // make sure king and the f8 squares are not under attacks
                        if (!IsSquareAttackedBB(e8, WHITE, pos) && !IsSquareAttackedBB(f8, WHITE, pos)){
                            add_move(move_list, encode_move(e8, g8, piece, 0, 0, 0, 0, 1));
                        }

                    }
                }
                
                // queen side castling is available
                if (pos->castlePerm & BQCA)
                {
                    // make sure square between king and queen's rook are empty
                    if (!GETBIT(pos->occupancies[BOTH], d8) && !GETBIT(pos->occupancies[BOTH], c8) && !GETBIT(pos->occupancies[BOTH], b8))
                    {
                        // make sure king and the d8 squares are not under attacks
                        if (!IsSquareAttackedBB(e8, WHITE, pos) && !IsSquareAttackedBB(d8, WHITE, pos)){
                            add_move(move_list, encode_move(e8, c8, piece, 0, 0, 0, 0, 1));
                        }
                    }
                }
            }
        }
        // genarate knight moves
        if ((pos->side == WHITE) ? piece == wN : piece == bN)
        {
            // loop over source squares of piece bitboard copy
            while (bitboard)
            {
                // init source square
                source_square = get_ls1b_index(bitboard);
                
                // init piece attacks in order to get set of target squares
                attacks = knight_attacks[source_square] & ((pos->side == WHITE) ? ~pos->occupancies[WHITE] : ~pos->occupancies[BLACK]);
                
                // loop over target squares available from generated attacks
                while (attacks)
                {
                    // init target square
                    target_square = get_ls1b_index(attacks);    
                    
                    // quiet move
                    if (!GETBIT(((pos->side == WHITE) ? pos->occupancies[BLACK] : pos->occupancies[WHITE]), target_square))
                        add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
                    
                    else{
                        // capture move
                        add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));
                    }
                    // pop ls1b in current attacks set
                    POPLS1B(attacks, target_square);
                }
                
                
                // pop ls1b of the current piece bitboard copy
                POPLS1B(bitboard, source_square);
            }
        }
        // generate bishop moves
        if ((pos->side == WHITE) ? piece == wB : piece == bB)
        {
            // loop over source squares of piece bitboard copy
            while (bitboard)
            {
                // init source square
                source_square = get_ls1b_index(bitboard);
                
                // init piece attacks in order to get set of target squares
                attacks = GetBishopAttacks(source_square, pos->occupancies[BOTH]) & ((pos->side == WHITE) ? ~pos->occupancies[WHITE] : ~pos->occupancies[BLACK]);
                
                // loop over target squares available from generated attacks
                while (attacks)
                {
                    // init target square
                    target_square = get_ls1b_index(attacks);    
                    
                    // quiet move
                    if (!GETBIT(((pos->side == WHITE) ? pos->occupancies[BLACK] : pos->occupancies[WHITE]), target_square)){
                        add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
                    }
                    else{
                        // capture move
                        add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));
                    }
                    // pop ls1b in current attacks set
                    POPLS1B(attacks, target_square);
                }
                
                
                // pop ls1b of the current piece bitboard copy
                POPLS1B(bitboard, source_square);
            }
        }

        // generate rook moves
        if ((pos->side == WHITE) ? piece == wR : piece == bR)
        {
            // loop over source squares of piece bitboard copy
            while (bitboard)
            {
                // init source square
                source_square = get_ls1b_index(bitboard);
                
                // init piece attacks in order to get set of target squares
                attacks = GetRookAttacks(source_square, pos->occupancies[BOTH]) & ((pos->side == WHITE) ? ~pos->occupancies[WHITE] : ~pos->occupancies[BLACK]);
                
                // loop over target squares available from generated attacks
                while (attacks)
                {
                    // init target square
                    target_square = get_ls1b_index(attacks);    
                    
                    // quiet move
                    if (!GETBIT(((pos->side == WHITE) ? pos->occupancies[BLACK] : pos->occupancies[WHITE]), target_square)){
                        add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
                    }
                    else{
                        // capture move
                        add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));
                    }
                    // pop ls1b in current attacks set
                    POPLS1B(attacks, target_square);
                }
                
                
                // pop ls1b of the current piece bitboard copy
                POPLS1B(bitboard, source_square);
            }
        }
        //generate queen moves
        if ((pos->side == WHITE) ? piece == wQ : piece == bQ)
        {
            // loop over source squares of piece bitboard copy
            while (bitboard)
            {
                // init source square
                source_square = get_ls1b_index(bitboard);
                
                // init piece attacks in order to get set of target squares
                attacks = GetQueenAttacks(source_square, pos->occupancies[BOTH]) & ((pos->side == WHITE) ? ~pos->occupancies[WHITE] : ~pos->occupancies[BLACK]);
                
                // loop over target squares available from generated attacks
                while (attacks)
                {
                    // init target square
                    target_square = get_ls1b_index(attacks);    
                    
                    // quiet move
                    if (!GETBIT(((pos->side == WHITE) ? pos->occupancies[BLACK] : pos->occupancies[WHITE]), target_square))
                        add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
                    
                    else{
                        // capture move
                        add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));
                    }
                    // pop ls1b in current attacks set
                    POPLS1B(attacks, target_square);
                }
                
                
                // pop ls1b of the current piece bitboard copy
                POPLS1B(bitboard, source_square);
            }
        }
        // generate king moves
        if ((pos->side == WHITE) ? piece == wK : piece == bK)
        {
            // loop over source squares of piece bitboard copy
            while (bitboard)
            {
                // init source square
                source_square = get_ls1b_index(bitboard);
                
                // init piece attacks in order to get set of target squares
                attacks = king_attacks[source_square] & ((pos->side == WHITE) ? ~pos->occupancies[WHITE] : ~pos->occupancies[BLACK]);
                
                // loop over target squares available from generated attacks
                while (attacks)
                {
                    // init target square
                    target_square = get_ls1b_index(attacks);    
                    
                    // quiet move
                    if (!GETBIT(((pos->side == WHITE) ? pos->occupancies[BLACK] : pos->occupancies[WHITE]), target_square)){
                        add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
                    }else{
                        // capture move
                        add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));
                    }
                    // pop ls1b in current attacks set
                    POPLS1B(attacks, target_square);
                }

                // pop ls1b of the current piece bitboard copy
                POPLS1B(bitboard, source_square);
            }
        }
    }
}

/*void _GenerateAllMoves(const S_BOARD *pos, S_MOVELIST *list){
    list->count = 0;
    
    int pce = EMPTY;
	int side = pos->side;
	int sq = 0; int t_sq = 0;
	int pceNum = 0;
	int dir = 0;
	int index = 0;
	int pceIndex = 0;

    if(side == WHITE) {
        for (pceNum = 0; pceNum < pos->pceNum[wP]; ++pceNum){
            sq = pos->pList[wP][pceNum];
            ASSERT(SqOnBoard(sq));
            if(pos->pieces[sq + 10] == EMPTY) {
				AddWhitePawnMove(pos, sq, sq+10, list);
				if(RanksBrd[sq] == RANK_2 && pos->pieces[sq + 20] == EMPTY) {
					AddQuietMove(pos, MOVE(sq,(sq+20),EMPTY,EMPTY,MFLAGPS),list);
				}
			}

			if(!SQOFFBOARD(sq + 9) && PieceCol[pos->pieces[sq + 9]] == BLACK) {
				AddWhitePawnCapMove(pos, sq, sq+9, pos->pieces[sq + 9], list);
			}
			if(!SQOFFBOARD(sq + 11) && PieceCol[pos->pieces[sq + 11]] == BLACK) {
				AddWhitePawnCapMove(pos, sq, sq+11, pos->pieces[sq + 11], list);
			}

			if(pos->enPas != NO_SQ) {
				if(sq + 9 == pos->enPas) {
					AddEnpassantMove(pos, MOVE(sq,sq + 9,EMPTY,EMPTY,MFLAGEP), list);
				}
				if(sq + 11 == pos->enPas) {
					AddEnpassantMove(pos, MOVE(sq,sq + 11,EMPTY,EMPTY,MFLAGEP), list);
				}
			}
        }
        if(pos->castlePerm & WKCA) {
			if(pos->pieces[F1] == EMPTY && pos->pieces[G1] == EMPTY) {
				if(!SqAttacked(E1,BLACK,pos) && !SqAttacked(F1,BLACK,pos) ) {
					AddQuietMove(pos, MOVE(E1, G1, EMPTY, EMPTY, MFLAGCA), list);
				}
			}
		}

		if(pos->castlePerm & WQCA) {
			if(pos->pieces[D1] == EMPTY && pos->pieces[C1] == EMPTY && pos->pieces[B1] == EMPTY) {
				if(!SqAttacked(E1,BLACK,pos) && !SqAttacked(D1,BLACK,pos) ) {
					AddQuietMove(pos, MOVE(E1, C1, EMPTY, EMPTY, MFLAGCA), list);
				}
			}
		}
    }else {
        for(pceNum = 0; pceNum < pos->pceNum[bP]; ++pceNum) {
			sq = pos->pList[bP][pceNum];
			ASSERT(SqOnBoard(sq));

			if(pos->pieces[sq - 10] == EMPTY) {
				AddBlackPawnMove(pos, sq, sq-10, list);
				if(RanksBrd[sq] == RANK_7 && pos->pieces[sq - 20] == EMPTY) {
					AddQuietMove(pos, MOVE(sq,(sq-20),EMPTY,EMPTY,MFLAGPS),list);
				}
			}

			if(!SQOFFBOARD(sq - 9) && PieceCol[pos->pieces[sq - 9]] == WHITE) {
				AddBlackPawnCapMove(pos, sq, sq-9, pos->pieces[sq - 9], list);
			}

			if(!SQOFFBOARD(sq - 11) && PieceCol[pos->pieces[sq - 11]] == WHITE) {
				AddBlackPawnCapMove(pos, sq, sq-11, pos->pieces[sq - 11], list);
			}
			if(pos->enPas != NO_SQ) {
				if(sq - 9 == pos->enPas) {
					AddEnpassantMove(pos, MOVE(sq,sq - 9,EMPTY,EMPTY,MFLAGEP), list);
				}
				if(sq - 11 == pos->enPas) {
					AddEnpassantMove(pos, MOVE(sq,sq - 11,EMPTY,EMPTY,MFLAGEP), list);
				}
			}
        }
        if(pos->castlePerm &  BKCA) {
			if(pos->pieces[F8] == EMPTY && pos->pieces[G8] == EMPTY) {
				if(!SqAttacked(E8,WHITE,pos) && !SqAttacked(F8,WHITE,pos) ) {
					AddQuietMove(pos, MOVE(E8, G8, EMPTY, EMPTY, MFLAGCA), list);
				}
			}
		}

		if(pos->castlePerm &  BQCA) {
			if(pos->pieces[D8] == EMPTY && pos->pieces[C8] == EMPTY && pos->pieces[B8] == EMPTY) {
				if(!SqAttacked(E8,WHITE,pos) && !SqAttacked(D8,WHITE,pos) ) {
					AddQuietMove(pos, MOVE(E8, C8, EMPTY, EMPTY, MFLAGCA), list);
				}
			}
		}
    }
    
    pceIndex = LoopSlideIndex[side];
    pce = LoopSlidePce[pceIndex++];

    while (pce != 0){
        
        ASSERT(PieceValid(pce));
        for (pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum){
            sq = pos->pList[pce][pceNum];
            
            ASSERT(SqOnBoard(sq));

            for (index = 0; index < NumDir[pce]; ++index){
                dir = PceDir[pce][index];
				t_sq = sq + dir;

				while(!SQOFFBOARD(t_sq)) {
					// BLACK ^ 1 == WHITE       WHITE ^ 1 == BLACK
					if(pos->pieces[t_sq] != EMPTY) {
						if( PieceCol[pos->pieces[t_sq]] == (side ^ 1)) {
                            AddCaptureMove(pos, MOVE(sq, t_sq, pos->pieces[t_sq], EMPTY, 0), list);
						}
						break;
					}
                    AddQuietMove(pos, MOVE(sq, t_sq, EMPTY, EMPTY, 0), list);
					t_sq += dir;
				}
            }
        }
        pce = LoopSlidePce[pceIndex++];
    }

    
    pceIndex = LoopNonSlideIndex[side];
    pce = LoopNonSlidePce[pceIndex++];

    while (pce != 0){
        ASSERT(PieceValid(pce));

        for (pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum){
            sq = pos->pList[pce][pceNum];
            ASSERT(SqOnBoard(sq));

            for (index = 0; index < NumDir[pce]; ++index){
                dir = PceDir[pce][index];
                t_sq = sq + dir;

                if (SQOFFBOARD(t_sq)){
                    continue;
                }

                if (pos->pieces[t_sq] != EMPTY){
                    if (PieceCol[pos->pieces[t_sq]] == side ^ 1){
                        AddCaptureMove(pos, MOVE(sq, t_sq, pos->pieces[t_sq], EMPTY, 0), list);
                    }
                    continue;
                }
                AddQuietMove(pos, MOVE(sq, t_sq, EMPTY, EMPTY, 0), list);
            }
        }
        
        pce = LoopNonSlidePce[pceIndex++];
    }
};*/

void GenerateAllCaps(const S_BOARD *pos, S_MOVELIST *list){

}

/*void GenerateAllCaps(const S_BOARD *pos, S_MOVELIST *list) {

	ASSERT(CheckBoard(pos));

	list->count = 0;

	int pce = EMPTY;
	int side = pos->side;
	int sq = 0; int t_sq = 0;
	int pceNum = 0;
	int dir = 0;
	int index = 0;
	int pceIndex = 0;

	if(side == WHITE) {

		for(pceNum = 0; pceNum < pos->pceNum[wP]; ++pceNum) {
			sq = pos->pList[wP][pceNum];
			ASSERT(SqOnBoard(sq));

			if(!SQOFFBOARD(sq + 9) && PieceCol[pos->pieces[sq + 9]] == BLACK) {
				AddWhitePawnCapMove(pos, sq, sq+9, pos->pieces[sq + 9], list);
			}
			if(!SQOFFBOARD(sq + 11) && PieceCol[pos->pieces[sq + 11]] == BLACK) {
				AddWhitePawnCapMove(pos, sq, sq+11, pos->pieces[sq + 11], list);
			}

			if(pos->enPas != NO_SQ) {
				if(sq + 9 == pos->enPas) {
					AddEnpassantMove(pos, MOVE(sq,sq + 9,EMPTY,EMPTY,MFLAGEP), list);
				}
				if(sq + 11 == pos->enPas) {
					AddEnpassantMove(pos, MOVE(sq,sq + 11,EMPTY,EMPTY,MFLAGEP), list);
				}
			}
		}

	} else {

		for(pceNum = 0; pceNum < pos->pceNum[bP]; ++pceNum) {
			sq = pos->pList[bP][pceNum];
			ASSERT(SqOnBoard(sq));

			if(!SQOFFBOARD(sq - 9) && PieceCol[pos->pieces[sq - 9]] == WHITE) {
				AddBlackPawnCapMove(pos, sq, sq-9, pos->pieces[sq - 9], list);
			}

			if(!SQOFFBOARD(sq - 11) && PieceCol[pos->pieces[sq - 11]] == WHITE) {
				AddBlackPawnCapMove(pos, sq, sq-11, pos->pieces[sq - 11], list);
			}
			if(pos->enPas != NO_SQ) {
				if(sq - 9 == pos->enPas) {
					AddEnpassantMove(pos, MOVE(sq,sq - 9,EMPTY,EMPTY,MFLAGEP), list);
				}
				if(sq - 11 == pos->enPas) {
					AddEnpassantMove(pos, MOVE(sq,sq - 11,EMPTY,EMPTY,MFLAGEP), list);
				}
			}
		}
	}

	
	pceIndex = LoopSlideIndex[side];
	pce = LoopSlidePce[pceIndex++];
	while( pce != 0) {
		ASSERT(PieceValid(pce));

		for(pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
			sq = pos->pList[pce][pceNum];
			ASSERT(SqOnBoard(sq));

			for(index = 0; index < NumDir[pce]; ++index) {
				dir = PceDir[pce][index];
				t_sq = sq + dir;

				while(!SQOFFBOARD(t_sq)) {
					// BLACK ^ 1 == WHITE       WHITE ^ 1 == BLACK
					if(pos->pieces[t_sq] != EMPTY) {
						if( PieceCol[pos->pieces[t_sq]] == (side ^ 1)) {
							AddCaptureMove(pos, MOVE(sq, t_sq, pos->pieces[t_sq], EMPTY, 0), list);
						}
						break;
					}
					t_sq += dir;
				}
			}
		}

		pce = LoopSlidePce[pceIndex++];
	}

	
	pceIndex = LoopNonSlideIndex[side];
	pce = LoopNonSlidePce[pceIndex++];

	while( pce != 0) {
		ASSERT(PieceValid(pce));

		for(pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
			sq = pos->pList[pce][pceNum];
			ASSERT(SqOnBoard(sq));

			for(index = 0; index < NumDir[pce]; ++index) {
				dir = PceDir[pce][index];
				t_sq = sq + dir;

				if(SQOFFBOARD(t_sq)) {
					continue;
				}

				// BLACK ^ 1 == WHITE       WHITE ^ 1 == BLACK
				if(pos->pieces[t_sq] != EMPTY) {
					if( PieceCol[pos->pieces[t_sq]] == (side ^ 1)) {
						AddCaptureMove(pos, MOVE(sq, t_sq, pos->pieces[t_sq], EMPTY, 0), list);
					}
					continue;
				}
			}
		}

		pce = LoopNonSlidePce[pceIndex++];
	}
}*/
