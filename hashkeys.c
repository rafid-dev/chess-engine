#include "defs.h"

U64 GeneratePosKey(const S_BOARD *pos) {

	int sq = 0;
	U64 finalKey = 0;
	int piece = EMPTY;
	
	// pieces
	for (int sq = 0; sq < 64; ++sq) {
		piece = PieceAtBB(sq, pos);
		if (piece != EMPTY) {
			ASSERT(piece >= wP && piece <= bK);
			finalKey ^= PieceKeys[piece][sq];
		}
	}
	
	if(pos->side == WHITE) {
		finalKey ^= SideKey;
	}
		
	if(pos->enPas != no_sq) {
		ASSERT(pos->enPas>=0 && pos->enPas<64);
		finalKey ^= PieceKeys[EMPTY][pos->enPas];
	}
	
	ASSERT(pos->castlePerm>=0 && pos->castlePerm<=15);
	
	finalKey ^= CastleKeys[pos->castlePerm];
	
	return finalKey;
}