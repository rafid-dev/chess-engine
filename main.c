#include <stdio.h>
#include "defs.h"

int main (){

    AllInit();

    S_BOARD board[1];
    S_SEARCHINFO info[1];

    ParseFen(START_FEN,board);


    char input[6];
    int move = NOMOVE;
    int pvnum = 0;
    int max = 0;

    info->depth = 1;

    while (TRUE){
        PrintBoard(board);
        fgets(input, 6, stdin);

        if (input[0] == 'q'){
            break;
        }else if (input[0] == 't'){
            TakeMove(board);
        }else if (input[0] == 's'){
            info->depth = 4;
            SearchPosition(board, info);
        }else {
            move = ParseMove(input, board);
            if (move != NOMOVE){
                StorePvMove(board, move);
                MakeMove(board, move);
                if (IsRepetition(board)){
                    printf("REP SEEN\n");
                }
            }else{
                printf("Move not parsed %s\n", input);
            }
        }

        fflush(stdin);
    }

    return 0;   
}