#include "defs.h"

char *PrSq(const int sq){
    static char SqStr[3];

    int file = FilesBrd[sq];
    int rank = RanksBrd[sq];

    sprintf(SqStr, "%c%c", ('a'+file), ('1'+rank));
    return SqStr;
}

char promoted_pieces[] = {
    [wQ] = 'q',
    [wR] = 'r',
    [wB] = 'b',
    [wN] = 'n',
	
    [bQ] = 'q',
    [bR] = 'r',
    [bB] = 'b',
    [bN] = 'n'
};

char *PrMove(const int move) {
	static char MvStr[6];

	if (get_move_promoted(move))
        sprintf(MvStr, "%s%s%c", square_to_coordinates[get_move_source(move)],square_to_coordinates[get_move_target(move)],promoted_pieces[get_move_promoted(move)]);
    else{
        sprintf(MvStr, "%s%s", square_to_coordinates[get_move_source(move)],square_to_coordinates[get_move_target(move)]);
	}
	// int ff = FilesBrd[FROMSQ(move)];
	// int rf = RanksBrd[FROMSQ(move)];
	// int ft = FilesBrd[TOSQ(move)];
	// int rt = RanksBrd[TOSQ(move)];

	// int promoted = PROMOTED(move);

	// if(promoted) {
	// 	char pchar = 'q';
	// 	if(IsKn(promoted)) {
	// 		pchar = 'n';
	// 	} else if(IsRQ(promoted) && !IsBQ(promoted))  {
	// 		pchar = 'r';
	// 	} else if(!IsRQ(promoted) && IsBQ(promoted))  {
	// 		pchar = 'b';
	// 	}
	// 	sprintf(MvStr, "%c%c%c%c%c", ('a'+ff), ('1'+rf), ('a'+ft), ('1'+rt), pchar);
	// } else {
	// 	sprintf(MvStr, "%c%c%c%c", ('a'+ff), ('1'+rf), ('a'+ft), ('1'+rt));
	// }

	return MvStr;
}

int ParseMove (char *ptrChar, S_BOARD *pos){
	// if (ptrChar[1] > '8' || ptrChar[1] < '1') return NOMOVE;
	// if (ptrChar[3] > '8' || ptrChar[3] < '1') return FALSE;
	// if (ptrChar[0] > 'h' || ptrChar[0] < 'a') return FALSE;
	// if (ptrChar[2] > 'h' || ptrChar[2] < 'a') return FALSE;

	// parse source square
    int source_square = (ptrChar[0] - 'a') + (8 - (ptrChar[1] - '0')) * 8;
    
    // parse target square
    int target_square = (ptrChar[2] - 'a') + (8 - (ptrChar[3] - '0')) * 8;

	//ASSERT(SqOnBoard(from) && SqOnBoard(to));


	S_MOVELIST list[1];
	generate_moves(list, pos);

	for (int move_count = 0; move_count < list->count; move_count++){
		// init move
        int move = list->moves[move_count].move;
        
        // make sure source & target squares are available within the generated move
        if (source_square == get_move_source(move) && target_square == get_move_target(move))
        {
            // init promoted piece
            int promoted_piece = get_move_promoted(move);
            
            // promoted piece is available
            if (promoted_piece)
            {
                // promoted to queen
                if ((promoted_piece == wQ || promoted_piece == bQ) && ptrChar[4] == 'q')
                    // return legal move
                    return move;
                
                // promoted to rook
                else if ((promoted_piece == wR || promoted_piece == bR) && ptrChar[4] == 'r')
                    // return legal move
                    return move;
                
                // promoted to bishop
                else if ((promoted_piece == wB || promoted_piece == bB) && ptrChar[4] == 'b')
                    // return legal move
                    return move;
                
                // promoted to knight
                else if ((promoted_piece == wN || promoted_piece == bN) && ptrChar[4] == 'n')
                    // return legal move
                    return move;
                
                // continue the loop on possible wrong promotions (e.g. "e7e8f")
                continue;
            }
            
            // return legal move
            return move;
        }
	}

	return NOMOVE;
}

static inline void PickNextMove(int movenum, S_MOVELIST *list)
{
    S_MOVE temp;
    int index = 0;
    int bestscore = 0;
    int bestnum = movenum;

    for (index = movenum; index < list->count; ++index)
    {
        if (list->moves[index].score > bestscore)
        {
            bestscore = list->moves[index].score;
            bestnum = index;
        }
    }

    temp = list->moves[movenum];
    list->moves[movenum] = list->moves[bestnum];
    list->moves[bestnum] = temp;
}

void PrintMoveList(S_MOVELIST *list) {
	int index = 0;
	int score = 0;
	int move = 0;
	printf("MoveList:\n");

	for(index = 0; index < list->count; ++index) {
		PickNextMove(index, list);
		move = list->moves[index].move;
		score = list->moves[index].score;

		printf("Move:%d > %s (score:%d)\n",index+1,PrMove(move),score);
	}
	printf("MoveList Total %d Moves:\n\n",list->count);
}


