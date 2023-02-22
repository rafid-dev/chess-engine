#include <stdio.h>
#include "defs.h"

long leafNodes;
long captures;
long castles;
long promotions;
long double_pushs;
long en_passants;

void Perft(int depth, S_BOARD *pos) {


	if(depth == 0) {
        leafNodes++;
        return;
    }	

    S_MOVELIST list[1];
    GenerateAllMoves(pos,list);
      
    int MoveNum = 0;
	for(MoveNum = 0; MoveNum < list->count; ++MoveNum) {	
       
        if ( !MakeMove(pos,list->moves[MoveNum].move))  {
            TakeMove(pos, list->moves[MoveNum].move);
            continue;
        }
        if (get_move_capture(list->moves[MoveNum].move)){
            captures++;
        }
        if (get_move_castling(list->moves[MoveNum].move)){
            castles++;
        }
        if (get_move_double(list->moves[MoveNum].move)){
            double_pushs++;
        }
        if(get_move_enpassant(list->moves[MoveNum].move)){
            en_passants++;
        }
        Perft(depth - 1, pos);
        TakeMove(pos, list->moves[MoveNum].move);
    }

    return;
}

void PerftTest(int depth, S_BOARD *pos) {


	PrintBoard(pos);
	printf("\nStarting Test To Depth:%d\n",depth);	
	leafNodes = 0;
    int start = GetTimeMs();
    S_MOVELIST list[1];
    GenerateAllMoves(pos,list);	
    PrintMoveList(list);
    
    
    int move;	    
    int MoveNum = 0;
     captures = 0;
     castles = 0;
     promotions = 0;
     double_pushs = 0;
     en_passants = 0;

	for(MoveNum = 0; MoveNum < list->count; ++MoveNum) {
        move = list->moves[MoveNum].move;
        if ( !MakeMove(pos,move))  {
            TakeMove(pos, move);
            continue;
        }
        if (get_move_capture(list->moves[MoveNum].move)){
            captures++;
        }
        if (get_move_castling(list->moves[MoveNum].move)){
            castles++;
        }
        if (get_move_double(list->moves[MoveNum].move)){
            double_pushs++;
        }
        if(get_move_enpassant(list->moves[MoveNum].move)){
            en_passants++;
        }
        long cumnodes = leafNodes;
        Perft(depth - 1, pos);
        TakeMove(pos, list->moves[MoveNum].move);        
        long oldnodes = leafNodes - cumnodes;
        printf("move %d : %s : %ld\n",MoveNum+1,PrMove(move),oldnodes);
    }
	
	printf("\nTest Complete : %ld nodes visited in %dms\n",leafNodes, GetTimeMs() - start);
    printf("Captures: %d Castles: %d Promotions: %d En_Passants %d\n", captures, castles, promotions, en_passants);

    return;
}
