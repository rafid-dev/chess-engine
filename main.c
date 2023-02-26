#include <stdio.h>
#include <string.h>
#include "defs.h"

//#include "perft_suite.h"

int main (){

    AllInit();

    //   S_BOARD pos[1];
    //   parse_fen(TRICKY_POSITION, pos);
    //   PerftTest(6, pos);
    // PerftSuite(5, pos , 117);
    

    // printf("%d\n", EvalPosition(pos));
    
    Uci_Loop();

    
    // // print_board(pos);
    // // printf("%s\n", PrMove(list->moves[0].move));
    // // make_move(pos, list->moves[0].move);
    // // print_board(pos);

    // PerftTest(6, pos);

    return 0;
}