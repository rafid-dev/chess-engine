#include "defs.h"
#include <string.h>

#define HASH_PCE(pce,sq) (pos->posKey ^= (PieceKeys[(pce)][(sq)]))
#define HASH_CA (pos->posKey ^= (CastleKeys[(pos->castlePerm)]))
#define HASH_SIDE (pos->posKey ^= (SideKey))
#define HASH_EP (pos->posKey ^= (EnpassantKeys[(pos->enPas)]))

U64 bitboards_copy[13], occupancies_copy[3];
U64 hash_key_copy = 0ULL;
int side_copy;
int enpassant_copy;
int castle_copy;
int castlePerm;
int fiftyMove;

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

static void clearpiece(const int piece, const int sq, S_BOARD* pos) {
	int color = PieceToPieceColor[piece];
	HASH_PCE(piece, sq);
	pop_bit(pos->bitboards[piece], sq);
	pos->pieces[sq] = EMPTY;
	pop_bit(pos->occupancies[BOTH], sq);
	pop_bit(pos->occupancies[color], sq);
}

static void addpiece(const int piece, const int to, S_BOARD* pos) {
	int color = PieceToPieceColor[piece];
	set_bit(pos->bitboards[piece], to);
	set_bit(pos->occupancies[color], to);
	set_bit(pos->occupancies[BOTH], to);
	pos->pieces[to] = piece;
	HASH_PCE(piece, to);
}

void movepiece(const int piece, const int from, const int to, S_BOARD* pos) {
	clearpiece(piece, from, pos);
	addpiece(piece, to, pos);
}

void MakeNullMove(S_BOARD *pos) {

    pos->history[pos->ply].posKey = pos->posKey;

    if(pos->enPas != no_sq) HASH_EP;

    pos->history[pos->ply].move = NOMOVE;
    pos->history[pos->ply].fiftyMove = pos->fiftyMove;
    pos->history[pos->ply].enPas = pos->enPas;
    pos->history[pos->ply].castlePerm = pos->castlePerm;

    pos->enPas = no_sq;

    pos->side ^= 1;

	pos->ply++;
    pos->hisPly++;
    HASH_SIDE;
	
    return;
}

void TakeNullMove(S_BOARD *pos) {

    pos->hisPly--;
    pos->ply--;

    pos->castlePerm = pos->history[pos->ply].castlePerm;
    pos->fiftyMove = pos->history[pos->ply].fiftyMove;
    pos->enPas = pos->history[pos->ply].enPas;
	
    pos->side ^= 1;
	pos->posKey = pos->history[pos->ply].posKey;
}

int make_move (S_BOARD *pos, int move){

	pos->history[pos->ply].move = move;
	pos->history[pos->ply].castlePerm = pos->castlePerm;
	pos->history[pos->ply].enPas = pos->enPas;
	pos->history[pos->ply].posKey = pos->posKey;
	pos->history[pos->ply].fiftyMove = pos->fiftyMove;

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

	// handling capture moves
	if (enpass){
		if (pos->side == WHITE){
			clearpiece(bP, target_square + 8, pos);
			HASH_EP;
		}else{
			clearpiece(wP, target_square - 8, pos);
			HASH_EP;
		}
		pos->fiftyMove = 0;
	}else if (capture)
	{
		int captured_piece = pos->pieces[target_square];
		pos->history[pos->ply].capture = captured_piece;
		clearpiece(captured_piece, target_square, pos);
		pos->fiftyMove = 0;
	}

	if (piece == wP || piece == bP){
		pos->fiftyMove = 0;
	}
	
	pos->ply++;
	pos->hisPly++;
	
	clearpiece(piece, source_square, pos);
	addpiece(promoted_piece ? promoted_piece : piece, target_square, pos);

	if (pos->enPas != no_sq) HASH_EP;

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
	
	if (castling){
        switch (target_square){
        case (g1):
			// move H rook
			movepiece(wR, h1, f1, pos);
			break;

		case (c1):
			// move A rook
			movepiece(wR, a1, d1, pos);
			break;

			// black castles king side
		case (g8):
			// move H rook
			movepiece(bR, h8, f8, pos);
			break;

			// black castles queen side
		case (c8):
			// move A rook
			movepiece(bR, a8, d8, pos);
			break;
        }
    }
	HASH_CA;

	pos->castlePerm &= CastlePerm[source_square];
	pos->castlePerm &= CastlePerm[target_square];

	HASH_CA;

	pos->side ^= 1;
	HASH_SIDE;

	if (is_square_attacked((pos->side == WHITE) ? get_ls1b_index(pos->bitboards[bK]) : get_ls1b_index(pos->bitboards[wK]), pos->side, pos)){
		return FALSE;
	}
	
	return TRUE;
}

// restore board state
void take_move(S_BOARD *pos){
	pos->hisPly--;
	pos->ply--;

	pos->enPas = pos->history[pos->ply].enPas;
	pos->castlePerm = pos->history[pos->ply].castlePerm;
	pos->fiftyMove = pos->history[pos->ply].fiftyMove;
	
	int move = pos->history[pos->ply].move;

	int source_square = get_move_source(move);
    int target_square = get_move_target(move);
	int piece = get_move_piece(move);
    int promoted_piece = get_move_promoted(move);
    int capture = get_move_capture(move);
    int enpass = get_move_enpassant(move);
    int castling = get_move_castling(move);

	if (promoted_piece){
		clearpiece(promoted_piece, target_square, pos);
	}

	movepiece(piece, target_square, source_square, pos);

	if (enpass){
		(pos->side == BLACK) ? addpiece(bP, target_square + 8, pos)
			: addpiece(wP, target_square - 8, pos);
	}

	// handle castling moves
	if (castling) {
		// switch target square
		switch (target_square) {
			// white castles king side
		case (g1):
			// move H rook
			movepiece(wR, f1, h1, pos);
			break;

			// white castles queen side
		case (c1):
			// move A rook
			movepiece(wR, d1, a1, pos);
			break;

			// black castles king side
		case (g8):
			// move H rook
			movepiece(bR, f8, h8, pos);
			break;

			// black castles queen side
		case (c8):
			// move A rook
			movepiece(bR, d8, a8, pos);
			break;
		}
	}
	if (capture && !enpass){
		addpiece(pos->history[pos->ply].capture, target_square, pos);
	}

	pos->side ^= 1;

	pos->posKey = pos->history[pos->ply].posKey;
}