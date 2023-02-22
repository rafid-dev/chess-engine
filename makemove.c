#include "defs.h"
#include "string.h"

#define HASH_PCE(pce,sq) (pos->posKey ^= (PieceKeys[(pce)][(sq)]))
#define HASH_CA (pos->posKey ^= (CastleKeys[(pos->castlePerm)]))
#define HASH_SIDE (pos->posKey ^= (SideKey))
#define HASH_EP (pos->posKey ^= (PieceKeys[EMPTY][(pos->enPas)]))

const int CastlePerm[64] = {
     7, 15, 15, 15,  3, 15, 15, 11,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    13, 15, 15, 15, 12, 15, 15, 14
};

static void ClearPiece(const int piece, const int sq, S_BOARD* pos) {
	int color = PieceToPieceColor[piece];
	HASH_PCE(piece, sq);
	POPLS1B(pos->bitboards[piece], sq);
	pos->pieces[sq] = EMPTY;
	POPLS1B(pos->occupancies[BOTH], sq);
	POPLS1B(pos->occupancies[color], sq);
}

void AddPiece(const int piece, const int to, S_BOARD* pos) {
	int color = PieceToPieceColor[piece];
	SETBIT(pos->bitboards[piece], to);
	SETBIT(pos->occupancies[color], to);
	SETBIT(pos->occupancies[BOTH], to);
	pos->pieces[to] = piece;
	HASH_PCE(piece, to);
}

void MovePiece(const int piece, const int from, const int to, S_BOARD* pos) {
	ClearPiece(piece, from, pos);
	AddPiece(piece, to, pos);
}

S_BOARD BBCOPY[1];

void TakeMove(S_BOARD *pos, const int move){
    pos->hisPly--;
	pos->ply--;

	pos->enPas = pos->history[pos->ply].enPas;
	pos->fiftyMove = pos->history[pos->ply].fiftyMove;
	pos->castlePerm = pos->history[pos->ply].castlePerm;
    
    int source_square = get_move_source(move);
	int target_square = get_move_target(move);
	int piece = get_move_piece(move);
	int promoted_piece = get_move_promoted(move);
    int capture = get_move_capture(move);
	int enpass = get_move_enpassant(move);
    int castling = get_move_castling(move);

    int piececap = pos->history[pos->ply].capture;

	// handle pawn promotions
	if (promoted_piece) {
		ClearPiece(promoted_piece, target_square, pos);
	}

    MovePiece(piece, target_square, source_square, pos);

    if (enpass) {
		// erase the pawn depending on side to move
		(pos->side == BLACK) ? AddPiece(bP, target_square + 8, pos)
			: AddPiece(wP, target_square - 8, pos);
	}
    if (castling) {
		// switch target square
		switch (target_square) {
			// white castles king side
		case (g1):
			// move H rook
			MovePiece(wR, f1, h1, pos);
			break;

			// white castles queen side
		case (c1):
			// move A rook
			MovePiece(wR, d1, a1, pos);
			break;

			// black castles king side
		case (g8):
			// move H rook
			MovePiece(bR, f8, h8, pos);
			break;

			// black castles queen side
		case (c8):
			// move A rook
			MovePiece(bR, d8, a8, pos);
			break;
		}
	}
    // handling capture moves
	if(capture && !enpass){
		AddPiece(piececap, target_square, pos);
	}
	
    pos->side ^= 1;

    pos->posKey = pos->history[pos->ply].posKey;
}   

int MakeMove (S_BOARD *pos, int move){

	pos->history[pos->ply].fiftyMove = pos->fiftyMove;
	pos->history[pos->ply].enPas = pos->enPas;
	pos->history[pos->ply].castlePerm = pos->castlePerm;
    pos->history[pos->ply].move = move;

    pos->history[pos->ply].posKey = pos->posKey;

	// parse move
	int source_square = get_move_source(move);
	int target_square = get_move_target(move);
	int piece = get_move_piece(move);
	int promoted_piece = get_move_promoted(move);
	int capture = get_move_capture(move);
	int double_push = get_move_double(move);
	int enpass = get_move_enpassant(move);
	int castling = get_move_castling(move);

    pos->fiftyMove++;

    if (enpass) 
	{
		//If it's an enpass we remove the pawn corresponding to the opponent square 
		(pos->side == WHITE) ? POPLS1B(pos->bitboards[bP], target_square + 8) :
                              POPLS1B(pos->bitboards[wP], target_square - 8);

		if (pos->side == WHITE) 
		{
			ClearPiece(bP, target_square + 8, pos);
		}
		else {
			ClearPiece(wP, target_square - 8, pos);
		
		}
		pos->fiftyMove = 0;
	}

	if (capture)
	{
		int piececap = pos->pieces[target_square];
		ClearPiece(piececap, target_square, pos);
		pos->history[pos->ply].capture = piececap;
		pos->fiftyMove = 0;
	}

	if (piece == wP || piece == bP){
        pos->fiftyMove = 0;
    }
	
    ClearPiece(piece, source_square, pos);
	AddPiece(piece, target_square, pos);

    pos->hisPly++;
    pos->ply++;

    if (pos->enPas != no_sq){
        HASH_EP;
    }
    pos->enPas = no_sq;

    if (double_push){
        if (pos->side == WHITE){
            pos->enPas = target_square + 8;
            HASH_EP;
        }else{
            pos->enPas = target_square - 8;
            HASH_EP;
        }
    }

	if (promoted_piece){
		if (pos->side == WHITE){
			ClearPiece(wP, target_square, pos);
		}else{
			ClearPiece(bP, target_square, pos);
		}
	}

    if (castling){
        switch (target_square){
        case (g1):
			// move H rook
			MovePiece(wR, h1, f1, pos);
			break;

		case (c1):
			// move A rook
			MovePiece(wR, a1, d1, pos);
			break;

			// black castles king side
		case (g8):
			// move H rook
			MovePiece(bR, h8, f8, pos);
			break;

			// black castles queen side
		case (c8):
			// move A rook
			MovePiece(bR, a8, d8, pos);
			break;
        }
    }
	
    HASH_CA;
    pos->castlePerm &= CastlePerm[source_square];
	pos->castlePerm &= CastlePerm[target_square];

    HASH_CA;
	// change side
	pos->side ^= 1;
	HASH_SIDE;
    if (IsSquareAttackedBB((pos->side == WHITE) ? get_ls1b_index(pos->bitboards[bK]) : get_ls1b_index(pos->bitboards[wK]), pos->side, pos)){
        return FALSE;
    }
    
    return TRUE;
}

void MakeNullMove(S_BOARD *pos) {

    ASSERT(!SqAttacked(pos->KingSq[pos->side],pos->side^1,pos));

    pos->ply++;
    pos->history[pos->hisPly].posKey = pos->posKey;

    if(pos->enPas != no_sq) HASH_EP;

    pos->history[pos->hisPly].move = NOMOVE;
    pos->history[pos->hisPly].fiftyMove = pos->fiftyMove;
    pos->history[pos->hisPly].enPas = pos->enPas;
    pos->history[pos->hisPly].castlePerm = pos->castlePerm;
    pos->enPas = no_sq;

    pos->side ^= 1;
    pos->hisPly++;
    HASH_SIDE;
   
	ASSERT(pos->hisPly >= 0 && pos->hisPly < MAXGAMEMOVES);
	ASSERT(pos->ply >= 0 && pos->ply < MAXDEPTH);

    return;
}

void TakeNullMove(S_BOARD *pos) {

    pos->hisPly--;
    pos->ply--;

    if(pos->enPas != no_sq) HASH_EP;

    pos->castlePerm = pos->history[pos->hisPly].castlePerm;
    pos->fiftyMove = pos->history[pos->hisPly].fiftyMove;
    pos->enPas = pos->history[pos->hisPly].enPas;

    if(pos->enPas != no_sq) HASH_EP;
    pos->side ^= 1;
    HASH_SIDE;
  
	ASSERT(pos->hisPly >= 0 && pos->hisPly < MAXGAMEMOVES);
	ASSERT(pos->ply >= 0 && pos->ply < MAXDEPTH);
}