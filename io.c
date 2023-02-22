#include "defs.h"

char *PrSq(const int sq){
    static char SqStr[3];

    int file = FilesBrd[sq];
    int rank = RanksBrd[sq];

    sprintf(SqStr, "%c%c", ('a'+file), ('1'+rank));
    return SqStr;
}

char *PrMove(const int move) {

	static char MvStr[6];

	int ff = FilesBrd[get_move_source(move)];
	int rf = RanksBrd[get_move_source(move)];
	int ft = FilesBrd[get_move_target(move)];
	int rt = RanksBrd[get_move_target(move)];

	int promoted = get_move_promoted(move);

	if(promoted) {
		char pchar = 'q';
		if(IsKn(promoted)) {
			pchar = 'n';
		} else if(IsRQ(promoted) && !IsBQ(promoted))  {
			pchar = 'r';
		} else if(!IsRQ(promoted) && IsBQ(promoted))  {
			pchar = 'b';
		}
		sprintf(MvStr, "%s%s%c", square_to_coordinates[get_move_source(move)], square_to_coordinates[get_move_target(move)], pchar);
	} else {
		sprintf(MvStr, "%s%s", square_to_coordinates[get_move_source(move)], square_to_coordinates[get_move_target(move)]);
	}

	return MvStr;
}

int ParseMove (char *ptrChar, S_BOARD *pos){
	if (ptrChar[1] > '8' || ptrChar[1] < '1') return NOMOVE;
	if (ptrChar[3] > '8' || ptrChar[3] < '1') return FALSE;
	if (ptrChar[0] > 'h' || ptrChar[0] < 'a') return FALSE;
	if (ptrChar[2] > 'h' || ptrChar[2] < 'a') return FALSE;

	int from = FR2SQ(ptrChar[0] - 'a', ptrChar[1] - '1');
	int to = FR2SQ(ptrChar[2] - 'a', ptrChar[3] - '1');

	ASSERT(SqOnBoard(from) && SqOnBoard(to));

	S_MOVELIST list[1];
	GenerateAllMoves(pos, list);
	int MoveNum = 0;
	int Move = 0;
	int PromPce = EMPTY;

	for (MoveNum = 0; MoveNum < list->count; ++MoveNum){
		Move = list->moves[MoveNum].move;

		if (get_move_source(Move) == from && get_move_target(Move) == to){
			PromPce = get_move_promoted(Move);
			if (PromPce != EMPTY){
				if (IsRQ(PromPce) && !IsBQ(PromPce) && ptrChar[4] == 'r'){
					return Move;
				}else if (!IsRQ(PromPce) && IsBQ(PromPce) && ptrChar[4] == 'b'){
					return Move;
				}else if (IsRQ(PromPce) && IsBQ(PromPce) && ptrChar[4] == 'q'){
					return Move;
				}else if (IsKn(PromPce) && ptrChar[4] == 'n'){
					return Move;
				}
				continue;
			}
			return Move;
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
